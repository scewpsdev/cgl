#include "AST.h"

#include "utils/Hash.h"
#include "utils/Log.h"

#include <stdio.h>


void initAST(AST* ast)
{
}

void destroyAST(AST* ast)
{
}

void initNode(Node* node, uint8_t type, int start)
{
	node->type = type;
	node->start = start;
}

void initType(Type* type, uint8_t typeKind, int start)
{
	initNode((Node*)type, NODE_TYPE, start);
	type->typeKind = typeKind;
}

void initSymbolTable(SymbolTable* symbols, int capacity, Arena* arena)
{
	// todo check if power of 2

	symbols->slots = arena->alloc<SymbolEntry>(capacity);
	symbols->capacity = capacity;
	symbols->count = 0;

	symbols->arena = arena;
}

static void growSymbolTable(SymbolTable* symbols)
{
	int newCapacity = symbols->capacity * 2;

	SymbolEntry* newSlots = symbols->arena->alloc<SymbolEntry>(newCapacity);

	uint32_t mask = newCapacity - 1;

	for (int i = 0; i < symbols->capacity; i++)
	{
		SymbolEntry* slot = &symbols->slots[i];

		if (slot->key)
		{
			uint32_t index = slot->key & mask;
			SymbolEntry* newSlot = &newSlots[index];

			while (true)
			{
				if (!newSlot->key)
				{
					*newSlot = *slot;
					break;
				}

				index = (index + 1) & mask;
			}
		}
	}

	symbols->slots = newSlots;
	symbols->capacity = newCapacity;
}

bool insertSymbol(SymbolTable* symbols, StringView identifier, Node* node)
{
	if (symbols->count * 100 > symbols->capacity * 70)
	{
		growSymbolTable(symbols);
	}

	uint32_t mask = symbols->capacity - 1;
	uint32_t h = hash(identifier);
	uint32_t index = h & mask;

	for (int i = 0; i < symbols->capacity; i++)
	{
		SymbolEntry* slot = &symbols->slots[index];

		if (!slot->key)
		{
			slot->key = h;
			slot->value = node;
			symbols->count++;
			return true;
		}

		if (slot->key == h)
		{
			return false;
		}

		index = (index + 1) & mask;
	}

	SnekAssert(false);
	return false;
}

Node* lookupSymbol(SymbolTable* symbols, StringView identifier)
{
	if (!symbols->count) return nullptr;

	uint32_t mask = symbols->capacity - 1;
	uint32_t h = hash(identifier);
	uint32_t index = h & mask;

	for (int i = 0; i < symbols->capacity; i++)
	{
		SymbolEntry* slot = &symbols->slots[index];

		if (!slot->key)
			return nullptr;

		if (slot->key == h)
			return slot->value;

		index = (index + 1) & mask;
	}

	return nullptr;
}

void initScope(Scope* scope, Scope* parent, bool isGlobal, Arena* arena)
{
	scope->parent = parent;

	int symbolCapacity = isGlobal ? 1024 : 16;
	initSymbolTable(&scope->symbols, symbolCapacity, arena);
}
