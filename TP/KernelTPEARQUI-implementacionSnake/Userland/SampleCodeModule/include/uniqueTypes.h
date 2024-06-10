#ifndef UNIQUE_TYPES_H
#define UNIQUE_TYPES_H

#include <stdint.h>
#include <stddef.h>

#define MAX_BUFFER 512
#define READY 0
#define BLOCKED 1

typedef int pid_t;
typedef unsigned int priority_t;
typedef unsigned int status_t;

typedef struct processInfo{
    pid_t pid;
    priority_t priority;
    uint64_t stackBase;
    status_t status;
    struct processInfo * next;
} processInfo;


// Memory
typedef struct memoryInfo
{
    char *memoryAlgorithmName;
    uint64_t freeMemory;
    uint64_t occupiedMemory;
    uint64_t totalMemory;
    unsigned int blocksUsed;
} MemoryInfo;

// Semaphores
typedef uint64_t sem_t;


#endif