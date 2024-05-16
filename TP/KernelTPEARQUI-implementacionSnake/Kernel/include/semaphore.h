#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

    typedef struct sem_t{
        char * name;
        int value;
    }sem_t;

    sem_t sem_init(char * name, int value);

    void sem_post(sem_t sem);

    void sem_wait(sem_t sem);

#endif