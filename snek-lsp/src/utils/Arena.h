#pragma once


struct Arena
{
	char* buffer;
	int capacity;
	int offset;

	void reset();
	void* alloc(int size);

	template<typename T>
	T* alloc(int count = 1)
	{
		return (T*)alloc(count * sizeof(T));
	}
};


void initArena(Arena* arena, int capacity);
void destroyArena(Arena* arena);
