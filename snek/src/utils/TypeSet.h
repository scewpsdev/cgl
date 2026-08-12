#pragma once

#include "Hash.h"
#include "typechecker/TypeSystem.h" 

#include <stdlib.h>
#include <string.h>


enum TypeSetSlotState : uint8_t
{
	TYPE_SET_SLOT_EMPTY = 0,
	TYPE_SET_SLOT_USED = 1,
	TYPE_SET_SLOT_TOMBSTONE = 2,
};

struct TypeSetSlot
{
	Type* key;
	TypeSetSlotState state;
};

struct TypeSet
{
	TypeSetSlot* slots;
	int capacity;
	int numUsedSlots;
	int numTombstones;


	void clear();
	void grow(int newCapacity);
	void add(Type* key);
	Type** get(Type* key);
	bool contains(Type* key);
	bool remove(Type* key);
};


void initTypeSet(TypeSet* set, int initialCapacity = 16);
void destroyTypeSet(TypeSet* set);
