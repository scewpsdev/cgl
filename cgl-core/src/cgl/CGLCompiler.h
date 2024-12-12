#pragma once

#include "pch.h"

#include "SourceFile.h"

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


	void init(MessageCallback_t msgCallback);
	void terminate();

	void addFile(const char* filename, const char* name, const char* src);
	void addLinkerFile(const char* filename);
	void addLinkerPath(const char* path);
	bool compile();
	int run(int argc, char* argv[], bool printIR);
	int output(const char* path, bool printIR);
};
