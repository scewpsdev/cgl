

#include "File.h"
#include "Platform.h"

#include "typechecker/TypeSystem.h"

#include "utils/Arena.h"

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

	List<SourceFile*> sourceFiles;
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

		initFile(&file->file, localPath, &compiler->blockPool);

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
		// error
		exit(1);
	}

	initGlobalBlockPool(&compiler->blockPool, 16);
	initTypeSystem(&compiler->types);
	initArena(&compiler->arena, &compiler->blockPool);

	initCodegen(&compiler->codegen, &compiler->types, &compiler->blockPool);

	const char* cmd = argv[1]; // build

	compiler->outPath = "bin/a.exe";

	for (int i = 2; i < argc; i++)
	{
		const char* arg = argv[i];
		int len = (int)strlen(arg);
		if (false)
			;
		else
		{
			// file path
			if (!compiler->mainFilePath)
			{
				compiler->mainFilePath = _strdup(arg);
			}

			char localPath[256] = "";
			getLocalPath(localPath, arg, compiler->mainFilePath);

			addSourceFile(compiler, localPath);
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

static void parseFilesRange(List<SourceFile*>& files, int start, int end)
{
	for (int i = start; i < end; i++)
	{
		SourceFile* file = files[i];

		initParser(&file->file.parser, file->path, file->src, file->length, &file->file.arena, &file->file.scratch, &file->file.diagnostics);
		parse(&file->file.parser, &file->file.ast);
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

		initTypeChecker(&file->file.typeChecker, &file->file.arena, &file->file.scratch, &file->file.parser.lexer, &file->file.diagnostics, &compiler.types);
		symbolCollection(&file->file.typeChecker, &file->file);
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

		symbolResolution(&file->file.typeChecker, &file->file);
	}
}

static void typeCheckFunctionsRange(List<SourceFile*>& files, int start, int end)
{
	for (int i = start; i < end; i++)
	{
		SourceFile* file = files[i];

		for (int j = 0; j < file->file.ast.numFunctions; j++)
		{
			typeCheckFunction(&file->file.typeChecker, file->file.ast.functions[j], &file->file);
		}

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

static void outputBinary(const char* out)
{
	TCCState* tcc = tcc_new();

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

	tcc_set_output_type(tcc, TCC_OUTPUT_EXE);

	localFilePath(buffer, "lib/snek.c");
	tcc_add_file(tcc, buffer);

	for (int i = 0; i < compiler.sourceFiles.size; i++)
	{
		tcc_add_file(tcc, compiler.sourceFiles[i]->outPath);
	}

	tcc_define_symbol(tcc, "DLLEXPORT", "__attribute__((dllexport))");

	createDirectories(out);
	int result = tcc_output_file(tcc, out);

	tcc_delete(tcc);
}

int main(int argc, const char* argv[])
{
	compiler = {};
	initCompiler(&compiler, argc, argv);

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
						if (addSourceFile(&compiler, localPath))
						{
							file->file.dependencies.add(fileHandle);
						}
						else
						{
							error(&file->file.parser, declaration, "Undefined module '%s'", localPath);
						}
					}
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

	symbolCollectFiles(compiler.sourceFiles);
	symbolResolveFiles(compiler.sourceFiles);
	typeCheckFunctions(compiler.sourceFiles);

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

	if (!hasError)
	{
		for (int i = 0; i < compiler.sourceFiles.size; i++)
		{
			SourceFile* file = compiler.sourceFiles[i];
			createDirectories(file->outPath);
			emitFile(&compiler.codegen, &file->file, file->localPath, file->outPath);
			file->state = FILE_STATE_OUTPUT;
		}

		outputBinary(compiler.outPath);
	}

	destroyCompiler(&compiler);

	return EXIT_SUCCESS;
}
