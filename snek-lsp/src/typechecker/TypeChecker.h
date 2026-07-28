#pragma once


struct AST;

struct TypeChecker
{
};


void initTypeChecker(TypeChecker* tc);
void destroyTypeChecker(TypeChecker* tc);

void symbolCollection(TypeChecker* tc, AST* ast);
void symbolResolution(TypeChecker* tc, AST* ast);
void typeChecking(TypeChecker* tc, AST* ast);
