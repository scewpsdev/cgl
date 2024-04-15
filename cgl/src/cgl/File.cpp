#include "File.h"

#include "Platform.h"

#include <stdio.h>


char* ReadText(const char* path)
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
		std::filesystem::create_directories(std::filesystem::path(path).parent_path());
}
