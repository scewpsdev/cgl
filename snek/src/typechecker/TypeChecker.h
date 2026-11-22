#pragma once

#include "utils/StringView.h"


struct File;
struct Function;
struct Arena;
struct ScratchBuffer;
struct Lexer;
struct Diagnostics;
struct TypeSystem;
struct Scope;
struct Symbol;
struct Expression;
struct Identifier;
struct Type;

struct TypeChecker
{
	Arena* arena;
	ScratchBuffer* scratch;
	Diagnostics* diagnostics;
	TypeSystem* types;

	Scope* globalScope;

	File* file;
	Scope* currentScope;
	Function* currentFunction;
	int loopDepth;
};


void initTypeChecker(TypeChecker* tc, Arena* arena, ScratchBuffer* scratch, Diagnostics* diagnostics, TypeSystem* types, Scope* globalScope);
void destroyTypeChecker(TypeChecker* tc);

void symbolCollection(TypeChecker* tc, File* file);
void symbolResolution(TypeChecker* tc, File* file);
void typeCheckFunctions(TypeChecker* tc, File* file);

Type* unwrapType(Type* type);
int getFieldIndex(TypeChecker* tc, Type* operandType, StringView name, Expression* operand);
