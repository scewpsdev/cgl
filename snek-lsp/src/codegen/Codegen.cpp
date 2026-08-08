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
	emitString(buffer, ";\n\n");

	codegen->declaredTypes.add(type);
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
	emitString(buffer, ";\n\n");

	codegen->declaredTypes.add(type);
}

static void declareOptionalType(Codegen* codegen, Type* type, CodeBuffer* buffer)
{
	emitString(buffer, "typedef struct{");
	emitString(buffer, type->optional.elementType->mangledName);
	emitString(buffer, " value;u8 flag;}");
	emitString(buffer, type->mangledName);
	emitString(buffer, ";\n\n");
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
	emitChar(buffer, ')');
	emitString(buffer, type->mangledName);
	emitString(buffer, ";\n\n");
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
	emitString(buffer, ";\n\n");
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
			if (!codegen->declaredTypes.contains(type))
				declareAnonymousStructType(codegen, type, &codegen->typesBuffer);

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
			if (!codegen->declaredTypes.contains(type))
				declareAnonymousUnionType(codegen, type, &codegen->typesBuffer);

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
		if (!codegen->declaredTypes.contains(type))
			declareOptionalType(codegen, type, &codegen->typesBuffer);

		emitString(buffer, type->mangledName);
	}
	else if (type->typeKind == TYPE_FUNCTION)
	{
		if (!codegen->declaredTypes.contains(type))
			declareFunctionType(codegen, type, &codegen->typesBuffer);

		emitString(buffer, type->mangledName);
	}
	else if (type->typeKind == TYPE_ARRAY)
	{
		if (!codegen->declaredTypes.contains(type))
			declareArrayType(codegen, type, &codegen->typesBuffer);

		emitString(buffer, type->mangledName);
	}
	else if (type->typeKind == TYPE_TYPE)
	{
		emitString(buffer, "type");
	}
}

static void emitField(Codegen* codegen, Field* field, CodeBuffer* buffer)
{
	for (int i = 0; i < field->numDeclarators; i++)
	{
		emitIndentation(codegen, buffer);
		emitType(codegen, field->variableType->inferredType, buffer);
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
	emitString(buffer, "struct ");
	emitString(buffer, struct_->name);
	emitString(buffer, " {\n");

	codegen->indentation++;
	for (int i = 0; i < struct_->numFields; i++)
	{
		Field* field = struct_->fields[i];
		emitField(codegen, field, buffer);
	}
	codegen->indentation--;
}

static void emitUnionDeclaration(Codegen* codegen, Union* union_, CodeBuffer* buffer)
{

}

static void emitUnion(Codegen* codegen, Union* union_, CodeBuffer* buffer)
{

}

static void emitEnum(Codegen* codegen, Enum* enum_, CodeBuffer* buffer)
{

}

static void emitTypedef(Codegen* codegen, Typedef* typedef_, CodeBuffer* buffer)
{

}

static void emitFunctionDeclaration(Codegen* codegen, Function* function, CodeBuffer* buffer)
{

}

static void emitFunction(Codegen* codegen, Function* function, CodeBuffer* buffer)
{

}

static void emitGlobalVariable(Codegen* codegen, GlobalVariable* globalVariable, CodeBuffer* buffer)
{

}

bool emitFile(Codegen* codegen, AST* ast, const char* localPath, const char* out)
{
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
