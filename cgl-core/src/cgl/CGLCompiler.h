#pragma once

#include "pch.h"

#include "SourceFile.h"

#include "cgl/parser/Parser.h"
#include "cgl/semantics/Resolver.h"

#include "cgl/ast/File.h"
#include "cgl/utils/Log.h"
#include "cgl/utils/List.h"


class CGLCompiler
{
public:
	List<SourceFile> sourceFiles;
	List<LinkerFile> linkerFiles;
	List<const char*> linkerPaths;

	List<AST::File*> asts;
	MessageCallback_t msgCallback = nullptr;
	bool disableError = false;

	bool staticLibrary = false;
	bool sharedLibrary = false;
	bool printIR = false;
	bool debugInfo = false;
	int optimization = 0;
	bool runtimeStackTrace = false;

	Parser* parser;
	Resolver* resolver;


	void init(MessageCallback_t msgCallback);
	void terminate();

	void addFile(const char* filename, const char* name, const char* src);
	void addLinkerFile(const char* filename, bool shorthandName);
	void addLinkerPath(const char* path);
	bool compile();

	int runLLVM(int argc, char* argv[]);
	int outputLLVM(const char* path);

	int runTCC(int argc, char* argv[]);
	int outputTCC(const char* path);

	int outputEmscripten(const char* path);

	AST::File* getASTByName(const char* name);
};
