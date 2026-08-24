#include "TypeChecker.h"

#include "TypeSystem.h"
#include "File.h"

#include "parser/AST.h"
#include "parser/Diagnostics.h"
#include "parser/Lexer.h"

#include "utils/Arena.h"
#include "utils/ScratchBuffer.h"

#include <stdarg.h>
#include <math.h>
#include <ctype.h>


void initTypeChecker(TypeChecker* tc, Arena* arena, ScratchBuffer* scratch, Diagnostics* diagnostics, TypeSystem* types)
{
	*tc = {};

	tc->arena = arena;
	tc->scratch = scratch;
	tc->diagnostics = diagnostics;
	tc->types = types;
}

void destroyTypeChecker(TypeChecker* tc)
{
}

static void getSourceLocation(TypeChecker* tc, Node* node, SourceLocation* start, SourceLocation* end)
{
	*start = getSourceLocation(tc->file, node->start);
	*end = getSourceLocation(tc->file, node->end);
}

static void getSourceLocation(TypeChecker* tc, StringView str, SourceLocation* start, SourceLocation* end)
{
	*start = getSourceLocation(tc->file, (int)(str.ptr - tc->file->src));
	*end = getSourceLocation(tc->file, (int)(str.ptr - tc->file->src) + str.length);
}

static void error(TypeChecker* tc, Node* node, const char* fmt, ...)
{
	if (!tc->diagnostics) return;

	SnekAssert(node->start >= 0 && node->end >= node->start);

	SourceLocation start, end;
	getSourceLocation(tc, node, &start, &end);

	va_list args;
	va_start(args, fmt);

	int length = vsnprintf(nullptr, 0, fmt, args);
	char* msg = (char*)tc->arena->alloc(length + 1);
	vsnprintf(msg, length + 1, fmt, args);

	va_end(args);

	logMessage(tc->diagnostics, msg, start.line, start.col, end.line, end.col, DIAGNOSTICS_ERROR);
}

static void error(TypeChecker* tc, StringView str, const char* fmt, ...)
{
	if (!tc->diagnostics) return;

	SourceLocation start, end;
	getSourceLocation(tc, str, &start, &end);

	va_list args;
	va_start(args, fmt);

	int length = vsnprintf(nullptr, 0, fmt, args);
	char* msg = (char*)tc->arena->alloc(length + 1);
	vsnprintf(msg, length + 1, fmt, args);

	va_end(args);

	logMessage(tc->diagnostics, msg, start.line, start.col, end.line, end.col, DIAGNOSTICS_ERROR);
}

static Scope* pushScope(TypeChecker* tc, Scope* scope = nullptr)
{
	if (!scope)
	{
		scope = tc->arena->alloc<Scope>();
		scope->parent = tc->currentScope;

		bool isGlobal = false;
		initSymbolTable(&scope->symbols, isGlobal ? 1024 : 16, tc->arena);
	}

	SnekAssert(scope->parent != scope);
	tc->currentScope = scope;

	return scope;
}

static void popScope(TypeChecker* tc)
{
	tc->currentScope = tc->currentScope->parent;
}

void symbolCollection(TypeChecker* tc, File* file)
{
	tc->file = file;

	AST* ast = &file->ast;

	ast->globalScope = pushScope(tc);

	ast->structs = tc->arena->alloc<Struct*>(ast->numStructs);
	ast->enums = tc->arena->alloc<Enum*>(ast->numEnums);
	ast->unions = tc->arena->alloc<Union*>(ast->numUnions);
	ast->typedefs = tc->arena->alloc<Typedef*>(ast->numTypedefs);
	ast->functions = tc->arena->alloc<Function*>(ast->numFunctions);
	ast->macros = tc->arena->alloc<Macro*>(ast->numMacros);
	ast->globalVariables = tc->arena->alloc<GlobalVariable*>(ast->numGlobalVariables);
	ast->imports = tc->arena->alloc<Import*>(ast->numImports);

	int numStructs = 0;
	int numEnums = 0;
	int numUnions = 0;
	int numTypedefs = 0;
	int numFunctions = 0;
	int numMacros = 0;
	int numGlobalVariables = 0;
	int numImports = 0;

	for (int i = 0; i < ast->numDeclarations; i++)
	{
		Node* declaration = ast->declarations[i];

		StringView identifier = {};
		if (declaration->type == NODE_STRUCT)
		{
			Struct* struct_ = &declaration->struct_;

			if (struct_->name.length)
			{
				struct_->structType = createNamedStructType(file, struct_->name, struct_);
				struct_->symbol = insertSymbol(&tc->currentScope->symbols, struct_->name, SYMBOL_TYPE, declaration, file->handle);
			}

			ast->structs[numStructs++] = struct_;
		}
		else if (declaration->type == NODE_UNION)
		{
			Union* union_ = &declaration->union_;

			union_->unionType = createNamedUnionType(file, union_->name, union_);
			union_->symbol = insertSymbol(&tc->currentScope->symbols, union_->name, SYMBOL_TYPE, declaration, file->handle);

			ast->unions[numUnions++] = union_;
		}
		else if (declaration->type == NODE_ENUM)
		{
			Enum* enum_ = &declaration->enum_;

			enum_->enumType = createEnumType(file, enum_->name, enum_);
			enum_->symbol = insertSymbol(&tc->currentScope->symbols, enum_->name, SYMBOL_TYPE, declaration, file->handle);

			ast->enums[numEnums++] = enum_;
		}
		else if (declaration->type == NODE_TYPEDEF)
		{
			Typedef* typedef_ = &declaration->typedef_;

			typedef_->aliasType = createAliasType(file, typedef_->name, typedef_);
			typedef_->symbol = insertSymbol(&tc->currentScope->symbols, typedef_->name, SYMBOL_TYPE, declaration, file->handle);

			ast->typedefs[numTypedefs++] = typedef_;
		}
		else if (declaration->type == NODE_FUNCTION)
		{
			Function* function = &declaration->function;

			function->symbol = insertSymbol(&tc->currentScope->symbols, declaration->function.name, SYMBOL_FUNCTION_SET, declaration, file->handle);

			ast->functions[numFunctions++] = &declaration->function;
		}
		else if (declaration->type == NODE_GLOBAL_VARIABLE)
		{
			for (int i = 0; i < declaration->globalVariable.numDeclarators; i++)
			{
				declaration->globalVariable.declarators[i].symbol = insertSymbol(&tc->currentScope->symbols, declaration->globalVariable.declarators[i].name, SYMBOL_VARIABLE, declaration, file->handle);
			}
			ast->globalVariables[numGlobalVariables++] = &declaration->globalVariable;
		}
		else if (declaration->type == NODE_MACRO)
		{
			insertSymbol(&tc->currentScope->symbols, declaration->macro.name, SYMBOL_MACRO, declaration, file->handle);
			ast->macros[numMacros++] = &declaration->macro;
		}
		else if (declaration->type == NODE_IMPORT)
		{
			ast->imports[numImports++] = &declaration->import;
		}
	}

	SnekAssert(numStructs == ast->numStructs);
	SnekAssert(numEnums == ast->numEnums);
	SnekAssert(numUnions == ast->numUnions);
	SnekAssert(numTypedefs == ast->numTypedefs);
	SnekAssert(numFunctions == ast->numFunctions);
	SnekAssert(numMacros == ast->numMacros);
	SnekAssert(numGlobalVariables == ast->numGlobalVariables);
	SnekAssert(numImports == ast->numImports);

	popScope(tc);
}

static int charToDigit(char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return 10 + c - 'a';
	if (c >= 'A' && c <= 'F') return 10 + c - 'A';
	return -1;
}

static uint64_t stringToIntConstant(TypeChecker* tc, Node* node, StringView str, bool* negative, int* outBase, Type** type)
{
	*negative = false;

	int i = 0;
	int base = 10;

	if (str[0] == '-')
	{
		*negative = true;
		i++;
	}

	if (str.length >= i + 2 && str[i] == '0')
	{
		if (tolower(str[i + 1]) == 'x')
		{
			base = 16;
			i += 2;
		}
		else if (tolower(str[i + 1]) == 'b')
		{
			base = 2;
			i += 2;
		}
		else if (tolower(str[i + 1]) == 'o')
		{
			base = 8;
			i += 2;
		}
	}

	uint64_t value = 0;
	int digitCount = 0;

	for (; i < str.length; i++)
	{
		char c = tolower(str[i]);

		if (c == '_') continue;

		int digit = charToDigit(c);
		if (digit == -1 || digit >= base) break;

		digitCount++;

		if (*negative && value > (uint64_t)(INT64_MAX - digit) / base || value > (UINT64_MAX - digit) / base)
		{
			error(tc, node, "Integer overflow");
		}

		value = value * base + digit;
	}

	if (digitCount == 0)
	{
		error(tc, node, "Integer base prefix must be followed by atleast one digit");
	}

	if (str.length - i == 2 && strncmp(&str[i], "i8", 2) == 0) *type = &tc->types->primitiveTypes[TYPE_INT8];
	else if (str.length - i == 3 && strncmp(&str[i], "i16", 3) == 0) *type = &tc->types->primitiveTypes[TYPE_INT16];
	else if (str.length - i == 3 && strncmp(&str[i], "i32", 3) == 0) *type = &tc->types->primitiveTypes[TYPE_INT32];
	else if (str.length - i == 3 && strncmp(&str[i], "i64", 3) == 0) *type = &tc->types->primitiveTypes[TYPE_INT64];
	else if (str.length - i == 2 && strncmp(&str[i], "u8", 2) == 0) *type = &tc->types->primitiveTypes[TYPE_UINT8];
	else if (str.length - i == 3 && strncmp(&str[i], "u16", 3) == 0) *type = &tc->types->primitiveTypes[TYPE_UINT16];
	else if (str.length - i == 3 && strncmp(&str[i], "u32", 3) == 0) *type = &tc->types->primitiveTypes[TYPE_UINT32];
	else if (str.length - i == 3 && strncmp(&str[i], "u64", 3) == 0) *type = &tc->types->primitiveTypes[TYPE_UINT64];
	else if (str.length - i == 1 && tolower(str[i]) == 'u') *type = &tc->types->primitiveTypes[TYPE_UINT32];
	else if (str.length - i != 0)
	{
		error(tc, CreateString(str.ptr + i, str.length - i), "Undefined integer constant suffix '%.*s'", str.length - i, str.ptr + i);
	}

	if (!*type)
	{
		if (*negative)
		{
			if (value <= INT32_MIN + 1) *type = &tc->types->primitiveTypes[TYPE_INT32];
			else *type = &tc->types->primitiveTypes[TYPE_INT64];
		}
		else
		{
			if (base == 10)
			{
				if (value <= INT32_MAX) *type = &tc->types->primitiveTypes[TYPE_INT32];
				else *type = &tc->types->primitiveTypes[TYPE_INT64];
			}
			else
			{
				if (value <= INT32_MAX) *type = &tc->types->primitiveTypes[TYPE_INT32];
				else if (value <= UINT32_MAX) *type = &tc->types->primitiveTypes[TYPE_UINT32];
				else if (value <= INT64_MAX) *type = &tc->types->primitiveTypes[TYPE_INT64];
				else *type = &tc->types->primitiveTypes[TYPE_UINT64];
			}
		}
	}

	*outBase = base;

	if ((*type)->typeKind == TYPE_INT8 && value > INT8_MAX)
	{
		error(tc, node, "Integer literal too large for 8-bit target");
	}
	else if ((*type)->typeKind == TYPE_INT16 && value > INT16_MAX)
	{
		error(tc, node, "Integer literal too large for 16-bit target");
	}
	else if ((*type)->typeKind == TYPE_INT32 && value > INT32_MAX)
	{
		error(tc, node, "Integer literal too large for 32-bit target");
	}
	else if ((*type)->typeKind == TYPE_INT64 && value > INT64_MAX)
	{
		error(tc, node, "Integer literal too large for 64-bit target");
	}
	else if ((*type)->typeKind == TYPE_UINT8 && value > UINT8_MAX)
	{
		error(tc, node, "Integer literal too large for unsigned 8-bit target");
	}
	else if ((*type)->typeKind == TYPE_UINT16 && value > UINT16_MAX)
	{
		error(tc, node, "Integer literal too large for unsigned 16-bit target");
	}
	else if ((*type)->typeKind == TYPE_UINT32 && value > UINT32_MAX)
	{
		error(tc, node, "Integer literal too large for unsigned 32-bit target");
	}
	else if ((*type)->typeKind == TYPE_UINT64 && value > UINT64_MAX)
	{
		// should never be triggered, since integer overflow happens before that
		error(tc, node, "Integer literal too large for unsigned 64-bit target");
		SnekAssert(false);
	}

	return value;
}

static double stringToFloatConstant(TypeChecker* tc, Node* node, StringView str, Type** type)
{
	int length = str.length;
	if (str.length > 3 && strncmp(str.ptr + str.length - 3, "f32", 3) == 0)
	{
		*type = &tc->types->primitiveTypes[TYPE_FLOAT];
		length -= 3;
	}
	else if (str.length > 3 && strncmp(str.ptr + str.length - 3, "f64", 3) == 0)
	{
		*type = &tc->types->primitiveTypes[TYPE_DOUBLE];
		length -= 3;
	}
	else if (str.length > 1 && (str[str.length - 1] == 'f' || str[str.length - 1] == 'F'))
	{
		*type = &tc->types->primitiveTypes[TYPE_FLOAT];
		length -= 1;
	}

	if (!*type)
	{
		*type = &tc->types->primitiveTypes[TYPE_DOUBLE];
	}

	char buffer[256];
	int numDigits = 0;
	for (int i = 0; i < length; i++)
	{
		if (i >= 256)
		{
			error(tc, node, "Float literal exceeds maximum of 255 characters");
			return 0.0;
		}

		char c = str[i];
		if (c == '_') continue;
		buffer[numDigits++] = c;
	}
	buffer[numDigits] = 0;

	char* endPtr = nullptr;
	errno = 0;
	double value = strtod(buffer, &endPtr);

	if (endPtr != buffer + numDigits)
	{
		error(tc, node, "Invalid float literal syntax");
		return 0.0;
	}
	else if (errno == ERANGE)
	{
		error(tc, node, "Float literal overflow");
		return 0.0;
	}

	if (*type == &tc->types->primitiveTypes[TYPE_FLOAT])
	{
		float f = (float)value;
		if (isinf(f) && !isinf(value))
		{
			error(tc, node, "Float literal overflow for 32-bit target");
			return 0.0;
		}
	}

	return value;
}

static Type* resolveExpression(TypeChecker* tc, Expression* expression, Type* expectedType = nullptr);
static Type* resolveField(TypeChecker* tc, Field* field);
static Type* resolveParameter(TypeChecker* tc, Parameter* parameter);
static Symbol* resolveSymbol(TypeChecker* tc, StringView identifier);

static SymbolHandle getSymbolHandle(TypeChecker* tc, Symbol* symbol)
{
	SymbolHandle handle = {};
	handle.file = symbol->file;
	handle.symbol = symbol->key;
	return handle;
}

static Type* resolveType(TypeChecker* tc, TypeNode* type)
{
	if (type->type == NODE_ERROR_TYPE)
	{
		return type->inferredType = &tc->types->errorType;
	}
	else if (type->type == NODE_PRIMITIVE_TYPE)
	{
		type->inferredType = &tc->types->primitiveTypes[type->typeKind];
		SnekAssert(type->inferredType->typeKind == type->typeKind);
		return type->inferredType;
	}
	else if (type->type == NODE_NAMED_TYPE)
	{
		NamedType* namedType = (NamedType*)type;

		Symbol* symbol = resolveSymbol(tc, namedType->name);

		if (symbol && symbol->type == SYMBOL_TYPE)
		{
			if (symbol->file == tc->file->handle)
				namedType->symbol = symbol;
			namedType->symbolHandle = getSymbolHandle(tc, symbol);

			Node* node = symbol->declaration;
			if (node->type == NODE_STRUCT)
			{
				Struct* struct_ = &node->struct_;
				return type->inferredType = struct_->structType;
			}
			else if (node->type == NODE_UNION)
			{
				Union* union_ = &node->union_;
				return type->inferredType = union_->unionType;
			}
			else if (node->type == NODE_ENUM)
			{
				Enum* enum_ = &node->enum_;
				return type->inferredType = enum_->enumType;
			}
			else if (node->type == NODE_TYPEDEF)
			{
				Typedef* typedef_ = &node->typedef_;
				return type->inferredType = typedef_->aliasType;
			}
			else
			{
				SnekAssert(false);
			}
		}
		else
		{
			error(tc, (Node*)type, "Undefined typename '%.*s'", namedType->name.length, namedType->name.ptr);
			return type->inferredType = &tc->types->errorType;
		}
	}
	else if (type->type == NODE_STRUCT_TYPE)
	{
		StructType* structType = (StructType*)type;

		int mark = tc->scratch->mark();

		for (int i = 0; i < structType->numFields; i++)
		{
			if (structType->fields[i])
			{
				resolveField(tc, structType->fields[i]);
				for (int j = 0; j < structType->fields[i]->numDeclarators; j++)
					tc->scratch->add(structType->fields[i]->variableType->inferredType);
			}
			else
			{
				tc->scratch->add(getErrorType(tc->types));
			}
		}

		Type** fieldTypes = tc->scratch->getData<Type*>(mark);

		int mark2 = tc->scratch->mark();

		for (int i = 0; i < structType->numFields; i++)
		{
			if (structType->fields[i])
			{
				for (int j = 0; j < structType->fields[i]->numDeclarators; j++)
				{
					if (structType->fields[i]->declarators[j].name.length)
						tc->scratch->add(structType->fields[i]->declarators[j].name);
					else
						tc->scratch->add(getErrorType(tc->types));
				}
			}
			else
			{
				tc->scratch->add(getErrorType(tc->types));
			}
		}

		StringView* fieldNames = tc->scratch->getData<StringView>(mark2);

		type->inferredType = getAnonymousStructType(tc->types, structType->numFields, fieldTypes, fieldNames, tc->file);

		tc->scratch->release(mark);

		return type->inferredType;
	}
	else if (type->type == NODE_UNION_TYPE)
	{
		UnionType* unionType = (UnionType*)type;

		int mark = tc->scratch->mark();

		for (int i = 0; i < unionType->numFields; i++)
		{
			if (unionType->fields[i])
			{
				resolveField(tc, unionType->fields[i]);
				if (unionType->fields[i]->numDeclarators)
				{
					for (int j = 0; j < unionType->fields[i]->numDeclarators; j++)
						tc->scratch->add(unionType->fields[i]->variableType->inferredType);
				}
				else
				{
					tc->scratch->add(unionType->fields[i]->variableType->inferredType);
				}
			}
			else
			{
				tc->scratch->add(getErrorType(tc->types));
			}
		}

		Type** fieldTypes = tc->scratch->getData<Type*>(mark);

		int mark2 = tc->scratch->mark();

		for (int i = 0; i < unionType->numFields; i++)
		{
			if (unionType->fields[i])
			{
				if (unionType->fields[i]->numDeclarators)
				{
					for (int j = 0; j < unionType->fields[i]->numDeclarators; j++)
					{
						if (unionType->fields[i]->declarators[j].name.length)
							tc->scratch->add(unionType->fields[i]->declarators[j].name);
						else
							tc->scratch->add(CreateString(""));
					}
				}
				else
				{
					tc->scratch->add(CreateString(""));
				}
			}
			else
			{
				tc->scratch->add(CreateString(""));
			}
		}

		StringView* fieldNames = tc->scratch->getData<StringView>(mark2);

		type->inferredType = getAnonymousUnionType(tc->types, tc->scratch->count<StringView>(mark2), fieldTypes, fieldNames, tc->file);

		tc->scratch->release(mark);

		return type->inferredType;
	}
	else if (type->type == NODE_POINTER_TYPE)
	{
		PointerType* pointerType = (PointerType*)type;

		Type* elementType = nullptr;
		if (pointerType->elementType)
		{
			resolveType(tc, pointerType->elementType);
			elementType = pointerType->elementType->inferredType;
		}
		else
		{
			elementType = &tc->types->errorType;
		}

		return type->inferredType = getPointerType(tc->types, elementType, tc->file);
	}
	else if (type->type == NODE_OPTIONAL_TYPE)
	{
		OptionalType* optionalType = (OptionalType*)type;

		Type* elementType = nullptr;
		if (optionalType->elementType)
		{
			resolveType(tc, optionalType->elementType);
			elementType = optionalType->elementType->inferredType;
		}
		else
		{
			elementType = &tc->types->errorType;
		}

		return type->inferredType = getOptionalType(tc->types, elementType, tc->file);
	}
	else if (type->type == NODE_FUNCTION_TYPE)
	{
		FunctionType* functionType = (FunctionType*)type;

		bool variadic = false;

		int mark = tc->scratch->mark();

		for (int j = 0; j < functionType->numParams; j++)
		{
			if (functionType->params[j])
			{
				Type* paramType = resolveParameter(tc, functionType->params[j]);
				tc->scratch->add(paramType);

				if (functionType->params[j]->variadic)
				{
					if (j == functionType->numParams - 1)
						variadic = true;
					else
					{
						error(tc, (Node*)functionType->params[j], "Only the last parameter can be declared as variadic");
					}
				}
			}
		}

		Type* returnType = nullptr;
		if (functionType->returnType)
		{
			resolveType(tc, functionType->returnType);
			returnType = functionType->returnType->inferredType;
		}

		type->inferredType = getFunctionType(tc->types, returnType, functionType->numParams, tc->scratch->getData<Type*>(mark), variadic, tc->file);

		tc->scratch->release(mark);

		return type->inferredType;
	}
	else if (type->type == NODE_TUPLE_TYPE)
	{
		TupleType* tupleType = (TupleType*)type;

		int mark = tc->scratch->mark();

		for (int i = 0; i < tupleType->numElementTypes; i++)
		{
			if (tupleType->elementTypes[i])
			{
				resolveType(tc, tupleType->elementTypes[i]);
				tc->scratch->add(tupleType->elementTypes[i]->inferredType);
			}
			else
			{
				tc->scratch->add(getErrorType(tc->types));
			}
		}

		Type** elementTypes = tc->scratch->getData<Type*>(mark);
		type->inferredType = getAnonymousStructType(tc->types, tupleType->numElementTypes, elementTypes, nullptr, tc->file);

		tc->scratch->release(mark);

		return type->inferredType;
	}
	else if (type->type == NODE_ARRAY_TYPE)
	{
		ArrayType* arrayType = (ArrayType*)type;

		Type* elementType = nullptr;
		if (arrayType->elementType)
		{
			resolveType(tc, arrayType->elementType);
			elementType = arrayType->elementType->inferredType;
		}
		else
		{
			elementType = &tc->types->errorType;
		}

		int64_t size = 0;
		if (arrayType->size)
		{
			resolveExpression(tc, arrayType->size);

			if (isConstant(arrayType->size) && (isIntegerType(arrayType->size->inferredType) || arrayType->size->inferredType->typeKind == TYPE_ENUM && isIntegerType(arrayType->size->inferredType->enum_.valueType)) && constantFold(arrayType->size, &size))
			{
				if (size < 0)
				{
					error(tc, (Node*)arrayType->size, "Array size cannot be negative");
				}
			}
			else
			{
				error(tc, (Node*)arrayType->size, "Array size must be a constant expression");
			}
		}

		return type->inferredType = getArrayType(tc->types, elementType, size, tc->file);
	}

	SnekAssert(false);
	return type->inferredType = &tc->types->errorType;
}

static Symbol* resolveSymbol(TypeChecker* tc, StringView identifier)
{
	Scope* scope = tc->currentScope;
	SnekAssert(scope->parent != scope);
	while (scope)
	{
		if (Symbol* symbol = lookupSymbol(&scope->symbols, identifier))
		{
			return symbol;
		}
		scope = scope->parent;
	}

	for (int i = 0; i < tc->file->dependencies.size; i++)
	{
		FileHandle dependency = tc->file->dependencies[i];
		if (File* file = getFileFromHandle(dependency))
		{
			if (Symbol* symbol = lookupSymbol(&file->ast.globalScope->symbols, identifier))
			{
				return symbol;
			}
		}
	}

	return nullptr;
}

static int getNumericRank(Type* type)
{
	if (type->typeKind == TYPE_DOUBLE) return 100;
	if (type->typeKind == TYPE_FLOAT) return 90;
	if (type->typeKind == TYPE_UINT64) return 80;
	if (type->typeKind == TYPE_INT64) return 70;
	if (type->typeKind == TYPE_UINT32) return 60;
	if (type->typeKind == TYPE_INT32) return 50;
	if (type->typeKind == TYPE_UINT16) return 40;
	if (type->typeKind == TYPE_INT16) return 30;
	if (type->typeKind == TYPE_UINT8) return 20;
	if (type->typeKind == TYPE_INT8) return 10;
	return 0;
}

static Type* getCommonNumericType(Type* a, Type* b)
{
	if (a == b)
		return a;

	int rankA = getNumericRank(a);
	int rankB = getNumericRank(b);

	if (rankA == 0 || rankB == 0)
		return nullptr;

	return rankA > rankB ? a : b;
}

static void insertImplicitCast(TypeChecker* tc, Expression** node, Type* type)
{
	Expression* value = *node;

	Cast* cast = tc->arena->alloc<Cast>();
	initNode((Node*)cast, NODE_CAST, value->start);
	cast->expression = value;
	cast->targetType = nullptr;
	cast->inferredType = type;
	cast->implicit = true;
	cast->end = value->end;

	*node = cast;
}

static Type* typeCheckArithmeticOperator(TypeChecker* tc, uint8_t op, Type* left, Type* right, Expression* expression, Type* expectedType, Expression** leftNode, Expression** rightNode)
{
	if (op == OPERATOR_ADD || op == OPERATOR_SUBTRACT)
	{
		if (left->typeKind == TYPE_POINTER && isIntegerType(right))
		{
			insertImplicitCast(tc, rightNode, &tc->types->primitiveTypes[TYPE_INT64]);
			return left;
		}
		if (op == OPERATOR_SUBTRACT && left->typeKind == TYPE_POINTER && right->typeKind == TYPE_POINTER)
		{
			if (left != right)
			{
				error(tc, (Node*)expression, "Cannot subtract pointers of different types");
				return &tc->types->errorType;
			}
			return &tc->types->primitiveTypes[TYPE_INT64];
		}
	}

	Type* commonType = getCommonNumericType(left, right);
	if (!commonType)
	{
		error(tc, (Node*)expression, "Invalid operands to binary arithmetic operator");
		return &tc->types->errorType;
	}

	if (isFloatingPointType(commonType) && expectedType && isFloatingPointType(expectedType))
	{
		commonType = expectedType;
	}

	if (left != commonType)
	{
		if (leftNode)
			insertImplicitCast(tc, leftNode, commonType);
		else
		{
			error(tc, (Node*)expression, "Cannot assign value of type '%.*s' to variable of type '%.*s' without an explicit narrowing cast");
		}
	}
	if (right != commonType)
	{
		insertImplicitCast(tc, rightNode, commonType);
	}

	return commonType;
}

static Type* typeCheckComparisonOperator(TypeChecker* tc, BinaryOperator* expression, Type* left, Type* right)
{
	Type* commonType = nullptr;

	if (left == right)
		commonType = left;
	else if (left->typeKind == TYPE_POINTER && right->typeKind == TYPE_POINTER)
		commonType = left;
	else
	{
		commonType = getCommonNumericType(left, right);
	}

	if (!commonType)
	{
		error(tc, (Node*)expression, "Cannot compare incompatible types '%.*s' and '%.*s'", left->name.length, left->name.ptr, right->name.length, right->name.ptr);
		return &tc->types->errorType;
	}

	if (left != commonType)
		insertImplicitCast(tc, &expression->left, commonType);
	if (right != commonType)
		insertImplicitCast(tc, &expression->right, commonType);

	return &tc->types->primitiveTypes[TYPE_BOOL];
}

static Type* typeCheckBitwiseOperator(TypeChecker* tc, BinaryOperator* expression, Type* left, Type* right)
{
	if (!isIntegerType(left) || !isIntegerType(right))
	{
		error(tc, (Node*)(!isIntegerType(left) ? expression->left : expression->right), "Operand of bitwise operator must be of integer type");
		return &tc->types->errorType;
	}

	if (expression->op == OPERATOR_BITSHIFT_LEFT || expression->op == OPERATOR_BITSHIFT_RIGHT)
		return left;

	Type* commonType = getCommonNumericType(left, right);

	if (left != commonType)
		insertImplicitCast(tc, &expression->left, commonType);
	if (right != commonType)
		insertImplicitCast(tc, &expression->right, commonType);

	return commonType;
}

static Type* typeCheckLogicalOperator(TypeChecker* tc, BinaryOperator* expression, Type* left, Type* right)
{
	if (!isTruthyType(left) || !isTruthyType(right))
	{
		error(tc, (Node*)(!isTruthyType(left) ? expression->left : expression->right), "Operand of logical operator must be bool or scalar");
		return &tc->types->errorType;
	}

	if (left->typeKind != TYPE_BOOL)
		insertImplicitCast(tc, &expression->left, &tc->types->primitiveTypes[TYPE_BOOL]);
	if (right->typeKind != TYPE_BOOL)
		insertImplicitCast(tc, &expression->right, &tc->types->primitiveTypes[TYPE_BOOL]);

	return &tc->types->primitiveTypes[TYPE_BOOL];
}

static Type* unwrapType(Type* type)
{
	if (type->typeKind == TYPE_ENUM)
		return type->enum_.valueType;
	if (type->typeKind == TYPE_ALIAS)
		return type->alias.valueType;
	return type;
}

static bool isCastLegal(Type* expressionType, Type* targetType)
{
	while (expressionType->typeKind == TYPE_ALIAS || expressionType->typeKind == TYPE_ENUM)
		expressionType = unwrapType(expressionType);
	while (targetType->typeKind == TYPE_ALIAS || targetType->typeKind == TYPE_ENUM)
		targetType = unwrapType(targetType);

	if (compareTypes(expressionType, targetType))
		return true;

	if (isNumericType(expressionType) && isNumericType(targetType))
		return true;

	if (expressionType->typeKind == TYPE_POINTER && targetType->typeKind == TYPE_POINTER)
		return true;

	if (expressionType->typeKind == TYPE_POINTER && isIntegerType(targetType))
		return true;
	if (isIntegerType(expressionType) && targetType->typeKind == TYPE_POINTER)
		return true;

	if (expressionType->typeKind == TYPE_ANY || targetType->typeKind == TYPE_ANY)
		return true;

	return false;
}

static bool canCoerceType(Expression* arg, Type* targetType)
{
	if (arg->type == NODE_INT_LITERAL && isIntegerType(targetType))
	{
		IntLiteral* intLiteral = (IntLiteral*)arg;
		if (intLiteral->negative && isUnsignedType(targetType))
			return false;
		return true;
	}
	if (arg->type == NODE_INT_LITERAL && isFloatingPointType(targetType))
		return true;
	if (arg->type == NODE_FLOAT_LITERAL && isFloatingPointType(targetType))
		return true;
	if (arg->type == NODE_STRING_LITERAL && (targetType->typeKind == TYPE_STRING || targetType->typeKind == TYPE_POINTER && targetType->pointer.elementType->typeKind == TYPE_INT8))
		return true;
	if (arg->type == NODE_NULL_LITERAL && targetType->typeKind == TYPE_POINTER)
		return true;
	if (arg->type == NODE_EXPRESSION_LIST && targetType->typeKind == TYPE_STRUCT && ((ExpressionList*)arg)->numValues == targetType->struct_.numFields)
		return true;
	if (arg->type == NODE_ARRAY_INITIALIZER && targetType->typeKind == TYPE_ARRAY && isConstant(arg) && (targetType->array.size == 0 || targetType->array.size == ((ArrayInitializer*)arg)->numValues))
		return true;
	return false;
}

static bool isAssignable(TypeChecker* tc, Type* expressionType, Type* targetType, Expression** ref)
{
	if (compareTypes(expressionType, targetType))
		return true;

	if (ref && *ref && canCoerceType(*ref, targetType))
		return true;

	if (isNumericType(expressionType) && isNumericType(targetType))
	{
		if (getNumericRank(expressionType) <= getNumericRank(targetType))
		{
			if (isUnsignedType(expressionType) == isUnsignedType(targetType))
			{
				if (ref)
					insertImplicitCast(tc, ref, targetType);
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	if (expressionType->typeKind == TYPE_POINTER && targetType->typeKind == TYPE_POINTER)
	{
		Type* elementType1 = expressionType->pointer.elementType;
		Type* elementType2 = targetType->pointer.elementType;
		elementType1 = unwrapType(elementType1);
		elementType2 = unwrapType(elementType2);

		if (elementType2->typeKind == TYPE_VOID || compareTypes(elementType1, elementType2))
		{
			if (ref)
				insertImplicitCast(tc, ref, targetType);
			return true;
		}
		else
		{
			return false;
		}
	}

	if (targetType->typeKind == TYPE_ANY)
	{
		if (ref)
			insertImplicitCast(tc, ref, targetType);
		return true;
	}

	if (targetType->typeKind == TYPE_ALIAS)
	{
		Type* unwrappedType = targetType;
		while (unwrappedType->typeKind == TYPE_ALIAS)
			unwrappedType = unwrapType(unwrappedType);

		if (isAssignable(tc, expressionType, unwrappedType, ref))
		{
			if (ref)
				insertImplicitCast(tc, ref, targetType);
			return true;
		}
		else
		{
			return false;
		}
	}

	return false;
}

OperatorType assignmentOperatorToBinary(OperatorType op)
{
	switch (op)
	{
	case (OPERATOR_ADD_ASSIGN): return OPERATOR_ADD;
	case (OPERATOR_SUBTRACT_ASSIGN): return OPERATOR_SUBTRACT;
	case (OPERATOR_MULTIPLY_ASSIGN): return OPERATOR_MULTIPLY;
	case (OPERATOR_DIVIDE_ASSIGN): return OPERATOR_DIVIDE;
	case (OPERATOR_MODULO_ASSIGN): return OPERATOR_MODULO;
	case (OPERATOR_BITSHIFT_LEFT_ASSIGN): return OPERATOR_BITSHIFT_LEFT;
	case (OPERATOR_BITSHIFT_RIGHT_ASSIGN): return OPERATOR_BITSHIFT_RIGHT;
	case (OPERATOR_BITWISE_AND_ASSIGN): return OPERATOR_BITWISE_AND;
	case (OPERATOR_BITWISE_XOR_ASSIGN): return OPERATOR_BITWISE_XOR;
	case (OPERATOR_BITWISE_OR_ASSIGN): return OPERATOR_BITWISE_OR;
	case (OPERATOR_LOGICAL_AND_ASSIGN): return OPERATOR_LOGICAL_AND;
	case (OPERATOR_LOGICAL_OR_ASSIGN): return OPERATOR_LOGICAL_OR;
	default:
		SnekAssert(false);
		return OPERATOR_NULL;
	}
}

static int argScore(TypeChecker* tc, Type* argType, Type* paramType, Expression* arg)
{
	if (compareTypes(argType, paramType))
		return 0;
	else if (canCoerceType(arg, paramType))
	{
		if (isIntegerType(argType) && isIntegerType(paramType) && isUnsignedType(argType) == isUnsignedType(paramType))
			return 1;
		return 2;
	}
	else if (isAssignable(tc, argType, paramType, nullptr))
	{
		if (isIntegerType(argType) && isIntegerType(paramType) && isUnsignedType(argType) == isUnsignedType(paramType))
			return 3;
		return 4;
	}
	return 999;
}

static void selectFunctionOverload(TypeChecker* tc, FunctionSet* functionSet, int numArgs, Expression** args, int& bestOverload, bool& ambiguous)
{
	int bestScore = 9999;

	for (int i = 0; i < functionSet->count; i++)
	{
		Function* function = functionSet->overloads[i].declaration;
		if (function->numParams != numArgs)
			continue;

		// function has not been resolved yet, eg while evaluating inline function return expressions
		if (!function->functionType)
			continue;

		int currentScore = 0;
		bool valid = true;

		for (int j = 0; j < numArgs; j++)
		{
			Type* argType = args[j]->inferredType;
			Type* paramType = function->params[j]->paramType->inferredType;

			int score = argScore(tc, argType, paramType, args[j]);
			if (score > 4)
			{
				valid = false;
				break;
			}

			currentScore += score;
		}

		if (valid)
		{
			if (currentScore < bestScore)
			{
				bestScore = currentScore;
				bestOverload = i;
				ambiguous = false;
			}
			else if (currentScore == bestScore)
			{
				ambiguous = true;
			}
		}
	}
}

static Type* resolveIdentifier(TypeChecker* tc, Identifier* identifier, bool hasArgs, int numArgs, Expression** args)
{
	if (Symbol* symbol = resolveSymbol(tc, identifier->name))
	{
		if (symbol->file == tc->file->handle)
			identifier->resolvedSymbol = symbol;
		identifier->resolvedSymbolHandle = getSymbolHandle(tc, symbol);

		if (symbol->type == SYMBOL_VARIABLE)
		{
			Node* node = symbol->declaration;
			if (node->type == NODE_VARIABLE_DECLARATION)
			{
				VariableDeclaration* variableDeclaration = &node->variableDeclaration;
				return identifier->inferredType = variableDeclaration->variableType->inferredType;
			}
			else if (node->type == NODE_GLOBAL_VARIABLE)
			{
				GlobalVariable* globalVariable = &node->globalVariable;
				return identifier->inferredType = globalVariable->variableType->inferredType;
			}
			else if (node->type == NODE_PARAMETER)
			{
				Parameter* parameter = &node->parameter;

				Type* paramType = parameter->paramType->inferredType;
				if (parameter->variadic)
					paramType = getArrayType(tc->types, paramType, 0, tc->file);

				return identifier->inferredType = paramType;
			}
			else if (node->type == NODE_FOR)
			{
				For* for_ = &node->for_;
				return identifier->inferredType = &tc->types->primitiveTypes[TYPE_INT32];
			}
			else
			{
				SnekAssert(false);
				return identifier->inferredType = &tc->types->errorType;
			}
		}
		else if (symbol->type == SYMBOL_FUNCTION_SET)
		{
			int bestOverload = -1;

			if (symbol->functionSet.count == 1)
			{
				bestOverload = 0;
			}
			else
			{
				if (hasArgs)
				{
					bool ambiguous = false;
					selectFunctionOverload(tc, &symbol->functionSet, numArgs, args, bestOverload, ambiguous);

					if (ambiguous)
					{
						char buffer[256];
						buffer[0] = 0;
						strcat(buffer, "(");
						for (int i = 0; i < numArgs; i++)
						{
							strcat(buffer, args[i]->inferredType->name.ptr);
							if (i < numArgs - 1)
								strcat(buffer, ",");
						}
						strcat(buffer, ")");

						error(tc, (Node*)identifier, "Ambiguous overload of function '%.*s' for arguments %s", identifier->name.length, identifier->name.ptr, buffer);
						return identifier->inferredType = &tc->types->errorType;
					}
					else if (bestOverload == -1)
					{
						char buffer[256];
						buffer[0] = 0;
						strcat(buffer, "(");
						for (int i = 0; i < numArgs; i++)
						{
							strcat(buffer, args[i]->inferredType->name.ptr);
							if (i < numArgs - 1)
								strcat(buffer, ",");
						}
						strcat(buffer, ")");

						error(tc, (Node*)identifier, "No overload of function '%.*s' for arguments %s", identifier->name.length, identifier->name.ptr, buffer);
						return identifier->inferredType = &tc->types->errorType;
					}
				}
				else
				{
					error(tc, (Node*)identifier, "Ambiguous identifier, multiple function overloads with name '%.*s'", identifier->name.length, identifier->name.ptr);
					return identifier->inferredType = &tc->types->errorType;
				}
			}

			identifier->functionOverloadID = bestOverload;

			if (!symbol->functionSet.overloads[bestOverload].declaration->functionType)
			{
				error(tc, (Node*)identifier, "Function '%.*s' referenced in type-defining inline expression must be declared above", identifier->name.length, identifier->name.ptr);
				return identifier->inferredType = &tc->types->errorType;
			}
			else
			{
				return identifier->inferredType = symbol->functionSet.overloads[bestOverload].declaration->functionType;
			}
		}
		else if (symbol->type == SYMBOL_TYPE)
		{
			return identifier->inferredType = &tc->types->primitiveTypes[TYPE_TYPE];
		}
		else
		{
			SnekAssert(false);
			return identifier->inferredType = &tc->types->errorType;
		}
	}
	else
	{
		error(tc, (Node*)identifier, "Undefined variable '%.*s'", identifier->name.length, identifier->name.ptr);
		return identifier->inferredType = &tc->types->errorType;
	}
}

int getFieldIndex(Type* operandType, StringView name, Expression* operand)
{
	if (operandType->typeKind == TYPE_STRUCT)
	{
		return getFieldIndex(name, operandType->struct_.numFields + operandType->struct_.numOffsetFields, operandType->struct_.fieldNames);
	}
	else if (operandType->typeKind == TYPE_UNION)
	{
		return getFieldIndex(name, operandType->union_.numFields, operandType->union_.fieldNames);
	}
	else if (operandType->typeKind == TYPE_STRING)
	{
		if (compareString(name, "data"))
			return 0;
		else if (compareString(name, "length"))
			return 1;
		return -1;
	}
	else if (operandType->typeKind == TYPE_ARRAY)
	{
		if (compareString(name, "data"))
			return 0;
		else if (compareString(name, "length"))
			return 1;
		return -1;
	}
	else if (operandType->typeKind == TYPE_ANY)
	{
		if (compareString(name, "value"))
			return 0;
		else if (compareString(name, "type"))
			return 1;
		return -1;
	}
	else if (operandType->typeKind == TYPE_TYPE)
	{
		SnekAssert(operand->type == NODE_IDENTIFIER);

		Identifier* typeName = (Identifier*)operand;
		if (Symbol* symbol = getIdentifierSymbol(typeName))
		{
			if (symbol->declaration->type == NODE_ENUM)
			{
				Enum* enum_ = &symbol->declaration->enum_;
				return getEnumValue(name, enum_->numValues, enum_->values);
			}
		}
		return -1;
	}

	return -1;
}

static Type* resolveMemberAccess(TypeChecker* tc, MemberAccess* member, bool hasArgs, int numArgs, Expression** args)
{
	Type* operandType = resolveExpression(tc, member->operand);

	if (operandType == &tc->types->errorType)
	{
		return member->inferredType = &tc->types->errorType;
	}

	if (Symbol* symbol = resolveSymbol(tc, member->name))
	{
		if (symbol->type == SYMBOL_FUNCTION_SET)
		{
			if (hasArgs)
			{
				int bestOverload = -1;

				int mark = tc->scratch->mark();

				tc->scratch->add(member->operand);

				for (int i = 0; i < numArgs; i++)
				{
					tc->scratch->add(args[i]);
				}

				bool ambiguous = false;
				selectFunctionOverload(tc, &symbol->functionSet, numArgs + 1, tc->scratch->getData<Expression*>(mark), bestOverload, ambiguous);

				tc->scratch->release(mark);

				if (ambiguous)
				{
					char buffer[256];
					buffer[0] = 0;
					strcat(buffer, "(");
					strcat(buffer, member->operand->inferredType->name.ptr);
					if (numArgs)
						strcat(buffer, ", ");
					for (int i = 0; i < numArgs; i++)
					{
						strcat(buffer, args[i]->inferredType->name.ptr);
						if (i < numArgs - 1)
							strcat(buffer, ",");
					}
					strcat(buffer, ")");

					error(tc, (Node*)member, "Ambiguous overload of function '%.*s.%.*s' for arguments %s", operandType->mangledName.length, operandType->mangledName.ptr, member->name.length, member->name.ptr, buffer);
					return member->inferredType = &tc->types->errorType;
				}
				else if (bestOverload == -1)
				{
					char buffer[256];
					buffer[0] = 0;
					strcat(buffer, "(");
					strcat(buffer, member->operand->inferredType->name.ptr);
					if (numArgs)
						strcat(buffer, ", ");
					for (int i = 0; i < numArgs; i++)
					{
						strcat(buffer, args[i]->inferredType->name.ptr);
						if (i < numArgs - 1)
							strcat(buffer, ",");
					}
					strcat(buffer, ")");

					error(tc, (Node*)member, "No overload of function '%.*s.%.*s' for arguments %s", operandType->mangledName.length, operandType->mangledName.ptr, member->name.length, member->name.ptr, buffer);
					return member->inferredType = &tc->types->errorType;
				}

				if (symbol->file == tc->file->handle)
					member->resolvedSymbol = symbol;
				member->resolvedSymbolHandle = getSymbolHandle(tc, symbol);
				member->functionOverloadID = bestOverload;

				if (!symbol->functionSet.overloads[bestOverload].declaration->functionType)
				{
					error(tc, (Node*)member, "Function '%.*s.%.*s' referenced in type-defining inline expression must be declared above", operandType->mangledName.length, operandType->mangledName.ptr, member->name.length, member->name.ptr);
					return member->inferredType = &tc->types->errorType;
				}
				else
				{
					return member->inferredType = symbol->functionSet.overloads[bestOverload].declaration->functionType;
				}
			}
		}
	}

	if (operandType->typeKind == TYPE_POINTER)
		operandType = operandType->pointer.elementType;

	int fieldID = getFieldIndex(operandType, member->name, member->operand);
	member->index = fieldID;

	if (operandType->typeKind == TYPE_STRUCT)
	{
		if (fieldID == -1)
		{
			error(tc, member->name, "Undefined struct field '%.*s.%.*s'", operandType->name.length, operandType->name.ptr, member->name.length, member->name.ptr);
			return member->inferredType = &tc->types->errorType;
		}
		return member->inferredType = operandType->struct_.fieldTypes[fieldID];
	}
	else if (operandType->typeKind == TYPE_UNION)
	{
		if (fieldID == -1)
		{
			error(tc, member->name, "Undefined union field '%.*s.%.*s'", operandType->name.length, operandType->name.ptr, member->name.length, member->name.ptr);
			return member->inferredType = &tc->types->errorType;
		}
		return member->inferredType = operandType->union_.fieldTypes[fieldID];
	}
	else if (operandType->typeKind == TYPE_STRING)
	{
		if (fieldID == 0)
			return member->inferredType = getPointerType(tc->types, &tc->types->primitiveTypes[TYPE_INT8], tc->file);
		else if (fieldID == 1)
			return member->inferredType = &tc->types->primitiveTypes[TYPE_UINT64];
		else
		{
			error(tc, member->name, "Undefined string field '%.*s.%.*s'", operandType->name.length, operandType->name.ptr, member->name.length, member->name.ptr);
			return member->inferredType = &tc->types->errorType;
		}
	}
	else if (operandType->typeKind == TYPE_ARRAY)
	{
		if (fieldID == 0)
			return member->inferredType = getPointerType(tc->types, operandType->array.elementType, tc->file);
		else if (fieldID == 1)
			return member->inferredType = &tc->types->primitiveTypes[TYPE_UINT64];
		else
		{
			error(tc, member->name, "Undefined array field '%.*s.%.*s'", operandType->name.length, operandType->name.ptr, member->name.length, member->name.ptr);
			return member->inferredType = &tc->types->errorType;
		}
	}
	else if (operandType->typeKind == TYPE_ANY)
	{
		if (fieldID == 0)
			return member->inferredType = getPointerType(tc->types, &tc->types->primitiveTypes[TYPE_VOID], tc->file);
		else if (fieldID == 1)
			return member->inferredType = &tc->types->primitiveTypes[TYPE_INT32];
		else
		{
			error(tc, member->name, "Undefined any field '%.*s.%.*s'", operandType->name.length, operandType->name.ptr, member->name.length, member->name.ptr);
			return member->inferredType = &tc->types->errorType;
		}
	}
	else if (operandType->typeKind == TYPE_TYPE)
	{
		SnekAssert(member->operand->type == NODE_IDENTIFIER);

		Identifier* typeName = (Identifier*)member->operand;
		Symbol* symbol = getIdentifierSymbol(typeName);
		if (!symbol)
		{
			error(tc, typeName->name, "Unknown symbol '%.*s', workspace might be out of sync", typeName->name.length, typeName->name.ptr);
			return member->inferredType = &tc->types->errorType;
		}

		if (symbol->declaration->type == NODE_ENUM)
		{
			Enum* enum_ = &symbol->declaration->enum_;

			if (fieldID != -1)
			{
				return member->inferredType = enum_->enumType;
			}
			else
			{
				error(tc, member->name, "Undefined enum value '%.*s'", member->name.length, member->name.ptr);
				return member->inferredType = &tc->types->errorType;
			}
		}
		else
		{
			error(tc, (Node*)member->operand, "Undefined namespace '%.*s'", typeName->name.length, typeName->name.ptr);
			return member->inferredType = &tc->types->errorType;
		}
	}
	else
	{
		error(tc, (Node*)member->operand, "Operand of member access must be one of struct, union, string, array, any");
		return member->inferredType = &tc->types->errorType;
	}
}

static Type* resolveExpression(TypeChecker* tc, Expression* expression, Type* expectedType)
{
	if (expression->type == NODE_ERROR_EXPRESSION)
	{
		return expression->inferredType = &tc->types->errorType;
	}
	if (expression->type == NODE_INT_LITERAL)
	{
		IntLiteral* intLiteral = (IntLiteral*)expression;

		Type* intType = nullptr;
		if (expectedType)
		{
			if (isIntegerType(expectedType))
				intType = expectedType;
			else if (expectedType->typeKind == TYPE_ALIAS)
			{
				while (expectedType->typeKind == TYPE_ALIAS)
					expectedType = unwrapType(expectedType);
				if (isIntegerType(expectedType))
					intType = expectedType;
			}
		}

		intLiteral->intValue = stringToIntConstant(tc, (Node*)intLiteral, intLiteral->value, &intLiteral->negative, &intLiteral->base, &intType);

		return expression->inferredType = intType;
	}
	else if (expression->type == NODE_FLOAT_LITERAL)
	{
		FloatLiteral* floatLiteral = (FloatLiteral*)expression;

		Type* floatType = expectedType && isFloatingPointType(expectedType) ? expectedType : nullptr;
		floatLiteral->floatValue = stringToFloatConstant(tc, (Node*)floatLiteral, floatLiteral->value, &floatType);

		return expression->inferredType = floatType;
	}
	else if (expression->type == NODE_STRING_LITERAL)
	{
		if (expectedType && expectedType->typeKind == TYPE_STRING)
			return expression->inferredType = expectedType;
		else if (expectedType && expectedType->typeKind == TYPE_POINTER && expectedType->pointer.elementType->typeKind == TYPE_INT8)
			return expression->inferredType = expectedType;
		else
		{
			return expression->inferredType = &tc->types->primitiveTypes[TYPE_STRING];
		}
	}
	else if (expression->type == NODE_CHAR_LITERAL)
	{
		return expression->inferredType = &tc->types->primitiveTypes[TYPE_INT8];
	}
	else if (expression->type == NODE_TRUE)
	{
		return expression->inferredType = &tc->types->primitiveTypes[TYPE_BOOL];
	}
	else if (expression->type == NODE_FALSE)
	{
		return expression->inferredType = &tc->types->primitiveTypes[TYPE_BOOL];
	}
	else if (expression->type == NODE_NULL_LITERAL)
	{
		if (expectedType && expectedType->typeKind == TYPE_POINTER)
			return expression->inferredType = expectedType;
		else
			return expression->inferredType = getPointerType(tc->types, &tc->types->primitiveTypes[TYPE_UINT8], tc->file);
	}
	else if (expression->type == NODE_SIZEOF)
	{
		Sizeof* sizeof_ = (Sizeof*)expression;

		if (sizeof_->expression)
		{
			sizeof_->expressionType = resolveExpression(tc, sizeof_->expression);
			if (sizeof_->expressionType->typeKind == TYPE_TYPE)
			{
				Symbol* symbol = sizeof_->expression->type == NODE_IDENTIFIER ? getIdentifierSymbol((Identifier*)sizeof_->expression) : nullptr;
				if (symbol)
				{
					SnekAssert(symbol->type == SYMBOL_TYPE);
					Node* declaration = symbol->declaration;
					if (declaration->type == NODE_STRUCT)
						sizeof_->expressionType = declaration->struct_.structType;
					else if (declaration->type == NODE_UNION)
						sizeof_->expressionType = declaration->union_.unionType;
					else if (declaration->type == NODE_ENUM)
						sizeof_->expressionType = declaration->enum_.enumType;
					else if (declaration->type == NODE_TYPEDEF)
						sizeof_->expressionType = declaration->typedef_.aliasType;
					else
					{
						SnekAssert(false);
					}
				}
			}
		}
		else if (sizeof_->targetType)
		{
			sizeof_->expressionType = resolveType(tc, sizeof_->targetType);
		}
		else
		{
			SnekAssert(false);
		}

		return expression->inferredType = getUInt64Type(tc->types);
	}
	else if (expression->type == NODE_IDENTIFIER)
	{
		Identifier* identifier = (Identifier*)expression;
		return resolveIdentifier(tc, identifier, false, 0, nullptr);
	}
	else if (expression->type == NODE_COMPOUND_EXPRESSION)
	{
		CompoundExpression* compound = (CompoundExpression*)expression;
		resolveExpression(tc, compound->value, expectedType);
		return expression->inferredType = compound->value->inferredType;
	}
	else if (expression->type == NODE_EXPRESSION_LIST)
	{
		ExpressionList* expressionList = (ExpressionList*)expression;

		if (expectedType && expectedType->typeKind == TYPE_STRUCT)
		{
			expression->inferredType = expectedType;

			if (expectedType->struct_.numFields != expressionList->numValues)
			{
				error(tc, (Node*)expressionList, "%d initializer values should be %d for type '%.*s'", expressionList->numValues, expectedType->struct_.numFields, expectedType->name.length, expectedType->name.ptr);
				return expression->inferredType;
			}

			for (int i = 0; i < expressionList->numValues; i++)
			{
				if (expressionList->values[i])
				{
					Type* targetType = expectedType->struct_.fieldTypes[i];

					resolveExpression(tc, expressionList->values[i], targetType);

					Type* fieldType = expressionList->values[i]->inferredType;

					if (fieldType != getErrorType(tc->types) && targetType != getErrorType(tc->types) && !isAssignable(tc, fieldType, targetType, &expressionList->values[i]))
					{
						if (expectedType->struct_.fieldNames)
						{
							StringView fieldName = expectedType->struct_.fieldNames[i];
							error(tc, (Node*)expressionList->values[i], "Can't assign value of type '%.*s' to struct field '%.*s' of type '%.*s'", fieldType->name.length, fieldType->name.ptr, fieldName.length, fieldName.ptr, targetType->name.length, targetType->name.ptr);
						}
						else
						{
							error(tc, (Node*)expressionList->values[i], "Can't assign value of type '%.*s' to struct field #%d of type '%.*s'", fieldType->name.length, fieldType->name.ptr, i, targetType->name.length, targetType->name.ptr);
						}
					}
				}
			}

			return expression->inferredType;
		}
		else
		{
			int mark = tc->scratch->mark();

			for (int i = 0; i < expressionList->numValues; i++)
			{
				if (expressionList->values[i])
				{
					resolveExpression(tc, expressionList->values[i]);
					tc->scratch->add(expressionList->values[i]->inferredType);
				}
				else
				{
					tc->scratch->add(getErrorType(tc->types));
				}
			}

			Type** valueTypes = tc->scratch->getData<Type*>(mark);

			expression->inferredType = getAnonymousStructType(tc->types, expressionList->numValues, valueTypes, nullptr, tc->file);

			tc->scratch->release(mark);

			return expression->inferredType;
		}
	}
	else if (expression->type == NODE_ARRAY_INITIALIZER)
	{
		ArrayInitializer* arrayInitializer = (ArrayInitializer*)expression;

		Type* elementType = expectedType && expectedType->typeKind == TYPE_ARRAY ? expectedType->array.elementType : nullptr;

		for (int i = 0; i < arrayInitializer->numValues; i++)
		{
			if (arrayInitializer->values[i])
			{
				resolveExpression(tc, arrayInitializer->values[i], elementType);

				Type* valueType = arrayInitializer->values[i]->inferredType;
				if (!elementType)
					elementType = valueType;

				if (!isErrorType(elementType) && !isAssignable(tc, valueType, elementType, &arrayInitializer->values[i]))
				{
					error(tc, (Node*)arrayInitializer->values[i], "Can't assign value of type '%.*s' to array element #%d of type '%.*s'", valueType->name.length, valueType->name.ptr, i, elementType->name.length, elementType->name.ptr);
				}
			}
		}

		return expression->inferredType = expectedType && expectedType->typeKind == TYPE_ARRAY ? expectedType : getArrayType(tc->types, elementType, arrayInitializer->numValues, tc->file);
	}
	else if (expression->type == NODE_BINARY_OPERATOR)
	{
		BinaryOperator* binaryOperator = (BinaryOperator*)expression;

		Type* left = resolveExpression(tc, binaryOperator->left);
		Type* right = resolveExpression(tc, binaryOperator->right);

		if (left == &tc->types->errorType || right == &tc->types->errorType)
		{
			return binaryOperator->inferredType = expression->inferredType = &tc->types->errorType;
		}

		if (left->typeKind == TYPE_ENUM && compareTypes(left->enum_.valueType, right))
		{
			left = left->enum_.valueType;
			insertImplicitCast(tc, &binaryOperator->left, right);
		}
		else if (right->typeKind == TYPE_ENUM && compareTypes(right->enum_.valueType, left))
		{
			right = right->enum_.valueType;
			insertImplicitCast(tc, &binaryOperator->right, left);
		}
		else if (left->typeKind == TYPE_ALIAS && compareTypes(left->alias.valueType, right))
		{
			left = left->alias.valueType;
			insertImplicitCast(tc, &binaryOperator->left, right);
		}
		else if (right->typeKind == TYPE_ALIAS && compareTypes(right->alias.valueType, left))
		{
			right = right->alias.valueType;
			insertImplicitCast(tc, &binaryOperator->right, left);
		}

		if (isErrorType(left) || isErrorType(right))
		{
			return binaryOperator->inferredType = getErrorType(tc->types);
		}

		// todo check operator overload

		if (binaryOperator->op == OPERATOR_ADD || binaryOperator->op == OPERATOR_SUBTRACT || binaryOperator->op == OPERATOR_MULTIPLY || binaryOperator->op == OPERATOR_DIVIDE || binaryOperator->op == OPERATOR_MODULO)
			return binaryOperator->inferredType = typeCheckArithmeticOperator(tc, binaryOperator->op, left, right, binaryOperator, expectedType, &binaryOperator->left, &binaryOperator->right);
		else if (binaryOperator->op == OPERATOR_EQUALS || binaryOperator->op == OPERATOR_NOT_EQUALS || binaryOperator->op == OPERATOR_LESS || binaryOperator->op == OPERATOR_LESS_EQUALS || binaryOperator->op == OPERATOR_GREATER || binaryOperator->op == OPERATOR_GREATER_EQUALS)
			return binaryOperator->inferredType = typeCheckComparisonOperator(tc, binaryOperator, left, right);
		else if (binaryOperator->op == OPERATOR_BITWISE_AND || binaryOperator->op == OPERATOR_BITWISE_XOR || binaryOperator->op == OPERATOR_BITWISE_OR || binaryOperator->op == OPERATOR_BITSHIFT_LEFT || binaryOperator->op == OPERATOR_BITSHIFT_RIGHT)
			return binaryOperator->inferredType = typeCheckBitwiseOperator(tc, binaryOperator, left, right);
		else if (binaryOperator->op == OPERATOR_LOGICAL_AND || binaryOperator->op == OPERATOR_LOGICAL_OR)
			return binaryOperator->inferredType = typeCheckLogicalOperator(tc, binaryOperator, left, right);
		else
		{
			SnekAssert(false);
			return binaryOperator->inferredType = &tc->types->errorType;
		}
	}
	else if (expression->type == NODE_UNARY_OPERATOR)
	{
		UnaryOperator* unaryOperator = (UnaryOperator*)expression;

		Type* operandType = resolveExpression(tc, unaryOperator->operand);

		if (operandType == &tc->types->errorType)
			return unaryOperator->inferredType = operandType;

		if (unaryOperator->op == OPERATOR_LOGICAL_NOT)
		{
			if (!isTruthyType(operandType))
			{
				error(tc, (Node*)unaryOperator->operand, "Operand of logical not operator must be a bool or number");
				return unaryOperator->inferredType = &tc->types->errorType;
			}

			if (operandType->typeKind != TYPE_BOOL)
				insertImplicitCast(tc, &unaryOperator->operand, &tc->types->primitiveTypes[TYPE_BOOL]);

			return unaryOperator->inferredType = &tc->types->primitiveTypes[TYPE_BOOL];
		}
		else if (unaryOperator->op == OPERATOR_BITWISE_NOT)
		{
			if (!isIntegerType(operandType))
			{
				error(tc, (Node*)unaryOperator->operand, "Operand of bitwise not operator must be an integer");
				return unaryOperator->inferredType = &tc->types->errorType;
			}

			return unaryOperator->inferredType = operandType;
		}
		else if (unaryOperator->op == OPERATOR_MINUS_PREFIX)
		{
			if (!isNumericType(operandType))
			{
				error(tc, (Node*)unaryOperator->operand, "Operand of negate operator must be a number");
				return unaryOperator->inferredType = &tc->types->errorType;
			}

			return unaryOperator->inferredType = operandType;
		}
		else if (unaryOperator->op == OPERATOR_PLUS_PREFIX)
		{
			if (!isIntegerType(operandType))
			{
				error(tc, (Node*)unaryOperator->operand, "Operand of plus operator must be an integer");
				return unaryOperator->inferredType = &tc->types->errorType;
			}

			return unaryOperator->inferredType = operandType;
		}
		else if (unaryOperator->op == OPERATOR_DEREFERENCE)
		{
			if (operandType->typeKind != TYPE_POINTER)
			{
				error(tc, (Node*)unaryOperator->operand, "Cannot deference non-pointer type");
				return unaryOperator->inferredType = &tc->types->errorType;
			}

			Type* targetType = operandType->pointer.elementType;

			if (targetType->typeKind == TYPE_VOID)
			{
				error(tc, (Node*)unaryOperator, "Cannot deference void pointer");
				return unaryOperator->inferredType = &tc->types->errorType;
			}

			return unaryOperator->inferredType = targetType;
		}
		else if (unaryOperator->op == OPERATOR_ADDRESS)
		{
			if (!isLValue(unaryOperator->operand))
			{
				error(tc, (Node*)unaryOperator->operand, "Cannot take address of rvalue expression");
				return unaryOperator->inferredType = &tc->types->errorType;
			}

			return unaryOperator->inferredType = getPointerType(tc->types, operandType, tc->file);
		}
		else if (unaryOperator->op == OPERATOR_INCREMENT_PREFIX || unaryOperator->op == OPERATOR_DECREMENT_PREFIX
			|| unaryOperator->op == OPERATOR_INCREMENT_POSTFIX || unaryOperator->op == OPERATOR_DECREMENT_POSTFIX)
		{
			if (!isLValue(unaryOperator->operand))
			{
				error(tc, (Node*)unaryOperator->operand, "Cannot increment/decrement rvalue expression");
				return unaryOperator->inferredType = &tc->types->errorType;
			}

			if (!isIntegerType(operandType) && !isFloatingPointType(operandType) && operandType->typeKind != TYPE_POINTER)
			{
				error(tc, (Node*)unaryOperator->operand, "Increment/decrement operator requires scalar or pointer type");
				return unaryOperator->inferredType = &tc->types->errorType;
			}

			return unaryOperator->inferredType = operandType;
		}
		else
		{
			SnekAssert(false);
			return unaryOperator->inferredType = &tc->types->errorType;
		}
	}
	else if (expression->type == NODE_FUNCTION_CALL)
	{
		FunctionCall* functionCall = (FunctionCall*)expression;

		for (int i = 0; i < functionCall->numArgs; i++)
		{
			resolveExpression(tc, functionCall->args[i]);
		}

		Function* function = nullptr;

		if (functionCall->expression->type == NODE_IDENTIFIER)
		{
			Identifier* identifier = (Identifier*)functionCall->expression;
			resolveIdentifier(tc, identifier, true, functionCall->numArgs, functionCall->args);
			if (Symbol* symbol = getIdentifierSymbol(identifier))
				function = symbol->functionSet.overloads[identifier->functionOverloadID].declaration;
		}
		else if (functionCall->expression->type == NODE_MEMBER_ACCESS)
		{
			MemberAccess* member = (MemberAccess*)functionCall->expression;
			resolveMemberAccess(tc, member, true, functionCall->numArgs, functionCall->args);
			if (Symbol* symbol = getMemberAccessSymbol(member))
				function = symbol->functionSet.overloads[member->functionOverloadID].declaration;
		}
		else
		{
			resolveExpression(tc, functionCall->expression);
		}

		Type* functionType = functionCall->expression->inferredType;

		if (functionType == &tc->types->errorType)
		{
			return expression->inferredType = &tc->types->errorType;
		}

		int numArgs = functionCall->numArgs;
		bool memberFunction = false;
		Expression** memberFunctionOperand = nullptr;

		if (functionCall->expression->type == NODE_MEMBER_ACCESS)
		{
			MemberAccess* member = (MemberAccess*)functionCall->expression;
			memberFunction = true;
			memberFunctionOperand = &member->operand;
			numArgs++;
		}

		if (functionType->typeKind != TYPE_FUNCTION)
		{
			error(tc, (Node*)functionCall->expression, "Operand of function call must be of type function");
			return expression->inferredType = &tc->types->errorType;
		}
		else
		{
			if (numArgs != functionType->function.numParams)
			{
				if (memberFunction)
					error(tc, (Node*)functionCall, "Incorrect number of member function arguments: %d, should be %d", functionCall->numArgs, functionType->function.numParams - 1);
				else
					error(tc, (Node*)functionCall, "Incorrect number of function arguments: %d, should be %d", numArgs, functionType->function.numParams);

				return expression->inferredType = getErrorType(tc->types);
			}

			for (int i = 0; i < functionCall->numArgs; i++)
			{
				resetExpression(&functionCall->args[i]);
				resolveExpression(tc, functionCall->args[i], functionType->function.paramTypes[i]);
			}

			for (int i = 0; i < numArgs; i++)
			{
				Expression** argRef = memberFunction ? (i == 0 ? memberFunctionOperand : &functionCall->args[i - 1]) : &functionCall->args[i];
				Expression* arg = *argRef;
				Type* argType = arg->inferredType;
				if (argType == &tc->types->errorType)
					continue;

				Type* paramType = nullptr;
				if (i < functionType->function.numParams)
					paramType = functionType->function.paramTypes[i];

				if (functionType->function.variadic && i >= functionType->function.numParams - 1)
				{
					SnekAssert(functionType->function.paramTypes[functionType->function.numParams - 1]->typeKind == TYPE_ARRAY);
					paramType = functionType->function.paramTypes[functionType->function.numParams - 1]->array.elementType;
				}

				if (paramType && memberFunction && i == 0)
				{
					if (compareTypes(argType, paramType) || paramType->typeKind == TYPE_POINTER && compareTypes(paramType->pointer.elementType, argType) && isLValue(arg))
					{
						//
					}
					else
					{
						SnekAssert(false);
					}
				}
				else if (paramType && !isAssignable(tc, argType, paramType, argRef))
				{
					if (function)
					{
						StringView paramName = function->params[i]->name;
						error(tc, (Node*)arg, "Cannot pass value of type '%.*s' to function parameter '%.*s' of type '%.*s'", argType->name.length, argType->name.ptr, paramName.length, paramName.ptr, paramType->name.length, paramType->name.ptr);
					}
					else
					{
						error(tc, (Node*)arg, "Cannot pass value of type '%.*s' to function parameter of type '%.*s'", argType->name.length, argType->name.ptr, paramType->name.length, paramType->name.ptr);
					}
				}
			}

			return expression->inferredType = (functionType->function.returnType ? functionType->function.returnType : getVoidType(tc->types));
		}
	}
	else if (expression->type == NODE_ARRAY_SUBSCRIPT)
	{
		ArraySubscript* subscript = (ArraySubscript*)expression;

		Type* operandType = resolveExpression(tc, subscript->operand);
		for (int i = 0; i < subscript->numArgs; i++)
		{
			resolveExpression(tc, subscript->args[i]);
		}

		if (operandType->typeKind == TYPE_STRING)
		{
			return expression->inferredType = &tc->types->primitiveTypes[TYPE_INT8];
		}
		else if (operandType->typeKind == TYPE_ARRAY)
		{
			return expression->inferredType = operandType->array.elementType;
		}
		else if (operandType->typeKind == TYPE_POINTER)
		{
			return expression->inferredType = operandType->pointer.elementType;
		}
		else
		{
			error(tc, (Node*)subscript->operand, "Operand of subscript operator must be one of array, string, pointer");
			return expression->inferredType = &tc->types->errorType;
		}
	}
	else if (expression->type == NODE_MEMBER_ACCESS)
	{
		MemberAccess* member = (MemberAccess*)expression;
		return resolveMemberAccess(tc, member, false, 0, nullptr);
	}
	else if (expression->type == NODE_TERNARY_CONDITION)
	{
		TernaryCondition* ternary = (TernaryCondition*)expression;

		Type* conditionType = resolveExpression(tc, ternary->condition);
		Type* thenType = resolveExpression(tc, ternary->then);
		Type* elseType = resolveExpression(tc, ternary->else_);

		if (isErrorType(conditionType) || isErrorType(thenType) || isErrorType(elseType))
		{
			return expression->inferredType = getErrorType(tc->types);
		}

		if (!isTruthyType(conditionType))
		{
			error(tc, (Node*)ternary->condition, "Ternary condition must be bool or scalar");
		}
		else if (conditionType->typeKind != TYPE_BOOL)
		{
			insertImplicitCast(tc, &ternary->condition, &tc->types->primitiveTypes[TYPE_BOOL]);
		}

		if (thenType == &tc->types->errorType || elseType == &tc->types->errorType)
		{
			return expression->inferredType = &tc->types->errorType;
		}

		Type* commonType = nullptr;
		if (thenType == elseType)
			commonType = thenType;
		else if (thenType->typeKind == TYPE_POINTER && elseType->typeKind == TYPE_POINTER)
			commonType = thenType;
		else
		{
			commonType = getCommonNumericType(thenType, elseType);
		}

		if (!commonType)
		{
			error(tc, (Node*)expression, "Incompatible types in ternary branches: ? '%.*s' : '%.*s'", thenType->name.length, thenType->name.ptr, elseType->name.length, elseType->name.ptr);
			return expression->inferredType = &tc->types->errorType;
		}

		if (thenType != commonType)
			insertImplicitCast(tc, &ternary->then, commonType);
		if (elseType != commonType)
			insertImplicitCast(tc, &ternary->else_, commonType);

		return expression->inferredType = commonType;
	}
	else if (expression->type == NODE_CAST)
	{
		Cast* cast = (Cast*)expression;

		// targetType can only be null for implicit casts inserted by the compiler.
		// those casts are already resolved and can only appear on re-typecheck
		SnekAssert(!cast->implicit);

		Type* targetType = resolveType(tc, cast->targetType);
		Type* expressionType = resolveExpression(tc, cast->expression);
		Type* expressionType2 = cast->expression2 ? resolveExpression(tc, cast->expression2) : nullptr;

		if (isErrorType(targetType) || isErrorType(expressionType) || expressionType2 && isErrorType(expressionType2))
		{
			return expression->inferredType = &tc->types->errorType;
		}

		if (targetType->typeKind == TYPE_STRING)
		{
			if (expressionType2)
			{
				if (isCharPointerType(expressionType) && isIntegerType(expressionType2))
				{
					return expression->inferredType = targetType;
				}
				else
				{
					error(tc, (Node*)expression, "String initializer arguments must be 'int8*' and 'int'");
					return expression->inferredType = getErrorType(tc->types);
				}
			}
			else
			{
				if (isCharPointerType(expressionType))
				{
					return expression->inferredType = targetType;
				}
			}
		}
		else
		{
			if (expressionType2)
			{
				error(tc, (Node*)expression, "Too many arguments for cast to '%.*s'", targetType->name.length, targetType->name.ptr);
				return expression->inferredType = getErrorType(tc->types);
			}
		}

		if (!isCastLegal(expressionType, targetType))
		{
			error(tc, (Node*)expression, "Illegal cast from '%.*s' to '%.*s'", expressionType->name.length, expressionType->name.ptr, targetType->name.length, targetType->name.ptr);
			return expression->inferredType = &tc->types->errorType;
		}

		return expression->inferredType = targetType;
	}

	SnekAssert(false);
	return expression->inferredType = &tc->types->errorType;
}

static void resolveStatement(TypeChecker* tc, Statement* statement)
{
	SnekAssert(tc->currentFunction);

	if (statement->type == NODE_ERROR_STATEMENT)
	{
	}
	else if (statement->type == NODE_BLOCK_STATEMENT)
	{
		BlockStatement* block = (BlockStatement*)statement;

		block->scope = pushScope(tc);

		for (int i = 0; i < block->numStatements; i++)
		{
			if (block->statements[i])
				resolveStatement(tc, block->statements[i]);
		}

		popScope(tc);
	}
	else if (statement->type == NODE_IF)
	{
		If* if_ = (If*)statement;

		Type* conditionType = resolveExpression(tc, if_->condition);

		resolveStatement(tc, if_->then);

		if (if_->else_)
		{
			resolveStatement(tc, if_->else_);
		}

		if (conditionType != &tc->types->errorType)
		{
			if (!isTruthyType(conditionType))
			{
				error(tc, (Node*)if_->condition, "if condition must be bool or scalar");
			}
			else if (conditionType->typeKind != TYPE_BOOL)
			{
				insertImplicitCast(tc, &if_->condition, &tc->types->primitiveTypes[TYPE_BOOL]);
			}
		}
	}
	else if (statement->type == NODE_WHILE)
	{
		While* while_ = (While*)statement;

		Type* conditionType = resolveExpression(tc, while_->condition);

		tc->loopDepth++;
		resolveStatement(tc, while_->then);
		tc->loopDepth--;

		if (!isTruthyType(conditionType))
		{
			error(tc, (Node*)while_->condition, "while condition must be bool or scalar");
		}
		else if (conditionType->typeKind != TYPE_BOOL)
		{
			insertImplicitCast(tc, &while_->condition, &tc->types->primitiveTypes[TYPE_BOOL]);
		}
	}
	else if (statement->type == NODE_FOR)
	{
		For* for_ = (For*)statement;

		tc->loopDepth++;
		for_->scope = pushScope(tc);

		Type* startValueType = for_->startValue ? resolveExpression(tc, for_->startValue) : getInt32Type(tc->types);
		Type* compareValueType = resolveExpression(tc, for_->compareValue);

		if (startValueType != getErrorType(tc->types) && !isNumericType(startValueType))
		{
			error(tc, (Node*)for_->startValue, "Initial value of for iterator must be a scalar");
		}
		if (startValueType != getErrorType(tc->types) && !isNumericType(compareValueType))
		{
			error(tc, (Node*)for_->compareValue, "Comparison value of for iterator must be a scalar");
		}

		if (for_->iteratorName.length)
		{
			insertSymbol(&for_->scope->symbols, for_->iteratorName, SYMBOL_VARIABLE, (Node*)statement, tc->file->handle);
		}

		resolveStatement(tc, for_->body);

		popScope(tc);
		tc->loopDepth--;
	}
	else if (statement->type == NODE_RETURN)
	{
		Return* return_ = (Return*)statement;

		if (!tc->currentFunction)
		{
			error(tc, (Node*)statement, "Cannot return outside of function");
		}
		else
		{
			Type* returnType = tc->currentFunction->functionType->function.returnType;

			if (return_->value)
			{
				Type* valueType = resolveExpression(tc, return_->value);
				if (valueType == &tc->types->errorType)
					return;

				if (!returnType)
				{
					error(tc, (Node*)return_->value, "Cannot return value of type '%.*s' from function without return value", valueType->name.length, valueType->name.ptr);
				}
				else if (!isAssignable(tc, valueType, returnType, &return_->value))
				{
					error(tc, (Node*)return_->value, "Cannot return value of type '%.*s' from function with return type '%.*s'", valueType->name.length, valueType->name.ptr, returnType->name.length, returnType->name.ptr);
				}
			}
			else
			{
				if (returnType)
				{
					error(tc, (Node*)return_, "Must return value of type '%.*s'", returnType->name.length, returnType->name.ptr);
				}
			}
		}
	}
	else if (statement->type == NODE_BREAK)
	{
		if (tc->loopDepth == 0)
		{
			error(tc, (Node*)statement, "Cannot break outside of a loop");
		}
	}
	else if (statement->type == NODE_CONTINUE)
	{
		if (tc->loopDepth == 0)
		{
			error(tc, (Node*)statement, "Cannot continue outside of a loop");
		}
	}
	else if (statement->type == NODE_DEFER)
	{
		Defer* defer = (Defer*)statement;

		if (!tc->currentFunction)
		{
			error(tc, (Node*)defer, "Cannot defer statement outside of function");
			return;
		}

		resolveStatement(tc, defer->body);
	}
	else if (statement->type == NODE_VARIABLE_DECLARATION)
	{
		VariableDeclaration* variableDeclaration = (VariableDeclaration*)statement;

		Type* variableType = resolveType(tc, variableDeclaration->variableType);

		for (int i = 0; i < variableDeclaration->numDeclarators; i++)
		{
			Symbol* symbol = lookupSymbol(&tc->currentScope->symbols, variableDeclaration->declarators[i].name);
			if (symbol)
			{
				error(tc, variableDeclaration->declarators[i].name, "Redeclaration of variable '%.*s'", variableDeclaration->declarators[i].name.length, variableDeclaration->declarators[i].name.ptr);
			}
			else
			{
				insertSymbol(&tc->currentScope->symbols, variableDeclaration->declarators[i].name, SYMBOL_VARIABLE, (Node*)statement, tc->file->handle);
			}

			if (variableDeclaration->declarators[i].value)
			{
				Type* initializerType = resolveExpression(tc, variableDeclaration->declarators[i].value, variableType);
				if (!isErrorType(initializerType) && !isAssignable(tc, initializerType, variableType, &variableDeclaration->declarators[i].value))
				{
					error(tc, (Node*)variableDeclaration->declarators[i].value, "Cannot initialize variable of type '%.*s' with value of type '%.*s'", variableType->name.length, variableType->name.ptr, initializerType->name.length, initializerType->name.ptr);
				}
			}
		}
	}
	else if (statement->type == NODE_ASSIGNMENT)
	{
		Assignment* assignment = (Assignment*)statement;

		Type* expressionType = resolveExpression(tc, assignment->expression);
		Type* valueType = resolveExpression(tc, assignment->value, expressionType);

		if (expressionType != getErrorType(tc->types) && !isLValue(assignment->expression))
		{
			error(tc, (Node*)assignment->expression, "Cannot assign value to non-lvalue expression");
		}

		if (isErrorType(valueType))
			return;

		if (assignment->op == OPERATOR_ASSIGN)
		{
			if (expressionType != getErrorType(tc->types) && valueType != getErrorType(tc->types) && !isAssignable(tc, valueType, expressionType, &assignment->value))
			{
				error(tc, (Node*)assignment->value, "Cannot assign value of type '%.*s' to expression of type '%.*s'", valueType->name.length, valueType->name.ptr, expressionType->name.length, expressionType->name.ptr);
			}
		}
		else
		{
			OperatorType op = assignmentOperatorToBinary(assignment->op);
			Type* resultType = typeCheckArithmeticOperator(tc, op, expressionType, valueType, assignment->value, expressionType, nullptr, &assignment->value);

			if (resultType != &tc->types->errorType && !isAssignable(tc, resultType, expressionType, nullptr))
			{
				error(tc, (Node*)statement, "Cannot assign result of type '%.*s' to expression of type '%.*s'", resultType->name.length, resultType->name.ptr, expressionType->name.length, expressionType->name.ptr);
			}
		}
	}
	else if (statement->type == NODE_EXPRESSION_STATEMENT)
	{
		ExpressionStatement* expression = (ExpressionStatement*)statement;

		resolveExpression(tc, expression->expression);
	}
	else
	{
		SnekAssert(false);
	}
}

static Type* resolveField(TypeChecker* tc, Field* field)
{
	if (field->variableType)
	{
		return resolveType(tc, field->variableType);
	}
	else
	{
		return &tc->types->errorType;
	}
}

static Type* resolveParameter(TypeChecker* tc, Parameter* parameter)
{
	Type* paramType = resolveType(tc, parameter->paramType);
	if (parameter->variadic)
		paramType = getArrayType(tc->types, paramType, 0, tc->file);
	return paramType;
}

void symbolResolution(TypeChecker* tc, File* file)
{
	tc->file = file;

	AST* ast = &file->ast;

	pushScope(tc, file->ast.globalScope);

	// Primitive global initializers
	for (int i = 0; i < ast->numGlobalVariables; i++)
	{
		GlobalVariable* globalVariable = ast->globalVariables[i];
		resolveType(tc, globalVariable->variableType);
		for (int j = 0; j < globalVariable->numDeclarators; j++)
		{
			Expression* value = globalVariable->declarators[j].value;
			StringView name = globalVariable->declarators[j].name;
			if (value)
			{
				if (isPrimitiveType(globalVariable->variableType->inferredType))
				{
					resolveExpression(tc, value, globalVariable->variableType->inferredType);

					if (value->inferredType != getErrorType(tc->types) && !isAssignable(tc, value->inferredType, globalVariable->variableType->inferredType, &globalVariable->declarators[j].value))
					{
						error(tc, (Node*)value, "Can't assign value of type '%.*s' to global variable '%.*s' of type '%.*s'", value->inferredType->name.length, value->inferredType->name.ptr, name.length, name.ptr, globalVariable->variableType->inferredType->name.length, globalVariable->variableType->inferredType->name.ptr);
					}
					else if (value->inferredType != getErrorType(tc->types) && !isConstant(value))
					{
						error(tc, (Node*)value, "Global variable initializer must be a constant value");
					}
				}
			}
			else if (globalVariable->storage & STORAGE_CONSTANT)
			{
				error(tc, (Node*)globalVariable, "Constant variable must have an initializer");
			}
		}
	}

	for (int i = 0; i < ast->numEnums; i++)
	{
		Enum* enum_ = ast->enums[i];

		Type* valueType = &tc->types->primitiveTypes[TYPE_INT32];
		if (enum_->valueType)
		{
			valueType = resolveType(tc, enum_->valueType);

			if (!isIntegerType(valueType))
			{
				error(tc, (Node*)enum_->valueType, "Enum type must be an integer");
			}
		}

		int lastWithValue = -1;
		for (int j = 0; j < enum_->numValues; j++)
		{
			EnumValue* enumValue = enum_->values[j];
			if (enumValue->value)
			{
				resolveExpression(tc, enumValue->value, valueType);
				if (!isConstant(enumValue->value))
				{
					error(tc, (Node*)enumValue->value, "Enum value must be constant");
				}
				else
				{
					constantFold(enumValue->value, &enumValue->intValue);
				}

				lastWithValue = j;
			}
			else
			{
				if (lastWithValue != -1)
				{
					int64_t previousValue = enum_->values[lastWithValue]->intValue;
					enumValue->intValue = previousValue + (j - lastWithValue);
				}
				else
				{
					enumValue->intValue = j;
				}
			}
		}

		resolveEnumType(enum_->enumType, valueType);
	}

	for (int i = 0; i < ast->numStructs; i++)
	{
		Struct* struct_ = ast->structs[i];

		int numFields = 0;
		int numOffsetFields = 0;

		int mark = tc->scratch->mark();

		for (int j = 0; j < struct_->numFields; j++)
		{
			resolveField(tc, struct_->fields[j]);
			for (int k = 0; k < struct_->fields[j]->numDeclarators; k++)
			{
				if (!struct_->fields[j]->declarators[k].hasOffset)
				{
					tc->scratch->add(struct_->fields[j]->variableType->inferredType);
					numFields++;
				}
			}
		}
		for (int j = 0; j < struct_->numFields; j++)
		{
			resolveField(tc, struct_->fields[j]);
			for (int k = 0; k < struct_->fields[j]->numDeclarators; k++)
			{
				if (struct_->fields[j]->declarators[k].hasOffset)
				{
					tc->scratch->add(struct_->fields[j]->variableType->inferredType);
					numOffsetFields++;
				}
			}
		}

		int mark2 = tc->scratch->mark();

		for (int j = 0; j < struct_->numFields; j++)
		{
			for (int k = 0; k < struct_->fields[j]->numDeclarators; k++)
			{
				if (!struct_->fields[j]->declarators[k].hasOffset)
				{
					if (struct_->fields[j]->declarators[k].name.length)
						tc->scratch->add(struct_->fields[j]->declarators[k].name);
					else
						tc->scratch->add(StringView{});
				}
			}
		}
		for (int j = 0; j < struct_->numFields; j++)
		{
			for (int k = 0; k < struct_->fields[j]->numDeclarators; k++)
			{
				if (struct_->fields[j]->declarators[k].hasOffset)
				{
					if (struct_->fields[j]->declarators[k].name.length)
						tc->scratch->add(struct_->fields[j]->declarators[k].name);
					else
						tc->scratch->add(StringView{});
				}
			}
		}

		int mark3 = tc->scratch->mark();

		for (int j = 0; j < struct_->numFields; j++)
		{
			for (int k = 0; k < struct_->fields[j]->numDeclarators; k++)
			{
				if (!struct_->fields[j]->declarators[k].hasOffset)
					tc->scratch->add(-1);
			}
		}
		for (int j = 0; j < struct_->numFields; j++)
		{
			for (int k = 0; k < struct_->fields[j]->numDeclarators; k++)
			{
				if (struct_->fields[j]->declarators[k].hasOffset)
					tc->scratch->add(struct_->fields[j]->declarators[k].offset);
			}
		}

		Type** fieldTypes = tc->scratch->getData<Type*>(mark);
		StringView* fieldNames = tc->scratch->getData<StringView>(mark2);
		int* fieldOffsets = tc->scratch->getData<int>(mark3);

		if (struct_->structType)
		{
			resolveNamedStructType(struct_->structType, numFields, numOffsetFields, fieldTypes, fieldNames, fieldOffsets, file);
		}

		tc->scratch->release(mark);
	}

	for (int i = 0; i < ast->numUnions; i++)
	{
		Union* union_ = ast->unions[i];

		int mark = tc->scratch->mark();

		for (int j = 0; j < union_->numFields; j++)
		{
			resolveField(tc, union_->fields[j]);
			for (int k = 0; k < union_->fields[j]->numDeclarators; k++)
				tc->scratch->add(union_->fields[j]->variableType->inferredType);
		}

		Type** fieldTypes = tc->scratch->getData<Type*>(mark);
		int numFields = tc->scratch->count<Type*>(mark);

		int mark2 = tc->scratch->mark();

		for (int j = 0; j < union_->numFields; j++)
		{
			for (int k = 0; k < union_->fields[j]->numDeclarators; k++)
			{
				if (union_->fields[j]->declarators[k].name.length)
					tc->scratch->add(union_->fields[j]->declarators[k].name);
				else
					tc->scratch->add(StringView{});
			}
		}

		StringView* fieldNames = tc->scratch->getData<StringView>(mark2);

		resolveNamedUnionType(union_->unionType, numFields, fieldTypes, fieldNames, file);

		tc->scratch->release(mark);
	}

	for (int i = 0; i < ast->numTypedefs; i++)
	{
		Typedef* typedef_ = ast->typedefs[i];

		Type* aliasType = resolveType(tc, typedef_->value);

		resolveAliasType(typedef_->aliasType, aliasType);
	}

	for (int i = 0; i < ast->numFunctions; i++)
	{
		Function* function = ast->functions[i];

		function->scope = pushScope(tc);

		int mark = tc->scratch->mark();

		bool variadic = false;
		for (int j = 0; j < function->numParams; j++)
		{
			if (function->params[j])
			{
				Type* paramType = resolveParameter(tc, function->params[j]);
				tc->scratch->add(paramType);

				insertSymbol(&function->scope->symbols, function->params[j]->name, SYMBOL_VARIABLE, (Node*)function->params[j], file->handle);

				if (function->params[j]->variadic)
				{
					if (j == function->numParams - 1)
						variadic = true;
					else
					{
						error(tc, (Node*)function->params[j], "Only the last parameter can be declared as variadic");
					}
				}
			}
			else
			{
				tc->scratch->add(getErrorType(tc->types));
			}
		}

		Type* returnType = nullptr;
		if (function->value)
		{
			Function* lastFunction = tc->currentFunction;
			tc->currentFunction = function;

			resolveExpression(tc, function->value);

			tc->currentFunction = lastFunction;

			returnType = function->value->inferredType;
		}
		else if (function->returnType)
		{
			resolveType(tc, function->returnType);
			returnType = function->returnType->inferredType;
		}

		function->functionType = getFunctionType(tc->types, returnType, function->numParams, tc->scratch->getData<Type*>(mark), variadic, tc->file);

		tc->scratch->release(mark);

		if (function->storage & (STORAGE_NOMANGLE | STORAGE_DLLEXPORT | STORAGE_DLLIMPORT | STORAGE_EXTERN))
			function->mangledName = copy(function->name);
		else
			function->mangledName = mangleFunctionName(tc->types, function->name, function->functionType, &file->arena);

		if (compareString(function->name, "main"))
		{
			if (returnType && returnType->typeKind != TYPE_INT32)
			{
				error(tc, (Node*)function->returnType, "Main function must either return 'int' or nothing");
			}
			if (function->numParams != 0)
			{
				Type* paramType = function->params[0]->paramType->inferredType;
				if (function->numParams > 1 || paramType->typeKind != TYPE_ARRAY || paramType->array.size || paramType->array.elementType->typeKind != TYPE_STRING)
					error(tc, (Node*)function->params[0], "Main function must either have arguments (string[]) or none");
			}
		}

		popScope(tc);
	}

	for (int i = 0; i < ast->numMacros; i++)
	{
		Macro* macro = ast->macros[i];

	}

	popScope(tc);
}

void typeCheckFunctions(TypeChecker* tc, File* file)
{
	tc->file = file;

	pushScope(tc, file->ast.globalScope);

	AST* ast = &file->ast;

	// Non primitive global initializers
	for (int i = 0; i < ast->numGlobalVariables; i++)
	{
		GlobalVariable* globalVariable = ast->globalVariables[i];
		for (int j = 0; j < globalVariable->numDeclarators; j++)
		{
			Expression* value = globalVariable->declarators[j].value;
			StringView name = globalVariable->declarators[j].name;
			if (value && !isPrimitiveType(globalVariable->variableType->inferredType))
			{
				resolveExpression(tc, value, globalVariable->variableType->inferredType);

				if (value->inferredType != getErrorType(tc->types) && !isAssignable(tc, value->inferredType, globalVariable->variableType->inferredType, &globalVariable->declarators[j].value))
				{
					error(tc, (Node*)value, "Can't assign value of type '%.*s' to global variable '%.*s' of type '%.*s'", value->inferredType->name.length, value->inferredType->name.ptr, name.length, name.ptr, globalVariable->variableType->inferredType->name.length, globalVariable->variableType->inferredType->name.ptr);
				}
				else if (value->inferredType != getErrorType(tc->types) && !isConstant(value))
				{
					error(tc, (Node*)value, "Global variable initializer must be a constant value");
				}
			}
		}
	}

	for (int i = 0; i < ast->numFunctions; i++)
	{
		Function* function = ast->functions[i];
		if (!function->value)
		{
			Function* lastFunction = tc->currentFunction;
			tc->currentFunction = function;

			pushScope(tc, function->scope);

			for (int i = 0; i < function->numStatements; i++)
			{
				if (function->statements[i])
					resolveStatement(tc, function->statements[i]);
			}

			popScope(tc);

			tc->currentFunction = lastFunction;
		}
	}

	popScope(tc);
}
