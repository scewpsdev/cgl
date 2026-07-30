#include "TypeChecker.h"

#include "TypeSystem.h"

#include "parser/AST.h"
#include "parser/Diagnostics.h"
#include "parser/Lexer.h"

#include <stdarg.h>
#include <math.h>


void initTypeChecker(TypeChecker* tc, Arena* arena, Lexer* lexer, Diagnostics* diagnostics, TypeSystem* types)
{
	tc->arena = arena;
	tc->lexer = lexer;
	tc->diagnostics = diagnostics;
	tc->types = types;

	initScratchBuffer(&tc->scratch, 16);
}

void destroyTypeChecker(TypeChecker* tc)
{
	destroyScratchBuffer(&tc->scratch);
}

static void getSourceLocation(TypeChecker* tc, Node* node, SourceLocation* start, SourceLocation* end)
{
	*start = getSourceLocation(tc->lexer, node->start);
	*end = getSourceLocation(tc->lexer, node->end);
}

static void getSourceLocation(TypeChecker* tc, StringView str, SourceLocation* start, SourceLocation* end)
{
	*start = getSourceLocation(tc->lexer, (int)(str.ptr - tc->lexer->src));
	*start = getSourceLocation(tc->lexer, (int)(str.ptr - tc->lexer->src) + str.length);
}

static void error(TypeChecker* tc, Node* node, const char* fmt, ...)
{
	if (!tc->diagnostics) return;

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

static Scope* pushScope(TypeChecker* tc)
{
	Scope* scope = tc->arena->alloc<Scope>();
	scope->parent = tc->currentScope;

	bool isGlobal = false;
	initSymbolTable(&scope->symbols, isGlobal ? 1024 : 16, tc->arena);

	tc->currentScope = scope;
}

static void popScope(TypeChecker* tc)
{
	tc->currentScope = tc->currentScope->parent;
}

void symbolCollection(TypeChecker* tc, AST* ast)
{
	//ast->globalScope = arena->alloc<Scope>();
	//initScope(ast->globalScope, nullptr, true, arena);
	//parser->currentScope = ast->globalScope;

	ast->structs = tc->arena->alloc<Struct*>(ast->numStructs);
	ast->enums = tc->arena->alloc<Enum*>(ast->numEnums);
	ast->unions = tc->arena->alloc<Union*>(ast->numUnions);
	ast->typedefs = tc->arena->alloc<Typedef*>(ast->numTypedefs);
	ast->functions = tc->arena->alloc<Function*>(ast->numFunctions);
	ast->macros = tc->arena->alloc<Macro*>(ast->numMacros);
	ast->globalVariables = tc->arena->alloc<GlobalVariable*>(ast->numGlobalVariables);

	int numStructs = 0;
	int numEnums = 0;
	int numUnions = 0;
	int numTypedefs = 0;
	int numFunctions = 0;
	int numMacros = 0;
	int numGlobalVariables = 0;

	for (int i = 0; i < ast->numDeclarations; i++)
	{
		Node* declaration = ast->declarations[i];

		StringView identifier = {};
		if (declaration->type == NODE_STRUCT)
		{
			insertSymbol(&tc->currentScope->symbols, declaration->struct_.name, SYMBOL_TYPE, declaration);
			ast->structs[numStructs++] = &declaration->struct_;
		}
		else if (declaration->type == NODE_ENUM)
		{
			insertSymbol(&tc->currentScope->symbols, declaration->enum_.name, SYMBOL_TYPE, declaration);
			ast->enums[numEnums++] = &declaration->enum_;
		}
		else if (declaration->type == NODE_UNION)
		{
			insertSymbol(&tc->currentScope->symbols, declaration->union_.name, SYMBOL_TYPE, declaration);
			ast->unions[numUnions++] = &declaration->union_;
		}
		else if (declaration->type == NODE_TYPEDEF)
		{
			insertSymbol(&tc->currentScope->symbols, declaration->typedef_.name, SYMBOL_TYPE, declaration);
			ast->typedefs[numTypedefs++] = &declaration->typedef_;
		}
		else if (declaration->type == NODE_FUNCTION)
		{
			insertSymbol(&tc->currentScope->symbols, declaration->function.name, SYMBOL_FUNCTION_SET, declaration);
			ast->functions[numFunctions++] = &declaration->function;
		}
		else if (declaration->type == NODE_GLOBAL_VARIABLE)
		{
			for (int i = 0; i < declaration->globalVariable.numDeclarators; i++)
			{
				insertSymbol(&tc->currentScope->symbols, declaration->globalVariable.declarators[i].name, SYMBOL_VARIABLE, declaration);
			}
			ast->globalVariables[numGlobalVariables++] = &declaration->globalVariable;
		}
		else if (declaration->type == NODE_MACRO)
		{
			insertSymbol(&tc->currentScope->symbols, declaration->macro.name, SYMBOL_MACRO, declaration);
			ast->macros[numMacros++] = &declaration->macro;
		}
	}
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
	int i = 0;
	int base = 10;

	if (str[0] == '-')
	{
		*negative = true;
		i++;
	}

	if (str.length >= i + 2 && str[i] == '0')
	{
		if (str[i + 1] == 'x')
		{
			base = 16;
			i += 2;
		}
		else if (str[i + 1] == 'b')
		{
			base = 2;
			i += 2;
		}
		else if (str[i + 1] == 'o')
		{
			base = 8;
			i += 2;
		}
	}

	uint64_t value = 0;
	int digitCount = 0;

	for (; i < str.length; i++)
	{
		char c = str[i];

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
	else if (str.length - i == 1 && strncmp(&str[i], "u", 1) == 0) *type = &tc->types->primitiveTypes[TYPE_UINT32];
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
	else
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

static bool isConstant(Expression* expression)
{
	if (!expression)
		return false;

	if (expression->type == NODE_INT_LITERAL)
	{
		return true;
	}
	else if (expression->type == NODE_FLOAT_LITERAL)
	{
		return true;
	}
	else if (expression->type == NODE_STRING_LITERAL)
	{
		return true;
	}
	else if (expression->type == NODE_CHAR_LITERAL)
	{
		return true;
	}
	else if (expression->type == NODE_TRUE)
	{
		return true;
	}
	else if (expression->type == NODE_FALSE)
	{
		return true;
	}
	else if (expression->type == NODE_NULL_LITERAL)
	{
		return true;
	}
	else if (expression->type == NODE_IDENTIFIER)
	{
		return false;
	}
	else if (expression->type == NODE_COMPOUND_EXPRESSION)
	{
		return isConstant(((CompoundExpression*)expression)->value);
	}
	else if (expression->type == NODE_EXPRESSION_LIST)
	{
		ExpressionList* expressionList = (ExpressionList*)expression;
		for (int i = 0; i < expressionList->numValues; i++)
		{
			if (!isConstant(expressionList->values[i]))
				return false;
		}
		return true;
	}
	else if (expression->type == NODE_BINARY_OPERATOR)
	{
		BinaryOperator* binaryOperator = (BinaryOperator*)expression;
		return isConstant(binaryOperator->left) && isConstant(binaryOperator->right);
	}
	else if (expression->type == NODE_CAST)
	{
		return isConstant(((Cast*)expression)->expression);
	}
	else if (expression->type == NODE_UNARY_OPERATOR)
	{
		return false;
	}
	else if (expression->type == NODE_FUNCTION_CALL)
	{
		return false;
	}
	else if (expression->type == NODE_ARRAY_SUBSCRIPT)
	{
		return false;
	}
	else if (expression->type == NODE_MEMBER_ACCESS)
	{
		return false;
	}
	else if (expression->type == NODE_TERNARY_CONDITION)
	{
		TernaryCondition* ternary = (TernaryCondition*)expression;
		return isConstant(ternary->condition) && isConstant(ternary->then) && isConstant(ternary->else_);
	}

	SnekAssert(false);
	return false;
}

static bool isLValue(Expression* expression)
{
	if (expression->type == NODE_IDENTIFIER)
	{
		return true;
	}
	else if (expression->type == NODE_MEMBER_ACCESS)
	{
		return true;
	}
	else if (expression->type == NODE_ARRAY_SUBSCRIPT)
	{
		return true;
	}
	else if (expression->type == NODE_UNARY_OPERATOR)
	{
		UnaryOperator* unaryOperator = (UnaryOperator*)expression;

		if (unaryOperator->op == OPERATOR_DEREFERENCE)
			return true;
		else if (unaryOperator->op == OPERATOR_INCREMENT_PREFIX || unaryOperator->op == OPERATOR_DECREMENT_PREFIX)
			return true;
	}
	else if (expression->type == NODE_COMPOUND_EXPRESSION)
	{
		CompoundExpression* compound = (CompoundExpression*)expression;
		return isLValue(compound->value);
	}

	return false;
}

static Type* resolveExpression(TypeChecker* tc, Expression* expression);
static Type* resolveField(TypeChecker* tc, Field* field);
static Type* resolveParameter(TypeChecker* tc, Parameter* parameter);

static Type* resolveType(TypeChecker* tc, TypeNode* type)
{
	if (type->type == NODE_PRIMITIVE_TYPE)
	{
		type->inferredType = &tc->types->primitiveTypes[type->typeKind];
		SnekAssert(type->inferredType->typeKind == type->typeKind);
	}
	else if (type->type == NODE_STRUCT_TYPE)
	{
		StructType* structType = (StructType*)type;

		int mark = tc->scratch.mark();

		for (int i = 0; i < structType->numFields; i++)
		{
			if (structType->fields[i])
			{
				resolveField(tc, structType->fields[i]);
				for (int j = 0; j < structType->fields[i]->numDeclarators; j++)
					tc->scratch.add(structType->fields[i]->inferredType);
			}
			else
			{
				tc->scratch.add(nullptr);
			}
		}

		Type** fieldTypes = tc->scratch.getData<Type*>(mark);
		type->inferredType = getAnonymousStructType(tc->types, structType->numFields, fieldTypes);

		tc->scratch.release(mark);
	}
	else if (type->type == NODE_UNION_TYPE)
	{
		UnionType* unionType = (UnionType*)type;

		int mark = tc->scratch.mark();

		for (int i = 0; i < unionType->numFields; i++)
		{
			if (unionType->fields[i])
			{
				resolveField(tc, unionType->fields[i]);
				for (int j = 0; j < unionType->fields[i]->numDeclarators; j++)
					tc->scratch.add(unionType->fields[i]->inferredType);
			}
			else
			{
				tc->scratch.add(nullptr);
			}
		}

		Type** fieldTypes = tc->scratch.getData<Type*>(mark);
		type->inferredType = getAnonymousStructType(tc->types, unionType->numFields, fieldTypes);

		tc->scratch.release(mark);
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

		type->inferredType = getPointerType(tc->types, elementType);
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

		type->inferredType = getOptionalType(tc->types, elementType);
	}
	else if (type->type == NODE_FUNCTION_TYPE)
	{
		FunctionType* functionType = (FunctionType*)type;

		int mark = tc->scratch.mark();

		for (int j = 0; j < functionType->numParams; j++)
		{
			if (functionType->params[j])
			{
				resolveParameter(tc, functionType->params[j]);
				if (functionType->params[j]->type)
					tc->scratch.add(functionType->params[j]->type->inferredType);
				else
					tc->scratch.add(nullptr);
			}
			else
			{
				tc->scratch.add(nullptr);
			}
		}

		Type* returnType = nullptr;
		if (functionType->returnType)
		{
			resolveType(tc, functionType->returnType);
			returnType = functionType->returnType->inferredType;
		}

		type->inferredType = getFunctionType(tc->types, returnType, functionType->numParams, tc->scratch.getData<Type*>(mark));

		tc->scratch.release(mark);
	}
	else if (type->type == NODE_TUPLE_TYPE)
	{
		TupleType* tupleType = (TupleType*)type;

		int mark = tc->scratch.mark();

		for (int i = 0; i < tupleType->numElementTypes; i++)
		{
			if (tupleType->elementTypes[i])
			{
				resolveType(tc, tupleType->elementTypes[i]);
				tc->scratch.add(tupleType->elementTypes[i]->inferredType);
			}
			else
			{
				tc->scratch.add(nullptr);
			}
		}

		Type** elementTypes = tc->scratch.getData<Type*>(mark);
		type->inferredType = getAnonymousStructType(tc->types, tupleType->numElementTypes, elementTypes);

		tc->scratch.release(mark);
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

		uint64_t size = 0;
		if (arrayType->size)
		{
			resolveExpression(tc, arrayType->size);

			//if (isConstant(arrayType->size))
			if (arrayType->size->type == NODE_INT_LITERAL)
			{
				//size = constantFold(arrayType->size);
				IntLiteral* sizeNode = (IntLiteral*)arrayType->size;

				bool negative;
				Type* sizeType;
				size = stringToIntConstant(tc, (Node*)sizeNode, sizeNode->value, &negative, &sizeType);

				if (negative)
				{
					error(tc, (Node*)arrayType->size, "Array size cannot be negative");
				}
			}
			else
			{
				error(tc, (Node*)arrayType->size, "Array size must be a constant expression");
			}
		}

		type->inferredType = getArrayType(tc->types, elementType, size);
	}
	else
	{
		SnekAssert(false);
	}

	SnekAssert(type->inferredType);
}

static Node* resolveSymbol(TypeChecker* tc, StringView identifier)
{
	Scope* scope = tc->currentScope;
	while (scope)
	{
		if (SymbolEntry* symbol = lookupSymbol(&scope->symbols, identifier))
		{
			if (symbol->type == SYMBOL_FUNCTION_SET)
			{
				if (symbol->functionSet.count == 1)
					return symbol->functionSet.overloads[0];
				else
				{
					error(tc, identifier, "Ambiguous identifier, multiple function overloads with name '%.*s'", identifier.length, identifier.ptr);
					return nullptr;
				}
			}
			else
			{
				return symbol->declaration;
			}
		}
		scope = scope->parent;
	}
	return nullptr;
}

static bool isIntegerType(Type* type)
{
	return type->typeKind > TYPE_INT_START && type->typeKind < TYPE_INT_END;
}

static bool isFloatingPointType(Type* type)
{
	return type->typeKind > TYPE_FLOAT_START || type->typeKind < TYPE_FLOAT_END;
}

static bool isTruthyType(Type* type)
{
	return type->typeKind == TYPE_BOOL || isIntegerType(type) || isFloatingPointType(type) || type->typeKind == TYPE_POINTER || type->typeKind == TYPE_OPTIONAL;
}

static int getFieldIndex(StringView name, int numFields, char** fieldNames)
{
	for (int i = 0; i < numFields; i++)
	{
		if (compareString(name, fieldNames[i]))
		{
			return i;
		}
	}
	return -1;
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

	ImplicitCast* cast = tc->arena->alloc<ImplicitCast>();
	initNode((Node*)cast, NODE_CAST_IMPLICIT, value->start);
	cast->expression = value;
	cast->targetType = type;
	cast->inferredType = type;
	cast->end = value->end;

	*node = cast;
}

static Type* typeCheckArithmeticOperator(TypeChecker* tc, BinaryOperator* expression, Type* left, Type* right)
{
	if (expression->op == OPERATOR_ADD || expression->op == OPERATOR_SUBTRACT)
	{
		if (left->typeKind == TYPE_POINTER && isIntegerType(right))
		{
			insertImplicitCast(tc, &expression->right, &tc->types->primitiveTypes[TYPE_INT64]);
			return left;
		}
		if (expression->op == OPERATOR_SUBTRACT && left->typeKind == TYPE_POINTER && right->typeKind == TYPE_POINTER)
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
		insertImplicitCast(tc, &expression->left, commonType);
	if (right != commonType)
		insertImplicitCast(tc, &expression->right, commonType);

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
		error(tc, (Node*)expression, "Cannot compare incompatible types");
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
		error(tc, (Node*)(!isTruthyType(left) ? expression->left : expression->right), "Operand of logical operator must be a scalar");
		return &tc->types->errorType;
	}

	if (left->typeKind != TYPE_BOOL)
		insertImplicitCast(tc, &expression->left, &tc->types->primitiveTypes[TYPE_BOOL]);
	if (right->typeKind != TYPE_BOOL)
		insertImplicitCast(tc, &expression->right, &tc->types->primitiveTypes[TYPE_BOOL]);

	return &tc->types->primitiveTypes[TYPE_BOOL];
}

static Type* resolveExpression(TypeChecker* tc, Expression* expression)
{
	if (expression->type == NODE_INT_LITERAL)
	{
		IntLiteral* intLiteral = (IntLiteral*)expression;
		intLiteral->intValue = stringToIntConstant(tc, (Node*)intLiteral, intLiteral->value, &intLiteral->negative, &intLiteral->inferredType);
	}
	else if (expression->type == NODE_FLOAT_LITERAL)
	{
		FloatLiteral* floatLiteral = (FloatLiteral*)expression;
		floatLiteral->floatValue = stringToFloatConstant(tc, (Node*)floatLiteral, floatLiteral->value, &floatLiteral->inferredType);
	}
	else if (expression->type == NODE_STRING_LITERAL)
	{
		expression->inferredType = &tc->types->primitiveTypes[TYPE_STRING];
	}
	else if (expression->type == NODE_CHAR_LITERAL)
	{
		expression->inferredType = &tc->types->primitiveTypes[TYPE_INT8];
	}
	else if (expression->type == NODE_TRUE)
	{
		expression->inferredType = &tc->types->primitiveTypes[TYPE_BOOL];
	}
	else if (expression->type == NODE_FALSE)
	{
		expression->inferredType = &tc->types->primitiveTypes[TYPE_BOOL];
	}
	else if (expression->type == NODE_NULL_LITERAL)
	{
		expression->inferredType = &tc->types->primitiveTypes[TYPE_VOID];
	}
	else if (expression->type == NODE_IDENTIFIER)
	{
		Identifier* identifier = (Identifier*)expression;
		if (Node* symbol = resolveSymbol(tc, identifier->name))
		{
			identifier->resolvedSymbol = symbol;

			if (symbol->type == NODE_VARIABLE_DECLARATION)
			{
				VariableDeclaration* variableDeclaration = &symbol->variableDeclaration;
				identifier->inferredType = variableDeclaration->variableType->inferredType;
			}
			else if (symbol->type == NODE_FUNCTION)
			{
				Function* function = &symbol->function;
				identifier->inferredType = function->functionType;
			}
			else
			{
				SnekAssert(false);
				identifier->inferredType = &tc->types->errorType;
			}
		}
		else
		{
			error(tc, (Node*)expression, "Undefined variable '%.*s'", identifier->name.length, identifier->name.ptr);
			identifier->inferredType = &tc->types->errorType;
		}
	}
	else if (expression->type == NODE_COMPOUND_EXPRESSION)
	{
		CompoundExpression* compound = (CompoundExpression*)expression;
		resolveExpression(tc, compound->value);
		expression->inferredType = compound->value->inferredType;
	}
	else if (expression->type == NODE_EXPRESSION_LIST)
	{
		ExpressionList* expressionList = (ExpressionList*)expression;

		int mark = tc->scratch.mark();

		for (int i = 0; i < expressionList->numValues; i++)
		{
			if (expressionList->values[i])
			{
				resolveExpression(tc, expressionList->values[i]);
				tc->scratch.add(expressionList->values[i]);
			}
			else
			{
				tc->scratch.add(nullptr);
			}
		}

		Type** valueTypes = tc->scratch.getData<Type*>(mark);

		expression->inferredType = getAnonymousStructType(tc->types, expressionList->numValues, valueTypes);

		tc->scratch.release(mark);
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

		// todo check operator overload

		if (binaryOperator->op == OPERATOR_ADD || binaryOperator->op == OPERATOR_SUBTRACT || binaryOperator->op == OPERATOR_MULTIPLY || binaryOperator->op == OPERATOR_DIVIDE || binaryOperator->op == OPERATOR_MODULO)
			return binaryOperator->inferredType = typeCheckArithmeticOperator(tc, binaryOperator, left, right);
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

		Type* operandType = resolveExpression(tc, unaryOperator->expression);

		if (operandType == &tc->types->errorType)
			return unaryOperator->inferredType = operandType;

		if (unaryOperator->op == OPERATOR_LOGICAL_NOT)
		{
			if (!isTruthyType(operandType))
			{
				error(tc, (Node*)unaryOperator->expression, "Operand of logical not operator must be a scalar");
				return unaryOperator->inferredType = &tc->types->errorType;
			}

			if (operandType->typeKind != TYPE_BOOL)
				insertImplicitCast(tc, &unaryOperator->expression, &tc->types->primitiveTypes[TYPE_BOOL]);

			return unaryOperator->inferredType = &tc->types->primitiveTypes[TYPE_BOOL];
		}
		else if (unaryOperator->op == OPERATOR_BITWISE_NOT)
		{
			if (!isIntegerType(operandType))
			{
				error(tc, (Node*)unaryOperator->expression, "Operand of bitwise not operator must be of integer type");
				return unaryOperator->inferredType = &tc->types->errorType;
			}

			return unaryOperator->inferredType = operandType;
		}
		else if (unaryOperator->op == OPERATOR_MINUS_PREFIX)
		{
			if (!isIntegerType(operandType))
			{
				error(tc, (Node*)unaryOperator->expression, "Operand of negate operator must be of integer type");
				return unaryOperator->inferredType = &tc->types->errorType;
			}

			return unaryOperator->inferredType = operandType;
		}
		else if (unaryOperator->op == OPERATOR_PLUS_PREFIX)
		{
			if (!isIntegerType(operandType))
			{
				error(tc, (Node*)unaryOperator->expression, "Operand of plus operator must be of integer type");
				return unaryOperator->inferredType = &tc->types->errorType;
			}

			return unaryOperator->inferredType = operandType;
		}
		else if (unaryOperator->op == OPERATOR_DEREFERENCE)
		{
			if (operandType->typeKind != TYPE_POINTER)
			{
				error(tc, (Node*)unaryOperator->expression, "Cannot deference non-pointer type");
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
			if (!isLValue(unaryOperator->expression))
			{
				error(tc, (Node*)unaryOperator->expression, "Cannot take address of rvalue expression");
				return unaryOperator->inferredType = &tc->types->errorType;
			}

			return unaryOperator->inferredType = getPointerType(tc->types, operandType);
		}
		else if (unaryOperator->op == OPERATOR_INCREMENT_PREFIX || unaryOperator->op == OPERATOR_DECREMENT_PREFIX
			|| unaryOperator->op == OPERATOR_INCREMENT_POSTFIX || unaryOperator->op == OPERATOR_DECREMENT_POSTFIX)
		{
			if (!isLValue(unaryOperator->expression))
			{
				error(tc, (Node*)unaryOperator->expression, "Cannot increment/decrement rvalue expression");
				return unaryOperator->inferredType = &tc->types->errorType;
			}

			if (!isIntegerType(operandType) && !isFloatingPointType(operandType) && operandType->typeKind != TYPE_POINTER)
			{
				error(tc, (Node*)unaryOperator->expression, "Increment/decrement operator requires scalar or pointer type");
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

		Type* operandType = resolveExpression(tc, functionCall->expression);
		for (int i = 0; i < functionCall->numArgs; i++)
		{
			resolveExpression(tc, functionCall->args[i]);
		}

		if (operandType->typeKind != TYPE_FUNCTION)
		{
			error(tc, (Node*)functionCall->expression, "Operand of function call must be of type function");
			return &tc->types->errorType;
		}

		return expression->inferredType = operandType->function.returnType;
	}
	else if (expression->type == NODE_ARRAY_SUBSCRIPT)
	{
		ArraySubscript* subscript = (ArraySubscript*)expression;

		Type* operandType = resolveExpression(tc, subscript->expression);
		for (int i = 0; i < subscript->numArgs; i++)
		{
			resolveExpression(tc, subscript->args[i]);
		}

		if (operandType->typeKind == TYPE_ARRAY)
		{
			return expression->inferredType = operandType->array.elementType;
		}
		else if (operandType->typeKind == TYPE_POINTER)
		{
			return expression->inferredType = operandType->pointer.elementType;
		}
		else
		{
			error(tc, (Node*)subscript->expression, "Operand of subscript operator must be of array or pointer type");
			return &tc->types->errorType;
		}
	}
	else if (expression->type == NODE_MEMBER_ACCESS)
	{
		MemberAccess* member = (MemberAccess*)expression;

		Type* operandType = resolveExpression(tc, member->expression);

		if (operandType->typeKind == TYPE_STRUCT)
		{
			if (member->index == -1)
				member->index = getFieldIndex(member->name, operandType->struct_.numFields, operandType->struct_.fieldNames);
			if (member->index == -1)
			{
				error(tc, member->name, "Undefined struct field '%.*s'", member->name.length, member->name.ptr);
				return &tc->types->errorType;
			}

			return expression->inferredType = operandType->struct_.fieldTypes[member->index];
		}
		else if (operandType->typeKind == TYPE_UNION)
		{
			if (member->index == -1)
				member->index = getFieldIndex(member->name, operandType->union_.numFields, operandType->union_.fieldNames);
			if (member->index == -1)
			{
				error(tc, member->name, "Undefined struct field '%.*s'", member->name.length, member->name.ptr);
				return &tc->types->errorType;
			}

			return expression->inferredType = operandType->union_.fieldTypes[member->index];
		}
		else if (operandType->typeKind == TYPE_STRING)
		{
			
		}
		else if (operandType->typeKind == TYPE_ARRAY)
		{

		}
		else
		{
			error(tc, (Node*)member->expression, "Operand of member access must be of struct, union, string or array type");
			return &tc->types->errorType;
		}
	}
	else if (expression->type == NODE_TERNARY_CONDITION)
	{
		TernaryCondition* ternary = (TernaryCondition*)expression;
		if (ternary->condition)
			resolveExpression(tc, ternary->condition);
		if (ternary->then)
			resolveExpression(tc, ternary->then);
		if (ternary->else_)
			resolveExpression(tc, ternary->else_);
	}
	else if (expression->type == NODE_CAST)
	{
		Cast* cast = (Cast*)expression;
		if (cast->targetType)
			resolveType(tc, cast->targetType);
		if (cast->expression)
			resolveExpression(tc, cast->expression);
	}
	else if (expression->type == NODE_CAST_IMPLICIT)
	{
		SnekAssert(false);
	}
	else
	{
		SnekAssert(false);
	}
}

static void resolveStatement(TypeChecker* tc, Statement* statement)
{
	if (statement->type == NODE_BLOCK_STATEMENT)
	{
		BlockStatement* block = (BlockStatement*)statement;
		for (int i = 0; i < block->numStatements; i++)
		{
			if (block->statements[i])
				resolveStatement(tc, block->statements[i]);
		}
	}
	else if (statement->type == NODE_IF)
	{
		If* if_ = (If*)statement;
		if (if_->condition)
			resolveExpression(tc, if_->condition);
		if (if_->then)
			resolveStatement(tc, if_->then);
		if (if_->else_)
			resolveStatement(tc, if_->else_);
	}
	else if (statement->type == NODE_WHILE)
	{
		While* while_ = (While*)statement;
		if (while_->condition)
			resolveExpression(tc, while_->condition);
		if (while_->then)
			resolveStatement(tc, while_->then);
	}
	else if (statement->type == NODE_FOR)
	{
		For* for_ = (For*)statement;
		if (for_->startValue)
			resolveExpression(tc, for_->startValue);
		if (for_->compareValue)
			resolveExpression(tc, for_->compareValue);
		if (for_->body)
			resolveStatement(tc, for_->body);
	}
	else if (statement->type == NODE_RETURN)
	{
		Return* return_ = (Return*)statement;
		if (return_->value)
			resolveExpression(tc, return_->value);
	}
	else if (statement->type == NODE_BREAK)
	{
	}
	else if (statement->type == NODE_CONTINUE)
	{
	}
	else if (statement->type == NODE_DEFER)
	{
		Defer* defer = (Defer*)statement;
		if (defer->body)
			resolveStatement(tc, defer->body);
	}
	else if (statement->type == NODE_VARIABLE_DECLARATION)
	{
		VariableDeclaration* variableDeclaration = (VariableDeclaration*)statement;
		if (variableDeclaration->variableType)
			resolveType(tc, variableDeclaration->variableType);
		for (int i = 0; i < variableDeclaration->numDeclarators; i++)
		{
			if (variableDeclaration->declarators[i].value)
				resolveExpression(tc, variableDeclaration->declarators[i].value);
		}
	}
	else if (statement->type == NODE_ASSIGNMENT)
	{
		Assignment* assignment = (Assignment*)statement;
		if (assignment->expression)
			resolveExpression(tc, assignment->expression);
		if (assignment->value)
			resolveExpression(tc, assignment->value);
	}
	else if (statement->type == NODE_EXPRESSION_STATEMENT)
	{
		ExpressionStatement* expression = (ExpressionStatement*)statement;
		if (expression->expression)
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
		return field->inferredType = resolveType(tc, field->variableType);
	}
	else
	{
		return field->inferredType = &tc->types->errorType;
	}
}

static Type* resolveParameter(TypeChecker* tc, Parameter* parameter)
{
	return resolveType(tc, parameter->type);
}

void symbolResolution(TypeChecker* tc, AST* ast)
{
	for (int i = 0; i < ast->numStructs; i++)
	{
		Struct* struct_ = ast->structs[i];
		for (int j = 0; j < struct_->numFields; j++)
		{
			if (struct_->fields[j])
				resolveField(tc, struct_->fields[j]);
		}
	}

	for (int i = 0; i < ast->numEnums; i++)
	{
		Enum* enum_ = ast->enums[i];
		for (int j = 0; j < enum_->numValues; j++)
		{
			if (enum_->values[j].value)
			{
				resolveExpression(tc, enum_->values[j].value);
				if (!isConstant(enum_->values[j].value))
				{
					error(tc, (Node*)enum_->values[j].value, "Enum value must be constant");
				}
			}
		}
	}

	for (int i = 0; i < ast->numUnions; i++)
	{
		Union* union_ = ast->unions[i];
		for (int j = 0; j < union_->numFields; j++)
		{
			if (union_->fields[j])
				resolveField(tc, union_->fields[j]);
		}
	}

	for (int i = 0; i < ast->numTypedefs; i++)
	{
		Typedef* typedef_ = ast->typedefs[i];
		if (typedef_->value)
			resolveType(tc, typedef_->value);
	}

	for (int i = 0; i < ast->numFunctions; i++)
	{
		Function* function = ast->functions[i];

		int mark = tc->scratch.mark();

		for (int j = 0; j < function->numParams; j++)
		{
			if (function->params[j])
			{
				resolveParameter(tc, function->params[j]);
				if (function->params[j]->type)
					tc->scratch.add(function->params[j]->type->inferredType);
				else
					tc->scratch.add(nullptr);
			}
			else
			{
				tc->scratch.add(nullptr);
			}
		}

		Type* returnType = nullptr;
		if (function->value)
		{
			resolveExpression(tc, function->value);
			returnType = function->value->inferredType;
		}
		else if (function->returnType)
		{
			resolveType(tc, function->returnType);
			returnType = function->returnType->inferredType;
		}

		function->functionType = getFunctionType(tc->types, returnType, function->numParams, tc->scratch.getData<Type*>(mark));

		tc->scratch.release(mark);
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
	}

	for (int i = 0; i < ast->numMacros; i++)
	{
		Macro* macro = ast->macros[i];

	}

	for (int i = 0; i < ast->numGlobalVariables; i++)
	{
		GlobalVariable* globalVariable = ast->globalVariables[i];
		if (globalVariable->type)
			resolveType(tc, globalVariable->type);
		for (int j = 0; j < globalVariable->numDeclarators; j++)
		{
			if (globalVariable->declarators[j].value)
				resolveExpression(tc, globalVariable->declarators[j].value);
		}
	}
}

void typeCheckFunction(TypeChecker* tc, Function* function)
{
	if (!function->value)
	{
		for (int j = 0; j < function->numStatements; j++)
		{
			if (function->statements[j])
				resolveStatement(tc, function->statements[j]);
		}
	}
}
