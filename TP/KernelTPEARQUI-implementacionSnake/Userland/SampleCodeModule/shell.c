#include <usr_stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys_calls.h>
#include <colores.h>
#include "snake.h"
#include "shell.h"

#define MAX_BUFFER 254
#define MAX_COMMANDS 11

char line[MAX_BUFFER+1] = {0}; //asi me aseguro que al menos va haber un cero
char parameter[MAX_BUFFER+1] = {0};
char command[MAX_BUFFER+1] = {0};
int linePos = 0;
char lastc;
const char * commands[] = {"undefined","help","time","clear","snake","inforeg","zerodiv","invopcode","sizeplus","sizeminus","memoryManager"};

void showCommands(){
	prints("\n-time-               muestra la hora actual en pantalla",MAX_BUFFER);
	prints("\n-clear-              limpia la pantalla",MAX_BUFFER);
	prints("\n-snake-              inicia el juego de snake",MAX_BUFFER);
	prints("\n-inforeg-            imprime los valores de los registros",MAX_BUFFER);
	prints("\n-zerodiv-            testeo de excepcion de division por cero",MAX_BUFFER);
	prints("\n-invopcode-          testeo codigo de operacion invalido",MAX_BUFFER);
	prints("\n-sizeplus-           aumenta el tamanio de letra",MAX_BUFFER);
	prints("\n-sizeminus-          disminuye el tamanio de letra",MAX_BUFFER);
	prints("\n-memoryManager-      memoryManager",MAX_BUFFER);
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


static void (*commands_ptr[MAX_COMMANDS])() = {cmd_undefined, cmd_help, cmd_time, cmd_clear, cmd_snake, cmd_inforeg, cmd_zeroDiv,cmd_invOpcode,cmd_charsizeplus,cmd_charsizeminus, cmd_memory_manager};


void shell (){
	char c;
	prints("$ User> ",9);

	while(1){
		c = getChar();
		printLine(c);
	};
}

static void printLine(char c){
	if (linePos < MAX_BUFFER && c != lastc){
		if (isChar(c) || c == ' ' ||isDigit(c)){
			line[linePos++] = c;
			printc(c);
		} else if (c == '\b' && linePos > 0){
			printc(c);
			line[--linePos] = 0;
		} else if (c == '\n'){
			newLine();
		}
	}
	lastc = c;
}


static void newLine(){


	int i = checkLine();

	(*commands_ptr[i])();

	for (int i = 0; line[i] != '\0' ; i++){
		line[i] = 0;
		command[i] = 0;
		parameter[i] = 0;
	}
	linePos = 0;

	if (i != 3 ){
		prints("\n$ User> ",9);
	} else {
		prints("$ User> ",9);
	}
}


//separa comando de parametro
static int checkLine(){
	int i = 0;
	int j = 0;
	int k = 0;
	for ( j = 0 ; j < linePos && line[j] != ' ' ; j ++){
		command[j] = line[j];
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


static void cmd_help(){
	prints("\n---HELP---\n",MAX_BUFFER);
	showCommands();
}

static void cmd_undefined(){
	prints("\n\nNo se reconoce \"",MAX_BUFFER);
	prints(command,MAX_BUFFER);
	prints("\" como un comando valido, para ver los comandos disponibles escribir \"help\"\n",MAX_BUFFER);
}

static void cmd_time(){
	display_time();
}


static void cmd_snake(){
	if(!startSnake(charToInt(parameter))){
		prints("\ningrese un parametro valido '1' o '2' jugadores \n",MAX_BUFFER);
	}
	
}

static void cmd_clear(){
	clear_scr();
}

static void cmd_inforeg(){
	inforeg();
}

static void cmd_invOpcode(){
	test_invopcode();
}

static void cmd_zeroDiv(){
	test_zerodiv();
}

static void cmd_charsizeplus(){
	increaseScale();
}

static void cmd_charsizeminus(){
	decreaseScale();
}

static void cmd_memory_manager(){
	char memoryManager[50];
	memoryManager = memory_manager((uint64_t)memoryManager);
	prints("\nHOLA",5);
	prints(memoryManager, 50);
	prints("\n", 1);
}





