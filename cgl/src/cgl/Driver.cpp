#include "cgl/CGLCompiler.h"

#include <stdio.h>
#include <stdarg.h>

#include <filesystem>


static void OnCompilerMessage(MessageType msgType, const char* filename, int line, int col, int errCode, const char* msg, ...)
{
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

#if _DEBUG
		fprintf(stderr, "%s\n", message);
#else
		compilerMessages << message << std::endl;
#endif
	}
}

static char* ReadText(const char* path)
{
	if (FILE* file = fopen(path, "r"))
	{
		fseek(file, 0, SEEK_END);
		long numBytes = ftell(file);
		fseek(file, 0, SEEK_SET);

		char* buffer = new char[numBytes + 1];
		memset(buffer, 0, numBytes);
		fread(buffer, 1, numBytes, file);
		fclose(file);
		buffer[numBytes] = 0;

		return buffer;
	}
	return nullptr;
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

	char* folder = new char[length + 1];
	strncpy(folder, path, length);
	folder[length] = 0;

	return folder;
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
	//SnekAddLinkerFile(context, path);
}

static bool AddFile(CGLCompiler& compiler, const char* path)
{
	const char* fileExtension = GetExtensionFromPath(path);
	if (strcmp(fileExtension, "obj") == 0 || strcmp(fileExtension, "o") == 0 || strcmp(fileExtension, "lib") == 0 || strcmp(fileExtension, "a") == 0)
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
			SnekFatal(&compiler, ERROR_CODE_FILE_NOT_FOUND, "Unknown file '%s'", path);
			return false;
		}
	}
}

static bool AddSourceFolder(CGLCompiler& compiler, const char* folder, const char* extension, bool recursive)
{
	if (!std::filesystem::exists(folder)) {
		SnekFatal(&compiler, ERROR_CODE_FOLDER_NOT_FOUND, "Unknown folder '%s'", folder);
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

int main(int argc, char* argv[])
{
	CGLCompiler compiler;
	compiler.init(OnCompilerMessage);

	bool result = true;

	List<const char*> inputFiles;

	for (int i = 1; i < argc; i++)
	{
		const char* arg = argv[i];
		if (strcmp(arg, "-o") == 0);
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

	for (const char* inputFile : inputFiles)
	{
		if (const char* src = ReadText(inputFile))
		{
			compiler.addFile(inputFile, GetModuleNameFromPath(inputFile), src);
		}
	}

	if (compiler.compile())
	{
		int result = compiler.run(0, nullptr);
		printf("Program exited with code %i\n", result);
	}

	compiler.terminate();
}