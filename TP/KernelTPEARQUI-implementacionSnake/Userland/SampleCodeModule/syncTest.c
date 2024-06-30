#include <stdint.h>
#include <usr_stdlib.h>
#include <syncTest.h>

#define SEM_ID 3
#define TOTAL_PAIR_PROCESSES 2

int64_t global; // shared memory
char * inc="-1";
void slowInc(int64_t *p, int64_t inc) {
	uint64_t aux = *p;
	yield(); // This makes the race condition highly probable
	aux += inc;
	*p = aux;
}

uint64_t my_process_inc(uint64_t argc, char *argv[]) {
	uint64_t n;
	int8_t inc;
	int8_t use_sem;

	if (argc != 3)
	return -1;

	if ((n = satoi(argv[0])) <= 0)
	return -1;
	if ((inc = satoi(argv[1])) == 0)
	return -1;
	if ((use_sem = satoi(argv[2])) < 0)
	return -1;

	if (use_sem)
	if (sem_init(SEM_ID, 1)==0) {
		prints("\n semaforo creado\n", 100);
	}

	uint64_t i;
	for (i = 0; i < n; i++) {
	if (use_sem)
		sem_wait(SEM_ID, get_pid());

	slowInc(&global, inc);

	if (use_sem)
		sem_wait(SEM_ID, get_pid());
	}

	if (use_sem)
		sem_close(SEM_ID);

	return 0;
}

static void my_process_wrapper(){

	char* argv[]={"2",inc,"1"};
	if(strcmp(inc, "-1")){
		inc = "1";
	}else{
		inc = "-1";
	}
	my_process_inc(3, argv);
}

uint64_t test_sync(uint64_t argc, char *argv[]) { //{n, use_sem, 0}

    uint64_t pids[2 * TOTAL_PAIR_PROCESSES];

    if (argc != 2)
    return -1;

    char *argvDec[] = {argv[0], "-1", argv[1], NULL};
    char *argvInc[] = {argv[0], "1", argv[1], NULL};

    global = 0;

    uint64_t i;
    for (i = 0; i < TOTAL_PAIR_PROCESSES; i++) {
      pids[i] = new_process(my_process_wrapper, 1, argvDec,3);
      pids[i + TOTAL_PAIR_PROCESSES] = new_process(my_process_wrapper, 1,argvInc,3);
    }

    for (i = 0; i < TOTAL_PAIR_PROCESSES; i++) {
      sem_wait(pids[i], get_pid());
      sem_wait(pids[i + TOTAL_PAIR_PROCESSES], get_pid());
    }
	wait(3000);

    prints("Final value: ", 100);
    printDec(global);
    printc('\n');
    return 0;
}