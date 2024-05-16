#include <memoryManager.h>
#include <lib.h>

#define FREE 0
#define USED 1
#define LEVELS 32
#define MIN_IDX 5 // MemoryBlock size

typedef struct buddyBlockCDT {
  uint8_t pos;           // position in blocks array
  uint8_t used;         // Used or not
  struct buddyBlockCDT *prev; // Pointer to prevoius block
  struct buddyBlockCDT *next; // Pointer to next block
} BuddyBlock;

typedef BuddyBlock* BlockADT;

typedef struct MemoryManagerCDT {
	size_t total_size;
    uint8_t max_pos; //Max allocable position in block array
    void *firstAddress;
	BlockADT blocks[LEVELS];
	MemoryInfo memory_info;
} MemoryManagerCDT;

typedef MemoryManagerCDT * MemoryManagerADT;

static BlockADT removeBlockFromList(BlockADT blocks[], BlockADT to_remove);
static BlockADT createMemoryBlock(void *ptrToAllocate, uint8_t pos,BlockADT next);
static BlockADT merge(BlockADT block,BlockADT buddy);
static void split(uint8_t pos);

MemoryManagerADT mem;

void create_memory(size_t size) {
    mem->total_size=size;
    mem->max_pos=log(size,2);
    for (int i = 0; i < LEVELS; i++)
		mem->blocks[i] = NULL;

    mem->firstAddress = (void*) START_ADDRESS;
    mem->memory_info.blocksUsed=0;
    mem->memory_info.freeMemory=size;
    mem->memory_info.occupiedMemory=0;
    mem->memory_info.memoryAlgorithmName="Buddy manager";
    mem->memory_info.totalMemory=size;

    mem->blocks[mem->max_pos - 1] =
		createMemoryBlock(mem, mem->max_pos, NULL);
}

void *memory_manager_malloc(size_t nbytes){
    uint8_t block_pos = log(nbytes+sizeof(BuddyBlock),2); //En que posición del array va a estar el bloque.
    block_pos = block_pos < MIN_IDX - 1 ? MIN_IDX - 1 : block_pos;

    if(block_pos>=mem->max_pos)
        return NULL; //me pidieron mas memoria de la disponible
    
    
	if (mem->blocks[block_pos] == NULL) {
		uint8_t closest_pos = 0;
		for (uint8_t i = block_pos + 1; i < mem->max_pos && !closest_pos; i++)
			if (mem->blocks[i] != NULL)
				closest_pos = i;
		if (closest_pos == 0)
			return NULL;
		for (; closest_pos > block_pos; closest_pos--)
			split(closest_pos);
	}

        BlockADT block = mem->blocks[block_pos];
        size_t block_size = 1L << block->pos;
        mem->memory_info.occupiedMemory += block_size;
        mem->memory_info.freeMemory -= block_size;
        mem->memory_info.blocksUsed++;

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
    mem->memory_info.blocksUsed--;
    mem->memory_info.freeMemory += blockSize;
    mem->memory_info.occupiedMemory -= blockSize;

    uint64_t relativePosition = (uint64_t) ((void *) block - mem->firstAddress);
	BlockADT buddyBlock = (BlockADT) ((uint64_t) mem->firstAddress + (((uint64_t) relativePosition) ^ (1L << block->pos)));
	while (buddyBlock->used == FREE && buddyBlock->pos == block->pos && block->pos < mem->max_pos) {
		block = merge(block, buddyBlock);
		relativePosition = (uint64_t) ((void *) block - mem->firstAddress);
		buddyBlock = (BlockADT) ((uint64_t) mem->firstAddress + (((uint64_t) relativePosition) ^ (1L << block->pos)));
	}
	mem->blocks[block->pos - 1] = createMemoryBlock((void *) block, block->pos, mem->blocks[block->pos - 1]);


}

MemoryInfo *mem_info(){
    return &mem->memory_info;
}

static void split(uint8_t pos) {
	BlockADT block = mem->blocks[pos];
	removeBlockFromList(mem->blocks, block);
	BlockADT buddyBlock =
		(BlockADT) ((void *) block + (1L << pos));
	createMemoryBlock((void *) buddyBlock, pos, mem->blocks[pos - 1]);
	mem->blocks[pos - 1] = createMemoryBlock((void *) block, pos, buddyBlock);

}

static BlockADT removeBlockFromList(BlockADT blocks[], BlockADT to_remove){
    if(to_remove->prev != NULL){
        to_remove->prev->next = to_remove->next;
    } else {
        blocks[to_remove->pos-1] = to_remove->next;
    }

    if(to_remove->next!=NULL){
        to_remove->next->prev = to_remove->prev;
    }
}   

static BlockADT createMemoryBlock(void *ptrToAllocate, uint8_t pos,BlockADT next){
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
	removeBlockFromList(mem->blocks, buddy);
	BlockADT leftBlock = block < buddy ? block : buddy;
	leftBlock->pos++;
	return leftBlock;
}