#include "pch.h"
#include "File.h"
#include "Module.h"

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
		macros = CreateList<Macro*>();
		globals = CreateList<GlobalVariable*>();
		imports = CreateList<Import*>();

		dependencies = CreateList<Module*>();
	}

	const char* File::getFullName()
	{
		if (fullName)
			return fullName;
		std::string str = "";
		Module* m = module;
		while (m->parent)
		{
			str = m->name + (str != "" ? std::string(".") + str : "");
			m = m->parent;
		}
		if (strcmp(module->name, name) != 0)
			str = (str != "" ? str + std::string(".") : "") + name;
		fullName = _strdup(str.c_str());
		return fullName;
	}

	const char* File::getFullIdentifier()
	{
		if (fullIdentifier)
			return fullIdentifier;
		fullIdentifier = _strdup(getFullName());
		for (int i = 0; i < (int)strlen(fullIdentifier); i++)
		{
			if (fullIdentifier[i] == '.')
				fullIdentifier[i] = '_';
		}
		return fullIdentifier;
	}
}