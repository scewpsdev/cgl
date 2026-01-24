#pragma once

#include "Type.h"
#include "Declaration.h"
#include "cgl/utils/List.h"


struct SourceFile;

namespace AST
{
	struct Module;
	struct Expression;
	struct Scope;


	struct File
	{
		const char* name;
		int moduleID;
		SourceFile* sourceFile;

		Module* module = nullptr;
		char* nameSpace = nullptr;
		char* fullName = nullptr;
		char* fullIdentifier = nullptr;

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

		List<AST::Identifier*> identifiers;
		List<AST::NamedType*> namedTypes;


		File(const char* name, int moduleID, SourceFile* sourceFile);

		const char* getFullName();
		const char* getFullIdentifier();
	};
}
