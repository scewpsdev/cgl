#include "pch.h"
#include "Function.h"

#include "Resolver.h"
#include "cgl/CGLCompiler.h"
#include "cgl/ast/File.h"
#include "cgl/ast/Declaration.h"
#include "cgl/ast/Module.h"


AST::Function* Resolver::findFunctionInFile(const char* name, AST::File* file, TypeID instanceType)
{
	for (int i = 0; i < file->functions.size; i++)
	{
		AST::Function* function = file->functions[i];
		if (function->visibility == AST::Visibility::Public || function->file == currentFile)
		{
			if (strcmp(function->name, name) == 0 && (!instanceType || function->paramTypes.size >= 1 && (CanConvertImplicit(instanceType, function->paramTypes[0]->typeID, false) || function->paramTypes[0]->typeKind == AST::TypeKind::Pointer && CompareTypes(instanceType, function->paramTypes[0]->typeID->pointerType.elementType))))
			{
				return function;
			}
		}
	}
	return nullptr;
}

AST::Function* Resolver::findFunctionInModule(const char* name, AST::Module* module, TypeID instanceType)
{
	for (AST::File* file : module->files)
	{
		if (AST::Function* function = findFunctionInFile(name, file, instanceType))
			return function;
	}
	return nullptr;
}

AST::Function* Resolver::findFunction(const char* name, TypeID instanceType)
{
	if (AST::Function* function = findFunctionInFile(name, currentFile, instanceType))
	{
		return function;
	}

	AST::Module* module = currentFile->moduleDecl ? currentFile->moduleDecl->module : globalModule;
	if (AST::Function* function = findFunctionInModule(name, module, instanceType))
	{
		return function;
	}

	for (int i = 0; i < currentFile->dependencies.size; i++)
	{
		AST::Module* dependency = currentFile->dependencies[i];
		if (AST::Function* function = findFunctionInModule(name, dependency, instanceType))
		{
			return function;
		}
	}

	return nullptr;
}

bool Resolver::findFunctionsInFile(const char* name, AST::File* file, List<AST::Function*>& functions)
{
	bool found = false;

	for (int i = 0; i < file->functions.size; i++)
	{
		AST::Function* function = file->functions[i];
		if (function->visibility == AST::Visibility::Public || function->file == currentFile)
		{
			if (strcmp(function->name, name) == 0)
			{
				functions.add(function);
				found = true;
			}
		}
	}

	return found;
}

bool Resolver::findFunctionsInModule(const char* name, AST::Module* module, List<AST::Function*>& functions)
{
	//if (AST::File* file = module->file)
	bool found = false;
	for (AST::File* file : module->files)
	{
		if (findFunctionsInFile(name, file, functions))
			found = true;
	}
	return found;
}

bool Resolver::findFunctions(const char* name, List<AST::Function*>& functions)
{
	bool found = false;

	found = findFunctionsInFile(name, currentFile, functions) || found;

	if (!found)
	{
		AST::Module* module = currentFile->moduleDecl ? currentFile->moduleDecl->module : globalModule;
		found = findFunctionsInModule(name, module, functions) || found;

		for (int i = 0; i < currentFile->dependencies.size; i++)
		{
			AST::Module* dependency = currentFile->dependencies[i];
			found = findFunctionsInModule(name, dependency, functions) || found;
		}
	}

	return found;
}

AST::Function* Resolver::findOperatorOverloadInFile(TypeID operandType, AST::OperatorOverload operatorOverload, AST::File* file)
{
	for (int i = 0; i < file->functions.size; i++)
	{
		AST::Function* function = file->functions[i];
		if (function->visibility == AST::Visibility::Public || function->file == currentFile)
		{
			if (function->paramTypes.size > 0)
			{
				if (function->operatorOverload == operatorOverload && (!function->isGeneric && CompareTypes(function->paramTypes[0]->typeID, operandType) || DeduceGenericArg(function->paramTypes[0], operandType, function)))
				{
					return function;
				}
			}
		}
	}
	return nullptr;
}

AST::Function* Resolver::findOperatorOverloadInModule(TypeID operandType, AST::OperatorOverload operatorOverload, AST::Module* module)
{
	for (AST::File* file : module->files)
	{
		return findOperatorOverloadInFile(operandType, operatorOverload, file);
	}
	return nullptr;
}

AST::Function* Resolver::findOperatorOverload(TypeID operandType, AST::OperatorOverload operatorOverload)
{
	if (AST::Function* function = findOperatorOverloadInFile(operandType, operatorOverload, currentFile))
	{
		return function;
	}

	AST::Module* module = currentFile->moduleDecl ? currentFile->moduleDecl->module : globalModule;
	if (AST::Function* function = findOperatorOverloadInModule(operandType, operatorOverload, module))
	{
		return function;
	}

	for (int i = 0; i < currentFile->dependencies.size; i++)
	{
		AST::Module* dependency = currentFile->dependencies[i];
		if (AST::Function* function = findOperatorOverloadInModule(operandType, operatorOverload, dependency))
		{
			return function;
		}
	}

	return nullptr;
}

int Resolver::getFunctionOverloadScore(const AST::Function* function, const List<AST::Expression*>& arguments)
{
	if (arguments.size != function->paramTypes.size)
		return 1000;

	int score = 0;

	for (int i = 0; i < arguments.size; i++)
	{
		if (function->paramTypes[i]->typeID && CompareTypes(arguments[i]->valueType, function->paramTypes[i]->typeID))
			;
		else if (function->isGeneric && !function->paramTypes[i]->typeID)
		{
			if (function->isGenericParam(i, arguments[i]->valueType))
				score++;
			else
			{
				context->disableError = true;
				if (!ResolveType(this, function->paramTypes[i]))
					score = 1000;
				context->disableError = false;
			}
		}
		else if (CanConvertImplicit(arguments[i]->valueType, function->paramTypes[i]->typeID, arguments[i]->isConstant()))
			score += 2;
		else
			score = 1000;
	}

	if (!isFunctionVisible(function, currentFile->module))
		score += arguments.size * 2 + 1;

	return score;
}

void Resolver::chooseFunctionOverload(List<AST::Function*>& functions, const List<AST::Expression*>& arguments, AST::Expression* methodInstance)
{
	if (functions.size <= 1)
		return;

	static const List<AST::Expression*>* argumentsRef = nullptr;
	argumentsRef = &arguments;

	static Resolver* resolver = nullptr;
	resolver = this;

	functions.sort([](AST::Function* const* function1, AST::Function* const* function2) -> int
		{
			const List<AST::Expression*>& arguments = *argumentsRef;

			int score1 = resolver->getFunctionOverloadScore(*function1, arguments);
			int score2 = resolver->getFunctionOverloadScore(*function2, arguments);

			return score1 < score2 ? -1 : score1 > score2 ? 1 : 0;
		});

	for (int i = functions.size - 1; i >= 0; i--)
	{
		if (getFunctionOverloadScore(functions[i], arguments) == INT32_MAX)
		{
			functions.removeAt(i);
		}
	}
}
