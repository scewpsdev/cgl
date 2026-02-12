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
		if (currentFile->moduleDecl)
			found = findFunctionsInModule(name, currentFile->moduleDecl->module, functions) || found;

		for (int i = 0; i < currentFile->dependencies.size; i++)
		{
			AST::Module* dependency = currentFile->dependencies[i];
			found = findFunctionsInModule(name, dependency, functions) || found;
		}

		found = findFunctionsInModule(name, globalModule, functions) || found;
	}

	return found;
}

AST::Function* Resolver::findUnaryOperatorOverloadInFile(TypeID operandType, AST::UnaryOperatorType operatorOverload, AST::File* file)
{
	for (int i = 0; i < file->functions.size; i++)
	{
		AST::Function* function = file->functions[i];
		if (function->visibility == AST::Visibility::Public || function->file == currentFile)
		{
			if (function->paramTypes.size > 0)
			{
				if (function->unaryOperator == operatorOverload && (!function->isGeneric && CompareTypes(function->paramTypes[0]->typeID, operandType) || DeduceGenericArg(function->paramTypes[0], operandType, function)))
				{
					return function;
				}
			}
		}
	}
	return nullptr;
}

AST::Function* Resolver::findUnaryOperatorOverloadInModule(TypeID operandType, AST::UnaryOperatorType operatorOverload, AST::Module* module)
{
	for (AST::File* file : module->files)
	{
		if (AST::Function* overload = findUnaryOperatorOverloadInFile(operandType, operatorOverload, file))
			return overload;
	}
	return nullptr;
}

AST::Function* Resolver::findUnaryOperatorOverload(TypeID operandType, AST::UnaryOperatorType operatorOverload)
{
	if (AST::Function* function = findUnaryOperatorOverloadInFile(operandType, operatorOverload, currentFile))
	{
		return function;
	}

	AST::Module* module = currentFile->moduleDecl ? currentFile->moduleDecl->module : globalModule;
	if (AST::Function* function = findUnaryOperatorOverloadInModule(operandType, operatorOverload, module))
	{
		return function;
	}

	for (int i = 0; i < currentFile->dependencies.size; i++)
	{
		AST::Module* dependency = currentFile->dependencies[i];
		if (AST::Function* function = findUnaryOperatorOverloadInModule(operandType, operatorOverload, dependency))
		{
			return function;
		}
	}

	return nullptr;
}

AST::Function* Resolver::findBinaryOperatorOverloadInFile(TypeID left, TypeID right, AST::BinaryOperatorType operatorOverload, AST::File* file)
{
	for (int i = 0; i < file->functions.size; i++)
	{
		AST::Function* function = file->functions[i];
		if (function->visibility == AST::Visibility::Public || function->file == currentFile)
		{
			if (function->paramTypes.size == 2)
			{
				if (function->binaryOperator == operatorOverload && (!function->isGeneric && CompareTypes(function->paramTypes[0]->typeID, left) && CompareTypes(function->paramTypes[1]->typeID, right) || DeduceGenericArg(function->paramTypes[0], left, function) && DeduceGenericArg(function->paramTypes[1], right, function)))
				{
					return function;
				}
			}
		}
	}
	return nullptr;
}

AST::Function* Resolver::findBinaryOperatorOverloadInModule(TypeID left, TypeID right, AST::BinaryOperatorType operatorOverload, AST::Module* module)
{
	for (AST::File* file : module->files)
	{
		if (AST::Function* overload = findBinaryOperatorOverloadInFile(left, right, operatorOverload, file))
			return overload;
	}
	return nullptr;
}

AST::Function* Resolver::findBinaryOperatorOverload(TypeID left, TypeID right, AST::BinaryOperatorType operatorOverload)
{
	if (AST::Function* function = findBinaryOperatorOverloadInFile(left, right, operatorOverload, currentFile))
	{
		return function;
	}

	AST::Module* module = currentFile->moduleDecl ? currentFile->moduleDecl->module : globalModule;
	if (AST::Function* function = findBinaryOperatorOverloadInModule(left, right, operatorOverload, module))
	{
		return function;
	}

	for (int i = 0; i < currentFile->dependencies.size; i++)
	{
		AST::Module* dependency = currentFile->dependencies[i];
		if (AST::Function* function = findBinaryOperatorOverloadInModule(left, right, operatorOverload, dependency))
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
			score += 10;
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
