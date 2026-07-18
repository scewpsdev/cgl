#pragma once

#include "Lexer.h"
#include "utils/Arena.h"

#include <stdint.h>


enum NodeType
{
	NODE_NULL = 0,

	NODE_STRUCT,
	NODE_ENUM,
	NODE_UNION,
	NODE_TYPEDEF,
	NODE_FUNCTION,
	NODE_MACRO,

	NODE_COUNT
};

enum StorageSpecifier
{
	STORAGE_NULL = 0,

	STORAGE_CONSTANT = 1 << 0,
	STORAGE_EXTERN = 1 << 1,
	STORAGE_DLLEXPORT = 1 << 2,
	STORAGE_DLLIMPORT = 1 << 3,
	STORAGE_PRIVATE = 1 << 4,
	STORAGE_PACKED = 1 << 5,
	STORAGE_NOMANGLE = 1 << 6,
};

struct NodeBase
{
	NodeType type;
	SourceLocation start, end;
};

struct Struct : NodeBase
{
	StringView name;
	uint32_t storage;
};

struct Enum : NodeBase
{
	StringView name;
	uint32_t storage;
};

struct Union : NodeBase
{
	StringView name;
	uint32_t storage;
};

struct Typedef : NodeBase
{
	StringView name;
	uint32_t storage;
};

struct Function : NodeBase
{
	StringView name;
	uint32_t storage;
};

struct Macro : NodeBase
{
	StringView name;
	uint32_t storage;
};

struct Node
{
	union {
		struct {
			NodeType type;
			SourceLocation start, end;
		};

		Struct struct_;
		Enum enum_;
		Union union_;
		Typedef typedef_;
		Function function;
		Macro macro;
	};
};

struct SymbolEntry
{
	uint32_t key;
	Node* value;
};

struct SymbolTable
{
	SymbolEntry* slots;
	int capacity;
	int count;
};

struct AST
{
	Node** declarations;
	int numDeclarations;

	SymbolTable symbols;
};


void initAST(AST* ast);
void destroyAST(AST* ast);

void initNode(Node* node, NodeType type, SourceLocation start);

void initSymbolTable(SymbolTable* symbols, int capacity, Arena* arena);
bool insertSymbol(SymbolTable* symbols, StringView identifier, Node* node);
Node* lookupSymbol(SymbolTable* symbols, StringView identifier);
