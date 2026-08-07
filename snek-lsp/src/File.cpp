#include "File.h"

#include "typechecker/TypeSystem.h"
#include "utils/Hash.h"


void initFile(File* file, const char* localPath, GlobalBlockPool* blockPool)
{
	*file = {};

	file->handle = getFileHandle(localPath);
	file->internedTypes = nullptr;

	initArena(&file->arena, blockPool);
	initScratchBuffer(&file->scratch, 16);
	initDiagnostics(&file->diagnostics, &file->arena);
}

void addInternedType(File* file, uint64_t key)
{
	InternedType* interned = file->arena.alloc<InternedType>();
	interned->key = key;
	interned->next = file->internedTypes;
	file->internedTypes = interned;
}

void clearInternedTypes(File* file, TypeSystem* types)
{
	InternedType* interned = file->internedTypes;
	while (interned)
	{
		bool removed = removeInternedType(&types->typeTable, interned->key);
		//SnekAssert(removed);
		interned = interned->next;
	}
	file->internedTypes = nullptr;
}

FileHandle getFileHandle(const char* localPath)
{
	return (FileHandle)hashPath(localPath);
}
