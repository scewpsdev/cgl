#include "cgl/CGLCompiler.h"

#include <stdio.h>


static const char* ReadText(const char* path)
{
	if (FILE* file = fopen(path, "r"))
	{
		fseek(file, 0, SEEK_END);
		long numBytes = ftell(file);
		fseek(file, 0, SEEK_SET);

		char* buffer = new char[numBytes];
		fread(buffer, 1, numBytes, file);
		fclose(file);

		return buffer;
	}
	return nullptr;
}

int main(int argc, char* argv[])
{
	CGLCompiler compiler;
	compiler.init();

	if (const char* src = ReadText("source.cgl"))
	{
		compiler.addFile(src);
	}

	compiler.compile();

	int result = compiler.run(0, nullptr);
	printf("Program exited with code %i\n", result);

	compiler.terminate();
}