#include "pch.h"
#include "Variable.h"

#include "Resolver.h"
#include "Mangling.h"

#include "cgl/utils/Log.h"


Variable::Variable(AST::File* file, char* name, char* mangledName, TypeID type, AST::Expression* value, bool isConstant, AST::Visibility visibility)
	: file(file), name(name), mangledName(mangledName), type(type), value(value), isConstant(isConstant), visibility(visibility)
{
}

Variable::~Variable()
{
	if (name)
		delete name;
	if (mangledName)
		delete mangledName;
	if (value)
		delete value;
}

void Resolver::registerLocalVariable(Variable* variable, AST::Element* declaration)
{
	variable->declaration = declaration;
	scope->localVariables.add(variable);
}

void Resolver::registerGlobalVariable(Variable* variable, AST::GlobalVariable* global)
{
	variable->declaration = global;
}

TypeID Resolver::getGenericTypeArgument(const char* name)
{
	if (currentFunction)
	{
		if (TypeID type = currentFunction->getGenericTypeArgument(name))
			return type;
	}
	if (currentStruct)
	{
		if (TypeID type = currentStruct->getGenericTypeArgument(name))
			return type;
	}
	if (currentClass)
	{
		if (TypeID type = currentClass->getGenericTypeArgument(name))
			return type;
	}
	return nullptr;
}

Variable* Resolver::findLocalVariableInScope(const char* name, Scope* scope, bool recursive)
{
	if (scope == globalScope)
		return nullptr;
	// iterate backwards to improve error messages when shadowing variable names
	for (int i = scope->localVariables.size - 1; i >= 0; i--)
	{
		Variable* variable = scope->localVariables[i];
		if (strcmp(variable->name, name) == 0)
		{
			return variable;
		}
	}
	if (recursive && scope->parent != globalScope)
	{
		return findLocalVariableInScope(name, scope->parent, recursive);
	}

	return nullptr;
}

Variable* Resolver::findGlobalVariableInFile(const char* name, AST::File* file)
{
	for (int i = 0; i < file->globals.size; i++)
	{
		for (int j = 0; j < file->globals[i]->declarators.size; j++)
		{
			Variable* variable = file->globals[i]->declarators[j]->variable;
			if (strcmp(variable->name, name) == 0)
			{
				return variable;
			}
		}
	}
	return nullptr;
}

Variable* Resolver::findGlobalVariableInModule(const char* name, AST::Module* module, AST::Module* current)
{
	for (AST::File* file : module->files)
	{
		if (Variable* variable = findGlobalVariableInFile(name, file))
		{
			if (variable->visibility >= AST::Visibility::Public || current == module)
				return variable;
			else
			{
				SnekErrorLoc(context, currentElement->location, "Variable '%s' is not visible", variable->name);
				return nullptr;
			}
		}
	}
	return nullptr;
}

Variable* Resolver::findVariable(const char* name)
{
	if (Variable* variable = findLocalVariableInScope(name, scope, true))
		return variable;

	if (Variable* variable = findGlobalVariableInFile(name, currentFile))
		return variable;

	AST::Module* module = currentFile->moduleDecl ? currentFile->moduleDecl->module : globalModule;
	if (Variable* variable = findGlobalVariableInModule(name, module, module))
		return variable;

	/*
	//if (AST::File* file = module->file)
	for (AST::File* file : module->files)
	{
		if (file != currentFile)
		{
			if (Variable* variable = findGlobalVariableInFile(name, file))
			{
				if (variable->visibility >= AST::Visibility::Public)
					return variable;
				else
				{
					SnekErrorLoc(context, currentElement->location, "Variable '%s' is not visible", variable->name);
					return nullptr;
				}
			}
		}
	}
	*/

	for (int i = 0; i < currentFile->dependencies.size; i++)
	{
		AST::Module* dependency = currentFile->dependencies[i];
		if (Variable* variable = findGlobalVariableInModule(name, dependency, module))
			return variable;

		/*
		//if (AST::File* file = dependency->file)
		for (AST::File* file : dependency->files)
		{
			if (Variable* variable = findGlobalVariableInFile(name, file))
			{
				if (variable->visibility >= AST::Visibility::Public)
					return variable;
				else
				{
					SnekErrorLoc(context, currentElement->location, "Variable '%s' is not visible", variable->name);
					return nullptr;
				}
			}
		}
		*/
	}
	return nullptr;
}

AST::EnumValue* FindEnumValueInFile(Resolver* resolver, AST::File* file, const char* name)
{
	for (int i = 0; i < file->enums.size; i++)
	{
		AST::Enum* enumDecl = file->enums[i];
		for (int j = 0; j < enumDecl->values.size; j++)
		{
			if (strcmp(enumDecl->values[j]->name, name) == 0)
			{
				return enumDecl->values[j];
			}
		}
	}
	return nullptr;
}

AST::EnumValue* FindEnumValueInNamespace(Resolver* resolver, AST::Module* module, const char* name, AST::Module* currentNamespace)
{
	//if (AST::File* file = module->file)
	for (AST::File* file : module->files)
	{
		if (AST::EnumValue* enumValue = FindEnumValueInFile(resolver, file, name))
		{
			if (enumValue->declaration->visibility >= AST::Visibility::Public || currentNamespace == module)
				return enumValue;
			else
			{
				SnekErrorLoc(resolver->context, resolver->currentElement->location, "Enum '%s' containing value '%s' is not visible", enumValue->declaration->name, enumValue->name);
				return nullptr;
			}
		}
	}
	return nullptr;
}

AST::EnumValue* FindEnumValue(Resolver* resolver, const char* name)
{
	if (AST::EnumValue* enumValue = FindEnumValueInFile(resolver, resolver->currentFile, name))
	{
		return enumValue;
	}

	AST::Module* module = resolver->currentFile->moduleDecl ? resolver->currentFile->moduleDecl->module : resolver->globalModule;
	if (AST::EnumValue* enumValue = FindEnumValueInNamespace(resolver, module, name, module))
	{
		return enumValue;
	}
	for (int i = 0; i < resolver->currentFile->dependencies.size; i++)
	{
		AST::Module* dependency = resolver->currentFile->dependencies[i];
		if (AST::EnumValue* enumValue = FindEnumValueInNamespace(resolver, dependency, name, module))
		{
			return enumValue;
		}
	}
	return NULL;
}

static AST::Struct* FindStructInFile(Resolver* resolver, AST::File* module, const char* name)
{
	for (int i = 0; i < module->structs.size; i++)
	{
		AST::Struct* str = module->structs[i];
		if (strcmp(str->name, name) == 0)
		{
			return str;
		}
	}
	return nullptr;
}

AST::Struct* FindStruct(Resolver* resolver, const char* name)
{
	if (AST::Struct* str = FindStructInFile(resolver, resolver->currentFile, name))
	{
		return str;
	}

	AST::Module* module = resolver->currentFile->moduleDecl ? resolver->currentFile->moduleDecl->module : resolver->globalModule;
	//if (AST::File* file = module->file)
	for (AST::File* file : module->files)
	{
		if (AST::Struct* str = FindStructInFile(resolver, file, name))
		{
			if (str->visibility >= AST::Visibility::Public)
				return str;
			else
			{
				SnekErrorLoc(resolver->context, resolver->currentElement->location, "Struct '%s' is not visible", str->name);
				return nullptr;
			}
		}
	}
	for (int i = 0; i < resolver->currentFile->dependencies.size; i++)
	{
		AST::Module* dependency = resolver->currentFile->dependencies[i];
		//if (AST::File* file = dependency->file)
		for (AST::File* file : dependency->files)
		{
			if (AST::Struct* str = FindStructInFile(resolver, file, name))
			{
				if (str->visibility >= AST::Visibility::Public)
					return str;
				else
				{
					SnekErrorLoc(resolver->context, resolver->currentElement->location, "Struct '%s' is not visible", str->name);
					return nullptr;
				}
			}
		}
	}
	return nullptr;
}

static AST::Class* FindClassInFile(Resolver* resolver, AST::File* module, const char* name)
{
	for (int i = 0; i < module->classes.size; i++)
	{
		AST::Class* str = module->classes[i];
		if (strcmp(str->name, name) == 0)
		{
			return str;
		}
	}
	return nullptr;
}

AST::Class* FindClass(Resolver* resolver, const char* name)
{
	if (AST::Class* clss = FindClassInFile(resolver, resolver->currentFile, name))
	{
		return clss;
	}

	AST::Module* module = resolver->currentFile->moduleDecl ? resolver->currentFile->moduleDecl->module : resolver->globalModule;
	//if (AST::File* file = module->file)
	for (AST::File* file : module->files)
	{
		if (AST::Class* clss = FindClassInFile(resolver, file, name))
		{
			if (clss->visibility >= AST::Visibility::Public)
				return clss;
			else
			{
				SnekErrorLoc(resolver->context, resolver->currentElement->location, "Class '%s' is not visible", clss->name);
				return nullptr;
			}
		}
	}
	for (int i = 0; i < resolver->currentFile->dependencies.size; i++)
	{
		AST::Module* dependency = resolver->currentFile->dependencies[i];
		//if (AST::File* file = module->file)
		for (AST::File* file : dependency->files)
		{
			if (AST::Class* clss = FindClassInFile(resolver, file, name))
			{
				if (clss->visibility >= AST::Visibility::Public)
					return clss;
				else
				{
					SnekErrorLoc(resolver->context, resolver->currentElement->location, "Class '%s' is not visible", clss->name);
					return nullptr;
				}
			}
		}
	}
	return nullptr;
}

static AST::Typedef* FindTypedefInFile(Resolver* resolver, AST::File* module, const char* name)
{
	for (int i = 0; i < module->typedefs.size; i++)
	{
		AST::Typedef* td = module->typedefs[i];
		if (strcmp(td->name, name) == 0 && td->type)
		{
			return td;
		}
	}
	return nullptr;
}

AST::Typedef* FindTypedef(Resolver* resolver, const char* name)
{
	if (AST::Typedef* td = FindTypedefInFile(resolver, resolver->currentFile, name))
	{
		return td;
	}

	AST::Module* module = resolver->currentFile->moduleDecl ? resolver->currentFile->moduleDecl->module : resolver->globalModule;
	//if (AST::File* file = module->file)
	for (AST::File* file : module->files)
	{
		if (AST::Typedef* td = FindTypedefInFile(resolver, file, name))
		{
			if (td->visibility >= AST::Visibility::Public)
				return td;
			else
			{
				SnekErrorLoc(resolver->context, resolver->currentElement->location, "Typedef '%s' is not visible", td->name);
				return nullptr;
			}
		}
	}
	for (int i = 0; i < resolver->currentFile->dependencies.size; i++)
	{
		AST::Module* dependency = resolver->currentFile->dependencies[i];
		//if (AST::File* file = dependency->file)
		for (AST::File* file : dependency->files)
		{
			if (AST::Typedef* td = FindTypedefInFile(resolver, file, name))
			{
				if (td->visibility >= AST::Visibility::Public)
					return td;
				else
				{
					SnekErrorLoc(resolver->context, resolver->currentElement->location, "Typedef '%s' is not visible", td->name);
					return nullptr;
				}
			}
		}
	}
	return nullptr;
}

static AST::Enum* FindEnumInFile(Resolver* resolver, AST::File* module, const char* name)
{
	for (int i = 0; i < module->enums.size; i++)
	{
		AST::Enum* en = module->enums[i];
		if (strcmp(en->name, name) == 0)
		{
			return en;
		}
	}
	return nullptr;
}

AST::Enum* FindEnum(Resolver* resolver, const char* name)
{
	if (AST::Enum* en = FindEnumInFile(resolver, resolver->currentFile, name))
	{
		return en;
	}

	AST::Module* module = resolver->currentFile->moduleDecl ? resolver->currentFile->moduleDecl->module : resolver->globalModule;
	//if (AST::File* file = module->file)
	for (AST::File* file : module->files)
	{
		if (AST::Enum* en = FindEnumInFile(resolver, file, name))
		{
			if (en->visibility >= AST::Visibility::Public)
				return en;
			else
			{
				SnekErrorLoc(resolver->context, resolver->currentElement->location, "Enum '%s' is not visible", en->name);
				return nullptr;
			}
		}
	}
	for (int i = 0; i < resolver->currentFile->dependencies.size; i++)
	{
		AST::Module* dependency = resolver->currentFile->dependencies[i];
		//if (AST::File* file = dependency->file)
		for (AST::File* file : dependency->files)
		{
			if (AST::Enum* en = FindEnumInFile(resolver, file, name))
			{
				if (en->visibility >= AST::Visibility::Public)
					return en;
				else
				{
					SnekErrorLoc(resolver->context, resolver->currentElement->location, "Enum '%s' is not visible", en->name);
					return nullptr;
				}
			}
		}
	}
	return nullptr;
}

static AST::Macro* FindMacroInFile(Resolver* resolver, AST::File* module, const char* name)
{
	for (int i = 0; i < module->macros.size; i++)
	{
		AST::Macro* ed = module->macros[i];
		if (strcmp(ed->name, name) == 0)
		{
			return ed;
		}
	}
	return nullptr;
}

AST::Macro* FindMacro(Resolver* resolver, const char* name)
{
	if (AST::Macro* ed = FindMacroInFile(resolver, resolver->currentFile, name))
	{
		return ed;
	}

	AST::Module* module = resolver->currentFile->moduleDecl ? resolver->currentFile->moduleDecl->module : resolver->globalModule;
	//if (AST::File* file = module->file)
	for (AST::File* file : module->files)
	{
		if (AST::Macro* ed = FindMacroInFile(resolver, file, name))
		{
			if (ed->visibility >= AST::Visibility::Public)
				return ed;
			else
			{
				SnekErrorLoc(resolver->context, resolver->currentElement->location, "Macro '%s' is not visible", ed->name);
				return nullptr;
			}
		}
	}
	for (int i = 0; i < resolver->currentFile->dependencies.size; i++)
	{
		AST::Module* dependency = resolver->currentFile->dependencies[i];
		//if (AST::File* file = dependency->file)
		for (AST::File* file : dependency->files)
		{
			if (AST::Macro* ed = FindMacroInFile(resolver, file, name))
			{
				if (ed->visibility >= AST::Visibility::Public)
					return ed;
				else
				{
					SnekErrorLoc(resolver->context, resolver->currentElement->location, "Macro '%s' is not visible", ed->name);
					return nullptr;
				}
			}
		}
	}
	return nullptr;
}
