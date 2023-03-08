#include "pch.h"
#include "CGLCompiler.h"

#include "parser/Parser.h"
#include "semantics/Resolver.h"


void CGLCompiler::init(MessageCallback_t msgCallback)
{
	this->msgCallback = msgCallback;
}

void CGLCompiler::terminate()
{
	for (AST::File* file : asts)
		delete file;
	DestroyList(asts);
}

void CGLCompiler::addFile(const char* filename, const char* name, const char* src)
{
	SourceFile sourceFile;
	sourceFile.filename = filename;
	sourceFile.name = name;
	sourceFile.source = src;
	sourceFiles.add(sourceFile);
}

bool CGLCompiler::compile()
{
	asts.reserve(sourceFiles.size);

	bool success = true;

	Parser parser(this);
	for (SourceFile& sourceFile : sourceFiles)
	{
		// TODO multithreading
		if (AST::File* ast = parser.run(sourceFile))
			asts.add(ast);
		else
			success = false;
	}

	Resolver resolver(this, asts);
	success = resolver.run() && success;

	return success;
}
