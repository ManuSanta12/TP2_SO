#include <scheduler.h>
#include <interrupts.h>
#include <memoryManager.h>
#include <queue.h>
#include <lib.h>
#include <pipe.h>
#include <linkedList.h>

extern void* load_process(void*, void* , void* , void* ); // implement on assembler
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
#define FIRST_PID 0
#define NUMBER_OF_PRIORITIES 9
#define DEFAULT_PRIORITY 4
priority_t priorities[NUMBER_OF_PRIORITIES] = {9, 8, 7, 6, 5, 4, 3, 2, 1};

uint8_t init=0;
typedef struct scheduler{
    listNode *processes[MAX_PROC];
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

void dummy_process()
{
    while (1)
    {
        _hlt();
    }
}

void create_sch()
{
    scheduler = (schedulerADT)SCHEDULER_ADDRESS;
	for (int i = 0; i < MAX_PROC; i++)
		scheduler->processes[i] = NULL;
	for (int i = 0; i < LEVELS + 1; i++)
		scheduler->lvls[i] = create_linked_list();
	scheduler->nextPid = 0;
	scheduler->killFg = 0;
    
}

static uint16_t get_next_pid() {
	PCB *process = NULL;
	for (int lvl = LEVELS - 1; lvl >= 0 && process == NULL; lvl--)
		if (!is_empty(scheduler->lvls[lvl]))
			process = (listNode *) (get_first(scheduler->lvls[lvl]))->data;

	if (process == NULL)
		return FIRST_PID;
	return process->pid;
}

int32_t set_priority(pid_t pid, priority_t newPriority) {
	listNode *listNode = scheduler->processes[pid];
	if (listNode == NULL || pid == FIRST_PID){
		return -1;
    }
	PCB *process = (PCB *) listNode->data;
	if (newPriority >= LEVELS)
		return -1;
	if (process->status == READY || process->status == RUNNING) {
		remove_node(scheduler->lvls[process->priority], listNode);
		scheduler->processes[process->pid] = append_node(scheduler->lvls[newPriority], listNode);
	}
	process->priority = newPriority;
	return newPriority;
}

int8_t set_status(uint16_t pid, uint8_t newStatus) {
	listNode *listNode = scheduler->processes[pid];
	if (listNode == NULL || pid == FIRST_PID)
		return -1;
	PCB *process = (PCB *) listNode->data;
	status_t oldStatus = process->status;
	if (newStatus == RUNNING || newStatus == ZOMBIE || oldStatus == ZOMBIE)
		return -1;
	if (newStatus == process->status)
		return newStatus;

	process->status = newStatus;
	if (newStatus == BLOCKED) {
		remove_node(scheduler->lvls[process->priority], listNode);
		append_node(scheduler->lvls[LEVELS], listNode); //los procesos bloqueados estan en el ultimo nivel
	}
	else if (oldStatus == BLOCKED) {
		remove_node(scheduler->lvls[LEVELS], listNode);
		process->priority = MAX_PRIORITY;
		prepend_node(scheduler->lvls[process->priority], listNode);
		scheduler->remainingQuantum = 0;
	}
	return newStatus;
}

status_t get_status(pid_t pid){
    listNode* listNode = scheduler->processes[pid];
    if(listNode == NULL){
        return DEAD;
    }
    return ((PCB *) listNode->data)->status;

}

void *schedule(void *prevStackPointer) {
	static int firstTime = 1;

	scheduler->remainingQuantum--;
	if (!scheduler->processAmount || scheduler->remainingQuantum > 0)
		return prevStackPointer;

	PCB *currentProcess;
	listNode *listNode = scheduler->processes[scheduler->currentPid];

	if (listNode != NULL) {
		currentProcess = (PCB *) listNode->data;
		if (!firstTime)
			currentProcess->rsp = prevStackPointer;
		else
			firstTime = 0;
		if (currentProcess->status == RUNNING)
			currentProcess->status = READY;

		uint8_t newPriority = currentProcess->priority > 0 ? currentProcess->priority - 1 : currentProcess->priority;
		set_priority(currentProcess->pid, newPriority);
	}

	scheduler->currentPid = get_next_pid(scheduler);
	currentProcess = scheduler->processes[scheduler->currentPid]->data;

	if (scheduler->killFg && currentProcess->fileDescriptors[STDIN] == STDIN) {
		scheduler->killFg = 0;
		if (kill_current_process(-1) != -1)
			force_tick();
	}
	scheduler->remainingQuantum = (MAX_PRIORITY - currentProcess->priority);
	currentProcess->status = RUNNING;
	return currentProcess->rsp;
}

int16_t create_process(main_foo code, char **args, char *name, uint8_t priority, int16_t fileDescriptors[], uint8_t unkillable) {
	if (scheduler->processAmount >= MAX_PROC)
		return -1;
	PCB *process = (PCB *) memory_manager_malloc(sizeof(PCB));
	new_process(scheduler->nextPid, scheduler->currentPid, code, args, name, priority, fileDescriptors, unkillable);

	listNode *processNode;
	if (process->pid != FIRST_PID)
		processNode = append_element(scheduler->lvls[process->priority], (void *) process);
	else {
		processNode = memory_manager_malloc(sizeof(listNode));
		processNode->data = (void *) process;
	}
	scheduler->processes[process->pid] = processNode;

	while (scheduler->processes[scheduler->nextPid] != NULL)
		scheduler->nextPid = (scheduler->nextPid + 1) % MAX_PROC;
	scheduler->processAmount++;
	return process->pid;
}

static void destroy_zombie(PCB *zombie) {
	listNode *zombieNode = scheduler->processes[zombie->pid];
	scheduler->processAmount--;
	scheduler->processes[zombie->pid] = NULL;
	free_process(zombie);
	free(zombieNode);
}

int32_t kill_current_process(int32_t retValue) {
	return kill_process(scheduler->currentPid, retValue);
}

int32_t kill_process(uint16_t pid, int32_t retValue) {
	listNode *processToKillNode = scheduler->processes[pid];
	if (processToKillNode == NULL)
		return -1;
	PCB *processToKill = (PCB *) processToKillNode->data;
	if (processToKill->status == ZOMBIE || processToKill->canBeKilled)
		return -1;

	close_FDs(processToKill);

	uint8_t priorityIndex = processToKill->status != BLOCKED ? processToKill->priority : LEVELS;
	remove_node(scheduler->lvls[priorityIndex], processToKillNode);
	processToKill->retVal = retValue;

	processToKill->status = ZOMBIE;

	begin(processToKill->zombie);
	while (has_next(processToKill->zombie)) {
		destroy_zombie((PCB *) next(processToKill->zombie));
	}

	listNode *parentNode = scheduler->processes[processToKill->parentPID];
	if (parentNode != NULL && ((PCB *) parentNode->data)->status != ZOMBIE) {
		PCB *parent = (PCB *) parentNode->data;
		append_node(parent->zombie, processToKillNode);
		if (is_process_waiting(parent, processToKill->pid))
			set_status(processToKill->parentPID, READY);
	}
	else {
		destroy_zombie(processToKill);
	}
	if (pid == scheduler->currentPid)
		yield();
	return 0;
}

processInfo * get_processes_info(){
    processInfoList * info = memory_manager_malloc(sizeof(processInfoList));
	processInfo *ps = memory_manager_malloc(scheduler->processAmount * sizeof(processInfo));
	int processIndex = 0;

	save_info(&ps[processIndex++],(PCB *) scheduler->processes[FIRST_PID]->data);
	for (int lvl = LEVELS; lvl >= 0; lvl--) { 
		begin(scheduler->lvls[lvl]);
		while (has_next(scheduler->lvls[lvl])) {
			PCB *nextProcess = (PCB *) next(scheduler->lvls[lvl]);
			save_info(&ps[processIndex], nextProcess);
			processIndex++;
			if (nextProcess->status != ZOMBIE) {
				get_zombie_info(processIndex, ps, nextProcess);
				processIndex += get_length(nextProcess->zombie);
			}
		}
	}
	info->lenght = scheduler->processAmount;
	info->processList = ps;
	return info;
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

void process_wrapper(main_foo fun, char **args) {
	int len = strlen(args);
	int retValue = fun(len, args);
	kill_current_process(retValue);
}

static int argv_len(char **argv) {
	int len = 0;
	while (*(argv++) != NULL)
		len++;
	return len;
}

listNode* new_process(pid_t pid, pid_t parentPID, main_foo mainFun, char **argv, char *name, priority_t priority, fd_t fileDescriptors[], uint8_t kill){  
    PCB* newProcess = memory_manager_malloc(sizeof(listNode*));
    newProcess->pid = scheduler->processAmount++;
	newProcess->parentPID = parentPID;
	newProcess->stackBase = memory_manager_malloc(STACK_SIZE);
    int argc = argv_len(argv);
	newProcess->argv = copy_argv(argc, argv);
	newProcess->name = memory_manager_malloc(strlen(name) + 1);
	newProcess->name = strcpy(name);
	newProcess->priority = DEFAULT_PRIORITY;
	void *stackEnd = (void *) ((uint64_t) newProcess->stackBase + STACK_SIZE);
	newProcess->rsp = load_process(&process_wrapper, mainFun, stackEnd, (void *) newProcess->argv);
	newProcess->status = READY;
	newProcess->zombie = newQueue();
	newProcess->canBeKilled = kill;
	newProcess->waiting = 0;

	assign_FD(newProcess, STDIN, newProcess->fileDescriptors[STDIN], READ);
	assign_FD(newProcess, STDOUT, newProcess->fileDescriptors[STDOUT], WRITE);
	assign_FD(newProcess, STDERR, newProcess->fileDescriptors[STDERR], WRITE);
}

static void assign_FD(PCB *new, uint8_t fdIndex, fd_t fdValue, uint8_t mode) {
	new->fileDescriptors[fdIndex] = fdValue;
	if (fdValue >= FDS)
		pipeOpenForPid(new->pid, fdValue, mode);
}

void close_all_fd(PCB *p) {
	close_fds(p->pid, p->fileDescriptors[STDIN]);
	close_fds(p->pid, p->fileDescriptors[STDOUT]);
	close_fds(p->pid, p->fileDescriptors[STDERR]);
}

void free_process(PCB *p) {
	freeQueue(p->zombie);
	free_memory_manager(p->stackBase);
	free_memory_manager(p->name);
	free_memory_manager(p->argv);
	free_memory_manager(p);
}

processInfo *save_info(processInfo* info, PCB *p) {
	info->name = strcpy(p->name);
	info->pid = p->pid;
	info->parentPID = p->parentPID;
	info->stackBase = p->stackBase;
	info->rsp = p->rsp;
	info->priority = p->priority;
	info->status = p->status;
	info->foreground = p->fileDescriptors[STDIN] == STDIN;
	return info;
}

int is_process_waiting(PCB *p, uint16_t pidToWait) {
	return p->waiting == pidToWait && p->status == BLOCKED;
}

int get_zombie_info(int processIndex, processInfo psArray[], PCB *nextProcess) {
	LinkedList zombieChildren = nextProcess->zombie;
	begin(zombieChildren);
	while (has_next(zombieChildren))
		save_info(&psArray[processIndex++], (PCB *) next(zombieChildren));
	return processIndex;
}

int get_zombie_value(uint16_t pid) {
	listNode *zombieNode = scheduler->processes[pid];
	if (zombieNode == NULL)
		return -1;
	PCB *zombieProcess = (PCB *) zombieNode->data;
	if (zombieProcess->parentPID != scheduler->currentPid)
		return -1;

	PCB *parent = (PCB *) scheduler->processes[scheduler->currentPid]->data;
	parent->waiting = pid;
	if (zombieProcess->status != ZOMBIE) {
		set_status(parent->pid, BLOCKED);
		yield();
	}
	remove_node(parent->zombie, zombieNode);
	destroy_zombie(zombieProcess);
	return zombieProcess->retVal;
}

int is_process_alive(uint16_t pid) {
	listNode *processNode = scheduler->processes[pid];
	return processNode != NULL && ((PCB *) processNode->data)->status != ZOMBIE;
}

void yield() {
	scheduler->remainingQuantum = 0;
	force_tick();
}

int change_file_descriptor(uint16_t pid, uint8_t position, int16_t newFd) {
	listNode *processNode = scheduler->processes[pid];
	if (pid == FIRST_PID || processNode == NULL)
		return -1;
	PCB *process = (PCB *) processNode->data;
	process->fileDescriptors[position] = newFd;
	return 0;
}

int get_current_process_FD(uint8_t fdIndex) {
	PCB *process = scheduler->processes[scheduler->currentPid]->data;
	return process->fileDescriptors[fdIndex];
}

void kill_foreground_process() {
	scheduler->killFg = 1;
}