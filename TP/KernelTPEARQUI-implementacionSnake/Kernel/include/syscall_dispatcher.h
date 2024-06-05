#ifndef SYSCALLDISPATCHER_H
#define SYSCALLDISPATCHER_H

#include <stdint.h>
#include <naiveConsole.h>
#include <lib.h>
#include <keyboard.h>
#include <scheduler.h>
#include <defs.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define PIPEOUT 3
#define PIPEIN 4

typedef struct
{
    uint8_t day, month, year;
    uint8_t hours, minutes, seconds;
} sysTime_t;

// uint64_t syscall_dispatcher(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rax, uint64_t *registers);
uint64_t syscall_dispatcher(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t r10,
                         uint64_t r8, uint64_t rax);
#endif