#include <usr_stdlib.h>
#include <sinc.h>
#include "shell.h"
int a = 0;

/*
void p1(){
    while(1){
        a++;
        sem_post("my_sem");
        sem_wait("my_sem", get_pid());
    }
}

void p2(){
    while(1){
        sem_wait("my_sem", get_pid());
        a--;
        sem_post("my_sem");
    }
}*/
void p1(){
    while(1);
}
void run_sinc(){
    /*
    sem_init("my_sem",0);
    new_process((uint64_t)p1,0,NULL);
    new_process((uint64_t)p2,0,NULL);
    int i = 0;
    while(i<10){
        sleep(1);
        printDec(a);
        i++;
    }*/
    new_process(dummy, 0, NULL);
    //new_process((uint64_t)p1, 0, NULL);
}
