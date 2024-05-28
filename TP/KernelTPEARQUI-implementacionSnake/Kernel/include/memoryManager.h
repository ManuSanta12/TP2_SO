#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <defs.h>
#include <stdint.h>
#include <unistd.h>

#define START_ADDRESS 0xF00000

void create_memory(size_t size);
void *memory_manager_malloc(size_t nbytes);
void free_memory_manager(void *ap);
MemoryInfo *mem_info();

#endif
