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

void initType(Type* type, uint8_t nodeType, uint8_t typeKind, int start)
{
	initNode((Node*)type, nodeType, start);
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

static void traverseType(Type* type, ASTVisitor_t visitor, void* userPtr)
{
	visitor((Node*)type, userPtr);

	if (type->typeKind == TYPE_STRUCT)
	{

	}
	else if (type->typeKind == TYPE_UNION)
	{

	}
	else if (type->typeKind == TYPE_POINTER)
	{
		PointerType* pointerType = (PointerType*)type;
		traverseType(pointerType->elementType, visitor, userPtr);
	}
	else if (type->typeKind == TYPE_OPTIONAL)
	{
		OptionalType* optionalType = (OptionalType*)type;
		traverseType(optionalType->elementType, visitor, userPtr);
	}
	else if (type->typeKind == TYPE_FUNCTION)
	{

	}
	else if (type->typeKind == TYPE_TUPLE)
	{

	}
	else if (type->typeKind == TYPE_ARRAY)
	{
		ArrayType* arrayType = (ArrayType*)type;
		traverseType(arrayType->elementType, visitor, userPtr);
	}
}

static void traverseField(Field* field, ASTVisitor_t visitor, void* userPtr)
{
	visitor((Node*)field, userPtr);

	traverseType(field->type, visitor, userPtr);
}

static void traverseParameter(Parameter* parameter, ASTVisitor_t visitor, void* userPtr)
{
	visitor((Node*)parameter, userPtr);

	traverseType(parameter->type, visitor, userPtr);
}

static void traverseDeclaration(Node* declaration, ASTVisitor_t visitor, void* userPtr)
{
	visitor(declaration, userPtr);

	if (declaration->type == NODE_STRUCT)
	{
		Struct* struct_ = (Struct*)declaration;
		for (int i = 0; i < struct_->numFields; i++)
		{
			traverseField(struct_->fields[i], visitor, userPtr);
		}
	}
	else if (declaration->type == NODE_ENUM)
	{
	}
	else if (declaration->type == NODE_UNION)
	{
		Union* union_ = (Union*)declaration;
		for (int i = 0; i < union_->numFields; i++)
		{
			traverseField(union_->fields[i], visitor, userPtr);
		}
	}
	else if (declaration->type == NODE_TYPEDEF)
	{
	}
	else if (declaration->type == NODE_FUNCTION)
	{
		Function* function = (Function*)declaration;
		for (int i = 0; i < function->numParams; i++)
		{
			traverseParameter(function->params[i], visitor, userPtr);
		}
		if (function->returnType)
		{
			traverseType(function->returnType, visitor, userPtr);
		}
	}
	else if (declaration->type == NODE_GLOBAL_VARIABLE)
	{
		GlobalVariable* globalVariable = &declaration->globalVariable;
		traverseType(globalVariable->type, visitor, userPtr);
	}
	else if (declaration->type == NODE_MACRO)
	{
	}
	else if (declaration->type == NODE_IMPORT)
	{
	}
}

void traverseAST(AST* ast, ASTVisitor_t visitor, void* userPtr)
{
	for (int i = 0; i < ast->numDeclarations; i++)
	{
		traverseDeclaration(ast->declarations[i], visitor, userPtr);
	}
}
