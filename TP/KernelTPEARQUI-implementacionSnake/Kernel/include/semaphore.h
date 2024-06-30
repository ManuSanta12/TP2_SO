#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#include <stdint.h>
#include <stddef.h>
#include "memoryManager.h"
#include "lib.h"
//incluir scheduler

typedef struct semManagerCDT *semManagerADT;

uint8_t create_sem_manager();

int8_t sem_init(int id, int value);

int8_t sem_post(int id, int pid);

int8_t sem_wait(int id, int pid);

void wait_mutex(int id);

void post_mutex(int id);

int8_t sem_close(int id);
int8_t sem_open(uint16_t id);

#endif // _SEMAPHORE_H_
