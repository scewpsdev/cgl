#include "cgl/CGLCompiler.h"

#include "cgl/File.h"

#include <filesystem>


#include "CodegenC.h"


int CGLCompiler::run(int argc, char* argv[])
{
	std::stringstream cmd;

	std::vector<std::string> tmpFiles;

	cmd << "clang ";

	cmd << "-w ";
	if (debugInfo)
		cmd << "-g ";
	if (optimization > 0)
		cmd << "-O" << optimization << ' ';
	cmd << "-D DLLEXPORT=__declspec(dllexport) ";
	if (runtimeStackTrace)
		cmd << "-D RUNTIME_STACK_TRACE ";

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

		std::string tmpDir = std::filesystem::temp_directory_path().string();

		char name[256];
		sprintf(name, "%s%s.c", tmpDir.c_str(), file->name);
		WriteText(cSrc.c_str(), name);

		cmd << name << ' ';

		tmpFiles.push_back(name);
	}

	for (const LinkerFile& linkerFile : linkerFiles)
	{
		cmd << linkerFile.filename << ' ';
	}

	for (const char* linkerPath : linkerPaths)
	{
		cmd << "-L " << linkerPath;
	}

	//CreateDirectories(path);
	cmd << "-o a.exe && \"./a.exe\"";

	std::string cmdStr = cmd.str();
	return system(cmdStr.c_str());
}

int CGLCompiler::output(const char* path)
{
	std::stringstream cmd;

	std::vector<std::string> tmpFiles;

	cmd << "clang ";

	cmd << "-w ";
	if (staticLibrary)
		cmd << "-c ";
	if (sharedLibrary)
		cmd << "-shared ";
	if (debugInfo)
		cmd << "-g ";
	if (optimization > 0)
		cmd << "-O" << optimization << ' ';
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

	for (const char* linkerPath : linkerPaths)
	{
		cmd << "-L " << linkerPath;
	}

	//CreateDirectories(path);
	cmd << "-o " << path;

	fprintf(stderr, "Running clang backend\n");

	std::string cmdStr = cmd.str();
	return system(cmdStr.c_str());
}
