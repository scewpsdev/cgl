#pragma once

#include "Lexer.h"
#include "utils/ScratchBuffer.h"


struct AST;
struct Node;
struct Scope;
struct Arena;
struct Diagnostics;

struct Parser
{
	Lexer lexer;
	int cursor, lastTokenEnd;

	Token lookahead[3];
	int lookaheadState[3];
	int lookaheadCount;

	Scope* currentScope;

	Arena* arena;
	ScratchBuffer scratch;

	Diagnostics* diagnostics;
};


void initParser(Parser* parser, const char* filename, const char* src, int length, Arena* arena, Diagnostics* diagnostics);
void destroyParser(Parser* parser);

SourceLocation getSourceLocation(Parser* parser);
SourceLocation getSourceLocation(Parser* parser, Token token);
void getSourceLocation(Parser* parser, Token token, SourceLocation* start, SourceLocation* end);

void parse(Parser* parser, AST* ast, Arena* arena);
