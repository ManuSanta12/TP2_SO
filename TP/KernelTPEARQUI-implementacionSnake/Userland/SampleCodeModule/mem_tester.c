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
    new->value=20;
    node* new3 = mm_malloc(sizeof(node));
    new->value=30;

    new->next=new2;
    new2->next=new3;
    new3->next=NULL;

    node*it=new;
    while(it!=NULL){
        printc('\n');
        printDec(it->value);
        printc('\n');
        it=it->next;
    }

}