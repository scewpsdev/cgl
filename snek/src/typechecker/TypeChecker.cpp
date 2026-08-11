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


void initTypeChecker(TypeChecker* tc, Arena* arena, ScratchBuffer* scratch, Lexer* lexer, Diagnostics* diagnostics, TypeSystem* types)
{
	*tc = {};

	tc->arena = arena;
	tc->scratch = scratch;
	tc->lexer = lexer;
	tc->diagnostics = diagnostics;
	tc->types = types;
}

void destroyTypeChecker(TypeChecker* tc)
{
}

static void getSourceLocation(TypeChecker* tc, Node* node, SourceLocation* start, SourceLocation* end)
{
	*start = getSourceLocation(tc->lexer, node->start);
	*end = getSourceLocation(tc->lexer, node->end);
}

static void getSourceLocation(TypeChecker* tc, StringView str, SourceLocation* start, SourceLocation* end)
{
	*start = getSourceLocation(tc->lexer, (int)(str.ptr - tc->lexer->src));
	*end = getSourceLocation(tc->lexer, (int)(str.ptr - tc->lexer->src) + str.length);
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
	tc->currentFile = file;

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
				insertSymbol(&tc->currentScope->symbols, struct_->name, SYMBOL_TYPE, declaration, file->handle);
			}

			ast->structs[numStructs++] = struct_;
		}
		else if (declaration->type == NODE_UNION)
		{
			Union* union_ = &declaration->union_;

			union_->unionType = createNamedUnionType(file, union_->name, union_);

			insertSymbol(&tc->currentScope->symbols, union_->name, SYMBOL_TYPE, declaration, file->handle);

			ast->unions[numUnions++] = union_;
		}
		else if (declaration->type == NODE_ENUM)
		{
			Enum* enum_ = &declaration->enum_;

			enum_->enumType = createEnumType(file, enum_->name, enum_);

			insertSymbol(&tc->currentScope->symbols, enum_->name, SYMBOL_TYPE, declaration, file->handle);
			ast->enums[numEnums++] = enum_;
		}
		else if (declaration->type == NODE_TYPEDEF)
		{
			Typedef* typedef_ = &declaration->typedef_;

			typedef_->aliasType = createAliasType(file, typedef_->name, typedef_);

			insertSymbol(&tc->currentScope->symbols, typedef_->name, SYMBOL_TYPE, declaration, file->handle);

			ast->typedefs[numTypedefs++] = typedef_;
		}
		else if (declaration->type == NODE_FUNCTION)
		{
			insertSymbol(&tc->currentScope->symbols, declaration->function.name, SYMBOL_FUNCTION_SET, declaration, file->handle);
			ast->functions[numFunctions++] = &declaration->function;
		}
		else if (declaration->type == NODE_GLOBAL_VARIABLE)
		{
			for (int i = 0; i < declaration->globalVariable.numDeclarators; i++)
			{
				insertSymbol(&tc->currentScope->symbols, declaration->globalVariable.declarators[i].name, SYMBOL_VARIABLE, declaration, file->handle);
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

static uint64_t stringToIntConstant(TypeChecker* tc, Node* node, StringView str, bool* negative, Type** type)
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
				tc->scratch->add(nullptr);
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
						tc->scratch->add(nullptr);
				}
			}
			else
			{
				tc->scratch->add(nullptr);
			}
		}

		StringView* fieldNames = tc->scratch->getData<StringView>(mark2);

		type->inferredType = getAnonymousStructType(tc->types, structType->numFields, fieldTypes, fieldNames, tc->currentFile);

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
				for (int j = 0; j < unionType->fields[i]->numDeclarators; j++)
					tc->scratch->add(unionType->fields[i]->variableType->inferredType);
			}
			else
			{
				tc->scratch->add(nullptr);
			}
		}

		Type** fieldTypes = tc->scratch->getData<Type*>(mark);

		int mark2 = tc->scratch->mark();

		for (int i = 0; i < unionType->numFields; i++)
		{
			if (unionType->fields[i])
			{
				for (int j = 0; j < unionType->fields[i]->numDeclarators; j++)
				{
					if (unionType->fields[i]->declarators[j].name.length)
						tc->scratch->add(unionType->fields[i]->declarators[j].name);
					else
						tc->scratch->add(nullptr);
				}
			}
			else
			{
				tc->scratch->add(nullptr);
			}
		}

		StringView* fieldNames = tc->scratch->getData<StringView>(mark2);

		type->inferredType = getAnonymousUnionType(tc->types, unionType->numFields, fieldTypes, fieldNames, tc->currentFile);

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

		return type->inferredType = getPointerType(tc->types, elementType, tc->currentFile);
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

		return type->inferredType = getOptionalType(tc->types, elementType, tc->currentFile);
	}
	else if (type->type == NODE_FUNCTION_TYPE)
	{
		FunctionType* functionType = (FunctionType*)type;

		int mark = tc->scratch->mark();

		for (int j = 0; j < functionType->numParams; j++)
		{
			if (functionType->params[j])
			{
				resolveParameter(tc, functionType->params[j]);
				tc->scratch->add(functionType->params[j]->paramType->inferredType);
			}
		}

		Type* returnType = nullptr;
		if (functionType->returnType)
		{
			resolveType(tc, functionType->returnType);
			returnType = functionType->returnType->inferredType;
		}

		type->inferredType = getFunctionType(tc->types, returnType, functionType->numParams, tc->scratch->getData<Type*>(mark), tc->currentFile);

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
				tc->scratch->add(nullptr);
			}
		}

		Type** elementTypes = tc->scratch->getData<Type*>(mark);
		type->inferredType = getAnonymousStructType(tc->types, tupleType->numElementTypes, elementTypes, nullptr, tc->currentFile);

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

		return type->inferredType = getArrayType(tc->types, elementType, size, tc->currentFile);
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

	for (int i = 0; i < tc->currentFile->dependencies.size; i++)
	{
		FileHandle dependency = tc->currentFile->dependencies[i];
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

static Type* typeCheckArithmeticOperator(TypeChecker* tc, uint8_t op, Type* left, Type* right, Expression* expression, Expression** leftNode, Expression** rightNode)
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
	expressionType = unwrapType(expressionType);
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

static bool isAssignable(TypeChecker* tc, Type* expressionType, Type* targetType, Expression** ref)
{
	if (compareTypes(expressionType, targetType))
		return true;

	if (isNumericType(expressionType) && isNumericType(targetType))
	{
		if (getNumericRank(expressionType) <= getNumericRank(targetType))
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

	if (expressionType->typeKind == TYPE_POINTER && targetType->typeKind == TYPE_POINTER)
	{
		if (targetType->pointer.elementType->typeKind == TYPE_VOID)
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

static SymbolHandle getSymbolHandle(TypeChecker* tc, Symbol* symbol)
{
	SymbolHandle handle = {};
	handle.file = symbol->file;
	handle.symbol = symbol->key;
	return handle;
}

static bool canCoerceType(Expression* arg, Type* targetType)
{
	if (arg->type == NODE_INT_LITERAL && isIntegerType(targetType))
		return true;
	if (arg->type == NODE_STRING_LITERAL && (targetType->typeKind == TYPE_STRING || targetType->typeKind == TYPE_POINTER && targetType->pointer.elementType->typeKind == TYPE_INT8))
		return true;
	return false;
}

static int argScore(TypeChecker* tc, Type* argType, Type* paramType, Expression* arg)
{
	if (compareTypes(argType, paramType))
		return 0;
	else if (canCoerceType(arg, paramType))
		return 1;
	else if (isAssignable(tc, argType, paramType, nullptr))
		return 2;
	return 999;
}

static Type* resolveIdentifier(TypeChecker* tc, Identifier* identifier, bool hasArgs, int numArgs, Expression** args)
{
	if (Symbol* symbol = resolveSymbol(tc, identifier->name))
	{
		if (symbol->file == tc->currentFile->handle)
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
					paramType = getArrayType(tc->types, paramType, 0, tc->currentFile);

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
			FunctionOverload* bestOverload = nullptr;

			if (symbol->functionSet.count == 1)
			{
				bestOverload = &symbol->functionSet.overloads[0];

				SnekAssert(bestOverload->declaration->type == NODE_FUNCTION);
			}
			else
			{
				if (hasArgs)
				{
					int bestScore = 9999;
					bool ambiguous = false;

					for (int i = 0; i < symbol->functionSet.count; i++)
					{
						FunctionOverload* overload = &symbol->functionSet.overloads[i];
						Function* function = overload->declaration;
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
							if (score > 2)
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
								bestOverload = overload;
								ambiguous = false;
							}
							else if (currentScore == bestScore)
							{
								ambiguous = true;
							}
						}
					}

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
					else if (!bestOverload)
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

			identifier->functionOverload = bestOverload;

			if (!bestOverload->declaration->functionType)
			{
				error(tc, (Node*)identifier, "Function '%.*s' referenced in type-defining inline expression must be declared above", identifier->name.length, identifier->name.ptr);
				return identifier->inferredType = &tc->types->errorType;
			}
			else
			{
				return identifier->inferredType = bestOverload->declaration->functionType;
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

		intLiteral->intValue = stringToIntConstant(tc, (Node*)intLiteral, intLiteral->value, &intLiteral->negative, &intType);

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
			return expression->inferredType = getPointerType(tc->types, &tc->types->primitiveTypes[TYPE_INT8], tc->currentFile);
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
		return expression->inferredType = &tc->types->primitiveTypes[TYPE_VOID];
	}
	else if (expression->type == NODE_IDENTIFIER)
	{
		Identifier* identifier = (Identifier*)expression;
		return resolveIdentifier(tc, identifier, false, 0, nullptr);
	}
	else if (expression->type == NODE_COMPOUND_EXPRESSION)
	{
		CompoundExpression* compound = (CompoundExpression*)expression;
		resolveExpression(tc, compound->value);
		return expression->inferredType = compound->value->inferredType;
	}
	else if (expression->type == NODE_EXPRESSION_LIST)
	{
		ExpressionList* expressionList = (ExpressionList*)expression;

		int mark = tc->scratch->mark();

		for (int i = 0; i < expressionList->numValues; i++)
		{
			if (expressionList->values[i])
			{
				resolveExpression(tc, expressionList->values[i]);
				tc->scratch->add(expressionList->values[i]);
			}
			else
			{
				tc->scratch->add(nullptr);
			}
		}

		Type** valueTypes = tc->scratch->getData<Type*>(mark);

		expression->inferredType = getAnonymousStructType(tc->types, expressionList->numValues, valueTypes, nullptr, tc->currentFile);

		tc->scratch->release(mark);

		return expression->inferredType;
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

		if (left->typeKind == TYPE_ENUM && left->enum_.valueType == right)
		{
			left = left->enum_.valueType;
			insertImplicitCast(tc, &binaryOperator->left, right);
		}
		else if (right->typeKind == TYPE_ENUM && right->enum_.valueType == left)
		{
			right = right->enum_.valueType;
			insertImplicitCast(tc, &binaryOperator->right, left);
		}

		// todo check operator overload

		if (binaryOperator->op == OPERATOR_ADD || binaryOperator->op == OPERATOR_SUBTRACT || binaryOperator->op == OPERATOR_MULTIPLY || binaryOperator->op == OPERATOR_DIVIDE || binaryOperator->op == OPERATOR_MODULO)
			return binaryOperator->inferredType = typeCheckArithmeticOperator(tc, binaryOperator->op, left, right, binaryOperator, &binaryOperator->left, &binaryOperator->right);
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

			return unaryOperator->inferredType = getPointerType(tc->types, operandType, tc->currentFile);
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

		if (functionCall->expression->type == NODE_IDENTIFIER)
			resolveIdentifier(tc, (Identifier*)functionCall->expression, true, functionCall->numArgs, functionCall->args);
		else
			resolveExpression(tc, functionCall->expression);

		Type* operandType = functionCall->expression->inferredType;

		for (int i = 0; i < functionCall->numArgs; i++)
		{
			resetExpression(&functionCall->args[i]);
			resolveExpression(tc, functionCall->args[i], operandType->function.paramTypes[i]);
		}

		if (operandType == &tc->types->errorType)
		{
			return expression->inferredType = &tc->types->errorType;
		}
		else if (operandType->typeKind != TYPE_FUNCTION)
		{
			error(tc, (Node*)functionCall->expression, "Operand of function call must be of type function");
			return expression->inferredType = &tc->types->errorType;
		}
		else
		{
			if (functionCall->numArgs != operandType->function.numParams)
			{
				error(tc, (Node*)functionCall, "Incorrect number of function arguments: %d, should be %d", functionCall->numArgs, operandType->function.numParams);
			}

			for (int i = 0; i < functionCall->numArgs; i++)
			{
				Type* argType = functionCall->args[i]->inferredType;
				if (argType == &tc->types->errorType)
					continue;

				if (i < operandType->function.numParams)
				{
					Type* paramType = operandType->function.paramTypes[i];
					if (!isAssignable(tc, argType, paramType, &functionCall->args[i]))
					{
						error(tc, (Node*)functionCall->args[i], "Cannot pass value of type '%.*s' to function parameter of type '%.*s'", argType->name.length, argType->name.ptr, paramType->name.length, paramType->name.ptr);
					}
				}
			}

			return expression->inferredType = operandType->function.returnType;
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

		Type* operandType = resolveExpression(tc, member->operand);

		if (operandType == &tc->types->errorType)
		{
			return expression->inferredType = &tc->types->errorType;
		}

		if (operandType->typeKind == TYPE_POINTER)
			operandType = operandType->pointer.elementType;

		if (operandType->typeKind == TYPE_STRUCT)
		{
			if (member->index == -1)
				member->index = getFieldIndex(member->name, operandType->struct_.numFields, operandType->struct_.fieldNames);
			if (member->index == -1)
			{
				error(tc, member->name, "Undefined struct field '%.*s.%.*s'", operandType->name.length, operandType->name.ptr, member->name.length, member->name.ptr);
				return expression->inferredType = &tc->types->errorType;
			}

			return expression->inferredType = operandType->struct_.fieldTypes[member->index];
		}
		else if (operandType->typeKind == TYPE_UNION)
		{
			if (member->index == -1)
				member->index = getFieldIndex(member->name, operandType->union_.numFields, operandType->union_.fieldNames);
			if (member->index == -1)
			{
				error(tc, member->name, "Undefined union field '%.*s.%.*s'", operandType->name.length, operandType->name.ptr, member->name.length, member->name.ptr);
				return expression->inferredType = &tc->types->errorType;
			}

			return expression->inferredType = operandType->union_.fieldTypes[member->index];
		}
		else if (operandType->typeKind == TYPE_STRING)
		{
			if (compareString(member->name, "length"))
			{
				member->index = 0;
				return expression->inferredType = &tc->types->primitiveTypes[TYPE_UINT64];
			}
			else if (compareString(member->name, "ptr"))
			{
				member->index = 1;
				return expression->inferredType = getPointerType(tc->types, &tc->types->primitiveTypes[TYPE_INT8], tc->currentFile);
			}
			else
			{
				error(tc, member->name, "Undefined string field '%.*s.%.*s'", operandType->name.length, operandType->name.ptr, member->name.length, member->name.ptr);
				return expression->inferredType = &tc->types->errorType;
			}
		}
		else if (operandType->typeKind == TYPE_ARRAY)
		{
			if (compareString(member->name, "length"))
			{
				member->index = 0;
				return expression->inferredType = &tc->types->primitiveTypes[TYPE_UINT64];
			}
			else if (compareString(member->name, "data"))
			{
				member->index = 1;
				return expression->inferredType = getPointerType(tc->types, operandType->array.elementType, tc->currentFile);
			}
			else
			{
				error(tc, member->name, "Undefined array field '%.*s.%.*s'", operandType->name.length, operandType->name.ptr, member->name.length, member->name.ptr);
				return expression->inferredType = &tc->types->errorType;
			}
		}
		else if (operandType->typeKind == TYPE_ANY)
		{
			if (compareString(member->name, "type"))
			{
				member->index = 0;
				return expression->inferredType = &tc->types->primitiveTypes[TYPE_INT32];
			}
			else
			{
				error(tc, member->name, "Undefined any field '%.*s.%.*s'", operandType->name.length, operandType->name.ptr, member->name.length, member->name.ptr);
				return expression->inferredType = &tc->types->errorType;
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
				return expression->inferredType = &tc->types->errorType;
			}

			if (symbol->declaration->type == NODE_ENUM)
			{
				Enum* enum_ = &symbol->declaration->enum_;
				int index = getEnumValue(member->name, enum_->numValues, enum_->values);
				member->index = index;

				if (index != -1)
				{
					return expression->inferredType = enum_->enumType;
				}
				else
				{
					error(tc, member->name, "Undefined enum value '%.*s'", member->name.length, member->name.ptr);
					return expression->inferredType = &tc->types->errorType;
				}
			}
			else
			{
				error(tc, (Node*)member->operand, "Undefined namespace '%.*s'", typeName->name.length, typeName->name.ptr);
				return expression->inferredType = &tc->types->errorType;
			}
		}
		else
		{
			error(tc, (Node*)member->operand, "Operand of member access must be one of struct, union, string, array, any");
			return expression->inferredType = &tc->types->errorType;
		}
	}
	else if (expression->type == NODE_TERNARY_CONDITION)
	{
		TernaryCondition* ternary = (TernaryCondition*)expression;

		Type* conditionType = resolveExpression(tc, ternary->condition);
		Type* thenType = resolveExpression(tc, ternary->then);
		Type* elseType = resolveExpression(tc, ternary->else_);

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

		if (targetType == &tc->types->errorType || expressionType == &tc->types->errorType)
		{
			return expression->inferredType = &tc->types->errorType;
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

		Type* startValueType = resolveExpression(tc, for_->startValue);
		Type* compareValueType = resolveExpression(tc, for_->compareValue);

		if (!isNumericType(startValueType))
		{
			error(tc, (Node*)for_->startValue, "Initial value of for iterator must be a scalar");
		}
		if (!isNumericType(compareValueType))
		{
			error(tc, (Node*)for_->compareValue, "Comparison value of for iterator must be a scalar");
		}

		insertSymbol(&for_->scope->symbols, for_->iteratorName, SYMBOL_VARIABLE, (Node*)statement, tc->currentFile->handle);

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

				if (!isAssignable(tc, valueType, returnType, &return_->value))
				{
					error(tc, (Node*)return_->value, "Cannot return value of type '%.*s' from function with return type '%.*s'", valueType->name.length, valueType->name.ptr, returnType->name.length, returnType->name.ptr);
				}
			}
			else
			{
				if (returnType)
				{
					error(tc, (Node*)return_->value, "Must return value of type '%.*s'", returnType->name.length, returnType->name.ptr);
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
				insertSymbol(&tc->currentScope->symbols, variableDeclaration->declarators[i].name, SYMBOL_VARIABLE, (Node*)statement, tc->currentFile->handle);
			}

			if (variableDeclaration->declarators[i].value)
			{
				Type* initializerType = resolveExpression(tc, variableDeclaration->declarators[i].value, variableType);
				if (!isAssignable(tc, initializerType, variableType, &variableDeclaration->declarators[i].value))
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
		Type* valueType = resolveExpression(tc, assignment->value);

		if (!isLValue(assignment->expression))
		{
			error(tc, (Node*)assignment->expression, "Cannot assign value to non-lvalue expression");
		}

		if (assignment->op == OPERATOR_ASSIGN)
		{
			if (!isAssignable(tc, valueType, expressionType, &assignment->value))
			{
				error(tc, (Node*)assignment->value, "Cannot assign value of type '%.*s' to expression of type '%.*s'", valueType->name.length, valueType->name.ptr, expressionType->name.length, expressionType->name.ptr);
			}
		}
		else
		{
			OperatorType op = assignmentOperatorToBinary(assignment->op);
			Type* resultType = typeCheckArithmeticOperator(tc, op, expressionType, valueType, assignment->value, nullptr, &assignment->value);

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
		paramType = getArrayType(tc->types, paramType, 0, tc->currentFile);
	return paramType;
}

void symbolResolution(TypeChecker* tc, File* file)
{
	tc->currentFile = file;

	AST* ast = &file->ast;

	pushScope(tc, file->ast.globalScope);

	for (int i = 0; i < ast->numGlobalVariables; i++)
	{
		GlobalVariable* globalVariable = ast->globalVariables[i];
		resolveType(tc, globalVariable->variableType);
		for (int j = 0; j < globalVariable->numDeclarators; j++)
		{
			if (globalVariable->declarators[j].value)
			{
				resolveExpression(tc, globalVariable->declarators[j].value);

				if (!isConstant(globalVariable->declarators[j].value))
				{
					error(tc, (Node*)globalVariable, "Global variable initializer must be a constant value");
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
				resolveExpression(tc, enumValue->value);
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

		int mark = tc->scratch->mark();

		for (int j = 0; j < struct_->numFields; j++)
		{
			resolveField(tc, struct_->fields[j]);
			for (int k = 0; k < struct_->fields[j]->numDeclarators; k++)
				tc->scratch->add(struct_->fields[j]->variableType->inferredType);
		}

		int numFields = tc->scratch->count<Type*>(mark);

		int mark2 = tc->scratch->mark();

		for (int j = 0; j < struct_->numFields; j++)
		{
			for (int k = 0; k < struct_->fields[j]->numDeclarators; k++)
			{
				if (struct_->fields[j]->declarators[k].name.length)
					tc->scratch->add(struct_->fields[j]->declarators[k].name);
				else
					tc->scratch->add(nullptr);
			}
		}

		Type** fieldTypes = tc->scratch->getData<Type*>(mark);
		StringView* fieldNames = tc->scratch->getData<StringView>(mark2);

		if (struct_->structType)
		{
			resolveNamedStructType(struct_->structType, numFields, fieldTypes, fieldNames, file);
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
					tc->scratch->add(nullptr);
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

		for (int j = 0; j < function->numParams; j++)
		{
			if (function->params[j])
			{
				resolveParameter(tc, function->params[j]);

				insertSymbol(&function->scope->symbols, function->params[j]->name, SYMBOL_VARIABLE, (Node*)function->params[j], file->handle);

				if (function->params[j]->type)
					tc->scratch->add(function->params[j]->paramType->inferredType);
				else
					tc->scratch->add(nullptr);
			}
			else
			{
				tc->scratch->add(nullptr);
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

		function->functionType = getFunctionType(tc->types, returnType, function->numParams, tc->scratch->getData<Type*>(mark), tc->currentFile);

		tc->scratch->release(mark);

		/*
		else
		{
			for (int j = 0; j < function->numStatements; j++)
			{
				if (function->statements[j])
					resolveStatement(tc, function->statements[j]);
			}
		}
		*/

		popScope(tc);
	}

	for (int i = 0; i < ast->numMacros; i++)
	{
		Macro* macro = ast->macros[i];

	}

	popScope(tc);
}

void typeCheckFunction(TypeChecker* tc, Function* function, File* file)
{
	tc->currentFile = file;

	pushScope(tc, file->ast.globalScope);

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

	popScope(tc);
}
