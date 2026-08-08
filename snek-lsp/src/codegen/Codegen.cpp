#include "Codegen.h"

#include "parser/AST.h"
#include "typechecker/TypeSystem.h"

#include <stdio.h>


void initCodegen(Codegen* codegen, TypeSystem* types, GlobalBlockPool* blockPool)
{
	codegen->types = types;

	initArena(&codegen->arena, blockPool);

	initCodeBuffer(&codegen->typesBuffer, &codegen->arena, 1024);
	initCodeBuffer(&codegen->prototypesBuffer, &codegen->arena, 1024);
	initCodeBuffer(&codegen->globalsBuffer, &codegen->arena, 1024);
	initCodeBuffer(&codegen->functionsBuffer, &codegen->arena, 1024);

	codegen->indentation = 0;

	initHashSet(&codegen->declaredTypes);
}

void destroyCodegen(Codegen* codegen)
{
	destroyArena(&codegen->arena);
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
		emitString(buffer, "];u64 length;}");
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

static void emitExpression(Codegen* codegen, Expression* expression, CodeBuffer* buffer)
{

}

static void emitStatement(Codegen* codegen, Statement* statement, CodeBuffer* buffer)
{

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

bool emitFile(Codegen* codegen, AST* ast, const char* localPath, const char* out)
{
	codegen->declaredTypes.clear();

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
