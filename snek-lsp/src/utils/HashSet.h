#pragma once

#include "Hash.h"

#include <stdlib.h>
#include <string.h>


enum HashSetSlotState : uint8_t
{
	HASH_SET_SLOT_EMPTY = 0,
	HASH_SET_SLOT_USED = 1,
	HASH_SET_SLOT_TOMBSTONE = 2,
};

template<typename KEY_TYPE>
struct HashSetSlot
{
	KEY_TYPE key;
	HashSetSlotState state;
};

template<typename KEY_TYPE>
struct HashSet
{
	HashSetSlot<KEY_TYPE>* slots;
	int capacity;
	int numUsedSlots;
	int numTombstones;


	void clear()
	{
		if (slots)
		{
			memset(slots, 0, sizeof(HashSetSlot<KEY_TYPE>) * capacity);
		}
		numUsedSlots = 0;
		numTombstones = 0;
	}

	void grow(int newCapacity)
	{
		HashSetSlot<KEY_TYPE>* oldSlots = slots;
		int oldCapacity = capacity;

		capacity = newCapacity;
		numUsedSlots = 0;
		numTombstones = 0; // Growth clears all tombstones
		slots = (HashSetSlot<KEY_TYPE>*)calloc(newCapacity, sizeof(HashSetSlot<KEY_TYPE>));

		int mask = newCapacity - 1;

		// Rehash all used slots into the new array
		for (int i = 0; i < oldCapacity; i++)
		{
			if (oldSlots[i].state == HASH_SET_SLOT_USED)
			{
				const KEY_TYPE& key = oldSlots[i].key;
				int idx = hash(key) & mask;

				while (slots[idx].state == HASH_SET_SLOT_USED)
				{
					idx = (idx + 1) & mask;
				}

				slots[idx].key = key;
				slots[idx].state = HASH_SET_SLOT_USED;
				numUsedSlots++;
			}
		}

		free(oldSlots);
	}

	KEY_TYPE* add(const KEY_TYPE& key)
	{
		if (capacity == 0)
		{
			initHashSet(this);
		}
		// Grow if 75% occupied (including tombstones)
		else if ((numUsedSlots + numTombstones) * 4 >= capacity * 3)
		{
			grow(capacity * 2);
		}

		int mask = capacity - 1;
		int idx = hash(key) & mask;
		int firstTombstone = INT32_MAX;

		while (true)
		{
			HashSetSlotState state = slots[idx].state;
			if (state == HASH_SET_SLOT_EMPTY)
			{
				int target = (firstTombstone != INT32_MAX ? firstTombstone : idx);
				HashSetSlot<KEY_TYPE>* slot = &slots[target];
				slot->key = key;

				if (slot->state == HASH_SET_SLOT_TOMBSTONE)
					numTombstones--;

				slot->state = HASH_SET_SLOT_USED;
				numUsedSlots++;
				return &slot->key;
			}
			else if (state == HASH_SET_SLOT_TOMBSTONE)
			{
				if (firstTombstone == INT32_MAX)
					firstTombstone = idx;
			}
			else if (state == HASH_SET_SLOT_USED && slots[idx].key == key)
			{
				return &slots[idx].key;
			}

			idx = (idx + 1) & mask;
		}
	}

	KEY_TYPE* get(const KEY_TYPE& key)
	{
		if (capacity == 0 || numUsedSlots == 0)
			return nullptr;

		int mask = capacity - 1;
		int idx = hash(key) & mask;
		int firstIdx = idx;

		while (true)
		{
			HashSetSlotState state = slots[idx].state;
			if (state == HASH_SET_SLOT_EMPTY)
				return nullptr;
			else if (state == HASH_SET_SLOT_USED && slots[idx].key == key)
				return &slots[idx].key;

			idx = (idx + 1) & mask;
			if (idx == firstIdx)
				return nullptr;
		}
	}

	bool contains(const KEY_TYPE& key)
	{
		return get(key) != nullptr;
	}

	bool remove(const KEY_TYPE& key)
	{
		if (capacity == 0 || numUsedSlots == 0)
			return false;

		int mask = capacity - 1;
		int idx = hash(key) & mask;
		int firstIdx = idx;

		while (true)
		{
			HashSetSlotState state = slots[idx].state;
			if (state == HASH_SET_SLOT_EMPTY)
			{
				return false;
			}
			else if (state == HASH_SET_SLOT_USED && slots[idx].key == key)
			{
				slots[idx].state = HASH_SET_SLOT_TOMBSTONE;
				numUsedSlots--;
				numTombstones++;
				return true;
			}

			idx = (idx + 1) & mask;

			if (idx == firstIdx)
			{
				return false;
			}
		}
	}
};


template<typename KEY_TYPE>
inline void initHashSet(HashSet<KEY_TYPE>* set, int initialCapacity = 16)
{
	// Enforce a minimum capacity and power of 2
	int capacity = 16;
	while (capacity < initialCapacity)
		capacity *= 2;

	set->numUsedSlots = 0;
	set->numTombstones = 0;
	set->capacity = capacity;
	set->slots = (HashSetSlot<KEY_TYPE>*)calloc(capacity, sizeof(HashSetSlot<KEY_TYPE>));
}

template<typename KEY_TYPE>
inline void freeHashSet(HashSet<KEY_TYPE>* set)
{
	if (set->slots)
	{
		free(set->slots);
		set->slots = nullptr;
	}
	set->capacity = 0;
	set->numUsedSlots = 0;
	set->numTombstones = 0;
}
