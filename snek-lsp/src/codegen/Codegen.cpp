#include "Codegen.h"

#include "File.h"
#include "parser/AST.h"
#include "typechecker/TypeSystem.h"

#include <stdio.h>


void initCodegen(Codegen* codegen, TypeSystem* types, GlobalBlockPool* blockPool)
{
	codegen->types = types;

	initArena(&codegen->arena, blockPool);
	initScratchBuffer(&codegen->scratch, 16);

	initCodeBuffer(&codegen->typesBuffer, &codegen->arena, 1024);
	initCodeBuffer(&codegen->prototypesBuffer, &codegen->arena, 1024);
	initCodeBuffer(&codegen->globalsBuffer, &codegen->arena, 1024);
	initCodeBuffer(&codegen->functionsBuffer, &codegen->arena, 1024);

	codegen->indentation = 0;
	codegen->nextGlobalID = 1;
	codegen->nextLocalID = 1;

	initHashSet(&codegen->declaredTypes);
}

void destroyCodegen(Codegen* codegen)
{
	destroyArena(&codegen->arena);
	destroyScratchBuffer(&codegen->scratch);
}

static void emitType(Codegen* codegen, Type* type, CodeBuffer* buffer);

static void emitIndentation(Codegen* codegen, CodeBuffer* buffer)
{
	for (int i = 0; i < codegen->indentation; i++)
	{
		emitString(buffer, "\t");
	}
}

static void declareAnonymousStructType(Codegen* codegen, Type* type, CodeBuffer* buffer)
{
	emitString(buffer, "typedef struct{\n");

	codegen->indentation++;
	for (int i = 0; i < type->struct_.numFields; i++)
	{
		emitIndentation(codegen, buffer);
		emitType(codegen, type->struct_.fieldTypes[i], buffer);
		emitChar(buffer, ' ');
		if (type->struct_.fieldNames)
			emitString(buffer, type->struct_.fieldNames[i]);
		else
		{
			emitChar(buffer, '_');
			emitInteger(buffer, i);
		}
		emitString(buffer, ";\n");
	}
	codegen->indentation--;

	emitIndentation(codegen, buffer);
	emitString(buffer, "} ");
	emitString(buffer, type->mangledName);
	emitString(buffer, ";\n");
}

static void declareAnonymousUnionType(Codegen* codegen, Type* type, CodeBuffer* buffer)
{
	emitString(buffer, "typedef union{\n");

	codegen->indentation++;
	for (int i = 0; i < type->union_.numFields; i++)
	{
		emitIndentation(codegen, buffer);
		emitType(codegen, type->union_.fieldTypes[i], buffer);
		emitChar(buffer, ' ');
		if (type->union_.fieldNames)
			emitString(buffer, type->union_.fieldNames[i]);
		else
		{
			emitChar(buffer, '_');
			emitInteger(buffer, i);
		}
		emitString(buffer, ";\n");
	}
	codegen->indentation--;

	emitIndentation(codegen, buffer);
	emitString(buffer, "} ");
	emitString(buffer, type->mangledName);
	emitString(buffer, ";\n");
}

static void declareOptionalType(Codegen* codegen, Type* type, CodeBuffer* buffer)
{
	emitString(buffer, "typedef struct{");
	emitString(buffer, type->optional.elementType->mangledName);
	emitString(buffer, " value;u8 flag;}");
	emitString(buffer, type->mangledName);
	emitString(buffer, ";\n");
}

static void declareFunctionType(Codegen* codegen, Type* type, CodeBuffer* buffer)
{
	emitString(buffer, "typedef ");
	emitString(buffer, type->function.returnType ? type->function.returnType->mangledName : codegen->types->primitiveTypes[TYPE_VOID].mangledName);
	emitString(buffer, "(*");
	emitString(buffer, type->mangledName);
	emitString(buffer, ")(");
	for (int i = 0; i < type->function.numParams; i++)
	{
		emitString(buffer, type->function.paramTypes[i]->mangledName);
		if (i < type->function.numParams - 1)
			emitChar(buffer, ',');
	}
	emitString(buffer, ");\n");
}

static void declareArrayType(Codegen* codegen, Type* type, CodeBuffer* buffer)
{
	emitString(buffer, "typedef struct{");
	if (type->array.size)
	{
		emitString(buffer, type->array.elementType->mangledName);
		emitString(buffer, " data[");
		emitInteger(buffer, type->array.size);
		emitString(buffer, "];}");
	}
	else
	{
		emitString(buffer, type->array.elementType->mangledName);
		emitString(buffer, "* data;u64 length;}");
	}
	emitString(buffer, type->mangledName);
	emitString(buffer, ";\n");
}

static void declareType(Codegen* codegen, Type* type)
{
	if (codegen->declaredTypes.contains(type))
		return;

	if (type->typeKind == TYPE_STRUCT && !type->struct_.name.length)
	{
		for (int i = 0; i < type->struct_.numFields; i++)
		{
			declareType(codegen, type->struct_.fieldTypes[i]);
		}

		declareAnonymousStructType(codegen, type, &codegen->typesBuffer);

		codegen->declaredTypes.add(type);
	}
	else if (type->typeKind == TYPE_UNION && !type->union_.name.length)
	{
		for (int i = 0; i < type->union_.numFields; i++)
		{
			declareType(codegen, type->union_.fieldTypes[i]);
		}

		declareAnonymousUnionType(codegen, type, &codegen->typesBuffer);

		codegen->declaredTypes.add(type);
	}
	else if (type->typeKind == TYPE_OPTIONAL)
	{
		declareType(codegen, type->optional.elementType);

		declareOptionalType(codegen, type, &codegen->typesBuffer);

		codegen->declaredTypes.add(type);
	}
	else if (type->typeKind == TYPE_FUNCTION)
	{
		if (type->function.returnType)
			declareType(codegen, type->function.returnType);
		for (int i = 0; i < type->function.numParams; i++)
		{
			declareType(codegen, type->function.paramTypes[i]);
		}

		declareFunctionType(codegen, type, &codegen->typesBuffer);

		codegen->declaredTypes.add(type);
	}
	else if (type->typeKind == TYPE_ARRAY)
	{
		declareType(codegen, type->array.elementType);

		declareArrayType(codegen, type, &codegen->typesBuffer);

		codegen->declaredTypes.add(type);
	}
}

static void emitType(Codegen* codegen, Type* type, CodeBuffer* buffer)
{
	if (type->typeKind == TYPE_VOID)
	{
		emitString(buffer, "void");
	}
	else if (type->typeKind == TYPE_INT8)
	{
		emitString(buffer, "i8");
	}
	else if (type->typeKind == TYPE_INT16)
	{
		emitString(buffer, "i16");
	}
	else if (type->typeKind == TYPE_INT32)
	{
		emitString(buffer, "i32");
	}
	else if (type->typeKind == TYPE_INT64)
	{
		emitString(buffer, "i64");
	}
	else if (type->typeKind == TYPE_UINT8)
	{
		emitString(buffer, "u8");
	}
	else if (type->typeKind == TYPE_UINT16)
	{
		emitString(buffer, "u16");
	}
	else if (type->typeKind == TYPE_UINT32)
	{
		emitString(buffer, "u32");
	}
	else if (type->typeKind == TYPE_UINT64)
	{
		emitString(buffer, "u64");
	}
	else if (type->typeKind == TYPE_FLOAT)
	{
		emitString(buffer, "float");
	}
	else if (type->typeKind == TYPE_DOUBLE)
	{
		emitString(buffer, "double");
	}
	else if (type->typeKind == TYPE_BOOL)
	{
		emitString(buffer, "bool");
	}
	else if (type->typeKind == TYPE_ANY)
	{
		emitString(buffer, "any");
	}
	else if (type->typeKind == TYPE_STRING)
	{
		emitString(buffer, "string");
	}
	else if (type->typeKind == TYPE_STRUCT)
	{
		if (type->struct_.name.length)
		{
			emitString(buffer, "struct ");
			emitString(buffer, type->struct_.name);
		}
		else
		{
			SnekAssert(codegen->declaredTypes.contains(type));

			emitString(buffer, type->mangledName);
		}
	}
	else if (type->typeKind == TYPE_UNION)
	{
		if (type->union_.name.length)
		{
			emitString(buffer, "union ");
			emitString(buffer, type->union_.name);
		}
		else
		{
			SnekAssert(codegen->declaredTypes.contains(type));

			emitString(buffer, type->mangledName);
		}
	}
	else if (type->typeKind == TYPE_ENUM)
	{
		emitString(buffer, "enum ");
		emitString(buffer, type->enum_.name);
	}
	else if (type->typeKind == TYPE_ALIAS)
	{
		emitString(buffer, type->enum_.name);
	}
	else if (type->typeKind == TYPE_POINTER)
	{
		emitType(codegen, type->pointer.elementType, buffer);
		emitChar(buffer, '*');
	}
	else if (type->typeKind == TYPE_OPTIONAL)
	{
		SnekAssert(codegen->declaredTypes.contains(type));

		emitString(buffer, type->mangledName);
	}
	else if (type->typeKind == TYPE_FUNCTION)
	{
		SnekAssert(codegen->declaredTypes.contains(type));

		emitString(buffer, type->mangledName);
	}
	else if (type->typeKind == TYPE_ARRAY)
	{
		SnekAssert(codegen->declaredTypes.contains(type));

		emitString(buffer, type->mangledName);
	}
	else if (type->typeKind == TYPE_TYPE)
	{
		emitString(buffer, "type");
	}
}

struct Value
{
	char name[32];
	Type* type;
	bool lvalue;
};

static Value createGlobalValue(Codegen* codegen, Type* type)
{
	Value value = {};
	value.type = type;
	sprintf(value.name, "_G%d", codegen->nextGlobalID++);
	return value;
}

static void emitValue(CodeBuffer* buffer, Value value)
{
	emitString(buffer, value.name);
}

static Value declareLocalValue(Codegen* codegen, Type* type, CodeBuffer* buffer)
{
	Value value = {};
	value.type = type;
	sprintf(value.name, "_%d", codegen->nextLocalID++);

	emitIndentation(codegen, buffer);
	emitString(buffer, "const ");
	emitType(codegen, type, buffer);
	emitChar(buffer, ' ');
	emitValue(buffer, value);
	emitChar(buffer, '=');

	return value;
}

static void emitOperator(CodeBuffer* buffer, OperatorType op)
{
	if (op == OPERATOR_MULTIPLY)
		emitChar(buffer, '*');
	else if (op == OPERATOR_DIVIDE)
		emitChar(buffer, '/');
	else if (op == OPERATOR_MODULO)
		emitChar(buffer, '%');
	else if (op == OPERATOR_ADD)
		emitChar(buffer, '+');
	else if (op == OPERATOR_SUBTRACT)
		emitChar(buffer, '-');
	else if (op == OPERATOR_BITSHIFT_LEFT)
		emitString(buffer, "<<");
	else if (op == OPERATOR_BITSHIFT_RIGHT)
		emitString(buffer, ">>");
	else if (op == OPERATOR_LESS)
		emitChar(buffer, '<');
	else if (op == OPERATOR_LESS_EQUALS)
		emitString(buffer, "<=");
	else if (op == OPERATOR_GREATER)
		emitChar(buffer, '>');
	else if (op == OPERATOR_GREATER_EQUALS)
		emitString(buffer, ">=");
	else if (op == OPERATOR_EQUALS)
		emitString(buffer, "==");
	else if (op == OPERATOR_NOT_EQUALS)
		emitString(buffer, "!=");
	else if (op == OPERATOR_BITWISE_AND)
		emitChar(buffer, '&');
	else if (op == OPERATOR_BITWISE_XOR)
		emitChar(buffer, '^');
	else if (op == OPERATOR_BITWISE_OR)
		emitChar(buffer, '|');
	else if (op == OPERATOR_LOGICAL_AND)
		emitString(buffer, "&&");
	else if (op == OPERATOR_LOGICAL_OR)
		emitString(buffer, "||");
	else
	{
		SnekAssert(false);
	}
}

static Value emitExpression(Codegen* codegen, Expression* expression, CodeBuffer* buffer)
{
	if (expression->type == NODE_INT_LITERAL)
	{
		IntLiteral* intLiteral = (IntLiteral*)expression;
		Value value = {};
		value.type = expression->inferredType;
		sprintf(value.name, intLiteral->negative ? "-%llu" : "%llu", intLiteral->intValue);
		return value;
	}
	else if (expression->type == NODE_FLOAT_LITERAL)
	{
		FloatLiteral* floatLiteral = (FloatLiteral*)expression;
		Value value = {};
		value.type = expression->inferredType;
		sprintf(value.name, "%f", floatLiteral->floatValue);
		return value;
	}
	else if (expression->type == NODE_STRING_LITERAL)
	{
		StringLiteral* stringLiteral = (StringLiteral*)expression;

		Value ptr = createGlobalValue(codegen, nullptr);

		emitString(&codegen->globalsBuffer, "const i8* ");
		emitValue(&codegen->globalsBuffer, ptr);
		emitString(&codegen->globalsBuffer, "=\"");
		emitString(&codegen->globalsBuffer, stringLiteral->value);
		emitString(&codegen->globalsBuffer, "\";\n");

		Value str = declareLocalValue(codegen, &codegen->types->primitiveTypes[TYPE_STRING], buffer);
		emitChar(buffer, '{');
		emitValue(buffer, ptr);
		emitChar(buffer, ',');
		emitInteger(buffer, stringLiteral->value.length);
		emitString(buffer, "};\n");

		return str;
	}
	else if (expression->type == NODE_CHAR_LITERAL)
	{
		CharLiteral* charLiteral = (CharLiteral*)expression;
		Value value = {};
		value.type = expression->inferredType;
		sprintf(value.name, "'%.*s'", charLiteral->value.length, charLiteral->value.ptr);
		return value;
	}
	else if (expression->type == NODE_TRUE)
	{
		Value value = {};
		value.type = expression->inferredType;
		strcat(value.name, "true");
		return value;
	}
	else if (expression->type == NODE_FALSE)
	{
		Value value = {};
		value.type = expression->inferredType;
		strcat(value.name, "false");
		return value;
	}
	else if (expression->type == NODE_NULL_LITERAL)
	{
		Value value = {};
		value.type = expression->inferredType;
		strcat(value.name, "{0}");
		return value;
	}
	else if (expression->type == NODE_IDENTIFIER)
	{
		Identifier* identifier = (Identifier*)expression;
		Value value = {};
		value.type = expression->inferredType;
		value.lvalue = true;
		sprintf(value.name, "%.*s", identifier->name.length, identifier->name.ptr);
		return value;
	}
	else if (expression->type == NODE_COMPOUND_EXPRESSION)
	{
		CompoundExpression* compound = (CompoundExpression*)expression;
		return emitExpression(codegen, compound->value, buffer);
	}
	else if (expression->type == NODE_EXPRESSION_LIST)
	{
		ExpressionList* expressionList = (ExpressionList*)expression;

		int mark = codegen->scratch.mark();
		for (int i = 0; i < expressionList->numValues; i++)
		{
			Value value = emitExpression(codegen, expressionList->values[i], buffer);
			codegen->scratch.add(value);
		}

		Value* values = codegen->scratch.getData<Value>(mark);

		Value tuple = declareLocalValue(codegen, expressionList->inferredType, buffer);
		emitChar(buffer, '{');

		for (int i = 0; i < expressionList->numValues; i++)
		{
			Value value = values[i];
			emitValue(buffer, value);
			if (i < expressionList->numValues - 1)
				emitChar(buffer, ',');
		}

		emitString(buffer, "};\n");

		codegen->scratch.release(mark);

		return tuple;
	}
	else if (expression->type == NODE_BINARY_OPERATOR)
	{
		BinaryOperator* binaryOperator = (BinaryOperator*)expression;

		Value left = emitExpression(codegen, binaryOperator->left, buffer);
		Value right = emitExpression(codegen, binaryOperator->right, buffer);

		Value result = declareLocalValue(codegen, binaryOperator->inferredType, buffer);

		emitValue(buffer, left);
		emitOperator(buffer, binaryOperator->op);
		emitValue(buffer, right);
		emitString(buffer, ";\n");

		return result;
	}
	else if (expression->type == NODE_UNARY_OPERATOR)
	{
		UnaryOperator* unaryOperator = (UnaryOperator*)expression;

		Value operand = emitExpression(codegen, unaryOperator->operand, buffer);

		if (unaryOperator->op == OPERATOR_INCREMENT_POSTFIX)
		{
			SnekAssert(operand.lvalue);

			Value oldValue = declareLocalValue(codegen, unaryOperator->inferredType, buffer);
			emitValue(buffer, operand);
			emitString(buffer, ";\n");

			emitIndentation(codegen, buffer);
			emitValue(buffer, operand);
			emitChar(buffer, '=');
			emitValue(buffer, operand);
			emitString(buffer, "+1;\n");

			return oldValue;
		}
		else if (unaryOperator->op == OPERATOR_DECREMENT_POSTFIX)
		{
			SnekAssert(operand.lvalue);

			Value oldValue = declareLocalValue(codegen, unaryOperator->inferredType, buffer);
			emitValue(buffer, operand);
			emitString(buffer, ";\n");

			emitIndentation(codegen, buffer);
			emitValue(buffer, operand);
			emitChar(buffer, '=');
			emitValue(buffer, operand);
			emitString(buffer, "-1;\n");

			return oldValue;
		}
		else if (unaryOperator->op == OPERATOR_INCREMENT_PREFIX)
		{
			SnekAssert(operand.lvalue);

			emitIndentation(codegen, buffer);
			emitValue(buffer, operand);
			emitChar(buffer, '=');
			emitValue(buffer, operand);
			emitString(buffer, "+1;\n");

			return operand;
		}
		else if (unaryOperator->op == OPERATOR_DECREMENT_PREFIX)
		{
			SnekAssert(operand.lvalue);

			emitIndentation(codegen, buffer);
			emitValue(buffer, operand);
			emitChar(buffer, '=');
			emitValue(buffer, operand);
			emitString(buffer, "-1;\n");

			return operand;
		}
		else if (unaryOperator->op == OPERATOR_PLUS_PREFIX)
		{
			return operand;
		}
		else if (unaryOperator->op == OPERATOR_MINUS_PREFIX)
		{
			Value result = declareLocalValue(codegen, unaryOperator->inferredType, buffer);
			emitChar(buffer, '-');
			emitValue(buffer, operand);
			emitString(buffer, ";\n");

			return result;
		}
		else if (unaryOperator->op == OPERATOR_LOGICAL_NOT)
		{
			Value result = declareLocalValue(codegen, unaryOperator->inferredType, buffer);
			emitChar(buffer, '!');
			emitValue(buffer, operand);
			emitString(buffer, ";\n");

			return result;
		}
		else if (unaryOperator->op == OPERATOR_BITWISE_NOT)
		{
			Value result = declareLocalValue(codegen, unaryOperator->inferredType, buffer);
			emitChar(buffer, '~');
			emitValue(buffer, operand);
			emitString(buffer, ";\n");

			return result;
		}
		else if (unaryOperator->op == OPERATOR_DEREFERENCE)
		{
			Value result = {};
			result.type = unaryOperator->inferredType;
			result.lvalue = true;
			sprintf(result.name, "(*%s)", operand.name);

			return result;
		}
		else if (unaryOperator->op == OPERATOR_ADDRESS)
		{
			Value result = {};
			result.type = unaryOperator->inferredType;
			sprintf(result.name, "(&%s)", operand.name);

			return result;
		}
		else
		{
			SnekAssert(false);
			return {};
		}
	}
	else if (expression->type == NODE_FUNCTION_CALL)
	{
		FunctionCall* functionCall = (FunctionCall*)expression;

		Value operand = emitExpression(codegen, functionCall->expression, buffer);

		int mark = codegen->scratch.mark();
		for (int i = 0; i < functionCall->numArgs; i++)
		{
			Value arg = emitExpression(codegen, functionCall->args[i], buffer);
			codegen->scratch.add(arg);
		}

		Value* args = codegen->scratch.getData<Value>(mark);

		Value result = {};
		if (functionCall->inferredType)
			result = declareLocalValue(codegen, functionCall->inferredType, buffer);
		else
			emitIndentation(codegen, buffer);

		emitValue(buffer, operand);
		emitChar(buffer, '(');
		for (int i = 0; i < functionCall->numArgs; i++)
		{
			emitValue(buffer, args[i]);
			if (i < functionCall->numArgs - 1)
				emitChar(buffer, ',');
		}
		emitString(buffer, ");\n");

		codegen->scratch.release(mark);

		return result;
	}
	else if (expression->type == NODE_ARRAY_SUBSCRIPT)
	{
		ArraySubscript* subscript = (ArraySubscript*)expression;

		Value operand = emitExpression(codegen, subscript->operand, buffer);

		if (operand.type->typeKind == TYPE_STRING)
		{
			SnekAssert(subscript->numArgs == 1);
			Value index = emitExpression(codegen, subscript->args[0], buffer);

			Type* charPtrType = getPointerType(codegen->types, subscript->inferredType, codegen->currentFile);
			Value charPtr = declareLocalValue(codegen, charPtrType, buffer);
			emitChar(buffer, '&');
			emitValue(buffer, operand);
			emitString(buffer, ".ptr[");
			emitValue(buffer, index);
			emitString(buffer, "];\n");

			Value result = {};
			result.type = subscript->inferredType;
			result.lvalue = true;
			sprintf(result.name, "(*%s)", charPtr.name);

			return result;
		}
		else if (operand.type->typeKind == TYPE_ARRAY)
		{
			SnekAssert(subscript->numArgs == 1);
			Value index = emitExpression(codegen, subscript->args[0], buffer);

			Type* ptrType = getPointerType(codegen->types, subscript->inferredType, codegen->currentFile);
			Value ptr = declareLocalValue(codegen, ptrType, buffer);
			emitChar(buffer, '&');
			emitValue(buffer, operand);
			emitString(buffer, ".data[");
			emitValue(buffer, index);
			emitString(buffer, "];\n");

			Value result = {};
			result.type = subscript->inferredType;
			result.lvalue = true;
			sprintf(result.name, "(*%s)", ptr.name);

			return result;
		}
		else if (operand.type->typeKind == TYPE_POINTER)
		{
			SnekAssert(subscript->numArgs == 1);
			Value index = emitExpression(codegen, subscript->args[0], buffer);

			Type* ptrType = getPointerType(codegen->types, subscript->inferredType, codegen->currentFile);
			Value ptr = declareLocalValue(codegen, ptrType, buffer);
			emitChar(buffer, '&');
			emitValue(buffer, operand);
			emitChar(buffer, '[');
			emitValue(buffer, index);
			emitString(buffer, "];\n");

			Value result = {};
			result.type = subscript->inferredType;
			result.lvalue = true;
			sprintf(result.name, "(*%s)", ptr.name);

			return result;
		}
		else
		{
			SnekAssert(false);
			return {};
		}
	}
	else if (expression->type == NODE_MEMBER_ACCESS)
	{
		MemberAccess* member = (MemberAccess*)expression;

		Value operand = emitExpression(codegen, member->operand, buffer);
		Type* operandType = member->operand->inferredType;

		bool pointer = operandType->typeKind == TYPE_POINTER;
		if (pointer)
			operandType = operandType->pointer.elementType;

		if (operandType->typeKind == TYPE_STRUCT)
		{
			Type* ptrType = getPointerType(codegen->types, member->inferredType, codegen->currentFile);
			Value ptr = declareLocalValue(codegen, ptrType, buffer);
			emitChar(buffer, '&');
			emitValue(buffer, operand);
			emitChar(buffer, '.');
			emitString(buffer, operandType->struct_.fieldNames[member->index]);
			emitString(buffer, ";\n");

			Value result = {};
			result.type = member->inferredType;
			result.lvalue = true;
			sprintf(result.name, "(*%s)", ptr.name);

			return result;
		}
		else if (operandType->typeKind == TYPE_UNION)
		{
			Type* ptrType = getPointerType(codegen->types, member->inferredType, codegen->currentFile);
			Value ptr = declareLocalValue(codegen, ptrType, buffer);
			emitChar(buffer, '&');
			emitValue(buffer, operand);
			emitChar(buffer, '.');
			emitString(buffer, operandType->union_.fieldNames[member->index]);
			emitString(buffer, ";\n");

			Value result = {};
			result.type = member->inferredType;
			result.lvalue = true;
			sprintf(result.name, "(*%s)", ptr.name);

			return result;
		}
		else if (operandType->typeKind == TYPE_STRING)
		{
			if (member->index == 0)
			{
				Value length = declareLocalValue(codegen, member->inferredType, buffer);
				emitValue(buffer, operand);
				emitString(buffer, ".length;\n");

				return length;
			}
			else if (member->index == 1)
			{
				Value data = declareLocalValue(codegen, member->inferredType, buffer);
				emitValue(buffer, operand);
				emitString(buffer, ".ptr;\n");

				return data;
			}
			else
			{
				SnekAssert(false);
				return {};
			}
		}
		else if (operandType->typeKind == TYPE_ARRAY)
		{
			if (member->index == 0)
			{
				if (operandType->array.size)
				{
					Value value = {};
					value.type = member->inferredType;
					sprintf(value.name, "%llu", operandType->array.size);

					return value;
				}
				else
				{
					Value length = declareLocalValue(codegen, member->inferredType, buffer);
					emitValue(buffer, operand);
					emitString(buffer, ".length;\n");

					return length;
				}
			}
			else if (member->index == 1)
			{
				Value data = declareLocalValue(codegen, member->inferredType, buffer);
				emitValue(buffer, operand);
				emitString(buffer, ".data;\n");

				return data;
			}
			else
			{
				SnekAssert(false);
				return {};
			}
		}
		else if (operandType->typeKind == TYPE_ANY)
		{
			if (member->index == 0)
			{
				Value type = declareLocalValue(codegen, member->inferredType, buffer);
				emitValue(buffer, operand);
				emitString(buffer, ".type;\n");

				return type;
			}
			else
			{
				SnekAssert(false);
				return {};
			}
		}
		else if (operandType->typeKind == TYPE_TYPE)
		{
			SnekAssert(member->operand->type == NODE_IDENTIFIER);

			Identifier* typeName = (Identifier*)member->operand;
			SymbolEntry* symbol = getIdentifierSymbol(typeName);
			SnekAssert(symbol);

			if (symbol->declaration->type == NODE_ENUM)
			{
				Enum* enum_ = &symbol->declaration->enum_;

				EnumValue* enumValue = enum_->values[member->index];

				Value value = {};
				value.type = member->inferredType;
				sprintf(value.name, "%lld", enumValue->intValue);

				return value;
			}
			else
			{
				SnekAssert(false);
				return {};
			}
		}
		else
		{
			SnekAssert(false);
			return {};
		}
	}
	else if (expression->type == NODE_TERNARY_CONDITION)
	{
		TernaryCondition* ternary = (TernaryCondition*)expression;

		Value condition = emitExpression(codegen, ternary->condition, buffer);
		Value then = emitExpression(codegen, ternary->then, buffer);
		Value else_ = emitExpression(codegen, ternary->else_, buffer);

		Value result = declareLocalValue(codegen, ternary->inferredType, buffer);
		emitValue(buffer, condition);
		emitChar(buffer, '?');
		emitValue(buffer, then);
		emitChar(buffer, ':');
		emitValue(buffer, else_);
		emitString(buffer, ";\n");

		return result;
	}
	else if (expression->type == NODE_CAST)
	{
		Cast* cast = (Cast*)expression;

		Value expression = emitExpression(codegen, cast->expression, buffer);

		Value result = declareLocalValue(codegen, cast->inferredType, buffer);
		emitChar(buffer, '(');
		emitType(codegen, result.type, buffer);
		emitChar(buffer, ')');
		emitValue(buffer, expression);
		emitString(buffer, ";\n");

		return result;
	}

	SnekAssert(false);
	return {};
}

static void emitStatement(Codegen* codegen, Statement* statement, CodeBuffer* buffer)
{
	if (statement->type == NODE_BLOCK_STATEMENT)
	{
		BlockStatement* block = (BlockStatement*)statement;
		for (int i = 0; i < block->numStatements; i++)
		{
			emitStatement(codegen, block->statements[i], buffer);
		}
	}
	else if (statement->type == NODE_IF)
	{
		If* if_ = (If*)statement;
		Value condition = emitExpression(codegen, if_->condition, buffer);

		emitIndentation(codegen, buffer);
		emitString(buffer, "if(");
		emitValue(buffer, condition);
		emitString(buffer, "){\n");

		codegen->indentation++;
		emitStatement(codegen, if_->then, buffer);
		codegen->indentation--;

		emitIndentation(codegen, buffer);
		emitString(buffer, "}");

		if (if_->else_)
		{
			emitString(buffer, "else{\n");

			codegen->indentation++;
			emitStatement(codegen, if_->else_, buffer);
			codegen->indentation--;

			emitIndentation(codegen, buffer);
			emitString(buffer, "}");
		}

		emitString(buffer, "\n");
	}
	else if (statement->type == NODE_WHILE)
	{
		While* while_ = (While*)statement;

		emitIndentation(codegen, buffer);
		emitString(buffer, "while(1){\n");

		codegen->indentation++;

		Value condition = emitExpression(codegen, while_->condition, buffer);
		emitIndentation(codegen, buffer);
		emitString(buffer, "if(!");
		emitValue(buffer, condition);
		emitString(buffer, ")break;\n");

		emitStatement(codegen, while_->then, buffer);

		codegen->indentation--;

		emitIndentation(codegen, buffer);
		emitString(buffer, "}\n");
	}
	else if (statement->type == NODE_FOR)
	{
	}
	else if (statement->type == NODE_RETURN)
	{
	}
	else if (statement->type == NODE_BREAK)
	{
	}
	else if (statement->type == NODE_CONTINUE)
	{
	}
	else if (statement->type == NODE_DEFER)
	{
	}
	else if (statement->type == NODE_VARIABLE_DECLARATION)
	{
	}
	else if (statement->type == NODE_ASSIGNMENT)
	{
	}
	else if (statement->type == NODE_EXPRESSION_STATEMENT)
	{
		ExpressionStatement* expression = (ExpressionStatement*)statement;
		emitExpression(codegen, expression->expression, buffer);
	}
}

static void emitField(Codegen* codegen, Field* field, CodeBuffer* buffer)
{
	for (int i = 0; i < field->numDeclarators; i++)
	{
		emitIndentation(codegen, buffer);
		emitType(codegen, field->variableType->inferredType, buffer);
		emitString(buffer, " ");
		emitString(buffer, field->declarators[i].name);
		if (field->declarators[i].value)
		{
			emitString(buffer, "=");
			emitExpression(codegen, field->declarators[i].value, buffer);
		}
		emitString(buffer, ";\n");
	}
}

static void emitStructDeclaration(Codegen* codegen, Struct* struct_, CodeBuffer* buffer)
{
	emitString(buffer, "struct ");
	emitString(buffer, struct_->name);
	emitString(buffer, ";\n");
}

static void emitStruct(Codegen* codegen, Struct* struct_, CodeBuffer* buffer)
{
	for (int i = 0; i < struct_->numFields; i++)
	{
		declareType(codegen, struct_->fields[i]->variableType->inferredType);
	}

	emitString(buffer, "struct ");
	emitString(buffer, struct_->name);
	emitString(buffer, "{\n");

	codegen->indentation++;
	for (int i = 0; i < struct_->numFields; i++)
	{
		Field* field = struct_->fields[i];
		emitField(codegen, field, buffer);
	}
	codegen->indentation--;

	emitString(buffer, "};\n");
}

static void emitUnionDeclaration(Codegen* codegen, Union* union_, CodeBuffer* buffer)
{
	emitString(buffer, "union ");
	emitString(buffer, union_->name);
	emitString(buffer, ";\n");
}

static void emitUnion(Codegen* codegen, Union* union_, CodeBuffer* buffer)
{
	for (int i = 0; i < union_->numFields; i++)
	{
		declareType(codegen, union_->fields[i]->variableType->inferredType);
	}

	emitString(buffer, "union ");
	emitString(buffer, union_->name);
	emitString(buffer, "{\n");

	codegen->indentation++;
	for (int i = 0; i < union_->numFields; i++)
	{
		Field* field = union_->fields[i];
		emitField(codegen, field, buffer);
	}
	codegen->indentation--;

	emitString(buffer, "};\n");
}

static void emitEnum(Codegen* codegen, Enum* enum_, CodeBuffer* buffer)
{
	declareType(codegen, enum_->enumType->enum_.valueType);

	emitString(buffer, "enum ");
	emitString(buffer, enum_->name);
	emitString(buffer, ":");
	emitType(codegen, enum_->enumType->enum_.valueType, buffer);
	emitString(buffer, "{\n");

	codegen->indentation++;
	for (int i = 0; i < enum_->numValues; i++)
	{
		emitIndentation(codegen, buffer);
		EnumValue* value = enum_->values[i];
		emitString(buffer, value->name);
		if (value->value)
		{
			emitString(buffer, "=");
			emitExpression(codegen, value->value, buffer);
		}
		emitString(buffer, ",\n");
	}
	codegen->indentation--;

	emitString(buffer, "};\n");
}

static void emitTypedef(Codegen* codegen, Typedef* typedef_, CodeBuffer* buffer)
{
	declareType(codegen, typedef_->value->inferredType);

	emitString(buffer, "typedef ");
	emitType(codegen, typedef_->value->inferredType, buffer);
	emitString(buffer, " ");
	emitString(buffer, typedef_->name);
	emitString(buffer, ";\n");
}

static void emitFunctionDeclaration(Codegen* codegen, Function* function, CodeBuffer* buffer)
{
	if (function->functionType->function.returnType)
		declareType(codegen, function->functionType->function.returnType);

	for (int i = 0; i < function->functionType->function.numParams; i++)
	{
		declareType(codegen, function->functionType->function.paramTypes[i]);
	}

	emitType(codegen, function->functionType->function.returnType ? function->functionType->function.returnType : &codegen->types->primitiveTypes[TYPE_VOID], buffer);
	emitString(buffer, " ");
	emitString(buffer, function->name); // todo mangled name
	emitString(buffer, "(");
	for (int i = 0; i < function->functionType->function.numParams; i++)
	{
		emitType(codegen, function->functionType->function.paramTypes[i], buffer);
		if (function->params[i]->name.length)
		{
			emitString(buffer, " ");
			emitString(buffer, function->params[i]->name);
		}
		if (i < function->functionType->function.numParams - 1)
			emitString(buffer, ",");
	}
	emitString(buffer, ");\n");
}

static void emitFunction(Codegen* codegen, Function* function, CodeBuffer* buffer)
{
	codegen->nextLocalID = 1;

	if (function->functionType->function.returnType)
		declareType(codegen, function->functionType->function.returnType);

	for (int i = 0; i < function->functionType->function.numParams; i++)
	{
		declareType(codegen, function->functionType->function.paramTypes[i]);
	}

	emitType(codegen, function->functionType->function.returnType ? function->functionType->function.returnType : &codegen->types->primitiveTypes[TYPE_VOID], buffer);
	emitString(buffer, " ");
	emitString(buffer, function->name); // todo mangled name
	emitString(buffer, "(");
	for (int i = 0; i < function->functionType->function.numParams; i++)
	{
		emitType(codegen, function->functionType->function.paramTypes[i], buffer);
		if (function->params[i]->name.length)
		{
			emitString(buffer, " ");
			emitString(buffer, function->params[i]->name);
		}
		if (i < function->functionType->function.numParams - 1)
			emitString(buffer, ",");
	}
	emitString(buffer, "){\n");

	if (function->value)
	{
		codegen->indentation++;
		for (int i = 0; i < function->numStatements; i++)
		{
			emitIndentation(codegen, buffer);
			emitString(buffer, "return ");
			emitExpression(codegen, function->value, buffer);
			emitString(buffer, ";\n");
		}
		codegen->indentation--;
	}
	else
	{
		codegen->indentation++;
		for (int i = 0; i < function->numStatements; i++)
		{
			emitStatement(codegen, function->statements[i], buffer);
		}
		codegen->indentation--;
	}

	emitString(buffer, "}\n");
}

static void emitGlobalVariable(Codegen* codegen, GlobalVariable* globalVariable, CodeBuffer* buffer)
{

}

bool emitFile(Codegen* codegen, File* f, const char* localPath, const char* out)
{
	codegen->declaredTypes.clear();
	codegen->currentFile = f;

	codegen->nextGlobalID = 1;

	AST* ast = &f->ast;

	FILE* file = fopen(out, "wb");
	if (!file)
		return false;

	resetCodeBuffer(&codegen->typesBuffer);
	resetCodeBuffer(&codegen->prototypesBuffer);
	resetCodeBuffer(&codegen->globalsBuffer);
	resetCodeBuffer(&codegen->functionsBuffer);

	char prologue[256];
	prologue[0] = 0;

	strcat(prologue, "// ");
	strcat(prologue, localPath);
	strcat(prologue, "\n#include <cgl.h>\n");

	fwrite(prologue, 1, strlen(prologue), file);

	for (int i = 0; i < ast->numDeclarations; i++)
	{
		Node* declaration = ast->declarations[i];
		if (declaration->type == NODE_STRUCT)
		{
			emitStructDeclaration(codegen, &declaration->struct_, &codegen->typesBuffer);
		}
		else if (declaration->type == NODE_UNION)
		{
			emitUnionDeclaration(codegen, &declaration->union_, &codegen->typesBuffer);
		}
	}

	for (int i = 0; i < ast->numDeclarations; i++)
	{
		Node* declaration = ast->declarations[i];
		if (declaration->type == NODE_STRUCT)
		{
			emitStruct(codegen, &declaration->struct_, &codegen->typesBuffer);
		}
		else if (declaration->type == NODE_UNION)
		{
			emitUnion(codegen, &declaration->union_, &codegen->typesBuffer);
		}
		else if (declaration->type == NODE_ENUM)
		{
			emitEnum(codegen, &declaration->enum_, &codegen->typesBuffer);
		}
		else if (declaration->type == NODE_TYPEDEF)
		{
			emitTypedef(codegen, &declaration->typedef_, &codegen->typesBuffer);
		}
		else if (declaration->type == NODE_FUNCTION)
		{
			emitFunctionDeclaration(codegen, &declaration->function, &codegen->prototypesBuffer);
			if (declaration->function.hasBody)
				emitFunction(codegen, &declaration->function, &codegen->functionsBuffer);
		}
		else if (declaration->type == NODE_GLOBAL_VARIABLE)
		{
			emitGlobalVariable(codegen, &declaration->globalVariable, &codegen->globalsBuffer);
		}
	}

	SnekAssert(codegen->indentation == 0);

	if (codegen->typesBuffer.count)
	{
		fwrite("\n\n", 1, 2, file);
		fwrite(codegen->typesBuffer.data, 1, codegen->typesBuffer.count, file);
	}
	if (codegen->prototypesBuffer.count)
	{
		fwrite("\n\n", 1, 2, file);
		fwrite(codegen->prototypesBuffer.data, 1, codegen->prototypesBuffer.count, file);
	}
	if (codegen->globalsBuffer.count)
	{
		fwrite("\n\n", 1, 2, file);
		fwrite(codegen->globalsBuffer.data, 1, codegen->globalsBuffer.count, file);
	}
	if (codegen->functionsBuffer.count)
	{
		fwrite("\n\n", 1, 2, file);
		fwrite(codegen->functionsBuffer.data, 1, codegen->functionsBuffer.count, file);
	}

	fclose(file);

	return true;
}
