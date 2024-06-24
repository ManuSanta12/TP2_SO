#include <usr_stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys_calls.h>
#include <colores.h>
#include "snake.h"
#include "shell.h"
#include "mem_tester.h"
#include "phylos.h"
#include "sinc.h"

#define MAX_BUFFER 254
#define MAX_COMMANDS 19

char line[MAX_BUFFER + 1] = {0}; // asi me aseguro que al menos va haber un cero
char parameter[MAX_BUFFER + 1] = {0};
char command[MAX_BUFFER + 1] = {0};
int linePos = 0;
char lastc;
const char *commands[] = {"undefined", "help", "time", "clear", "snake", "inforeg", "zerodiv", "invopcode", "sizeplus", "sizeminus", "mem", "memtest", "phylos", "loop", "cat", "filter", "wc", "ps", "nice"};

void showCommands()
{
	prints("\n-time-               muestra la hora actual en pantalla", MAX_BUFFER);
	prints("\n-clear-              limpia la pantalla", MAX_BUFFER);
	prints("\n-snake-              inicia el juego de snake", MAX_BUFFER);
	prints("\n-inforeg-            imprime los valores de los registros", MAX_BUFFER);
	prints("\n-zerodiv-            testeo de excepcion de division por cero", MAX_BUFFER);
	prints("\n-invopcode-          testeo codigo de operacion invalido", MAX_BUFFER);
	prints("\n-sizeplus-           aumenta el tamanio de letra", MAX_BUFFER);
	prints("\n-sizeminus-          disminuye el tamanio de letra", MAX_BUFFER);
	prints("\n-mem-                muestra la informacion de la memory manager actual", MAX_BUFFER);
	prints("\n-memtest-            corre un programa de testeo para el memory manager", MAX_BUFFER);
	prints("\n-phylos-             corre una simulacion al problema de filosofos", MAX_BUFFER);
	prints("\n-loop-               imprime pid cada 2 segundos", MAX_BUFFER);
	prints("\n-cat-                imprime el STDIN tal como lo recibe", MAX_BUFFER);
	prints("\n-wc-                 cuenta la cantidad de lineas del input", MAX_BUFFER);
	prints("\n-filter-             filtra las vocales del input", MAX_BUFFER);
	prints("\n-ps-                 muestra en pantalla la inforacion de proceso actual", MAX_BUFFER);
	prints("\n-nice-               aumenta la prioridad del proceso deseado  ", MAX_BUFFER);
	printc('\n');
}

static void newLine();
static void printLine(char c);
static int checkLine();
static void cmd_undefined();
static void cmd_help();
static void cmd_time();
static void cmd_clear();
static void cmd_snake();
static void cmd_inforeg();
static void cmd_zeroDiv();
static void cmd_invOpcode();
static void cmd_charsizeplus();
static void cmd_charsizeminus();
static void cmd_memory_manager();
static void cmd_memory_tester();
static void cmd_phylos();
static void cmd_loop();
static void cmd_cat();
static void cmd_wc();
static void cmd_filter();
static void cmd_ps();
static void cmd_nice();
static void runCommandInBackground(void *cmd);
void pipeManager();

static void (*commands_ptr[MAX_COMMANDS])() = {cmd_undefined, cmd_help, cmd_time, cmd_clear, cmd_snake, cmd_inforeg, cmd_zeroDiv, cmd_invOpcode, cmd_charsizeplus, cmd_charsizeminus, cmd_memory_manager, cmd_memory_tester, cmd_phylos, cmd_loop, cmd_cat, cmd_filter, cmd_wc, cmd_ps, cmd_nice};
int runInBackground = 0;
int pipeDetected = 0;
int cmd1Index = 0;
int cmd2Index = 0;
int fds[2];

void shell()
{
	char c;
	prints("$ User> ", 9);

	while (1)
	{
		c = getChar();
		printLine(c);
	};
}

static void runCommandInBackground(void *cmd)
{
	pid_t pid = sys_exec(cmd, 0, NULL);
}

static void printLine(char c)
{
	if (linePos < MAX_BUFFER && c != lastc)
	{
		if (isChar(c) || c == ' ' || isDigit(c))
		{
			line[linePos++] = c;
			printc(c);
		}
		else if (c == '\b' && linePos > 0)
		{
			printc(c);
			line[--linePos] = 0;
		}
		else if (c == '\n')
		{
			newLine();
		}
	}
	lastc = c;
}

static void newLine()
{

	int i = checkLine();

	if (pipeDetected)
	{
		pipeManager(cmd1Index, cmd2Index);
		prints("\n$ User> ", 9);
		return;
	}
	else
	{

		(*commands_ptr[i])();

		for (int i = 0; line[i] != '\0'; i++)
		{
			line[i] = 0;
			command[i] = 0;
			parameter[i] = 0;
		}
		linePos = 0;

		if (i != 3)
		{
			prints("\n$ User> ", 9);
		}
		else
		{
			prints("$ User> ", 9);
		}
	}
}

void pipeManager()
{
	// prints("Comando 1: ", MAX_BUFFER);
	// for (int i = 0; i < cmd1Index; i++)
	// {
	// 	prints(command[i], MAX_BUFFER);
	// }
	// prints("\nComando 2: ", MAX_BUFFER);
	// for (int i = cmd1Index; i < cmd2Index; i++)
	// {
	// 	prints(command[i], MAX_BUFFER);
	// }


	// sys_pipe(fds);
    // sys_close(fds[1]);
    // sys_dup2(fds[0], STDOUT);
    // sys_close(STDOUT);
	// pid_t pidW = sys_exec((uint64_t)*commands_ptr[cmd1Index], 0, NULL);
	// sys_close(fds[0]);
    // sys_dup2(fds[1], STDIN);
    // sys_close(STDIN);
    // sys_open(STDOUT);
    // pid_t pidR = sys_exec((uint64_t)*commands_ptr[cmd2Index], 0, NULL);

    // sys_waitpid(pidW);
    // sys_waitpid(pidR);

    // sys_open(STDIN);
    // sys_close(fds[0]);
    // sys_close(fds[1]);
}

// separa comando de parametro
static int checkLine() {
  	int i = 0;
	int j = 0;
	int k = 0;
	runInBackground = 0; // Reset background flag
    if (linePos >= 2 && line[0] == 'B') {
		prints("\nCorriendo en background\n",30);
        runInBackground = 1; 
    }

	for (; j < linePos && line[j] != ' ' ; j ++){
		command[j] = line[j];
	}
	int m=0;
	if(runInBackground){
		while (command[m] != '\0') {
        	command[m] = command[m + 1];
        	m++;
    }
	}

	if (j < linePos){
		j++;
		while (j < linePos){
			parameter[k++] = line[j++];
		}
	}




	for (i = 1 ; i < MAX_COMMANDS ; i++ ){
		if (strcmp(command,commands[i]) == 0){
			return i;
		}
	}

	return 0;

}



static void cmd_help()
{
	prints("\n---HELP---\n", MAX_BUFFER);
	showCommands();
}

static void cmd_undefined()
{
	prints("\n\nNo se reconoce \"", MAX_BUFFER);
	prints(command, MAX_BUFFER);
	prints("\" como un comando valido, para ver los comandos disponibles escribir \"help\"\n", MAX_BUFFER);
}

static void cmd_time()
{
	display_time();
}

static void cmd_snake()
{
	if (!startSnake(charToInt(parameter)))
	{
		prints("\ningrese un parametro valido '1' o '2' jugadores \n", MAX_BUFFER);
	}
}

static void cmd_clear()
{
	clear_scr();
}

static void cmd_inforeg()
{
	inforeg();
}

static void cmd_invOpcode()
{
	test_invopcode();
}

static void cmd_zeroDiv()
{
	test_zerodiv();
}

static void cmd_charsizeplus()
{
	increaseScale();
}

static void cmd_charsizeminus()
{
	decreaseScale();
}

static void cmd_memory_manager()
{
	print_meminfo();
}

static void cmd_memory_tester()
{
	run_test();
}

static void cmd_phylos()
{
	run_phylos();
}

static void cmd_loop()
{
	run_loop(runInBackground);
}

static void cmd_wc()
{
	run_wc();
}
static void cmd_cat()
{
	run_sinc();
}
static void cmd_filter()
{
	run_filter();
}
static void cmd_ps()
{
	getProcessesInfo();
}

static void cmd_nice()
{
	if (strlen(parameter) == 0)
	{
		prints("\nIngresar el pid del proceso por parametro\n", MAX_BUFFER);
		return;
	}
	pid_t pid = charToInt(parameter);
	int ret = up_priority(pid);
	if (ret == -1)
	{
		prints("\nNo se pudo actualizar la prioridad\n", MAX_BUFFER);
	}
	else
	{
		prints("\nPrioridad aumentada!\n", MAX_BUFFER);
	}
}