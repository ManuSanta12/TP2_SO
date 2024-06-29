#include "./include/semaphore.h"
#include <scheduler.h>
#include <linkedList.h>
#define MAX_SEMAPHORES 10
#define SEM_MANAGER_ADDRESS 0x70000

typedef struct pidNode {
    int pid;
    struct pidNode * next;
} pidNode;

typedef struct {
    // char * name;
    int value;
    int mutex; // mutex para evitar que dos procesos modifiquen el mismo semaforo al mismo tiempo.
    // pidNode * first;
    // pidNode * last;
    LinkedListADT semaphoreQueue;
    LinkedListADT mutexQueue;
} semaphore_t;

typedef struct semManagerCDT {
    semaphore_t *semaphores[MAX_SEMAPHORES]; // Array con los semaforos que se hayan creado
    // int8_t lastId;
} semManagerCDT;

semManagerADT semManager;

// void queue_pid(semaphore_t *sem, int pid) {
//     pidNode *new = memory_manager_malloc(sizeof(pidNode));
//     new->pid = pid;
//     new->next = NULL;

//     if (sem->first == NULL) {
//         sem->first = new;
//         sem->last = new;
//     } else {
//         sem->last->next = new;
//         sem->last = new;
//     }
// }

// int peek_pid(semaphore_t *sem) {
//     if (sem->first == NULL) {
//         return -1; 
//     }
//     return sem->first->pid;
// }

// int dequeue_pid(semaphore_t *sem) {
//     if (sem->first == NULL) {
//         return -1; // No había ningun pid en la cola
//     }
//     pidNode *aux = sem->first;
//     int pidToReturn = sem->first->pid;
//     sem->first = sem->first->next;
//     if (sem->first == NULL) {
//         sem->last = NULL;
//     }
//     free_memory_manager(aux);
//     return pidToReturn;
// }

void wait_mutex(int id) {
    while (semManager->semaphores[id]->mutex == 0) {
        yieldProcess();
    }
    semManager->semaphores[id]->mutex--;
}

void post_mutex(int id) {
    semManager->semaphores[id]->mutex++;
    return;
}


uint8_t create_sem_manager() {
    semManager = (semManagerADT) SEM_MANAGER_ADDRESS;
    // semManager->lastId = 0;
    for (int i = 0; i < MAX_SEMAPHORES; i++)
        semManager->semaphores[i] = NULL;
    return 1;
}

static semaphore_t* createSem(int initialValue){
    semaphore_t *sem = (semaphore_t *) memory_manager_malloc(sizeof(semaphore_t));
    sem->value = initialValue;
	sem->mutex = 0;
	sem->semaphoreQueue = createLinkedListADT();
	sem->mutexQueue = createLinkedListADT();
	return sem;
}

uint8_t sem_init(int id, int value) {
    // Los semaforos se acceden por nombre, no puede haber dos con el mismo nombre.
    // retorna -1 si no lo pudo crear, 0 si pudo.
    /*if(semManager->lastId == 0){
        return 2;
    }else{
        return 1;
    }*/
    //     if (semManager->lastId >= MAX_SEMAPHORES) {
    //     return -1;
    // }
    // for (int i = 0; i < semManager->lastId; i++) {
    //     if (strcmp(name, semManager->semaphores[i]->name) == 0) {
    //         return -1;
    //     }
    // }

    // semaphore_t *new_sem = memory_manager_malloc(sizeof(semaphore_t));
    // if (new_sem == NULL) {
    //     return -1;
    // }
    // new_sem->name = strcpy(name);
    // new_sem->value = value;
    // new_sem->mutex = 1;
    // new_sem->first = NULL;
    // new_sem->last = NULL;
    // semManager->semaphores[semManager->lastId] = new_sem;
    // semManager->lastId++;
	if (semManager->semaphores[id] != NULL)
		return -1;
	semManager->semaphores[id] = createSem(value);
	return 0;
}


static void freeSemaphore(semaphore_t *sem) {
	freeLinkedListADTDeep(sem->semaphoreQueue);
	freeLinkedListADTDeep(sem->mutexQueue);
	free_memory_manager(sem);
}

static void acquireMutex(semaphore_t *sem, int pid) {
	while (_xchg(&(sem->mutex), 1)) {
		appendElement(sem->mutexQueue, (void *) ((uint64_t) pid));
		// setStatus(pid, BLOCKED);
		yieldProcess();
	}
}

static void resumeFirstAvailableProcess(LinkedListADT queue) {
	ListNode *current;
	while ((current = getFirst(queue)) != NULL) {
		removeNode(queue, current);
		uint16_t pid = (uint16_t) ((uint64_t) current->data);
		free_memory_manager(current);
		// if (processIsAlive(pid)) {
			// setStatus(pid, READY);
		// 	break;
		// }
	}
}

static void releaseMutex(semaphore_t *sem) {
	resumeFirstAvailableProcess(sem->mutexQueue);
	sem->mutex = 0;
}

static int up(semaphore_t *sem, int pid) {
	acquireMutex(sem, pid);
	sem->value++;
	if (sem->value == 0) {
		releaseMutex(sem);
		return -1;
	}
	resumeFirstAvailableProcess(sem->semaphoreQueue);
	releaseMutex(sem);
	// yield();
	return 0;
}

static int down(semaphore_t *sem, int pid) {
	acquireMutex(sem, pid);
	while (sem->value == 0) {
		// uint16_t pid = get_pid();
		appendElement(sem->semaphoreQueue, (void *) ((uint64_t) pid));
		// setStatus(pid, BLOCKED);
		releaseMutex(sem);
		yieldProcess();

		acquireMutex(sem, pid);
	}
	sem->value--;
	releaseMutex(sem);

	return 0;
}

uint8_t sem_post(int id, int pid) {
    // Si no existe un semaforo con ese nombre retorna -1, sino 0
    //i < semManager->lastId; ??
    // for (int i = 0; semManager->semaphores[i] != NULL; i++) {
    //     if (strcmp(name, semManager->semaphores[i]->name) == 0) {
    //         wait_mutex(i);
    //         semManager->semaphores[i]->value++;
    //         post_mutex(i);
    //         return 0;
    //     }
    // }
    // return -1;
	if (semManager->semaphores[id] == NULL)
		return -1;
	return up(semManager->semaphores[id], pid);
}

uint8_t sem_wait(int id, int pid) {
    /* semaphore_t *s;
    // for (int i = 0; (s = semManager->semaphores[i]) != NULL; i++) {
    //     if (strcmp(name, s->name) == 0) {
        
    //         if (s->value == 0) {
    //             queue_pid(s, pid);
    //         } else {
    //             wait_mutex(i);
    //             s->value--;
    //             post_mutex(i);
    //             return 0;
    //         }
    //         while (s->value == 0 || peek_pid(s) != pid) {
    //             yieldProcess();
    //         }
    //         wait_mutex(i);
    //         s->value--;
    //         dequeue_pid(s);
    //         post_mutex(i);
    //         return 0;
    //     }
    // }
    return -1;
    */ 
	if (semManager->semaphores[id] == NULL)
		return -1;
	return down(semManager->semaphores[id], pid);
}

uint8_t sem_close(int id){
    //  for (int i = 0; semManager->semaphores[i] != NULL; i++) {
    //     if (strcmp(name, semManager->semaphores[i]->name) == 0) {
    //         free_memory_manager(semManager->semaphores[i]->name);
    //         free_memory_manager(semManager->semaphores[i]);
    //         int j;
    //         for (j = i; semManager->semaphores[j + 1] != NULL; j++) {
    //             semManager->semaphores[j] = semManager->semaphores[j + 1];
    //         }
    //         semManager->semaphores[j] = NULL;
    //         semManager->lastId--;
    //         return 0;
    //     }
    // }
	if (semManager->semaphores[id] == NULL)
		return -1;

	freeSemaphore(semManager->semaphores[id]);
	semManager->semaphores[id] = NULL;
	return 0;
}