#include <usr_stdlib.h>
#include <stdio.h>
#include <sys_calls.h>
#include "shell.h"
#include "mem_tester.h"


typedef struct node{
    int value;
    struct node * next;
}node;

void run_test(){
    //this test will allocate nodes of a list and then it whill free them. 
    //It will show the state of the list and the state of the memory between steps.
    node* new = mm_malloc(sizeof(node));
    if(new==NULL){
        prints("malloc failed", 100);
    }
    new->value=10;
    node* new2 = mm_malloc(sizeof(node));
    new2->value=20;
    node* new3 = mm_malloc(sizeof(node));
    new3->value=30;

    new->next=new2;
    new2->next=new3;
    new3->next=NULL;

    node*it=new;
    printc('\n');
    while(it!=NULL){
        printDec(it->value);
        prints("->",10);
        it=it->next;
    }
    printc('\n');
    
    print_meminfo();
    printc('\n');
    printc('\n');

    prints("Liberando nodos de la lista...",100);
    printc('\n');
    printc('\n');

    mm_free(new);
    mm_free(new2);
    mm_free(new3);
    
    printc('\n');
    print_meminfo();
    printc('\n');

    printc('\n');
    //print_meminfo();
    prints("Alocando 100",100);
    printc('\n');
    void* ptr = mm_malloc(100);
    print_meminfo();


}