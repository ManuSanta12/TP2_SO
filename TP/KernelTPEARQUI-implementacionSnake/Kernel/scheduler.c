#include <scheduler.h>
#include <interrupts.h>
#include <memoryManager.h>
#include <queue.h>
#include <lib.h>
#include <pipe.h>
#include <linkedList.h>

extern uint64_t loadProcess(uint64_t, uint64_t , uint64_t , uint64_t ); // implement on assembler
extern void _int20h;                                                                 // implement int20h con assembler
extern void execute_next(uint64_t);
extern void execute_from_rip(uint64_t);
#define SCHEDULER_ADDRESS 0x60000
// tck and ppriorities
#define STACK_SIZE 4096
#define LEVELS 5
#define MIN_PRIORITY 1
#define MAX_PRIORITY 9
#define EOF -1
#define FIRST_PID
#define NUMBER_OF_PRIORITIES 9
#define DEFAULT_PRIORITY 4
priority_t priorities[NUMBER_OF_PRIORITIES] = {9, 8, 7, 6, 5, 4, 3, 2, 1};

uint8_t init=0;
typedef struct scheduler{
    Node *processes[MAX_PROC];
	LinkedList lvls[LEVELS + 1];

    // Schelduler states
    int processAmount;
    unsigned int processReadyCount;
    uint16_t nextPid;
    pid_t currentPid;
    int8_t remainingQuantum;
	int8_t killFg;
}schedulerCDT;

typedef schedulerCDT* schedulerADT;

schedulerADT scheduler;

void dummyProcess()
{
    while (1)
    {
        _hlt();
    }
}

void createScheduler()
{
    scheduler = (schedulerADT)SCHEDULER_ADDRESS;
	for (int i = 0; i < MAX_PROC; i++)
		scheduler->processes[i] = NULL;
	for (int i = 0; i < LEVELS + 1; i++)
		scheduler->lvls[i] = createLinkedList();
	scheduler->nextPid = 0;
	scheduler->killFg = 0;
    
}

static uint16_t getNextPid() {
	Node *process = NULL;
	for (int lvl = LEVELS - 1; lvl >= 0 && process == NULL; lvl--)
		if (!isEmpty(scheduler->lvls[lvl]))
			process = (Node *) (getFirst(scheduler->lvls[lvl]))->data;

	if (process == NULL)
		return FIRST_PID;
	return process->process.pid;
}



 PCB *getProcess(pid_t pid)
{
    Node *current = scheduler->active;
    while (current != NULL)
    {
        if (current->process.pid == pid)
        {
            return &(current->process);
        }
        else
        {
            current = current->next;
        }
    }
    current = scheduler->expired;
    while (current != NULL)
    {
        if (current->process.pid == pid)
        {
            return &(current->process);
        }
        else
        {
            current = current->next;
        }
    }
    return NULL;
}

priority_t get_priority(pid_t pid){
    PCB* auxPCB = getProcess(pid);
    return auxPCB->priority;
}

uint64_t getCurrentPid()
{
    if (scheduler->active != NULL)
    {
        return scheduler->active->process.pid;
    }
    return -1;
}

int blockProcess(pid_t pid)
{
    Node *current = scheduler->active;
    char found = 0;

    while (!found && current != NULL)
    {
        if (current->process.pid == pid)
        {
            found = 1;
            current->process.status = BLOCKED;
        }
        else
        {
            current = current->next;
        }
    }
    current = scheduler->expired;
    while (!found && current != NULL)
    {
        if (current->process.pid == pid)
        {
            found = 1;
            current->process.status = BLOCKED;
        }
        else
        {
            current = current->next;
        }
    }
    if (found)
    {
        scheduler->processReadyCount--;
        _int20h;
        return 0;
    }
    return -1;
}

int unblockProcess(pid_t pid)
{
    Node *current = scheduler->active;
    char found = 0;

    while (!found && current != NULL)
    {
        if (current->process.pid == pid)
        {
            found = 1;
            current->process.status = READY;
        }
        else
        {
            current = current->next;
        }
    }
    current = scheduler->expired;
    while (!found && current != NULL)
    {
        if (current->process.pid == pid)
        {
            found = 1;
            current->process.status = READY;
        }
        else
        {
            current = current->next;
        }
    }
    if (found)
    {
        scheduler->processReadyCount++;
        return 0;
    }
    return -1;
}

char **copy_argv(int argc, char **argv)
{
    char **new_argv = memory_manager_malloc(sizeof(char *) * argc);
    for (int i = 0; i < argc; i++)
    {
        new_argv[i] = strcpy(argv[i]);
    }
    return new_argv;
}

void processWrapper(main_foo fun, char **args) {
	int len = stringArrayLen(args);
	int retValue = fun(len, args);
	killCurrentProcess(retValue);
}

Node* new_process(uint64_t rip, int argc, char *argv[],pid_t parentPID, char* name){  
    Node* newProcess = memory_manager_malloc(sizeof(Node*));
    newProcess->process.pid = scheduler->processAmount++;
	newProcess->process.parentPID = parentPID;
	newProcess->process.stackBase = memory_manager_malloc(STACK_SIZE);
	newProcess->process.argv = copy_argv(argc, argv);
	newProcess->process.name = allocMemory(strlen(name) + 1);
	newProcess->process.name = strcpy(name);
	newProcess->process.priority = DEFAULT_PRIORITY;
	void *stackEnd = (void *) ((uint64_t) newProcess->process.stackBase + STACK_SIZE);
	newProcess->process.rsp = loadProcess(&processWrapper, rip, stackEnd, (void *) newProcess->process.argv);
	newProcess->process.status = READY;
	newProcess->process.zombie = newQueue();
	//newProcess->process.unkillable = unkillable;
	newProcess->process.waiting = 0;

	assignFileDescriptor(newProcess, STDIN, newProcess->process.fileDescriptors[STDIN], READ);
	assignFileDescriptor(newProcess, STDOUT, newProcess->process.fileDescriptors[STDOUT], WRITE);
	assignFileDescriptor(newProcess, STDERR, newProcess->process.fileDescriptors[STDERR], WRITE);
}

static void assignFileDescriptor(Node *new, uint8_t fdIndex, fd_t fdValue, uint8_t mode) {
	new->process.fileDescriptors[fdIndex] = fdValue;
	if (fdValue >= FDS)
		pipeOpenForPid(new->process.pid, fdValue, mode);
}

void close_all_fd(Node *p) {
	close_fds(p->process.pid, p->process.fileDescriptors[STDIN]);
	close_fds(p->process.pid, p->process.fileDescriptors[STDOUT]);
	close_fds(p->process.pid, p->process.fileDescriptors[STDERR]);
}

void free_process(Node *p) {
	freeQueue(p->process->zombie);
	free_memory_manager(p->process.stackBase);
	free_memory_manager(p->process.name);
	free_memory_manager(p->process.argv);
	free_memory_manager(p);
}

processInfo *save_info(Node *p) {
    processInfo* info = memory_manager_malloc(sizeof(processInfo));
	info->name = strcpy(p->process.name);
	info->pid = p->process.pid;
	info->parentPID = p->process.parentPID;
	info->stackBase = p->process.stackBase;
	info->rsp = p->process.rsp;
	info->priority = p->process.priority;
	info->status = p->process.status;
	info->foreground = p->process.fileDescriptors[STDIN] == STDIN;
	return info;
}

int processIsWaiting(Node *p, uint16_t pidToWait) {
	return p->process.waiting == pidToWait && p->process.status == BLOCKED;
}
/*
int getZombiesSnapshots(int processIndex, ProcessSnapshot psArray[], Process *nextProcess) {
	LinkedListADT zombieChildren = nextProcess->zombieChildren;
	begin(zombieChildren);
	while (hasNext(zombieChildren))
		loadSnapshot(&psArray[processIndex++], (Process *) next(zombieChildren));
	return processIndex;
}*/



void nextProcess()
{
    Node *current = scheduler->active;
    Node *previous = NULL;
    while (current != NULL && current->process.status == BLOCKED)
    {
        previous = current;
        current = current->next;
    }
    if (current != NULL)
    {
        if (previous != NULL)
        {
            previous->next = current->next;
            current->next = scheduler->active;
            scheduler->active = current;
        }
    }
    else
    {
        Node *aux = scheduler->active;
        scheduler->active = scheduler->expired;
        scheduler->expired = aux;

        current = scheduler->active;
        previous = NULL;
        while (current != NULL && current->process.status == BLOCKED)
        {
            previous = current;
            current = current->next;
        }
        if (previous != NULL && current != NULL)
        {
            previous->next = current->next;
            current->next = scheduler->active;
            scheduler->active = current;
        }
    }
}

int prepareDummy(pid_t pid)
{
    Node *current = scheduler->active;
    Node *previous = NULL;
    while (current != NULL && current->process.pid != pid)
    {
        previous = current;
        current = current->next;
    }

    if (current != NULL)
    {
        if (previous != NULL)
        {
            previous->next = current->next;
            current->next = scheduler->active;
            scheduler->active = current;
        }
    }
    else
    {
        current = scheduler->expired;
        previous = NULL;
        while (current != NULL && current->process.pid != pid)
        {
            previous = current;
            current = current->next;
        }
        if (current == NULL)
        {
            return -1;
        }
        if (previous == NULL)
        {
            scheduler->expired = current->next;
        }
        else
        {
            previous->next = current->next;
        }
        current->next = scheduler->active;
        scheduler->active = current;
    }
    return 0;
}
void nohagonada(){
    return;
}

uint64_t contextSwitch(uint64_t rsp)
{
    if(init==0){
        return;
    }
    scheduler->active->process.rsp=rsp;
    Node* aux = scheduler->active;
    // C1.1 y C1.3 (Todos)
    if (!scheduler->proccessBeingRun)
    {
        scheduler->proccessBeingRun = 1;
        // C1.1 o C1.3.1: NO HAY NADA CORRIENDOSE Y TENGO ALGO PARA CORRER
        if (scheduler->processReadyCount > 0)
        {
            nextProcess();
        }
        else
        { // C1.3.2 y C1.3.3
            prepareDummy(scheduler->placeholderProcessPid);
        }
        return scheduler->active->process.rsp;
        //return 0;
    }

    Node *currentProcess = scheduler->active;
    //currentProcess->process.rsp = rsp;
    // Si no tengo procesos en ready, es decir, estan todos bloqueados tengo que correr el placeholderProcess
    if (scheduler->processReadyCount == 0)
    {
        prepareDummy(scheduler->placeholderProcessPid);
        return scheduler->active->process.rsp;
    }

    if (currentProcess->process.status != BLOCKED && currentProcess->process.quantumsLeft > 0)
    {
        currentProcess->process.quantumsLeft--;
        return rsp;
    }

    // Acomodo el que termino de correr (no me interesa el status) en su lugar en la lista de expirados
    // teniendo en cuenta su prioridad.
    if (currentProcess->process.quantumsLeft == 0)
    {
        if (currentProcess->process.newPriority != -1)
        {
            currentProcess->process.priority = currentProcess->process.newPriority;
            currentProcess->process.newPriority = -1;
        }
        currentProcess->process.quantumsLeft = priorities[currentProcess->process.priority];

        Node *currentExpired = scheduler->expired;
        Node *previousExpired = NULL;
        while (currentExpired != NULL && currentProcess->process.priority >= currentExpired->process.priority)
        {
            previousExpired = currentExpired;
            currentExpired = currentExpired->next;
        }
        /*
            Debo colocar el current_process en el lugar indicado dentro de los expirados pero teniendo muy en cuenta
            que antes de cambiar el next de este nodo tengo que hacerlo en el active para evitar problemas.
            En cualquiera de ambos casos active tendra que ser igual a active->next porque paso el current_process a expirados.
        */
        scheduler->active = scheduler->active->next;
        if (previousExpired == NULL)
        {
            currentProcess->next = scheduler->expired;
            scheduler->expired = currentProcess;
        }
        else
        {
            previousExpired->next = currentProcess;
            currentProcess->next = currentExpired;
        }
        if (scheduler->active == NULL)
        {
            scheduler->active = scheduler->expired;
            scheduler->expired = NULL;
        }
    }
    nextProcess();
    if(scheduler->active->process.run == 1){
        execute_next(scheduler->active->process.rsp);  
    } 
    else {
        scheduler->active->process.run = 1;
        execute_from_rip(scheduler->active->process.rip);
    }
    nohagonada();
    kill_by_pid(scheduler->active->process.pid);//si llega a este punto es porque termino de ejecutar.
    return scheduler->active->process.rsp;
}

int killProcess(int returnValue, char autokill)
{
    Node *currentProcess = scheduler->active;

    pid_t blockedPid;
    while ((blockedPid = dequeuePid(currentProcess->process.blockedQueue)) != -1)
    {
        unblockProcess(blockedPid);
    }
    scheduler->active = currentProcess->next;
    if (currentProcess->process.status != BLOCKED)
    {
        scheduler->processReadyCount--;
    }
    for (int i = 0; i < currentProcess->process.argc; i++)
    {
        free_memory_manager(currentProcess->process.argv[i]);
    }
    free_memory_manager(currentProcess->process.argv);
    freeQueue(currentProcess->process.blockedQueue);
    free_memory_manager((void *)currentProcess->process.stackBase);
    if (currentProcess->process.pipe != NULL)
    {
        char msg[1] = {EOF};
        //(currentProcess->process.pipe, msg, 1);
        // pipeClose(currentProcess->process.pipe);
    }
    free_memory_manager(currentProcess);
    if (autokill)
    {
       scheduler->proccessBeingRun = 0;
        _int20h;
    }
    return returnValue;
}

int changePriority(pid_t pid, int priorityValue)
{
    if (priorityValue < 0 || priorityValue > 8)
    {
        return -1;
    }
    PCB *process = getProcess(pid);
    if (process == NULL)
    {
        return -1;
    }

    process->newPriority = priorityValue;
    return 0;
}

int yieldProcess()
{
    scheduler->active->process.quantumsLeft = 0;
    _int20h;
    return 0;
}

processInfo *getProccessesInfo()
{
    processInfo *first = NULL;
    processInfo *current = NULL;
    Queue currentNode = scheduler->active;
    pid_t firstPid = scheduler->active->process.pid;
    while (currentNode != NULL)
    {
        if (current != NULL)
        {
            current->next = (processInfo *)memory_manager_malloc(sizeof(processInfo));
            current = current->next;
        }
        else
        {
            current = (processInfo *)memory_manager_malloc(sizeof(processInfo));
        }
        current->pid = currentNode->process.pid;
        if (current->pid == firstPid)
        {
            first = current;
        }
        current->priority = currentNode->process.priority;
        current->stackBase = currentNode->process.stackBase;
        current->status = currentNode->process.status;
        currentNode = currentNode->next;
    }
    currentNode = scheduler->expired;
    /**/
    while (currentNode != NULL)
    {
        if (current != NULL)
        {
            current->next = (processInfo *)memory_manager_malloc(sizeof(processInfo));
            current = current->next;
        }
        else
        {
            current = (processInfo *)memory_manager_malloc(sizeof(processInfo));
        }
        current->pid = currentNode->process.pid;
        current->priority = currentNode->process.priority;
        current->stackBase = currentNode->process.stackBase;
        current->status = currentNode->process.status;
        currentNode = currentNode->next;
    }
    current->next = NULL;
    return first;
}

int kill_by_pid(pid_t pid){
    Node * current = scheduler->active;
    if(current->process.pid == pid){
        scheduler->active = current->next;
        free_memory_manager(current);
        return 1;
    }
    while(current!=NULL){
        if(current->next->process.pid == pid){
            Node* aux = current->next;
            current->next = current->next->next;
            free_memory_manager(aux);
            return 1;
        }
        current=current->next;
    }

    current = scheduler->expired;
    if(current->process.pid == pid){
        scheduler->expired = current->next;
        free_memory_manager(current);
        return 1;
    }
    while(current!=NULL){
        if(current->next->process.pid == pid){
            Node* aux = current->next;
            current->next = current->next->next;
            free_memory_manager(aux);
            return 1;
        }
        current=current->next;
    }
    return 0;
    
}