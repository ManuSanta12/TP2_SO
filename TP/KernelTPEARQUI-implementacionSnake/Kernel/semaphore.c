#include "./include/semaphore.h"
#include <scheduler.h>
#include <linkedList.h>
#define MAX_SEMAPHORES 10
#define SEM_MANAGER_ADDRESS 0x70000

typedef struct {
    int value;
    int mutex; // mutex para evitar que dos procesos modifiquen el mismo semaforo al mismo tiempo.
    LinkedListADT semaphoreQueue;
    LinkedListADT mutexQueue;
} semaphore_t;

typedef struct semManagerCDT {
    semaphore_t *semaphores[MAX_SEMAPHORES]; // Array con los semaforos que se hayan creado
    // int8_t lastId;
} semManagerCDT;

semManagerADT semManager;

uint8_t create_sem_manager() {
    semManager = (semManagerADT) SEM_MANAGER_ADDRESS;
    // semManager->lastId = 0;
    for (int i = 0; i < MAX_SEMAPHORES; i++)
        semManager->semaphores[i] = NULL;
    return 1;
}

static semaphore_t* create_sem(int initialValue){
    semaphore_t *sem = (semaphore_t *) memory_manager_malloc(sizeof(semaphore_t));
    sem->value = initialValue;
	sem->mutex = 0;
	sem->semaphoreQueue = create_list();
	sem->mutexQueue = create_list();
	return sem;
}

int8_t sem_init(int id, int value) {
	if (semManager->semaphores[id] != NULL)
		return -1;
	semManager->semaphores[id] = create_sem(value);
	return 0;
}


static void free_sem(semaphore_t *sem) {
	free_linked_list(sem->semaphoreQueue);
	free_linked_list(sem->mutexQueue);
	free_memory_manager(sem);
}

static void acquire_mutex(semaphore_t *sem, int pid) {
	while (_xchg(&(sem->mutex), 1)) {
		append(sem->mutexQueue, (void *) ((uint64_t) pid));
        blockProcess(pid);
		yieldProcess();
	}
}

static void first_avail_process(LinkedListADT queue) {
	ListNode *current;
	while ((current = get_first(queue)) != NULL) {
		remove_node(queue, current);
		uint16_t pid = (uint16_t) ((uint64_t) current->data);
		free_memory_manager(current);
		if (is_alive(pid)) {
            unblockProcess(pid);
			break;
		}
	}
}

static void release_mutex(semaphore_t *sem) {
	first_avail_process(sem->mutexQueue);
	sem->mutex = 0;
}

static int up(semaphore_t *sem, int pid) {
	acquire_mutex(sem, pid);
	sem->value++;
	if (sem->value == 0) {
		release_mutex(sem);
		return -1;
	}
	first_avail_process(sem->semaphoreQueue);
	release_mutex(sem);
	// yield();
	return 0;
}

static int down(semaphore_t *sem, int pid) {
	acquire_mutex(sem, pid);
	while (sem->value == 0) {
		append(sem->semaphoreQueue, (void *) ((uint64_t) pid));
        blockProcess(pid);
		release_mutex(sem);
		yieldProcess();

		acquire_mutex(sem, pid);
	}
	sem->value--;
	release_mutex(sem);

	return 0;
}

int8_t sem_post(int id, int pid) {
	if (semManager->semaphores[id] == NULL)
		return -1;
	return up(semManager->semaphores[id], pid);
}

int8_t sem_wait(int id, int pid) {
	if (semManager->semaphores[id] == NULL)
		return -1;
	return down(semManager->semaphores[id], pid);
}

int8_t sem_close(int id){
	if (semManager->semaphores[id] == NULL)
		return -1;

	free_sem(semManager->semaphores[id]);
	semManager->semaphores[id] = NULL;
	return 0;
}

int8_t sem_open(uint16_t id) {
	return -1 * (semManager->semaphores[id] == NULL);
}