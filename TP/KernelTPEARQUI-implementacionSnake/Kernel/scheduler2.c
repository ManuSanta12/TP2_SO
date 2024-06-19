#include <scheduler.h>
#include <interrupts.h>
#include <memoryManager.h>
#include <queue.h>
#include <lib.h>
#include <pipe.h>

#define SCHEDULER_ADDRESS 0x60000
#define EFLAGS 0x202
#define CS     0x8
#define SS          0
#define EXIT   0

#define READY 0
#define TERMINATED 1

extern void move_flag;


extern void _int20h;

int init=0;

typedef struct process{
    pid_t pid;
    context* context;
    char **argv;
    int argc;
    uint8_t status;
}process;

typedef struct node{
    struct node* next;
    process* data;
}node;

typedef struct scheduler{
    node* p_list;
    node* running;
    uint16_t p_amount;
}scheduler;

scheduler* sch;

void createScheduler(){
    sch = (scheduler*)SCHEDULER_ADDRESS;
    sch->p_amount=0;
    sch->p_list = NULL;
    init=1;
    move_flag;
}

void exit(int status){
    sch->running->data->status = TERMINATED;    
    _int20h;
}

static void start(fun function, int argc, char *argv[]) {
    int status = function(argc, argv);
    exit(status);
}

static context *get_context(fun main,
                                   int argc, char *argv[]) {
    context *context = memory_manager_malloc(sizeof(context));

    context->rdi = (uint64_t)main;
    context->rsi = (uint64_t)argc;
    context->rdx = (uint64_t)argv;
    context->rip = (uint64_t)&start;

    context->cs = CS;
    context->eflags = EFLAGS;
    context->ss = SS;

    context->rsp = (uint64_t)context;

    return context;
}

static char **copy_argv(int argc, char **argv)
{
    char **new_argv = memory_manager_malloc(sizeof(char *) * argc);
    for (int i = 0; i < argc; i++)
    {
        new_argv[i] = strcpy(argv[i]);
    }
    return new_argv;
}

void add_process(node* process){
    process->next = sch->p_list;
    sch->p_list = process;
}

pid_t new_process(fun f, int argc, char *argv[]){
    node* newProcess = memory_manager_malloc(sizeof(node));
    newProcess->data = memory_manager_malloc(sizeof(process));
    newProcess->data->argc=argc;
    newProcess->data->pid=sch->p_amount;
    newProcess->data->status=READY;
    sch->p_amount++;
    newProcess->data->context = get_context(f,argc,argv);
    newProcess->data->argv = copy_argv(argc, argv);
    add_process(newProcess);
}

context* context_switch(context* rsp){
    if(init==0){
        return;
    }
    if(sch->p_amount==0){
        return rsp;
    }
    node* current = sch->running->next;
    while(current!=NULL){
        if(current->data->status==READY){
            sch->running=current;
            return sch->running->data->context;
        }
        current=current->next;
    }
    sch->running = sch->p_list;
    return sch->running->data->context;
}   




void dummyProcess() {
    return;
}

PCB *getProcess(pid_t pid) {
    return;
}

uint64_t getCurrentPid() {
    return;
}

int blockProcess(pid_t pid) {
    return;
}

int unblockProcess(pid_t pid) {
    return;
}

void nextProcess() {
    return;
}

int prepareDummy(pid_t pid) {
    return;
}

int killProcess(int returnValue, char autokill) {
    return;
}

int changePriority(pid_t pid, int priorityValue) {
    return;
}

int yieldProcess() {
    return;
}

processInfo * getProccessesInfo() {
    return;
}

priority_t get_priority(pid_t pid) {
    return;
}

int kill_by_pid(pid_t pid) {
    return;
}



/*
void dummyProcess();
PCB *getProcess(pid_t pid);
uint64_t getCurrentPid();
int blockProcess(pid_t pid);
int unblockProcess(pid_t pid);
void nextProcess();
int prepareDummy(pid_t pid);
int killProcess(int returnValue, char autokill);
int changePriority(pid_t pid, int priorityValue);
int yieldProcess();
processInfo * getProccessesInfo();
priority_t get_priority(pid_t pid);
int kill_by_pid(pid_t pid);*/