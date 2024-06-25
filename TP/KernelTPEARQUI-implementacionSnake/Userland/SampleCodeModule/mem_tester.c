#include <usr_stdlib.h>
#include <stdio.h>
#include <sys_calls.h>
#include "shell.h"
#include "mem_tester.h"
#include <utilTest.h>

typedef struct node{
    int value;
    struct node * next;
}node;

#define MAX_BLOCKS 128

typedef struct MM_rq {
  void *address;
  uint32_t size;
} mm_rq;


uint64_t test_mm(uint64_t argc, char *argv[]) {
  prints("\nEmpezando prueba...\n", 100);
  mm_rq mm_rqs[MAX_BLOCKS];
  uint8_t rq;
  uint32_t total;
  uint64_t max_memory;

  if (argc != 1)
    return -1;

  if ((max_memory = satoi(argv[0])) <= 0)
    return -1;

  while (1) {
    rq = 0;
    total = 0;

    // Request as many blocks as we can
    while (rq < MAX_BLOCKS && total < max_memory) {
      mm_rqs[rq].size = GetUniform(max_memory - total - 1) + 1;
      mm_rqs[rq].address = mm_malloc(mm_rqs[rq].size);

      if (mm_rqs[rq].address) {
        total += mm_rqs[rq].size;
        rq++;
      }
    }

    // Set
    uint32_t i;
    for (i = 0; i < rq; i++)
      if (mm_rqs[i].address)
        memset(mm_rqs[i].address, i, mm_rqs[i].size);

    // Check
    for (i = 0; i < rq; i++)
      if (mm_rqs[i].address)
        if (!memcheck(mm_rqs[i].address, i, mm_rqs[i].size)) {
          prints("test_mm ERROR\n",100);
          return -1;
        }

    // Free
    for (i = 0; i < rq; i++){
      if (mm_rqs[i].address)
        mm_free(mm_rqs[i].address);
    }

    prints("\n Prueba realizada con exito\n",100);
    return 0;
    }
    
}


void memory_test(){
    //this test will allocate nodes of a list and then it whill free them. 
    //It will show the state of the list and the state of the memory between steps.
    prints("\nCreando lista...\n",100);
    node* new = mm_malloc(sizeof(node));
    if(new==NULL){
        prints("mm_malloc failed", 100);
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



static void test_wrapper(){
    char * argv[] = {"100000"};
    test_mm(1,argv);
}
void run_test(){
    new_process(test_wrapper, 0, NULL,0);
}