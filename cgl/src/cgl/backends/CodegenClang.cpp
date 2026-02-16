#include "cgl/CGLCompiler.h"

#include "cgl/File.h"

#include <filesystem>

#include "CodegenC.h"


#define BUILD_CMD "gcc"


void CGLCompiler::outputIR()
{
	for (AST::File* file : asts)
	{
		CodegenC codegen(this, file);
		std::string cSrc = codegen.genFile(file);

		char name[256];
		sprintf(name, "%s%s.c", "tmp/", file->getFullName());
		CreateDirectories(name);
		WriteText(cSrc.c_str(), name);
	}
}

int CGLCompiler::runLLVM(int argc, char* argv[])
{
	std::stringstream cmd;

	cmd << BUILD_CMD << ' ';

	cmd << "-w ";
	if (debugInfo)
		cmd << "-g ";
	//if (optimization > 0)
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
		sprintf(name, "%s%s.c", tmpDir.c_str(), file->getFullName());
		CreateDirectories(name);
		WriteText(cSrc.c_str(), name);

		cmd << name << ' ';
	}

	for (const char* linkerPath : linkerPaths)
	{
		cmd << "-L " << linkerPath;
	}

	for (const LinkerFile& linkerFile : linkerFiles)
	{
		cmd << "-l" << linkerFile.filename << ' ';
	}

	//CreateDirectories(path);
	cmd << "-o a.exe && \"./a.exe\"";

	std::string cmdStr = cmd.str();
	return system(cmdStr.c_str());
}

int CGLCompiler::outputLLVM(const char* path)
{
	std::stringstream cmd;

	cmd << BUILD_CMD << ' ';

	cmd << "-w ";
	if (staticLibrary)
		cmd << "-c ";
	if (sharedLibrary)
		cmd << "-shared ";
	if (debugInfo)
	{
		cmd << "-g ";
		//cmd << "-Wl,/DEBUG ";
	}
	if (optimization > 0)
		cmd << "-O" << optimization << ' ';
	cmd << "-I " << LocalFilePath("lib") << ' ';
	cmd << LocalFilePath("lib/cgl.c") << ' ';

	//cmd << "--target=x86_64-w64-windows-gnu ";

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
	}

	for (const LinkerFile& linkerFile : linkerFiles)
	{
		if (linkerFile.shorthandName)
			cmd << "-l";
		cmd << linkerFile.filename << ' ';
	}

	for (const char* linkerPath : linkerPaths)
	{
		cmd << "-L " << linkerPath;
	}

	CreateDirectories(path);
	cmd << "-o " << path;

	fprintf(stderr, "Running " BUILD_CMD " backend\n");

	std::string cmdStr = cmd.str();
	int result = system(cmdStr.c_str());

	if (debugInfo)
	{
		fprintf(stderr, "Running cv2pdb\n");

		std::string pdbStr = std::string("cv2pdb ") + path;
		system(pdbStr.c_str());
	}

	return result;
}
