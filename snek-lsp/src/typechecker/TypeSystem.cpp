#include "TypeSystem.h"

#include "utils/Hash.h"

#include <stdarg.h>
#include <stdio.h>


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
		if (type->struct_.name.length)
			hash = (hash ^ (uintptr_t)type->struct_.name.ptr) * 1099511628211ULL;
		for (int i = 0; i < type->struct_.numFields; i++)
			hash = (hash ^ (uintptr_t)type->struct_.fieldTypes[i]) * 1099511628211ULL;
		break;
	case TYPE_UNION:
		if (type->union_.name.length)
			hash = (hash ^ (uintptr_t)type->union_.name.ptr) * 1099511628211ULL;
		for (int i = 0; i < type->union_.numFields; i++)
			hash = (hash ^ (uintptr_t)type->union_.fieldTypes[i]) * 1099511628211ULL;
		break;
	case TYPE_ENUM:
		hash = (hash ^ (uintptr_t)type->enum_.name.ptr) * 1099511628211ULL;
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
		if (a->struct_.name.length)
			return compareString(a->struct_.name, b->struct_.name);
		for (int i = 0; i < a->struct_.numFields; i++)
		{
			if (a->struct_.fieldTypes[i] != b->struct_.fieldTypes[i])
				return false;
		}
		return true;
	case TYPE_UNION:
		if (a->union_.name.length)
			return compareString(a->union_.name, b->union_.name);
		for (int i = 0; i < a->union_.numFields; i++)
		{
			if (a->union_.fieldTypes[i] != b->union_.fieldTypes[i])
				return false;
		}
		return true;
	case TYPE_ENUM:
		return compareString(a->enum_.name, b->enum_.name);
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

static Type* internType(TypeSystem* types, Type key, bool* newType)
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

	types->errorType = { .typeKind = TYPE_NULL, .name = CreateString("<error>") };

	types->primitiveTypes[TYPE_VOID] = { .typeKind = TYPE_VOID, .name = CreateString("void") };
	types->primitiveTypes[TYPE_INT8] = { .typeKind = TYPE_INT8, .name = CreateString("int8") };
	types->primitiveTypes[TYPE_INT16] = { .typeKind = TYPE_INT16, .name = CreateString("int16") };
	types->primitiveTypes[TYPE_INT32] = { .typeKind = TYPE_INT32, .name = CreateString("int32") };
	types->primitiveTypes[TYPE_INT64] = { .typeKind = TYPE_INT64, .name = CreateString("int64") };
	types->primitiveTypes[TYPE_UINT8] = { .typeKind = TYPE_UINT8, .name = CreateString("uint8") };
	types->primitiveTypes[TYPE_UINT16] = { .typeKind = TYPE_UINT16, .name = CreateString("uint16") };
	types->primitiveTypes[TYPE_UINT32] = { .typeKind = TYPE_UINT32, .name = CreateString("uint32") };
	types->primitiveTypes[TYPE_UINT64] = { .typeKind = TYPE_UINT64, .name = CreateString("uint64") };
	types->primitiveTypes[TYPE_FLOAT] = { .typeKind = TYPE_FLOAT, .name = CreateString("float") };
	types->primitiveTypes[TYPE_DOUBLE] = { .typeKind = TYPE_DOUBLE, .name = CreateString("double") };
	types->primitiveTypes[TYPE_BOOL] = { .typeKind = TYPE_BOOL, .name = CreateString("bool") };
	types->primitiveTypes[TYPE_ANY] = { .typeKind = TYPE_ANY, .name = CreateString("any") };
	types->primitiveTypes[TYPE_STRING] = { .typeKind = TYPE_STRING, .name = CreateString("string") };

	initTypeTable(&types->typeTable, 64);
}

void destroyTypeSystem(TypeSystem* types)
{
	destroyTypeTable(&types->typeTable);
	destroyArena(&types->arena);
}

static StringView createTypeString(TypeSystem* types, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	int length = vsnprintf(nullptr, 0, fmt, args);
	char* buffer = (char*)types->arena.alloc(length + 1);
	vsnprintf(buffer, length + 1, fmt, args);
	buffer[length] = 0;

	va_end(args);

	return CreateString(buffer, length);
}

static Type** copyTypes(TypeSystem* types, int numElements, Type** elements)
{
	Type** newElements = types->arena.alloc<Type*>(numElements);
	memcpy(newElements, elements, numElements * sizeof(Type*));
	return newElements;
}

static StringView* copyNames(TypeSystem* types, int numElements, StringView* elements)
{
	StringView* newElements = types->arena.alloc<StringView>(numElements);
	memcpy(newElements, elements, numElements * sizeof(StringView));
	return newElements;
}

Type* getPointerType(TypeSystem* types, Type* elementType)
{
	Type key = {};
	key.typeKind = TYPE_POINTER;
	key.pointer.elementType = elementType;

	bool newType;
	Type* type = internType(types, key, &newType);

	if (newType)
	{
		type->name = createTypeString(types, "%.*s*", elementType->name.length, elementType->name.ptr);
	}

	return type;
}

Type* getOptionalType(TypeSystem* types, Type* elementType)
{
	Type key = {};
	key.typeKind = TYPE_OPTIONAL;
	key.optional.elementType = elementType;

	bool newType;
	Type* type = internType(types, key, &newType);

	if (newType)
	{
		type->name = createTypeString(types, "%.*s?", elementType->name.length, elementType->name.ptr);
	}

	return type;
}

Type* getAnonymousStructType(TypeSystem* types, int numElements, Type** fieldTypes, StringView* fieldNames)
{
	Type key = {};
	key.typeKind = TYPE_STRUCT;
	key.struct_.numFields = numElements;
	key.struct_.fieldTypes = fieldTypes;
	key.struct_.fieldNames = fieldNames;

	bool newType;
	Type* type = internType(types, key, &newType);

	if (newType)
	{
		type->struct_.fieldTypes = copyTypes(types, numElements, fieldTypes);
		if (type->struct_.fieldNames)
			type->struct_.fieldNames = copyNames(types, numElements, fieldNames);

		type->name = createTypeString(types, "<struct>");
	}

	return type;
}

Type* getNamedStructType(TypeSystem* types, StringView name, int numElements, Type** fieldTypes, StringView* fieldNames)
{
	Type key = {};
	key.typeKind = TYPE_STRUCT;
	key.struct_.name = name;
	key.struct_.numFields = numElements;
	key.struct_.fieldTypes = fieldTypes;

	bool newType;
	Type* type = internType(types, key, &newType);

	if (newType)
	{
		type->struct_.name = copy(name);
		type->struct_.fieldTypes = copyTypes(types, numElements, fieldTypes);

		type->name = createTypeString(types, "%.*s", name.length, name.ptr);
	}

	return type;
}

Type* getAnonymousUnionType(TypeSystem* types, int numElements, Type** fieldTypes, StringView* fieldNames)
{
	Type key = {};
	key.typeKind = TYPE_UNION;
	key.union_.numFields = numElements;
	key.union_.fieldTypes = fieldTypes;
	key.union_.fieldNames = fieldNames;

	bool newType;
	Type* type = internType(types, key, &newType);

	if (newType)
	{
		type->union_.fieldTypes = copyTypes(types, numElements, fieldTypes);
		if (type->union_.fieldNames)
			type->union_.fieldNames = copyNames(types, numElements, fieldNames);

		type->name = createTypeString(types, "<union>");
	}

	return type;
}

Type* getNamedUnionType(TypeSystem* types, StringView name, int numElements, Type** fieldTypes, StringView* fieldNames)
{
	Type key = {};
	key.typeKind = TYPE_UNION;
	key.union_.name = name;
	key.union_.numFields = numElements;
	key.union_.fieldTypes = fieldTypes;

	bool newType;
	Type* type = internType(types, key, &newType);

	if (newType)
	{
		type->union_.name = copy(name);
		type->union_.fieldTypes = copyTypes(types, numElements, fieldTypes);

		type->name = createTypeString(types, "%.*s", name.length, name.ptr);
	}

	return type;
}

Type* getEnumType(TypeSystem* types, StringView name, Type* valueType)
{
	Type key = {};
	key.typeKind = TYPE_ENUM;
	key.enum_.name = name;
	key.enum_.valueType = valueType;

	bool newType;
	Type* type = internType(types, key, &newType);

	if (newType)
	{
		type->enum_.name = copy(name);

		type->name = createTypeString(types, "%.*s", name.length, name.ptr);
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

	bool newType;
	Type* type = internType(types, key, &newType);

	if (newType)
	{
		type->function.paramTypes = copyTypes(types, numParams, paramTypes);

		char buffer[256];
		buffer[0] = 0;
		strcat(buffer, "func(");
		for (int i = 0; i < numParams; i++)
		{
			strcat(buffer, paramTypes[i]->name.ptr);
			if (i < numParams - 1)
				strcat(buffer, ",");
		}
		strcat(buffer, ")");

		if (returnType)
		{
			strcat(buffer, "->");
			strcat(buffer, returnType->name.ptr);
		}

		type->name = createTypeString(types, "%s", buffer);
	}

	return type;
}

Type* getArrayType(TypeSystem* types, Type* elementType, uint64_t size)
{
	Type key = {};
	key.typeKind = TYPE_ARRAY;
	key.array.elementType = elementType;
	key.array.size = size;

	bool newType;
	Type* type = internType(types, key, &newType);

	if (newType)
	{
		if (size)
			type->name = createTypeString(types, "%.*s[%ull]", elementType->name.length, elementType->name.ptr, size);
		else
			type->name = createTypeString(types, "%.*s[]", elementType->name.length, elementType->name.ptr);
	}

	return type;
}
