#pragma once

#include "Lexer.h"

#include "cgl/utils/List.h"
#include "cgl/utils/Log.h"
#include "cgl/SourceFile.h"


namespace AST
{
	struct File;
}

class Parser
{
public:
	Lexer* lexer;

	// TODO rename to "file"
	AST::File* module;

	bool failed;


	Parser();
	~Parser();

	AST::File* run(SourceFile& sourceFile);
};
