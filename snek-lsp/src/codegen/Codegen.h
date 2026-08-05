#pragma once

#include "CodeBuffer.h"

#include "utils/Arena.h"


struct AST;

struct Codegen
{
	Arena arena;

	CodeBuffer types;
	CodeBuffer prototypes;
	CodeBuffer globals;
	CodeBuffer functions;

	int indentation;
};


void initCodegen(Codegen* codegen);
void destroyCodegen(Codegen* codegen);

bool emitFile(Codegen* codegen, AST* ast, const char* out);
