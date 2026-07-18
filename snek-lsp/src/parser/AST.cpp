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

void initNode(Node* node, NodeType type, SourceLocation start)
{
	node->type = type;
	node->start = start;
}

static uint32_t ceilToPowerOf2(uint32_t n)
{
	n--;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	n++;
	return n;
}

void initSymbolTable(SymbolTable* symbols, int capacity, Arena* arena)
{
	capacity = capacity * 100 / 70; // keep load under 70% for performance
	capacity = ceilToPowerOf2(capacity);

	symbols->slots = arena->alloc<SymbolEntry>(capacity);
	symbols->capacity = capacity;
	symbols->count = 0;
}

bool insertSymbol(SymbolTable* symbols, StringView identifier, Node* node)
{
	SnekAssert(symbols->count < symbols->capacity);

	uint32_t mask = symbols->capacity - 1;
	uint32_t h = hash(identifier);
	uint32_t index = h & mask;

	while (true)
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
}

Node* lookupSymbol(SymbolTable* symbols, StringView identifier)
{
	if (!symbols->count) return nullptr;

	uint32_t mask = symbols->capacity - 1;
	uint32_t h = hash(identifier);
	uint32_t index = h & mask;

	while (true)
	{
		SymbolEntry* slot = &symbols->slots[index];

		if (!slot->key)
			return nullptr;

		if (slot->key == h)
			return slot->value;

		index = (index + 1) & mask;
	}
}
