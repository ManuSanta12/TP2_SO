#include <memoryManager.h>

#define FREE 0
#define USED 1
#define LEVELS 32


typedef struct buddyBlockCDT {
  size_t size;           // Number of bytes allocated
  uint8_t used;         // Number of bytes free for allocated
  struct blockCDT *prev; // Pointer to prevoius block
  struct blockCDT *next; // Pointer to next block
} BuddyBlockCDT;

typedef BuddyBlockCDT *BlockADT;

BlockADT head = NULL;

void create_memory(size_t size) {}

void *memory_manager_malloc(size_t nbytes){}

void free_memory_manager(void *ap){}

MemoryInfo *mem_info(){}