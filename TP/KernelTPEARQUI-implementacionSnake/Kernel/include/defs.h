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

#define DEV_NULL -1


#define FDS 3
#define OPEN 1
#define CLOSED 0

#define STDIN 0
#define STDOUT 1
#define STDERR 2

#define READ 0
#define WRITE 1

#define MAX_PROC 4096

#define BLOCKED 0
#define READY 1
#define RUNNING 2
#define ZOMBIE 3
#define DEAD 4

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
/*
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
*/
/*
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
*/
typedef int16_t fd_t;


typedef unsigned int priority_t;
typedef int8_t status_t;

typedef struct
{
    pid_t pid;
    pid_t  parentPID;
    priority_t priority;
    status_t status;
    uint16_t waiting;
    unsigned int quantumsLeft;
    void *rsp;
    void *stackBase;
    BlockedQueueADT zombie; //los hijos que quedan zombie
    fd_t fileDescriptors[FDS];
    //Pipe *pipe;
    char * name;
    unsigned int argc;
    char **argv;
    int64_t retVal;
    uint8_t canBeKilled;

} PCB;

typedef struct node
{
    PCB process;
    struct node *next;
} Node;

typedef Node *Queue;

typedef struct processInfo
{
    char*name;
    pid_t pid;
    pid_t parentPID;
    priority_t priority;
    void *stackBase;
    void *rsp;
    status_t status;
    uint8_t foreground;
    //struct processInfo *next;
} processInfo;


typedef struct processInfoList {
    int lenght;
    processInfo *processList;
}processInfoList;


typedef struct memoryInfo
{
    char *memoryAlgorithmName;
    size_t freeMemory;
    size_t occupiedMemory;
    size_t totalMemory;
    unsigned int blocksUsed;
} MemoryInfo;

typedef int (*main_foo)(int argc, char **args);

#endif
