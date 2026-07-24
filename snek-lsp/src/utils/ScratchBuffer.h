#pragma once

#include <stdint.h>

#include "utils/Log.h"


struct ScratchBuffer
{
	uint8_t* memory;
	int capacity;
	int used;


	void* alloc(int size, int alignment);
	int mark();
	void release(int mark);

	template<typename T>
	T* alloc(int count = 1)
	{
		return (T*)alloc(sizeof(T) * count, alignof(T));
	}

	template<typename T>
	T* add(T value)
	{
		T* t = alloc<T>();
		*t = value;
		return t;
	}
	
	template<typename T>
	int count(int mark)
	{
		int alignment = alignof(T);
		size_t current = (size_t)(memory + mark);
		size_t aligned = (current + (alignment - 1)) & ~(alignment - 1);
		int offset = (int)(aligned - (size_t)memory);
		int bytes = used - offset;
		int n = bytes / sizeof(T);
		SnekAssert(n * sizeof(T) == bytes);

		return n;
	}
};


void initScratchBuffer(ScratchBuffer* buffer, int initialCapacity);
void destroyScratchBuffer(ScratchBuffer* buffer);
