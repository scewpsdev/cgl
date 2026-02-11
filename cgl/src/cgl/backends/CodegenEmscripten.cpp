#include "cgl/CGLCompiler.h"

#include "cgl/File.h"

#include <filesystem>

#include "CodegenC.h"


#define BUILD_CMD "emcc"


int CGLCompiler::outputEmscripten(const char* path)
{
	std::stringstream cmd;

	std::vector<std::string> tmpFiles;

	cmd << BUILD_CMD << ' ';

	cmd << "-I " << LocalFilePath("lib") << ' ';
	cmd << LocalFilePath("lib/cgl.c") << ' ';

	for (AST::File* file : asts)
	{
		CodegenC codegen(this, file);
		std::string cSrc = codegen.genFile(file);
		if (printIR)
		{
			fprintf(stderr, "%s\n", cSrc.c_str());
			fprintf(stderr, "----------------------\n");
		}

		char name[256];
		sprintf(name, "%s%s.c", "tmp/", file->getFullName());
		CreateDirectories(name);
		WriteText(cSrc.c_str(), name);

		cmd << name << ' ';

		tmpFiles.push_back(name);
	}

	for (const LinkerFile& linkerFile : linkerFiles)
	{
		cmd << linkerFile.filename << ' ';
	}

	cmd << "-sUSE_SDL=3 ";

	CreateDirectories(path);
	cmd << "-o " << path;

	fprintf(stderr, "Running " BUILD_CMD " backend\n");

	std::string cmdStr = cmd.str();
	int result = system(cmdStr.c_str());

	return result;
}
