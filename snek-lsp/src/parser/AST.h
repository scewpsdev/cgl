#pragma once

#include "Lexer.h"
#include "utils/Arena.h"

#include <stdint.h>


enum NodeType : uint8_t
{
	NODE_NULL = 0,

	NODE_STRUCT,
	NODE_ENUM,
	NODE_UNION,
	NODE_TYPEDEF,
	NODE_FUNCTION,
	NODE_MACRO,

	NODE_TYPE,

	NODE_COUNT
};

enum StorageSpecifier : uint8_t
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
	uint8_t type;
	int start, end;
};

enum TypeKind : uint8_t
{
	TYPE_NULL = 0,

	TYPE_VOID,
	TYPE_INTEGER,
	TYPE_FLOAT,
	TYPE_BOOL,
	TYPE_ANY,
	TYPE_STRING,
	TYPE_NAMED,
	TYPE_STRUCT,
	TYPE_UNION,
	TYPE_POINTER,
	TYPE_OPTIONAL,
	TYPE_FUNCTION,
	TYPE_TUPLE,
	TYPE_ARRAY,
};

struct Type : NodeBase
{
	uint8_t typeKind;
};

struct IntegerType : Type
{
	uint8_t bitWidth;
	bool isSigned;
};

struct FloatType : Type
{
	uint8_t bitWidth;
};

struct NamedType : Type
{
	StringView name;
};

struct Field : NodeBase
{
	Type* type;
	StringView name;
};

struct StructType : Type
{
	Field** fields;
	int numFields;
};

struct UnionType : Type
{
	Field** fields;
	int numFields;
};

struct PointerType : Type
{
	Type* elementType;
};

struct OptionalType : Type
{
	Type* elementType;
};

struct FunctionType : Type
{
	Type* returnType;
	Type** paramTypes;
	StringView* paramNames;
	int numParams;
	Type* variadicType;
};

struct TupleType : Type
{
	Type** elementTypes;
	int numElementTypes;
};

struct ArrayType : Type
{
	Type* elementType;
	int constantSize;
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
			uint8_t type;
			int start, end;
		};

		// types

		IntegerType integerType;
		FloatType floatType;
		NamedType namedType;
		StructType structType;
		UnionType unionType;
		PointerType pointerType;
		OptionalType optionalType;
		FunctionType functionType;
		TupleType tupleType;
		ArrayType arrayType;

		// declarations

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

	Arena* arena;
};

struct Scope
{
	Scope* parent;
	SymbolTable symbols;
};

struct AST
{
	Node** declarations;
	int numDeclarations;

	Scope* globalScope;
};


void initAST(AST* ast);
void destroyAST(AST* ast);

void initNode(Node* node, uint8_t type, int start);
void initType(Type* type, uint8_t typeKind, int start);

void initSymbolTable(SymbolTable* symbols, int capacity, Arena* arena);
bool insertSymbol(SymbolTable* symbols, StringView identifier, Node* node);
Node* lookupSymbol(SymbolTable* symbols, StringView identifier);

void initScope(Scope* scope, Scope* parent, bool isGlobal, Arena* arena);
