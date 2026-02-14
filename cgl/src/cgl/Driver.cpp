#include "cgl/CGLCompiler.h"

#include "cgl/File.h"

#include <stdio.h>
#include <stdarg.h>

#include <filesystem>


#define VERSION_MAJOR 0
#define VERSION_MINOR 7
#define VERSION_PATCH 2
#define VERSION_SUFFIX "-alpha"

#define OPT_BUILD_HELP "-help"
#define OPT_BUILD_OUTPATH "-o"
#define OPT_BUILD_LINKER_PATH "-L"
#define OPT_BUILD_LINKER_FILE "-l"
#define OPT_BUILD_PRINT_IR "-print-ir"
#define OPT_BUILD_GEN_DEBUG_INFO "-debuginfo"
#define OPT_BUILD_OPTIMIZE "-optimize"
#define OPT_BUILD_STATIC_LIBRARY "-library"
#define OPT_BUILD_SHARED_LIBRARY "-shared"
#define OPT_BUILD_GEN_RUNTIME_STACKTRACE "-runtime-stacktrace"
#define OPT_BUILD_BACKEND_CLANG "-gcc"
#define OPT_BUILD_BACKEND_TCC "-tcc"
#define OPT_BUILD_BACKEND_EMSCRIPTEN "-emcc"


void printBuildHelp(char linePrefix)
{
	printf("%cbuild - Builds source files into a binary file.\n", linePrefix);
	printf("%c\t%s - The path of the output binary.\n", linePrefix, OPT_BUILD_OUTPATH);
	printf("%c\t%s - Linker path for static libraries to be linked.\n", linePrefix, OPT_BUILD_LINKER_PATH);
	printf("%c\t%s - Name of a static library to be linked.\n", linePrefix, OPT_BUILD_LINKER_FILE);
	printf("%c\t%s - Print the intermediate representation (such as C or LLVM IR) of the used backend to stdout. Useful for debugging code generation.\n", linePrefix, OPT_BUILD_PRINT_IR);
	printf("%c\t%s - Generate debug information long with the binaries.\n", linePrefix, OPT_BUILD_GEN_DEBUG_INFO);
	printf("%c\t%s - Optimize the binaries.\n", linePrefix, OPT_BUILD_OPTIMIZE);
	printf("%c\t%s - Build a static library. This does not invoke the linker.\n", linePrefix, OPT_BUILD_STATIC_LIBRARY);
	printf("%c\t%s - Build a shared library. This performs a link just like when building an executable.\n", linePrefix, OPT_BUILD_SHARED_LIBRARY);
	printf("%c\t%s - Generate rudimentary runtime stacktrace information that gets printed to stderr on segfault.\n", linePrefix, OPT_BUILD_GEN_RUNTIME_STACKTRACE);
}

void printRunHelp(char linePrefix)
{
	printf("%c\trun - Runs the main function of the given source files.\n", linePrefix);
	printf("%c\t%s - Linker path for static libraries to be linked.\n", linePrefix, OPT_BUILD_LINKER_PATH);
	printf("%c\t%s - Name of a static library to be linked.\n", linePrefix, OPT_BUILD_LINKER_FILE);
	printf("%c\t%s - Print the intermediate representation (such as C or LLVM IR) of the used backend to stdout. Useful for debugging code generation.\n", linePrefix, OPT_BUILD_PRINT_IR);
	printf("%c\t%s - Optimize the code before running it.\n", linePrefix, OPT_BUILD_OPTIMIZE);
	printf("%c\t%s - Generate rudimentary runtime stacktrace information that gets printed to stderr on segfault.\n", linePrefix, OPT_BUILD_GEN_RUNTIME_STACKTRACE);
}

static void OnCompilerMessage(CGLCompiler* context, MessageType msgType, const char* filename, int line, int col, const char* msg, ...)
{
	if (context->disableError)
		return;

	static const char* const MSG_TYPE_NAMES[MESSAGE_TYPE_MAX] = {
		"<null>",
		"info",
		"warning",
		"error",
		"fatal error",
	};

	MessageType minMsgType =
#if _DEBUG
		MESSAGE_TYPE_INFO
#else
		MESSAGE_TYPE_WARNING
#endif
		;

	if (msgType >= minMsgType)
	{
		static char message[4192] = {};
		message[0] = 0;

		if (filename)
			sprintf(message + strlen(message), "%s:%d:%d: ", filename, line, col);

		if (msgType != MESSAGE_TYPE_INFO)
			sprintf(message + strlen(message), "%s: ", MSG_TYPE_NAMES[msgType]);

		va_list args;
		va_start(args, msg);
		vsprintf(message + strlen(message), msg, args);
		va_end(args);

		fprintf(stderr, "%s\n", message);
	}
}

static uint64_t max(uint64_t a, uint64_t b)
{
	return a > b ? a : b;
}

static char* GetModuleNameFromPath(const char* path)
{
	const char* forwardSlash = strrchr(path, '/');
	const char* backwardSlash = strrchr(path, '\\');
	const char* slash = (forwardSlash && backwardSlash) ? (const char*)max((uint64_t)forwardSlash, (uint64_t)backwardSlash) : forwardSlash ? forwardSlash : backwardSlash ? backwardSlash : NULL;
	if (slash)
		slash++;
	else
		slash = path;

	const char* dot = strrchr(slash, '.');
	if (!dot)
		dot = slash + strlen(slash);

	int length = (int)(dot - slash);

	char* name = new char[length + 1];
	strncpy(name, slash, length);
	name[length] = 0;

	return name;
}

static const char* GetFilenameFromPath(const char* path)
{
	const char* forwardSlash = strrchr(path, '/');
	const char* backwardSlash = strrchr(path, '\\');
	const char* slash = (forwardSlash && backwardSlash) ? (const char*)max((uint64_t)forwardSlash, (uint64_t)backwardSlash) : forwardSlash ? forwardSlash : backwardSlash ? backwardSlash : NULL;
	if (slash)
		slash++;
	else
		slash = path;

	return slash;
}

static char* GetFolderFromPath(const char* path)
{
	const char* forwardSlash = strrchr(path, '/');
	const char* backwardSlash = strrchr(path, '\\');
	const char* slash = (forwardSlash && backwardSlash) ? (const char*)max((uint64_t)forwardSlash, (uint64_t)backwardSlash) : forwardSlash ? forwardSlash : backwardSlash ? backwardSlash : NULL;
	if (!slash)
		slash = path;

	int length = (int)(slash - path);

	if (length == 0)
	{
		char* folder = new char[2];
		strcpy(folder, ".");
		folder[1] = 0;
		return folder;
	}
	else
	{
		char* folder = new char[length + 1];
		strncpy(folder, path, length);
		folder[length] = 0;
		return folder;
	}
}

static const char* GetExtensionFromPath(const char* path)
{
	const char* ext = strrchr(path, '.');
	if (ext)
		ext++;
	else
		ext = path + strlen(path);

	return ext;
}

static void AddSourceFile(CGLCompiler& compiler, char* src, char* path, char* moduleName, char* filename, char* directory)
{
	compiler.addFile(path, moduleName, src);
	//SnekAddSource(context, src, path, moduleName, filename, directory);
}

static void AddLinkerFile(CGLCompiler& compiler, const char* path)
{
	compiler.addLinkerFile(path, false);
	//SnekAddLinkerFile(context, path);
}

static bool AddFile(CGLCompiler& compiler, const char* path)
{
	const char* fileExtension = GetExtensionFromPath(path);
	if (strcmp(fileExtension, "obj") == 0 || strcmp(fileExtension, "o") == 0 || strcmp(fileExtension, "lib") == 0 || strcmp(fileExtension, "a") == 0 || strcmp(fileExtension, "def") == 0 || strcmp(fileExtension, "dll") == 0)
	{
		AddLinkerFile(compiler, path);
		return true;
	}
	else
	{
		if (char* src = ReadText(path))
		{
			char* moduleName = GetModuleNameFromPath(path);
			char* filename = _strdup(GetFilenameFromPath(path));
			char* directory = GetFolderFromPath(path);
			AddSourceFile(compiler, src, _strdup(path), moduleName, filename, directory);
			return true;
		}
		else
		{
			SnekError(&compiler, "Unknown file '%s'", path);
			return false;
		}
	}
}

static bool AddSourceFolder(CGLCompiler& compiler, const char* folder, const char* extension, bool recursive)
{
	if (!std::filesystem::exists(folder)) {
		SnekError(&compiler, "Unknown folder '%s'", folder);
		return false;
	}

	bool result = true;

	for (auto& de : std::filesystem::directory_iterator(folder)) {
		std::u8string dePathStr = de.path().u8string();
		const char* dePath = (const char*)dePathStr.c_str();
		if (de.is_directory() && recursive) {
			result = AddSourceFolder(compiler, dePath, extension, recursive) && result;
		}
		else
		{
			const char* fileExtension = GetExtensionFromPath(dePath);
			if (fileExtension && strcmp(fileExtension, extension) == 0)
			{
				result = AddFile(compiler, dePath) && result;
			}
		}
	}

	return result;
}

int build(int argc, char* argv[])
{
	const char* outPath = "a.exe";
	bool llvm = false;
	bool tcc = false;
	bool emcc = false;

	CGLCompiler compiler;
	compiler.init(OnCompilerMessage);

	bool result = true;

	//result = AddSourceFolder(compiler, LocalFilePath("cgl/"), "src", true) && result;

	for (int i = 0; i < argc; i++)
	{
		const char* arg = argv[i];
		if (strlen(arg) > 0 && arg[0] == '-')
		{
			if (strcmp(arg, OPT_BUILD_HELP) == 0)
			{
				printBuildHelp(' ');
				return 0;
			}
			else if (strcmp(arg, OPT_BUILD_OUTPATH) == 0)
			{
				if (i < argc - 1)
				{
					outPath = argv[++i];
				}
				else
				{
					SnekError(&compiler, "%s must be followed by a path", arg);
					result = false;
				}
			}
			else if (strcmp(arg, OPT_BUILD_LINKER_PATH) == 0)
			{
				if (i < argc - 1)
					compiler.addLinkerPath(argv[++i]);
				else
				{
					SnekError(&compiler, "%s must be followed by a path", arg);
					result = false;
				}
			}
			else if (strcmp(arg, OPT_BUILD_LINKER_FILE) == 0)
			{
				if (i < argc - 1)
					compiler.addLinkerFile(argv[++i], true);
				else
				{
					SnekError(&compiler, "%s must be followed by a path", arg);
					result = false;
				}
			}
			else if (strcmp(arg, OPT_BUILD_PRINT_IR) == 0)
				compiler.printIR = true;
			else if (strcmp(arg, OPT_BUILD_GEN_DEBUG_INFO) == 0)
				compiler.debugInfo = true;
			else if (strcmp(arg, OPT_BUILD_OPTIMIZE) == 0)
				compiler.optimization = 3;
			else if (strlen(arg) == 3 && arg[0] == '-' && arg[1] == 'O')
			{
				if (arg[2] >= '0' && arg[2] <= '3')
				{
					compiler.optimization = arg[2] - '0';
				}
				else
				{
					SnekError(&compiler, "Invalid optimization level: %d, should be 0-3", arg[2] - '0');
				}
			}
			else if (strcmp(arg, OPT_BUILD_STATIC_LIBRARY) == 0)
				compiler.staticLibrary = true;
			else if (strcmp(arg, OPT_BUILD_SHARED_LIBRARY) == 0)
				compiler.sharedLibrary = true;
			else if (strcmp(arg, OPT_BUILD_GEN_RUNTIME_STACKTRACE) == 0)
				compiler.runtimeStackTrace = true;
			else if (strcmp(arg, OPT_BUILD_BACKEND_CLANG) == 0)
				llvm = true;
			else if (strcmp(arg, OPT_BUILD_BACKEND_TCC) == 0)
				tcc = true;
			else if (strcmp(arg, OPT_BUILD_BACKEND_EMSCRIPTEN) == 0)
				emcc = true;
			else
			{
				SnekError(&compiler, "Unknown argument %s", arg);
				result = false;
			}
		}
		else
		{
			char* path = _strdup(argv[i]);
			const char* extension = GetExtensionFromPath(path);
			char* moduleName = GetModuleNameFromPath(path);
			if (strcmp(moduleName, "*") == 0)
			{
				char* folder = GetFolderFromPath(path);
				result = AddSourceFolder(compiler, folder, extension, false) && result;
				delete folder, moduleName;
			}
			else if (strcmp(moduleName, "**") == 0)
			{
				char* folder = GetFolderFromPath(path);
				result = AddSourceFolder(compiler, folder, extension, true) && result;
				delete folder, moduleName;
			}
			else
			{
				result = AddFile(compiler, path) && result;
			}
		}
	}

	if (result)
	{
		if (compiler.sourceFiles.size > 0)
		{
			fprintf(stderr, "Compiling source files\n");
			if (compiler.compile())
			{
				fprintf(stderr, "Generating code\n");
				if (llvm)
					result = compiler.outputLLVM(outPath) == 0;
				else if (emcc)
					result = compiler.outputEmscripten(outPath) == 0;
				else
					result = compiler.outputTCC(outPath) == 0;
			}
			else
			{
				result = false;
			}
		}
		else
		{
			SnekError(&compiler, "No input files");
			result = false;
		}
	}

	compiler.terminate();

	if (result)
		fprintf(stderr, "Build complete.\n");
	else
		fprintf(stderr, "Build finished with errors.\n");

	return result ? 0 : 1;
}

int run(int argc, char* argv[])
{
	bool llvm = false;
	bool tcc = false;

	CGLCompiler compiler;
	compiler.init(OnCompilerMessage);

	bool result = true;

	//result = AddSourceFolder(compiler, LocalFilePath("cgl/"), "src", true) && result;

	for (int i = 0; i < argc; i++)
	{
		const char* arg = argv[i];
		if (strlen(arg) > 0 && arg[0] == '-')
		{
			if (strcmp(arg, OPT_BUILD_HELP) == 0)
			{
				printRunHelp(' ');
				return 0;
			}
			else if (strcmp(arg, OPT_BUILD_LINKER_PATH) == 0)
			{
				if (i < argc - 1)
					compiler.addLinkerPath(argv[++i]);
				else
				{
					SnekError(&compiler, "%s must be followed by a path", arg);
					result = false;
				}
			}
			else if (strcmp(arg, OPT_BUILD_LINKER_FILE) == 0)
			{
				if (i < argc - 1)
					compiler.addLinkerFile(argv[++i], true);
				else
				{
					SnekError(&compiler, "%s must be followed by a path", arg);
					result = false;
				}
			}
			else if (strcmp(arg, OPT_BUILD_PRINT_IR) == 0)
				compiler.printIR = true;
			else if (strcmp(arg, OPT_BUILD_OPTIMIZE) == 0)
				compiler.optimization = 3;
			else if (strlen(arg) == 3 && arg[0] == '-' && arg[1] == 'O')
			{
				if (arg[2] >= '0' && arg[2] <= '3')
				{
					compiler.optimization = arg[2] - '0';
				}
				else
				{
					SnekError(&compiler, "Invalid optimization level: %d, should be 0-3", arg[2] - '0');
				}
			}
			else if (strcmp(arg, OPT_BUILD_GEN_RUNTIME_STACKTRACE) == 0)
				compiler.runtimeStackTrace = true;
			else if (strcmp(arg, OPT_BUILD_BACKEND_CLANG) == 0)
				llvm = true;
			else if (strcmp(arg, OPT_BUILD_BACKEND_TCC) == 0)
				tcc = true;
			else
			{
				SnekError(&compiler, "Unknown argument %s", arg);
				result = false;
			}
		}
		else
		{
			char* path = _strdup(argv[i]);
			const char* extension = GetExtensionFromPath(path);
			char* moduleName = GetModuleNameFromPath(path);
			if (strcmp(moduleName, "*") == 0)
			{
				char* folder = GetFolderFromPath(path);
				result = AddSourceFolder(compiler, folder, extension, false) && result;
				delete folder, moduleName;
			}
			else if (strcmp(moduleName, "**") == 0)
			{
				char* folder = GetFolderFromPath(path);
				result = AddSourceFolder(compiler, folder, extension, true) && result;
				delete folder, moduleName;
			}
			else
			{
				result = AddFile(compiler, path) && result;
			}
		}
	}

	if (result)
	{
		if (compiler.sourceFiles.size > 0)
		{
			fprintf(stderr, "Compiling source files\n");
			if (compiler.compile())
			{
				fprintf(stderr, "Running code\n");

				int argc = 0;
				char** argv = nullptr;
				int invokeResult = 0;

				if (llvm)
					invokeResult = compiler.runLLVM(argc, argv);
				else
					invokeResult = compiler.runTCC(argc, argv);

				fprintf(stderr, "Program exited with code %i\n", invokeResult);
				result = invokeResult == 0;
			}
			else
			{
				result = false;
			}
		}
		else
		{
			SnekError(&compiler, "No input files");
			result = false;
		}
	}

	compiler.terminate();

	return result ? 0 : 1;
}

int main(int argc, char* argv[])
{
	if (argc == 1 || argc >= 2 && strcmp(argv[1], "help") == 0)
	{
		printf("=== Sneklang Compiler version %d.%d.%d%s ===\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_SUFFIX);
		printf("Commands:\n");

		printBuildHelp('\t');
		printRunHelp('\t');

		printf("===\n");
	}
	else
	{
		const char* cmd = argv[1];
		if (strcmp(cmd, "build") == 0)
		{
			return build(argc - 2, &argv[2]);
		}
		else if (strcmp(cmd, "run") == 0)
		{
			return run(argc - 2, &argv[2]);
		}
		else
		{
			fprintf(stderr, "Unknown command '%s'", cmd);
			return 1;
		}
	}
}