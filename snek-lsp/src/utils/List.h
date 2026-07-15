#pragma once

#include "Log.h"

#include <stdlib.h>


template<typename T>
struct List
{
	T* buffer = nullptr;
	int capacity = 0;
	int size = 0;

	T& operator[](int idx)
	{
		SnekAssert(idx >= 0 && idx < size);
		return buffer[idx];
	}

	const T& operator[](int idx) const
	{
		SnekAssert(idx >= 0 && idx < size);
		return buffer[idx];
	}

	void ensureCapacity(int minCapacity)
	{
		if (minCapacity > capacity)
		{
			int newCapacity = capacity == 0 ? 8 : capacity * 2;
			if (newCapacity < minCapacity)
				newCapacity = minCapacity;

			// realloc acts like malloc if buffer is nullptr
			T* newBuffer = (T*)realloc(buffer, sizeof(T) * newCapacity);
			SnekAssert(newBuffer != nullptr);

			buffer = newBuffer;
			capacity = newCapacity;
		}
	}

	void resize(int newSize)
	{
		if (newSize > capacity)
		{
			ensureCapacity(newSize);
		}
		size = newSize;
	}

	void add(const T& t)
	{
		if (size >= capacity)
		{
			ensureCapacity(capacity == 0 ? 8 : capacity * 2);
		}
		buffer[size++] = t;
	}

	void removeAt(int idx)
	{
		SnekAssert(idx >= 0 && idx < size);
		for (int i = idx; i < size - 1; i++)
			buffer[i] = buffer[i + 1];
		size--;
	}

	void insert(int idx, const T& element)
	{
		SnekAssert(idx >= 0 && idx < size);
		resize(size + 1);
		for (int i = size - 1; i >= idx + 1; i--)
			buffer[i] = buffer[i - 1];
		buffer[idx] = element;
	}

	int indexOf(const T& t)
	{
		for (int i = 0; i < size; i++)
		{
			if (buffer[i] == t)
				return i;
		}
		return -1;
	}

	bool contains(const T& t)
	{
		return indexOf(t) != -1;
	}

	void remove(const T& t)
	{
		int idx = indexOf(t);
		if (idx != -1)
			removeAt(idx);
	}

	void clear()
	{
		size = 0;
	}
};

// C-style explicit initialization
template<typename T>
void InitList(List<T>* list, int initialCapacity = 8)
{
	list->capacity = initialCapacity;
	list->size = 0;
	list->buffer = (T*)malloc(sizeof(T) * initialCapacity);
	SnekAssert(list->buffer != nullptr);
}

// C-style explicit destruction
template<typename T>
void FreeList(List<T>* list)
{
	if (list->buffer)
	{
		free(list->buffer);
		list->buffer = nullptr;
	}
	list->capacity = 0;
	list->size = 0;
}