#include "Codegen.h"

#include "File.h"
#include "Value.h"
#include "parser/AST.h"
#include "typechecker/TypeSystem.h"

#include <stdio.h>


void initCodegen(Codegen* codegen, TypeSystem* types, Arena* globalArena)
{
	codegen->types = types;
	codegen->arena = globalArena;

	initScratchBuffer(&codegen->scratch, 16);

	initCodeBuffer(&codegen->typesBuffer, codegen->arena, 1024);
	initCodeBuffer(&codegen->prototypesBuffer, codegen->arena, 1024);
	initCodeBuffer(&codegen->globalsBuffer, codegen->arena, 1024);
	initCodeBuffer(&codegen->functionsBuffer, codegen->arena, 1024);

	codegen->indentation = 0;
	codegen->nextGlobalID = 1;
	codegen->nextLocalID = 1;

	initTypeSet(&codegen->declaredTypes);
	initTypeSet(&codegen->declaredTypeStubs);
}

void destroyCodegen(Codegen* codegen)
{
	destroyTypeSet(&codegen->declaredTypes);
	destroyTypeSet(&codegen->declaredTypeStubs);
	destroyScratchBuffer(&codegen->scratch);
}

static void declareType(Codegen* codegen, Type* type);
static void emitType(Codegen* codegen, Type* type, CodeBuffer* buffer);
static Value emitExpression(Codegen* codegen, Expression* expression, CodeBuffer* buffer);

static Value createGlobalValue(Codegen* codegen, Type* type)
{
	Value value = {};
	value.type = type;
	snprintf(value.name, sizeof(value.name), "_G%d", codegen->nextGlobalID++);
	return value;
}

static void emitValue(CodeBuffer* buffer, Value value)
{
	if (value.isIdentifier)
	{
		emitString(buffer, value.identifier);
	}
	else
	{
		emitString(buffer, value.name);
	}
}

static void emitIndentation(Codegen* codegen, CodeBuffer* buffer)
{
	for (int i = 0; i < codegen->indentation; i++)
	{
		emitString(buffer, "\t");
	}
}

static Value createLocalValue(Codegen* codegen, Type* type)
{
	Value value = {};
	value.type = type;
	snprintf(value.name, sizeof(value.name), "_%d", codegen->nextLocalID++);
	return value;
}

static Value declareLocalValue(Codegen* codegen, Type* type, CodeBuffer* buffer)
{
	Value value = createLocalValue(codegen, type);

	emitIndentation(codegen, buffer);
	if (type->typeKind == TYPE_POINTER)
	{
		emitType(codegen, type, buffer);
		emitString(buffer, " const ");
	}
	else
	{
		emitString(buffer, "const ");
		emitType(codegen, type, buffer);
		emitChar(buffer, ' ');
	}

	emitValue(buffer, value);
	emitChar(buffer, '=');

	return value;
}

static void declareAnonymousStructType(Codegen* codegen, Type* type)
{
	if (codegen->declaredTypes.contains(type))
		return;

	codegen->declaredTypes.add(type);

	for (int i = 0; i < type->struct_.numFields; i++)
	{
		declareType(codegen, type->struct_.fieldTypes[i]);
	}

	CodeBuffer* buffer = &codegen->typesBuffer;

	int lastIndentation = codegen->indentation;
	codegen->indentation = 0;

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

	codegen->indentation = lastIndentation;
}

static void declareNamedStructStub(Codegen* codegen, Struct* struct_)
{
	if (codegen->declaredTypeStubs.contains(struct_->structType))
		return;

	codegen->declaredTypeStubs.add(struct_->structType);

	CodeBuffer* buffer = &codegen->typesBuffer;

	emitString(buffer, "struct ");
	emitString(buffer, struct_->name);
	emitString(buffer, ";\n");
}

static void declareNamedStruct(Codegen* codegen, Struct* struct_)
{
	if (codegen->declaredTypes.contains(struct_->structType))
		return;

	codegen->declaredTypes.add(struct_->structType);

	for (int i = 0; i < struct_->numFields; i++)
	{
		declareType(codegen, struct_->fields[i]->variableType->inferredType);
	}

	CodeBuffer* buffer = &codegen->typesBuffer;

	int lastIndentation = codegen->indentation;
	codegen->indentation = 0;

	emitString(buffer, "struct ");
	emitString(buffer, struct_->name);
	emitString(buffer, "{\n");

	codegen->indentation++;
	for (int i = 0; i < struct_->numFields; i++)
	{
		Field* field = struct_->fields[i];
		for (int j = 0; j < field->numDeclarators; j++)
		{
			if (!field->declarators[j].hasOffset)
			{
				emitIndentation(codegen, buffer);
				emitType(codegen, field->variableType->inferredType, buffer);
				emitChar(buffer, ' ');
				emitString(buffer, field->declarators[j].name);
				emitString(buffer, ";\n");
			}
		}
	}
	codegen->indentation--;

	emitIndentation(codegen, buffer);
	emitString(buffer, "};\n");

	codegen->indentation = lastIndentation;
}

static void declareAnonymousUnionType(Codegen* codegen, Type* type)
{
	if (codegen->declaredTypes.contains(type))
		return;

	codegen->declaredTypes.add(type);

	for (int i = 0; i < type->union_.numFields; i++)
	{
		declareType(codegen, type->union_.fieldTypes[i]);
	}

	CodeBuffer* buffer = &codegen->typesBuffer;

	int lastIndentation = codegen->indentation;
	codegen->indentation = 0;

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

	codegen->indentation = lastIndentation;
}

static void declareNamedUnionStub(Codegen* codegen, Union* union_)
{
	if (codegen->declaredTypeStubs.contains(union_->unionType))
		return;

	codegen->declaredTypes.add(union_->unionType);

	CodeBuffer* buffer = &codegen->typesBuffer;

	emitString(buffer, "union ");
	emitString(buffer, union_->name);
	emitString(buffer, ";\n");
}

static void declareNamedUnion(Codegen* codegen, Union* union_)
{
	if (codegen->declaredTypes.contains(union_->unionType))
		return;

	codegen->declaredTypes.add(union_->unionType);

	for (int i = 0; i < union_->numFields; i++)
	{
		declareType(codegen, union_->fields[i]->variableType->inferredType);
	}

	CodeBuffer* buffer = &codegen->typesBuffer;

	int lastIndentation = codegen->indentation;
	codegen->indentation = 0;

	emitString(buffer, "union ");
	emitString(buffer, union_->name);
	emitString(buffer, "{\n");

	codegen->indentation++;
	for (int i = 0; i < union_->numFields; i++)
	{
		Field* field = union_->fields[i];
		for (int j = 0; j < field->numDeclarators; j++)
		{
			emitIndentation(codegen, buffer);
			emitType(codegen, field->variableType->inferredType, buffer);
			emitChar(buffer, ' ');
			emitString(buffer, field->declarators[j].name);
			emitString(buffer, ";\n");
		}
	}
	codegen->indentation--;

	emitIndentation(codegen, buffer);
	emitString(buffer, "};\n");

	codegen->indentation = lastIndentation;
}

static void declareEnum(Codegen* codegen, Enum* enum_)
{
	if (codegen->declaredTypes.contains(enum_->enumType))
		return;

	codegen->declaredTypes.add(enum_->enumType);

	CodeBuffer* buffer = &codegen->typesBuffer;

	int lastIndentation = codegen->indentation;
	codegen->indentation = 0;

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
		EnumValue* enumValue = enum_->values[i];
		emitString(buffer, enumValue->name);
		if (enumValue->value)
		{
			emitString(buffer, "=");

			int64_t intValue;
			constantFold(enumValue->value, &intValue);

			emitInteger(buffer, intValue);

			//Value value = emitExpression(codegen, enumValue->value, buffer);
			//emitValue(buffer, value);
		}
		emitString(buffer, ",\n");
	}
	codegen->indentation--;

	emitString(buffer, "};\n");

	codegen->indentation = lastIndentation;
}

static void declareTypedef(Codegen* codegen, Typedef* typedef_)
{
	if (codegen->declaredTypes.contains(typedef_->aliasType))
		return;

	codegen->declaredTypes.add(typedef_->aliasType);

	CodeBuffer* buffer = &codegen->typesBuffer;

	declareType(codegen, typedef_->value->inferredType);

	emitString(buffer, "typedef ");
	emitType(codegen, typedef_->value->inferredType, buffer);
	emitString(buffer, " ");
	emitString(buffer, typedef_->name);
	emitString(buffer, ";\n");
}

static void declareOptionalType(Codegen* codegen, Type* type)
{
	CodeBuffer* buffer = &codegen->typesBuffer;

	emitString(buffer, "typedef struct{");
	emitType(codegen, type->optional.elementType, buffer);
	emitString(buffer, " value;u8 flag;}");
	emitString(buffer, type->mangledName);
	emitString(buffer, ";\n");
}

static void declareFunctionType(Codegen* codegen, Type* type)
{
	CodeBuffer* buffer = &codegen->typesBuffer;

	emitString(buffer, "typedef ");
	emitType(codegen, type->function.returnType ? type->function.returnType : &codegen->types->primitiveTypes[TYPE_VOID], buffer);
	emitString(buffer, "(*");
	emitString(buffer, type->mangledName);
	emitString(buffer, ")(");
	for (int i = 0; i < type->function.numParams; i++)
	{
		emitType(codegen, type->function.paramTypes[i], buffer);
		if (i < type->function.numParams - 1)
			emitChar(buffer, ',');
	}
	emitString(buffer, ");\n");
}

static void declareArrayType(Codegen* codegen, Type* type)
{
	CodeBuffer* buffer = &codegen->typesBuffer;

	emitString(buffer, "typedef struct{");
	if (type->array.size)
	{
		//emitString(buffer, "const ");
		emitType(codegen, type->array.elementType, buffer);
		emitString(buffer, " data[");
		emitInteger(buffer, type->array.size);
		emitString(buffer, "];}");
	}
	else
	{
		//emitString(buffer, "const ");
		emitType(codegen, type->array.elementType, buffer);
		emitString(buffer, "* data;u64 length;}");
	}
	emitString(buffer, type->mangledName);
	emitString(buffer, ";\n");
}

static void declareTypeStub(Codegen* codegen, Type* type)
{
	if (codegen->declaredTypeStubs.contains(type) || codegen->declaredTypes.contains(type))
		return;

	if (type->typeKind == TYPE_STRUCT)
	{
		//codegen->declaredTypeStubs.add(type);

		if (type->struct_.name.length)
			declareNamedStructStub(codegen, type->struct_.declaration);
	}
	else if (type->typeKind == TYPE_UNION)
	{
		//codegen->declaredTypeStubs.add(type);

		if (type->union_.name.length)
			declareNamedUnionStub(codegen, type->union_.declaration);
	}
	else if (type->typeKind == TYPE_ENUM)
	{
	}
	else if (type->typeKind == TYPE_ALIAS)
	{
		declareTypedef(codegen, type->alias.declaration);
	}
}

static void declareType(Codegen* codegen, Type* type)
{
	if (codegen->declaredTypes.contains(type))
		return;

	if (type->typeKind == TYPE_STRUCT)
	{
		if (!type->struct_.name.length)
			declareAnonymousStructType(codegen, type);
		else if (type->struct_.declaration)
			declareNamedStruct(codegen, type->struct_.declaration);
	}
	else if (type->typeKind == TYPE_UNION)
	{
		if (!type->union_.name.length)
			declareAnonymousUnionType(codegen, type);
		else if (type->union_.declaration)
			declareNamedUnion(codegen, type->union_.declaration);
	}
	else if (type->typeKind == TYPE_ENUM)
	{
		declareEnum(codegen, type->enum_.declaration);
	}
	else if (type->typeKind == TYPE_ALIAS)
	{
		declareTypedef(codegen, type->alias.declaration);
	}
	else if (type->typeKind == TYPE_POINTER)
	{
		codegen->declaredTypes.add(type);

		declareTypeStub(codegen, type->pointer.elementType);

		//declarePointerType(codegen, type);
	}
	else if (type->typeKind == TYPE_OPTIONAL)
	{
		codegen->declaredTypes.add(type);

		declareType(codegen, type->optional.elementType);

		declareOptionalType(codegen, type);
	}
	else if (type->typeKind == TYPE_FUNCTION)
	{
		codegen->declaredTypes.add(type);

		if (type->function.returnType)
			declareTypeStub(codegen, type->function.returnType);
		for (int i = 0; i < type->function.numParams; i++)
		{
			declareTypeStub(codegen, type->function.paramTypes[i]);
		}

		declareFunctionType(codegen, type);
	}
	else if (type->typeKind == TYPE_ARRAY)
	{
		codegen->declaredTypes.add(type);

		declareType(codegen, type->array.elementType);

		declareArrayType(codegen, type);
	}
}

static void emitType(Codegen* codegen, Type* type, CodeBuffer* buffer)
{
	if (type->typeKind == TYPE_STRUCT)
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
	else
	{
		emitString(buffer, type->mangledName);
	}
}

static void declareGlobalVariable(Codegen* codegen, GlobalVariable* globalVariable, CodeBuffer* buffer)
{
	for (int i = 0; i < globalVariable->numDeclarators; i++)
	{
		VariableDeclarator* declarator = &globalVariable->declarators[i];
		emitString(buffer, "extern ");
		if (globalVariable->storage & STORAGE_CONSTANT)
			emitString(buffer, "const ");
		emitType(codegen, globalVariable->variableType->inferredType, buffer);
		emitChar(buffer, ' ');
		emitString(buffer, declarator->name);
		emitString(buffer, ";\n");
	}
}

static void declareFunction(Codegen* codegen, Function* function, CodeBuffer* buffer)
{
	if (function->functionType->function.returnType)
		declareType(codegen, function->functionType->function.returnType);
	for (int i = 0; i < function->functionType->function.numParams; i++)
		declareType(codegen, function->functionType->function.paramTypes[i]);

	if (function->storage & STORAGE_DLLIMPORT)
		emitString(buffer, "DLLIMPORT ");
	else if (function->storage & STORAGE_DLLEXPORT)
		emitString(buffer, "DLLEXPORT ");

	emitString(buffer, "extern ");

	emitType(codegen, function->functionType->function.returnType ? function->functionType->function.returnType : &codegen->types->primitiveTypes[TYPE_VOID], buffer);
	emitChar(buffer, ' ');
	emitString(buffer, function->mangledName);
	emitChar(buffer, '(');
	for (int i = 0; i < function->functionType->function.numParams; i++)
	{
		emitType(codegen, function->functionType->function.paramTypes[i], buffer);

		if (function->params[i]->name.length)
		{
			emitChar(buffer, ' ');
			emitString(buffer, function->params[i]->name);
		}

		if (i < function->functionType->function.numParams - 1)
			emitChar(buffer, ',');
	}
	emitString(buffer, ");\n");
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
	else if (op == OPERATOR_ASSIGN)
		emitChar(buffer, '=');
	else if (op == OPERATOR_ADD_ASSIGN)
		emitString(buffer, "+=");
	else if (op == OPERATOR_SUBTRACT_ASSIGN)
		emitString(buffer, "-=");
	else if (op == OPERATOR_MULTIPLY_ASSIGN)
		emitString(buffer, "*=");
	else if (op == OPERATOR_DIVIDE_ASSIGN)
		emitString(buffer, "/=");
	else if (op == OPERATOR_MODULO_ASSIGN)
		emitString(buffer, "%=");
	else if (op == OPERATOR_BITSHIFT_LEFT_ASSIGN)
		emitString(buffer, "<<=");
	else if (op == OPERATOR_BITSHIFT_RIGHT_ASSIGN)
		emitString(buffer, ">>=");
	else if (op == OPERATOR_BITWISE_AND_ASSIGN)
		emitString(buffer, "&=");
	else if (op == OPERATOR_BITWISE_XOR_ASSIGN)
		emitString(buffer, "^=");
	else if (op == OPERATOR_BITWISE_OR_ASSIGN)
		emitString(buffer, "|=");
	else if (op == OPERATOR_LOGICAL_AND_ASSIGN)
		emitString(buffer, "&&=");
	else if (op == OPERATOR_LOGICAL_OR_ASSIGN)
		emitString(buffer, "||=");
	else
	{
		SnekAssert(false);
	}
}

static void emitExpressionConstant(Codegen* codegen, Expression* expression, CodeBuffer* buffer);

static Value emitExpression(Codegen* codegen, Expression* expression, CodeBuffer* buffer)
{
	if (expression->type == NODE_INT_LITERAL)
	{
		IntLiteral* intLiteral = (IntLiteral*)expression;

		Value value = {};
		value.type = expression->inferredType;

		SnekAssert(isIntegerType(expression->inferredType));

		if (intLiteral->negative)
			strcat(value.name, "-");

		if (intLiteral->base == 2)
		{
			strcat(value.name, "0b");

			uint64_t tmp = intLiteral->intValue;
			while (tmp)
			{
				int digit = tmp % 2;
				size_t len = strlen(value.name);
				value.name[len] = '0' + digit;
				value.name[len + 1] = 0;
				tmp /= 2;
			}
		}
		else if (intLiteral->base == 8)
		{
			snprintf(value.name + strlen(value.name), sizeof(value.name) - strlen(value.name), "0%llo", intLiteral->intValue);
		}
		else if (intLiteral->base == 16)
		{
			snprintf(value.name + strlen(value.name), sizeof(value.name) - strlen(value.name), "0x%llx", intLiteral->intValue);
		}
		else
		{
			snprintf(value.name + strlen(value.name), sizeof(value.name) - strlen(value.name), "%llu", intLiteral->intValue);
		}

		if (expression->inferredType->typeKind == TYPE_INT64 || expression->inferredType->typeKind == TYPE_UINT64)
			strcat(value.name, "ll");
		if (isUnsignedType(expression->inferredType))
			strcat(value.name, "u");

		return value;
	}
	else if (expression->type == NODE_FLOAT_LITERAL)
	{
		FloatLiteral* floatLiteral = (FloatLiteral*)expression;

		Value value = {};
		value.type = expression->inferredType;
		value.isIdentifier = true;
		value.identifier = floatLiteral->value;
		return value;
	}
	else if (expression->type == NODE_STRING_LITERAL)
	{
		StringLiteral* stringLiteral = (StringLiteral*)expression;

		Value ptr = createGlobalValue(codegen, nullptr);

		emitString(&codegen->globalsBuffer, "static i8* const ");
		emitValue(&codegen->globalsBuffer, ptr);
		emitString(&codegen->globalsBuffer, "=\"");
		emitString(&codegen->globalsBuffer, stringLiteral->value);
		emitString(&codegen->globalsBuffer, "\";\n");

		if (expression->inferredType->typeKind == TYPE_STRING)
		{
			Value str = declareLocalValue(codegen, &codegen->types->primitiveTypes[TYPE_STRING], buffer);

			emitChar(buffer, '{');
			emitValue(buffer, ptr);
			emitChar(buffer, ',');
			emitInteger(buffer, stringLiteral->value.length);
			emitString(buffer, "};\n");

			return str;
		}
		else
		{
			return ptr;
		}
	}
	else if (expression->type == NODE_CHAR_LITERAL)
	{
		CharLiteral* charLiteral = (CharLiteral*)expression;
		Value value = {};
		value.type = expression->inferredType;
		snprintf(value.name, sizeof(value.name), "'%.*s'", charLiteral->value.length, charLiteral->value.ptr);
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
		if (expression->inferredType->typeKind == TYPE_POINTER)
		{
			Value value = {};
			value.type = expression->inferredType;
			strcat(value.name, "0");
			return value;
		}
		else
		{
			Value value = {};
			value.type = expression->inferredType;
			strcat(value.name, "{0}");
			return value;
		}
	}
	else if (expression->type == NODE_SIZEOF)
	{
		Sizeof* sizeof_ = (Sizeof*)expression;

		Value value = declareLocalValue(codegen, sizeof_->inferredType, buffer);
		emitString(buffer, "(u64)sizeof(");
		emitType(codegen, sizeof_->expressionType, buffer);
		emitString(buffer, ");\n");

		return value;
	}
	else if (expression->type == NODE_IDENTIFIER)
	{
		Identifier* identifier = (Identifier*)expression;

		Symbol* symbol = getIdentifierSymbol(identifier);
		if (symbol->type == SYMBOL_VARIABLE)
		{
			Node* declaration = symbol->declaration;
			if (declaration->type == NODE_GLOBAL_VARIABLE)
			{
				GlobalVariable* globalVariable = &declaration->globalVariable;
				bool isConstant = globalVariable->storage & STORAGE_CONSTANT;

				if (isConstant)
				{
					VariableDeclarator* declarator = getDeclarator(globalVariable, identifier->name);
					SnekAssert(declarator->value);

					return emitExpression(codegen, declarator->value, buffer);
				}
				else if (symbol->file != codegen->currentFile->handle)
				{
					if (!codegen->declaredVariables.contains(globalVariable))
					{
						declareGlobalVariable(codegen, globalVariable, &codegen->globalsBuffer);
						codegen->declaredVariables.add(globalVariable);
					}
				}
			}
			else if (declaration->type == NODE_VARIABLE_DECLARATION)
			{
				VariableDeclaration* variable = &declaration->variableDeclaration;
				bool isConstant = variable->storage & STORAGE_CONSTANT;

				if (isConstant)
				{
					VariableDeclarator* declarator = getDeclarator(variable, identifier->name);
					SnekAssert(declarator->value);

					return emitExpression(codegen, declarator->value, buffer);
				}
			}
		}
		else if (symbol->type == SYMBOL_TYPE)
		{
		}
		else if (symbol->type == SYMBOL_FUNCTION_SET)
		{
			SnekAssert(identifier->functionOverloadID != -1);
			Function* function = symbol->functionSet.overloads[identifier->functionOverloadID].declaration;

			if (symbol->file != codegen->currentFile->handle)
			{
				if (!codegen->declaredFunctions.contains(function))
				{
					declareFunction(codegen, function, &codegen->prototypesBuffer);
					codegen->declaredFunctions.add(function);
				}
			}

			Value value = {};
			value.type = expression->inferredType;
			value.isIdentifier = true;
			value.identifier = function->mangledName;

			return value;
		}
		else if (symbol->type == SYMBOL_MACRO)
		{
		}

		Value value = {};
		value.type = expression->inferredType;
		value.lvalue = true;
		value.isIdentifier = true;
		value.identifier = identifier->name;

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

		declareType(codegen, expressionList->inferredType);

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
	else if (expression->type == NODE_ARRAY_INITIALIZER)
	{
		ArrayInitializer* arrayInitializer = (ArrayInitializer*)expression;

		SnekAssert(arrayInitializer->inferredType->typeKind == TYPE_ARRAY);

		if (arrayInitializer->inferredType->array.size == 0)
		{
			declareType(codegen, arrayInitializer->inferredType);

			Value ptr = createGlobalValue(codegen, nullptr);

			emitString(&codegen->globalsBuffer, "static ");
			emitType(codegen, arrayInitializer->inferredType->array.elementType, &codegen->globalsBuffer);
			emitString(&codegen->globalsBuffer, "* const ");
			emitValue(&codegen->globalsBuffer, ptr);
			emitString(&codegen->globalsBuffer, "={");
			for (int i = 0; i < arrayInitializer->numValues; i++)
			{
				emitExpressionConstant(codegen, arrayInitializer->values[i], &codegen->globalsBuffer);
				if (i < arrayInitializer->numValues - 1)
					emitChar(buffer, ',');
			}
			emitString(&codegen->globalsBuffer, "};\n");

			Value arr = declareLocalValue(codegen, arrayInitializer->inferredType, buffer);

			emitChar(buffer, '(');
			emitType(codegen, arrayInitializer->inferredType, buffer);
			emitString(buffer, "){");
			emitValue(buffer, ptr);
			emitChar(buffer, ',');
			emitInteger(buffer, arrayInitializer->numValues);
			emitString(buffer, "};\n");

			return arr;
		}
		else
		{
			int mark = codegen->scratch.mark();
			for (int i = 0; i < arrayInitializer->numValues; i++)
			{
				Value value = emitExpression(codegen, arrayInitializer->values[i], buffer);
				codegen->scratch.add(value);
			}

			Value* values = codegen->scratch.getData<Value>(mark);

			declareType(codegen, arrayInitializer->inferredType);

			Value array = declareLocalValue(codegen, arrayInitializer->inferredType, buffer);
			emitChar(buffer, '{');

			for (int i = 0; i < arrayInitializer->numValues; i++)
			{
				Value value = values[i];
				emitValue(buffer, value);
				if (i < arrayInitializer->numValues - 1)
					emitChar(buffer, ',');
			}

			emitString(buffer, "};\n");

			codegen->scratch.release(mark);

			return array;
		}
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
			if (!operand.isIdentifier)
			{
				Value result = {};
				result.type = unaryOperator->inferredType;
				result.lvalue = true;
				snprintf(result.name, sizeof(result.name), "(*%s)", operand.name);

				return result;
			}
			else if (operand.identifier.length <= 28)
			{
				Value result = {};
				result.type = unaryOperator->inferredType;
				result.lvalue = true;
				snprintf(result.name, sizeof(result.name), "(*%.*s)", operand.identifier.length, operand.identifier.ptr);

				return result;
			}
			else
			{
				// make sure the name fits by copying it to a new variable

				Value result = declareLocalValue(codegen, getPointerType(codegen->types, unaryOperator->inferredType, codegen->currentFile), buffer);
				emitChar(buffer, '&');
				emitValue(buffer, operand);
				emitString(buffer, ";\n");

				Value deref = {};
				deref.type = unaryOperator->inferredType;
				deref.lvalue = true;
				snprintf(deref.name, sizeof(deref.name), "(*%s)", deref.name);

				return deref;
			}
		}
		else if (unaryOperator->op == OPERATOR_ADDRESS)
		{
			Value result = declareLocalValue(codegen, unaryOperator->inferredType, buffer);
			emitChar(buffer, '&');
			emitValue(buffer, operand);
			emitString(buffer, ";\n");

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

		bool memberFunction = functionCall->expression->type == NODE_MEMBER_ACCESS;
		Value memberFunctionInstance = ((MemberAccess*)functionCall->expression)->memberFunctionInstance;

		int mark = codegen->scratch.mark();
		for (int i = 0; i < functionCall->numArgs; i++)
		{
			Value arg = emitExpression(codegen, functionCall->args[i], buffer);
			codegen->scratch.add(arg);
		}

		Value* args = codegen->scratch.getData<Value>(mark);

		Type* functionType = functionCall->expression->inferredType;

		Value variadicArgs = {};
		if (functionType->function.variadic)
		{
			int numVariadicArgs = functionCall->numArgs - functionType->function.numParams + 1;

			SnekAssert(functionType->function.paramTypes[functionType->function.numParams - 1]->typeKind == TYPE_ARRAY);
			Type* variadicType = functionType->function.paramTypes[functionType->function.numParams - 1]->array.elementType;

			Value argsData = createLocalValue(codegen, nullptr);

			emitIndentation(codegen, buffer);
			//emitString(buffer, "const "); statically sized array is already constant by default
			emitType(codegen, variadicType, buffer);
			emitChar(buffer, ' ');
			emitValue(buffer, argsData);
			emitChar(buffer, '[');
			emitInteger(buffer, numVariadicArgs);
			emitString(buffer, "]={");

			for (int i = 0; i < numVariadicArgs; i++)
			{
				Value variadicArg = args[functionType->function.numParams - 1 + i];
				emitValue(buffer, variadicArg);
				if (i < numVariadicArgs - 1)
					emitChar(buffer, ',');
			}

			emitString(buffer, "};\n");

			variadicArgs = declareLocalValue(codegen, getArrayType(codegen->types, variadicType, 0, codegen->currentFile), buffer);
			emitChar(buffer, '{');
			emitString(buffer, argsData.name);
			emitChar(buffer, ',');
			emitInteger(buffer, numVariadicArgs);
			emitString(buffer, "};\n");
		}

		Value result = {};
		if (!isVoidType(functionCall->inferredType))
			result = declareLocalValue(codegen, functionCall->inferredType, buffer);
		else
			emitIndentation(codegen, buffer);

		emitValue(buffer, operand);
		emitChar(buffer, '(');

		if (memberFunction)
		{
			emitValue(buffer, memberFunctionInstance);
			emitChar(buffer, ',');
		}

		for (int i = 0; i < functionCall->numArgs; i++)
		{
			Value arg = args[i];

			if (functionType->function.variadic && i == functionCall->numArgs - 1)
				arg = variadicArgs;

			emitValue(buffer, arg);

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
			snprintf(result.name, sizeof(result.name), "(*%s)", charPtr.name);

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
			snprintf(result.name, sizeof(result.name), "(*%s)", ptr.name);

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
			snprintf(result.name, sizeof(result.name), "(*%s)", ptr.name);

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

		if (Symbol* symbol = getMemberAccessSymbol(member))
		{
			SnekAssert(member->functionOverloadID != -1);

			member->memberFunctionInstance = operand;

			Function* function = symbol->functionSet.overloads[member->functionOverloadID].declaration;

			if (symbol->file != codegen->currentFile->handle)
			{
				if (!codegen->declaredFunctions.contains(function))
				{
					declareFunction(codegen, function, &codegen->prototypesBuffer);
					codegen->declaredFunctions.add(function);
				}
			}

			Value value = {};
			value.type = expression->inferredType;
			value.isIdentifier = true;
			value.identifier = function->mangledName;

			return value;
		}

		bool pointer = operandType->typeKind == TYPE_POINTER;
		if (pointer)
			operandType = operandType->pointer.elementType;

		if (operandType->typeKind == TYPE_STRUCT)
		{
			int offset = operandType->struct_.fieldOffsets[member->index];
			if (offset == -1)
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
				snprintf(result.name, sizeof(result.name), "(*%s)", ptr.name);

				return result;
			}
			else
			{
				Type* ptrType = getPointerType(codegen->types, member->inferredType, codegen->currentFile);
				Value ptr = declareLocalValue(codegen, ptrType, buffer);

				emitChar(buffer, '(');
				emitType(codegen, ptrType, buffer);
				emitString(buffer, ")((i8*)&");
				emitValue(buffer, operand);
				emitChar(buffer, '+');
				emitInteger(buffer, offset);
				emitString(buffer, ");\n");

				Value result = {};
				result.type = member->inferredType;
				result.lvalue = true;
				snprintf(result.name, sizeof(result.name), "(*%s)", ptr.name);

				return result;
			}
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
			snprintf(result.name, sizeof(result.name), "(*%s)", ptr.name);

			return result;
		}
		else if (operandType->typeKind == TYPE_STRING)
		{
			if (member->index == 0)
			{
				Value data = declareLocalValue(codegen, member->inferredType, buffer);
				emitValue(buffer, operand);
				emitString(buffer, ".data;\n");

				return data;
			}
			else if (member->index == 1)
			{
				Value length = declareLocalValue(codegen, member->inferredType, buffer);
				emitValue(buffer, operand);
				emitString(buffer, ".length;\n");

				return length;
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
				Value data = declareLocalValue(codegen, member->inferredType, buffer);
				emitValue(buffer, operand);
				emitString(buffer, ".data;\n");

				return data;
			}
			else if (member->index == 1)
			{
				if (operandType->array.size)
				{
					Value value = {};
					value.type = member->inferredType;
					snprintf(value.name, sizeof(value.name), "%llu", operandType->array.size);

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
				emitString(buffer, ".ptr;\n");

				return type;
			}
			else if (member->index == 1)
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
			Symbol* symbol = getIdentifierSymbol(typeName);
			SnekAssert(symbol);

			if (symbol->declaration->type == NODE_ENUM)
			{
				Enum* enum_ = &symbol->declaration->enum_;

				EnumValue* enumValue = enum_->values[member->index];

				Value value = {};
				value.type = member->inferredType;
				snprintf(value.name, sizeof(value.name), "%lld", enumValue->intValue);

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

		if (cast->targetType)
			declareType(codegen, cast->targetType->inferredType);

		Value expression = emitExpression(codegen, cast->expression, buffer);

		Type* expressionType = cast->expression->inferredType;
		Type* targetType = cast->inferredType;

		if (targetType->typeKind == TYPE_STRING)
		{
			if (cast->expression2)
			{
				Value expression2 = emitExpression(codegen, cast->expression2, buffer);

				Value str = declareLocalValue(codegen, getStringType(codegen->types), buffer);

				emitChar(buffer, '{');
				emitValue(buffer, expression);
				emitChar(buffer, ',');
				emitValue(buffer, expression2);
				emitString(buffer, "};\n");

				return str;
			}
			else if (isCharPointerType(expressionType))
			{
				Value str = declareLocalValue(codegen, getStringType(codegen->types), buffer);

				emitChar(buffer, '{');
				emitValue(buffer, expression);
				emitString(buffer, ",__cstrl(");
				emitValue(buffer, expression);
				emitString(buffer, ")};\n");

				return str;
			}
		}
		else if (targetType->typeKind == TYPE_ANY)
		{
			if (isIntegerType(expressionType))
			{
				Value result = declareLocalValue(codegen, targetType, buffer);

				emitString(buffer, "{.int_=");
				if (expressionType->typeKind != TYPE_INT64)
					emitString(buffer, "(i64)");
				emitValue(buffer, expression);
				emitString(buffer, ",.type=");
				emitInteger(buffer, (int64_t)TYPE_INT_START);
				emitString(buffer, "};\n");

				return result;
			}
			else if (isFloatingPointType(expressionType))
			{
				Value result = declareLocalValue(codegen, targetType, buffer);

				emitString(buffer, "{.float_=");
				if (expressionType->typeKind != TYPE_DOUBLE)
					emitString(buffer, "(double)");
				emitValue(buffer, expression);
				emitString(buffer, ",.type=");
				emitInteger(buffer, (int64_t)TYPE_FLOAT_START);
				emitString(buffer, "};\n");

				return result;
			}
			else if (expressionType->typeKind == TYPE_BOOL)
			{
				Value result = declareLocalValue(codegen, targetType, buffer);

				emitString(buffer, "{.bool_=");
				emitValue(buffer, expression);
				emitString(buffer, ",.type=");
				emitInteger(buffer, (int64_t)TYPE_BOOL);
				emitString(buffer, "};\n");

				return result;
			}
			else
			{
				Value result = declareLocalValue(codegen, targetType, buffer);

				emitString(buffer, "{.ptr=&");
				emitValue(buffer, expression);
				emitString(buffer, ",.type=");
				emitInteger(buffer, (int)expressionType->typeKind);
				emitString(buffer, "};\n");

				return result;
			}
		}

		if (expressionType->typeKind == TYPE_ANY)
		{
			if (isIntegerType(targetType))
			{
				Value result = declareLocalValue(codegen, targetType, buffer);

				if (targetType->typeKind != TYPE_INT64)
				{
					emitChar(buffer, '(');
					emitType(codegen, targetType, buffer);
					emitChar(buffer, ')');
				}
				emitValue(buffer, expression);
				emitString(buffer, ".int_;\n");

				return result;
			}
			else if (isFloatingPointType(targetType))
			{
				Value result = declareLocalValue(codegen, targetType, buffer);

				if (targetType->typeKind != TYPE_DOUBLE)
				{
					emitChar(buffer, '(');
					emitType(codegen, targetType, buffer);
					emitChar(buffer, ')');
				}
				emitValue(buffer, expression);
				emitString(buffer, ".float_;\n");

				return result;
			}
			else if (targetType->typeKind == TYPE_BOOL)
			{
				Value result = declareLocalValue(codegen, targetType, buffer);
				emitValue(buffer, expression);
				emitString(buffer, ".bool_;\n");
				return result;
			}
			else if (targetType->typeKind == TYPE_ANY
				|| targetType->typeKind == TYPE_STRING
				|| targetType->typeKind == TYPE_STRUCT
				|| targetType->typeKind == TYPE_UNION
				|| targetType->typeKind == TYPE_ENUM
				|| targetType->typeKind == TYPE_ALIAS
				|| targetType->typeKind == TYPE_POINTER
				|| targetType->typeKind == TYPE_OPTIONAL
				|| targetType->typeKind == TYPE_FUNCTION
				|| targetType->typeKind == TYPE_ARRAY)
			{
				Value ptr = declareLocalValue(codegen, getPointerType(codegen->types, targetType, codegen->currentFile), buffer);

				emitChar(buffer, '(');
				emitType(codegen, targetType, buffer);
				emitString(buffer, "*)");
				emitValue(buffer, expression);
				emitString(buffer, ".ptr;\n");

				Value result = {};
				result.type = targetType;
				snprintf(result.name, sizeof(result.name), "(*%s)", ptr.name);

				return result;
			}
			else
			{
				Value result = declareLocalValue(codegen, targetType, buffer);

				emitChar(buffer, '(');
				emitType(codegen, targetType, buffer);
				emitChar(buffer, ')');
				emitValue(buffer, expression);
				emitString(buffer, ";\n");

				return result;
			}
		}

		Value result = declareLocalValue(codegen, targetType, buffer);

		emitChar(buffer, '(');
		emitType(codegen, targetType, buffer);
		emitChar(buffer, ')');
		emitValue(buffer, expression);
		emitString(buffer, ";\n");

		return result;
	}

	SnekAssert(false);
	return {};
}

static void emitExpressionConstant(Codegen* codegen, Expression* expression, CodeBuffer* buffer)
{
	if (expression->type == NODE_INT_LITERAL ||
		expression->type == NODE_FLOAT_LITERAL ||
		expression->type == NODE_STRING_LITERAL ||
		expression->type == NODE_CHAR_LITERAL ||
		expression->type == NODE_TRUE ||
		expression->type == NODE_FALSE ||
		expression->type == NODE_NULL_LITERAL)
	{
		Value value = emitExpression(codegen, expression, buffer);
		emitValue(buffer, value);
		return;
	}
	else if (expression->type == NODE_SIZEOF)
	{
		Sizeof* sizeof_ = (Sizeof*)expression;

		declareType(codegen, sizeof_->expressionType);

		emitString(buffer, "(u64)sizeof(");
		emitType(codegen, sizeof_->expressionType, buffer);
		emitString(buffer, ")");

		return;
	}
	else if (expression->type == NODE_IDENTIFIER)
	{
		Identifier* identifier = (Identifier*)expression;

		Symbol* symbol = getIdentifierSymbol(identifier);
		if (symbol->type == SYMBOL_VARIABLE)
		{
			Node* declaration = symbol->declaration;
			if (declaration->type == NODE_GLOBAL_VARIABLE)
			{
				GlobalVariable* globalVariable = &declaration->globalVariable;
				bool isConstant = globalVariable->storage & STORAGE_CONSTANT;

				if (isConstant)
				{
					VariableDeclarator* declarator = getDeclarator(globalVariable, identifier->name);
					SnekAssert(declarator->value);

					return emitExpressionConstant(codegen, declarator->value, buffer);
				}
				else
				{
					SnekAssert(false);
				}
			}
			else if (declaration->type == NODE_VARIABLE_DECLARATION)
			{
				VariableDeclaration* variable = &declaration->variableDeclaration;
				bool isConstant = variable->storage & STORAGE_CONSTANT;

				if (isConstant)
				{
					VariableDeclarator* declarator = getDeclarator(variable, identifier->name);
					SnekAssert(declarator->value);

					return emitExpressionConstant(codegen, declarator->value, buffer);
				}
				else
				{
					SnekAssert(false);
				}
			}
		}
		else if (symbol->type == SYMBOL_TYPE)
		{
		}
		else if (symbol->type == SYMBOL_FUNCTION_SET)
		{
			SnekAssert(identifier->functionOverloadID != -1);
			Function* function = symbol->functionSet.overloads[identifier->functionOverloadID].declaration;

			if (symbol->file != codegen->currentFile->handle)
			{
				if (!codegen->declaredFunctions.contains(function))
				{
					declareFunction(codegen, function, &codegen->prototypesBuffer);
					codegen->declaredFunctions.add(function);
				}
			}

			return emitString(buffer, function->mangledName);
		}
		else if (symbol->type == SYMBOL_MACRO)
		{
		}

		emitString(buffer, identifier->name);
		return;
	}
	else if (expression->type == NODE_COMPOUND_EXPRESSION)
	{
		CompoundExpression* compound = (CompoundExpression*)expression;
		emitExpressionConstant(codegen, compound->value, buffer);
		return;
	}
	else if (expression->type == NODE_EXPRESSION_LIST)
	{
		ExpressionList* expressionList = (ExpressionList*)expression;

		declareType(codegen, expressionList->inferredType);

		emitChar(buffer, '(');
		emitType(codegen, expressionList->inferredType, buffer);
		emitString(buffer, "){");

		for (int i = 0; i < expressionList->numValues; i++)
		{
			emitExpressionConstant(codegen, expressionList->values[i], buffer);
			if (i < expressionList->numValues - 1)
				emitChar(buffer, ',');
		}

		emitChar(buffer, '}');
		return;
	}
	else if (expression->type == NODE_ARRAY_INITIALIZER)
	{
		ArrayInitializer* arrayInitializer = (ArrayInitializer*)expression;

		SnekAssert(arrayInitializer->inferredType->typeKind == TYPE_ARRAY);

		if (arrayInitializer->inferredType->array.size == 0)
		{
			declareType(codegen, arrayInitializer->inferredType);

			emitString(buffer, "{(const ");
			emitType(codegen, arrayInitializer->inferredType->array.elementType, buffer);
			emitString(buffer, "[]){");

			for (int i = 0; i < arrayInitializer->numValues; i++)
			{
				emitExpressionConstant(codegen, arrayInitializer->values[i], buffer);
				if (i < arrayInitializer->numValues - 1)
					emitChar(buffer, ',');
			}

			emitString(buffer, "}, ");
			emitInteger(buffer, arrayInitializer->numValues);
			emitChar(buffer, '}');
			return;
		}
		else
		{
			declareType(codegen, arrayInitializer->inferredType);

			emitChar(buffer, '(');
			emitType(codegen, arrayInitializer->inferredType, buffer);
			emitString(buffer, "){");

			for (int i = 0; i < arrayInitializer->numValues; i++)
			{
				emitExpressionConstant(codegen, arrayInitializer->values[i], buffer);
				if (i < arrayInitializer->numValues - 1)
					emitChar(buffer, ',');
			}

			emitChar(buffer, '}');
			return;
		}
	}
	else if (expression->type == NODE_BINARY_OPERATOR)
	{
		BinaryOperator* binaryOperator = (BinaryOperator*)expression;

		emitChar(buffer, '(');
		emitExpressionConstant(codegen, binaryOperator->left, buffer);
		emitOperator(buffer, binaryOperator->op);
		emitExpressionConstant(codegen, binaryOperator->right, buffer);
		emitChar(buffer, ')');
		return;
	}
	else if (expression->type == NODE_UNARY_OPERATOR)
	{
		UnaryOperator* unaryOperator = (UnaryOperator*)expression;

		if (unaryOperator->op == OPERATOR_PLUS_PREFIX)
		{
			emitExpressionConstant(codegen, unaryOperator->operand, buffer);
			return;
		}
		else if (unaryOperator->op == OPERATOR_MINUS_PREFIX)
		{
			emitChar(buffer, '-');
			emitExpressionConstant(codegen, unaryOperator->operand, buffer);
			return;
		}
		else if (unaryOperator->op == OPERATOR_LOGICAL_NOT)
		{
			emitChar(buffer, '!');
			emitExpressionConstant(codegen, unaryOperator->operand, buffer);
			return;
		}
		else if (unaryOperator->op == OPERATOR_BITWISE_NOT)
		{
			emitChar(buffer, '~');
			emitExpressionConstant(codegen, unaryOperator->operand, buffer);
			return;
		}
		else
		{
			SnekAssert(false);
		}
	}
	else if (expression->type == NODE_MEMBER_ACCESS)
	{
		MemberAccess* member = (MemberAccess*)expression;

		Type* operandType = member->operand->inferredType;

		if (operandType->typeKind == TYPE_TYPE)
		{
			SnekAssert(member->operand->type == NODE_IDENTIFIER);

			Identifier* typeName = (Identifier*)member->operand;
			Symbol* symbol = getIdentifierSymbol(typeName);
			SnekAssert(symbol);

			if (symbol->declaration->type == NODE_ENUM)
			{
				Enum* enum_ = &symbol->declaration->enum_;

				EnumValue* enumValue = enum_->values[member->index];

				emitInteger(buffer, enumValue->intValue);
				return;
			}
			else
			{
				SnekAssert(false);
			}
		}
		else
		{
			SnekAssert(false);
		}
	}
	else if (expression->type == NODE_TERNARY_CONDITION)
	{
		TernaryCondition* ternary = (TernaryCondition*)expression;

		int64_t conditionValue;
		if (constantFold(ternary->condition, &conditionValue))
		{
			if (conditionValue)
				emitExpressionConstant(codegen, ternary->then, buffer);
			else
				emitExpressionConstant(codegen, ternary->else_, buffer);
			return;
		}
		else
		{
			SnekAssert(false);
		}
	}
	else if (expression->type == NODE_CAST)
	{
		Cast* cast = (Cast*)expression;

		if (cast->targetType)
			declareType(codegen, cast->targetType->inferredType);

		Type* expressionType = cast->expression->inferredType;
		Type* targetType = cast->inferredType;

		if (targetType->typeKind == TYPE_STRING)
		{
			if (cast->expression2)
			{
				emitChar(buffer, '{');
				emitExpressionConstant(codegen, cast->expression, buffer);
				emitChar(buffer, ',');
				emitExpressionConstant(codegen, cast->expression2, buffer);
				emitChar(buffer, '}');
				return;
			}
			else if (isCharPointerType(expressionType))
			{
				emitChar(buffer, '{');
				emitExpressionConstant(codegen, cast->expression, buffer);
				emitChar(buffer, ',');
				if (cast->expression->type == NODE_STRING_LITERAL)
					emitInteger(buffer, ((StringLiteral*)cast->expression)->value.length);
				else
				{
					emitString(buffer, "__cstrl(");
					emitExpressionConstant(codegen, cast->expression, buffer);
					emitChar(buffer, ')');
				}
				emitChar(buffer, '}');
				return;
			}
		}
	}

	SnekAssert(false);
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
		For* for_ = (For*)statement;

		if ((!for_->startValue || isConstant(for_->startValue)) && isConstant(for_->compareValue))
		{
			int64_t startValue = 0, compareValue;
			if (for_->startValue)
				constantFold(for_->startValue, &startValue);
			constantFold(for_->compareValue, &compareValue);

			StringView iteratorName = for_->iteratorName.length ? for_->iteratorName : CreateString("__it");

			emitIndentation(codegen, buffer);
			emitString(buffer, "for(int ");
			emitString(buffer, iteratorName);
			emitChar(buffer, '=');
			emitInteger(buffer, startValue);
			emitChar(buffer, ';');
			emitString(buffer, iteratorName);
			emitChar(buffer, compareValue > startValue ? '<' : '>');
			if (for_->equals)
				emitChar(buffer, '=');
			emitInteger(buffer, compareValue);
			emitChar(buffer, ';');
			emitString(buffer, iteratorName);
			emitString(buffer, compareValue > startValue ? "++" : "--");
		}
		else
		{
			Value startValue;
			if (for_->startValue)
				startValue = emitExpression(codegen, for_->startValue, buffer);
			else
			{
				startValue = {};
				strcat(startValue.name, "0");
			}

			Value compareValue = emitExpression(codegen, for_->compareValue, buffer);

			StringView iteratorName = for_->iteratorName.length ? for_->iteratorName : CreateString("__it");

			Value sign = declareLocalValue(codegen, &codegen->types->primitiveTypes[TYPE_INT32], buffer);

			emitValue(buffer, compareValue);
			emitString(buffer, ">=");
			emitValue(buffer, startValue);
			emitString(buffer, "?1:-1;\n");

			emitIndentation(codegen, buffer);
			emitString(buffer, "for(int ");
			emitString(buffer, iteratorName);
			emitChar(buffer, '=');
			emitValue(buffer, startValue);
			emitChar(buffer, ';');
			emitString(buffer, iteratorName);
			emitChar(buffer, '*');
			emitValue(buffer, sign);
			emitChar(buffer, '<');
			if (for_->equals)
				emitChar(buffer, '=');
			emitValue(buffer, compareValue);
			emitChar(buffer, '*');
			emitValue(buffer, sign);
			emitChar(buffer, ';');
			emitString(buffer, iteratorName);
			emitString(buffer, "+=");
			emitValue(buffer, sign);
		}

		emitString(buffer, "){\n");

		codegen->indentation++;
		emitStatement(codegen, for_->body, buffer);
		codegen->indentation--;

		emitIndentation(codegen, buffer);
		emitString(buffer, "}\n");
	}
	else if (statement->type == NODE_RETURN)
	{
		Return* return_ = (Return*)statement;

		Value value = {};
		if (return_->value)
			value = emitExpression(codegen, return_->value, buffer);

		emitIndentation(codegen, buffer);
		emitString(buffer, "return");

		if (return_->value)
		{
			emitChar(buffer, ' ');
			emitValue(buffer, value);
		}

		emitString(buffer, ";\n");
	}
	else if (statement->type == NODE_BREAK)
	{
		emitIndentation(codegen, buffer);
		emitString(buffer, "break;\n");
	}
	else if (statement->type == NODE_CONTINUE)
	{
		emitIndentation(codegen, buffer);
		emitString(buffer, "continue;\n");
	}
	else if (statement->type == NODE_DEFER)
	{
	}
	else if (statement->type == NODE_VARIABLE_DECLARATION)
	{
		VariableDeclaration* variable = (VariableDeclaration*)statement;

		if (variable->storage & STORAGE_CONSTANT)
			emitString(buffer, "const ");

		declareType(codegen, variable->variableType->inferredType);

		for (int i = 0; i < variable->numDeclarators; i++)
		{
			VariableDeclarator* declarator = &variable->declarators[i];

			Value value = {};
			if (declarator->value)
				value = emitExpression(codegen, declarator->value, buffer);

			emitIndentation(codegen, buffer);
			emitType(codegen, variable->variableType->inferredType, buffer);
			emitChar(buffer, ' ');
			emitString(buffer, declarator->name);

			if (declarator->value)
			{
				emitChar(buffer, '=');
				emitValue(buffer, value);
				emitString(buffer, ";\n");
			}
			else
			{
				emitString(buffer, "={0};\n");
			}
		}
	}
	else if (statement->type == NODE_ASSIGNMENT)
	{
		Assignment* assignment = (Assignment*)statement;

		Value expression = emitExpression(codegen, assignment->expression, buffer);
		Value value = emitExpression(codegen, assignment->value, buffer);

		emitIndentation(codegen, buffer);
		emitValue(buffer, expression);
		emitOperator(buffer, assignment->op);
		emitValue(buffer, value);
		emitString(buffer, ";\n");
	}
	else if (statement->type == NODE_EXPRESSION_STATEMENT)
	{
		ExpressionStatement* expression = (ExpressionStatement*)statement;
		emitExpression(codegen, expression->expression, buffer);
	}
}

static void emitFunctionDeclaration(Codegen* codegen, Function* function, CodeBuffer* buffer)
{
	if (function->storage & STORAGE_DLLIMPORT)
		emitString(buffer, "DLLIMPORT extern ");
	else if (function->storage & STORAGE_DLLEXPORT)
		emitString(buffer, "DLLEXPORT extern ");
	else if (function->storage & STORAGE_EXTERN)
		emitString(buffer, "extern ");
	else if (function->storage & STORAGE_PRIVATE)
		emitString(buffer, "static ");

	if (function->functionType->function.returnType)
		declareType(codegen, function->functionType->function.returnType);

	for (int i = 0; i < function->functionType->function.numParams; i++)
	{
		declareType(codegen, function->functionType->function.paramTypes[i]);
	}

	emitType(codegen, function->functionType->function.returnType ? function->functionType->function.returnType : &codegen->types->primitiveTypes[TYPE_VOID], buffer);
	emitString(buffer, " ");
	emitString(buffer, function->mangledName);
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

	if (function->storage & STORAGE_DLLIMPORT)
		emitString(buffer, "DLLIMPORT extern ");
	else if (function->storage & STORAGE_DLLEXPORT)
		emitString(buffer, "DLLEXPORT extern ");
	else if (function->storage & STORAGE_EXTERN)
		emitString(buffer, "extern ");
	else if (function->storage & STORAGE_PRIVATE)
		emitString(buffer, "static ");

	if (function->functionType->function.returnType)
		declareType(codegen, function->functionType->function.returnType);

	for (int i = 0; i < function->functionType->function.numParams; i++)
	{
		declareType(codegen, function->functionType->function.paramTypes[i]);
	}

	emitType(codegen, function->functionType->function.returnType ? function->functionType->function.returnType : &codegen->types->primitiveTypes[TYPE_VOID], buffer);

	emitString(buffer, " ");
	emitString(buffer, function->mangledName);
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
		Value value = emitExpression(codegen, function->value, buffer);

		codegen->indentation++;
		emitIndentation(codegen, buffer);
		emitString(buffer, "return ");
		emitValue(buffer, value);
		emitString(buffer, ";\n");
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
	declareType(codegen, globalVariable->variableType->inferredType);

	for (int i = 0; i < globalVariable->numDeclarators; i++)
	{
		VariableDeclarator* declarator = &globalVariable->declarators[i];

		if (globalVariable->storage & STORAGE_CONSTANT)
			emitString(buffer, "const ");

		emitType(codegen, globalVariable->variableType->inferredType, buffer);
		emitChar(buffer, ' ');
		emitString(buffer, declarator->name);

		if (declarator->value)
		{
			emitChar(buffer, '=');
			emitExpressionConstant(codegen, declarator->value, buffer);
		}

		emitString(buffer, ";\n");
	}
}

static Function* getMainFunction(AST* ast)
{
	for (int i = 0; i < ast->numFunctions; i++)
	{
		Function* function = ast->functions[i];
		if (compareString(function->name, "main"))
			return function;
	}
	return nullptr;
}

static void generateEntryPoint(Codegen* codegen, CodeBuffer* buffer, Function* mainFunction)
{
	emitString(buffer, "int main(int argc, char* argv[]){\n");

	codegen->indentation++;

	if (mainFunction->returnType)
	{
		if (mainFunction->numParams)
		{
			emitIndentation(codegen, buffer);
			emitString(buffer, "string* args=__alloca(sizeof(string)*argc);\n");
		}

		emitIndentation(codegen, buffer);
		emitString(buffer, "int result=");
		emitString(buffer, mainFunction->mangledName);

		if (mainFunction->numParams)
		{
			Type* paramsType = getArrayType(codegen->types, getStringType(codegen->types), 0, codegen->currentFile);

			emitString(buffer, "((");
			emitType(codegen, paramsType, buffer);
			emitString(buffer, "){args,argc});\n");
		}
		else
		{
			emitString(buffer, "();\n");
		}

		emitIndentation(codegen, buffer);
		emitString(buffer, "return result;\n");
	}
	else
	{
		if (mainFunction->numParams)
		{
			emitIndentation(codegen, buffer);
			emitString(buffer, "string argsData[argc];\n");

			emitIndentation(codegen, buffer);
			emitString(buffer, "for(int i=0;i<argc;i++)argsData[i]=(string){argv[i],__cstrl(argv[i])};\n");

			Type* paramsType = getArrayType(codegen->types, getStringType(codegen->types), 0, codegen->currentFile);
			emitIndentation(codegen, buffer);
			emitType(codegen, paramsType, buffer);
			emitString(buffer, " args={argsData,argc};\n");
		}

		emitIndentation(codegen, buffer);
		emitString(buffer, mainFunction->mangledName);

		if (mainFunction->numParams)
		{
			emitString(buffer, "(args);\n");
		}
		else
		{
			emitString(buffer, "();\n");
		}

		emitIndentation(codegen, buffer);
		emitString(buffer, "return 0;\n");
	}

	codegen->indentation--;

	emitString(buffer, "}");
}

bool emitFile(Codegen* codegen, File* f, const char* localPath, const char* out)
{
	codegen->declaredTypes.clear();
	codegen->declaredTypeStubs.clear();
	codegen->declaredVariables.clear();
	codegen->declaredFunctions.clear();

	codegen->currentFile = f;

	codegen->nextGlobalID = 1;
	codegen->nextLocalID = 1;

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
	strcat(prologue, "\n#include <snek.h>\n");

	fwrite(prologue, 1, strlen(prologue), file);

	for (int i = 0; i < ast->numDeclarations; i++)
	{
		Node* declaration = ast->declarations[i];
		if (declaration->type == NODE_STRUCT)
		{
			declareNamedStructStub(codegen, &declaration->struct_);
		}
		else if (declaration->type == NODE_UNION)
		{
			declareNamedUnionStub(codegen, &declaration->union_);
		}
	}

	for (int i = 0; i < ast->numDeclarations; i++)
	{
		Node* declaration = ast->declarations[i];
		if (declaration->type == NODE_STRUCT)
		{
			declareNamedStruct(codegen, &declaration->struct_);
		}
		else if (declaration->type == NODE_UNION)
		{
			declareNamedUnion(codegen, &declaration->union_);
		}
		else if (declaration->type == NODE_ENUM)
		{
			declareEnum(codegen, &declaration->enum_);
		}
		else if (declaration->type == NODE_TYPEDEF)
		{
			declareTypedef(codegen, &declaration->typedef_);
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

	if (Function* mainFunction = getMainFunction(ast))
		generateEntryPoint(codegen, &codegen->functionsBuffer, mainFunction);

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
