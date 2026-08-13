#include "TypeSystem.h"

#include "File.h"
#include "utils/Arena.h"
#include "utils/Hash.h"
#include "utils/Log.h"

#include <stdarg.h>
#include <stdio.h>


void initTypeTable(TypeTable* table, Arena* arena, int initialCapacity)
{
	*table = {};

	table->arena = arena;
	table->entries = arena->alloc<TypeEntry>(initialCapacity);
	table->capacity = initialCapacity;
	table->count = 0;
}

bool isVoidType(Type* type)
{
	return type->typeKind == TYPE_VOID;
}

bool isIntegerType(Type* type)
{
	return type->typeKind > TYPE_INT_START && type->typeKind < TYPE_INT_END;
}

bool isUnsignedType(Type* type)
{
	return type->typeKind >= TYPE_UINT8 && type->typeKind < TYPE_INT_END;
}

bool isFloatingPointType(Type* type)
{
	return type->typeKind > TYPE_FLOAT_START && type->typeKind < TYPE_FLOAT_END;
}

bool isTruthyType(Type* type)
{
	return type->typeKind == TYPE_BOOL || isIntegerType(type) || isFloatingPointType(type) || type->typeKind == TYPE_POINTER || type->typeKind == TYPE_OPTIONAL;
}

bool isNumericType(Type* type)
{
	return isIntegerType(type) || isFloatingPointType(type);
}

bool isErrorType(Type* type)
{
	return type->typeKind == TYPE_NULL;
}

bool isCharPointerType(Type* type)
{
	return type->typeKind == TYPE_POINTER && type->pointer.elementType->typeKind == TYPE_INT8;
}

Type* getVoidType(TypeSystem* types)
{
	return &types->primitiveTypes[TYPE_VOID];
}

Type* getStringType(TypeSystem* types)
{
	return &types->primitiveTypes[TYPE_STRING];
}

Type* getErrorType(TypeSystem* types)
{
	return &types->errorType;
}

uint64_t hashType(Type* type)
{
	uint64_t h = 14695981039346656037ULL; // FNV offset basis

	h = (h ^ type->typeKind) * 1099511628211ULL;

	switch (type->typeKind)
	{
	case TYPE_STRUCT:
		if (type->struct_.name.length)
			h = (h ^ hash(type->struct_.name)) * 1099511628211ULL;
		for (int i = 0; i < type->struct_.numFields; i++)
			h = (h ^ type->struct_.fieldTypes[i]->hash) * 1099511628211ULL;
		break;
	case TYPE_UNION:
		if (type->union_.name.length)
			h = (h ^ hash(type->union_.name)) * 1099511628211ULL;
		for (int i = 0; i < type->union_.numFields; i++)
			h = (h ^ type->union_.fieldTypes[i]->hash) * 1099511628211ULL;
		break;
	case TYPE_ENUM:
		h = (h ^ hash(type->enum_.name)) * 1099511628211ULL;
		break;
	case TYPE_ALIAS:
		h = (h ^ hash(type->alias.name)) * 1099511628211ULL;
		break;
	case TYPE_POINTER:
		h = (h ^ type->pointer.elementType->hash) * 1099511628211ULL;
		break;
	case TYPE_OPTIONAL:
		h = (h ^ type->optional.elementType->hash) * 1099511628211ULL;
		break;
	case TYPE_ARRAY:
		h = (h ^ type->array.elementType->hash) * 1099511628211ULL;
		h = (h ^ type->array.size) * 1099511628211ULL;
		break;
	case TYPE_FUNCTION:
		if (type->function.returnType)
			h = (h ^ type->function.returnType->hash) * 1099511628211ULL;
		for (int i = 0; i < type->function.numParams; i++)
			h = (h ^ type->function.paramTypes[i]->hash) * 1099511628211ULL;
		h = (h ^ (uint64_t)type->function.variadic) * 1099511628211ULL;
		break;
	default:
		break;
	}

	return h;
}

bool compareTypes(Type* a, Type* b)
{
	if (a == b)
		return true;

	if (a->typeKind != b->typeKind)
		return false;

	if (a->hash != b->hash)
		return false;

	switch (a->typeKind) {
	case TYPE_STRUCT:
		if (a->struct_.name.length)
			return compareString(a->struct_.name, b->struct_.name);
		for (int i = 0; i < a->struct_.numFields; i++)
		{
			if (!compareTypes(a->struct_.fieldTypes[i], b->struct_.fieldTypes[i]))
				return false;
		}
		return true;
	case TYPE_UNION:
		if (a->union_.name.length)
			return compareString(a->union_.name, b->union_.name);
		for (int i = 0; i < a->union_.numFields; i++)
		{
			if (!compareTypes(a->union_.fieldTypes[i], b->union_.fieldTypes[i]))
				return false;
		}
		return true;
	case TYPE_ENUM:
		return compareString(a->enum_.name, b->enum_.name);
	case TYPE_POINTER:
		return compareTypes(a->pointer.elementType, b->pointer.elementType);
	case TYPE_OPTIONAL:
		return compareTypes(a->optional.elementType, b->optional.elementType);
	case TYPE_ARRAY:
		return compareTypes(a->array.elementType, b->array.elementType) &&
			a->array.size == b->array.size;
	case TYPE_FUNCTION:
		if (!compareTypes(a->function.returnType, b->function.returnType))
			return false;
		if (a->function.numParams != b->function.numParams)
			return false;
		for (int i = 0; i < a->function.numParams; i++)
		{
			if (compareTypes(a->function.paramTypes[i], b->function.paramTypes[i]))
				return false;
		}
		if (a->function.variadic != b->function.variadic)
			return false;
		return true;
	default:
		return true;
	}
}

static void growTypeTable(TypeTable* table)
{
	int newCapacity = table->capacity == 0 ? 16 : table->capacity * 2;
	TypeEntry* newEntries = (TypeEntry*)table->arena->alloc<TypeEntry>(newCapacity);
	int newMask = newCapacity - 1;

	for (int i = 0; i < table->capacity; i++)
	{
		if (table->entries[i].value)
		{
			int index = table->entries[i].key & newMask;
			while (newEntries[index].value)
			{
				index = (index + 1) & newMask;
			}
			newEntries[index] = table->entries[i];
		}
	}

	//free(table->entries);
	table->entries = newEntries;
	table->capacity = newCapacity;
}

static Type* getType(TypeTable* table, uint64_t hash, Type key)
{
	int mask = table->capacity - 1;
	int index = hash & mask;

	while (table->entries[index].value)
	{
		if (table->entries[index].key == hash && compareTypes(table->entries[index].value, &key))
		{
			return table->entries[index].value;
		}
		index = (index + 1) & mask;
	}

	return nullptr;
}

static Type* internType(TypeTable* table, Type key, Arena* arena, bool* newType)
{
	uint64_t h = hashType(&key);

	if (Type* type = getType(table, h, key))
	{
		*newType = false;
		return type;
	}

	if (table->count * 4 >= table->capacity * 3)
		growTypeTable(table);

	Type* type = arena->alloc<Type>();
	*type = key;

	type->hash = h;

	int mask = table->capacity - 1;
	int index = h & mask;

	table->entries[index].key = h;
	table->entries[index].value = type;
	table->count++;

	*newType = true;
	return type;
}

static bool removeInternedType(TypeTable* table, uint64_t key, Type* type)
{
	if (table->capacity == 0)
		return false;

	uint64_t h = key;
	int mask = table->capacity - 1;
	int index = h & mask;

	while (table->entries[index].value)
	{
		if (table->entries[index].key == h && compareTypes(table->entries[index].value, type))
		{
			table->count--;

			*type = {};

			int j = index;
			int next = (index + 1) & mask;

			while (table->entries[next].value)
			{
				int r = table->entries[next].key & mask;

				if ((j <= next) ? (r <= j || r > next) : (r <= j && r > next))
				{
					table->entries[j] = table->entries[next];
					j = next;
				}
				next = (next + 1) & mask;
			}

			table->entries[j].key = 0;
			table->entries[j].value = nullptr;
			return true;
		}
		index = (index + 1) & mask;
	}
	return false;
}

void initTypeSystem(TypeSystem* types, Arena* globalArena)
{
	types->errorType = { .typeKind = TYPE_NULL, .name = CreateString("<error>"), .mangledName = CreateString("error") };

	types->primitiveTypes[TYPE_VOID] = { .typeKind = TYPE_VOID, .name = CreateString("void"), .mangledName = CreateString("void") };
	types->primitiveTypes[TYPE_INT8] = { .typeKind = TYPE_INT8, .name = CreateString("int8"), .mangledName = CreateString("i8") };
	types->primitiveTypes[TYPE_INT16] = { .typeKind = TYPE_INT16, .name = CreateString("int16"), .mangledName = CreateString("i16") };
	types->primitiveTypes[TYPE_INT32] = { .typeKind = TYPE_INT32, .name = CreateString("int32"), .mangledName = CreateString("i32") };
	types->primitiveTypes[TYPE_INT64] = { .typeKind = TYPE_INT64, .name = CreateString("int64"), .mangledName = CreateString("i64") };
	types->primitiveTypes[TYPE_UINT8] = { .typeKind = TYPE_UINT8, .name = CreateString("uint8"), .mangledName = CreateString("u8") };
	types->primitiveTypes[TYPE_UINT16] = { .typeKind = TYPE_UINT16, .name = CreateString("uint16"), .mangledName = CreateString("u16") };
	types->primitiveTypes[TYPE_UINT32] = { .typeKind = TYPE_UINT32, .name = CreateString("uint32"), .mangledName = CreateString("u32") };
	types->primitiveTypes[TYPE_UINT64] = { .typeKind = TYPE_UINT64, .name = CreateString("uint64"), .mangledName = CreateString("u64") };
	types->primitiveTypes[TYPE_FLOAT] = { .typeKind = TYPE_FLOAT, .name = CreateString("float"), .mangledName = CreateString("float") };
	types->primitiveTypes[TYPE_DOUBLE] = { .typeKind = TYPE_DOUBLE, .name = CreateString("double"), .mangledName = CreateString("double") };
	types->primitiveTypes[TYPE_BOOL] = { .typeKind = TYPE_BOOL, .name = CreateString("bool"), .mangledName = CreateString("bool") };
	types->primitiveTypes[TYPE_ANY] = { .typeKind = TYPE_ANY, .name = CreateString("any"), .mangledName = CreateString("any") };
	types->primitiveTypes[TYPE_STRING] = { .typeKind = TYPE_STRING, .name = CreateString("string"), .mangledName = CreateString("string") };
	types->primitiveTypes[TYPE_TYPE] = { .typeKind = TYPE_TYPE, .name = CreateString("type"), .mangledName = CreateString("type") };

	types->primitiveTypes[TYPE_VOID].hash = hashType(&types->primitiveTypes[TYPE_VOID]);
	types->primitiveTypes[TYPE_INT8].hash = hashType(&types->primitiveTypes[TYPE_INT8]);
	types->primitiveTypes[TYPE_INT16].hash = hashType(&types->primitiveTypes[TYPE_INT16]);
	types->primitiveTypes[TYPE_INT32].hash = hashType(&types->primitiveTypes[TYPE_INT32]);
	types->primitiveTypes[TYPE_INT64].hash = hashType(&types->primitiveTypes[TYPE_INT64]);
	types->primitiveTypes[TYPE_UINT8].hash = hashType(&types->primitiveTypes[TYPE_UINT8]);
	types->primitiveTypes[TYPE_UINT16].hash = hashType(&types->primitiveTypes[TYPE_UINT16]);
	types->primitiveTypes[TYPE_UINT32].hash = hashType(&types->primitiveTypes[TYPE_UINT32]);
	types->primitiveTypes[TYPE_UINT64].hash = hashType(&types->primitiveTypes[TYPE_UINT64]);
	types->primitiveTypes[TYPE_FLOAT].hash = hashType(&types->primitiveTypes[TYPE_FLOAT]);
	types->primitiveTypes[TYPE_DOUBLE].hash = hashType(&types->primitiveTypes[TYPE_DOUBLE]);
	types->primitiveTypes[TYPE_BOOL].hash = hashType(&types->primitiveTypes[TYPE_BOOL]);
	types->primitiveTypes[TYPE_ANY].hash = hashType(&types->primitiveTypes[TYPE_ANY]);
	types->primitiveTypes[TYPE_STRING].hash = hashType(&types->primitiveTypes[TYPE_STRING]);
	types->primitiveTypes[TYPE_TYPE].hash = hashType(&types->primitiveTypes[TYPE_TYPE]);

	types->arena = globalArena;
	initTypeTable(&types->typeTable, globalArena, 64);
}

void destroyTypeSystem(TypeSystem* types)
{
}

static StringView createTypeString(Arena* arena, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	int length = vsnprintf(nullptr, 0, fmt, args);
	char* buffer = (char*)arena->alloc(length + 1);
	vsnprintf(buffer, length + 1, fmt, args);
	buffer[length] = 0;

	va_end(args);

	return CreateString(buffer, length);
}

static Type** copyTypes(Arena* arena, int numElements, Type** elements)
{
	Type** newElements = (Type**)arena->alloc<Type*>(numElements);
	memcpy(newElements, elements, numElements * sizeof(Type*));
	return newElements;
}

static StringView* copyNames(Arena* arena, int numElements, StringView* elements)
{
	StringView* newElements = (StringView*)arena->alloc<StringView>(numElements);
	memcpy(newElements, elements, numElements * sizeof(StringView));
	return newElements;
}

static bool isPrimitiveDerivative(Type* type)
{
	if (type->typeKind == TYPE_STRUCT)
	{
		if (type->struct_.name.length)
			return false;
		for (int i = 0; i < type->struct_.numFields; i++)
		{
			if (!isPrimitiveDerivative(type->struct_.fieldTypes[i]))
				return false;
		}
		return true;
	}
	else if (type->typeKind == TYPE_UNION)
	{
		if (type->struct_.name.length)
			return false;
		for (int i = 0; i < type->struct_.numFields; i++)
		{
			if (!isPrimitiveDerivative(type->struct_.fieldTypes[i]))
				return false;
		}
		return true;
	}
	else if (type->typeKind == TYPE_ENUM)
	{
		return false;
	}
	else if (type->typeKind == TYPE_ALIAS)
	{
		return false;
	}
	else if (type->typeKind == TYPE_POINTER)
	{
		return isPrimitiveDerivative(type->pointer.elementType);
	}
	else if (type->typeKind == TYPE_OPTIONAL)
	{
		return isPrimitiveDerivative(type->optional.elementType);
	}
	else if (type->typeKind == TYPE_FUNCTION)
	{
		if (type->function.returnType && !isPrimitiveDerivative(type->function.returnType))
			return false;
		for (int i = 0; i < type->function.numParams; i++)
		{
			if (!isPrimitiveDerivative(type->function.paramTypes[i]))
				return false;
		}
		return true;
	}
	else if (type->typeKind == TYPE_ARRAY)
	{
		return isPrimitiveDerivative(type->array.elementType);
	}
	else
	{
		return true;
	}
}

Type* getPointerType(TypeSystem* types, Type* elementType, File* file)
{
	Type key = {};
	key.typeKind = TYPE_POINTER;
	key.pointer.elementType = elementType;

	bool primitive = isPrimitiveDerivative(&key);
	TypeTable* typeTable = primitive ? &types->typeTable : &file->typeTable;
	Arena* arena = primitive ? types->arena : &file->arena;

	bool newType;
	Type* type = internType(typeTable, key, arena, &newType);

	if (newType)
	{
		type->name = createTypeString(arena, "%.*s*", elementType->name.length, elementType->name.ptr);
		type->mangledName = createTypeString(arena, "ptr_%.*s", elementType->mangledName.length, elementType->mangledName.ptr);
	}

	return type;
}

Type* getOptionalType(TypeSystem* types, Type* elementType, File* file)
{
	Type key = {};
	key.typeKind = TYPE_OPTIONAL;
	key.optional.elementType = elementType;

	bool primitive = isPrimitiveDerivative(&key);
	TypeTable* typeTable = primitive ? &types->typeTable : &file->typeTable;
	Arena* arena = primitive ? types->arena : &file->arena;

	bool newType;
	Type* type = internType(typeTable, key, arena, &newType);

	if (newType)
	{
		type->name = createTypeString(arena, "%.*s?", elementType->name.length, elementType->name.ptr);
		type->mangledName = createTypeString(arena, "opt_%.*s", elementType->mangledName.length, elementType->mangledName.ptr);
	}

	return type;
}

Type* getAnonymousStructType(TypeSystem* types, int numElements, Type** fieldTypes, StringView* fieldNames, File* file)
{
	Type key = {};
	key.typeKind = TYPE_STRUCT;
	key.struct_.numFields = numElements;
	key.struct_.fieldTypes = fieldTypes;
	key.struct_.fieldNames = fieldNames;

	bool primitive = isPrimitiveDerivative(&key);
	TypeTable* typeTable = primitive ? &types->typeTable : &file->typeTable;
	Arena* arena = primitive ? types->arena : &file->arena;

	bool newType;
	Type* type = internType(typeTable, key, arena, &newType);

	if (newType)
	{
		type->struct_.fieldTypes = copyTypes(arena, numElements, fieldTypes);
		if (type->struct_.fieldNames)
			type->struct_.fieldNames = copyNames(arena, numElements, fieldNames);

		type->name = createTypeString(arena, "<struct>");

		char elementTypes[256] = "";
		for (int i = 0; i < type->struct_.numFields; i++)
		{
			Type* fieldType = type->struct_.fieldTypes[i];
			strncat(elementTypes, fieldType->mangledName.ptr, fieldType->mangledName.length);
			if (i < type->struct_.numFields - 1)
				strcat(elementTypes, "_");
		}
		type->mangledName = createTypeString(arena, "struct_%s", elementTypes);
	}

	return type;
}

Type* getAnonymousUnionType(TypeSystem* types, int numElements, Type** fieldTypes, StringView* fieldNames, File* file)
{
	Type key = {};
	key.typeKind = TYPE_UNION;
	key.union_.numFields = numElements;
	key.union_.fieldTypes = fieldTypes;
	key.union_.fieldNames = fieldNames;

	bool primitive = isPrimitiveDerivative(&key);
	TypeTable* typeTable = primitive ? &types->typeTable : &file->typeTable;
	Arena* arena = primitive ? types->arena : &file->arena;

	bool newType;
	Type* type = internType(typeTable, key, arena, &newType);

	if (newType)
	{
		type->union_.fieldTypes = copyTypes(arena, numElements, fieldTypes);
		if (type->union_.fieldNames)
			type->union_.fieldNames = copyNames(arena, numElements, fieldNames);

		type->name = createTypeString(arena, "<union>");

		char elementTypes[256] = "";
		for (int i = 0; i < type->union_.numFields; i++)
		{
			Type* fieldType = type->union_.fieldTypes[i];
			strncat(elementTypes, fieldType->mangledName.ptr, fieldType->mangledName.length);
			if (i < type->union_.numFields - 1)
				strcat(elementTypes, "_");
		}
		type->mangledName = createTypeString(arena, "union_%s", elementTypes);
	}

	return type;
}

Type* getFunctionType(TypeSystem* types, Type* returnType, int numParams, Type** paramTypes, bool variadic, File* file)
{
	Type key = {};
	key.typeKind = TYPE_FUNCTION;
	key.function.returnType = returnType;
	key.function.numParams = numParams;
	key.function.paramTypes = paramTypes;
	key.function.variadic = variadic;

	bool primitive = isPrimitiveDerivative(&key);
	TypeTable* typeTable = primitive ? &types->typeTable : &file->typeTable;
	Arena* arena = primitive ? types->arena : &file->arena;

	bool newType;
	Type* type = internType(typeTable, key, arena, &newType);

	if (newType)
	{
		type->function.paramTypes = copyTypes(arena, numParams, paramTypes);

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

		type->name = createTypeString(arena, "%s", buffer);

		char paramTypes[256] = "";
		for (int i = 0; i < type->function.numParams; i++)
		{
			Type* paramType = type->function.paramTypes[i];
			strncat(paramTypes, paramType->mangledName.ptr, paramType->mangledName.length);
			if (i < type->function.numParams - 1)
				strcat(paramTypes, "_");
		}

		StringView returnTypeStr = type->function.returnType ? type->function.returnType->mangledName : types->primitiveTypes[TYPE_VOID].mangledName;

		type->mangledName = createTypeString(arena, "func_%d_%s_%.*s", type->function.numParams, paramTypes, returnTypeStr.length, returnTypeStr.ptr);
	}

	return type;
}

Type* getArrayType(TypeSystem* types, Type* elementType, uint64_t size, File* file)
{
	Type key = {};
	key.typeKind = TYPE_ARRAY;
	key.array.elementType = elementType;
	key.array.size = size;

	bool primitive = isPrimitiveDerivative(&key);
	TypeTable* typeTable = primitive ? &types->typeTable : &file->typeTable;
	Arena* arena = primitive ? types->arena : &file->arena;

	bool newType;
	Type* type = internType(typeTable, key, arena, &newType);

	if (newType)
	{
		if (size)
			type->name = createTypeString(arena, "%.*s[%llu]", elementType->name.length, elementType->name.ptr, size);
		else
			type->name = createTypeString(arena, "%.*s[]", elementType->name.length, elementType->name.ptr);

		type->mangledName = createTypeString(arena, "arr_%llu_%.*s", type->array.size, elementType->mangledName.length, elementType->mangledName.ptr);
	}

	return type;
}

Type* createNamedStructType(File* file, StringView name, Struct* declaration)
{
	Type key = {};
	key.typeKind = TYPE_STRUCT;
	key.struct_.name = name;

	TypeTable* typeTable = &file->typeTable;
	Arena* arena = &file->arena;

	bool newType;
	Type* type = internType(typeTable, key, arena, &newType);

	SnekAssert(newType);

	type->struct_.name = copy(name);
	type->struct_.declaration = declaration;
	type->name = createTypeString(arena, "%.*s", name.length, name.ptr);

	type->mangledName = createTypeString(arena, "%.*s", name.length, name.ptr);

	return type;
}

void resolveNamedStructType(Type* type, int numFields, Type** fieldTypes, StringView* fieldNames, File* file)
{
	type->struct_.numFields = numFields;
	type->struct_.fieldTypes = copyTypes(&file->arena, numFields, fieldTypes);
	type->struct_.fieldNames = copyNames(&file->arena, numFields, fieldNames);
}

Type* createNamedUnionType(File* file, StringView name, Union* declaration)
{
	Type key = {};
	key.typeKind = TYPE_UNION;
	key.union_.name = name;

	TypeTable* typeTable = &file->typeTable;
	Arena* arena = &file->arena;

	bool newType;
	Type* type = internType(typeTable, key, arena, &newType);

	SnekAssert(newType);

	type->union_.name = copy(name);
	type->union_.declaration = declaration;
	type->name = createTypeString(arena, "%.*s", name.length, name.ptr);

	type->mangledName = createTypeString(arena, "%.*s", name.length, name.ptr);

	return type;
}

void resolveNamedUnionType(Type* type, int numFields, Type** fieldTypes, StringView* fieldNames, File* file)
{
	type->union_.numFields = numFields;
	type->union_.fieldTypes = copyTypes(&file->arena, numFields, fieldTypes);
	type->union_.fieldNames = copyNames(&file->arena, numFields, fieldNames);
}

Type* createEnumType(File* file, StringView name, Enum* declaration)
{
	Type key = {};
	key.typeKind = TYPE_ENUM;
	key.enum_.name = name;

	TypeTable* typeTable = &file->typeTable;
	Arena* arena = &file->arena;

	bool newType;
	Type* type = internType(typeTable, key, arena, &newType);

	SnekAssert(newType);

	type->enum_.name = copy(name);
	type->enum_.declaration = declaration;
	type->name = createTypeString(arena, "%.*s", name.length, name.ptr);
	type->mangledName = createTypeString(arena, "%.*s", name.length, name.ptr);

	return type;
}

void resolveEnumType(Type* type, Type* valueType)
{
	type->enum_.valueType = valueType;
}

Type* createAliasType(File* file, StringView name, Typedef* declaration)
{
	Type key = {};
	key.typeKind = TYPE_ALIAS;
	key.alias.name = name;

	TypeTable* typeTable = &file->typeTable;
	Arena* arena = &file->arena;

	bool newType;
	Type* type = internType(typeTable, key, arena, &newType);

	SnekAssert(newType);

	type->alias.name = copy(name);
	type->alias.declaration = declaration;
	type->name = createTypeString(arena, "%.*s", name.length, name.ptr);
	type->mangledName = createTypeString(arena, "%.*s", name.length, name.ptr);

	return type;
}

void resolveAliasType(Type* type, Type* value)
{
	type->alias.valueType = value;
}

static void appendTypeCharacter(char* buffer, Type* type)
{
	switch (type->typeKind)
	{
	case TYPE_VOID:
		strcat(buffer, "v");
		break;
	case TYPE_INT8:
		strcat(buffer, "c");
		break;
	case TYPE_INT16:
		strcat(buffer, "sh");
		break;
	case TYPE_INT32:
		strcat(buffer, "i");
		break;
	case TYPE_INT64:
		strcat(buffer, "l");
		break;
	case TYPE_UINT8:
		strcat(buffer, "uc");
		break;
	case TYPE_UINT16:
		strcat(buffer, "us");
		break;
	case TYPE_UINT32:
		strcat(buffer, "ui");
		break;
	case TYPE_UINT64:
		strcat(buffer, "ul");
		break;
	case TYPE_FLOAT:
		strcat(buffer, "f");
		break;
	case TYPE_DOUBLE:
		strcat(buffer, "d");
		break;
	case TYPE_BOOL:
		strcat(buffer, "b");
		break;
	case TYPE_ANY:
		strcat(buffer, "a");
		break;
	case TYPE_STRING:
		strcat(buffer, "st");
		break;
	case TYPE_STRUCT:
		strcat(buffer, "S");
		break;
	case TYPE_UNION:
		strcat(buffer, "U");
		break;
	case TYPE_ENUM:
		strcat(buffer, "E");
		break;
	case TYPE_ALIAS:
		strcat(buffer, "t");
		break;
	case TYPE_POINTER:
		strcat(buffer, "p");
		break;
	case TYPE_OPTIONAL:
		strcat(buffer, "o");
		break;
	case TYPE_FUNCTION:
		strcat(buffer, "F");
		break;
	case TYPE_ARRAY:
		strcat(buffer, "A");
		break;
	case TYPE_TYPE:
		strcat(buffer, "T");
		break;
	default:
		break;
	}
}

StringView mangleFunctionName(TypeSystem* types, StringView name, Type* functionType, Arena* arena)
{
	char paramTypes[256] = "";
	for (int i = 0; i < functionType->function.numParams; i++)
	{
		Type* paramType = functionType->function.paramTypes[i];
		appendTypeCharacter(paramTypes, paramType);
	}

	StringView mangledName = createTypeString(arena, "_%.*s_%d%s", name.length, name.ptr, functionType->function.numParams, paramTypes);
	if (functionType->function.returnType)
	{
		strcat(mangledName.ptr, "_");
		appendTypeCharacter(mangledName.ptr, functionType->function.returnType);
	}

	return mangledName;
}
