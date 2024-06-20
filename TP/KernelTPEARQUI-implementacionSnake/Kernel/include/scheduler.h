#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <naiveConsole.h>
#include <interrupts.h>
#include <defs.h>
#include <queue.h>

#define READY 0
#define BLOCKED 1
#define TERMINATED 2
#define FDS 10
#define OPEN 1
#define CLOSED 0
#define PIPESIZE 512

#define FOREGROUND 1
#define BACKGROUND 0


void dummyProcess();
void createScheduler();
PCB *getProcess(pid_t pid);
uint64_t getCurrentPid();
int blockProcess(pid_t pid);
int unblockProcess(pid_t pid);
pid_t new_process(fun foo, int argc, char *argv[]);
void nextProcess();
int prepareDummy(pid_t pid);
context* contextSwitch(context* rsp);
int killProcess(int returnValue, char autokill);
int changePriority(pid_t pid, int priorityValue);
int yieldProcess();
processInfo * getProccessesInfo();
priority_t get_priority(pid_t pid);
int kill_by_pid(pid_t pid);

#endif // SCHEDULER_H