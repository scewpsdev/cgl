#include "File.h"

#include "typechecker/TypeSystem.h"
#include "utils/Hash.h"


void initFile(File* file, const char* localPath, GlobalBlockPool* blockPool)
{
	*file = {};

	file->handle = getFileHandle(localPath);
	//file->internedTypes = nullptr;

	initArena(&file->arena, blockPool);
	initScratchBuffer(&file->scratch, 16);
	initDiagnostics(&file->diagnostics, &file->arena);
}

FileHandle getFileHandle(const char* localPath)
{
	return (FileHandle)hashPath(localPath);
}
