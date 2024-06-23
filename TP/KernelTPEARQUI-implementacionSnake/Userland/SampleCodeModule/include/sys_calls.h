#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

#include <stdint.h>
#include <colores.h>
#include "usr_stdlib.h"
#include <uniqueTypes.h>


/*
 * Pasaje de parametros en C:
   %rdi %rsi %rdx %rcx %r8 %r9
 */
uint64_t sys_read(uint64_t fd, char* buf, uint64_t count);

uint64_t sys_write(uint64_t fd, const char buf, uint64_t count);

uint64_t sys_clear();

uint64_t sys_getHours();

uint64_t sys_getMinutes();

uint64_t sys_getSeconds();

uint64_t sys_scrHeight();

uint64_t sys_scrWidth();

uint64_t sys_fillRect (int x, int y, int x2, int y2, Color color);

uint64_t sys_wait(uint64_t ms);

uint64_t sys_inforeg(uint64_t reg[17]);

uint64_t sys_printmem(uint64_t mem);

uint64_t sys_pixelPlus();

uint64_t sys_pixelMinus();

uint64_t sys_playSound(uint32_t frequence);

uint64_t sys_mute();

uint64_t sys_mem(uint64_t memoryManager);

MemoryInfo *sys_memInfo();

void* sys_memMalloc(uint64_t size);

void sys_memFree(void * ap);

uint8_t sys_semInit(char*name,int value);

uint8_t sys_semPost(char*name);

uint8_t sys_semWait(char*name, int pid);

uint8_t sys_semClose(char * name);

int sys_newProcess(void* rip, int bg, char*argv[],int argc);

uint64_t sys_getPid();

uint64_t sys_sleepTime(int sec);

processInfo * sys_ps();

priority_t sys_getPriority(pid_t pid);

int sys_nice(pid_t pid, int newPriority);

int sys_kill(pid_t pid);

#endif  
