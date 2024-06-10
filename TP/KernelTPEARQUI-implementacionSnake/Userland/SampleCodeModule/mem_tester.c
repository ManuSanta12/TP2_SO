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
    prints("\nCreando lista...\n",100);
    node* new = mm_malloc(sizeof(node));
    if(new==NULL){
        prints("malloc failed", 100);
    }
    new->value=10;
    node* new2 = mm_malloc(sizeof(node));
    new2->value=20;
    node* new3 = mm_malloc(sizeof(node));
    new3->value=30;
    print_meminfo();
    printc('\n');

    new->next=new2;
    new2->next=new3;
    new3->next=NULL;

    node*it=new;
    prints("\nLista:\n",100);
    printc('\n');
    while(it!=NULL){
        printDec(it->value);
        if(it->next!=NULL)
            prints("->",10);
        it=it->next;
    }
    printc('\n');
    prints("\nLiberando lista...\n",100);

    mm_free(new);
    mm_free(new2);
    mm_free(new3);
    
    print_meminfo();
    printc('\n');
    prints("\nAlocando 100000000\n",100000000);
    void* ptr = mm_malloc(100000000);
    if(ptr==NULL){
        prints("\nNo se pudo alocar\n",100);
    }
    printc('\n');
    print_meminfo();
}