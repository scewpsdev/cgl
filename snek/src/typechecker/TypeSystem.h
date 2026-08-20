#pragma once

#include "parser/TypeKind.h"
#include "parser/AST.h"

#include "utils/Arena.h"
#include "utils/HashMap.h"

#include <stdint.h>


struct File;
struct AST;

struct Type
{
	TypeKind typeKind;
	StringView name;
	StringView mangledName;
	uint64_t hash;

	union {
		struct {
			StringView name;
			int numFields;
			int numOffsetFields;
			Type** fieldTypes;
			StringView* fieldNames;
			int* fieldOffsets;
			Struct* declaration;
		} struct_;

		struct {
			StringView name;
			int numFields;
			Type** fieldTypes;
			StringView* fieldNames;
			Union* declaration;
		} union_;

		struct {
			StringView name;
			Type* valueType;
			Enum* declaration;
		} enum_;

		struct {
			StringView name;
			Type* valueType;
			Typedef* declaration;
		} alias;

		struct {
			Type* elementType;
		} pointer;

		struct {
			Type* elementType;
		} optional;

		struct {
			Type* returnType;
			int numParams;
			Type** paramTypes;
			bool variadic;
		} function;

		struct {
			Type* elementType;
			uint64_t size;
		} array;
	};
};

struct TypeEntry
{
	uint64_t key;
	Type* value;
};

struct TypeTable
{
	Arena* arena;
	TypeEntry* entries;
	int count;
	int capacity;
};

struct TypeSystem
{
	Type errorType;
	Type primitiveTypes[TYPE_COUNT];

	Arena* arena;
	TypeTable typeTable;
};


void initTypeTable(TypeTable* table, Arena* arena, int initialCapacity);

bool isVoidType(Type* type);
bool isIntegerType(Type* type);
bool isUnsignedType(Type* type);
bool isFloatingPointType(Type* type);
bool isTruthyType(Type* type);
bool isNumericType(Type* type);
bool isErrorType(Type* type);
bool isCharPointerType(Type* type);
bool isPrimitiveType(Type* type);

Type* getVoidType(TypeSystem* types);
Type* getInt32Type(TypeSystem* types);
Type* getStringType(TypeSystem* types);
Type* getErrorType(TypeSystem* types);

uint64_t hashType(Type* type);
bool compareTypes(Type* a, Type* b);

void initTypeSystem(TypeSystem* types, Arena* globalArena);
void destroyTypeSystem(TypeSystem* types);

Type* getPointerType(TypeSystem* types, Type* elementType, File* file);
Type* getOptionalType(TypeSystem* types, Type* elementType, File* file);
Type* getAnonymousStructType(TypeSystem* types, int numElements, Type** fieldTypes, StringView* fieldNames, File* file);
Type* getAnonymousUnionType(TypeSystem* types, int numElements, Type** fieldTypes, StringView* fieldNames, File* file);
Type* getFunctionType(TypeSystem* types, Type* returnType, int numParams, Type** paramTypes, bool variadic, File* file);
Type* getArrayType(TypeSystem* types, Type* elementType, uint64_t size, File* file);

Type* createNamedStructType(File* file, StringView name, Struct* declaration);
void resolveNamedStructType(Type* type, int numFields, int numOffsetFields, Type** fieldTypes, StringView* fieldNames, int* fieldOffsets, File* file);

Type* createNamedUnionType(File* file, StringView name, Union* declaration);
void resolveNamedUnionType(Type* type, int numFields, Type** fieldTypes, StringView* fieldNames, File* file);

Type* createEnumType(File* file, StringView name, Enum* declaration);
void resolveEnumType(Type* type, Type* valueType);

Type* createAliasType(File* file, StringView name, Typedef* declaration);
void resolveAliasType(Type* type, Type* value);

StringView mangleFunctionName(TypeSystem* types, StringView name, Type* functionType, Arena* arena);
