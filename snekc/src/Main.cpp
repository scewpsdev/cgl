

#include "File.h"
#include "Platform.h"

#include "typechecker/TypeSystem.h"

#include "utils/Arena.h"
#include "utils/StringBuffer.h"

#include "tcc/libtcc.h"

#include <thread>
#include <filesystem>


enum SourceFileState
{
	FILE_STATE_NULL = 0,

	FILE_STATE_UNLOADED,
	FILE_STATE_PARSED,
	FILE_STATE_TYPECHECKED,
	FILE_STATE_OUTPUT,
};

struct SourceFile
{
	SourceFileState state;

	char* localPath;
	char* path;
	char* outPath;
	char* src;
	int length;
	File file;

	Parser parser;
	TypeChecker typeChecker;

	Codegen codegen;
};

struct Compiler
{
	GlobalBlockPool blockPool;
	TypeSystem types;
	Arena arena;
	Codegen codegen;

	char* mainFilePath;
	const char* outPath;
	bool run;
	bool debugInfo;
	bool optimize;
	bool gcc;

	List<SourceFile*> sourceFiles;
	List<const char*> libraryFiles;
	List<const char*> dllFiles;
};


Compiler compiler;


static void getLocalPath(char* buffer, const char* path, const char* rootPath)
{
	namespace fs = std::filesystem;
	fs::path mainFileDirectory = fs::path(rootPath).parent_path();
	fs::path localPath = fs::relative(path, mainFileDirectory);
	std::string localPathStr = localPath.string();
	strcpy(buffer, localPathStr.c_str());
}

static char* getPathFromLocalPath(const char* localPath, const char* rootPath)
{
	namespace fs = std::filesystem;
	std::string mainFileDirectory = fs::path(rootPath).parent_path().string();
	fs::path absolutePath = mainFileDirectory + "/" + localPath;
	std::string absolutePathStr = absolutePath.string();
	return _strdup(absolutePathStr.c_str());
}

static char* getOutPath(const char* localPath)
{
	std::string outPath = localPath;
	outPath = outPath.substr(0, outPath.find('.'));
	outPath = "tmp/" + outPath + ".c";
	return _strdup(outPath.c_str());
}

static const char* getExtension(const char* path)
{
	const char* ext = strrchr(path, '.');
	if (ext)
		ext++;
	else
		ext = path + strlen(path);

	return ext;
}

static StringView getDirectory(const char* path)
{
	const char* forwardSlash = strrchr(path, '/');
	const char* backwardSlash = strrchr(path, '\\');
	const char* slash = (forwardSlash && backwardSlash) ? (const char*)__max((uint64_t)forwardSlash, (uint64_t)backwardSlash) : forwardSlash ? forwardSlash : backwardSlash ? backwardSlash : NULL;
	if (!slash)
		slash = path;

	int length = (int)(slash - path);

	if (length == 0)
	{
		return CreateString(".");
	}
	else
	{
		return CreateString(path, length);
	}
}

static StringView getFilename(const char* path)
{
	const char* forwardSlash = strrchr(path, '/');
	const char* backwardSlash = strrchr(path, '\\');
	const char* slash = (forwardSlash && backwardSlash) ? (const char*)__max((uint64_t)forwardSlash, (uint64_t)backwardSlash) : forwardSlash ? forwardSlash : backwardSlash ? backwardSlash : NULL;
	if (slash)
		slash++;
	else
		slash = path;

	const char* dot = strrchr(slash, '.');
	if (!dot)
		dot = slash + strlen(slash);

	int length = (int)(dot - slash);

	return CreateString(slash, length);
}

static char* readText(const char* path, int* length)
{
	if (FILE* file = fopen(path, "rb"))
	{
		fseek(file, 0, SEEK_END);
		long numBytes = ftell(file);
		fseek(file, 0, SEEK_SET);

		char* buffer = (char*)malloc(numBytes + 1);
		memset(buffer, 0, numBytes);
		numBytes = (long)fread(buffer, 1, numBytes, file);
		fclose(file);
		buffer[numBytes] = 0;

		*length = (int)numBytes;
		return buffer;
	}

	*length = 0;
	return nullptr;
}

void createDirectories(const char* path)
{
	namespace fs = std::filesystem;
	if (fs::is_directory(path))
		fs::create_directories(path);
	else
	{
		auto directory = fs::path(path).parent_path();
		if (!directory.empty())
			fs::create_directories(directory);
	}
}

static bool addSourceFile(Compiler* compiler, const char* localPath)
{
	SourceFile* file = compiler->arena.alloc<SourceFile>();

	file->localPath = _strdup(localPath);
	file->path = getPathFromLocalPath(localPath, compiler->mainFilePath);
	file->outPath = getOutPath(localPath);
	file->state = FILE_STATE_UNLOADED;

	if (char* src = readText(file->path, &file->length))
	{
		file->src = src;

		initFile(&file->file, file->localPath, file->path, file->src, file->length, &compiler->blockPool);

		compiler->sourceFiles.add(file);

		return true;
	}
	else
	{
		free(file->localPath);
		free(file->path);
		free(file->outPath);

		return false;
	}
}

static void initCompiler(Compiler* compiler, int argc, const char* argv[])
{
	if (argc == 0)
	{
		// todo print help
		fprintf(stderr, "Must specify command\n");
		exit(1);
	}

	initGlobalBlockPool(&compiler->blockPool, 16);
	initArena(&compiler->arena, &compiler->blockPool);
	initTypeSystem(&compiler->types, &compiler->arena);

	initCodegen(&compiler->codegen, &compiler->types, &compiler->arena);

	const char* cmd = argv[1]; // build
	if (strcmp(cmd, "build") == 0)
		compiler->outPath = "bin/a.exe";
	else if (strcmp(cmd, "run") == 0)
		compiler->run = true;
	else
	{
		fprintf(stderr, "Undefined command: %s\n", cmd);
		exit(1);
	}

	for (int i = 2; i < argc; i++)
	{
		const char* arg = argv[i];
		int len = (int)strlen(arg);
		if (strcmp(arg, "-o") == 0)
		{
			if (i < argc - 1)
			{
				compiler->outPath = argv[++i];
			}
		}
		else if (strcmp(arg, "-debug") == 0)
		{
			compiler->debugInfo = true;
		}
		else if (strcmp(arg, "-optimize") == 0)
		{
			compiler->optimize = true;
		}
		else if (strcmp(arg, "-gcc") == 0)
		{
			compiler->gcc = true;
		}
		else
		{
			// file path

			const char* extension = getExtension(arg);
			if (strcmp(extension, "src") == 0)
			{
				if (!compiler->mainFilePath)
				{
					compiler->mainFilePath = _strdup(arg);
				}

				char localPath[256] = "";
				getLocalPath(localPath, arg, compiler->mainFilePath);

				addSourceFile(compiler, localPath);
			}
			else if (strcmp(extension, "a") == 0 || strcmp(extension, "lib") == 0)
			{
				compiler->libraryFiles.add(arg);
			}
			else if (strcmp(extension, "dll") == 0)
			{
				compiler->dllFiles.add(arg);
			}
			else
			{
				fprintf(stderr, "Undefined %s argument: %s\n", cmd, arg);
				exit(1);
			}
		}
	}
}

static void destroyCompiler(Compiler* compiler)
{
	destroyTypeSystem(&compiler->types);
}

SourceFile* getSourceFile(FileHandle fileHandle)
{
	for (int i = 0; i < compiler.sourceFiles.size; i++)
	{
		if (compiler.sourceFiles[i]->file.handle == fileHandle)
			return compiler.sourceFiles[i];
	}
	return nullptr;
}

File* getFileFromHandle(FileHandle fileHandle)
{
	if (SourceFile* file = getSourceFile(fileHandle))
		return &file->file;
	return nullptr;
}

bool isFileLoaded(FileHandle fileHandle)
{
	if (SourceFile* file = getSourceFile(fileHandle))
		return true;
	return false;
}

static void parseFilesRange(List<SourceFile*>& files, int start, int end)
{
	for (int i = start; i < end; i++)
	{
		SourceFile* file = files[i];

		initParser(&file->parser, &file->file);
		parse(&file->parser, &file->file.ast);
		//destroyParser(&file->file.parser);

		file->state = FILE_STATE_PARSED;
	}
}

static void parseFiles(List<SourceFile*>& files)
{
	int numThreads = (int)std::thread::hardware_concurrency();
	if (numThreads == 0) numThreads = 2;

	int chunkSize = files.size / numThreads;
	int remainder = files.size % numThreads;

	int start = 0;

	std::thread workers[32] = {};
	int numWorkers = 0;

	for (int i = 0; i < numThreads; i++)
	{
		int end = start + chunkSize + (i < remainder ? 1 : 0);

		workers[numWorkers++] = std::thread(parseFilesRange, std::ref(files), start, end);

		start = end;
	}

	for (int i = 0; i < numWorkers; i++)
	{
		if (workers[i].joinable())
			workers[i].join();
	}
}

static void symbolCollectFilesRange(List<SourceFile*>& files, int start, int end)
{
	for (int i = start; i < end; i++)
	{
		SourceFile* file = files[i];

		initTypeTable(&file->file.typeTable, &file->file.arena, 64);
		initTypeChecker(&file->typeChecker, &file->file.arena, &file->file.scratch, &file->file.diagnostics, &compiler.types);
		symbolCollection(&file->typeChecker, &file->file);
	}
}

static void symbolCollectFiles(List<SourceFile*>& files)
{
	int numThreads = (int)std::thread::hardware_concurrency();
	if (numThreads == 0) numThreads = 2;

	int chunkSize = files.size / numThreads;
	int remainder = files.size % numThreads;

	int start = 0;

	std::thread workers[32] = {};
	int numWorkers = 0;

	for (int i = 0; i < numThreads; i++)
	{
		int end = start + chunkSize + (i < remainder ? 1 : 0);

		workers[numWorkers++] = std::thread(symbolCollectFilesRange, std::ref(files), start, end);

		start = end;
	}

	for (int i = 0; i < numWorkers; i++)
	{
		if (workers[i].joinable())
			workers[i].join();
	}
}

static void symbolResolveFiles(List<SourceFile*>& files)
{
	for (int i = 0; i < files.size; i++)
	{
		SourceFile* file = files[i];

		symbolResolution(&file->typeChecker, &file->file);
	}
}

static void typeCheckFunctionsRange(List<SourceFile*>& files, int start, int end)
{
	for (int i = start; i < end; i++)
	{
		SourceFile* file = files[i];

		typeCheckFunctions(&file->typeChecker, &file->file);

		file->state = FILE_STATE_TYPECHECKED;
	}
}

static void typeCheckFunctions(List<SourceFile*>& files)
{
	int numThreads = (int)std::thread::hardware_concurrency();
	if (numThreads == 0) numThreads = 2;

	int chunkSize = files.size / numThreads;
	int remainder = files.size % numThreads;

	int start = 0;

	std::thread workers[32] = {};
	int numWorkers = 0;

	for (int i = 0; i < numThreads; i++)
	{
		int end = start + chunkSize + (i < remainder ? 1 : 0);

		workers[numWorkers++] = std::thread(typeCheckFunctionsRange, std::ref(files), start, end);

		start = end;
	}

	for (int i = 0; i < numWorkers; i++)
	{
		if (workers[i].joinable())
			workers[i].join();
	}
}

static void localFilePath(char* result, const char* path)
{
	std::filesystem::path exePath = GetExecutablePath();
	std::filesystem::path localDir = exePath.parent_path();
	std::string localDirStr = localDir.string();

	strcpy(result, localDirStr.c_str());
	strcat(result, "/");
	strcat(result, path);
}

static TCCState* tcc;

static int outputBinaryTCC()
{
	char buffer[512] = "";

	localFilePath(buffer, "lib/libtcc");
	tcc_set_lib_path(tcc, buffer);

	localFilePath(buffer, "lib/libtcc");
	tcc_add_library_path(tcc, buffer);
	localFilePath(buffer, "lib");
	tcc_add_include_path(tcc, buffer);
	localFilePath(buffer, "lib/libtcc/include");
	tcc_add_include_path(tcc, buffer);
	localFilePath(buffer, "lib/libtcc/include/winapi");
	tcc_add_include_path(tcc, buffer);

	tcc_add_library_path(tcc, ".");

	tcc_define_symbol(tcc, "DLLEXPORT", "__attribute__((dllexport))");
	tcc_define_symbol(tcc, "DLLIMPORT", "__attribute__((dllimport))");
	tcc_define_symbol(tcc, "true", "1");
	tcc_define_symbol(tcc, "false", "0");

	tcc_set_output_type(tcc, compiler.run ? TCC_OUTPUT_MEMORY : TCC_OUTPUT_EXE);

	localFilePath(buffer, "lib/snek.c");
	tcc_add_file(tcc, buffer);

	for (int i = 0; i < compiler.sourceFiles.size; i++)
	{
		tcc_add_file(tcc, compiler.sourceFiles[i]->outPath);
	}

	for (int i = 0; i < compiler.libraryFiles.size; i++)
	{
		const char* library = compiler.libraryFiles[i];
		StringView directory = getDirectory(library);
		StringView name = getFilename(library);

		if (name.startsWith("lib"))
			name = name.substring(3);
		if (name.endsWith(".dll"))
			name = name.substring(0, name.length - 4);

		directory = copy(directory);
		name = copy(name);

		tcc_add_library_path(tcc, directory.ptr);
		tcc_add_library(tcc, name.ptr);

		destroy(directory);
		destroy(name);
	}

	for (int i = 0; i < compiler.dllFiles.size; i++)
	{
		tcc_add_file(tcc, compiler.dllFiles[i]);
	}

	if (compiler.run)
	{
		int result = tcc_relocate(tcc);

		return result;
	}
	else
	{
		createDirectories(compiler.outPath);
		int result = tcc_output_file(tcc, compiler.outPath);

		return result;
	}
}

static int runBinaryTCC()
{
	int(*entrypoint)(int, const char**) = (int(*)(int, const char**))tcc_get_symbol(tcc, "main");
	const char* arg = "TCC Runtime";
	int result = entrypoint(1, &arg);
	return result;
}

static int outputBinaryGCC()
{
	char buffer[512] = "";
	StringBuffer command = CreateStringBuffer(256);

	StringBufferAppend(command, "gcc -I lib ");

	localFilePath(buffer, "lib");
	StringBufferAppend(command, "-I ");
	StringBufferAppend(command, buffer);
	StringBufferAppend(command, ' ');

	StringBufferAppend(command, "-D DLLEXPORT=__attribute((dllexport)) ");
	StringBufferAppend(command, "-D DLLIMPORT=__attribute((dllimport)) ");
	StringBufferAppend(command, "-D true=1 ");
	StringBufferAppend(command, "-D false=0 ");

	localFilePath(buffer, "lib/snek.c");
	StringBufferAppend(command, buffer);
	StringBufferAppend(command, ' ');

	//tcc_set_output_type(tcc, compiler.run ? TCC_OUTPUT_MEMORY : TCC_OUTPUT_EXE);

	for (int i = 0; i < compiler.sourceFiles.size; i++)
	{
		StringBufferAppend(command, compiler.sourceFiles[i]->outPath);
		StringBufferAppend(command, ' ');
	}

	for (int i = 0; i < compiler.libraryFiles.size; i++)
	{
		const char* library = compiler.libraryFiles[i];
		StringView directory = getDirectory(library);
		StringView name = getFilename(library);

		if (name.startsWith("lib"))
			name = name.substring(3);
		if (name.endsWith(".dll"))
			name = name.substring(0, name.length - 4);

		directory = copy(directory);
		name = copy(name);

		StringBufferAppend(command, "-L ");
		StringBufferAppend(command, directory.ptr);
		StringBufferAppend(command, ' ');

		StringBufferAppend(command, "-l");
		StringBufferAppend(command, name.ptr);
		StringBufferAppend(command, ' ');

		destroy(directory);
		destroy(name);
	}

	for (int i = 0; i < compiler.dllFiles.size; i++)
	{
		//tcc_add_file(tcc, compiler.dllFiles[i]);
	}

	if (compiler.debugInfo)
	{
		StringBufferAppend(command, "-g ");
	}
	if (compiler.optimize)
	{
		StringBufferAppend(command, "-O3 ");
	}

	if (compiler.run)
	{
		//int result = tcc_relocate(tcc);

		return -1;
	}
	else
	{
		createDirectories(compiler.outPath);

		StringBufferAppend(command, "-o ");
		StringBufferAppend(command, compiler.outPath);

		int result = system(command.buffer);

		return result;
	}

	DestroyStringBuffer(command);
}

static int runBinaryGCC()
{
	return -1;
}

int main(int argc, const char* argv[])
{
	uint64_t t0 = GetTimeNS();

	compiler = {};
	initCompiler(&compiler, argc, argv);

	uint64_t t1 = GetTimeNS();

	List<SourceFile*> filesToParse;
	List<SourceFile*> filesToTypeCheck;
	bool allFilesParsed = false;

	while (!allFilesParsed)
	{
		filesToParse.clear();
		filesToTypeCheck.clear();

		for (int i = 0; i < compiler.sourceFiles.size; i++)
		{
			if (compiler.sourceFiles[i]->state == FILE_STATE_UNLOADED)
			{
				filesToParse.add(compiler.sourceFiles[i]);
			}
		}

		parseFiles(filesToParse);

		for (int i = 0; i < filesToParse.size; i++)
		{
			SourceFile* file = filesToParse[i];

			for (int j = 0; j < file->file.ast.numDeclarations; j++)
			{
				Node* declaration = file->file.ast.declarations[j];
				if (declaration->type == NODE_IMPORT)
				{
					Import* import_ = &declaration->import;

					char localPath[256];
					getLocalPathFromModuleName(localPath, import_->path, import_->pathCount);

					FileHandle fileHandle = getFileHandle(localPath);
					if (!getFileFromHandle(fileHandle))
					{
						if (!addSourceFile(&compiler, localPath))
						{
							error(&file->parser, declaration, "Undefined module '%s'", localPath);
						}
					}

					file->file.dependencies.add(fileHandle);
				}
			}
		}

		allFilesParsed = true;
		for (int i = 0; i < compiler.sourceFiles.size; i++)
		{
			if (compiler.sourceFiles[i]->state == FILE_STATE_UNLOADED)
			{
				allFilesParsed = false;
				break;
			}
		}
	}

	uint64_t t2 = GetTimeNS();

	symbolCollectFiles(compiler.sourceFiles);
	symbolResolveFiles(compiler.sourceFiles);
	typeCheckFunctions(compiler.sourceFiles);

	uint64_t t3 = GetTimeNS();

	bool hasError = false;
	for (int i = 0; i < compiler.sourceFiles.size; i++)
	{
		SourceFile* file = compiler.sourceFiles[i];
		Diagnostics* diagnostics = &file->file.diagnostics;
		for (int j = 0; j < diagnostics->items.size; j++)
		{
			fprintf(stderr, "error %s:%d:%d: %s\n", file->localPath, diagnostics->items[j].startLine + 1, diagnostics->items[j].startCol + 1, diagnostics->items[j].message);
			hasError = true;
		}
	}

	uint64_t t4 = GetTimeNS();
	uint64_t t5 = t4;

	int result = hasError ? 1 : 0;
	if (!hasError)
	{
		for (int i = 0; i < compiler.sourceFiles.size; i++)
		{
			SourceFile* file = compiler.sourceFiles[i];
			createDirectories(file->outPath);
			emitFile(&compiler.codegen, &file->file, file->localPath, file->outPath, compiler.debugInfo);
			file->state = FILE_STATE_OUTPUT;
		}

		t5 = GetTimeNS();

		if (compiler.gcc)
		{
			result = outputBinaryGCC();
		}
		else
		{
			tcc = tcc_new();
			result = outputBinaryTCC();
		}

		uint64_t t6 = GetTimeNS();

		if (result == 0)
		{
			float totalMs = (t6 - t0) / 1e6f;

			fprintf(stderr, "Compilation completed in %.2f ms.\n", totalMs);

			float parseMs = (t2 - t1) / 1e6f;
			float typeCheckMs = (t3 - t2) / 1e6f;
			float codegenMs = (t5 - t4) / 1e6f;
			float outputMs = (t6 - t5) / 1e6f;

			fprintf(stderr, "Parser: %.2f ms\n", parseMs);
			fprintf(stderr, "Typecheck: %.2f ms\n", typeCheckMs);
			fprintf(stderr, "Codegen: %.2f ms\n", codegenMs);
			fprintf(stderr, "Output: %.2f ms\n", outputMs);

			if (compiler.run)
			{
				if (compiler.gcc)
				{
					int runResult = runBinaryGCC();
					fprintf(stderr, "Exited with code %d\n", runResult);
				}
				else
				{
					int runResult = runBinaryTCC();
					fprintf(stderr, "Exited with code %d\n", runResult);
				}
			}
		}

		if (compiler.gcc)
		{
		}
		else
		{
			tcc_delete(tcc);
		}
	}

	if (result != 0)
	{
		fprintf(stderr, "Compilation failed.\n");
	}

	destroyCompiler(&compiler);

	return result;
}
