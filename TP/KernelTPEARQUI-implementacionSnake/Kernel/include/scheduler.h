#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <linkedList.h>
#include <defs.h>
#include <pipe.h>
// Prototipos de las funciones definidas en el archivo .c
void create_sch();
static uint16_t get_next_pid();
int32_t set_priority(pid_t pid, priority_t newPriority);
int8_t set_status(uint16_t pid, uint8_t newStatus);
status_t get_status(pid_t pid);
void* schedule(void* prevStackPointer);
int16_t create_process(main_foo code, char** args, char* name, uint8_t priority, int16_t fileDescriptors[], uint8_t unkillable);
static void destroy_zombie(PCB* zombie);
int32_t kill_current_process(int32_t retValue);
int32_t kill_process(uint16_t pid, int32_t retValue);
processInfoList* get_processes_info();
char** copy_argv(int argc, char** argv);
void process_wrapper(main_foo fun, char** args);
static int argv_len(char** argv);
void new_process(pid_t pid, pid_t parentPID, main_foo mainFun, char** argv, char* name, priority_t priority, fd_t fileDescriptors[], uint8_t kill);
static void assign_FD(PCB* new, uint8_t fdIndex, fd_t fdValue, uint8_t mode);
void close_all_fd(PCB* p);
void free_process(PCB* p);
processInfo* save_info(processInfo* info, PCB* p);
int is_process_waiting(PCB* p, uint16_t pidToWait);
int get_zombie_info(int processIndex, processInfo psArray[], PCB* nextProcess);
int get_zombie_value(uint16_t pid);
int is_process_alive(uint16_t pid);
void yield();
int change_file_descriptor(uint16_t pid, uint8_t position, int16_t newFd);
int get_current_process_FD(uint8_t fdIndex);
void kill_foreground_process();
pid_t get_current_pid();

#endif // SCHEDULER_H
