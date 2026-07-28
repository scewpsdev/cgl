#pragma once

#include "Type.h"


struct AST;
struct Function;
struct Arena;
struct Lexer;
struct Diagnostics;
struct Scope;

struct TypeChecker
{
	Arena* arena;
	Lexer* lexer;
	Diagnostics* diagnostics;

	Type errorType;
	Type int8Type;
	Type int16Type;
	Type int32Type;
	Type int64Type;
	Type uint8Type;
	Type uint16Type;
	Type uint32Type;
	Type uint64Type;
	Type floatType;
	Type doubleType;
	Type boolType;
	Type anyType;
	Type stringType;

	Scope* currentScope;
};


void initTypeChecker(TypeChecker* tc, Arena* arena, Lexer* lexer, Diagnostics* diagnostics);
void destroyTypeChecker(TypeChecker* tc);

void symbolCollection(TypeChecker* tc, AST* ast);
void symbolResolution(TypeChecker* tc, AST* ast);
void typeCheckFunction(TypeChecker* tc, Function* function);
