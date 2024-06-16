#include <scheduler.h>
#include <interrupts.h>
#include <memoryManager.h>
#include <queue.h>
#include <lib.h>
#include <pipe.h>

extern uint64_t loadProcess(uint64_t rip, uint64_t rsp, uint64_t argc, uint64_t argv); // implement on assembler
extern void _int20h();                                                                 // implement int20h con assembler
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
typedef struct scheduler{
    Queue active;
    Queue expired;

    // Schelduler states
    int processAmount;
    unsigned int processReadyCount;
    pid_t placeholderProcessPid;
    unsigned int proccessBeingRun;
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
    /*
    scheduler->active = memory_manager_malloc(sizeof(Node));
    scheduler->expired = memory_manager_malloc(sizeof(Node));;
    */
    scheduler->processAmount = 0;
    scheduler->processReadyCount = 0;
    scheduler->proccessBeingRun = 0;
    scheduler->active=NULL;
    scheduler->expired=NULL;
    
    scheduler->placeholderProcessPid = new_process((uint64_t)dummyProcess, 0, NULL);
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
        _int20h();
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

pid_t new_process(uint64_t rip, int argc, char *argv[])
{
    Node *newProcess = memory_manager_malloc(sizeof(Node));
    //newProcess->process = memory_manager_malloc(sizeof(PCB));
    newProcess->process.pid = scheduler->processAmount++;
    newProcess->process.priority = DEFAULT_PRIORITY;
    newProcess->process.quantumsLeft = priorities[DEFAULT_PRIORITY];
    newProcess->process.blockedQueue = newQueue();
    newProcess->process.newPriority = -1;
    newProcess->process.status = READY;
    newProcess->process.argc = argc;
    newProcess->process.argv = copy_argv(argc, argv);

    // STDIN, STDOUT, STDERR, PIPEOUT, PIPEIN
    if (scheduler->active != NULL)
    {
        for (int i = 0; i <= scheduler->active->process.lastFd; i++)
        {
            newProcess->process.fileDescriptors[i].mode = scheduler->active->process.fileDescriptors[i].mode;
        }
        newProcess->process.lastFd = scheduler->active->process.lastFd;
        newProcess->process.pipe = scheduler->active->process.pipe;
    }

    uint64_t rsp = (uint64_t)memory_manager_malloc(4 * 1024);
    if (rsp == 0)
    {
        return -1;
    }
    newProcess->process.stackBase = rsp;
    uint64_t newRsp = (uint64_t)loadProcess(rip, rsp + 4 * 1024, newProcess->process.argc, (uint64_t)newProcess->process.argv);
    newProcess->process.rsp = newRsp;

    if (scheduler->active == NULL)
    {
        newProcess->next = NULL;
        scheduler->active = newProcess;
    }
    else
    {
        if (scheduler->expired == NULL)
        {
            newProcess->next = NULL;
            scheduler->expired = newProcess;
        }
        else
        {
            Node *current = scheduler->expired;
            Node *previous = NULL;
            // Si el numero del que quiero insertar es mayor que el current entonces tengo que insertarlo despues
            // new_process -> 2 y current -> 1. La 1 es mejor que la 2. El menor numero gana
            while (current->next != NULL && newProcess->process.priority >= current->process.priority)
            {
                previous = current;
                current = current->next;
            }
            if (current->next == NULL && newProcess->process.priority >= current->process.priority)
            {
                newProcess->next = NULL;
                current->next = newProcess;
            }
            else
            {
                newProcess->next = current;
                if (previous != NULL)
                {
                    previous->next = newProcess;
                }
                else
                {
                    scheduler->expired = newProcess;
                }
            }
        }
    }
    scheduler->processReadyCount++;
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

uint64_t contextSwitch(uint64_t rsp)
{
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
    }

    Node *currentProcess = scheduler->active;
    currentProcess->process.rsp = rsp;

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
        _int20h();
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
    _int20h();
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

