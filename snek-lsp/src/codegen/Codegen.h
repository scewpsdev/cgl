#pragma once

#include "CodeBuffer.h"

#include "utils/Arena.h"
#include "utils/ScratchBuffer.h"
#include "utils/HashSet.h"


struct File;
struct Type;
struct TypeSystem;

struct Codegen
{
	TypeSystem* types;

	Arena arena;
	ScratchBuffer scratch;

	CodeBuffer typesBuffer;
	CodeBuffer prototypesBuffer;
	CodeBuffer globalsBuffer;
	CodeBuffer functionsBuffer;

	File* currentFile;

	int indentation;
	int nextGlobalID;
	int nextLocalID;

	HashSet<Type*> declaredTypes;
};


void initCodegen(Codegen* codegen, TypeSystem* types, GlobalBlockPool* blockPool);
void destroyCodegen(Codegen* codegen);

bool emitFile(Codegen* codegen, File* f, const char* localPath, const char* out);
