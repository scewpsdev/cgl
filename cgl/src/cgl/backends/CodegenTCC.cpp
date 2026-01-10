#include "cgl/CGLCompiler.h"

#include "cgl/File.h"

#include "lib/libtcc/libtcc.h"


#include "CodegenC.h"


int CGLCompiler::runTCC(int argc, char* argv[])
{
	TCCState* tcc = tcc_new();

	tcc_add_library_path(tcc, LocalFilePath("lib\\libtcc"));
	tcc_add_include_path(tcc, LocalFilePath("lib"));
	tcc_add_include_path(tcc, LocalFilePath("lib\\libtcc\\include"));
	tcc_add_include_path(tcc, LocalFilePath("lib\\libtcc\\include\\winapi"));

	//tcc_add_library(tcc, ".\\lib\\libtcc1-64.a");
	//tcc_add_include_path(tcc, "D:\\Repositories\\tcc\\include\\");
	//tcc_add_library_path(tcc, "D:\\Repositories\\tcc\\lib\\");
	//tcc_set_options(tcc, "-L D:\\Repositories\\tcc\\lib\\ -lucrt -ldl -lmsvcrt");
	//tcc_set_lib_path(tcc, "D:\\Repositories\\tcc\\");
	//tcc_add_library_path(tcc, "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.19041.0\\ucrt\\x64\\");
	//tcc_add_library(tcc, "libb/llibtcc1-64.a");
	//tcc_add_library(tcc, "msvcrt.lib");
	//tcc_set_options(tcc, "-Wall -LC:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\14.37.32822\\lib\\x64 -lucrt -lcmt");
	//tcc_add_library_path(tcc, "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\14.37.32822\\lib\\x64");
	//tcc_add_library(tcc, "libucrt.lib");

	tcc_set_output_type(tcc, TCC_OUTPUT_MEMORY);

	{
		char* builtinSrc = ReadText(LocalFilePath("lib/cgl.c"));
		if (builtinSrc)
		{
			tcc_compile_string(tcc, builtinSrc);
			delete builtinSrc;
		}
		else
		{
			fprintf(stderr, "cgl.c is missing!\n");
			return -1;
		}
	}

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
		sprintf(name, "%s%s.c", "tmp/", file->name);
		CreateDirectories(name);
		WriteText(cSrc.c_str(), name);

		if (tcc_add_file(tcc, name) == -1)
		{
			SnekAssert(false);
		}
	}

	for (const LinkerFile& linkerFile : linkerFiles)
	{
		if (tcc_add_file(tcc, linkerFile.filename))
		{
			SnekError(this, "Failed to add file %s", linkerFile.filename);
		}
	}

	for (const char* linkerPath : linkerPaths)
	{
		if (tcc_add_library_path(tcc, linkerPath))
		{
			SnekError(this, "Failed to add linker path %s", linkerPath);
		}
	}

	if (runtimeStackTrace)
		tcc_define_symbol(tcc, "RUNTIME_STACK_TRACE", nullptr);

	int result = tcc_run(tcc, argc, argv);

	tcc_delete(tcc);

	return result;
}

int CGLCompiler::outputTCC(const char* path)
{
	TCCState* tcc = tcc_new();

	tcc_add_library_path(tcc, LocalFilePath("lib\\libtcc"));
	tcc_add_include_path(tcc, LocalFilePath("lib"));
	tcc_add_include_path(tcc, LocalFilePath("lib\\libtcc\\include"));
	tcc_add_include_path(tcc, LocalFilePath("lib\\libtcc\\include\\winapi"));

	if (staticLibrary)
		tcc_set_output_type(tcc, TCC_OUTPUT_OBJ);
	else if (sharedLibrary)
		tcc_set_output_type(tcc, TCC_OUTPUT_DLL);
	else
		tcc_set_output_type(tcc, TCC_OUTPUT_EXE);

	{
		char* builtinSrc = ReadText(LocalFilePath("lib/cgl.c"));
		if (builtinSrc)
		{
			tcc_compile_string(tcc, builtinSrc);
			delete builtinSrc;
		}
		else
		{
			fprintf(stderr, "cgl.c is missing!\n");
			return -1;
		}
	}

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
		sprintf(name, "%s%s.c", "tmp/", file->name);
		CreateDirectories(name);
		WriteText(cSrc.c_str(), name);

		if (tcc_add_file(tcc, name) == -1)
		{
			SnekAssert(false);
		}
	}

	for (const char* linkerPath : linkerPaths)
	{
		if (tcc_add_library_path(tcc, linkerPath))
		{
			SnekError(this, "Failed to add linker path %s", linkerPath);
		}
	}

	for (const LinkerFile& linkerFile : linkerFiles)
	{
		if (tcc_add_library(tcc, linkerFile.filename))
		{
			SnekError(this, "Failed to add library %s", linkerFile.filename);
		}
	}

	tcc_define_symbol(tcc, "DLLEXPORT", "__attribute__((dllexport))");
	if (runtimeStackTrace)
		tcc_define_symbol(tcc, "RUNTIME_STACK_TRACE", nullptr);

	fprintf(stderr, "Running TCC backend\n");

	CreateDirectories(path);
	int result = tcc_output_file(tcc, path);

	tcc_delete(tcc);

	return result;
}
