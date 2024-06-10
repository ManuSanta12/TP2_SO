#ifndef BUDDY_MM

#include <memoryManager.h>
#include <lib.h>

#define FREE 0
#define USED 1
#define LEVELS 32
#define MIN_IDX 5 // MemoryBlock size
#define MEMORY_MANAGER_ADDRESS 0x50000	

typedef struct buddyBlockCDT {
  uint8_t pos;           // position in blocks array
  uint8_t used;         // Used or not
  struct buddyBlockCDT *prev; // Pointer to prevoius block
  struct buddyBlockCDT *next; // Pointer to next block
} BuddyBlock;

typedef BuddyBlock* BlockADT;

typedef struct MemoryManagerCDT {
	size_t totalSize;
    uint8_t maxPos; //Max allocable position in block array
    void *firstAddress;
	BlockADT blocks[LEVELS];
    size_t usedMem;
    size_t blockCount;
    size_t freeMem
} MemoryManagerCDT;

typedef MemoryManagerCDT * MemoryManagerADT;

static BlockADT remove_block(BlockADT blocks[], BlockADT to_remove);
static BlockADT create_memory_block(void *ptrToAllocate, uint8_t pos,BlockADT next);
static BlockADT merge(BlockADT block,BlockADT buddy);
static void split(uint8_t pos);

MemoryManagerADT mem;

void create_memory(size_t size) {
    mem = (MemoryManagerADT)START_ADDRESS;
    mem->totalSize=size;
    mem->maxPos=log(size,2);
    for (int i = 0; i < LEVELS; i++)
		mem->blocks[i] = NULL;

    mem->firstAddress = (void*) START_ADDRESS+(sizeof(MemoryManagerCDT)+1);
    mem->blockCount=0;
    mem->freeMem=size;
    mem->usedMem=0;

    mem->blocks[mem->maxPos - 1] =
		create_memory_block(mem, mem->maxPos, NULL);
}

void *memory_manager_malloc(size_t nbytes){
    uint8_t blockPos = log(nbytes+sizeof(BuddyBlock),2); //En que posición del array va a estar el bloque.
    blockPos = blockPos < MIN_IDX - 1 ? MIN_IDX - 1 : blockPos;

    if(blockPos>=mem->maxPos){
        return NULL; 
    }
    
	if(mem->blocks[blockPos] == NULL) {
		uint8_t closestPos = 0;
		for (uint8_t i = blockPos + 1; i < mem->maxPos && !closestPos; i++)
			if (mem->blocks[i] != NULL)
				closestPos = i;
		if (closestPos == 0)
			return NULL;
		for (; closestPos > blockPos; closestPos--)
			split(closestPos);
	}

        BlockADT block = mem->blocks[blockPos];
        size_t blockSize = 1L << block->pos;
        mem->usedMem += blockSize;
        mem->freeMem -= blockSize;
        mem->blockCount++;

        void *allocation = (void *) block + sizeof(BuddyBlock);
	    return (void *) allocation;
}

void free_memory_manager(void *ap){
    if(ap == NULL )
        return;
    BlockADT block = (BlockADT) (ap - sizeof(BuddyBlock));
    
    if(block->used==FREE)
        return;
    
    uint64_t blockSize = 1L << block->pos;
    mem->blockCount--;
    mem->freeMem += blockSize;
    mem->usedMem -= blockSize;

    uint64_t relativePosition = (uint64_t) ((void *) block - mem->firstAddress);
	BlockADT buddyBlock = (BlockADT) ((uint64_t) mem->firstAddress + (((uint64_t) relativePosition) ^ (1L << block->pos)));
	while (buddyBlock->used == FREE && buddyBlock->pos == block->pos && block->pos < mem->maxPos) {
		block = merge(block, buddyBlock);
		relativePosition = (uint64_t) ((void *) block - mem->firstAddress);
		buddyBlock = (BlockADT) ((uint64_t) mem->firstAddress + (((uint64_t) relativePosition) ^ (1L << block->pos)));
	}
	mem->blocks[block->pos - 1] = create_memory_block((void *) block, block->pos, mem->blocks[block->pos - 1]);


}

MemoryInfo *mem_info(){
    MemoryInfo * info = memory_manager_malloc(sizeof(MemoryInfo));
    if(info==NULL){
        return NULL;
    }
    info->memoryAlgorithmName = "Buddy manager";
    info->totalMemory = mem->totalSize;
    info->occupiedMemory = mem->usedMem;
    info->blocksUsed = mem->blockCount;
    info->freeMemory = mem->freeMem;
    
}

static void split(uint8_t pos) {
	BlockADT block = mem->blocks[pos];
	remove_block(mem->blocks, block);
	BlockADT buddyBlock =
		(BlockADT) ((void *) block + (1L << pos));
	create_memory_block((void *) buddyBlock, pos, mem->blocks[pos - 1]);
	mem->blocks[pos - 1] = create_memory_block((void *) block, pos, buddyBlock);

}

static BlockADT remove_block(BlockADT blocks[], BlockADT to_remove){
    if(to_remove->prev != NULL){
        to_remove->prev->next = to_remove->next;
    } else {
        blocks[to_remove->pos-1] = to_remove->next;
    }

    if(to_remove->next!=NULL){
        to_remove->next->prev = to_remove->prev;
    }
}   

static BlockADT create_memory_block(void *ptrToAllocate, uint8_t pos,BlockADT next){
    	BlockADT new_block = (BlockADT) ptrToAllocate;
        new_block->pos  = pos;
        new_block->used = FREE;        
        new_block->prev = NULL;
        new_block->next = next;
        if (next != NULL) {
		    next->prev = new_block;
	    }
	return new_block;
} 

static BlockADT merge(BlockADT block,BlockADT buddy) {
	remove_block(mem->blocks, buddy);
	BlockADT leftBlock = block < buddy ? block : buddy;
	leftBlock->pos++;
	return leftBlock;
}

#endif
