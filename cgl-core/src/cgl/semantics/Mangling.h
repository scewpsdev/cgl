#pragma once


#define ENTRY_POINT_NAME "main"


struct Resolver;

namespace AST
{
	struct Function;
	struct GlobalVariable;
	struct VariableDeclarator;
	struct Struct;
}

char* MangleFunctionName(AST::Function* function);
char* MangleGlobalName(AST::GlobalVariable* variable, AST::VariableDeclarator* declarator);
char* MangleStructName(AST::Struct* str);