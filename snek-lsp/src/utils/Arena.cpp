#include "Arena.h"

#include <stdlib.h>
#include <string.h>


void initGlobalBlockPool(GlobalBlockPool* pool, int numBlocks)
{
	*pool = {};

	pool->freeList = nullptr;
	for (int i = 0; i < numBlocks; i++)
	{
		MemoryBlock* block = (MemoryBlock*)malloc(sizeof(MemoryBlock));
		block->next = pool->freeList;
		pool->freeList = block;
	}
}

MemoryBlock* acquireMemoryBlock(GlobalBlockPool* pool)
{
	if (!pool->freeList)
		return (MemoryBlock*)malloc(sizeof(MemoryBlock));

	MemoryBlock* block = pool->freeList;
	pool->freeList = block->next;
	block->next = nullptr;
	return block;
}

void initArena(Arena* arena, GlobalBlockPool* blockPool)
{
	*arena = {};

	arena->blockPool = blockPool;
	arena->head = nullptr;
	arena->offset = 0;
}

void destroyArena(Arena* arena)
{
	resetArena(arena);
}

void resetArena(Arena* arena)
{
	if (!arena->head)
		return;

	MemoryBlock* tail = arena->head;
	while (tail->next)
	{
		tail = tail->next;
	}

	tail->next = arena->blockPool->freeList;
	arena->blockPool->freeList = arena->head;

	arena->head = nullptr;
	arena->offset = 0;
}

void* Arena::alloc(int size)
{
	int alignedSize = (size + 7) & ~7;

	if (!head || offset + alignedSize > MEMORY_BLOCK_SIZE)
	{
		MemoryBlock* block = acquireMemoryBlock(blockPool);
		block->next = head;
		head = block;
		offset = 0;
	}

	void* data = head->data + offset;
	offset += alignedSize;

	memset(data, 0, alignedSize);

	return data;
}
