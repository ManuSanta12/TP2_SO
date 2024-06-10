#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

#include <stdint.h>
#include <colores.h>
#include "usr_stdlib.h"

/*
 * Pasaje de parametros en C:
   %rdi %rsi %rdx %rcx %r8 %r9
 */
uint64_t sys_read(uint64_t fd, char *buf);

uint64_t sys_write(uint64_t fd, const char buf);

uint64_t sys_clear();

uint64_t sys_getHours();

uint64_t sys_getMinutes();

uint64_t sys_getSeconds();

uint64_t sys_scrHeight();

uint64_t sys_scrWidth();

uint64_t sys_fillRect(int x, int y, int x2, int y2, Color color);

uint64_t sys_wait(uint64_t ms);

uint64_t sys_inforeg(uint64_t reg[17]);

uint64_t sys_printmem(uint64_t mem);

uint64_t sys_pixelPlus();

uint64_t sys_pixelMinus();

uint64_t sys_playSound(uint32_t frequence);

uint64_t sys_mute();

uint64_t sys_mem(uint64_t memoryManager);

MemoryInfo *sys_memInfo();

void *sys_memMalloc(uint64_t size);

void sys_memFree(void *ap);

sem_t sys_sem_open(char *name, uint64_t value);

int sys_sem_close(sem_t sem);

int sys_sem_post(sem_t sem);

int sys_sem_wait(sem_t sem);

int sys_yieldProcess();

int sys_nice(pid_t pid, int new_priority);

int sys_pipe(int pipefd[2]);

int sys_dup2(int fd1, int fd2);

int sys_open(int fd);

int sys_close(int fd);

processInfo *sys_ps();

int sys_changeProcessStatus(pid_t pid);

pid_t sys_getCurrentPid();

pid_t sys_exec(uint64_t program, unsigned int argc, char *argv[]);

void sys_exit(int return_value, char autokill);

pid_t sys_waitpid(pid_t pid);

int sys_kill(pid_t pid);

int sys_block(pid_t pid);

int sys_unblock(pid_t pid);
#endif
