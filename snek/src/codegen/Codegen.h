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
	CodeBuffer constantBuffer;
	CodeBuffer globalsBuffer;
	CodeBuffer functionsBuffer;

	char* outPath;
	FILE* outFile;

	File* currentFile;
	bool debugInfo;
	bool hotreload;

	int indentation;
	int currentLine;
	int lastLine;
	int nextGlobalID;
	int nextLocalID;

	TypeSet declaredTypes;
	TypeSet declaredTypeStubs;
	HashSet<GlobalVariable*> declaredVariables;
	HashSet<Function*> declaredFunctions;
};


void initCodegen(Codegen* codegen, TypeSystem* types, Arena* globalArena, char* outPath);
void destroyCodegen(Codegen* codegen);

bool prepare(Codegen* codegen);
bool emitFile(Codegen* codegen, File* f, const char* localPath, const char* out, bool debugInfo, bool hotreload);
void output(Codegen* codegen);
