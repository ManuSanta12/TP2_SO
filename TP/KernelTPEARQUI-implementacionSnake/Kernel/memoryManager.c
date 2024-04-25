#include "memoryManager.h"

char *my_memory;
char *current;

void * my_malloc(){
    if (current < my_memory + MEM_SIZE){
        return current++;
    }
    else{
        return NULL;
    }
}

void mm_init(){
    my_memory = (char *)0xA00000;
    current = my_memory;
}

void * memory_manager(char * buffer){
    mm_init();
    char *ptr = (char *) my_malloc();
    if (!ptr) *ptr = 0; //revisar aca que imprima mensaje de error
    *ptr = '5';
    buffer[0] = *ptr;
    return buffer;
}