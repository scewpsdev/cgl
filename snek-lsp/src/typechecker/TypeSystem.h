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

	union {
		struct {
			StringView name;
			int numFields;
			Type** fieldTypes;
			StringView* fieldNames;
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
	Type errorType;
	Type primitiveTypes[TYPE_COUNT];

	TypeTable typeTable;
};


void initTypeSystem(TypeSystem* types);
void destroyTypeSystem(TypeSystem* types);

bool removeInternedType(TypeTable* table, uint64_t key, Type* type);

bool isIntegerType(Type* type);
bool isFloatingPointType(Type* type);
bool isTruthyType(Type* type);
bool isNumericType(Type* type);

uint64_t hash(Type* type);

Type* getPointerType(TypeSystem* types, Type* elementType, File* file);
Type* getOptionalType(TypeSystem* types, Type* elementType, File* file);
Type* getAnonymousStructType(TypeSystem* types, int numElements, Type** fieldTypes, StringView* fieldNames, File* file);
Type* getAnonymousUnionType(TypeSystem* types, int numElements, Type** fieldTypes, StringView* fieldNames, File* file);
Type* getFunctionType(TypeSystem* types, Type* returnType, int numParams, Type** paramTypes, File* file);
Type* getArrayType(TypeSystem* types, Type* elementType, uint64_t size, File* file);

Type* createNamedStructType(TypeSystem* types, StringView name, File* file, Struct* declaration);
void resolveNamedStructType(TypeSystem* types, Type* type, int numFields, Type** fieldTypes, StringView* fieldNames);

Type* createNamedUnionType(TypeSystem* types, StringView name, File* file, Union* declaration);
void resolveNamedUnionType(TypeSystem* types, Type* type, int numFields, Type** fieldTypes, StringView* fieldNames);

Type* createEnumType(TypeSystem* types, StringView name, File* file, Enum* declaration);
void resolveEnumType(TypeSystem* types, Type* type, Type* valueType);

Type* createAliasType(TypeSystem* types, StringView name, File* file, Typedef* declaration);
void resolveAliasType(TypeSystem* types, Type* type, Type* value);
