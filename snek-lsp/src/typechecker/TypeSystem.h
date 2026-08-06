#pragma once

#include "parser/TypeKind.h"
#include "parser/AST.h"

#include "utils/Arena.h"
#include "utils/HashMap.h"

#include <stdint.h>


struct AST;

struct Type
{
	TypeKind typeKind;
	StringView name;

	union {
		struct {
			StringView name;
			int numFields;
			Type** fieldTypes;
			StringView* fieldNames;
		} struct_;

		struct {
			StringView name;
			int numFields;
			Type** fieldTypes;
			StringView* fieldNames;
		} union_;

		struct {
			StringView name;
			Type* valueType;
		} enum_;

		struct {
			StringView name;
			Type* valueType;
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
	FileHandle file;
};

struct TypeTable
{
	TypeEntry* entries;
	int count;
	int capacity;
};

struct TypeSystem
{
	Type errorType;
	Type primitiveTypes[TYPE_COUNT];

	TypeTable typeTable;
};


void initTypeSystem(TypeSystem* types);
void destroyTypeSystem(TypeSystem* types);

void freeTypesFromFile(FileHandle fileHandle);

Type* getPointerType(TypeSystem* types, Type* elementType, Arena* arena, FileHandle file);
Type* getOptionalType(TypeSystem* types, Type* elementType, Arena* arena, FileHandle file);
Type* getAnonymousStructType(TypeSystem* types, int numElements, Type** fieldTypes, StringView* fieldNames, Arena* arena, FileHandle file);
Type* getAnonymousUnionType(TypeSystem* types, int numElements, Type** fieldTypes, StringView* fieldNames, Arena* arena, FileHandle file);
Type* getFunctionType(TypeSystem* types, Type* returnType, int numParams, Type** paramTypes, Arena* arena, FileHandle file);
Type* getArrayType(TypeSystem* types, Type* elementType, uint64_t size, Arena* arena, FileHandle file);

Type* createNamedStructType(TypeSystem* types, StringView name, Arena* arena);
void resolveNamedStructType(TypeSystem* types, Type* type, int numFields, Type** fieldTypes, StringView* fieldNames);

Type* createNamedUnionType(TypeSystem* types, StringView name, Arena* arena);
void resolveNamedUnionType(TypeSystem* types, Type* type, int numFields, Type** fieldTypes, StringView* fieldNames);

Type* createEnumType(TypeSystem* types, StringView name, Arena* arena);
void resolveEnumType(TypeSystem* types, Type* type, Type* valueType);

Type* createAliasType(TypeSystem* types, StringView name, Arena* arena);
void resolveAliasType(TypeSystem* types, Type* type, Type* value);
