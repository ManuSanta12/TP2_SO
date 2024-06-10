#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#include <stdint.h>
#include <stddef.h>
#include "memoryManager.h"
#include "lib.h"
//incluir scheduler

typedef struct semManagerCDT *semManagerADT;

uint8_t sem_init(char *name, int value);

uint8_t sem_post(char *name);

uint8_t sem_wait(char *name, int pid);

void wait_mutex(int id);

void post_mutex(int id);

uint8_t sem_close(char*name);
#endif // _SEMAPHORE_H_
