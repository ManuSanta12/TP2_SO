#include <sys_calls.h>
#include <stdio.h>
#include <exc_test.h>
#include "./include/usr_stdlib.h"
#include <uniqueTypes.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define EOF (-1)

int r=10;

static char buffer[64] = { '0' };

int scr_height;
int scr_width;



static uint32_t uintToBase(uint64_t value, char * buffer, uint32_t base);

void printc (char c){
	sys_write(STDOUT, c);
}

void prints (const char * str, int lenght){
	for (int i = 0 ; i < lenght && str[i] != 0 ; i++){
		printc(str[i]);
	}
}

char getChar(){
	char c;
	sys_read(0,&c);
	return c;
}


int isChar (char c){
	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')){
		return 1;
	}
	return 0;
}

int isDigit(char c){
		if (c >= '0' && c <= '9'){
		return 1;
	}
	return 0;
}

int strlen(const char *str){
	int i = 0;
	while (str[i] != 0){
		i++;
	}

	return i;
}

int strcmp(const char *str1, const char *str2){
	int i = 0;
	int ret = 0;

	while (str1[i] != 0 && str2[i] != 0){
		if (str1[i] - str2[i] != 0){
			return str1[i] - str2[i];
		}
		i++;
	}

	if (str1[i] != 0 || str2[i] != 0){
		return str1[i] - str2[i];
	}

	return ret;
}

uint64_t charToInt(char* str){
	uint64_t ret = 0;

	if (!isDigit(str[0])){
		ret += str[0] - '0';
		return -1;
	}

	for (int i = 0 ; str[i] != 0 ; i++ ){
		if (!isDigit(str[i])){
			return -1;
		}
		ret *= 10;
		ret += str[i] - '0';
	}
	return ret;
}

void printBase(uint64_t value, uint32_t base){
    uintToBase(value, buffer, base);
    for (int i = 0 ; buffer[i] != '\0' ; i++ ){
        printc(buffer[i]);
    }
}

void printDec(uint64_t value){
    printBase(value, (uint32_t) 10);
}

void printHex(uint64_t value){
    printBase(value, (uint32_t) 16);
}

void printBin(uint64_t value){
    printBase(value, (uint32_t) 2);
}




static uint32_t uintToBase(uint64_t value, char * buffer, uint32_t base){
    char *p = buffer;
    char *p1, *p2;
    uint32_t digits = 0;

    //Calculate characters for each digit
    do{
        uint32_t remainder = value % base;
        *p++ = (remainder < 10) ? remainder + '0' : remainder + 'A' - 10;
        digits++;
    }while (value /= base);

    // Terminate string in buffer.
    *p = 0;

    //Reverse string in buffer.
    p1 = buffer;
    p2 = p - 1;
    while (p1 < p2){
        char tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
    }
    return digits;
}


void clear_scr(){
	sys_clear();
}

int get_scrWidht(){
	return scr_width;
}

int get_scrHeight(){
	return scr_height;
}

static void set_screSize(){
	scr_width = sys_scrWidth();
	scr_height = sys_scrHeight();
}

void fill_rect(int x, int y, int x2, int y2, Color color){
	set_screSize();
	if ( (x >= 0 && x+x2 < scr_width) && ((y >= 0 && y+y2 < scr_height)) ){
		sys_fillRect  (x, y, x2, y2, color);
	}
}

void wait(uint64_t ms){
	sys_wait(ms);
}


static const char* register_names[17] = {
    "RIP", "RAX", "RBX", "RCX", "RDX", "RSI", "RDI", "RBP", "RSP", "R8 ", "R9 ", "R10", "R11", "R12", "R13", "R14", "R15"
};


static void uint64ToHex(uint64_t n, char buf[16]) {
    int i=15;
    do {
        int digit = n % 16;
        buf[i] = (digit < 10 ? '0' : ('A' - 10)) + digit;
        n /= 16;
    } while(i-- != 0);
}

void inforeg(){
	char hexbuf[19];
    hexbuf[0] = '0';
    hexbuf[1] = 'x';
    hexbuf[18] = '\0';

	uint64_t registers[17];
	int i = sys_inforeg(registers);

	printc('\n');	
	if (i == 1){
		for (int i = 0; i < 17; i++) {
        	prints(register_names[i],100);
        	prints(": ",100);
        	uint64ToHex(registers[i], hexbuf+2);
        	prints(hexbuf,100);
        	if (i % 4 == 3)
            printc('\n');
        else
            prints("   ",100);
        }
    } else {
		prints("\nTodavia no hay un snapshot de los registros, presione SHIFT + S para sacar una foto\n",100);
	}
}



void test_invopcode(){
	ex_invopcode();
}

void test_zerodiv(){
	ex_zerodiv();
}


void increaseScale(){
	sys_pixelPlus();
}

void decreaseScale(){
	sys_pixelMinus();
}

void startBeep(uint32_t frequnce){
	sys_playSound(1500);
}

void stopBeep(){
	sys_mute();
}

void print_meminfo(){
	MemoryInfo* mem = sys_memInfo();
	printc('\n');
	prints("Using: ",100);
	prints(mem->memoryAlgorithmName,100);
	printc('\n');
	prints("Memory size: ", 100);
	printDec(mem->totalMemory);
	printc('\n');
	prints("free memory: ", 100);
	printDec(mem->freeMemory);
	printc('\n');
	prints("Ocupied memory: ", 100);
	printDec(mem->occupiedMemory);
	sys_memFree((void *)mem);
}

void * mm_malloc(uint64_t size){
	return sys_memMalloc(size);
}

void mm_free(void * ap){
	sys_memFree((void *)ap);
}

uint8_t sem_init(char*name,int value){
	return sys_semInit(name,value);
}

uint8_t sem_post(char*name){
	return sys_semPost(name);
}

uint8_t sem_wait(char*name, int pid){
	return sys_semWait(name, pid);
}

uint8_t sem_close(char*name){
	return sys_semClose(name);
}

int new_process(uint64_t rip, int argc, char *argv[]){
	return sys_newProcess(rip, argc, argv);
}



static void dummy(){
	//prints("\nsoy el dummy\n",MAX_BUFFER);
	r=5;
	return;
}


void getProcessesInfo()
{
    processInfo *current = NULL;
	//new_process((uint64_t)run_loop, 0, NULL);
	new_process((uint64_t)dummy, 0, NULL);

	current = sys_ps();
	//printDec(current->pid);
    while (current != NULL)
    {
		printc('\n');
		prints("PID: ", 100);
		printDec(current->pid);
		printc('\n');
		prints("Priority: ",100);
		printDec(current->priority);
		printc('\n');
		prints("Stack Base: 0x",100);
		printDec(current->stackBase);
		printc('\n');
		prints("Status: ",MAX_BUFFER);
		prints((current->status) ? "BLOCKED" : "READY", MAX_BUFFER);
        sys_memFree(current);
        current = current->next;
    }
}

uint64_t get_pid(){
	return sys_getPid();
}

void run_loop(){
	prints("\n Presionar q para finalizar\n",30);
	char c=' ';
	while((c=getChar())!='q'){
		prints("\n Hola soy el proceso: ",30);
		printDec(get_pid());
		wait(2000);
		printc('\n');
	}
}


void sleep(int sec){
	sys_sleepTime(sec);
}


static int isVocal(char c){
	  if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' ||
        c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U') {
        return 1; 
    }
    return 0; 
}

int run_filter() {
 	prints("\nfiltrando vocales:", 50);
	char c;
	while ((c = getChar()) != EOF) {
		if (isVocal(c)){
	 		printc(c);
			wait(40);
		}
	}
	printc('\n');
	return 0;
}

int run_wc() {
	char c;
	int inputLines = 0;
	while ((int) (c = getChar()) != EOF)
		inputLines += (c == '\n');
	prints("\nCantidad de lineas:", MAX_BUFFER);
	printc(inputLines);
	printc('\n');
	return 0;
}

int run_cat() {
	/*
	char c;
	while ((c = getChar()) != EOF)
		printc(c);
	return 0;*/
	printc('\n');
	printDec(r);
	printc('\n');
}



static void reverse(char str[], int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

char* itoa(int num, char* str, int base) {
    int i = 0;
    int isNegative = 0;

    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    if (num < 0 && base == 10) {
        isNegative = 1;
        num = -num;
    }
    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }
    if (isNegative) {
        str[i++] = '-';
    }
    str[i] = '\0'; 
    reverse(str, i);

    return str;
}

int atoi(const char* str) {
    int result = 0; 
    int sign = 1;  
    int i = 0;     

    if (str[0] == '-') {
        sign = -1;
        i++; 
    }

    for (; str[i] != '\0'; ++i) {
        if (isDigit(str[i])) {
            result = result * 10 + str[i] - '0';
        } else {
            break;
        }
    }

    return sign * result;
}



int up_priority(pid_t pid){
	priority_t prio = sys_getPriority(pid);
	//prints("\n old prio: \n",MAX_BUFFER);
	//printDec(prio);
	//printc('\n');
    return sys_nice(pid, prio+1);
}