#include "TypeChecker.h"

#include "parser/AST.h"
#include "parser/Diagnostics.h"
#include "parser/Lexer.h"

#include <stdarg.h>


void initTypeChecker(TypeChecker* tc, Arena* arena, Lexer* lexer, Diagnostics* diagnostics)
{
	tc->arena = arena;
	tc->lexer = lexer;
	tc->diagnostics = diagnostics;

	tc->errorType = { .typeKind = TYPE_NULL };
	tc->int8Type = { .typeKind = TYPE_INT8 };
	tc->int16Type = { .typeKind = TYPE_INT16 };
	tc->int32Type = { .typeKind = TYPE_INT32 };
	tc->int64Type = { .typeKind = TYPE_INT64 };
	tc->uint8Type = { .typeKind = TYPE_UINT8 };
	tc->uint16Type = { .typeKind = TYPE_UINT16 };
	tc->uint32Type = { .typeKind = TYPE_UINT32 };
	tc->uint64Type = { .typeKind = TYPE_UINT64 };
	tc->floatType = { .typeKind = TYPE_FLOAT };
	tc->doubleType = { .typeKind = TYPE_DOUBLE };
	tc->boolType = { .typeKind = TYPE_BOOL };
	tc->anyType = { .typeKind = TYPE_ANY };
	tc->stringType = { .typeKind = TYPE_STRING };
}

void destroyTypeChecker(TypeChecker* tc)
{
}

static void getSourceLocation(TypeChecker* tc, Node* node, SourceLocation* start, SourceLocation* end)
{
	*start = getSourceLocation(tc->lexer, node->start);
	*end = getSourceLocation(tc->lexer, node->end);
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

	logMessage(tc->diagnostics, msg, start.line, start.col, end.line, end.col + 1, DIAGNOSTICS_ERROR);
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
			insertSymbol(&tc->currentScope->symbols, declaration->struct_.name, declaration);
			ast->structs[numStructs++] = &declaration->struct_;
		}
		else if (declaration->type == NODE_ENUM)
		{
			insertSymbol(&tc->currentScope->symbols, declaration->enum_.name, declaration);
			ast->enums[numEnums++] = &declaration->enum_;
		}
		else if (declaration->type == NODE_UNION)
		{
			insertSymbol(&tc->currentScope->symbols, declaration->union_.name, declaration);
			ast->unions[numUnions++] = &declaration->union_;
		}
		else if (declaration->type == NODE_TYPEDEF)
		{
			insertSymbol(&tc->currentScope->symbols, declaration->typedef_.name, declaration);
			ast->typedefs[numTypedefs++] = &declaration->typedef_;
		}
		else if (declaration->type == NODE_FUNCTION)
		{
			insertSymbol(&tc->currentScope->symbols, declaration->function.name, declaration);
			ast->functions[numFunctions++] = &declaration->function;
		}
		else if (declaration->type == NODE_GLOBAL_VARIABLE)
		{
			for (int i = 0; i < declaration->globalVariable.numDeclarators; i++)
			{
				insertSymbol(&tc->currentScope->symbols, declaration->globalVariable.declarators[i].name, declaration);
			}
			ast->globalVariables[numGlobalVariables++] = &declaration->globalVariable;
		}
		else if (declaration->type == NODE_MACRO)
		{
			insertSymbol(&tc->currentScope->symbols, declaration->macro.name, declaration);
			ast->macros[numMacros++] = &declaration->macro;
		}
	}
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
	else if (expression->type == NODE_PREFIX_OPERATOR)
	{
		return false;
	}
	else if (expression->type == NODE_POSTFIX_OPERATOR)
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

static void resolveType(TypeChecker* tc, TypeNode* type)
{
	if (type->type == TYPE_VOID)
	{
		
	}
	else if (type->type == TYPE_INT8)
	{
		type->inferredType = &tc->int8Type;
	}
	else if (type->type == TYPE_INT16)
	{
		type->inferredType = &tc->int16Type;
	}
	else if (type->type == TYPE_INT32)
	{
		type->inferredType = &tc->int32Type;
	}
	else if (type->type == TYPE_INT64)
	{
		type->inferredType = &tc->int64Type;
	}
	else if (type->type == TYPE_UINT8)
	{
		type->inferredType = &tc->uint8Type;
	}
	else if (type->type == TYPE_UINT16)
	{
		type->inferredType = &tc->uint16Type;
	}
	else if (type->type == TYPE_UINT32)
	{
		type->inferredType = &tc->uint32Type;
	}
	else if (type->type == TYPE_UINT64)
	{
		type->inferredType = &tc->uint64Type;
	}
	else if (type->type == TYPE_FLOAT)
	{
		type->inferredType = &tc->floatType;
	}
	else if (type->type == TYPE_DOUBLE)
	{
		type->inferredType = &tc->doubleType;
	}
	else if (type->type == TYPE_BOOL)
	{
		type->inferredType = &tc->boolType;
	}
	else if (type->type == TYPE_ANY)
	{
		type->inferredType = &tc->anyType;
	}
	else if (type->type == TYPE_STRING)
	{
		type->inferredType = &tc->stringType;
	}
	else if (type->type == TYPE_STRUCT)
	{
		
	}
	else if (type->type == TYPE_UNION)
	{
		
	}
	else if (type->type == TYPE_POINTER)
	{
		
	}
	else if (type->type == TYPE_OPTIONAL)
	{
		
	}
	else if (type->type == TYPE_FUNCTION)
	{
		
	}
	else if (type->type == TYPE_TUPLE)
	{
		
	}
	else if (type->type == TYPE_ARRAY)
	{
		
	}
}

static void resolveExpression(TypeChecker* tc, Expression* expression)
{

}

static void resolveStatement(TypeChecker* tc, Statement* statement)
{

}

static void resolveField(TypeChecker* tc, Field* field)
{
	if (field->type)
		resolveType(tc, field->type);
}

static void resolveParameter(TypeChecker* tc, Parameter* parameter)
{
	if (parameter->type)
		resolveType(tc, parameter->type);
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
		for (int j = 0; j < function->numParams; j++)
		{
			if (function->params[j])
				resolveParameter(tc, function->params[j]);
		}
		if (function->returnType)
			resolveType(tc, function->returnType);
		if (function->value)
			resolveExpression(tc, function->value);
		else
		{
			for (int j = 0; j < function->numStatements; j++)
			{
				if (function->statements[j])
					resolveStatement(tc, function->statements[j]);
			}
		}
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

}
