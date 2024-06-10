#include <phylos.h>
#include <stdint.h>
#include <stdio.h>
#include <usr_stdlib.h>
#include <sys_calls.h>
int run_phylos(){
    prints("dummy",10);
}
/*
#define MAX_QTY 32
#define MIN_QTY 3
#define MAX_PHYLO_NUMBER 3
#define MAX_BUFFER 254
#define MUTEX_SEM_NAME "phylos"

#define DEV_NULL -1
#define STDIN 0
#define STDOUT 1
#define STDERR 2

#define EAT 3
#define THINK 2

#define QUIT 'q'
#define ADD 'a'
#define REMOVE 'r'
#define CLEAR 'c'

static char *phylos_names[] = {
	"Godio", "Gleiser", "Aquili", "Mogni",
    "Garberoglio","Noni", "Meola", "Valles", "Fernandez",
	"Oviedo", "Poggi", "Deiana", "Vega",
	"Arias", "Orecchia", "Bonucci",
	"Boutet", "Marti", "Turrin", "Buquete",
	"Bagini", "Peña", "Soliani", "De Santis",
	"Ramos", "Scaglioni", "Gomez", "Staudenmann",
	"Peñaloza", "Sosa", "Vilas", "Dolagaratz"};

typedef enum { NONE = 0, EATING, HUNGRY, THINKING } PHYLO_STATE;

uint8_t phylos_qty = 0;
uint8_t phylos_line = 0;
PHYLO_STATE phylos_states[MAX_QTY];
int16_t phylos_pids[MAX_QTY];

int run_phylos() {

    printc('\n');
    prints("creando semaforo...",100);
    printc('\n');
    
    if (sem_init(MUTEX_SEM_NAME, 1) == -1){
        prints("retorne -1",100);
        
        return -1;
    } 
    printc('\n');
    prints("semaforo creado!",100);
    printc('\n');

    for (int i = 0; i < MAX_QTY; i++) {
       
        phylos_states[i] = NONE;
        phylos_pids[i] = -1;
    }

    for (int i = 0; i < MIN_QTY; i++){
        add_phylo(i);
    }
  

    char command = '\0';
    while ((command = getChar())!= QUIT) {
        //printc('\n');
       // prints("Me metí al while",100);
        switch (command) {
            case REMOVE:
                if (phylos_qty > MIN_QTY) {
                    remove_phylo(phylos_qty - 1);
                } else {
                    prints("\nDebe haber 3 filosofos para empezar\n", MAX_BUFFER);
                }
                break;
            case ADD:
                printc('\n');
                prints("Agregando filosofo...",100);
                printc('\n');
                if (phylos_qty < MAX_QTY) {
                    if (add_phylo(phylos_qty) == -1)
                        prints("\nNo se pudo agregar un filosofo\n", MAX_BUFFER);
                } else {
                    prints("\nLa mesa esta llena\n", MAX_BUFFER);
                }
                break;
            case CLEAR:
                phylos_line = !phylos_line;
                break;
        }
    }

    for (int i = phylos_qty - 1; i >= 0; i--) {
        remove_phylo(i);
    }
    sem_close(MUTEX_SEM_NAME);
    return 0;
}

 void show_phylos() {
    
    if (phylos_line) {
        clear_scr();
    }
    const static char letters[] = {' ', 'E', '.', '.'};
    uint8_t present_phylos = 0;

    for (int i = 0; i < phylos_qty; i++) {
        if (letters[phylos_states[i]] != ' ') {
            present_phylos = 1;
            printc(letters[phylos_states[i]]);
        }
    }
    if (present_phylos) {
        printc('\n');
    }
}

 uint8_t add_phylo(int index) {
    sem_wait(MUTEX_SEM_NAME, get_pid());
    char philo_number_buffer[MAX_PHYLO_NUMBER] = {0};
    if (sem_init(phylo_sem(index), 0) == -1)
        return -1;
    itoa(index, philo_number_buffer, 10);
    char *params[] = {"philosopher", philo_number_buffer, NULL};
    int16_t file_descriptors[] = {DEV_NULL, STDOUT, STDERR};
    phylos_pids[index] = new_process(&phylo, 3,params);
    if (phylos_pids[index] != -1)
        prints("hola",11);
        phylos_qty++;
    show_phylos();
    sem_post(MUTEX_SEM_NAME);
    return -1 * !(phylos_pids[index] + 1);
}

 uint8_t remove_phylo(int index) {
    sem_wait(MUTEX_SEM_NAME, get_pid());
    while (phylos_states[left(index)] == EATING && phylos_states[right(index)] == EATING) { 
        sem_post(MUTEX_SEM_NAME);
        sem_wait(phylo_sem(index),get_pid());
        sem_wait(MUTEX_SEM_NAME,get_pid());
    }
    // killProcess(phylos_pids[index]);
    // waitpid(phylos_pids[index]);
    printc('\n');
    sem_close(phylo_sem(index));
    phylos_pids[index] = -1;
    phylos_states[index] = NONE;
    phylos_qty--;
    show_phylos();
    sem_post(MUTEX_SEM_NAME);
    return 0;
}

 int phylo(int argc, char **argv) {
    int i = atoi(argv[1]);
    phylos_states[i] = THINKING;
    while (1) {
        sleep(THINK);
        take_fork(i);
        sleep(EAT);
        put_fork(i);
    }
    return 0;
}

 void take_fork(int i) {
    sem_wait(MUTEX_SEM_NAME, get_pid());
    phylos_states[i] = HUNGRY;
    test(i);
    sem_post(MUTEX_SEM_NAME);
    sem_wait(phylo_sem(i), get_pid());
}

 void put_fork(int i) {
    sem_wait(MUTEX_SEM_NAME,get_pid());
    phylos_states[i] = THINKING;
    show_phylos();
    test(left(i));
    test(right(i));
    sem_post(MUTEX_SEM_NAME);
}

 void test(int i) {
    if (phylos_states[i] == HUNGRY && phylos_states[left(i)] != EATING && phylos_states[right(i)] != EATING) {
        phylos_states[i] = EATING;
        sem_post(phylo_sem(i));
        show_phylos();
    }
}

 char * phylo_sem(int i){
    return phylos_names[i];
}
 int left(int i){
    return (i + phylos_qty - 1) % phylos_qty;
}
 int right(int i){
    return (i + 1) % phylos_qty;
}*/