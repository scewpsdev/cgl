#include "pch.h"
#include "Mangling.h"

#include "cgl/semantics/Resolver.h"
#include "cgl/ast/Declaration.h"
#include "cgl/ast/File.h"
#include "cgl/ast/Module.h"

#include "cgl/utils/Stringbuffer.h"
#include "cgl/utils/Log.h"
#include "cgl/utils/Hash.h"

#include <string.h>


static void MangleType(TypeID type, StringBuffer& buffer)
{
	if (type->typeKind == AST::TypeKind::Struct && type->structType.declaration && type->structType.declaration->isGenericInstance)
	{
		char* mangledName = MangleStructName(type->structType.declaration);
		StringBufferAppend(buffer, mangledName);
		delete mangledName;
	}
	else
	{
		switch (type->typeKind)
		{
		case AST::TypeKind::Void:
			StringBufferAppend(buffer, 'v');
			break;
		case AST::TypeKind::Integer:
			StringBufferAppend(buffer, type->integerType.isSigned ? 'i' : 'u');
			StringBufferAppend(buffer, type->integerType.bitWidth);
			break;
		case AST::TypeKind::FloatingPoint:
			StringBufferAppend(buffer, 'f');
			StringBufferAppend(buffer, (int)type->fpType.precision);
			break;
		case AST::TypeKind::Boolean:
			StringBufferAppend(buffer, 'b');
			break;
		case AST::TypeKind::String:
			StringBufferAppend(buffer, 's');
			break;
		case AST::TypeKind::Any:
			StringBufferAppend(buffer, 'y');
			break;
		case AST::TypeKind::Struct:
			StringBufferAppend(buffer, 'x');
			StringBufferAppend(buffer, (uint16_t)hash(type->structType.name));
			break;
		case AST::TypeKind::Class:
			StringBufferAppend(buffer, 'X');
			StringBufferAppend(buffer, (uint16_t)hash(type->classType.name));
			break;
		case AST::TypeKind::Alias:
			MangleType(type->aliasType.alias, buffer);
			break;
		case AST::TypeKind::Pointer:
			StringBufferAppend(buffer, 'p');
			MangleType(type->pointerType.elementType, buffer);
			break;
		case AST::TypeKind::Function:
			StringBufferAppend(buffer, 'f');
			MangleType(type->functionType.returnType, buffer);
			StringBufferAppend(buffer, type->functionType.numParams);
			for (int i = 0; i < type->functionType.numParams; i++)
				MangleType(type->functionType.paramTypes[i], buffer);
			break;
		case AST::TypeKind::Array:
			StringBufferAppend(buffer, 'a');
			MangleType(type->arrayType.elementType, buffer);
			break;
		default:
			SnekAssert(false);
			break;
		}
	}
}

static void AppendModuleName(AST::Module* module, StringBuffer& buffer)
{
	if (module->parent->parent) // If parent is not the global module
	{
		AppendModuleName(module->parent, buffer);
		StringBufferAppend(buffer, "_");
	}
	StringBufferAppend(buffer, module->name);
}

char* MangleFunctionName(AST::Function* function)
{
	bool isExtern = HasFlag(function->flags, AST::DeclarationFlags::Extern);

	if (function->isEntryPoint)
	{
		return _strdup("main");
	}
	else if (isExtern || function->dllImport || HasFlag(function->flags, AST::DeclarationFlags::DllExport))
	{
		return _strdup(function->name);
	}
	else if (function->unaryOperator != AST::UnaryOperatorType::Null)
	{
		if (function->unaryOperator == AST::UnaryOperatorType::Subscript)
			return _strdup("__operator_subscript");
		SnekAssert(false);
		return nullptr;
	}
	else if (function->binaryOperator != AST::BinaryOperatorType::Null)
	{
		if (function->binaryOperator == AST::BinaryOperatorType::Add)
			return _strdup("__operator_add");
		if (function->binaryOperator == AST::BinaryOperatorType::Subtract)
			return _strdup("__operator_sub");
		if (function->binaryOperator == AST::BinaryOperatorType::Multiply)
			return _strdup("__operator_mul");
		if (function->binaryOperator == AST::BinaryOperatorType::Divide)
			return _strdup("__operator_div");
		if (function->binaryOperator == AST::BinaryOperatorType::Modulo)
			return _strdup("__operator_mod");
		SnekAssert(false);
		return nullptr;
	}
	else
	{
		StringBuffer result = CreateStringBuffer(4);
		AST::Module* module = function->file->module;
		if (module->parent)
		{
			AppendModuleName(module, result);
			StringBufferAppend(result, "__");
		}
		StringBufferAppend(result, function->name);

		if (function->paramTypes.size > 0)
		{
			StringBufferAppend(result, '_');
			StringBufferAppend(result, function->paramTypes.size);
			for (int i = 0; i < function->paramTypes.size; i++)
			{
				MangleType(function->paramTypes[i]->typeID, result);
			}
		}

		if (function->varArgs)
		{
			StringBufferAppend(result, "_v");
			MangleType(function->varArgsType, result);
		}

		if (function->isGenericInstance)
		{
			for (int i = 0; i < function->genericTypeArguments.size; i++)
			{
				StringBufferAppend(result, '_');
				MangleType(function->genericTypeArguments[i], result);
			}
		}

		return result.buffer;
	}
}

char* MangleGlobalName(AST::GlobalVariable* variable, AST::VariableDeclarator* declarator)
{
	bool isExtern = HasFlag(variable->flags, AST::DeclarationFlags::Extern);

	if (isExtern)
	{
		return _strdup(declarator->name);
	}
	else
	{
		StringBuffer result = CreateStringBuffer(4);
		AST::Module* module = variable->file->module;
		if (module->parent)
		{
			AppendModuleName(module, result);
			StringBufferAppend(result, "__");
		}
		StringBufferAppend(result, declarator->name);

		return result.buffer;
	}
}

char* MangleStructName(AST::Struct* str)
{
	StringBuffer buffer = CreateStringBuffer(4);

	StringBufferAppend(buffer, str->name);

	if (str && str->isGenericInstance)
	{
		for (int i = 0; i < str->genericTypeArguments.size; i++)
		{
			StringBufferAppend(buffer, '_');
			MangleType(str->genericTypeArguments[i], buffer);
		}
	}

	return buffer.buffer;
}

char* MangleClassName(AST::Class* cls)
{
	StringBuffer buffer = CreateStringBuffer(4);

	StringBufferAppend(buffer, cls->name);

	if (cls && cls->isGenericInstance)
	{
		for (int i = 0; i < cls->genericTypeArguments.size; i++)
		{
			StringBufferAppend(buffer, '_');
			MangleType(cls->genericTypeArguments[i], buffer);
		}
	}

	return buffer.buffer;
}
