#pragma once

#include "Lexer.h"
#include "utils/ScratchBuffer.h"


struct File;
struct AST;
struct Node;
struct Arena;
struct Diagnostics;

struct Parser
{
	File* file;

	Lexer lexer;
	int cursor, lastTokenEnd;

	Token lookahead[3];
	int lookaheadState[3];
	int lookaheadCount;

	Arena* arena;
	ScratchBuffer* scratch;

	Diagnostics* diagnostics;
};


void initParser(Parser* parser, File* file);

void error(Parser* parser, Node* node, const char* fmt, ...);

SourceLocation getSourceLocation(Parser* parser);
SourceLocation getSourceLocation(File* file, Token token);
void getSourceLocation(File* file, Token token, SourceLocation* start, SourceLocation* end);
void getSourceLocation(File* file, Node* node, SourceLocation* start, SourceLocation* end);
void getSourceLocation(File* file, StringView str, SourceLocation* start, SourceLocation* end);

StringView getTokenString(Token token, File* file);
StringView getRangedString(int start, int end, File* file);

void parse(Parser* parser, AST* ast);

void resolveDependencies(Parser* parser, File* file);
