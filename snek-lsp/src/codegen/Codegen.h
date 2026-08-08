#pragma once

#include "CodeBuffer.h"

#include "utils/Arena.h"
#include "utils/HashSet.h"


struct AST;
struct Type;
struct TypeSystem;

struct Codegen
{
	TypeSystem* types;

	Arena arena;

	CodeBuffer typesBuffer;
	CodeBuffer prototypesBuffer;
	CodeBuffer globalsBuffer;
	CodeBuffer functionsBuffer;

	int indentation;

	HashSet<Type*> declaredTypes;
};


void initCodegen(Codegen* codegen, TypeSystem* types, GlobalBlockPool* blockPool);
void destroyCodegen(Codegen* codegen);

bool emitFile(Codegen* codegen, AST* ast, const char* localPath, const char* out);
