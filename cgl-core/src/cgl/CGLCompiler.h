#pragma once

#include "pch.h"

#include "SourceFile.h"


class CGLCompiler
{
	std::vector<SourceFile> sourceFiles;

public:
	void init();
	void terminate();

	void addFile(const char* src);
	bool compile();
	int run(int argc, char* argv[]);
};
