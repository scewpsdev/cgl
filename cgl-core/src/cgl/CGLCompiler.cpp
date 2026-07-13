#include "pch.h"
#include "CGLCompiler.h"

#include "parser/Parser.h"
#include "semantics/Resolver.h"

#include "cgl/Platform.h"

#include <thread>
#include <chrono>


void CGLCompiler::init(MessageCallback_t msgCallback)
{
	this->msgCallback = msgCallback;
}

void CGLCompiler::terminate()
{
	for (AST::File* file : asts)
		delete file;
	DestroyList(asts);

	for (SourceFile& sourceFile : sourceFiles)
	{
		if (sourceFile.source)
			delete sourceFile.source;
	}

	if (parser) delete parser;
	if (resolver) delete resolver;
}

void CGLCompiler::addFile(const char* filename, const char* name, const char* src)
{
	SourceFile sourceFile;
	sourceFile.filename = filename;
	sourceFile.name = name;
	sourceFile.source = src;
	sourceFiles.add(sourceFile);
}

void CGLCompiler::addLinkerFile(const char* filename, bool shorthandName)
{
	LinkerFile linkerFile;
	linkerFile.filename = filename;
	linkerFile.shorthandName = shorthandName;
	linkerFiles.add(linkerFile);
}

void CGLCompiler::addLinkerPath(const char* path)
{
	linkerPaths.add(path);
}

bool CGLCompiler::compile()
{
	asts.reserve(sourceFiles.size);

	bool success = true;

	uint64_t beforeParse = GetTimeNS();

	parser = new Parser(this);
	for (int i = 0; i < sourceFiles.size; i++)
	{
		// TODO multithreading
		SourceFile& sourceFile = sourceFiles[i];
		if (AST::File* ast = parser->run(sourceFile))
			asts.add(ast);
		else
			success = false;
		success = success && !parser->failed;
	}

	uint64_t afterParse = GetTimeNS();
	fprintf(stderr, "parse %.3fms\n", (afterParse - beforeParse) / 1e6f);

	resolver = new Resolver(this, asts);
	success = resolver->run() && success;

	uint64_t afterAnalyze = GetTimeNS();
	fprintf(stderr, "analyze %.3fms\n", (afterAnalyze - afterParse) / 1e6f);

	return success;
}

AST::File* CGLCompiler::getASTByName(const char* name)
{
	for (int i = 0; i < asts.size; i++)
	{
		if (strcmp(asts[i]->name, name) == 0)
			return asts[i];
	}
	return nullptr;
}
