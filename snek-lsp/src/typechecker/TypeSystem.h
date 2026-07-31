#pragma once

#include "parser/TypeKind.h"

#include "utils/Arena.h"
#include "utils/HashMap.h"

#include <stdint.h>


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
};

struct TypeTable
{
	TypeEntry* entries;
	int count;
	int capacity;
};

struct TypeSystem
{
	Arena arena;

	Type errorType;
	Type primitiveTypes[TYPE_COUNT];

	TypeTable typeTable;
};


void initTypeSystem(TypeSystem* types);
void destroyTypeSystem(TypeSystem* types);

Type* getPointerType(TypeSystem* types, Type* elementType);
Type* getOptionalType(TypeSystem* types, Type* elementType);
Type* getAnonymousStructType(TypeSystem* types, int numElements, Type** fieldTypes, StringView* fieldNames);
Type* getNamedStructType(TypeSystem* types, StringView name, int numElements, Type** fieldTypes, StringView* fieldNames);
Type* getAnonymousUnionType(TypeSystem* types, int numElements, Type** fieldTypes, StringView* fieldNames);
Type* getNamedUnionType(TypeSystem* types, StringView name, int numElements, Type** fieldTypes, StringView* fieldNames);
Type* getEnumType(TypeSystem* types, StringView name, Type* valueType);
Type* getFunctionType(TypeSystem* types, Type* returnType, int numParams, Type** paramTypes);
Type* getArrayType(TypeSystem* types, Type* elementType, uint64_t size);
