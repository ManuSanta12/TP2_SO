#include <scheduler.h>
#include <interrupts.h>
#include <memoryManager.h>
#include <queue.h>
#include <lib.h>
#include <pipe.h>

extern void _int20h;// implement int20h con assembler
extern void forced_schedule(void);

#define SCHEDULER_ADDRESS 0x60000
// tck and ppriorities
#define STACK_SIZE 4096
#define MAX_PROCESSES 100
#define MIN_PRIORITY 1
#define MAX_PRIORITY 9
#define EOF -1
#define NUMBER_OF_PRIORITIES 9
#define DEFAULT_PRIORITY 4
priority_t priorities[NUMBER_OF_PRIORITIES] = {9, 8, 7, 6, 5, 4, 3, 2, 1};

// Queues
uint8_t init=0;

uint32_t active;
PCB processes[MAX_PROCESSES];

// Schelduler states
int processAmount;
unsigned int processReadyCount;
pid_t placeholderProcessPid;
unsigned int proccessBeingRun;
uint16_t quantumsLeft;

//typedef schedulerCDT* schedulerADT;

//schedulerADT scheduler;

void dummyProcess()
{
    while (1)
    {
        _hlt();
    }
}

void createScheduler()
{
    //scheduler = (schedulerADT)SCHEDULER_ADDRESS;
    processAmount = 0;
    processReadyCount = 0;
    proccessBeingRun = 0;
    quantumsLeft = 0;
    active=-1;
    //process1es={};
    
    //placeholderProcessPid = new_process((uint64_t)dummyProcess, 0, NULL);
    /*
    for (int i = 0; i <= 2; i++)
    {
        active->process.fileDescriptors[i].mode = OPEN;
    }
    // PIPEOUT, PIPEIN
    for (int i = 3; i <= 4; i++)
    {
        active->process.fileDescriptors[i].mode = CLOSED;
    }
    active->process.lastFd = 4;
    active->process.status = BLOCKED;
    
    processReadyCount--;
    */
    init = 1;
    
}

 PCB *getProcess(pid_t pid)
{
    Node *current = active;
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
    current = processes;
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
    
    if (active != -1)
    {
        return processes[active].pid;
    }
    return -1;
}

int blockProcess(pid_t pid)
{
    Node *current = active;
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
    current = processes;
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
        processReadyCount--;
        _int20h;
        return 0;
    }
    return -1;
}

int unblockProcess(pid_t pid)
{
    Node *current = active;
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
    current = processes;
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
        processReadyCount++;
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
    int status = f(argc, argv);
    pid_t pid;
    for(int i=0;i<processAmount;i++){
        pid = processes[i].pid;
    }
    processes[active].status = TERMINATED;
    quantumsLeft=0;
    processReadyCount--; 
    forced_schedule();
    proccessBeingRun--;
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
    PCB newProcess;
    int a =0;
    newProcess.pid = processAmount++;
    a = processAmount;
    newProcess.priority = DEFAULT_PRIORITY;
    newProcess.quantumsLeft = priorities[DEFAULT_PRIORITY];
    newProcess.blockedQueue = newQueue();
    newProcess.newPriority = -1;
    newProcess.status = READY;
    //ReadyCount++;
    newProcess.argc = argc;
    newProcess.argv = copy_argv(argc, argv);
    
    newProcess.context = new_context(foo,argc,argv);

    // STDIN, STDOUT, STDERR, PIPEOUT, PIPEIN
    /*
    if (active != NULL)
    {
        for (int i = 0; i <= active->process.lastFd; i++)
        {
            newProcess->process.fileDescriptors[i].mode = active->process.fileDescriptors[i].mode;
        }
        newProcess->process.lastFd = active->process.lastFd;
        newProcess->process.pipe = active->process.pipe;
    }*/

    if(active==-1){
        active = 0;
    }
    processes[processAmount-1]=newProcess;
    
    pid_t pid;
    status_t st;
    for(int i=0;i<processAmount;i++){
        //pid = processes[i].pid;
        //st = processes[i].status;
    }
    return newProcess.pid;
}

void nextProcess()
{
    /*
    Node *current = active;
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
            current->next = active;
            active = current;
        }
    }
    else
    {
        Node *aux = active;
        active = processes;
        processes = aux;

        current = active;
        previous = NULL;
        while (current != NULL && current->process.status == BLOCKED)
        {
            previous = current;
            current = current->next;
        }
        if (previous != NULL && current != NULL)
        {
            previous->next = current->next;
            current->next = active;
            active = current;
        }
    }
    */
}

int prepareDummy(pid_t pid)
{
    /*
    Node *current = active;
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
            current->next = active;
            active = current;
        }
    }
    else
    {
        current = processes;
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
            processes = current->next;
        }
        else
        {
            previous->next = current->next;
        }
        current->next = active;
        active = current;
    }*/
    return 0;
}


context* contextSwitch(context* rsp)
{
    if(init==0){
        return rsp;
    }
    if(processAmount==0){
        return rsp;
    }
    //status_t active_st = active->process.status;

    if(processes[active].run==0){
        processes[active].run = 1;
        return processes[active].context;    
    }   

    processes[active].context=rsp;
    //sigo corriendo el mismo
    if(active != NULL && quantumsLeft>0){
        quantumsLeft--;
        return processes[active].context;
    }
    pid_t pid;
    for(int i=0;i<processAmount;i++){
        if(processes[i].status==READY && processes[active].pid!=processes[i].pid){
            active = i;
            quantumsLeft =priorities[processes[active].priority];
            return processes[active].context;
        }
    }
    

    return processes[active].context;
}

int killProcess(int returnValue, char autokill)
{
    processes[active].status = TERMINATED;
    quantumsLeft=0;
    processReadyCount--; 
    forced_schedule();
    return returnValue; 
    /*
    Node *currentProcess = active;

    pid_t blockedPid;
    while ((blockedPid = dequeuePid(currentProcess->process.blockedQueue)) != -1)
    {
        unblockProcess(blockedPid);
    }
    active = currentProcess->next;
    if (currentProcess->process.status != BLOCKED)
    {
        processReadyCount--;
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
       proccessBeingRun = 0;
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
    processes[active].quantumsLeft = 0;
    _int20h;
    return 0;
}

processInfo *getProccessesInfo()
{
    processInfo *first = NULL;
    processInfo *current = NULL;
    Queue currentNode = active;
    pid_t firstPid =  processes[active].pid;
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
    currentNode = processes;
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
    /*
    Node * current = active;
    if(current->process.pid == pid){
        active = current->next;
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

    current = processes;
    if(current->process.pid == pid){
        processes = current->next;
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
    }*/
    return 0;
    
}