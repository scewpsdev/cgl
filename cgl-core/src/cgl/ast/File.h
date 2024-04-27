#pragma once

#include "Type.h"
#include "Declaration.h"
#include "cgl/utils/List.h"


struct SourceFile;

namespace AST
{
	struct Module;
	struct Expression;


	struct File
	{
		const char* name;
		int moduleID;
		SourceFile* sourceFile;

		Module* module = nullptr;
		char* nameSpace = nullptr;

		ModuleDeclaration* moduleDecl = nullptr;
		NamespaceDeclaration* namespaceDecl = nullptr;

		List<Function*> functions;
		List<Struct*> structs;
		List<Class*> classes;
		List<Typedef*> typedefs;
		List<Enum*> enums;
		List<Macro*> macros;
		List<GlobalVariable*> globals;
		List<Import*> imports;

		List<Module*> dependencies;


		File(const char* name, int moduleID, SourceFile* sourceFile);
	};
}
