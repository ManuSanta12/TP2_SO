/* sampleCodeModule.c */
#include <sys_calls.h>
#include <stdint.h>
#include <usr_stdlib.h>
#include <shell.h>


char * v = (char*)0xB8000 + 79 * 2;


int main() {
	
	prints("a continuacion se detallan las rutinas disponibles para ejecutar:\n",MAX_BUFFER);
	
	showCommands();

	//new_process((uint64_t)shell, 0, NULL);
	shell();


	return 0;
} 