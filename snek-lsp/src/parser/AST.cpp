#include "AST.h"


void initAST(AST* ast)
{
	initArena(&ast->arena, 16 * 1024 * 1024);
}

void destroyAST(AST* ast)
{
	destroyArena(&ast->arena);
}
