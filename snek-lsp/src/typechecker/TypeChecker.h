#pragma once


struct AST;
struct Function;
struct Arena;
struct Lexer;
struct Diagnostics;
struct TypeSystem;
struct Scope;

struct TypeChecker
{
	Arena* arena;
	Lexer* lexer;
	Diagnostics* diagnostics;

	TypeSystem* types;

	Scope* currentScope;
};


void initTypeChecker(TypeChecker* tc, Arena* arena, Lexer* lexer, Diagnostics* diagnostics, TypeSystem* types);
void destroyTypeChecker(TypeChecker* tc);

void symbolCollection(TypeChecker* tc, AST* ast);
void symbolResolution(TypeChecker* tc, AST* ast);
void typeCheckFunction(TypeChecker* tc, Function* function);
