#include "pch.h"
#include "CGLCompiler.h"

#include "parser/Parser.h"
#include "semantics/Resolver.h"


void CGLCompiler::init()
{
}

void CGLCompiler::terminate()
{
}

void CGLCompiler::addFile(const char* src)
{
	SourceFile sourceFile;
	sourceFile.source = src;
	sourceFiles.push_back(sourceFile);
}

bool CGLCompiler::compile()
{
	std::vector<AST::File*> asts;
	asts.reserve(sourceFiles.size());

	Parser parser;
	for (SourceFile& sourceFile : sourceFiles)
	{
		AST::File* ast = parser.run(sourceFile);
		asts.push_back(ast);
	}

	Resolver resolver;
	resolver.run(asts);

	return true;
}
