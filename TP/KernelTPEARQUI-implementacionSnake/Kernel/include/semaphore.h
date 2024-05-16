#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

    typedef struct sem_t{
        char * name;
        int value;
    }sem_t;

    sem_t sem_init(char * name, int value){
        sem_t new_sem;
        new_sem.name = name;
        new_sem.value = value;
        return new_sem;
    }

    void sem_post(sem_t sem){
        sem.value++;
    }

    void sem_wait(sem_t sem){
        while(sem.value == 0);
        sem.value--;
    }

#endif