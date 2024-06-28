#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_



typedef newSem_t *sem_ptr;


void init_sem_list();
sem_ptr sem_open(char *, int value);
int sem_wait(sem_ptr semaphore);
int sem_post(sem_ptr semaphore);
int sem_close(sem_ptr semaphore);
int get_semaphores(copy_sem_t *sems[]);

#endif