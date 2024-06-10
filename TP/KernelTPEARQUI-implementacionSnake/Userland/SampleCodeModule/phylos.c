#include <phylos.h>
#include <stdint.h>
#include <stdio.h>
#include <usr_stdlib.h>
#include <sys_calls.h>

#define MAX_QTY 32
#define MIN_QTY 3
#define MAX_PHYLO_NUMBER 3
#define MAX_BUFFER 254
#define MUTEX_SEM_NAME "phylos"

#define EAT 3
#define THINK 2

#define QUIT 'q'
#define ADD 'a'
#define REMOVE 'r'
#define CLEAR 'c'

static const char *phylos_names[] = {
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
uint8_t line = 0;
PHYLO_STATE phylos_states[MAX_QTY];
int16_t phylos_pids[MAX_QTY];

int run_phylos() {
    if (sem_init(MUTEX_SEM_NAME, 1) == -1){
        return -1;
    } 
    for (int i = 0; i < MAX_QTY; i++) {
        phylos_states[i] = NONE;
        phylos_pids[i] = -1;
    }

    for (int i = 0; i < MIN_QTY; i++){
        add_phylo(i);
    }

    char command = '\0';
    while ((command = getchar()) != QUIT) {
        switch (command) {
            case REMOVE:
                if (phylos_qty > MIN_QTY) {
                    remove_phylo(phylos_qty - 1);
                } else {
                    prints("\nDebe haber 3 filosofos para empezar\n", MAX_BUFFER);
                }
                break;
            case ADD:
                if (phylos_qty < MAX_QTY) {
                    if (add_phylo(phylos_qty) == -1)
                        prints("\nNo se pudo agregar un filosofo\n", MAX_BUFFER);
                } else {
                    prints("\nLa mesa esta llena\n", MAX_BUFFER);
                }
                break;
            case CLEAR:
                line = !line;
                break;
        }
    }

    for (int i = phylos_qty - 1; i >= 0; i--) {
        remove_phylo(i);
    }
    // sem_close(MUTEX_SEM_NAME);
    return 0;
}

 void show_phylos() {
    if (line) {
        clear_scr();
    }
    const static char letters[] = {' ', 'E', '.', '.'};
    uuint8_t present_phylos = 0;
    for (int i = 0; i < phylos_qty; i++) {
        if (letters[phylos_states[i]] != ' ') {
            present_phylos = 1;
            prints(letters[phylos_states[i]]);
        }
    }
    if (present_phylos) {
        printc('\n');
    }
}

 uint8_t add_phylo(int index) {
    sem_wait(MUTEX_SEM_NAME);
    char philo_number_buffer[MAX_PHYLO_NUMBER] = {0};
    if (semInit(phylo_sem(index), 0) == -1)
        return -1;
    itoa(index, philoNumberBuffer, 10);
    char *params[] = {"philosopher", philo_number_buffer, NULL};
    int16_t file_descriptors[] = {DEV_NULL, STDOUT, STDERR};
    // phylos_pids[index] = createProcessWithFds(&phylo, params, "philosopher", 4, file_descriptors);
    if (phylos_pids[index] != -1)
        phylos_qty++;
    show_phylos();
    sem_post(MUTEX_SEM_NAME);
    return -1 * !(phylos_pids[index] + 1);
}

 uint8_t remove_phylo(int index) {
    sem_wait(MUTEX_SEM_NAME);
    while (phylos_states[left(index)] == EATING && phylos_states[right(index)] == EATING) { 
        sem_post(MUTEX_SEM_NAME);
        sem_wait(phylo_sem(index));
        sem_wait(MUTEX_SEM_NAME);
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
    sem_wait(MUTEX_SEM_NAME);
    phylos_states[i] = HUNGRY;
    test(i);
    sem_post(MUTEX_SEM_NAME);
    sem_wait(phylo_sem(i));
}

 void put_fork(int i) {
    sem_wait(MUTEX_SEM_NAME);
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

 char* phylo_sem(int i){
    return phylos_names[i];
}
 int left(int i){
    return (i + phylos_qty - 1) % phylos_qty;
}
 int right(int i){
    return (i + 1) % phylos_qty;
}