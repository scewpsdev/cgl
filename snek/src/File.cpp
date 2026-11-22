#include "File.h"

#include "typechecker/TypeSystem.h"
#include "utils/Hash.h"


void initFile(File* file, const char* localPath, const char* path, const char* src, int length, GlobalBlockPool* blockPool)
{
	*file = {};

	file->handle = getFileHandle(localPath);
	file->localPath = _strdup(localPath);
	file->path = _strdup(path);
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

StringView getFileModuleName(File* file)
{
	const char* forwardSlash = strrchr(file->localPath, '/');
	const char* backwardSlash = strrchr(file->localPath, '\\');
	const char* slash = (forwardSlash && backwardSlash) ? (const char*)__max((uint64_t)forwardSlash, (uint64_t)backwardSlash) : forwardSlash ? forwardSlash : backwardSlash ? backwardSlash : NULL;
	if (slash)
		slash++;
	else
		slash = file->localPath;

	const char* dot = strrchr(slash, '.');
	if (!dot)
		dot = slash + strlen(slash);

	int length = (int)(dot - slash);

	return CreateString(slash, length);
}
