#include "./include/semaphore.h"

#define MAX_SEMAPHORES 10


typedef struct {
        char * name;
        int value;
        int mutex; //mutex para evitar que dos procesos modifiquen el mismo semaforo al mismo tiempo.
}sem_t;

typedef struct semManagerCDT {
	sem_t *semaphores[MAX_SEMAPHORES];//Array con los semaforos que se hayan creado
    int8_t lastId;
} semManagerCDT;

semManagerADT semManager;

void create_sem_manager(){
    semManager->lastId=0;
    for (int i = 0; i < MAX_SEMAPHORES; i++)
		semManager->semaphores[i] = NULL;
}

int8_t sem_init(char * name, int value){
    //Los semaforos se acceden por nombre, no puede haber dos con el mismo nombre.
    //retorna -1 si no lo pudo crear, 0 si pudo.
    if(semManager->lastId>MAX_SEMAPHORES){
        return -1;
    }
    for(int i=0;semManager->semaphores[i]!=NULL;i++){
        if(strcmp(name, semManager->semaphores[i]->name)==0){
            return -1;
        }
    }

    sem_t* new_sem;
    new_sem->name = name;
    new_sem->value = value;
    
    semManager->semaphores[semManager->lastId]=new_sem;
    semManager->lastId++;
    
    return 0;
}

void sem_post(char * name){
    //Si no existe un semaforo con ese nombre retorna -1, sino 0
    for(int i=0;semManager->semaphores[i]!=NULL;i++){
        if(strcmp(name, semManager->semaphores[i]->name)==0){
            wait_mutex(i);
            semManager->semaphores[i]->value++;
            return 0;
            post_mutex(i);
        }
    }
    return -1;
}

void sem_wait(char * name){
    for(int i=0;semManager->semaphores[i]!=NULL;i++){
        if(strcmp(name, semManager->semaphores[i]->name)==0){
            while(semManager->semaphores[i]->value==0){
                //yieldProcess():
            }
            wait_mutex(i);
            semManager->semaphores[i]->value--;
            return 0;
            post_mutex(i);  
        }
    }
    return -1;
}

void wait_mutex(int id){
    while(semManager->semaphores[id]->mutex==0){
        //yieldProcess()
    }
    return;
}

void post_mutex(int id){
    semManager->semaphores[id]->mutex++;
    return;
}