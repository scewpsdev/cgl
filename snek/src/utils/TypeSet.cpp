#include "TypeSet.h"


void initTypeSet(TypeSet* set, int initialCapacity)
{
	// Enforce a minimum capacity and power of 2
	int capacity = 16;
	while (capacity < initialCapacity)
		capacity *= 2;

	set->numUsedSlots = 0;
	set->numTombstones = 0;
	set->capacity = capacity;
	set->slots = (TypeSetSlot*)calloc(capacity, sizeof(TypeSetSlot));
}

void destroyTypeSet(TypeSet* set)
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

void TypeSet::clear()
{
	if (slots)
	{
		memset(slots, 0, sizeof(TypeSetSlot) * capacity);
	}
	numUsedSlots = 0;
	numTombstones = 0;
}

void TypeSet::grow(int newCapacity)
{
	TypeSetSlot* oldSlots = slots;
	int oldCapacity = capacity;

	capacity = newCapacity;
	numUsedSlots = 0;
	numTombstones = 0; // Growth clears all tombstones
	slots = (TypeSetSlot*)calloc(newCapacity, sizeof(TypeSetSlot));

	int mask = newCapacity - 1;

	// Rehash all used slots into the new array
	for (int i = 0; i < oldCapacity; i++)
	{
		if (oldSlots[i].state == TYPE_SET_SLOT_USED)
		{
			Type* key = oldSlots[i].key;
			int idx = hashType(key) & mask;

			while (slots[idx].state == TYPE_SET_SLOT_USED)
			{
				idx = (idx + 1) & mask;
			}

			slots[idx].key = key;
			slots[idx].state = TYPE_SET_SLOT_USED;
			numUsedSlots++;
		}
	}

	free(oldSlots);
}

void TypeSet::add(Type* key)
{
	if (capacity == 0)
	{
		initTypeSet(this);
	}
	// Grow if 75% occupied (including tombstones)
	else if ((numUsedSlots + numTombstones) * 4 >= capacity * 3)
	{
		grow(capacity * 2);
	}

	int mask = capacity - 1;
	int idx = hashType(key) & mask;
	int firstTombstone = INT32_MAX;

	while (true)
	{
		TypeSetSlotState state = slots[idx].state;
		if (state == TYPE_SET_SLOT_EMPTY)
		{
			int target = (firstTombstone != INT32_MAX ? firstTombstone : idx);
			TypeSetSlot* slot = &slots[target];
			slot->key = key;

			if (slot->state == TYPE_SET_SLOT_TOMBSTONE)
				numTombstones--;

			slot->state = TYPE_SET_SLOT_USED;
			numUsedSlots++;
			return;
		}
		else if (state == TYPE_SET_SLOT_TOMBSTONE)
		{
			if (firstTombstone == INT32_MAX)
				firstTombstone = idx;
		}
		else if (state == TYPE_SET_SLOT_USED && compareTypes(slots[idx].key, key))
		{
			return;
		}

		idx = (idx + 1) & mask;
	}
}

Type** TypeSet::get(Type* key)
{
	if (capacity == 0 || numUsedSlots == 0)
		return nullptr;

	int mask = capacity - 1;
	int idx = hashType(key) & mask;
	int firstIdx = idx;

	while (true)
	{
		TypeSetSlotState state = slots[idx].state;
		if (state == TYPE_SET_SLOT_EMPTY)
			return nullptr;
		else if (state == TYPE_SET_SLOT_USED && compareTypes(slots[idx].key, key))
			return &slots[idx].key;

		idx = (idx + 1) & mask;
		if (idx == firstIdx)
			return nullptr;
	}
}

bool TypeSet::contains(Type* key)
{
	return get(key) != nullptr;
}

bool TypeSet::remove(Type* key)
{
	if (capacity == 0 || numUsedSlots == 0)
		return false;

	int mask = capacity - 1;
	int idx = hashType(key) & mask;
	int firstIdx = idx;

	while (true)
	{
		TypeSetSlotState state = slots[idx].state;
		if (state == TYPE_SET_SLOT_EMPTY)
		{
			return false;
		}
		else if (state == TYPE_SET_SLOT_USED && compareTypes(slots[idx].key, key))
		{
			slots[idx].state = TYPE_SET_SLOT_TOMBSTONE;
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
