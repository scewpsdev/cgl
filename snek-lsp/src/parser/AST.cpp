#include "AST.h"

#include "utils/Hash.h"
#include "utils/Log.h"

#include <stdio.h>


FileHandle getFileHandle(const char* localPath)
{
	return (FileHandle)hashPath(localPath);
}

void getLocalPathFromModuleName(char* path, StringView* parts, int numParts)
{
	path[0] = 0;
	for (int i = 0; i < numParts; i++)
	{
		strncat(path, parts[i].ptr, parts[i].length);
		if (i < numParts - 1)
			strcat(path, "/");
	}
	strcat(path, ".src");
}

void initAST(AST* ast, const char* localPath)
{
	*ast = {};

	ast->fileHandle = getFileHandle(localPath);
}

void destroyAST(AST* ast)
{
}

void initNode(Node* node, NodeType type, int start)
{
	node->type = type;
	node->start = start;
}

void initType(TypeNode* type, NodeType nodeType, TypeKind typeKind, int start)
{
	initNode((Node*)type, nodeType, start);
	type->typeKind = typeKind;
}

void initSymbolTable(SymbolTable* symbols, int capacity, Arena* arena)
{
	// todo check if power of 2

	symbols->slots = arena->alloc<SymbolEntry>(capacity);
	symbols->capacity = capacity;
	symbols->count = 0;

	symbols->arena = arena;
}

static void growSymbolTable(SymbolTable* symbols)
{
	int newCapacity = symbols->capacity * 2;

	SymbolEntry* newSlots = symbols->arena->alloc<SymbolEntry>(newCapacity);

	uint32_t mask = newCapacity - 1;

	for (int i = 0; i < symbols->capacity; i++)
	{
		SymbolEntry* slot = &symbols->slots[i];

		if (slot->key)
		{
			uint32_t index = slot->key & mask;
			SymbolEntry* newSlot = &newSlots[index];

			while (true)
			{
				if (!newSlot->key)
				{
					*newSlot = *slot;
					break;
				}

				index = (index + 1) & mask;
				newSlot = &newSlots[index];
			}
		}
	}

	symbols->slots = newSlots;
	symbols->capacity = newCapacity;
}

bool insertSymbol(SymbolTable* symbols, StringView identifier, SymbolType type, Node* declaration, FileHandle file)
{
	if (symbols->count * 4 >= symbols->capacity * 3)
	{
		growSymbolTable(symbols);
	}

	uint32_t mask = symbols->capacity - 1;
	uint32_t h = hash(identifier);
	uint32_t index = h & mask;

	for (int i = 0; i < symbols->capacity; i++)
	{
		SymbolEntry* slot = &symbols->slots[index];

		if (!slot->key)
		{
			slot->key = h;
			slot->type = type;
			slot->file = file;

			if (type == SYMBOL_FUNCTION_SET)
			{
				slot->functionSet.overloads = symbols->arena->alloc<FunctionOverload>(slot->functionSet.capacity = 8);
				slot->functionSet.count = 0;
				slot->functionSet.overloads[slot->functionSet.count++] = {
					.declaration = declaration,
				};
			}
			else
			{
				slot->declaration = declaration;
			}

			symbols->count++;
			return true;
		}

		if (slot->key == h)
		{
			if (slot->type == SYMBOL_FUNCTION_SET)
			{
				if (slot->type != type)
					return false;

				if (slot->functionSet.count == slot->functionSet.capacity)
				{
					FunctionOverload* newOverloads = symbols->arena->alloc<FunctionOverload>(slot->functionSet.capacity *= 2);
					memcpy(newOverloads, slot->functionSet.overloads, slot->functionSet.count * sizeof(FunctionOverload));
					slot->functionSet.overloads = newOverloads;
				}

				slot->functionSet.overloads[slot->functionSet.count++] = {
					.declaration = declaration,
				};
			}
			else
			{
				return false;
			}
		}

		index = (index + 1) & mask;
	}

	SnekAssert(false);
	return false;
}

SymbolEntry* lookupSymbol(SymbolTable* symbols, StringView identifier)
{
	if (!symbols->count) return nullptr;

	uint32_t mask = symbols->capacity - 1;
	uint32_t h = hash(identifier);
	uint32_t index = h & mask;

	for (int i = 0; i < symbols->capacity; i++)
	{
		SymbolEntry* slot = &symbols->slots[index];

		if (!slot->key)
			return nullptr;

		if (slot->key == h)
			return slot;

		index = (index + 1) & mask;
	}

	return nullptr;
}

SymbolEntry* lookupSymbol(SymbolTable* symbols, uint32_t h)
{
	if (!symbols->count) return nullptr;

	uint32_t mask = symbols->capacity - 1;
	uint32_t index = h & mask;

	for (int i = 0; i < symbols->capacity; i++)
	{
		SymbolEntry* slot = &symbols->slots[index];

		if (!slot->key)
			return nullptr;

		if (slot->key == h)
			return slot;

		index = (index + 1) & mask;
	}

	return nullptr;
}

void initScope(Scope* scope, Scope* parent, bool isGlobal, Arena* arena)
{
	scope->parent = parent;

	int symbolCapacity = isGlobal ? 1024 : 16;
	initSymbolTable(&scope->symbols, symbolCapacity, arena);
}

static void traverseField(Field* field, Scope* scope, ASTVisitor_t visitor, void* userPtr);
static void traverseParameter(Parameter* parameter, Scope* scope, ASTVisitor_t visitor, void* userPtr);

static void traverseType(TypeNode* type, Scope* scope, ASTVisitor_t visitor, void* userPtr)
{
	visitor((Node*)type, scope, userPtr);

	if (type->type == NODE_STRUCT_TYPE)
	{
		StructType* structType = (StructType*)type;
		for (int i = 0; i < structType->numFields; i++)
		{
			if (structType->fields[i])
				traverseField(structType->fields[i], scope, visitor, userPtr);
		}
	}
	else if (type->type == NODE_UNION_TYPE)
	{
		UnionType* unionType = (UnionType*)type;
		for (int i = 0; i < unionType->numFields; i++)
		{
			if (unionType->fields[i])
				traverseField(unionType->fields[i], scope, visitor, userPtr);
		}
	}
	else if (type->type == NODE_POINTER_TYPE)
	{
		PointerType* pointerType = (PointerType*)type;
		if (pointerType->elementType)
			traverseType(pointerType->elementType, scope, visitor, userPtr);
	}
	else if (type->type == NODE_OPTIONAL_TYPE)
	{
		OptionalType* optionalType = (OptionalType*)type;
		if (optionalType->elementType)
			traverseType(optionalType->elementType, scope, visitor, userPtr);
	}
	else if (type->type == NODE_FUNCTION_TYPE)
	{
		FunctionType* functionType = (FunctionType*)type;
		for (int i = 0; i < functionType->numParams; i++)
		{
			if (functionType->params[i])
				traverseParameter(functionType->params[i], scope, visitor, userPtr);
		}
		if (functionType->returnType)
		{
			traverseType(functionType->returnType, scope, visitor, userPtr);
		}
	}
	else if (type->type == NODE_TUPLE_TYPE)
	{
		TupleType* tupleType = (TupleType*)type;
		for (int i = 0; i < tupleType->numElementTypes; i++)
		{
			if (tupleType->elementTypes[i])
				traverseType(tupleType->elementTypes[i], scope, visitor, userPtr);
		}
	}
	else if (type->type == NODE_ARRAY_TYPE)
	{
		ArrayType* arrayType = (ArrayType*)type;
		if (arrayType->elementType)
			traverseType(arrayType->elementType, scope, visitor, userPtr);
	}
}

static void traverseExpression(Expression* expression, Scope* scope, ASTVisitor_t visitor, void* userPtr)
{
	visitor((Node*)expression, scope, userPtr);

	if (expression->type == NODE_COMPOUND_EXPRESSION)
	{
		CompoundExpression* compound = (CompoundExpression*)expression;
		if (compound->value)
			traverseExpression(compound->value, scope, visitor, userPtr);
	}
	else if (expression->type == NODE_EXPRESSION_LIST)
	{
		ExpressionList* expressionList = (ExpressionList*)expression;
		for (int i = 0; i < expressionList->numValues; i++)
		{
			if (expressionList->values[i])
				traverseExpression(expressionList->values[i], scope, visitor, userPtr);
		}
	}
	else if (expression->type == NODE_BINARY_OPERATOR)
	{
		BinaryOperator* binaryOperator = (BinaryOperator*)expression;
		if (binaryOperator->left)
			traverseExpression(binaryOperator->left, scope, visitor, userPtr);
		if (binaryOperator->right)
			traverseExpression(binaryOperator->right, scope, visitor, userPtr);
	}
	else if (expression->type == NODE_CAST)
	{
		Cast* cast = (Cast*)expression;
		if (cast->expression)
			traverseExpression(cast->expression, scope, visitor, userPtr);
		if (cast->targetType)
			traverseType(cast->targetType, scope, visitor, userPtr);
	}
	else if (expression->type == NODE_UNARY_OPERATOR)
	{
		UnaryOperator* unaryOperator = (UnaryOperator*)expression;
		if (unaryOperator->expression)
			traverseExpression(unaryOperator->expression, scope, visitor, userPtr);
	}
	else if (expression->type == NODE_FUNCTION_CALL)
	{
		FunctionCall* functionCall = (FunctionCall*)expression;
		if (functionCall->expression)
			traverseExpression(functionCall->expression, scope, visitor, userPtr);
		for (int i = 0; i < functionCall->numArgs; i++)
		{
			if (functionCall->args[i])
				traverseExpression(functionCall->args[i], scope, visitor, userPtr);
		}
	}
	else if (expression->type == NODE_ARRAY_SUBSCRIPT)
	{
		ArraySubscript* arraySubscript = (ArraySubscript*)expression;
		if (arraySubscript->expression)
			traverseExpression(arraySubscript->expression, scope, visitor, userPtr);
		for (int i = 0; i < arraySubscript->numArgs; i++)
		{
			if (arraySubscript->args[i])
				traverseExpression(arraySubscript->args[i], scope, visitor, userPtr);
		}
	}
	else if (expression->type == NODE_MEMBER_ACCESS)
	{
		MemberAccess* memberAccess = (MemberAccess*)expression;
		if (memberAccess->expression)
			traverseExpression(memberAccess->expression, scope, visitor, userPtr);
	}
	else if (expression->type == NODE_TERNARY_CONDITION)
	{
		TernaryCondition* ternaryCondition = (TernaryCondition*)expression;
		if (ternaryCondition->condition)
			traverseExpression(ternaryCondition->condition, scope, visitor, userPtr);
		if (ternaryCondition->then)
			traverseExpression(ternaryCondition->then, scope, visitor, userPtr);
		if (ternaryCondition->else_)
			traverseExpression(ternaryCondition->else_, scope, visitor, userPtr);
	}
}

static void traverseStatement(Statement* statement, Scope* scope, ASTVisitor_t visitor, void* userPtr)
{
	if (statement->type == NODE_BLOCK_STATEMENT)
	{
		BlockStatement* block = (BlockStatement*)statement;
		for (int i = 0; i < block->numStatements; i++)
		{
			if (block->statements[i])
				traverseStatement(block->statements[i], block->scope, visitor, userPtr);
		}
	}
	else if (statement->type == NODE_IF)
	{
		If* if_ = (If*)statement;
		if (if_->condition)
			traverseExpression(if_->condition, scope, visitor, userPtr);
		if (if_->then)
			traverseStatement(if_->then, scope, visitor, userPtr);
		if (if_->else_)
			traverseStatement(if_->else_, scope, visitor, userPtr);
	}
	else if (statement->type == NODE_WHILE)
	{
		While* while_ = (While*)statement;
		if (while_->condition)
			traverseExpression(while_->condition, scope, visitor, userPtr);
		if (while_->then)
			traverseStatement(while_->then, scope, visitor, userPtr);
	}
	else if (statement->type == NODE_FOR)
	{
		For* for_ = (For*)statement;
		if (for_->startValue)
			traverseExpression(for_->startValue, for_->scope, visitor, userPtr);
		if (for_->compareValue)
			traverseExpression(for_->compareValue, for_->scope, visitor, userPtr);
		if (for_->body)
			traverseStatement(for_->body, for_->scope, visitor, userPtr);
	}
	else if (statement->type == NODE_RETURN)
	{
		Return* return_ = (Return*)statement;
		if (return_->value)
			traverseExpression(return_->value, scope, visitor, userPtr);
	}
	else if (statement->type == NODE_DEFER)
	{
		Defer* defer = (Defer*)statement;
		if (defer->body)
			traverseStatement(defer->body, scope, visitor, userPtr);
	}
	else if (statement->type == NODE_VARIABLE_DECLARATION)
	{
		VariableDeclaration* variableDeclaration = (VariableDeclaration*)statement;
		if (variableDeclaration->type)
			traverseType(variableDeclaration->variableType, scope, visitor, userPtr);
		for (int i = 0; i < variableDeclaration->numDeclarators; i++)
		{
			if (variableDeclaration->declarators[i].value)
				traverseExpression(variableDeclaration->declarators[i].value, scope, visitor, userPtr);
		}
	}
	else if (statement->type == NODE_ASSIGNMENT)
	{
		Assignment* assignment = (Assignment*)statement;
		if (assignment->expression)
			traverseExpression(assignment->expression, scope, visitor, userPtr);
		if (assignment->value)
			traverseExpression(assignment->value, scope, visitor, userPtr);
	}
	else if (statement->type == NODE_EXPRESSION_STATEMENT)
	{
		ExpressionStatement* expression = (ExpressionStatement*)statement;
		if (expression->expression)
			traverseExpression(expression->expression, scope, visitor, userPtr);
	}
}

static void traverseField(Field* field, Scope* scope, ASTVisitor_t visitor, void* userPtr)
{
	visitor((Node*)field, scope, userPtr);

	traverseType(field->variableType, scope, visitor, userPtr);
	for (int i = 0; i < field->numDeclarators; i++)
	{
		if (field->declarators[i].value)
			traverseExpression(field->declarators[i].value, scope, visitor, userPtr);
	}
}

static void traverseParameter(Parameter* parameter, Scope* scope, ASTVisitor_t visitor, void* userPtr)
{
	visitor((Node*)parameter, scope, userPtr);

	traverseType(parameter->paramType, scope, visitor, userPtr);
}

static void traverseEnumValue(EnumValue* enumValue, Scope* scope, ASTVisitor_t visitor, void* userPtr)
{
	visitor((Node*)enumValue, scope, userPtr);

	if (enumValue->value)
		traverseExpression(enumValue->value, scope, visitor, userPtr);
}

static void traverseDeclaration(Node* declaration, Scope* scope, ASTVisitor_t visitor, void* userPtr)
{
	visitor(declaration, scope, userPtr);

	if (declaration->type == NODE_STRUCT)
	{
		Struct* struct_ = (Struct*)declaration;
		for (int i = 0; i < struct_->numFields; i++)
		{
			traverseField(struct_->fields[i], scope, visitor, userPtr);
		}
	}
	else if (declaration->type == NODE_UNION)
	{
		Union* union_ = (Union*)declaration;
		for (int i = 0; i < union_->numFields; i++)
		{
			traverseField(union_->fields[i], scope, visitor, userPtr);
		}
	}
	else if (declaration->type == NODE_ENUM)
	{
		Enum* enum_ = (Enum*)declaration;
		for (int i = 0; i < enum_->numValues; i++)
		{
			traverseEnumValue(enum_->values[i], scope, visitor, userPtr);
		}
	}
	else if (declaration->type == NODE_TYPEDEF)
	{
		Typedef* typedef_ = (Typedef*)declaration;
		if (typedef_->value)
			traverseType(typedef_->value, scope, visitor, userPtr);
	}
	else if (declaration->type == NODE_FUNCTION)
	{
		Function* function = (Function*)declaration;

		if (function->returnType)
		{
			traverseType(function->returnType, scope, visitor, userPtr);
		}

		for (int i = 0; i < function->numParams; i++)
		{
			if (function->params[i])
				traverseParameter(function->params[i], function->scope, visitor, userPtr);
		}
		if (function->value)
		{
			traverseExpression(function->value, function->scope, visitor, userPtr);
		}
		else
		{
			for (int i = 0; i < function->numStatements; i++)
			{
				if (function->statements[i])
					traverseStatement(function->statements[i], function->scope, visitor, userPtr);
			}
		}
	}
	else if (declaration->type == NODE_GLOBAL_VARIABLE)
	{
		GlobalVariable* globalVariable = &declaration->globalVariable;
		traverseType(globalVariable->variableType, scope, visitor, userPtr);
		for (int i = 0; i < globalVariable->numDeclarators; i++)
		{
			if (globalVariable->declarators[i].value)
				traverseExpression(globalVariable->declarators[i].value, scope, visitor, userPtr);
		}
	}
	else if (declaration->type == NODE_MACRO)
	{
	}
	else if (declaration->type == NODE_IMPORT)
	{
	}
}

void traverseAST(AST* ast, ASTVisitor_t visitor, void* userPtr)
{
	for (int i = 0; i < ast->numDeclarations; i++)
	{
		traverseDeclaration(ast->declarations[i], ast->globalScope, visitor, userPtr);
	}
}
