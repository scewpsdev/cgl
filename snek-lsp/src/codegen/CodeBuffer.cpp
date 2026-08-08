#include "CodeBuffer.h"

#include "utils/Arena.h"

#include <string.h>
#include <stdio.h>


void initCodeBuffer(CodeBuffer* buffer, Arena* arena, int initialCapacity)
{
	buffer->data = (char*)arena->alloc(initialCapacity);
	buffer->capacity = initialCapacity;
	buffer->count = 0;

	buffer->arena = arena;
}

void resetCodeBuffer(CodeBuffer* buffer)
{
	buffer->count = 0;
}

static void reserve(CodeBuffer* buffer, int size)
{
	if (buffer->count + size > buffer->capacity)
	{
		int newCapacity = buffer->capacity * 2;
		while (newCapacity < buffer->count + size)
			newCapacity *= 2;

		char* newData = (char*)buffer->arena->alloc(newCapacity);
		if (buffer->count)
			memcpy(newData, buffer->data, buffer->count);
		buffer->data = newData;
		buffer->capacity = newCapacity;
	}
}

void emitString(CodeBuffer* buffer, const char* str)
{
	int len = (int)strlen(str);
	reserve(buffer, len);
	memcpy(buffer->data + buffer->count, str, len);
	buffer->count += len;
}

void emitString(CodeBuffer* buffer, StringView str)
{
	reserve(buffer, str.length);
	memcpy(buffer->data + buffer->count, str.ptr, str.length);
	buffer->count += str.length;
}

void emitChar(CodeBuffer* buffer, char c)
{
	reserve(buffer, 1);
	buffer->data[buffer->count++] = c;
}

void emitInteger(CodeBuffer* buffer, int64_t i)
{
	char str[64] = "";
	sprintf(str, "%lld", i);
	emitString(buffer, str);
}
