#include <usr_stdlib.h>
#include <sinc.h>
#include "shell.h"
int a = 0;


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
}


void run_sinc(){
    sem_init("my_sem",0);
    new_process(p1,0,NULL,0);
    new_process(p2,0,NULL,0);
    int i = 0;
    wait(4000);
    printDec(a);
}
