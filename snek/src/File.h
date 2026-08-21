#pragma once

#include "parser/Lexer.h"
#include "parser/Parser.h"
#include "parser/AST.h"
#include "parser/Diagnostics.h"
#include "typechecker/TypeChecker.h"
#include "typechecker/TypeSystem.h"
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
	char* localPath;

	Arena arena;
	ScratchBuffer scratch;
	Diagnostics diagnostics;

	char* src;
	int length;
	AST ast;
	List<int> lineOffsets;

	TypeTable typeTable;

	List<FileHandle> dependencies;
};


void initFile(File* file, const char* localPath, const char* src, int length, GlobalBlockPool* blockPool);
void destroyFile(File* file);

FileHandle getFileHandle(const char* localPath);
File* getFileFromHandle(FileHandle fileHandle);
bool isFileLoaded(FileHandle fileHandle);
