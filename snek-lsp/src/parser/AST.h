#pragma once

#include "utils/Arena.h"


enum NodeType
{
	NODE_NULL = 0,

	NODE_STRUCT,

	NODE_COUNT
};

struct NodeBase
{
	NodeType type;
};

struct Struct : NodeBase
{
	
};

struct Enum : NodeBase
{

};

struct Function : NodeBase
{

};

struct Node
{
	union {
		NodeBase node;
		Struct struct_;
		Enum enum_;
		Function function;
	};
};

struct AST
{
	Arena arena;

	Node** declarations;
	int numDeclarations;
};


void initAST(AST* ast);
void destroyAST(AST* ast);
