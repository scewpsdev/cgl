#include "pch.h"
#include "File.h"

#include "cgl/utils/Log.h"

#include <stdlib.h>


namespace AST
{
	File::File(const char* name, int moduleID, SourceFile* sourceFile)
		: name(name), moduleID(moduleID), sourceFile(sourceFile)
	{
		functions = CreateList<Function*>();
		structs = CreateList<Struct*>();
		classes = CreateList<Class*>();
		typedefs = CreateList<Typedef*>();
		enums = CreateList<Enum*>();
		exprdefs = CreateList<Exprdef*>();
		globals = CreateList<GlobalVariable*>();
		imports = CreateList<Import*>();

		dependencies = CreateList<Module*>();
	}
}