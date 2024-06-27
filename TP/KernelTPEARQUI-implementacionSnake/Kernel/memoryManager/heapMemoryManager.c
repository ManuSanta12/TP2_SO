#ifndef SIMPLE_MEMORY_MANAGER
#define SIMPLE_MEMORY_MANAGER

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define START_ADDRESS ((void*)0x600000)
#define MEMORY_SIZE (128 * 1024 * 1024) // 128MB
#define BLOCK_SIZE (8 * 1024) // 8KB
#define BLOCK_COUNT (MEMORY_SIZE / BLOCK_SIZE)

void* memory = START_ADDRESS;
void* pointerArray[BLOCK_COUNT];
size_t blockCount;

typedef struct {
    size_t totalMemory;
    size_t occupiedMemory;
    size_t freeMemory;
    size_t blocksUsed;
} MemoryInfo;

void create_memory(size_t size) {
    blockCount = size/BLOCK_SIZE;
    for (size_t i = 0; i < blockCount; i++) {
        pointerArray[i] = (void*)((void*)memory + i * BLOCK_SIZE);
    }
}

void* memory_manager_malloc(size_t size) {
    if (size > BLOCK_SIZE) {
        return NULL;
    }

    for (size_t i = 0; i < blockCount; i++) {
        if (pointerArray[i] != NULL) {
            void* block = pointerArray[i];
            pointerArray[i] = NULL;
            return block;
        }
    }

    return NULL; 
}

void free_memory_manager(void* ptr) {
    if (ptr == NULL) {
        return;
    }

    void* start = (void*)memory;
    void* end = start + MEMORY_SIZE;
    void* address = (void*)ptr;

    if (address < start || address >= end) {
        return; 
    }

    size_t index = (address - start) / BLOCK_SIZE;

    if (pointerArray[index] == NULL) {
        pointerArray[index] = ptr;

        if (index > 0 && pointerArray[index - 1] != NULL) {
            pointerArray[index - 1] = NULL;
            index = index - 1;
        }

        while (index < BLOCK_COUNT - 1 && pointerArray[index + 1] != NULL) {
            pointerArray[index] = NULL;
            index = index + 1;
        }
    }
}


MemoryInfo mem_info() {
    MemoryInfo info;
    info.totalMemory = MEMORY_SIZE;

    size_t freeBlocks = 0;
    for (size_t i = 0; i < BLOCK_COUNT; i++) {
        if (pointerArray[i] != NULL) {
            freeBlocks++;
        }
    }

    info.occupiedMemory = (BLOCK_COUNT - freeBlocks) * BLOCK_SIZE;
    info.freeMemory = freeBlocks * BLOCK_SIZE;
    info.blocksUsed = BLOCK_COUNT - freeBlocks;

    return info;
}

#endif




// #include <memoryManager.h>

// #define MEMORY_MANAGMENT_NAME "Size allocator"

// typedef struct blockCDT {
//   size_t size;           // Number of bytes allocated
//   size_t unused;         // Number of bytes free for allocated
//   struct blockCDT *prev; // Pointer to prevoius block
//   struct blockCDT *next; // Pointer to next block
// } BlockCDT;

// typedef BlockCDT *BlockADT;

// size_t totalHeapMemory;
// size_t usedHeapMemory;
// size_t memoryBlockCount;

// BlockADT head = NULL;

// void create_memory(size_t size) {
//   totalHeapMemory = size;
//   usedHeapMemory = sizeof(BlockCDT);
//   memoryBlockCount = 1;
//   head = (BlockADT)START_ADDRESS;
//   head->size = 0;
//   head->unused = totalHeapMemory - usedHeapMemory;
//   head->prev = NULL;
//   head->next = NULL;
// }
// void *memory_manager_malloc(size_t nbytes) {
//   if (head == NULL || nbytes == 0 ||
//       (totalHeapMemory - usedHeapMemory < nbytes))
//     return NULL;
//   BlockADT current = head;

//   while (current->unused < (nbytes + sizeof(BlockCDT))) {
//     if (current == NULL)
//       return NULL;
//     current = current->next;
//   }
//   if (current->size == 0) {
//     current->size = nbytes;
//     usedHeapMemory += nbytes; // Ver si hay que sumarle un sizeof(BlockCDT)
//     current->unused -= nbytes;
//     return (void *)current + sizeof(BlockCDT); // Return pointer to next block
//   } else {
//     BlockADT nextBlock =
//         (void *)((size_t)current + current->size + sizeof(BlockCDT));
//     nextBlock->size = nbytes;
//     nextBlock->unused = current->unused - nbytes - sizeof(BlockCDT);
//     current->unused = 0;
//     nextBlock->next = current->next;
//     nextBlock->prev = current;
//     if (current->next != NULL)
//       current->next->prev = nextBlock;
//     current->next = nextBlock;
//     usedHeapMemory += nbytes + sizeof(BlockCDT);
//     memoryBlockCount++;
//     return (void *)nextBlock + sizeof(BlockCDT); // Return pointer to next block
//   }
// }

// void free_memory_manager(void *ap) {
//   if (ap == NULL)
//     return;
//   BlockADT current = (void *)((size_t)ap - sizeof(BlockCDT));
//   if (current->prev == NULL) {
//     current->unused += current->size;
//     usedHeapMemory -= current->size;
//     current->size = 0;
//   } else {
//     BlockADT prevBlock = current->prev;
//     prevBlock->next = current->next;
//     if (current->next != NULL)
//       current->next->prev = prevBlock;
//     prevBlock->unused +=
//         current->size + current->unused + sizeof(usedHeapMemory);
//     usedHeapMemory -= current->size + sizeof(BlockCDT);
//     memoryBlockCount--;
//   }
// }

// MemoryInfo *mem_info() {
//   MemoryInfo *info = memory_manager_malloc(sizeof(MemoryInfo));
//   if (info == NULL) {
//     return NULL;
//   }
//   info->memoryAlgorithmName = MEMORY_MANAGMENT_NAME;
//   info->totalMemory = totalHeapMemory;
//   info->occupiedMemory = usedHeapMemory;
//   info->freeMemory = totalHeapMemory - usedHeapMemory;
//   info->blocksUsed = memoryBlockCount;
//   return info;
// }

