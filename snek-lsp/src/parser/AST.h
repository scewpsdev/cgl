#pragma once

#include "Lexer.h"
#include "utils/Arena.h"
#include "utils/StringView.h"

#include <stdint.h>


enum NodeType : uint8_t
{
	NODE_NULL = 0,

	NODE_PRIMITIVE_TYPE,
	NODE_NAMED_TYPE,
	NODE_STRUCT_TYPE,
	NODE_UNION_TYPE,
	NODE_POINTER_TYPE,
	NODE_OPTIONAL_TYPE,
	NODE_FUNCTION_TYPE,
	NODE_TUPLE_TYPE,
	NODE_ARRAY_TYPE,

	NODE_EXPRESSION,

	NODE_FIELD,
	NODE_PARAMETER,

	NODE_STRUCT,
	NODE_ENUM,
	NODE_UNION,
	NODE_TYPEDEF,
	NODE_FUNCTION,
	NODE_GLOBAL_VARIABLE,
	NODE_MACRO,
	NODE_IMPORT,

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

	TYPE_INT_START,
	TYPE_INT8,
	TYPE_INT16,
	TYPE_INT32,
	TYPE_INT64,
	TYPE_UINT8,
	TYPE_UINT16,
	TYPE_UINT32,
	TYPE_UINT64,
	TYPE_INT_END,

	TYPE_FLOAT_BEGIN,
	TYPE_FLOAT,
	TYPE_DOUBLE,
	TYPE_FLOAT_END,

	TYPE_BOOL,
	TYPE_ANY,
	TYPE_STRING,
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
	int64_t constantSize;
};


struct Expression : NodeBase
{

};


struct Struct : NodeBase
{
	StringView name;
	uint32_t storage;

	Field** fields;
	int numFields;
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

	Field** fields;
	int numFields;
};

struct Typedef : NodeBase
{
	StringView name;
	uint32_t storage;
};

struct Parameter : NodeBase
{
	Type* type;
	StringView name;
};

struct Function : NodeBase
{
	StringView name;
	uint32_t storage;

	Parameter** params;
	int numParams;
	Type* returnType;
};

struct VariableDeclarator
{
	StringView name;
	Expression* value;
};

struct GlobalVariable : NodeBase
{
	Type* type;
	uint32_t storage;
	StringView* names;
	int numNames;
};

struct Macro : NodeBase
{
	StringView name;
	uint32_t storage;
};

struct Import : NodeBase
{
	StringView name;
};

struct Node
{
	union {
		struct {
			uint8_t type;
			int start, end;
		};

		// types

		Type primitiveType;
		NamedType namedType;
		StructType structType;
		UnionType unionType;
		PointerType pointerType;
		OptionalType optionalType;
		FunctionType functionType;
		TupleType tupleType;
		ArrayType arrayType;

		Expression expression;

		Field field;
		Parameter parameter;

		// declarations

		Struct struct_;
		Enum enum_;
		Union union_;
		Typedef typedef_;
		Function function;
		GlobalVariable globalVariable;
		Macro macro;
		Import import;
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

typedef void(*ASTVisitor_t)(Node* node, void* userPtr);


void initAST(AST* ast);
void destroyAST(AST* ast);

void initNode(Node* node, uint8_t type, int start);
void initType(Type* type, uint8_t nodeType, uint8_t typeKind, int start);

void initSymbolTable(SymbolTable* symbols, int capacity, Arena* arena);
bool insertSymbol(SymbolTable* symbols, StringView identifier, Node* node);
Node* lookupSymbol(SymbolTable* symbols, StringView identifier);

void initScope(Scope* scope, Scope* parent, bool isGlobal, Arena* arena);

void traverseAST(AST* ast, ASTVisitor_t visitor, void* userPtr);
