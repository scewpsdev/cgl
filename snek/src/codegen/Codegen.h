#pragma once

#include "CodeBuffer.h"

#include "utils/Arena.h"
#include "utils/ScratchBuffer.h"
#include "utils/HashSet.h"
#include "utils/TypeSet.h"


struct File;
struct Type;
struct TypeSystem;
struct Symbol;
struct GlobalVariable;
struct Function;

struct Codegen
{
	TypeSystem* types;

	Arena* arena;
	ScratchBuffer scratch;

	CodeBuffer typesBuffer;
	CodeBuffer prototypesBuffer;
	CodeBuffer globalsBuffer;
	CodeBuffer functionsBuffer;

	File* currentFile;

	int indentation;
	int nextGlobalID;
	int nextLocalID;

	TypeSet declaredTypes;
	TypeSet declaredTypeStubs;
	HashSet<GlobalVariable*> declaredVariables;
	HashSet<Function*> declaredFunctions;
};


void initCodegen(Codegen* codegen, TypeSystem* types, Arena* globalArena);
void destroyCodegen(Codegen* codegen);

bool emitFile(Codegen* codegen, File* f, const char* localPath, const char* out);
