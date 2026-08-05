#include "Codegen.h"

#include "parser/AST.h"

#include <stdio.h>


void initCodegen(Codegen* codegen)
{
	initArena(&codegen->arena, 1024 * 1024);

	initCodeBuffer(&codegen->types, &codegen->arena, 1024);
	initCodeBuffer(&codegen->prototypes, &codegen->arena, 1024);
	initCodeBuffer(&codegen->globals, &codegen->arena, 1024);
	initCodeBuffer(&codegen->functions, &codegen->arena, 1024);

	codegen->indentation = 0;
}

void destroyCodegen(Codegen* codegen)
{
	destroyArena(&codegen->arena);
}

static void emitIndentation(Codegen* codegen, CodeBuffer* buffer)
{
	for (int i = 0; i < codegen->indentation; i++)
	{
		emitString(buffer, "\t");
	}
}

static void emitType(Codegen* codegen, Type* type, CodeBuffer* buffer)
{
	
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

	resetCodeBuffer(&codegen->types);
	resetCodeBuffer(&codegen->prototypes);
	resetCodeBuffer(&codegen->globals);
	resetCodeBuffer(&codegen->functions);

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
			emitStruct(codegen, &declaration->struct_, &codegen->types);
		}
		else if (declaration->type == NODE_UNION)
		{
			emitUnion(codegen, &declaration->union_, &codegen->types);
		}
		else if (declaration->type == NODE_ENUM)
		{
			emitEnum(codegen, &declaration->enum_, &codegen->types);
		}
		else if (declaration->type == NODE_TYPEDEF)
		{
			emitTypedef(codegen, &declaration->typedef_, &codegen->types);
		}
		else if (declaration->type == NODE_FUNCTION)
		{
			emitFunctionDeclaration(codegen, &declaration->function, &codegen->prototypes);
			emitFunction(codegen, &declaration->function, &codegen->functions);
		}
		else if (declaration->type == NODE_GLOBAL_VARIABLE)
		{
			emitGlobalVariable(codegen, &declaration->globalVariable, &codegen->globals);
		}
	}

	SnekAssert(codegen->indentation == 0);

	if (codegen->types.count)
	{
		fwrite("\n\n", 1, 2, file);
		fwrite(codegen->types.data, 1, codegen->types.count, file);
	}
	if (codegen->prototypes.count)
	{
		fwrite("\n\n", 1, 2, file);
		fwrite(codegen->prototypes.data, 1, codegen->prototypes.count, file);
	}
	if (codegen->globals.count)
	{
		fwrite("\n\n", 1, 2, file);
		fwrite(codegen->globals.data, 1, codegen->globals.count, file);
	}
	if (codegen->functions.count)
	{
		fwrite("\n\n", 1, 2, file);
		fwrite(codegen->functions.data, 1, codegen->functions.count, file);
	}

	fclose(file);

	return true;
}
