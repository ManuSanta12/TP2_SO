#include <scheduler.h>
#include <interrupts.h>
#include <memoryManager.h>
#include <queue.h>
#include <lib.h>
#include <pipe.h>

extern void _int20h; // implement int20h con assembler
extern void forced_schedule(void);

// tck and ppriorities
#define SHELL_PID 0
#define STACK_SIZE 4096
#define MAX_PROCESSES 100
#define MIN_PRIORITY 1
#define MAX_PRIORITY 9
#define EOF -1
#define NUMBER_OF_PRIORITIES 9
#define DEFAULT_PRIORITY 4
priority_t priorities[NUMBER_OF_PRIORITIES] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

uint8_t init = 0;

uint32_t active;
PCB processes[MAX_PROCESSES];

// Schelduler states
int processAmount;
pid_t placeholderProcessPid;
uint16_t quantumsLeft;

// typedef schedulerCDT* schedulerADT;

void dummyProcess()
{
    while (1)
    {
        _hlt();
    }
}

void createScheduler()
{
    // scheduler = (schedulerADT)SCHEDULER_ADDRESS;
    processAmount = 0;
    quantumsLeft = 0;
    active = -1;


    // process1es={};

    // placeholderProcessPid = new_process((uint64_t)dummyProcess, 0, NULL);
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

priority_t get_priority(pid_t pid)
{
    for (int i = 0; i < processAmount && i < MAX_PROCESSES; i++)
    {
        if (processes[i].pid == pid)
        {
            return processes[i].priority;
        }
    }
    return 0;
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

static void start(fun f, int argc, char *argv[])
{
    int status = f(argc, argv);
    pid_t pid;
    processes[active].status = TERMINATED;
    quantumsLeft = 0;
    forced_schedule();
}

static context *new_context(fun foo, int argc, char **argv)
{
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

pid_t new_process(fun foo, int bg, char *argv[], int argc)
{
    PCB newProcess;
    newProcess.pid = processAmount++;
    newProcess.priority = DEFAULT_PRIORITY;
    newProcess.quantumsLeft = priorities[DEFAULT_PRIORITY];
    newProcess.blockedQueue = newQueue();
    newProcess.newPriority = -1;
    newProcess.status = READY;
    newProcess.argc = argc;
    newProcess.argv = copy_argv(argc, argv);


    for (int i = 0; i <= 2; i++)
    {
        newProcess.fileDescriptors[i].mode = OPEN;
    }
    // PIPEOUT, PIPEIN
    for (int i = 3; i <= 4; i++)
    {
        newProcess.fileDescriptors[i].mode = CLOSED;
    }


    if (bg)
    {
        // BG process
        newProcess.priority = 1;
    }
    newProcess.context = new_context(foo, argc, argv);

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

    if (active == -1)
    {
        active = 0;
    }
    processes[processAmount - 1] = newProcess;

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

context *contextSwitch(context *rsp)
{
    if (init == 0)
    {
        return rsp;
    }
    if (processAmount == 0)
    {
        return rsp;
    }
    // status_t active_st = active->process.status;

    if (processes[active].run == 0)
    {
        processes[active].run = 1;
        return processes[active].context;
    }

    processes[active].context = rsp;
    // sigo corriendo el mismo
    if (quantumsLeft > 0)
    {
        quantumsLeft--;
        return processes[active].context;
    }
    pid_t pid;
    int k = 0;
    for (int i = active; k < processAmount; i++)
    {
        if (i >= processAmount)
        {
            i = 0;
        }
        if (processes[i].status == READY && processes[active].pid != processes[i].pid)
        {
            active = i;
            quantumsLeft = priorities[processes[active].priority];
            return processes[active].context;
        }
        k++;
    }

    return processes[active].context;
}

int killProcess(int returnValue, char autokill)
{
    if (processes[active].pid == SHELL_PID)
    {
        return -1;
    }
    processes[active].status = TERMINATED;
    quantumsLeft = 0;
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

int changePriority(pid_t pid, int priorityValue)
{
    if (priorityValue < 0 || priorityValue > 8)
    {
        return -1;
    }
    for (int i = 0; i < processAmount && i < MAX_PRIORITY; i++)
    {
        if (processes[i].pid == pid)
        {
            processes[i].priority = priorityValue;
            return 0;
        }
    }
    return -1;
}

int yieldProcess()
{
    processes[active].quantumsLeft = 0;
    _int20h;
    return 0;
}

processInfo *getProcessesInfo()
{
    processInfo *first = NULL;
    processInfo *current = NULL;
    pid_t firstPid = processes[active].pid;

    processInfo *previous = NULL;

    for (int i = 0; i < processAmount && i < MAX_PROCESSES; i++)
    {
        if (processes[i].status != TERMINATED)
        {
            // Crear un nuevo nodo para la lista
            current = (processInfo *)memory_manager_malloc(sizeof(processInfo));
            if (current == NULL)
            {
                return NULL;
            }

            current->pid = processes[i].pid;
            current->priority = processes[i].priority;
            current->status = processes[i].status;
            current->next = NULL;

            if (first == NULL)
            {
                first = current;
            }
            else
            {
                previous->next = current;
            }

            previous = current;
        }
    }
    return first;
}

int kill_by_pid(pid_t pid)
{
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