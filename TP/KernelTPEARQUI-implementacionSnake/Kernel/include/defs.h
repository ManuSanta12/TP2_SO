/***************************************************
  Defs.h
****************************************************/

#ifndef _defs_
#define _defs_

#include <stddef.h>
#include <stdint.h>

/* Flags para derechos de acceso de los segmentos */
#define ACS_PRESENT 0x80 /* segmento presente en memoria */
#define ACS_CSEG 0x18    /* segmento de codigo */
#define ACS_DSEG 0x10    /* segmento de datos */
#define ACS_READ 0x02    /* segmento de lectura */
#define ACS_WRITE 0x02   /* segmento de escritura */
#define ACS_IDT ACS_DSEG
#define ACS_INT_386 0x0E /* Interrupt GATE 32 bits */
#define ACS_INT (ACS_PRESENT | ACS_INT_386)

#define ACS_CODE (ACS_PRESENT | ACS_CSEG | ACS_READ)
#define ACS_DATA (ACS_PRESENT | ACS_DSEG | ACS_WRITE)
#define ACS_STACK (ACS_PRESENT | ACS_DSEG | ACS_WRITE)

#define PIPESIZE 512
#define FDS 10
#define OPEN 1
#define CLOSED 0

#define EFLAGS 0x202
#define CS     0x8
#define SS          0
#define EXIT_CODE   0



typedef int pid_t;

typedef struct blockednode {
    pid_t pid;
    struct blockednode *next;
} blockednode;

typedef struct {
    blockednode *head;
    blockednode *tail;
    unsigned int size;
} BlockedQueueCDT;


typedef BlockedQueueCDT *BlockedQueueADT;

typedef struct
{
    char *name;
    uint64_t value; // it wont be negative, process that try to wait when 0 will be stacked in blockedProcess
    uint64_t processesOpened;
    uint8_t locked; // if its locked its 1 if not 0;
    BlockedQueueADT blockedProcesses;
} semaphore;

typedef semaphore *sem_t;

typedef struct Pipe
{
    char data[PIPESIZE];
    unsigned int processCount;
    unsigned int isReadOpen;
    unsigned int isWriteOpen;
    uint64_t readIndex;
    uint64_t writeIndex;
    BlockedQueueADT writeQueue;
    BlockedQueueADT readQueue;
} Pipe;

typedef struct pipeNode
{
    Pipe *pipe;
    struct pipeNode *next;
    struct pipeNode *previous;
} pipeNode;

typedef struct PipeList {
    pipeNode *head;
    pipeNode *tail;
} PipeList;

typedef pipeNode *pipeList;

typedef struct
{
    unsigned int mode;
} fd_t;


typedef unsigned int priority_t;
typedef unsigned int status_t;

typedef struct context{
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;

    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;

    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;

    uint64_t rip;
    uint64_t cs;
    uint64_t eflags;
    uint64_t rsp;
    uint64_t ss;
}context;

typedef struct
{
    pid_t pid;
    priority_t priority;
    status_t status;
    BlockedQueueADT blockedQueue;
    fd_t fileDescriptors[FDS];
    Pipe *pipe;
    unsigned int lastFd;
    unsigned int argc;
    char **argv;
    context* context;
    uint8_t run;
} PCB;


typedef struct node
{
    PCB process;
    struct node *next;
} Node;

typedef Node *Queue;

typedef struct processInfo
{
    pid_t pid;
    priority_t priority;
    uint64_t stackBase;
    status_t status;
    struct processInfo *next;
} processInfo;

typedef int (*fun)(int, char *[]);

typedef struct memoryInfo
{
    char *memoryAlgorithmName;
    size_t freeMemory;
    size_t occupiedMemory;
    size_t totalMemory;
    unsigned int blocksUsed;
} MemoryInfo;

#endif