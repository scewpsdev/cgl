#pragma once

#include "Type.h"

#include "utils/Arena.h"
#include "utils/HashMap.h"


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
Type* getAnonymousStructType(TypeSystem* types, int numElements, Type** fieldTypes, char** fieldNames);
Type* getNamedStructType(TypeSystem* types, StringView name, int numElements, Type** fieldTypes, char** fieldNames);
Type* getFunctionType(TypeSystem* types, Type* returnType, int numParams, Type** paramTypes);
Type* getArrayType(TypeSystem* types, Type* elementType, uint64_t size);
