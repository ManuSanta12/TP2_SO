#include <scheduler.h>
#include <interrupts.h>
#include <memoryManager.h>
#include <queue.h>
#include <lib.h>
#include <pipe.h>

extern uint64_t loadProcess(uint64_t, uint64_t , uint64_t , uint64_t ); // implement on assembler
extern void _int20h;                                                                 // implement int20h con assembler
extern void execute_next(uint64_t);
extern void execute_from_rip(uint64_t);
#define SCHEDULER_ADDRESS 0x60000
// tck and ppriorities
#define STACK_SIZE 4096
#define MIN_PRIORITY 1
#define MAX_PRIORITY 9
#define EOF -1
#define NUMBER_OF_PRIORITIES 9
#define DEFAULT_PRIORITY 4
priority_t priorities[NUMBER_OF_PRIORITIES] = {9, 8, 7, 6, 5, 4, 3, 2, 1};

// Queues
uint8_t init=0;
typedef struct scheduler{
    Queue active;
    Queue processes;

    // Schelduler states
    int processAmount;
    unsigned int processReadyCount;
    pid_t placeholderProcessPid;
    unsigned int proccessBeingRun;
    uint16_t quantumsLeft;
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
    scheduler->processAmount = 0;
    scheduler->processReadyCount = 0;
    scheduler->proccessBeingRun = 0;
    scheduler->quantumsLeft = 0;
    scheduler->active=NULL;
    scheduler->processes=NULL;
    
    //scheduler->placeholderProcessPid = new_process((uint64_t)dummyProcess, 0, NULL);
    /*
    for (int i = 0; i <= 2; i++)
    {
        scheduler->active->process.fileDescriptors[i].mode = OPEN;
    }
    // PIPEOUT, PIPEIN
    for (int i = 3; i <= 4; i++)
    {
        scheduler->active->process.fileDescriptors[i].mode = CLOSED;
    }
    scheduler->active->process.lastFd = 4;
    scheduler->active->process.status = BLOCKED;
    
    scheduler->processReadyCount--;
    */
    init = 1;
    
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
    current = scheduler->processes;
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
    current = scheduler->processes;
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
    current = scheduler->processes;
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

static void start(fun f, int argc, char *argv[]) {
    scheduler->proccessBeingRun++;
    int status = f(argc, argv);
    killProcess(status,0);
    scheduler->proccessBeingRun--;
}

static context* new_context(fun foo, int argc, char**argv){
    context *context = memory_manager_malloc(sizeof(context));

    context->rdi = (uint64_t)foo;
    context->rsi = (uint64_t)argc;
    context->rdx = (uint64_t)argv;  
    context->rip = (uint64_t)&start;

    context->cs = CS;
    context->eflags = EFLAGS;
    context->ss = SS;

    context->rsp = (uint64_t)context;

    return context;
}

pid_t new_process(fun foo, int argc, char *argv[])
{
    Node *newProcess = memory_manager_malloc(sizeof(Node));
    newProcess->process.pid = scheduler->processAmount++;
    newProcess->process.priority = DEFAULT_PRIORITY;
    newProcess->process.quantumsLeft = priorities[DEFAULT_PRIORITY];
    newProcess->process.blockedQueue = newQueue();
    newProcess->process.newPriority = -1;
    newProcess->process.status = READY;
    scheduler->processReadyCount++;
    newProcess->process.argc = argc;
    newProcess->process.argv = copy_argv(argc, argv);
    
    newProcess->process.context = new_context(foo,argc,argv);

    // STDIN, STDOUT, STDERR, PIPEOUT, PIPEIN
    /*
    if (scheduler->active != NULL)
    {
        for (int i = 0; i <= scheduler->active->process.lastFd; i++)
        {
            newProcess->process.fileDescriptors[i].mode = scheduler->active->process.fileDescriptors[i].mode;
        }
        newProcess->process.lastFd = scheduler->active->process.lastFd;
        newProcess->process.pipe = scheduler->active->process.pipe;
    }*/
    /*
    uint64_t rsp = (uint64_t)memory_manager_malloc(4 * 1024);
    if (rsp == 0)
    {
        return -1;
    }
    newProcess->process.stackBase = rsp;
    uint64_t newRsp = (uint64_t)loadProcess(rip, rsp + 4 * 1024, newProcess->process.argc, (uint64_t)newProcess->process.argv);
    newProcess->process.rsp = newRsp;
    */
    if(scheduler->active==NULL){
        newProcess->next = NULL;
        scheduler->active = newProcess;
        scheduler->quantumsLeft = priorities[scheduler->active->process.priority];
    }
    newProcess->next = scheduler->processes;
    scheduler->processes = newProcess;


    return newProcess->process.pid;
}

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
        scheduler->active = scheduler->processes;
        scheduler->processes = aux;

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
        current = scheduler->processes;
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
            scheduler->processes = current->next;
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


context* contextSwitch(context* rsp)
{
    if(init==0){
        return rsp;
    }
    if(scheduler->processAmount==0){
        return rsp;
    }

    if(scheduler->active->process.run==0){
        scheduler->active->process.run = 1;
        return scheduler->active->process.context;    
    }   

    scheduler->active->process.context=rsp;
    //sigo corriendo el mismo
    if(scheduler->active != NULL && scheduler->quantumsLeft>0){
        scheduler->quantumsLeft--;
        return scheduler->active->process.context;
    }

    //voy a buscar otro, si no hay vuelvo al mismo.
    if(scheduler->processes==NULL){
        return scheduler->active->process.context;
    }
    else
    {
        Queue aux = scheduler->processes;
        int i=0;
        int a = scheduler->processAmount;
        while(aux != NULL && i < scheduler->processAmount){
            if(aux->process.status==READY && scheduler->active->process.pid!=aux->process.pid){
                scheduler->active = aux;
                scheduler->quantumsLeft =priorities[scheduler->active->process.priority];
                return scheduler->active->process.context;
            }
            i++;
            aux = aux->next;
        }
    }

    return scheduler->active->process.context;
}

int killProcess(int returnValue, char autokill)
{
    scheduler->active->process.status = TERMINATED;
    scheduler->quantumsLeft=0;
    scheduler->processReadyCount--;
    contextSwitch(scheduler->active->process.context);
    /*
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
    //free_memory_manager((void *)currentProcess->process.stackBase);
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
}*/
}

int changePriority(pid_t pid, int priorityValue){
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
        //current->stackBase = currentNode->process.stackBase;
        current->status = currentNode->process.status;
        currentNode = currentNode->next;
    }
    currentNode = scheduler->processes;
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
        //current->stackBase = currentNode->process.stackBase;
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

    current = scheduler->processes;
    if(current->process.pid == pid){
        scheduler->processes = current->next;
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