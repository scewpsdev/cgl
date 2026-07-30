#pragma once

#include <stdint.h>
#include <stdlib.h>


template<typename T>
struct HashTableEntry
{
	uint64_t key;
	T value;
};

template<typename S, typename T>
struct HashTable
{
	HashTableEntry<T>* entries;
	int count;
	int capacity;


	T* getOrInsert(S key, T value)
	{
		if (count * 4 >= capacity * 3)
		{
			int newCapacity = capacity * 2;
			HashTableEntry<T>* newEntries = (HashTableEntry<T>*)calloc(newCapacity, sizeof(HashTableEntry<T>));

			for (int i = 0; i < capacity; i++)
			{
				if (entries[i].value)
				{
					uint64_t h = hash(entries[i].key);
					int index = h & (newCapacity - 1);
					while (newEntries[index].value)
					{
						index = (index + 1) & (newCapacity - 1);
					}
					newEntries[index] = entries[i];
				}
			}

			free(entries);
			entries = newEntries;
			capacity = newCapacity;
		}

		uint64_t h = hash(key);
		index = h & (capacity - 1);
		while (entries[index])
		{
			if (equals(entries[index].key, key))
			{
				return entries[index].value;
			}
			index = (index + 1) & (capacity - 1);
		}


	}

	T* get(uint64_t key)
	{

	}
};


template<typename T>
void initHashTable(HashTable<T>* table, int initialCapacity)
{
	table->entries = (HashTableEntry<T>*)calloc(initialCapacity, sizeof(HashTableEntry<T>));
	table->capacity = initialCapacity;
	table->count = 0;
}
