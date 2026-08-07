#pragma once

#include "utils/ScratchBuffer.h"


struct File;
struct Function;
struct Arena;
struct Lexer;
struct Diagnostics;
struct TypeSystem;
struct Scope;
struct SymbolEntry;
struct Identifier;

struct TypeChecker
{
	Arena* arena;
	ScratchBuffer* scratch;
	Lexer* lexer;
	Diagnostics* diagnostics;
	TypeSystem* types;

	File* currentFile;
	Scope* currentScope;
	Function* currentFunction;
	int loopDepth;
};


void initTypeChecker(TypeChecker* tc, Arena* arena, ScratchBuffer* scratch, Lexer* lexer, Diagnostics* diagnostics, TypeSystem* types);
void destroyTypeChecker(TypeChecker* tc);

void symbolCollection(TypeChecker* tc, File* file);
void symbolResolution(TypeChecker* tc, File* file);
void typeCheckFunction(TypeChecker* tc, Function* function, File* file);

SymbolEntry* getIdentifierSymbol(Identifier* identifier);
