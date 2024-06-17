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
	PCB *process = NULL;
	for (int lvl = LEVELS - 1; lvl >= 0 && process == NULL; lvl--)
		if (!isEmpty(scheduler->lvls[lvl]))
			process = (Node *) (getFirst(scheduler->lvls[lvl]))->data;

	if (process == NULL)
		return FIRST_PID;
	return process->pid;
}

int32_t set_priority(pid_t pid, priority_t newPriority) {
	listNode *node = scheduler->processes[pid];
	if (node == NULL || pid == FIRST_PID){
		return -1;
    }
	PCB *process = (PCB *) node->data;
	if (newPriority >= LEVELS)
		return -1;
	if (process->status == READY || process->status == RUNNING) {
		removeNode(scheduler->lvls[process->priority], node);
		scheduler->processes[process->pid] = appendNode(scheduler->lvls[newPriority], node);
	}
	process->priority = newPriority;
	return newPriority;
}

int8_t setStatus(uint16_t pid, uint8_t newStatus) {
	listNode *node = scheduler->processes[pid];
	if (node == NULL || pid == FIRST_PID)
		return -1;
	PCB *process = (PCB *) node->data;
	status_t oldStatus = process->status;
	if (newStatus == RUNNING || newStatus == ZOMBIE || oldStatus == ZOMBIE)
		return -1;
	if (newStatus == process->status)
		return newStatus;

	process->status = newStatus;
	if (newStatus == BLOCKED) {
		removeNode(scheduler->lvls[process->priority], node);
		appendNode(scheduler->lvls[LEVELS], node); //los procesos bloqueados estan en el ultimo nivel
	}
	else if (oldStatus == BLOCKED) {
		removeNode(scheduler->lvls[LEVELS], node);
		process->priority = MAX_PRIORITY;
		prependNode(scheduler->lvls[process->priority], node);
		scheduler->remainingQuantum = 0;
	}
	return newStatus;
}

status_t get_status(pid_t pid){
    listNode* node = scheduler->processes[pid];
    if(node == NULL){
        return DEAD;
    }
    return ((PCB *) node->data)->status;

}

void *schedule(void *prevStackPointer) {
	static int firstTime = 1;

	scheduler->remainingQuantum--;
	if (!scheduler->processAmount || scheduler->remainingQuantum > 0)
		return prevStackPointer;

	PCB *currentProcess;
	listNode *node = scheduler->processes[scheduler->currentPid];

	if (node != NULL) {
		currentProcess = (PCB *) node->data;
		if (!firstTime)
			currentProcess->rsp = prevStackPointer;
		else
			firstTime = 0;
		if (currentProcess->status == RUNNING)
			currentProcess->status = READY;

		uint8_t newPriority = currentProcess->priority > 0 ? currentProcess->priority - 1 : currentProcess->priority;
		setPriority(currentProcess->pid, newPriority);
	}

	scheduler->currentPid = getNextPid(scheduler);
	currentProcess = scheduler->processes[scheduler->currentPid]->data;

	if (scheduler->killFg && currentProcess->fileDescriptors[STDIN] == STDIN) {
		scheduler->killFg = 0;
		if (killCurrentProcess(-1) != -1)
			forceTimerTick();
	}
	scheduler->remainingQuantum = (MAX_PRIORITY - currentProcess->priority);
	currentProcess->status = RUNNING;
	return currentProcess->rsp;
}

int16_t createProcess(main_foo code, char **args, char *name, uint8_t priority, int16_t fileDescriptors[], uint8_t unkillable) {
	if (scheduler->processAmount >= MAX_PROC) // TODO: Agregar panic?
		return -1;
	PCB *process = (PCB *) memory_manager_malloc(sizeof(PCB));
	initProcess(process, scheduler->nextPid, scheduler->currentPid, code, args, name, priority, fileDescriptors, unkillable);

	listNode *processNode;
	if (process->pid != FIRST_PID)
		processNode = appendElement(scheduler->lvls[process->priority], (void *) process);
	else {
		processNode = allocMemory(sizeof(Node));
		processNode->data = (void *) process;
	}
	scheduler->processes[process->pid] = processNode;

	while (scheduler->processes[scheduler->nextPid] != NULL)
		scheduler->nextPid = (scheduler->nextPid + 1) % MAX_PROC;
	scheduler->processAmount++;
	return process->pid;
}

static void destroyZombie(PCB *zombie) {
	listNode *zombieNode = scheduler->processes[zombie->pid];
	scheduler->processAmount--;
	scheduler->processes[zombie->pid] = NULL;
	freeProcess(zombie);
	free(zombieNode);
}

int32_t killCurrentProcess(int32_t retValue) {
	return killProcess(scheduler->currentPid, retValue);
}

int32_t killProcess(uint16_t pid, int32_t retValue) {
	listNode *processToKillNode = scheduler->processes[pid];
	if (processToKillNode == NULL)
		return -1;
	PCB *processToKill = (PCB *) processToKillNode->data;
	if (processToKill->status == ZOMBIE || processToKill->canBeKilled)
		return -1;

	closeFileDescriptors(processToKill);

	uint8_t priorityIndex = processToKill->status != BLOCKED ? processToKill->priority : BLOCKED_INDEX;
	removeNode(scheduler->lvls[priorityIndex], processToKillNode);
	processToKill->retVal = retValue;

	processToKill->status = ZOMBIE;

	begin(processToKill->zombie);
	while (hasNext(processToKill->zombie)) {
		destroyZombie(scheduler, (PCB *) next(processToKill->zombie));
	}

	listNode *parentNode = scheduler->processes[processToKill->parentPID];
	if (parentNode != NULL && ((PCB *) parentNode->data)->status != ZOMBIE) {
		PCB *parent = (PCB *) parentNode->data;
		appendNode(parent->zombie, processToKillNode);
		if (processIsWaiting(parent, processToKill->pid))
			setStatus(processToKill->parentPID, READY);
	}
	else {
		destroyZombie(processToKill);
	}
	if (pid == scheduler->currentPid)
		yield();
	return 0;
}

processInfo * getProccessesInfo(){
    processInfo * info = memory_manager_malloc(sizeof(processInfo));
	processInfo *ps = memory_manager_malloc(scheduler->processAmount * sizeof(processInfo));
	int processIndex = 0;

	loadSnapshot(&ps[processIndex++], (PCB *) scheduler->processes[FIRST_PID]->data);
	for (int lvl = LEVELS; lvl >= 0; lvl--) { // Se cuentan tambien los bloqueados
		begin(scheduler->lvls[lvl]);
		while (hasNext(scheduler->lvls[lvl])) {
			PCB *nextProcess = (PCB *) next(scheduler->lvls[lvl]);
			loadSnapshot(&ps[processIndex], nextProcess);
			processIndex++;
			if (nextProcess->status != ZOMBIE) {
				getZombiesSnapshots(processIndex, ps, nextProcess);
				processIndex += getLength(nextProcess->zombie);
			}
		}
	}
	info->length = scheduler->processAmount;
	info->snapshotList = ps;
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



