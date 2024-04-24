
typedef struct Memory {
    uint8_t exponent; //exponente para tamaño de los bytes
    uint8_t used;
    struct Memory *prev;
    struct Memory *next;
}
#define MM_SIZE 1024
//[- - - - - -- -]
//   ^current

//char my_memory[MM_SIZE];
char *my_memory;
char *current;

void * my_malloc(){
    if (current < my_memory + MM_SIZE){
        return current++;
    }
    else{
        return NULL;
    }
}
void mm_init(){
    current = my_memory();
    my_memory = 0xA00000;
}
int main(){
    mm_init();
    char *ptr = (char *) my_malloc();
    if (!ptr) exit();
    *ptr = 5;
    printf("%c\n", *ptr);
}