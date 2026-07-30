#include "TypeSystem.h"

#include "utils/Hash.h"


static void initTypeTable(TypeTable* table, int initialCapacity)
{
	table->entries = (TypeEntry*)calloc(initialCapacity, sizeof(TypeEntry));
	table->capacity = initialCapacity;
	table->count = 0;
}

static void destroyTypeTable(TypeTable* table)
{
	if (table->entries)
		free(table->entries);
}

static uint64_t hashType(Type* type)
{
	uint64_t hash = 14695981039346656037ULL; // FNV offset basis

	// Hash the kind
	hash = (hash ^ type->typeKind) * 1099511628211ULL;

	switch (type->typeKind)
	{
	case TYPE_STRUCT:
		if (type->struct_.name)
			hash = (hash ^ (uintptr_t)type->struct_.name) * 1099511628211ULL;
		for (int i = 0; i < type->struct_.numFields; i++)
			hash = (hash ^ (uintptr_t)type->struct_.fieldTypes[i]) * 1099511628211ULL;
		break;
	case TYPE_UNION:
		if (type->union_.name)
			hash = (hash ^ (uintptr_t)type->union_.name) * 1099511628211ULL;
		for (int i = 0; i < type->union_.numFields; i++)
			hash = (hash ^ (uintptr_t)type->union_.fieldTypes[i]) * 1099511628211ULL;
		break;
	case TYPE_POINTER:
		hash = (hash ^ (uintptr_t)type->pointer.elementType) * 1099511628211ULL;
		break;
	case TYPE_OPTIONAL:
		hash = (hash ^ (uintptr_t)type->optional.elementType) * 1099511628211ULL;
		break;
	case TYPE_ARRAY:
		hash = (hash ^ (uintptr_t)type->array.elementType) * 1099511628211ULL;
		hash = (hash ^ type->array.size) * 1099511628211ULL;
		break;
	case TYPE_FUNCTION:
		hash = (hash ^ (uintptr_t)type->function.returnType) * 1099511628211ULL;
		for (int i = 0; i < type->function.numParams; i++)
			hash = (hash ^ (uintptr_t)type->function.paramTypes[i]) * 1099511628211ULL;
		break;
	default:
		break;
	}

	return hash;
}

static bool compareTypes(Type* a, Type* b)
{
	if (a->typeKind != b->typeKind)
		return false;

	switch (a->typeKind) {
	case TYPE_STRUCT:
		if (a->struct_.name)
			return a->struct_.name == b->struct_.name;
		for (int i = 0; i < a->struct_.numFields; i++)
		{
			if (a->struct_.fieldTypes[i] != b->struct_.fieldTypes[i])
				return false;
		}
		return true;
	case TYPE_UNION:
		if (a->union_.name)
			return a->union_.name == b->union_.name;
		for (int i = 0; i < a->union_.numFields; i++)
		{
			if (a->union_.fieldTypes[i] != b->union_.fieldTypes[i])
				return false;
		}
		return true;
	case TYPE_POINTER:
		return a->pointer.elementType == b->pointer.elementType;
	case TYPE_OPTIONAL:
		return a->optional.elementType == b->optional.elementType;
	case TYPE_ARRAY:
		return a->array.elementType == b->array.elementType &&
			a->array.size == b->array.size;
	case TYPE_FUNCTION:
		if (a->function.returnType != b->function.returnType)
			return false;
		if (a->function.numParams != b->function.numParams)
			return false;
		for (int i = 0; i < a->function.numParams; i++)
		{
			if (a->function.paramTypes[i] != b->function.paramTypes[i])
				return false;
		}
		return true;
	default:
		return true;
	}
}

static void growTypeTable(TypeTable* table)
{
	int newCapacity = table->capacity * 2;
	TypeEntry* newEntries = (TypeEntry*)calloc(newCapacity, sizeof(TypeEntry));

	for (int i = 0; i < table->capacity; i++)
	{
		if (table->entries[i].value)
		{
			int index = table->entries[i].key & (newCapacity - 1);
			while (newEntries[index].value)
			{
				index = (index + 1) & (newCapacity - 1);
			}
			newEntries[index] = table->entries[i];
		}
	}

	free(table->entries);
	table->entries = newEntries;
	table->capacity = newCapacity;
}

static Type* internType(TypeSystem* types, Type key)
{
	TypeTable* table = &types->typeTable;

	if (table->count * 4 >= table->capacity * 3)
	{
		growTypeTable(table);
	}

	uint64_t h = hashType(&key);
	int index = h & (table->capacity - 1);
	while (table->entries[index].value)
	{
		if (table->entries[index].key == h && compareTypes(table->entries[index].value, &key))
		{
			return table->entries[index].value;
		}
		index = (index + 1) & (table->capacity - 1);
	}

	Type* type = types->arena.alloc<Type>();
	*type = key;

	table->entries[index].key = h;
	table->entries[index].value = type;
	table->count++;

	return type;
}

void initTypeSystem(TypeSystem* types)
{
	initArena(&types->arena, 1024 * 1024);

	types->errorType = { .typeKind = TYPE_NULL };

	types->primitiveTypes[TYPE_VOID] = { .typeKind = TYPE_VOID };
	types->primitiveTypes[TYPE_INT8] = { .typeKind = TYPE_INT8 };
	types->primitiveTypes[TYPE_INT16] = { .typeKind = TYPE_INT16 };
	types->primitiveTypes[TYPE_INT32] = { .typeKind = TYPE_INT32 };
	types->primitiveTypes[TYPE_INT64] = { .typeKind = TYPE_INT64 };
	types->primitiveTypes[TYPE_UINT8] = { .typeKind = TYPE_UINT8 };
	types->primitiveTypes[TYPE_UINT16] = { .typeKind = TYPE_UINT16 };
	types->primitiveTypes[TYPE_UINT32] = { .typeKind = TYPE_UINT32 };
	types->primitiveTypes[TYPE_UINT64] = { .typeKind = TYPE_UINT64 };
	types->primitiveTypes[TYPE_FLOAT] = { .typeKind = TYPE_FLOAT };
	types->primitiveTypes[TYPE_DOUBLE] = { .typeKind = TYPE_DOUBLE };
	types->primitiveTypes[TYPE_BOOL] = { .typeKind = TYPE_BOOL };
	types->primitiveTypes[TYPE_ANY] = { .typeKind = TYPE_ANY };
	types->primitiveTypes[TYPE_STRING] = { .typeKind = TYPE_STRING };

	types->typeTable.entries = (TypeEntry*)malloc((types->typeTable.capacity = 64) * sizeof(TypeEntry*));
	types->typeTable.count = 0;
}

void destroyTypeSystem(TypeSystem* types)
{
	destroyTypeTable(&types->typeTable);
	destroyArena(&types->arena);
}

static Type** copyTypes(TypeSystem* types, int numTypes, Type** elements)
{
	Type** newElements = types->arena.alloc<Type*>(numTypes);
	memcpy(newElements, elements, numTypes * sizeof(Type*));
	return newElements;
}

Type* getPointerType(TypeSystem* types, Type* elementType)
{
	Type key = {};
	key.typeKind = TYPE_POINTER;
	key.pointer.elementType = elementType;

	return internType(types, key);
}

Type* getOptionalType(TypeSystem* types, Type* elementType)
{
	Type key = {};
	key.typeKind = TYPE_OPTIONAL;
	key.optional.elementType = elementType;

	return internType(types, key);
}

Type* getAnonymousStructType(TypeSystem* types, int numElements, Type** elementTypes)
{
	Type key = {};
	key.typeKind = TYPE_STRUCT;
	key.struct_.numFields = numElements;
	key.struct_.fieldTypes = elementTypes;

	Type* type = internType(types, key);

	if (type == &key)
		type->struct_.fieldTypes = copyTypes(types, numElements, elementTypes);

	return type;
}

Type* getNamedStructType(TypeSystem* types, StringView name, int numElements, Type** elementTypes)
{
	char tmpName[256];
	strncpy(tmpName, name.ptr, name.length);

	Type key = {};
	key.typeKind = TYPE_STRUCT;
	key.struct_.name = tmpName;
	key.struct_.numFields = numElements;
	key.struct_.fieldTypes = elementTypes;

	Type* type = internType(types, key);

	if (type == &key)
	{
		type->struct_.name = _strdup(tmpName);
		type->struct_.fieldTypes = copyTypes(types, numElements, elementTypes);
	}

	return type;
}

Type* getFunctionType(TypeSystem* types, Type* returnType, int numParams, Type** paramTypes)
{
	Type key = {};
	key.typeKind = TYPE_FUNCTION;
	key.function.returnType = returnType;
	key.function.numParams = numParams;
	key.function.paramTypes = paramTypes;

	Type* type = internType(types, key);

	if (type == &key)
		type->function.paramTypes = copyTypes(types, numParams, paramTypes);

	return type;
}

Type* getArrayType(TypeSystem* types, Type* elementType, uint64_t size)
{
	Type key = {};
	key.typeKind = TYPE_ARRAY;
	key.array.elementType = elementType;
	key.array.size = size;

	return internType(types, key);
}
