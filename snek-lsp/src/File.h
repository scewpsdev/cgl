#pragma once

#include "parser/Lexer.h"
#include "parser/Parser.h"
#include "parser/AST.h"
#include "parser/Diagnostics.h"
#include "typechecker/TypeChecker.h"
#include "codegen/Codegen.h"
#include "utils/List.h"
#include "utils/Arena.h"
#include "utils/ScratchBuffer.h"

#include <stdint.h>


struct TypeSystem;
struct GlobalBlockPool;

struct InternedType
{
	uint64_t key;
	Type* type;
	InternedType* next;
};

struct File
{
	FileHandle handle;

	Arena arena;
	ScratchBuffer scratch;
	Diagnostics diagnostics;

	Parser parser;
	TypeChecker typeChecker;
	Codegen codegen;

	AST ast;

	List<FileHandle> dependencies;

	InternedType* internedTypes;
};


void initFile(File* file, const char* localPath, GlobalBlockPool* blockPool);

void addInternedType(File* file, uint64_t key, Type* type);
void clearInternedTypes(File* file, TypeSystem* types);

FileHandle getFileHandle(const char* localPath);
