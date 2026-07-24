#include "ScratchBuffer.h"

#include <stdlib.h>
#include <string.h>


void initScratchBuffer(ScratchBuffer* buffer, int initialCapacity)
{
	buffer->capacity = initialCapacity;
	buffer->used = 0;

	buffer->memory = (uint8_t*)malloc(initialCapacity);
}

void destroyScratchBuffer(ScratchBuffer* buffer)
{
	if (buffer->memory)
		free(buffer->memory);
}

static void growScratchBuffer(ScratchBuffer* buffer, int requiredSize)
{
	int newCapacity = buffer->capacity;
	while (requiredSize > newCapacity)
		newCapacity *= 2;

	uint8_t* newMemory = (uint8_t*)malloc(newCapacity);
	memcpy(newMemory, buffer->memory, buffer->used);
	free(buffer->memory);
	buffer->memory = newMemory;
	buffer->capacity = newCapacity;
}

void* ScratchBuffer::alloc(int size, int alignment)
{
	size_t current = (size_t)(memory + used);
	size_t aligned = (current + (alignment - 1)) & ~(alignment - 1);
	int offset = (int)(aligned - (size_t)memory);

	if (offset + size > capacity)
		growScratchBuffer(this, offset + size);

	used = offset + size;
	return memory + offset;
}

int ScratchBuffer::mark()
{
	return used;
}

void ScratchBuffer::release(int mark)
{
	used = mark;
}
