#ifndef _USR_STDLIB_H_
#define _USR_STDLIB_H_

#include <stdint.h>
#include <colores.h>
#include <stddef.h>
#include <uniqueTypes.h>

#define MAX_BUFFER 254

/* Prints a char in screen */
void printc (char c);

/* Prints a string in screen */
void prints (const char * str, int lenght);

/* Gets a single char from keyboard */
char getChar();

/*is c a char*/
int isChar(char c);

/*is digit??*/
int isDigit(char c);

/* returns lenght of a string*/
int strlen(const char *str);

/* compares str1 with str2*/
int strcmp(const char *str1, const char *str2);

/*parses a num string into uint64_t*/
uint64_t charToInt(char* str);

/* prints in different bases */
void printDec(uint64_t value);
void printHex(uint64_t value);
void printBin(uint64_t value);
void printBase(uint64_t value, uint32_t base);


void clear_scr();
int get_scrWidht();
int get_scrHeight();
void fill_rect(int x, int y, int x2, int y2, Color color);
void wait(uint64_t ms);
void inforeg();
void test_invopcode();
void test_zerodiv();
//void sizePlus();
//void sizeMinus();
void dummy();

int print_mem(uint64_t mem);

void increaseScale();

void decreaseScale();

void startBeep(uint32_t frequnce);

void stopBeep();

void memory_manager_usr(uint64_t memoryManager);

void* mm_malloc(uint64_t size);

void mm_free(void * ap);

void print_meminfo();

uint8_t sem_init(char*name,int value);

uint8_t sem_post(char*name);

uint8_t sem_wait(char*name, int pid);

uint8_t sem_close(char*name);

void run_loop();

uint64_t get_pid();

int run_filter();

int run_wc();

int run_cat();

char* itoa(int num, char* str, int base);

int atoi(const char* str);

void sleep(int sec);

void new_process(void *code, char **args, char *name, uint8_t priority);

void get_process_info();

int up_priority(pid_t pid);
    
char get_char_block();


#endif