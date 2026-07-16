#include "Arena.h"

#include <stdlib.h>

#ifdef _WIN32
#include <Windows.h>
#endif


void initArena(Arena* arena, int capacity)
{
	arena->capacity = capacity;
	arena->committed = 0;
	arena->offset = 0;

#ifdef _WIN32
	arena->buffer = (char*)VirtualAlloc(NULL, capacity, MEM_RESERVE, PAGE_READWRITE);
#else
	arena->buffer = (char*)malloc(capacity);
#endif
}

void destroyArena(Arena* arena)
{
#ifdef _WIN32
	VirtualFree(arena->buffer, 0, MEM_RELEASE);
#else
	free(arena->buffer);
#endif

	arena->capacity = 0;
}

void Arena::reset()
{
	offset = 0;
}

void* Arena::alloc(int size)
{
	int alignedSize = (size + 7) & ~7;
	if (offset + alignedSize > capacity)
		return nullptr;

#ifdef _WIN32
	if (offset + alignedSize > committed)
	{
		const int pageSize = 4096;
		int neededCommit = offset + alignedSize;
		int alignedCommit = (neededCommit + (pageSize - 1)) & ~(pageSize - 1);

		int sizeToCommit = alignedCommit - committed;
		void* commitAddress = &buffer[committed];

		void* result = VirtualAlloc(commitAddress, sizeToCommit, MEM_COMMIT, PAGE_READWRITE);

		committed = alignedCommit;
	}
#endif

	void* ptr = &buffer[offset];
	offset += alignedSize;
	return ptr;
}
