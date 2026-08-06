#pragma once

#include <stdint.h>


struct MemoryBlock
{
	MemoryBlock* next;

#define MEMORY_BLOCK_SIZE 64 * 1024
	uint8_t data[MEMORY_BLOCK_SIZE];
};

struct GlobalBlockPool
{
	MemoryBlock* freeList;
};

struct Arena
{
	GlobalBlockPool* blockPool;
	MemoryBlock* head;
	int offset;


	void reset();
	void* alloc(int size);

	template<typename T>
	T* alloc(int count = 1)
	{
		return (T*)alloc(count * sizeof(T));
	}
};


void initGlobalBlockPool(GlobalBlockPool* pool, int numBlocks);
MemoryBlock* acquireMemoryBlock(GlobalBlockPool* pool);

void initArena(Arena* arena, GlobalBlockPool* blockPool);
void destroyArena(Arena* arena);

void resetArena(Arena* arena);
