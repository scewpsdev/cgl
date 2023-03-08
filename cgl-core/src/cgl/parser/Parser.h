#pragma once

#include "Lexer.h"

#include "cgl/utils/List.h"
#include "cgl/utils/Log.h"
#include "cgl/SourceFile.h"


namespace AST
{
	struct File;
}

class CGLCompiler;

class Parser
{
public:
	CGLCompiler* context;
	Lexer* lexer = nullptr;

	AST::File* module = nullptr;

	bool failed = false;


	Parser(CGLCompiler* context);
	~Parser();

	AST::File* run(SourceFile& sourceFile);

private:

};
