#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#include <stdint.h>
#include <stddef.h>
//incluir scheduler

typedef struct semManagerCDT *semManagerADT;

int8_t sem_init(char * name, int value);

void sem_post(char * name);

void sem_wait(char * name);

void wait_mutex(int id);

void post_mutex(int id);

#endif