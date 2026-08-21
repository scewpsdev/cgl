#include "File.h"

#include "typechecker/TypeSystem.h"
#include "utils/Hash.h"


void initFile(File* file, const char* localPath, const char* src, int length, GlobalBlockPool* blockPool)
{
	*file = {};

	file->handle = getFileHandle(localPath);
	file->localPath = _strdup(localPath);
	//file->internedTypes = nullptr;

	initArena(&file->arena, blockPool);
	initScratchBuffer(&file->scratch, 16);
	initDiagnostics(&file->diagnostics, &file->arena);

	file->src = _strdup(src);
	file->length = length;
	file->ast = {};

	file->lineOffsets.add(0);
	for (int i = 0; i < length; i++)
	{
		if (src[i] == '\n')
			file->lineOffsets.add(i + 1);
	}
}

void destroyFile(File* file)
{
	free(file->localPath);

	destroyArena(&file->arena);
	destroyScratchBuffer(&file->scratch);
	destroyDiagnostics(&file->diagnostics);

	free(file);
}

FileHandle getFileHandle(const char* localPath)
{
	return (FileHandle)hashPath(localPath);
}
