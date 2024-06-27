#include <scheduler.h>
#include <interrupts.h>
#include <memoryManager.h>
#include <queue.h>
#include <lib.h>
#include <pipe.h>

extern void _int20h;// implement int20h con assembler
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

uint8_t init=0;

uint32_t active;
PCB processes[MAX_PROCESSES];

// Schelduler states
int processAmount;
pid_t placeholderProcessPid;
uint16_t quantumsLeft;



void createScheduler()
{
    processAmount = 0;
    quantumsLeft = 0;
    active=-1;
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
    for(int i = 0; i<processAmount && i<MAX_PROCESSES;i++){
        if(processes[i].pid==pid){
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
    for(int i=0; i<processAmount && i<MAX_PROCESSES;i++){
        if(processes[i].pid==pid && processes[i].status!=TERMINATED){
            processes[i].status = BLOCKED;
            return 0;
        }
    }
    return -1;
}

int unblockProcess(pid_t pid)
{
   for(int i=0; i<processAmount && i<MAX_PROCESSES;i++){
        if(processes[i].pid==pid){
            processes[i].status = READY;
            return 0;
        }
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
    processes[active].status = TERMINATED;
    quantumsLeft=0;
    forced_schedule();
}

static context* new_context(PCB* proc, fun foo, int argc, char**argv){
    context *con =  (context *)((uint64_t)proc + STACK_SIZE - sizeof(context));

    con->rdi = (uint64_t)foo;
    con->rsi = (uint64_t)argc;
    con->rdx = (uint64_t)argv;  
    con->rip = (uint64_t)&start;

    con->cs = CS;
    con->eflags = EFLAGS;
    con->ss = SS;

    con->rsp = (uint64_t)con;

    return con;
}

pid_t new_process(fun foo, int bg, char*argv[],int argc)
{
    PCB newProcess;
    newProcess.pid = processAmount++;
    newProcess.priority = DEFAULT_PRIORITY;
    //newProcess.blockedQueue = newQueue();
    newProcess.status = READY;
    newProcess.argc = argc;
    newProcess.argv = copy_argv(argc, argv);
    
    if(bg){
        newProcess.priority=1;
    }
    newProcess.context = new_context(&newProcess,foo,argc,argv);

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
    if(processAmount>MAX_PROCESSES){
        for(int i = 1; i<MAX_PROCESSES; i++){
            if(processes[i].status == TERMINATED){
                processes[i] = newProcess;
                return newProcess.pid;
            }
        }
    } else {
        processes[processAmount-1]=newProcess;
    }
    return newProcess.pid;
}



context* contextSwitch(context* rsp)
{
    if(init==0){
        return rsp;
    }
    if(processAmount==0){
        return rsp;
    }

    if(processes[active].run==0){
        processes[active].run = 1;
        return processes[active].context;    
    }   

    processes[active].context=rsp;

    //sigo corriendo el mismo
    if(quantumsLeft>0 && processes[active].status==READY){
        quantumsLeft--;
        return processes[active].context;
    }
    pid_t pid;
    int k=0;
    for(int i=active+1;k<processAmount;i++){
        if(i>=MAX_PROCESSES || i>=processAmount){
            i=0;
        }
        if(processes[i].status==READY){
            active = i;
            quantumsLeft =priorities[processes[active].priority];
            return processes[active].context;
        }
        k++;
    }
    

    return processes[active].context;
}

int killProcess(int returnValue, char autokill)
{
    if(processes[active].pid == SHELL_PID){
        return -1;
    }
    processes[active].status = TERMINATED;
    free_memory_manager(processes[active].context);
    free_memory_manager(processes[active].argv);
    quantumsLeft=0;
    return returnValue; 
    
}

int changePriority(pid_t pid, int priorityValue){
    if (priorityValue < 0 || priorityValue > 8)
    {
        return -1;
    }
    for(int i = 0; i<processAmount && i<MAX_PRIORITY; i++){
        if(processes[i].pid == pid && processes[i].status!=TERMINATED){
            processes[i].priority = priorityValue;
            return 0;
        }
    }
    return -1;
}

int yieldProcess()
{
    quantumsLeft = 0;
    forced_schedule();
    return 0;
}

processInfo *getProcessesInfo()
{
    processInfo *first = NULL;
    processInfo *current = NULL;
    pid_t firstPid =  processes[active].pid;
    
    processInfo *previous = NULL;

    for(int i = 0; i < processAmount && i < MAX_PROCESSES; i++) {
        if(processes[i].status != TERMINATED) {
            // Crear un nuevo nodo para la lista
            current = (processInfo *)memory_manager_malloc(sizeof(processInfo));
            if(current == NULL) {
                return NULL;
            }

            current->pid = processes[i].pid;
            current->priority = processes[i].priority;
            current->status = processes[i].status;
            current->next = NULL;

            if(first == NULL) {
                first = current;
            } else {
                previous->next = current;
            }

            previous = current;
        }
    }
    return first;
}

int kill_by_pid(pid_t pid){
    if(pid==SHELL_PID){
        return -1;
    }
    for(int i =0; i<processAmount && i<MAX_PROCESSES;i++){
        if(processes[i].pid == pid){
            processes[i].status=TERMINATED;
            free_memory_manager(processes[i].context);
            free_memory_manager(processes[active].argv);
        }
    }
    return 0;
    
}