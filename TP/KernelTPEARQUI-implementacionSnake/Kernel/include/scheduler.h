#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <naiveConsole.h>
#include <interrupts.h>
#include <defs.h>
#include <queue.h>

#define READY 0
#define BLOCKED 1
#define FDS 10
#define OPEN 1
#define CLOSED 0
#define PIPESIZE 512

#define FOREGROUND 1
#define BACKGROUND 0

extern void force_tick();

void dummy_process();
void create_sch();
static uint16_t get_next_pid();
int32_t set_priority(pid_t pid, priority_t newPriority);
int8_t set_status(uint16_t pid, uint8_t newStatus);
status_t get_status(pid_t pid);
void *schedule(void *prevStackPointer);
int16_t create_process(void (*code)(), char **args, char *name, uint8_t priority, int16_t fileDescriptors[], uint8_t unkillable);
static void destroy_zombie(PCB *zombie);
int32_t kill_current_process(int32_t retValue);
int32_t kill_process(uint16_t pid, int32_t retValue);
processInfo *get_processes_info();
char **copy_argv(int argc, char **argv);
void process_wrapper(void (*fun)(), char **args);
Node *new_process(uint64_t rip, int argc, char *argv[], pid_t parentPID, char *name);
static void assignFileDescriptor(Node *new, uint8_t fdIndex, int16_t fdValue, uint8_t mode);
void close_all_fd(Node *p);
void free_process(Node *p);
processInfo *save_info(Node *p);
int is_process_waiting(Node *p, uint16_t pidToWait);
int get_zombie_info(int processIndex, processInfo psArray[], PCB *nextProcess);
int get_zombie_value(uint16_t pid);
int is_process_alive(uint16_t pid);
void yield();
int change_file_descriptor(uint16_t pid, uint8_t position, int16_t newFd);
int get_current_process_FD(uint8_t fdIndex);
void kill_foreground_process();

#endif 