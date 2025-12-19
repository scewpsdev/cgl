#include "File.h"

#include "cgl/Platform.h"

#include <stdio.h>


char* ReadText(const char* path)
{
	if (FILE* file = fopen(path, "rb"))
	{
		fseek(file, 0, SEEK_END);
		long numBytes = ftell(file);
		fseek(file, 0, SEEK_SET);

		char* buffer = new char[numBytes + 1];
		memset(buffer, 0, numBytes);
		numBytes = (long)fread(buffer, 1, numBytes, file);
		fclose(file);
		buffer[numBytes] = 0;

		return buffer;
	}
	return nullptr;
}

int WriteText(const char* txt, const char* path)
{
	if (FILE* file = fopen(path, "wb"))
	{
		int numBytes = fputs(txt, file);
		fclose(file);
		return numBytes;
	}
	return 0;
}

int WriteText(const char* txt, FILE* file)
{
	int numBytes = fputs(txt, file);
	fclose(file);
	return numBytes;
}

const char* LocalFilePath(const char* path)
{
	std::filesystem::path exePath = GetExecutablePath();
	std::filesystem::path localDir = exePath.parent_path();
	std::string localDirStr = localDir.string();

	static char result[512];
	strcpy(result, localDirStr.c_str());
	strcat(result, "/../../../cgl/");
	strcat(result, path);

	return result;
}

void CreateDirectories(const char* path)
{
	if (std::filesystem::is_directory(path))
		std::filesystem::create_directories(path);
	else
	{
		auto directory = std::filesystem::path(path).parent_path();
		if (!directory.empty())
			std::filesystem::create_directories(directory);
	}
}
