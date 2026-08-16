#include "AST.h"

#include "File.h"
#include "typechecker/TypeSystem.h"

#include "utils/Hash.h"
#include "utils/Log.h"

#include <stdio.h>


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

	symbols->slots = arena->alloc<Symbol>(capacity);
	symbols->capacity = capacity;
	symbols->count = 0;

	symbols->arena = arena;
}

static void growSymbolTable(SymbolTable* symbols)
{
	int newCapacity = symbols->capacity * 2;

	Symbol* newSlots = symbols->arena->alloc<Symbol>(newCapacity);

	uint32_t mask = newCapacity - 1;

	for (int i = 0; i < symbols->capacity; i++)
	{
		Symbol* slot = &symbols->slots[i];

		if (slot->key)
		{
			uint32_t index = slot->key & mask;
			Symbol* newSlot = &newSlots[index];

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
		Symbol* slot = &symbols->slots[index];

		if (!slot->key)
		{
			*slot = {};
			slot->key = h;
			slot->name = identifier;
			slot->type = type;
			slot->file = file;

			if (type == SYMBOL_FUNCTION_SET)
			{
				SnekAssert(declaration->type == NODE_FUNCTION);

				slot->functionSet.overloads = symbols->arena->alloc<FunctionOverload>(slot->functionSet.capacity = 8);
				slot->functionSet.count = 0;
				slot->functionSet.overloads[slot->functionSet.count++] = {
					.declaration = &declaration->function,
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

				SnekAssert(declaration->type == NODE_FUNCTION);

				if (slot->functionSet.count == slot->functionSet.capacity)
				{
					FunctionOverload* newOverloads = symbols->arena->alloc<FunctionOverload>(slot->functionSet.capacity *= 2);
					memcpy(newOverloads, slot->functionSet.overloads, slot->functionSet.count * sizeof(FunctionOverload));
					slot->functionSet.overloads = newOverloads;
				}

				slot->functionSet.overloads[slot->functionSet.count++] = {
					.declaration = &declaration->function,
				};

				return true;
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

Symbol* lookupSymbol(SymbolTable* symbols, StringView identifier)
{
	if (!symbols->count) return nullptr;

	uint32_t mask = symbols->capacity - 1;
	uint32_t h = hash(identifier);
	uint32_t index = h & mask;

	for (int i = 0; i < symbols->capacity; i++)
	{
		Symbol* slot = &symbols->slots[index];

		if (!slot->key)
			return nullptr;

		if (slot->key == h)
			return slot;

		index = (index + 1) & mask;
	}

	return nullptr;
}

Symbol* lookupSymbol(SymbolTable* symbols, uint32_t h)
{
	if (!symbols->count) return nullptr;

	uint32_t mask = symbols->capacity - 1;
	uint32_t index = h & mask;

	for (int i = 0; i < symbols->capacity; i++)
	{
		Symbol* slot = &symbols->slots[index];

		if (!slot->key)
			return nullptr;

		if (slot->key == h)
			return slot;

		index = (index + 1) & mask;
	}

	return nullptr;
}

Symbol* getIdentifierSymbol(Identifier* identifier)
{
	if (identifier->resolvedSymbol)
		return identifier->resolvedSymbol;
	if (File* file = getFileFromHandle(identifier->resolvedSymbolHandle.file))
	{
		return lookupSymbol(&file->ast.globalScope->symbols, identifier->resolvedSymbolHandle.symbol);
	}
	return nullptr;
}

Symbol* getMemberAccessSymbol(MemberAccess* member)
{
	if (member->resolvedSymbol)
		return member->resolvedSymbol;
	if (File* file = getFileFromHandle(member->resolvedSymbolHandle.file))
	{
		return lookupSymbol(&file->ast.globalScope->symbols, member->resolvedSymbolHandle.symbol);
	}
	return nullptr;
}

int getFieldIndex(StringView name, int numFields, StringView* fieldNames)
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

int getEnumValue(StringView name, int numValues, EnumValue** values)
{
	for (int i = 0; i < numValues; i++)
	{
		if (compareString(name, values[i]->name))
			return i;
	}
	return -1;
}

VariableDeclarator* getDeclarator(VariableDeclaration* variable, StringView name)
{
	for (int i = 0; i < variable->numDeclarators; i++)
	{
		if (compareString(variable->declarators[i].name, name))
			return &variable->declarators[i];
	}
	return nullptr;
}

VariableDeclarator* getDeclarator(GlobalVariable* variable, StringView name)
{
	for (int i = 0; i < variable->numDeclarators; i++)
	{
		if (compareString(variable->declarators[i].name, name))
			return &variable->declarators[i];
	}
	return nullptr;
}

bool isConstant(Expression* expression)
{
	Node* node = (Node*)expression;

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
		Identifier* identifier = &node->identifier;
		if (Symbol* symbol = getIdentifierSymbol(identifier))
		{
			if (symbol->type == SYMBOL_VARIABLE)
			{
				Node* node = symbol->declaration;
				if (node->type == NODE_VARIABLE_DECLARATION)
				{
					VariableDeclaration* variableDeclaration = &node->variableDeclaration;
					return variableDeclaration->storage & STORAGE_CONSTANT;
				}
				else if (node->type == NODE_GLOBAL_VARIABLE)
				{
					GlobalVariable* globalVariable = &node->globalVariable;
					return globalVariable->storage & STORAGE_CONSTANT;
				}
			}
		}
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
		UnaryOperator* unaryOperator = (UnaryOperator*)expression;
		return isConstant(unaryOperator->operand);
	}
	else if (expression->type == NODE_MEMBER_ACCESS)
	{
		MemberAccess* member = &node->memberAccess;
		Type* operandType = member->operand->inferredType;

		if (member->operand->type == NODE_STRING_LITERAL)
		{
			StringLiteral* string = (StringLiteral*)member->operand;
			if (compareString(member->name, "length"))
			{
				return true;
			}
		}
		else if (member->operand->type == NODE_IDENTIFIER)
		{
			if (operandType->typeKind == TYPE_TYPE)
			{
				Identifier* typeName = (Identifier*)member->operand;
				if (Symbol* symbol = getIdentifierSymbol(typeName))
				{
					if (symbol->declaration->type == NODE_ENUM)
					{
						return true;
					}
				}
			}
		}
	}
	else if (expression->type == NODE_TERNARY_CONDITION)
	{
		TernaryCondition* ternary = (TernaryCondition*)expression;
		return isConstant(ternary->condition) && isConstant(ternary->then) && isConstant(ternary->else_);
	}

	return false;
}

bool isLValue(Expression* expression)
{
	if (expression->type == NODE_IDENTIFIER)
	{
		Identifier* identifier = (Identifier*)expression;
		if (Symbol* symbol = getIdentifierSymbol(identifier))
		{
			if (symbol->type == SYMBOL_VARIABLE)
			{
				Node* declaration = symbol->declaration;
				if (declaration->type == NODE_VARIABLE_DECLARATION)
				{
					VariableDeclaration* variable = &declaration->variableDeclaration;
					bool isConstant = variable->storage & STORAGE_CONSTANT;
					return !isConstant;
				}
				else if (declaration->type == NODE_PARAMETER)
				{
					return true;
				}
				else if (declaration->type == NODE_GLOBAL_VARIABLE)
				{
					GlobalVariable* globalVariable = &declaration->globalVariable;
					bool isConstant = globalVariable->storage & STORAGE_CONSTANT;
					return !isConstant;
				}
				else if (declaration->type == NODE_FOR)
				{
					return true;
				}
				else
				{
					SnekAssert(false);
				}
			}
		}
		return false;
	}
	else if (expression->type == NODE_MEMBER_ACCESS)
	{
		MemberAccess* member = (MemberAccess*)expression;

		Type* operandType = member->operand->inferredType;
		return operandType->typeKind == TYPE_STRUCT || operandType->typeKind == TYPE_UNION;
	}
	else if (expression->type == NODE_ARRAY_SUBSCRIPT)
	{
		return true;
	}
	else if (expression->type == NODE_CAST)
	{
		Cast* cast = (Cast*)expression;
		if (cast->targetType)
		{
			switch (cast->targetType->inferredType->typeKind)
			{
			case TYPE_ANY:
			case TYPE_STRING:
			case TYPE_STRUCT:
			case TYPE_UNION:
			case TYPE_ENUM:
			case TYPE_ALIAS:
			case TYPE_POINTER:
			case TYPE_OPTIONAL:
			case TYPE_FUNCTION:
			case TYPE_ARRAY:
				return true;
			default:
				return false;
			}
		}
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

bool constantFold(Expression* expression, int64_t* value)
{
	Node* node = (Node*)expression;
	if (expression->type == NODE_INT_LITERAL)
	{
		*value = (int64_t)node->intLiteral.intValue;
		return true;
	}
	else if (expression->type == NODE_IDENTIFIER)
	{
		Identifier* identifier = &node->identifier;
		if (Symbol* symbol = getIdentifierSymbol(identifier))
		{
			if (symbol->type == SYMBOL_VARIABLE)
			{
				Node* node = symbol->declaration;
				if (node->type == NODE_VARIABLE_DECLARATION)
				{
					VariableDeclaration* variableDeclaration = &node->variableDeclaration;
					if (variableDeclaration->storage & STORAGE_CONSTANT)
					{
						for (int i = 0; i < variableDeclaration->numDeclarators; i++)
						{
							if (compareString(variableDeclaration->declarators[i].name, identifier->name))
							{
								if (variableDeclaration->declarators[i].value)
								{
									SnekAssert(isIntegerType(variableDeclaration->declarators[i].value->inferredType));
									return constantFold(variableDeclaration->declarators[i].value, value);
								}
								break;
							}
						}
					}
				}
				else if (node->type == NODE_GLOBAL_VARIABLE)
				{
					GlobalVariable* globalVariable = &node->globalVariable;
					if (globalVariable->storage & STORAGE_CONSTANT)
					{
						for (int i = 0; i < globalVariable->numDeclarators; i++)
						{
							if (compareString(globalVariable->declarators[i].name, identifier->name))
							{
								if (globalVariable->declarators[i].value)
								{
									SnekAssert(isIntegerType(globalVariable->declarators[i].value->inferredType));
									return constantFold(globalVariable->declarators[i].value, value);
								}
							}
						}
					}
				}
			}
		}
	}
	else if (expression->type == NODE_COMPOUND_EXPRESSION)
	{
		CompoundExpression* compound = &node->compoundExpression;
		SnekAssert(isIntegerType(compound->value->inferredType));
		return constantFold(compound->value, value);
	}
	else if (expression->type == NODE_BINARY_OPERATOR)
	{
		BinaryOperator* binaryOperator = &node->binaryOperator;

		int64_t left, right;
		if (constantFold(binaryOperator->left, &left) && constantFold(binaryOperator->right, &right))
		{
			if (binaryOperator->op == OPERATOR_MULTIPLY)
			{
				*value = left * right;
				return true;
			}
			else if (binaryOperator->op == OPERATOR_DIVIDE)
			{
				*value = left / right;
				return true;
			}
			else if (binaryOperator->op == OPERATOR_MODULO)
			{
				*value = left % right;
				return true;
			}
			else if (binaryOperator->op == OPERATOR_ADD)
			{
				*value = left + right;
				return true;
			}
			else if (binaryOperator->op == OPERATOR_SUBTRACT)
			{
				*value = left - right;
				return true;
			}
			else if (binaryOperator->op == OPERATOR_BITSHIFT_LEFT)
			{
				*value = left << right;
				return true;
			}
			else if (binaryOperator->op == OPERATOR_BITSHIFT_RIGHT)
			{
				*value = left >> right;
				return true;
			}
			else if (binaryOperator->op == OPERATOR_LESS)
			{
				*value = left < right;
				return true;
			}
			else if (binaryOperator->op == OPERATOR_LESS_EQUALS)
			{
				*value = left <= right;
				return true;
			}
			else if (binaryOperator->op == OPERATOR_GREATER)
			{
				*value = left > right;
				return true;
			}
			else if (binaryOperator->op == OPERATOR_GREATER_EQUALS)
			{
				*value = left >= right;
				return true;
			}
			else if (binaryOperator->op == OPERATOR_EQUALS)
			{
				*value = left == right;
				return true;
			}
			else if (binaryOperator->op == OPERATOR_NOT_EQUALS)
			{
				*value = left != right;
				return true;
			}
			else if (binaryOperator->op == OPERATOR_BITWISE_AND)
			{
				*value = left & right;
				return true;
			}
			else if (binaryOperator->op == OPERATOR_BITWISE_XOR)
			{
				*value = left ^ right;
				return true;
			}
			else if (binaryOperator->op == OPERATOR_BITWISE_OR)
			{
				*value = left | right;
				return true;
			}
			else if (binaryOperator->op == OPERATOR_LOGICAL_AND)
			{
				*value = left && right;
				return true;
			}
			else if (binaryOperator->op == OPERATOR_LOGICAL_OR)
			{
				*value = left || right;
				return true;
			}
		}
	}
	else if (expression->type == NODE_UNARY_OPERATOR)
	{
		UnaryOperator* unaryOperator = &node->unaryOperator;

		int64_t operandValue;
		if (constantFold(unaryOperator->operand, &operandValue))
		{
			if (unaryOperator->op == OPERATOR_INCREMENT_PREFIX)
			{
				*value = operandValue + 1;
				return true;
			}
			else if (unaryOperator->op == OPERATOR_DECREMENT_PREFIX)
			{
				*value = operandValue - 1;
				return true;
			}
			else if (unaryOperator->op == OPERATOR_PLUS_PREFIX)
			{
				*value = operandValue;
				return true;
			}
			else if (unaryOperator->op == OPERATOR_MINUS_PREFIX)
			{
				*value = -operandValue;
				return true;
			}
			else if (unaryOperator->op == OPERATOR_LOGICAL_NOT)
			{
				*value = !operandValue;
				return true;
			}
			else if (unaryOperator->op == OPERATOR_BITWISE_NOT)
			{
				*value = ~operandValue;
				return true;
			}
		}
	}
	else if (expression->type == NODE_MEMBER_ACCESS)
	{
		MemberAccess* member = &node->memberAccess;
		Type* operandType = member->operand->inferredType;

		if (member->operand->type == NODE_STRING_LITERAL)
		{
			StringLiteral* string = (StringLiteral*)member->operand;
			if (compareString(member->name, "length"))
			{
				*value = string->value.length;
				return true;
			}
		}
		else if (member->operand->type == NODE_IDENTIFIER)
		{
			if (operandType->typeKind == TYPE_TYPE)
			{
				Identifier* typeName = (Identifier*)member->operand;
				if (Symbol* symbol = getIdentifierSymbol(typeName))
				{
					if (symbol->declaration->type == NODE_ENUM)
					{
						Enum* enum_ = &symbol->declaration->enum_;
						int index = getEnumValue(member->name, enum_->numValues, enum_->values);
						if (index != -1)
						{
							EnumValue* enumValue = enum_->values[index];
							if (enumValue->value)
							{
								return constantFold(enumValue->value, value);
							}
							else
							{
								for (int i = index - 1; i >= 0; i--)
								{
									EnumValue* previousEnumValue = enum_->values[i];
									if (previousEnumValue->value)
									{
										int64_t previousValue = constantFold(previousEnumValue->value, value);
										return previousValue + (index - i);
									}
									else if (i == 0)
									{
										return index;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	else if (expression->type == NODE_TERNARY_CONDITION)
	{
		TernaryCondition* ternary = &node->ternaryCondition;

		int64_t condition, then, else_;
		if (constantFold(ternary->condition, &condition) && constantFold(ternary->then, &then) && constantFold(ternary->else_, &else_))
		{
			*value = condition ? then : else_;
			return true;
		}
	}

	return false;
}

static void resetField(Field* field);
static void resetParameter(Parameter* parameter);

static void resetType(TypeNode* type)
{
	type->inferredType = nullptr;

	if (type->type == NODE_STRUCT_TYPE)
	{
		StructType* structType = (StructType*)type;
		for (int i = 0; i < structType->numFields; i++)
		{
			if (structType->fields[i])
				resetField(structType->fields[i]);
		}
	}
	else if (type->type == NODE_UNION_TYPE)
	{
		UnionType* unionType = (UnionType*)type;
		for (int i = 0; i < unionType->numFields; i++)
		{
			if (unionType->fields[i])
				resetField(unionType->fields[i]);
		}
	}
	else if (type->type == NODE_POINTER_TYPE)
	{
		PointerType* pointerType = (PointerType*)type;
		if (pointerType->elementType)
			resetType(pointerType->elementType);
	}
	else if (type->type == NODE_OPTIONAL_TYPE)
	{
		OptionalType* optionalType = (OptionalType*)type;
		if (optionalType->elementType)
			resetType(optionalType->elementType);
	}
	else if (type->type == NODE_FUNCTION_TYPE)
	{
		FunctionType* functionType = (FunctionType*)type;
		for (int i = 0; i < functionType->numParams; i++)
		{
			if (functionType->params[i])
				resetParameter(functionType->params[i]);
		}
		if (functionType->returnType)
		{
			resetType(functionType->returnType);
		}
	}
	else if (type->type == NODE_TUPLE_TYPE)
	{
		TupleType* tupleType = (TupleType*)type;
		for (int i = 0; i < tupleType->numElementTypes; i++)
		{
			if (tupleType->elementTypes[i])
				resetType(tupleType->elementTypes[i]);
		}
	}
	else if (type->type == NODE_ARRAY_TYPE)
	{
		ArrayType* arrayType = (ArrayType*)type;
		if (arrayType->elementType)
			resetType(arrayType->elementType);
		if (arrayType->size)
			resetExpression(&arrayType->size);
	}
}

void resetExpression(Expression** ref)
{
	Expression* expression = *ref;

	expression->inferredType = nullptr;

	if (expression->type == NODE_COMPOUND_EXPRESSION)
	{
		CompoundExpression* compound = (CompoundExpression*)expression;
		if (compound->value)
			resetExpression(&compound->value);
	}
	else if (expression->type == NODE_EXPRESSION_LIST)
	{
		ExpressionList* expressionList = (ExpressionList*)expression;
		for (int i = 0; i < expressionList->numValues; i++)
		{
			if (expressionList->values[i])
				resetExpression(&expressionList->values[i]);
		}
	}
	else if (expression->type == NODE_BINARY_OPERATOR)
	{
		BinaryOperator* binaryOperator = (BinaryOperator*)expression;
		if (binaryOperator->left)
			resetExpression(&binaryOperator->left);
		if (binaryOperator->right)
			resetExpression(&binaryOperator->right);
	}
	else if (expression->type == NODE_CAST)
	{
		Cast* cast = (Cast*)expression;

		if (cast->expression)
			resetExpression(&cast->expression);
		if (cast->targetType)
			resetType(cast->targetType);

		if (cast->implicit)
		{
			*ref = cast->expression;
		}
	}
	else if (expression->type == NODE_UNARY_OPERATOR)
	{
		UnaryOperator* unaryOperator = (UnaryOperator*)expression;
		if (unaryOperator->operand)
			resetExpression(&unaryOperator->operand);
	}
	else if (expression->type == NODE_FUNCTION_CALL)
	{
		FunctionCall* functionCall = (FunctionCall*)expression;
		if (functionCall->expression)
			resetExpression(&functionCall->expression);
		for (int i = 0; i < functionCall->numArgs; i++)
		{
			if (functionCall->args[i])
				resetExpression(&functionCall->args[i]);
		}
	}
	else if (expression->type == NODE_ARRAY_SUBSCRIPT)
	{
		ArraySubscript* arraySubscript = (ArraySubscript*)expression;
		if (arraySubscript->operand)
			resetExpression(&arraySubscript->operand);
		for (int i = 0; i < arraySubscript->numArgs; i++)
		{
			if (arraySubscript->args[i])
				resetExpression(&arraySubscript->args[i]);
		}
	}
	else if (expression->type == NODE_MEMBER_ACCESS)
	{
		MemberAccess* memberAccess = (MemberAccess*)expression;
		if (memberAccess->operand)
			resetExpression(&memberAccess->operand);
	}
	else if (expression->type == NODE_TERNARY_CONDITION)
	{
		TernaryCondition* ternaryCondition = (TernaryCondition*)expression;
		if (ternaryCondition->condition)
			resetExpression(&ternaryCondition->condition);
		if (ternaryCondition->then)
			resetExpression(&ternaryCondition->then);
		if (ternaryCondition->else_)
			resetExpression(&ternaryCondition->else_);
	}
}

static void resetStatement(Statement* statement)
{
	if (statement->type == NODE_BLOCK_STATEMENT)
	{
		BlockStatement* block = (BlockStatement*)statement;
		for (int i = 0; i < block->numStatements; i++)
		{
			if (block->statements[i])
				resetStatement(block->statements[i]);
		}
	}
	else if (statement->type == NODE_IF)
	{
		If* if_ = (If*)statement;
		if (if_->condition)
			resetExpression(&if_->condition);
		if (if_->then)
			resetStatement(if_->then);
		if (if_->else_)
			resetStatement(if_->else_);
	}
	else if (statement->type == NODE_WHILE)
	{
		While* while_ = (While*)statement;
		if (while_->condition)
			resetExpression(&while_->condition);
		if (while_->then)
			resetStatement(while_->then);
	}
	else if (statement->type == NODE_FOR)
	{
		For* for_ = (For*)statement;
		if (for_->startValue)
			resetExpression(&for_->startValue);
		if (for_->compareValue)
			resetExpression(&for_->compareValue);
		if (for_->body)
			resetStatement(for_->body);
	}
	else if (statement->type == NODE_RETURN)
	{
		Return* return_ = (Return*)statement;
		if (return_->value)
			resetExpression(&return_->value);
	}
	else if (statement->type == NODE_DEFER)
	{
		Defer* defer = (Defer*)statement;
		if (defer->body)
			resetStatement(defer->body);
	}
	else if (statement->type == NODE_VARIABLE_DECLARATION)
	{
		VariableDeclaration* variableDeclaration = (VariableDeclaration*)statement;
		if (variableDeclaration->type)
			resetType(variableDeclaration->variableType);
		for (int i = 0; i < variableDeclaration->numDeclarators; i++)
		{
			if (variableDeclaration->declarators[i].value)
				resetExpression(&variableDeclaration->declarators[i].value);
		}
	}
	else if (statement->type == NODE_ASSIGNMENT)
	{
		Assignment* assignment = (Assignment*)statement;
		if (assignment->expression)
			resetExpression(&assignment->expression);
		if (assignment->value)
			resetExpression(&assignment->value);
	}
	else if (statement->type == NODE_EXPRESSION_STATEMENT)
	{
		ExpressionStatement* expression = (ExpressionStatement*)statement;
		if (expression->expression)
			resetExpression(&expression->expression);
	}
}

static void resetField(Field* field)
{
	resetType(field->variableType);
	for (int i = 0; i < field->numDeclarators; i++)
	{
		if (field->declarators[i].value)
			resetExpression(&field->declarators[i].value);
	}
}

static void resetParameter(Parameter* parameter)
{
	resetType(parameter->paramType);
}

static void resetEnumValue(EnumValue* enumValue)
{
	if (enumValue->value)
		resetExpression(&enumValue->value);
}

static void resetDeclaration(Node* declaration)
{
	if (declaration->type == NODE_STRUCT)
	{
		Struct* struct_ = (Struct*)declaration;
		for (int i = 0; i < struct_->numFields; i++)
		{
			resetField(struct_->fields[i]);
		}
	}
	else if (declaration->type == NODE_UNION)
	{
		Union* union_ = (Union*)declaration;
		for (int i = 0; i < union_->numFields; i++)
		{
			resetField(union_->fields[i]);
		}
	}
	else if (declaration->type == NODE_ENUM)
	{
		Enum* enum_ = (Enum*)declaration;
		for (int i = 0; i < enum_->numValues; i++)
		{
			resetEnumValue(enum_->values[i]);
		}
	}
	else if (declaration->type == NODE_TYPEDEF)
	{
		Typedef* typedef_ = (Typedef*)declaration;
		if (typedef_->value)
			resetType(typedef_->value);
	}
	else if (declaration->type == NODE_FUNCTION)
	{
		Function* function = (Function*)declaration;

		function->scope = nullptr;

		if (function->returnType)
		{
			resetType(function->returnType);
		}

		for (int i = 0; i < function->numParams; i++)
		{
			if (function->params[i])
				resetParameter(function->params[i]);
		}
		if (function->value)
		{
			resetExpression(&function->value);
		}
		else
		{
			for (int i = 0; i < function->numStatements; i++)
			{
				if (function->statements[i])
					resetStatement(function->statements[i]);
			}
		}
	}
	else if (declaration->type == NODE_GLOBAL_VARIABLE)
	{
		GlobalVariable* globalVariable = &declaration->globalVariable;
		resetType(globalVariable->variableType);
		for (int i = 0; i < globalVariable->numDeclarators; i++)
		{
			if (globalVariable->declarators[i].value)
				resetExpression(&globalVariable->declarators[i].value);
		}
	}
	else if (declaration->type == NODE_MACRO)
	{
	}
	else if (declaration->type == NODE_IMPORT)
	{
	}
}

void resetAST(AST* ast)
{
	ast->globalScope = nullptr;

	for (int i = 0; i < ast->numDeclarations; i++)
	{
		resetDeclaration(ast->declarations[i]);
	}
}

static void traverseExpression(Expression* expression, Scope* scope, ASTVisitor_t visitor, void* userPtr);
static void traverseField(Field* field, Scope* scope, ASTVisitor_t visitor, void* userPtr);
static void traverseParameter(Parameter* parameter, Scope* scope, ASTVisitor_t visitor, void* userPtr);

static void traverseType(TypeNode* type, Scope* scope, ASTVisitor_t visitor, void* userPtr)
{
	if (!visitor((Node*)type, scope, userPtr))
		return;

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
		if (arrayType->size)
			traverseExpression(arrayType->size, scope, visitor, userPtr);
	}
}

static void traverseExpression(Expression* expression, Scope* scope, ASTVisitor_t visitor, void* userPtr)
{
	if (!visitor((Node*)expression, scope, userPtr))
		return;

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
		if (unaryOperator->operand)
			traverseExpression(unaryOperator->operand, scope, visitor, userPtr);
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
		if (arraySubscript->operand)
			traverseExpression(arraySubscript->operand, scope, visitor, userPtr);
		for (int i = 0; i < arraySubscript->numArgs; i++)
		{
			if (arraySubscript->args[i])
				traverseExpression(arraySubscript->args[i], scope, visitor, userPtr);
		}
	}
	else if (expression->type == NODE_MEMBER_ACCESS)
	{
		MemberAccess* memberAccess = (MemberAccess*)expression;
		if (memberAccess->operand)
			traverseExpression(memberAccess->operand, scope, visitor, userPtr);
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
	if (!visitor((Node*)statement, scope, userPtr))
		return;

	if (statement->type == NODE_BLOCK_STATEMENT)
	{
		BlockStatement* block = (BlockStatement*)statement;
		block->scope = nullptr;
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
		for_->scope = nullptr;
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
	if (!visitor((Node*)field, scope, userPtr))
		return;

	traverseType(field->variableType, scope, visitor, userPtr);
	for (int i = 0; i < field->numDeclarators; i++)
	{
		if (field->declarators[i].value)
			traverseExpression(field->declarators[i].value, scope, visitor, userPtr);
	}
}

static void traverseParameter(Parameter* parameter, Scope* scope, ASTVisitor_t visitor, void* userPtr)
{
	if (!visitor((Node*)parameter, scope, userPtr))
		return;

	traverseType(parameter->paramType, scope, visitor, userPtr);
}

static void traverseEnumValue(EnumValue* enumValue, Scope* scope, ASTVisitor_t visitor, void* userPtr)
{
	if (!visitor((Node*)enumValue, scope, userPtr))
		return;

	if (enumValue->value)
		traverseExpression(enumValue->value, scope, visitor, userPtr);
}

static void traverseDeclaration(Node* declaration, Scope* scope, ASTVisitor_t visitor, void* userPtr)
{
	if (!visitor(declaration, scope, userPtr))
		return;

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
