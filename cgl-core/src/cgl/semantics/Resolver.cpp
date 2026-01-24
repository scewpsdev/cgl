#include "pch.h"
#include "Resolver.h"

#include "Variable.h"
#include "Type.h"
#include "Mangling.h"

#include "cgl/parser/lexer.h"
#include "cgl/utils/Utils.h"
#include "cgl/utils/Log.h"
#include "cgl/CGLCompiler.h"

#include <math.h>
#include <stddef.h>


static AST::Module* CreateModule(Resolver* resolver, const char* name, AST::Module* parent)
{
	AST::Module* module = new AST::Module(name, parent);

	if (parent)
		parent->children.add(module);

	return module;
}

static AST::Module* FindModule(Resolver* resolver, const char* name, AST::Module* parent)
{
	for (int i = 0; i < parent->children.size; i++)
	{
		if (strcmp(name, parent->children[i]->name) == 0)
			return parent->children[i];
	}
	return nullptr;
}

static AST::Module* FindModuleInDependencies(Resolver* resolver, const char* name, AST::File* file)
{
	for (int i = 0; i < file->dependencies.size; i++)
	{
		if (strcmp(name, file->dependencies[i]->name) == 0)
			return file->dependencies[i];
	}
	return nullptr;
}

static AST::File* FindFileWithNamespace(Resolver* resolver, const char* name, AST::File* parent)
{
	for (int i = 0; i < parent->dependencies.size; i++)
	{
		AST::Module* dependency = parent->dependencies[i];
		for (AST::File* file : dependency->files)
		{
			if (file->nameSpace && strcmp(file->nameSpace, name) == 0)
				return file;
		}
	}
	return nullptr;
}

bool ResolveType(Resolver* resolver, AST::Type* type);
static bool ResolveExpression(Resolver* resolver, AST::Expression* expr);
static bool ResolveFunctionHeader(Resolver* resolver, AST::Function* decl);
static bool ResolveFunction(Resolver* resolver, AST::Function* decl);
static bool ResolveStructHeader(Resolver* resolver, AST::Struct* decl);
static bool ResolveStruct(Resolver* resolver, AST::Struct* decl);
static bool ResolveClassHeader(Resolver* resolver, AST::Class* decl);
static bool ResolveClass(Resolver* resolver, AST::Class* decl);

static const char* SourceLocationToString(AST::SourceLocation location)
{
	static char str[512];
	sprintf(str, "%s:%d:%d", location.filename, location.line, location.col);
	return str;
}

static bool IsPrimitiveType(AST::TypeKind typeKind)
{
	return typeKind == AST::TypeKind::Integer
		|| typeKind == AST::TypeKind::FloatingPoint
		|| typeKind == AST::TypeKind::Boolean;
}

static int64_t ConstantFoldInt(Resolver* resolver, AST::Expression* expr, bool& success)
{
	switch (expr->type)
	{
	case AST::ExpressionType::IntegerLiteral:
		return ((AST::IntegerLiteral*)expr)->value;
	case AST::ExpressionType::Identifier:
	{
		Variable* variable = ((AST::Identifier*)expr)->variable;
		if (!variable)
		{
			SnekErrorLoc(resolver->context, expr->location, "Variable '%s' must be defined before using it to initialize a constant", ((AST::Identifier*)expr)->name);
			success = false;
			return 0;
		}
		else
		{
			SnekAssert(variable->isConstant);
			return ConstantFoldInt(resolver, variable->value, success);
		}
	}
	case AST::ExpressionType::Compound:
		return ConstantFoldInt(resolver, ((AST::CompoundExpression*)expr)->value, success);

	case AST::ExpressionType::UnaryOperator:
	{
		AST::UnaryOperator* unaryOperator = (AST::UnaryOperator*)expr;
		switch (unaryOperator->operatorType)
		{
		case AST::UnaryOperatorType::Negate:
			return -ConstantFoldInt(resolver, unaryOperator->operand, success);

		default:
			SnekAssert(false);
			return 0;
		}
	}
	case AST::ExpressionType::BinaryOperator:
	{
		AST::BinaryOperator* binaryOperator = (AST::BinaryOperator*)expr;
		switch (binaryOperator->operatorType)
		{
		case AST::BinaryOperatorType::Add:
			return ConstantFoldInt(resolver, binaryOperator->left, success) + ConstantFoldInt(resolver, binaryOperator->right, success);
		case AST::BinaryOperatorType::Subtract:
			return ConstantFoldInt(resolver, binaryOperator->left, success) - ConstantFoldInt(resolver, binaryOperator->right, success);
		case AST::BinaryOperatorType::Multiply:
			return ConstantFoldInt(resolver, binaryOperator->left, success) * ConstantFoldInt(resolver, binaryOperator->right, success);
		case AST::BinaryOperatorType::Divide:
			return ConstantFoldInt(resolver, binaryOperator->left, success) / ConstantFoldInt(resolver, binaryOperator->right, success);
		case AST::BinaryOperatorType::Modulo:
			return ConstantFoldInt(resolver, binaryOperator->left, success) % ConstantFoldInt(resolver, binaryOperator->right, success);

		case AST::BinaryOperatorType::BitwiseAnd:
			return ConstantFoldInt(resolver, binaryOperator->left, success) & ConstantFoldInt(resolver, binaryOperator->right, success);
		case AST::BinaryOperatorType::BitwiseOr:
			return ConstantFoldInt(resolver, binaryOperator->left, success) | ConstantFoldInt(resolver, binaryOperator->right, success);
		case AST::BinaryOperatorType::BitwiseXor:
			return ConstantFoldInt(resolver, binaryOperator->left, success) ^ ConstantFoldInt(resolver, binaryOperator->right, success);
		case AST::BinaryOperatorType::BitshiftLeft:
			return ConstantFoldInt(resolver, binaryOperator->left, success) << ConstantFoldInt(resolver, binaryOperator->right, success);
		case AST::BinaryOperatorType::BitshiftRight:
			return ConstantFoldInt(resolver, binaryOperator->left, success) >> ConstantFoldInt(resolver, binaryOperator->right, success);

		default:
			SnekAssert(false);
			return 0;
		}
	}

	default:
		SnekAssert(false);
		return 0;
	}
}

static bool ResolveVoidType(Resolver* resolver, AST::VoidType* type)
{
	if (type->typeID = GetVoidType())
		return true;
	return false;
}

static bool ResolveIntegerType(Resolver* resolver, AST::IntegerType* type)
{
	if (type->typeID = GetIntegerType(type->bitWidth, type->isSigned))
		return true;
	return false;
}

static bool ResolveFPType(Resolver* resolver, AST::FloatingPointType* type)
{
	switch (type->bitWidth)
	{
	case 16:
		SnekErrorLoc(resolver->context, type->location, "16 bit floating point not supported");
		//type->typeID = GetFloatingPointType(FloatingPointPrecision::Half);
		type->typeID = GetFloatingPointType(FloatingPointPrecision::Single);
		return false;
	case 32:
		type->typeID = GetFloatingPointType(FloatingPointPrecision::Single);
		return true;
	case 64:
		type->typeID = GetFloatingPointType(FloatingPointPrecision::Double);
		return true;
	case 80:
		SnekErrorLoc(resolver->context, type->location, "80 bit floating point not supported");
		//type->typeID = GetFloatingPointType(FloatingPointPrecision::Decimal);
		type->typeID = GetFloatingPointType(FloatingPointPrecision::Double);
		return false;
	case 128:
		SnekErrorLoc(resolver->context, type->location, "128 bit floating point not supported");
		//type->typeID = GetFloatingPointType(FloatingPointPrecision::Quad);
		type->typeID = GetFloatingPointType(FloatingPointPrecision::Double);
		return false;
	default:
		SnekAssert(false);
		return false;
	}
}

static bool ResolveBoolType(Resolver* resolver, AST::BooleanType* type)
{
	if (type->typeID = GetBoolType())
		return true;
	return false;
}

static bool ResolveAnyType(Resolver* resolver, AST::AnyType* type)
{
	if (type->typeID = GetAnyType())
		return true;
	return false;
}

static bool ResolveNamedType(Resolver* resolver, AST::NamedType* type)
{
	resolver->currentFile->namedTypes.add(type);

	if (AST::Struct* structDecl = FindStruct(resolver, type->name))
	{
		if (structDecl->isGeneric)
		{
			if (!type->hasGenericArgs)
			{
				SnekErrorLoc(resolver->context, type->location, "Using generic struct type '%s' without type arguments", structDecl->name);
				return false;
			}

			bool result = true;

			for (int i = 0; i < type->genericArgs.size; i++)
			{
				result = ResolveType(resolver, type->genericArgs[i]) && result;
			}

			AST::Struct* instance = structDecl->getGenericInstance(type->genericArgs);
			if (!instance)
			{
				AST::File* lastFile = resolver->currentFile;
				resolver->currentFile = structDecl->file;

				instance = (AST::Struct*)structDecl->copy();
				instance->isGeneric = false;
				instance->isGenericInstance = true;

				instance->genericTypeArguments.resize(type->genericArgs.size);
				for (int i = 0; i < type->genericArgs.size; i++)
				{
					instance->genericTypeArguments[i] = type->genericArgs[i]->typeID;
				}

				// add instance to list before resolving to avoid stack overflow with circular dependencies
				structDecl->genericInstances.add(instance);

				result = ResolveStructHeader(resolver, instance) && result;
				result = ResolveStruct(resolver, instance) && result;

				resolver->currentFile = lastFile;
			}

			type->typeKind = AST::TypeKind::Struct;
			type->typeID = instance->type;
			type->declaration = instance;

			return result;
		}
		else
		{
			if (type->hasGenericArgs)
			{
				SnekErrorLoc(resolver->context, type->location, "Can't use type arguments on non-generic struct type '%s'", structDecl->name);
				return false;
			}

			type->typeKind = AST::TypeKind::Struct;
			type->typeID = structDecl->type;
			type->declaration = structDecl;
			return true;
		}
	}
	else if (AST::Class* classDecl = FindClass(resolver, type->name))
	{
		if (classDecl->isGeneric)
		{
			if (!type->hasGenericArgs)
			{
				SnekErrorLoc(resolver->context, type->location, "Using generic class type '%s' without type arguments", classDecl->name);
				return false;
			}

			bool result = true;

			for (int i = 0; i < type->genericArgs.size; i++)
			{
				result = ResolveType(resolver, type->genericArgs[i]) && result;
			}

			AST::Class* instance = classDecl->getGenericInstance(type->genericArgs);
			if (!instance)
			{
				AST::File* lastFile = resolver->currentFile;
				resolver->currentFile = classDecl->file;

				instance = (AST::Class*)classDecl->copy();
				instance->isGeneric = false;
				instance->isGenericInstance = true;

				instance->genericTypeArguments.resize(type->genericArgs.size);
				for (int i = 0; i < type->genericArgs.size; i++)
				{
					instance->genericTypeArguments[i] = type->genericArgs[i]->typeID;
				}

				classDecl->genericInstances.add(instance);

				result = ResolveClassHeader(resolver, instance) && result;
				result = ResolveClass(resolver, instance) && result;

				resolver->currentFile = lastFile;
			}

			type->typeKind = AST::TypeKind::Class;
			type->typeID = instance->type;
			type->declaration = instance;

			return result;
		}
		else
		{
			if (type->hasGenericArgs)
			{
				SnekErrorLoc(resolver->context, type->location, "Can't use type arguments on non-generic class type '%s'", classDecl->name);
				return false;
			}
			type->typeKind = AST::TypeKind::Class;
			type->typeID = classDecl->type;
			type->declaration = classDecl;
			return true;
		}
	}
	else if (AST::Typedef* typedefDecl = FindTypedef(resolver, type->name))
	{
		type->typeKind = AST::TypeKind::Alias;
		type->typeID = typedefDecl->type;
		type->declaration = typedefDecl;
		return true;
	}
	else if (AST::Enum* enumDecl = FindEnum(resolver, type->name))
	{
		type->typeKind = AST::TypeKind::Alias;
		type->typeID = enumDecl->type;
		type->declaration = enumDecl;
		return true;
	}
	else if (TypeID genericTypeArgument = resolver->getGenericTypeArgument(type->name))
	{
		type->typeKind = genericTypeArgument->typeKind;
		type->typeID = genericTypeArgument;
		return true;
	}

	SnekErrorLoc(resolver->context, type->location, "Undefined type '%s'", type->name);
	return false;
}

static bool ResolvePointerType(Resolver* resolver, AST::PointerType* type)
{
	if (ResolveType(resolver, type->elementType))
	{
		type->typeID = GetPointerType(type->elementType->typeID);
		return true;
	}
	return false;
}

static bool ResolveOptionalType(Resolver* resolver, AST::OptionalType* type)
{
	if (ResolveType(resolver, type->elementType))
	{
		type->typeID = GetOptionalType(type->elementType->typeID);
		return true;
	}
	return false;
}

static bool ResolveFunctionType(Resolver* resolver, AST::FunctionType* type)
{
	bool result = true;

	result = ResolveType(resolver, type->returnType) && result;

	for (int i = 0; i < type->paramTypes.size; i++)
	{
		result = ResolveType(resolver, type->paramTypes[i]) && result;
	}

	TypeID returnType = type->returnType->typeID;
	int numParams = type->paramTypes.size;
	TypeID* paramTypes = new TypeID[numParams];
	bool varArgs = type->varArgs;
	for (int i = 0; i < numParams; i++)
	{
		paramTypes[i] = type->paramTypes[i]->typeID;
	}

	if (type->varArgsType)
		result = ResolveType(resolver, type->varArgsType) && result;

	type->typeID = GetFunctionType(type->returnType->typeID, numParams, paramTypes, varArgs, type->varArgsType ? type->varArgsType->typeID : nullptr, false, nullptr, nullptr);

	return result;
}

static bool ResolveTupleType(Resolver* resolver, AST::TupleType* type)
{
	bool success = true;

	int numValues = type->valueTypes.size;
	TypeID* valueTypes = new TypeID[numValues];
	for (int i = 0; i < type->valueTypes.size; i++)
	{
		success = ResolveType(resolver, type->valueTypes[i]) && success;
		valueTypes[i] = type->valueTypes[i]->typeID;
	}

	type->typeID = GetTupleType(numValues, valueTypes);

	return success;
}

static bool ResolveArrayType(Resolver* resolver, AST::ArrayType* type)
{
	if (ResolveType(resolver, type->elementType))
	{
		if (type->length)
		{
			if (ResolveExpression(resolver, type->length))
			{
				if (type->length->valueType->typeKind == AST::TypeKind::Integer && type->length->isConstant())
				{
					bool success = true;
					type->typeID = GetArrayType(type->elementType->typeID, (int)ConstantFoldInt(resolver, type->length, success));
					return success;
				}
				else
				{
					bool success = true;
					type->typeID = GetArrayType(type->elementType->typeID, -1);
					return success;

					//SnekError(resolver->context, type->length->location, ERROR_CODE_ARRAY_LENGTH_WRONG_TYPE, "Array length specifier must be a constant integer value");
					//return false;
				}
			}
			else
			{
				return false;
			}
		}
		else
		{
			type->typeID = GetArrayType(type->elementType->typeID, -1);
			return true;
		}
	}
	else
	{
		return false;
	}
}

static bool ResolveStringType(Resolver* resolver, AST::StringType* type)
{
	if (type->length)
	{
		if (ResolveExpression(resolver, type->length))
		{
			//if (type->length->valueType->typeKind == AST::TypeKind::Integer && type->length->isConstant())
			//{
			bool success = true;
			//type->typeID = GetStringType((int)ConstantFoldInt(resolver, type->length, success));
			type->typeID = GetStringType();
			return success;
			//}
			//else
			//{
			//	SnekError(resolver->context, type->length->location, ERROR_CODE_ARRAY_LENGTH_WRONG_TYPE, "String length specifier must be a constant integer value");
			//	return false;
			//}
		}
		else
		{
			return false;
		}
	}
	else
	{
		type->typeID = GetStringType();
		return true;
	}
}

bool ResolveType(Resolver* resolver, AST::Type* type)
{
	AST::Element* lastElement = resolver->currentElement;
	resolver->currentElement = type;
	defer _(nullptr, [=](...) { resolver->currentElement = lastElement; });

	switch (type->typeKind)
	{
	case AST::TypeKind::Void:
		return ResolveVoidType(resolver, (AST::VoidType*)type);
	case AST::TypeKind::Integer:
		return ResolveIntegerType(resolver, (AST::IntegerType*)type);
	case AST::TypeKind::FloatingPoint:
		return ResolveFPType(resolver, (AST::FloatingPointType*)type);
	case AST::TypeKind::Boolean:
		return ResolveBoolType(resolver, (AST::BooleanType*)type);
	case AST::TypeKind::Any:
		return ResolveAnyType(resolver, (AST::AnyType*)type);
	case AST::TypeKind::NamedType:
		return ResolveNamedType(resolver, (AST::NamedType*)type);
	case AST::TypeKind::Pointer:
		return ResolvePointerType(resolver, (AST::PointerType*)type);
	case AST::TypeKind::Optional:
		return ResolveOptionalType(resolver, (AST::OptionalType*)type);
	case AST::TypeKind::Function:
		return ResolveFunctionType(resolver, (AST::FunctionType*)type);

	case AST::TypeKind::Tuple:
		return ResolveTupleType(resolver, (AST::TupleType*)type);
	case AST::TypeKind::Array:
		return ResolveArrayType(resolver, (AST::ArrayType*)type);
	case AST::TypeKind::String:
		return ResolveStringType(resolver, (AST::StringType*)type);

	default:
		SnekAssert(false);
		return false;
	}
}

static TypeID GetFittingTypeForIntegerLiteral(Resolver* resolver, AST::IntegerLiteral* expr)
{
	if (expr->value > INT64_MAX)
	{
		if (expr->value >= 0 && expr->value <= UINT64_MAX)
			return GetIntegerType(64, false);
		else
		{
			SnekErrorLoc(resolver->context, expr->location, "Integer value too big to fit into a 64 bit integer type");
			return nullptr;
		}
	}
	else if (expr->value > INT32_MAX || expr->value < INT32_MIN)
		return GetIntegerType(64, true);
	else
		return GetIntegerType(32, true);
}

static bool ResolveIntegerLiteral(Resolver* resolver, AST::IntegerLiteral* expr)
{
	expr->valueType = GetFittingTypeForIntegerLiteral(resolver, expr);
	expr->lvalue = false;
	return true;
}

static bool ResolveFPLiteral(Resolver* resolver, AST::FloatingPointLiteral* expr)
{
	expr->valueType = GetFloatingPointType(expr->isDouble ? FloatingPointPrecision::Double : FloatingPointPrecision::Single);
	expr->lvalue = false;
	return true;
}

static bool ResolveBoolLiteral(Resolver* resolver, AST::BooleanLiteral* expr)
{
	expr->valueType = GetBoolType();
	expr->lvalue = false;
	return true;
}

static bool ResolveCharacterLiteral(Resolver* resolver, AST::CharacterLiteral* expr)
{
	expr->valueType = GetIntegerType(8, false);
	expr->lvalue = false;
	return true;
}

static bool ResolveNullLiteral(Resolver* resolver, AST::NullLiteral* expr)
{
	expr->valueType = GetPointerType(GetVoidType());
	expr->lvalue = false;
	return true;
}

static bool ResolveStringLiteral(Resolver* resolver, AST::StringLiteral* expr)
{
	expr->valueType = GetStringType();
	//expr->valueType = GetPointerType(GetIntegerType(8, true));
	expr->lvalue = true;
	return true;
}

static bool ResolveInitializerList(Resolver* resolver, AST::InitializerList* expr)
{
	bool result = true;

	if (!expr->initializerTypeAST && resolver->expectedType)
		expr->initializerType = resolver->expectedType;

	//if (ResolveType(resolver, expr->structType))
	{
		TypeID* valueTypes = new TypeID[expr->values.size];
		for (int i = 0; i < expr->values.size; i++)
		{
			if (ResolveExpression(resolver, expr->values[i]))
			{
				//if (IsConstant(expr->values[i]))
				//{
				valueTypes[i] = expr->values[i]->valueType;
				//}
				//else
				//{
					// ERROR
				//	result = false;
				//}
			}
			else
			{
				result = false;
			}

			char* label = expr->labels[i];
			if (label)
			{
				if (expr->initializerType->typeKind == AST::TypeKind::Struct)
				{
					SnekAssert(expr->initializerType->structType.declaration);
					AST::StructField* field = expr->initializerType->structType.declaration->getFieldWithName(label);
					if (!field)
					{
						SnekErrorLoc(resolver->context, expr->location, "Undefined initializer label %s", label);
						result = false;
					}
				}
			}
		}

		//if (expr->initializerTypeAST)
		if (resolver->expectedType)
		{
			//result = ResolveType(resolver, expr->initializerTypeAST) && result;
			//expr->initializerType = expr->initializerTypeAST->typeID;
			expr->initializerType = resolver->expectedType;
		}
		else
		{
			//SnekAssert(expr->values.size > 0);
			int numValues = expr->values.size;
			TypeID* valueTypes = new TypeID[numValues];
			for (int i = 0; i < numValues; i++)
				valueTypes[i] = expr->values[i]->valueType;

			expr->initializerType = GetTupleType(numValues, valueTypes);
		}

		expr->valueType = expr->initializerType;
		expr->lvalue = false;
	}

	return result;
}

static bool ResolveIdentifier(Resolver* resolver, AST::Identifier* expr)
{
	resolver->currentFile->identifiers.add(expr);

	if (Variable* variable = resolver->findVariable(expr->name))
	{
		expr->valueType = variable->type;
		expr->lvalue = true; // !variable->isConstant;
		expr->variable = variable;
		return variable->type != nullptr;
	}
	if (resolver->findFunctions(expr->name, expr->functions))
	{
		expr->valueType = expr->functions[0]->functionType;
		expr->lvalue = false;
		//expr->function = function;
		return true;
	}
	if (AST::EnumValue* enumValue = FindEnumValue(resolver, expr->name))
	{
		expr->valueType = enumValue->declaration->type;
		expr->lvalue = false;
		expr->enumValue = enumValue;
		return true;
	}
	if (AST::Macro* exprdef = FindMacro(resolver, expr->name))
	{
		AST::Expression* value = (AST::Expression*)exprdef->alias->copy();
		value->file = resolver->currentFile;

		if (ResolveExpression(resolver, value))
		{
			expr->valueType = value->valueType;
			expr->lvalue = value->lvalue;
			expr->exprdefValue = value;
			return true;
		}
	}

	SnekErrorLoc(resolver->context, expr->location, "Unresolved identifier '%s'", expr->name);
	return false;
}

static bool ResolveCompoundExpression(Resolver* resolver, AST::CompoundExpression* expr)
{
	if (ResolveExpression(resolver, expr->value))
	{
		expr->valueType = expr->value->valueType;
		expr->lvalue = expr->value->lvalue;
		return true;
	}
	return false;
}

static bool ResolveTupleExpression(Resolver* resolver, AST::TupleExpression* expr)
{
	TypeID* valueTypes = new TypeID[expr->values.size];

	bool success = true;
	for (int i = 0; i < expr->values.size; i++)
	{
		success = ResolveExpression(resolver, expr->values[i]) && success;
		valueTypes[i] = expr->values[i]->valueType;
	}

	expr->valueType = GetTupleType(expr->values.size, valueTypes);
	expr->lvalue = false;

	return success;
}

TypeID DeduceGenericArg(AST::Type* type, TypeID argType, const AST::Function* function)
{
	if (type->typeKind == AST::TypeKind::NamedType)
	{
		AST::NamedType* namedType = (AST::NamedType*)type;
		if (namedType->hasGenericArgs)
		{
			if (argType->typeKind == AST::TypeKind::Struct)
			{
				if (argType->structType.declaration && argType->structType.declaration->isGenericInstance &&
					namedType->genericArgs.size == argType->structType.declaration->genericTypeArguments.size)
				{
					for (int i = 0; i < argType->structType.declaration->genericTypeArguments.size; i++)
					{
						if (TypeID type = DeduceGenericArg(namedType->genericArgs[i], argType->structType.declaration->genericTypeArguments[i], function))
							return type;
					}
				}
			}
			else if (argType->typeKind == AST::TypeKind::Class)
			{
				if (argType->classType.declaration && argType->classType.declaration->isGenericInstance &&
					namedType->genericArgs.size == argType->classType.declaration->genericTypeArguments.size)
				{
					for (int i = 0; i < argType->classType.declaration->genericTypeArguments.size; i++)
					{
						if (TypeID type = DeduceGenericArg(namedType->genericArgs[i], argType->classType.declaration->genericTypeArguments[i], function))
							return type;
					}
				}
			}
			else
			{
				// TODO implement
				__debugbreak();
			}
		}
		else
		{
			for (int i = 0; i < function->genericParams.size; i++)
			{
				if (strcmp(function->genericParams[i], namedType->name) == 0)
				{
					return argType;
				}
			}
		}
	}
	else if (type->typeKind == argType->typeKind)
	{
		if (type->typeKind == AST::TypeKind::Pointer)
		{
			return DeduceGenericArg(((AST::PointerType*)type)->elementType, argType->pointerType.elementType, function);
		}
		else if (type->typeKind == AST::TypeKind::Optional)
		{
			return DeduceGenericArg(((AST::OptionalType*)type)->elementType, argType->optionalType.elementType, function);
		}
		else if (type->typeKind == AST::TypeKind::Function)
		{
			AST::FunctionType* functionType = (AST::FunctionType*)type;
			if (TypeID type = DeduceGenericArg(functionType->returnType, argType->functionType.returnType, function))
				return type;
			if (functionType->paramTypes.size == argType->functionType.numParams)
			{
				for (int i = 0; i < functionType->paramTypes.size; i++)
				{
					if (TypeID type = DeduceGenericArg(functionType->paramTypes[i], argType->functionType.paramTypes[i], function))
						return type;
				}
			}
		}
		else if (type->typeKind == AST::TypeKind::Tuple)
		{
			AST::TupleType* tupleType = (AST::TupleType*)type;
			if (tupleType->valueTypes.size == argType->tupleType.numValues)
			{
				for (int i = 0; i < tupleType->valueTypes.size; i++)
				{
					if (TypeID type = DeduceGenericArg(tupleType->valueTypes[i], argType->tupleType.valueTypes[i], function))
						return type;
				}
			}
		}
		else if (type->typeKind == AST::TypeKind::Array)
		{
			AST::ArrayType* arrayType = (AST::ArrayType*)type;
			if ((arrayType->length && arrayType->length->isConstant()) == (argType->arrayType.length != -1))
			{
				return DeduceGenericArg(arrayType->elementType, argType->arrayType.elementType, function);
			}
		}
	}

	return nullptr;
}

static bool ResolveFunctionCall(Resolver* resolver, AST::FunctionCall* expr)
{
	if (expr->callee->type == AST::ExpressionType::Identifier)
	{
		AST::Identifier* identifier = (AST::Identifier*)expr->callee;
		KeywordType keywordType = getKeywordType(identifier->name, (int)strlen(identifier->name));
		if (keywordType == KEYWORD_TYPE_INT8)
			expr->castDstType = GetIntegerType(8, true);
		else if (keywordType == KEYWORD_TYPE_INT16)
			expr->castDstType = GetIntegerType(16, true);
		else if (keywordType == KEYWORD_TYPE_INT32)
			expr->castDstType = GetIntegerType(32, true);
		else if (keywordType == KEYWORD_TYPE_INT64)
			expr->castDstType = GetIntegerType(64, true);
		else if (keywordType == KEYWORD_TYPE_UINT8)
			expr->castDstType = GetIntegerType(8, false);
		else if (keywordType == KEYWORD_TYPE_UINT16)
			expr->castDstType = GetIntegerType(16, false);
		else if (keywordType == KEYWORD_TYPE_UINT32)
			expr->castDstType = GetIntegerType(32, false);
		else if (keywordType == KEYWORD_TYPE_UINT64)
			expr->castDstType = GetIntegerType(64, false);
		else if (keywordType == KEYWORD_TYPE_FLOAT16)
			expr->castDstType = GetFloatingPointType(FloatingPointPrecision::Half);
		else if (keywordType == KEYWORD_TYPE_FLOAT32)
			expr->castDstType = GetFloatingPointType(FloatingPointPrecision::Single);
		else if (keywordType == KEYWORD_TYPE_FLOAT64)
			expr->castDstType = GetFloatingPointType(FloatingPointPrecision::Double);
		else if (keywordType == KEYWORD_TYPE_FLOAT80)
			expr->castDstType = GetFloatingPointType(FloatingPointPrecision::Decimal);
		else if (keywordType == KEYWORD_TYPE_FLOAT128)
			expr->castDstType = GetFloatingPointType(FloatingPointPrecision::Quad);
		else if (keywordType == KEYWORD_TYPE_BOOL)
			expr->castDstType = GetBoolType();
		else if (keywordType == KEYWORD_TYPE_STRING)
			expr->castDstType = GetStringType();

		if (expr->castDstType)
		{
			expr->isCast = true;

			if (keywordType == KEYWORD_TYPE_STRING && expr->arguments.size == 2)
			{
				if (ResolveExpression(resolver, expr->arguments[0]) && ResolveExpression(resolver, expr->arguments[1]))
				{
					if (!CompareTypes(expr->arguments[0]->valueType, GetPointerType(GetIntegerType(8, true))) ||
						expr->arguments[1]->valueType->typeKind != AST::TypeKind::Integer)
					{
						SnekErrorLoc(resolver->context, expr->arguments[0]->location, "bleh bleh");
						return false;
					}

					expr->valueType = expr->castDstType;
					expr->lvalue = false;

					return true;
				}
				else
				{
					return false;
				}
			}
			else if (expr->arguments.size == 1)
			{
				ResolveExpression(resolver, expr->arguments[0]);

				expr->valueType = expr->castDstType;
				expr->lvalue = false;

				return true;
			}
			else
			{
				SnekErrorLoc(resolver->context, expr->location, "bleh");
				return false;
			}
		}
	}


	bool result = true;

	TypeID functionType = NULL;
	AST::Function* function = NULL;
	List<AST::Expression*> arguments;
	arguments.addAll(expr->arguments);

	if (ResolveExpression(resolver, expr->callee))
	{
		for (int i = 0; i < expr->arguments.size; i++)
		{
			if (!ResolveExpression(resolver, expr->arguments[i]))
				result = false;
		}
		if (!result)
			return false;


		if (expr->hasGenericArgs)
		{
			SnekAssert(expr->callee->type == AST::ExpressionType::Identifier);

			for (int i = 0; i < expr->genericArgsAST.size; i++)
			{
				result = ResolveType(resolver, expr->genericArgsAST[i]) && result;
				expr->genericArgs.add(expr->genericArgsAST[i]->typeID);
			}
		}


		const char* functionName = nullptr;
		List<AST::Function*>* functionOverloads = nullptr;
		AST::Expression* methodInstance = nullptr;
		if (expr->callee->type == AST::ExpressionType::Identifier && ((AST::Identifier*)expr->callee)->functions.size > 0)
		{
			functionName = ((AST::Identifier*)expr->callee)->name;
			functionOverloads = &((AST::Identifier*)expr->callee)->functions;
		}
		else if (expr->callee->type == AST::ExpressionType::DotOperator && ((AST::DotOperator*)expr->callee)->namespacedFunctions.size > 0)
		{
			functionName = ((AST::DotOperator*)expr->callee)->name;
			functionOverloads = &((AST::DotOperator*)expr->callee)->namespacedFunctions;
		}
		else if (expr->callee->type == AST::ExpressionType::DotOperator && ((AST::DotOperator*)expr->callee)->methods.size > 0)
		{
			functionName = ((AST::DotOperator*)expr->callee)->name;
			functionOverloads = &((AST::DotOperator*)expr->callee)->methods;
			methodInstance = ((AST::DotOperator*)expr->callee)->operand;
			arguments.insert(0, methodInstance);
		}

		if (functionOverloads)
			resolver->chooseFunctionOverload(*functionOverloads, arguments, methodInstance);

		if (functionOverloads && functionOverloads->size > 0 && resolver->getFunctionOverloadScore(functionOverloads->get(0), arguments) < 1000)
		{
			int lowestScore = resolver->getFunctionOverloadScore(functionOverloads->get(0), arguments);
			function = functionOverloads->get(0);

			if (function->isGeneric)
			{
				if (!expr->hasGenericArgs || expr->genericArgs.size < function->genericParams.size)
				{
					// Deduce generic args
					expr->hasGenericArgs = true;

					for (int i = expr->genericArgs.size; i < function->genericParams.size; i++)
					{
						for (int j = 0; j < function->paramTypes.size; j++)
						{
							if (TypeID type = DeduceGenericArg(function->paramTypes[j], arguments[j]->valueType, function))
							{
								expr->genericArgs.add(type);
								break;
							}
						}
					}
				}

				function = functionOverloads->get(0)->getGenericInstance(expr->genericArgs);
				if (!function)
				{
					AST::File* lastFile = resolver->currentFile;
					resolver->currentFile = functionOverloads->get(0)->file;

					function = (AST::Function*)(functionOverloads->get(0)->copy()); // Create a separate version of the function, TODO: reuse functions with the same type arguments
					function->isGeneric = false;
					function->isGenericInstance = true;

					function->genericTypeArguments.resize(expr->genericArgs.size);
					for (int i = 0; i < expr->genericArgs.size; i++)
					{
						function->genericTypeArguments[i] = expr->genericArgs[i];
					}

					functionOverloads->get(0)->genericInstances.add(function);

					Scope* callerScope = resolver->scope;
					resolver->scope = resolver->globalScope;

					result = ResolveFunctionHeader(resolver, function) && result;
					result = ResolveFunction(resolver, function) && result;

					resolver->scope = callerScope;

					resolver->currentFile = lastFile;
				}
			}

			functionType = function->functionType;

			if (functionOverloads->size == 1)
			{
				expr->callee->valueType = function->functionType;
			}
			else if (functionOverloads->size > 1 && resolver->getFunctionOverloadScore(functionOverloads->get(1), arguments) > lowestScore)
			{
				expr->callee->valueType = function->functionType;
			}
			else
			{
				char argumentTypesString[128] = "";
				for (int i = 0; i < arguments.size; i++)
				{
					strcat(argumentTypesString, GetTypeString(arguments[i]->valueType));
					if (i < arguments.size - 1)
						strcat(argumentTypesString, ",");
				}

				char functionList[1024] = "";
				for (int i = 0; i < functionOverloads->size; i++)
				{
					AST::Function* function = (*functionOverloads)[i];

					const char* functionTypeString = GetTypeString(function->functionType);
					strcat(functionList, function->name);
					strcat(functionList, " ");
					strcat(functionList, functionTypeString);

					if (i < functionOverloads->size - 1)
						strcat(functionList, ", ");
				}

				SnekErrorLoc(resolver->context, expr->callee->location, "Ambiguous function call %s(%s): %i possible overloads: %s", functionName, argumentTypesString, functionOverloads->size, functionList);
				return false;
			}
		}
		else if (expr->callee->valueType)
		{
			if (expr->callee->type == AST::ExpressionType::Identifier)
			{
				AST::Identifier* calleeIdentifier = (AST::Identifier*)expr->callee;
				if (calleeIdentifier->functions.size > 0 && calleeIdentifier->functions[0]->isGeneric)
				{
					SnekErrorLoc(resolver->context, expr->callee->location, "Calling generic function '%s' without type arguments", calleeIdentifier->functions[0]->name);
					return false;
				}
			}
			else if (expr->callee->type == AST::ExpressionType::DotOperator)
			{
				AST::DotOperator* calleeDotOperator = (AST::DotOperator*)expr->callee;
				if (calleeDotOperator->namespacedFunctions.size > 0 && calleeDotOperator->namespacedFunctions[0]->isGeneric)
				{
					// HACK
					if (calleeDotOperator->operand->type == AST::ExpressionType::Identifier)
					{
						SnekErrorLoc(resolver->context, expr->callee->location, "Calling generic function '%s'.'%s' without type arguments", ((AST::Identifier*)calleeDotOperator->operand)->name, calleeDotOperator->namespacedFunctions[0]->name);
						return false;
					}
					else
					{
						SnekErrorLoc(resolver->context, expr->callee->location, "Calling generic function '%s' without type arguments", calleeDotOperator->namespacedFunctions[0]->name);
						return false;
					}
				}
			}

			functionType = UnwrapType(expr->callee->valueType);

			if (functionType->typeKind != AST::TypeKind::Function)
			{
				SnekErrorLoc(resolver->context, expr->callee->location, "Can't invoke expression of type %s", GetTypeString(expr->callee->valueType));
				return false;
			}

			function = functionType->functionType.declaration;
		}
		else
		{
			char argumentTypesString[128] = "";
			for (int i = 0; i < arguments.size; i++)
			{
				strcat(argumentTypesString, GetTypeString(arguments[i]->valueType));
				if (i < arguments.size - 1)
					strcat(argumentTypesString, ",");
			}

			resolver->getFunctionOverloadScore(functionOverloads->get(0), arguments);
			SnekErrorLoc(resolver->context, expr->callee->location, "No overload for function call %s(%s)", functionName, argumentTypesString);
			return false;
		}


		expr->function = function;
		expr->valueType = functionType->functionType.returnType;
		expr->lvalue = false;
		expr->methodInstance = methodInstance;

		if (expr->callee->type == AST::ExpressionType::DotOperator)
		{
			AST::DotOperator* dotOperator = (AST::DotOperator*)expr->callee;
			if (dotOperator->classMethod || dotOperator->methods.size > 0)
			{
				expr->isMethodCall = true;
				expr->methodInstance = ((AST::DotOperator*)expr->callee)->operand;

				if (!expr->methodInstance->lvalue && expr->methodInstance->valueType->typeKind != AST::TypeKind::Pointer)
				{
					SnekErrorLoc(resolver->context, expr->location, "Instance for method call must be an lvalue or a reference");
					result = false;
				}
			}
		}

		if (arguments.size < (function ? function->getNumRequiredParams() : functionType->functionType.numParams) ||
			arguments.size > functionType->functionType.numParams && !functionType->functionType.varArgs)
		{
			if (function)
				SnekErrorLoc(resolver->context, expr->location, "Wrong number of arguments when calling function '%s': should be %d instead of %d", function->name, functionType->functionType.numParams - (methodInstance ? 1 : 0), expr->arguments.size);
			else
				SnekErrorLoc(resolver->context, expr->location, "Wrong number of arguments in function call: should be %d instead of %d", functionType->functionType.numParams, expr->arguments.size);
			result = false;
		}
	}
	else
	{
		result = false;
	}


	if (!result)
		return false;


	for (int i = 0; i < arguments.size; i++)
	{
		TypeID argType = arguments[i]->valueType;
		bool argIsConstant = arguments[i]->isConstant();

		int paramOffset = expr->isMethodCall ? 1 : 0;
		int paramCount = functionType->functionType.numParams - (expr->isMethodCall ? 1 : 0);

		if (i < paramCount || expr->isMethodCall)
		{
			TypeID paramType = functionType->functionType.paramTypes[i];

			if (i == 0 && expr->isMethodCall)
			{
				TypeID instanceType = functionType->functionType.paramTypes[0];
				if (instanceType->typeKind == AST::TypeKind::Pointer && CompareTypes(instanceType->pointerType.elementType, argType))
					;
				else
				{
					SnekAssert(false);
				}
			}
			else if (!CanConvertImplicit(argType, paramType, argIsConstant))
			{
				result = false;

				const char* argTypeStr = GetTypeString(argType);
				const char* paramTypeStr = GetTypeString(paramType);

				if (function)
				{
					SnekErrorLoc(resolver->context, expr->location, "Invalid argument type '%s' for function parameter '%s %s'", argTypeStr, paramTypeStr, function->paramNames[i]);
					result = false;
				}
				else
				{
					SnekErrorLoc(resolver->context, expr->location, "Invalid argument type '%s' for function parameter '%s #%d'", argTypeStr, paramTypeStr, i - paramOffset);
					result = false;
				}
			}
		}
		else
		{
			SnekAssert(functionType->functionType.varArgs);

			if (!CanConvertImplicit(argType, functionType->functionType.varArgsType, argIsConstant))
			{
				result = false;

				const char* argTypeStr = GetTypeString(argType);
				const char* varArgsTypeStr = GetTypeString(functionType->functionType.varArgsType);

				SnekErrorLoc(resolver->context, expr->location, "Can't implicitly cast variadic argument #%d to type %s, should be %s", i - functionType->functionType.numParams, varArgsTypeStr, argTypeStr);
				result = false;
			}
		}
	}

	return result;
}

static bool ResolveSubscriptOperator(Resolver* resolver, AST::SubscriptOperator* expr)
{
	if (ResolveExpression(resolver, expr->operand))
	{
		bool resolved = true;
		for (int i = 0; i < expr->arguments.size; i++)
		{
			if (!ResolveExpression(resolver, expr->arguments[i]))
				resolved = false;
		}

		if (resolved)
		{
			if (expr->operand->valueType->typeKind == AST::TypeKind::Pointer)
			{
				if (expr->arguments.size == 1)
				{
					if (expr->operand->valueType->pointerType.elementType->typeKind == AST::TypeKind::Array)
					{
						expr->valueType = expr->operand->valueType->pointerType.elementType->arrayType.elementType;
					}
					else
					{
						expr->valueType = expr->operand->valueType->pointerType.elementType;
					}
					expr->lvalue = true;
					return true;
				}
				else
				{
					SnekAssert(false);
					return false;
				}
			}
			else if (expr->operand->valueType->typeKind == AST::TypeKind::Array)
			{
				if (expr->arguments.size == 1)
				{
					expr->valueType = expr->operand->valueType->arrayType.elementType;
					expr->lvalue = true;
					return true;
				}
				else
				{
					SnekAssert(false);
					return false;
				}
			}
			else if (expr->operand->valueType->typeKind == AST::TypeKind::String)
			{
				if (expr->arguments.size == 1)
				{
					expr->valueType = GetIntegerType(8, true);
					expr->lvalue = true;
					return true;
				}
				else
				{
					SnekAssert(false);
					return false;
				}
			}
			else if (expr->operand->valueType->typeKind == AST::TypeKind::Struct || expr->operand->valueType->typeKind == AST::TypeKind::Class)
			{
				TypeID operandType = expr->operand->valueType;
				List<AST::Expression*> arguments;
				arguments.addAll(expr->arguments);
				arguments.insert(0, expr->operand);

				AST::Function* function = resolver->findUnaryOperatorOverload(operandType, AST::UnaryOperatorType::Subscript);
				if (function)
				{
					bool result = true;

					if (function->isGeneric)
					{
						SnekAssert(function->genericParams.size == 1);

						List<TypeID> genericArgs;
						genericArgs.add(DeduceGenericArg(function->paramTypes[0], operandType, function));

						AST::Function* instance = function->getGenericInstance(genericArgs);
						if (!instance)
						{
							AST::File* lastFile = resolver->currentFile;
							resolver->currentFile = function->file;

							instance = (AST::Function*)(function->copy()); // Create a separate version of the function, TODO: reuse functions with the same type arguments
							instance->isGeneric = false;
							instance->isGenericInstance = true;

							instance->genericTypeArguments.reserve(genericArgs.size);
							instance->genericTypeArguments.addAll(genericArgs);

							function->genericInstances.add(instance);

							Scope* callerScope = resolver->scope;
							resolver->scope = resolver->globalScope;

							result = ResolveFunctionHeader(resolver, instance) && result;
							result = ResolveFunction(resolver, instance) && result;

							resolver->scope = callerScope;

							resolver->currentFile = lastFile;
						}

						function = instance;
					}

					TypeID functionType = function->functionType;

					expr->operatorOverload = function;
					expr->valueType = functionType->functionType.returnType;
					expr->lvalue = false;

					SnekAssert(!function->varArgs);

					for (int i = 0; i < arguments.size; i++)
					{
						TypeID argType = arguments[i]->valueType;
						bool argIsConstant = arguments[i]->isConstant();

						int paramCount = functionType->functionType.numParams;

						if (i < paramCount)
						{
							TypeID paramType = functionType->functionType.paramTypes[i];

							if (!CanConvertImplicit(argType, paramType, argIsConstant))
							{
								result = false;

								const char* argTypeStr = GetTypeString(argType);
								const char* paramTypeStr = GetTypeString(paramType);

								if (function)
								{
									SnekErrorLoc(resolver->context, expr->location, "Invalid argument type '%s' for function parameter '%s %s'", argTypeStr, paramTypeStr, function->paramNames[i]);
									result = false;
								}
								else
								{
									SnekErrorLoc(resolver->context, expr->location, "Invalid argument type '%s' for function parameter '%s #%d'", argTypeStr, paramTypeStr, i);
									result = false;
								}
							}
						}
					}

					return result;
				}

				SnekAssert(false);
				return false;
			}

			SnekErrorLoc(resolver->context, expr->operand->location, "Subscript operator [int] not applicable to value of type %s", GetTypeString(expr->operand->valueType));
			return false;
		}
	}
	return false;
}

/*
static AstModule* GetModuleFromNamespace(Resolver* resolver, const char* nameSpace)
{
	for (int i = 0; i < resolver->asts.size; i++)
	{
		AstModule* module = resolver->asts[i];
		if (module->nameSpace && strcmp(module->nameSpace, nameSpace) == 0)
			return module;
	}
	return NULL;
}
*/

static bool ResolveDotOperator(Resolver* resolver, AST::DotOperator* expr)
{
	if (expr->operand->type == AST::ExpressionType::Identifier)
	{
		AST::Identifier* identifier = (AST::Identifier*)expr->operand;
		const char* nameSpace = identifier->name;

		//if (AST::File* file = FindFileWithNamespace(resolver, nameSpace, resolver->currentFile))
		if (!resolver->findVariable(nameSpace))
		{
			if (AST::Module* module = FindModuleInDependencies(resolver, nameSpace, resolver->currentFile))
			{
				identifier->module = module;

				//if (AST::File* file = module->file)
				for (AST::File* file : module->files)
				{
					if (resolver->findFunctionsInFile(expr->name, file, expr->namespacedFunctions))
					{
						//if (function->visibility >= AST::Visibility::Public || file->module == resolver->currentFile->module)
						//{
							//expr->namespacedFunction = function;
						expr->valueType = expr->namespacedFunctions[0]->functionType;
						expr->lvalue = false;
						return true;
						//}
						//else
						//{
						//	SnekError(resolver->context, expr->location, ERROR_CODE_NON_VISIBLE_DECLARATION, "Function '%s' is not visible", function->name);
						//	return false;
						//}
					}
					if (Variable* variable = resolver->findGlobalVariableInFile(expr->name, file))
					{
						expr->namespacedVariable = variable;
						expr->valueType = variable->type;
						expr->lvalue = true;
						return true;
					}
				}
			}
			else if (AST::Enum* en = FindEnum(resolver, nameSpace))
			{
				identifier->enumDecl = en;
				for (int i = 0; i < en->values.size; i++)
				{
					if (strcmp(en->values[i]->name, expr->name) == 0)
					{
						expr->enumValue = en->values[i];
						expr->valueType = en->type;
						return true;
					}
				}
			}

			SnekErrorLoc(resolver->context, expr->location, "No property '%s.%s'", nameSpace, expr->name);
			return false;
		}

		if (KeywordType keywordType = getKeywordType(nameSpace, (int)strlen(nameSpace)))
		{
			identifier->builtinType = true;

			if (keywordType == KEYWORD_TYPE_INT8)
			{
				if (strcmp(expr->name, "min") == 0)
					expr->builtinTypeProperty = AST::BuiltinTypeProperty::Int8Min;
				else if (strcmp(expr->name, "max") == 0)
					expr->builtinTypeProperty = AST::BuiltinTypeProperty::Int8Max;
				expr->valueType = GetIntegerType(8, true);
				expr->lvalue = false;
				return true;
			}
			else if (keywordType == KEYWORD_TYPE_INT16)
			{
				if (strcmp(expr->name, "min") == 0)
					expr->builtinTypeProperty = AST::BuiltinTypeProperty::Int16Min;
				else if (strcmp(expr->name, "max") == 0)
					expr->builtinTypeProperty = AST::BuiltinTypeProperty::Int16Max;
				expr->valueType = GetIntegerType(16, true);
				expr->lvalue = false;
				return true;
			}
			else if (keywordType == KEYWORD_TYPE_INT32)
			{
				if (strcmp(expr->name, "min") == 0)
					expr->builtinTypeProperty = AST::BuiltinTypeProperty::Int32Min;
				else if (strcmp(expr->name, "max") == 0)
					expr->builtinTypeProperty = AST::BuiltinTypeProperty::Int32Max;
				expr->valueType = GetIntegerType(32, true);
				expr->lvalue = false;
				return true;
			}
			else if (keywordType == KEYWORD_TYPE_INT64)
			{
				if (strcmp(expr->name, "min") == 0)
					expr->builtinTypeProperty = AST::BuiltinTypeProperty::Int64Min;
				else if (strcmp(expr->name, "max") == 0)
					expr->builtinTypeProperty = AST::BuiltinTypeProperty::Int64Max;
				expr->valueType = GetIntegerType(64, true);
				expr->lvalue = false;
				return true;
			}
			else if (keywordType == KEYWORD_TYPE_UINT8)
			{
				if (strcmp(expr->name, "min") == 0)
					expr->builtinTypeProperty = AST::BuiltinTypeProperty::UInt8Min;
				else if (strcmp(expr->name, "max") == 0)
					expr->builtinTypeProperty = AST::BuiltinTypeProperty::UInt8Max;
				expr->valueType = GetIntegerType(8, false);
				expr->lvalue = false;
				return true;
			}
			else if (keywordType == KEYWORD_TYPE_UINT16)
			{
				if (strcmp(expr->name, "min") == 0)
					expr->builtinTypeProperty = AST::BuiltinTypeProperty::UInt16Min;
				else if (strcmp(expr->name, "max") == 0)
					expr->builtinTypeProperty = AST::BuiltinTypeProperty::UInt16Max;
				expr->valueType = GetIntegerType(16, false);
				expr->lvalue = false;
				return true;
			}
			else if (keywordType == KEYWORD_TYPE_UINT32)
			{
				if (strcmp(expr->name, "min") == 0)
					expr->builtinTypeProperty = AST::BuiltinTypeProperty::UInt32Min;
				else if (strcmp(expr->name, "max") == 0)
					expr->builtinTypeProperty = AST::BuiltinTypeProperty::UInt32Max;
				expr->valueType = GetIntegerType(32, false);
				expr->lvalue = false;
				return true;
			}
			else if (keywordType == KEYWORD_TYPE_UINT64)
			{
				if (strcmp(expr->name, "min") == 0)
					expr->builtinTypeProperty = AST::BuiltinTypeProperty::UInt64Min;
				else if (strcmp(expr->name, "max") == 0)
					expr->builtinTypeProperty = AST::BuiltinTypeProperty::UInt64Max;
				expr->valueType = GetIntegerType(64, false);
				expr->lvalue = false;
				return true;
			}
		}

		/*
		else
		{
			AstModule* ns = NULL;

			if (!ns) ns = FindModuleInDependencies(resolver, nameSpace, resolver->file); // Search in imported namespaces
			if (!ns) ns = FindModule(resolver, nameSpace, NULL); // Search in global namespace

			if (ns)
			{
				if (AstFunction* function = FindFunctionInNamespace(resolver, ns, expr->name, resolver->file->module))
				{
					expr->namespacedFunction = function;
					expr->valueType = function->type;
					expr->lvalue = false;
					return true;
				}
				if (AstVariable* variable = FindGlobalVariableInNamespace(resolver, ns, expr->name, resolver->file->module))
				{
					expr->namespacedVariable = variable;
					expr->valueType = variable->type;
					expr->lvalue = true;
					return true;
				}
				if (AstModule* nestedNs = FindModule(resolver, expr->name, ns))
				{
					expr->ns = nestedNs;
					expr->valueType = NULL;
					return true;
				}

				SnekError(resolver->context, expr->location, ERROR_CODE_UNRESOLVED_IDENTIFIER, "Unresolved identifier '%s' in module %s", expr->name, ns->name);
				return false;
			}
		}
		*/
	}
	if (ResolveExpression(resolver, expr->operand))
	{
		if (expr->operand->type == AST::ExpressionType::DotOperator)
		{
			if (AST::Module* module = ((AST::DotOperator*)expr->operand)->module)
			{
				//if (AST::File* file = module->file)
				for (AST::File* file : module->files)
				{
					if (resolver->findFunctionsInFile(expr->name, file, expr->namespacedFunctions))
					{
						//expr->namespacedFunction = function;
						expr->valueType = expr->namespacedFunctions[0]->functionType;
						expr->lvalue = false;
						return true;
					}
				}
				if (AST::Module* nestedNs = FindModule(resolver, expr->name, module))
				{
					expr->module = nestedNs;
					expr->valueType = NULL;
					return true;
				}

				SnekErrorLoc(resolver->context, expr->location, "Unresolved identifier '%s' in namespace %s", expr->name, module->name);
				return false;
			}
		}

		TypeID operandType = expr->operand->valueType;
		operandType = UnwrapType(operandType);
		if (operandType->typeKind == AST::TypeKind::Pointer)
			operandType = operandType->pointerType.elementType;
		operandType = UnwrapType(operandType);

		SnekAssert(expr->methods.size == 0);
		/*
		if (resolver->findFunctions(expr->name, expr->methods))
		{
			for (int i = 0; i < expr->methods.size; i++)
			{
				if (expr->methods[i]->paramTypes.size < 1)
					expr->methods.removeAt(i--);
			}

			if (expr->methods.size > 0)
				return true;
		}
		*/

		if (operandType->typeKind == AST::TypeKind::Struct)
		{
			if (AST::StructField* field = operandType->structType.declaration->getFieldWithName(expr->name))
			{
				SnekAssert(field->type);
				expr->valueType = field->type->typeID;
				expr->lvalue = true;
				expr->structField = field;
				return true;
			}
			/*
			for (int i = 0; i < operandType->structType.declaration->fields.size; i++)
			{
				AST::StructField* field = operandType->structType.declaration->fields[i];
				TypeID memberType = operandType->structType.fieldTypes[i];
				if (strcmp(field->name, expr->name) == 0)
				{
					expr->valueType = memberType;
					expr->lvalue = true;
					expr->structField = field;
					return true;
				}
			}
			*/

			if (AST::Function* method = resolver->findFunction(expr->name, operandType))
			{
				expr->valueType = method->functionType;
				expr->lvalue = false;
				expr->methods.add(method);
				return true;
			}

			SnekErrorLoc(resolver->context, expr->location, "Unresolved struct member %s.%s", GetTypeString(expr->operand->valueType), expr->name);
			return false;
		}
		else if (operandType->typeKind == AST::TypeKind::Class)
		{
			for (int i = 0; i < operandType->classType.declaration->fields.size; i++)
			{
				AST::ClassField* field = operandType->classType.declaration->fields[i];
				TypeID memberType = operandType->classType.fieldTypes[i];
				if (strcmp(field->name, expr->name) == 0)
				{
					expr->valueType = memberType;
					expr->lvalue = true;
					expr->classField = field;
					return true;
				}
			}
			for (int i = 0; i < operandType->classType.declaration->methods.size; i++)
			{
				AST::Method* method = operandType->classType.declaration->methods[i];
				if (strcmp(method->name, expr->name) == 0)
				{
					expr->valueType = method->functionType;
					expr->lvalue = false;
					expr->classMethod = method;
					return true;
				}
			}

			SnekErrorLoc(resolver->context, expr->location, "Unresolved class member %s.%s", GetTypeString(operandType), expr->name);
			return false;
		}
		else if (operandType->typeKind == AST::TypeKind::Optional)
		{
			if (expr->name)
			{
				if (strcmp(expr->name, "value") == 0)
				{
					expr->valueType = operandType->optionalType.elementType;
					expr->lvalue = false;
					expr->fieldIndex = 0;
					return true;
				}
				else if (strcmp(expr->name, "hasValue") == 0)
				{
					expr->valueType = GetBoolType();
					expr->lvalue = false;
					expr->fieldIndex = 1;
					return true;
				}

				SnekErrorLoc(resolver->context, expr->location, "Unresolved optional property %s.%s", GetTypeString(operandType), expr->name);
				return false;
			}
			else if (expr->fieldIndex != -1)
			{
				SnekErrorLoc(resolver->context, expr->location, "Unresolved optional property %s.%d", GetTypeString(operandType), expr->fieldIndex);
				return false;
			}
			else
			{
				SnekAssert(false);
			}
		}
		else if (operandType->typeKind == AST::TypeKind::Tuple)
		{
			if (expr->fieldIndex >= 0 && expr->fieldIndex < operandType->tupleType.numValues)
			{
				expr->valueType = operandType->tupleType.valueTypes[expr->fieldIndex];
				expr->lvalue = true;
				return true;
			}

			SnekErrorLoc(resolver->context, expr->location, "Tuple index %d out of range for type %s", expr->fieldIndex, GetTypeString(operandType));
			return false;
		}
		else if (operandType->typeKind == AST::TypeKind::Array)
		{
			//expr->fieldIndex = -1;
			if (strcmp(expr->name, "length") == 0)
			{
				expr->valueType = GetIntegerType(32, false);
				expr->lvalue = false;
				expr->fieldIndex = 0;
				return true;
			}
			else if (strcmp(expr->name, "ptr") == 0)
			{
				expr->valueType = GetPointerType(operandType->arrayType.elementType);
				expr->lvalue = false;
				expr->fieldIndex = 1;
				return true;
			}

			SnekErrorLoc(resolver->context, expr->location, "Unresolved array property %s.%s", GetTypeString(expr->operand->valueType), expr->name);
			return false;
		}
		else if (operandType->typeKind == AST::TypeKind::String)
		{
			expr->fieldIndex = -1;
			if (strcmp(expr->name, "length") == 0)
			{
				expr->valueType = GetIntegerType(32, false);
				expr->lvalue = false;
				expr->fieldIndex = 0;
				return true;
			}
			else if (strcmp(expr->name, "ptr") == 0)
			{
				expr->valueType = GetPointerType(GetIntegerType(8, true));
				expr->lvalue = false;
				expr->fieldIndex = 1;
				return true;
			}

			SnekErrorLoc(resolver->context, expr->location, "Unresolved string property %s.%s", GetTypeString(expr->operand->valueType), expr->name);
			return false;
		}
		else if (operandType->typeKind == AST::TypeKind::Any)
		{
			if (strcmp(expr->name, "type") == 0)
			{
				expr->valueType = GetIntegerType(64, false);
				expr->lvalue = false;
				expr->fieldIndex = 0;
				return true;
			}
			else if (strcmp(expr->name, "value") == 0)
			{
				expr->valueType = GetPointerType(GetVoidType());
				expr->lvalue = false;
				expr->fieldIndex = 1;
				return true;
			}

			SnekErrorLoc(resolver->context, expr->location, "Unresolved any type property %s.%s", GetTypeString(expr->operand->valueType), expr->name);
			return false;
		}
		else
		{
			SnekErrorLoc(resolver->context, expr->location, "Unresolved property %s.%s", GetTypeString(expr->operand->valueType), expr->name);
			return false;
		}
	}
	else
	{
		return false;
	}

	SnekAssert(false);
	return false;
}

static bool ResolveCast(Resolver* resolver, AST::Typecast* expr)
{
	if (ResolveType(resolver, expr->dstType))
	{
		if (ResolveExpression(resolver, expr->value))
		{
			if (CanConvert(expr->value->valueType, expr->dstType->typeID))
			{
				expr->valueType = expr->dstType->typeID;
				expr->lvalue = false;
				return true;
			}
			else
			{
				SnekErrorLoc(resolver->context, expr->location, "Invalid cast: %s to %s", GetTypeString(expr->value->valueType), GetTypeString(expr->dstType->typeID));
			}
		}
	}
	return false;
}

static bool ResolveSizeof(Resolver* resolver, AST::Sizeof* expr)
{
	if (ResolveType(resolver, expr->dstType))
	{
		expr->valueType = GetIntegerType(64, false);
		expr->lvalue = false;
		return true;
	}
	return false;
}

static bool ResolveMalloc(Resolver* resolver, AST::Malloc* expr)
{
	bool result = true;

	if (ResolveType(resolver, expr->dstType))
	{
		if (expr->dstType->typeID->typeKind == AST::TypeKind::Array)
		{
			AST::ArrayType* arrayType = (AST::ArrayType*)expr->dstType;

			if (!expr->malloc && !arrayType->length->isConstant())
			{
				SnekErrorLoc(resolver->context, arrayType->location, "Dynamically sized arrays need to be allocated on the heap using 'new'");
				result = false;
			}
			if (!arrayType->length)
			{
				SnekErrorLoc(resolver->context, expr->location, "Allocating array needs a length");
				result = false;
			}

			if (expr->count)
			{
				SnekErrorLoc(resolver->context, expr->location, "Can't allocate multiple arrays at once");
				result = false;
			}

			expr->valueType = GetArrayType(arrayType->typeID->arrayType.elementType, arrayType->typeID->arrayType.length);
			expr->lvalue = false;
		}
		else if (expr->dstType->typeID->typeKind == AST::TypeKind::String)
		{
			AST::StringType* stringType = (AST::StringType*)expr->dstType;

			if (!expr->malloc && !stringType->length->isConstant())
			{
				SnekErrorLoc(resolver->context, stringType->location, "Stack allocated strings need to have a constant length");
				result = false;
			}
			if (!stringType->length)
			{
				SnekErrorLoc(resolver->context, expr->location, "Allocating string needs a length");
				result = false;
			}

			if (expr->count)
			{
				SnekErrorLoc(resolver->context, expr->location, "Can't allocate multiple strings at once");
				result = false;
			}

			expr->valueType = GetPointerType(GetStringType());
			expr->lvalue = false;
		}
		else if (expr->dstType->typeID->typeKind == AST::TypeKind::Class)
		{
			if (AST::Constructor* constructor = expr->dstType->typeID->classType.declaration->constructor)
			{
				if (expr->arguments.size < constructor->paramTypes.size || expr->arguments.size > constructor->paramTypes.size && !constructor->varArgs)
				{
					SnekErrorLoc(resolver->context, expr->location, "Wrong number of arguments in constructor: should be %d instead of %d", constructor->paramTypes.size, expr->arguments.size);
					result = false;
				}
				for (int i = 0; i < expr->arguments.size; i++)
				{
					if (ResolveExpression(resolver, expr->arguments[i]))
					{
						if (i < constructor->paramTypes.size)
						{
							TypeID argType = expr->arguments[i]->valueType;
							TypeID paramType = constructor->paramTypes[i]->typeID;
							const char* argTypeStr = GetTypeString(expr->arguments[i]->valueType);
							const char* paramTypeStr = GetTypeString(constructor->paramTypes[i]->typeID);
							if (!CanConvertImplicit(argType, paramType, expr->arguments[i]->isConstant()))
							{
								SnekErrorLoc(resolver->context, expr->arguments[i]->location, "Wrong type of argument '%s': should be %s instead of %s", constructor->paramNames[i], paramTypeStr, argTypeStr);
								result = false;
							}
						}
					}
					else
					{
						result = false;
					}
				}
			}

			expr->valueType = expr->dstType->typeID;
			expr->lvalue = false;
		}
		else
		{
			if (expr->hasArguments)
			{
				SnekErrorLoc(resolver->context, expr->dstType->location, "Can't call constructor of non-class type");
				result = false;
			}
			else
			{
				expr->valueType = GetPointerType(expr->dstType->typeID);
				expr->lvalue = false;
			}
		}
		if (expr->count)
		{
			if (!ResolveExpression(resolver, expr->count))
				result = false;
			if (!expr->count->isConstant() && !expr->malloc)
			{
				SnekErrorLoc(resolver->context, expr->count->location, "Stack allocation instance count must be a constant integer");
				result = false;
			}
		}
	}
	else
	{
		result = false;
	}
	return result;
}

static bool ResolveUnaryOperator(Resolver* resolver, AST::UnaryOperator* expr)
{
	if (ResolveExpression(resolver, expr->operand))
	{
		if (!expr->position)
		{
			switch (expr->operatorType)
			{
			case AST::UnaryOperatorType::Not:
				expr->valueType = GetBoolType();
				expr->lvalue = false;
				return true;
			case AST::UnaryOperatorType::Negate:
				expr->valueType = expr->operand->valueType;
				expr->lvalue = false;
				return true;
			case AST::UnaryOperatorType::Reference:
				expr->valueType = GetPointerType(expr->operand->valueType);
				expr->lvalue = false;
				return true;
			case AST::UnaryOperatorType::Dereference:
				if (expr->operand->valueType->typeKind == AST::TypeKind::Pointer)
				{
					expr->valueType = expr->operand->valueType->pointerType.elementType;
					expr->lvalue = true;
					return true;
				}
				else
				{
					SnekErrorLoc(resolver->context, expr->operand->location, "Can't dereference non-pointer type value");
					return false;
				}

			case AST::UnaryOperatorType::Increment:
				expr->valueType = expr->operand->valueType;
				expr->lvalue = true;
				return true;
			case AST::UnaryOperatorType::Decrement:
				expr->valueType = expr->operand->valueType;
				expr->lvalue = true;
				return true;

			default:
				SnekAssert(false);
				return false;
			}
		}
		else
		{
			switch (expr->operatorType)
			{
			case AST::UnaryOperatorType::Increment:
				expr->valueType = expr->operand->valueType;
				expr->lvalue = false;
				return true;
			case AST::UnaryOperatorType::Decrement:
				expr->valueType = expr->operand->valueType;
				expr->lvalue = false;
				return true;

			default:
				SnekAssert(false);
				return false;
			}
		}
	}
	return false;
}

static bool ResolveBinaryOperator(Resolver* resolver, AST::BinaryOperator* expr)
{
	bool result = true;

	result = ResolveExpression(resolver, expr->left) && result;
	result = ResolveExpression(resolver, expr->right) && result;

	if (result)
	{
		if (expr->left->valueType && expr->right->valueType)
		{
			switch (expr->operatorType)
			{
			case AST::BinaryOperatorType::Add:
			case AST::BinaryOperatorType::Subtract:
			case AST::BinaryOperatorType::Multiply:
			case AST::BinaryOperatorType::Divide:
			case AST::BinaryOperatorType::Modulo:
			{
				TypeID resultingType = BinaryOperatorTypeMeet(resolver, expr->left->valueType, expr->right->valueType);

				if (resultingType)
				{
					expr->valueType = resultingType;
					expr->lvalue = false;
					return expr->valueType;
				}
				else
				{
					if (AST::Function* overload = resolver->findBinaryOperatorOverload(expr->left->valueType, expr->right->valueType, expr->operatorType))
					{
						expr->operatorOverload = overload;
						expr->valueType = overload->returnType->typeID;
						expr->lvalue = false;
						return expr->valueType;
					}

					const char* operatorTokenStr = GetBinaryOperatorStr(expr->operatorType);
					SnekErrorLoc(resolver->context, expr->location, "Invalid binary operator %s %s %s", GetTypeString(expr->left->valueType), operatorTokenStr, GetTypeString(expr->right->valueType));
					return false;
				}
			}

			case AST::BinaryOperatorType::Equals:
			case AST::BinaryOperatorType::DoesNotEqual:
			case AST::BinaryOperatorType::LessThan:
			case AST::BinaryOperatorType::GreaterThan:
			case AST::BinaryOperatorType::LessThanEquals:
			case AST::BinaryOperatorType::GreaterThanEquals:
				expr->valueType = GetBoolType();
				expr->lvalue = false;
				return true;

			case AST::BinaryOperatorType::LogicalAnd:
			case AST::BinaryOperatorType::LogicalOr:
				if (CanConvertImplicit(expr->left->valueType, GetBoolType(), false) && CanConvertImplicit(expr->right->valueType, GetBoolType(), false))
				{
					expr->valueType = GetBoolType();
					expr->lvalue = false;
					return true;
				}
				else
				{
					if (expr->operatorType == AST::BinaryOperatorType::LogicalAnd)
						SnekErrorLoc(resolver->context, expr->left->location, "Operands of && must be of bool type");
					else if (expr->operatorType == AST::BinaryOperatorType::LogicalOr)
						SnekErrorLoc(resolver->context, expr->left->location, "Operands of || must be of bool type");
					else
						SnekAssert(false);
					return false;
				}

			case AST::BinaryOperatorType::BitwiseAnd:
			case AST::BinaryOperatorType::BitwiseOr:
			case AST::BinaryOperatorType::BitwiseXor:
			case AST::BinaryOperatorType::BitshiftLeft:
			case AST::BinaryOperatorType::BitshiftRight:
				expr->valueType = BinaryOperatorTypeMeet(resolver, expr->left->valueType, expr->right->valueType);
				expr->lvalue = false;
				return true;

			case AST::BinaryOperatorType::Assignment:
			case AST::BinaryOperatorType::PlusEquals:
			case AST::BinaryOperatorType::MinusEquals:
			case AST::BinaryOperatorType::TimesEquals:
			case AST::BinaryOperatorType::DividedByEquals:
			case AST::BinaryOperatorType::ModuloEquals:
			case AST::BinaryOperatorType::BitshiftLeftEquals:
			case AST::BinaryOperatorType::BitshiftRightEquals:
			case AST::BinaryOperatorType::BitwiseAndEquals:
			case AST::BinaryOperatorType::BitwiseOrEquals:
			case AST::BinaryOperatorType::BitwiseXorEquals:
				if (expr->left->lvalue)
				{
					if (!expr->left->isConstant())
					{
						if (expr->operatorType == AST::BinaryOperatorType::Assignment)
						{
							if (CanConvertImplicit(expr->right->valueType, expr->left->valueType, expr->right->isConstant()))
							{
								//expr->valueType = expr->left->valueType;
								//expr->lvalue = true;
								expr->valueType = GetVoidType();
								expr->lvalue = false;
								return true;
							}
							else
							{
								SnekErrorLoc(resolver->context, expr->left->location, "Can't assign value of type '%s' to variable of type '%s'", GetTypeString(expr->right->valueType), GetTypeString(expr->left->valueType));
								return false;
							}
						}
						else
						{
							TypeID resultingType = BinaryOperatorTypeMeet(resolver, expr->left->valueType, expr->right->valueType);
							SnekAssert(resultingType); // TODO handle
							expr->opAssignResultingType = resultingType;
							if (CanConvertImplicit(resultingType, expr->left->valueType, expr->right->isConstant()))
							{
								//expr->valueType = expr->left->valueType;
								//expr->lvalue = true;
								expr->valueType = GetVoidType();
								expr->lvalue = false;
								return true;
							}
							else
							{
								SnekErrorLoc(resolver->context, expr->left->location, "Can't assign value of type '%s' to variable of type '%s'", GetTypeString(resultingType), GetTypeString(expr->left->valueType));
								return false;
							}
						}
					}
					else
					{
						SnekErrorLoc(resolver->context, expr->left->location, "Can't assign to constant value");
						return false;
					}
				}
				else
				{
					SnekErrorLoc(resolver->context, expr->left->location, "Can't assign to a non lvalue");
					return false;
				}

			case AST::BinaryOperatorType::NullCoalescing:
				if (expr->left->valueType->typeKind == AST::TypeKind::Optional)
				{
					if (CanConvertImplicit(expr->right->valueType, expr->left->valueType->optionalType.elementType, expr->right->isConstant()))
					{
						expr->valueType = expr->left->valueType->optionalType.elementType;
						expr->lvalue = false;
						return true;
					}
					else
					{
						SnekErrorLoc(resolver->context, expr->left->location, "Right operand of null coalescing operator must be compatible with optional type '%s'", GetTypeString(expr->left->valueType));
						return false;
					}
				}
				else
				{
					SnekErrorLoc(resolver->context, expr->left->location, "Left operand of null coalescing operator must be an optional, is '%s'", GetTypeString(expr->left->valueType));
					return false;
				}

			default:
				SnekAssert(false);
				return false;
			}
		}
		else
		{
			SnekErrorLoc(resolver->context, resolver->currentElement->location, "Incompatible operands: '%s' and '%s'", GetTypeString(expr->left->valueType), GetTypeString(expr->right->valueType));
			return false;
		}
	}
	return false;
}

static bool ResolveTernaryOperator(Resolver* resolver, AST::TernaryOperator* expr)
{
	if (ResolveExpression(resolver, expr->condition) && ResolveExpression(resolver, expr->thenValue) && ResolveExpression(resolver, expr->elseValue))
	{
		TypeID type = BinaryOperatorTypeMeet(resolver, expr->thenValue->valueType, expr->elseValue->valueType);
		//if (CompareTypes(expr->thenValue->type, expr->elseValue->type))
		if (type)
		{
			expr->valueType = type;
			expr->lvalue = false;
			return true;
		}
		else
		{
			SnekErrorLoc(resolver->context, expr->location, "Values of ternary operator must be of the same type: %s and %s", GetTypeString(expr->thenValue->valueType), GetTypeString(expr->elseValue->valueType));
			return false;
		}
	}
	return false;
}

static bool ResolveExpression(Resolver* resolver, AST::Expression* expression)
{
	AST::Element* lastElement = resolver->currentElement;
	resolver->currentElement = expression;
	defer _(nullptr, [=](...) { resolver->currentElement = lastElement; });

	switch (expression->type)
	{
	case AST::ExpressionType::IntegerLiteral:
		return ResolveIntegerLiteral(resolver, (AST::IntegerLiteral*)expression);
	case AST::ExpressionType::FloatingPointLiteral:
		return ResolveFPLiteral(resolver, (AST::FloatingPointLiteral*)expression);
	case AST::ExpressionType::BooleanLiteral:
		return ResolveBoolLiteral(resolver, (AST::BooleanLiteral*)expression);
	case AST::ExpressionType::CharacterLiteral:
		return ResolveCharacterLiteral(resolver, (AST::CharacterLiteral*)expression);
	case AST::ExpressionType::NullLiteral:
		return ResolveNullLiteral(resolver, (AST::NullLiteral*)expression);
	case AST::ExpressionType::StringLiteral:
		return ResolveStringLiteral(resolver, (AST::StringLiteral*)expression);
	case AST::ExpressionType::InitializerList:
		return ResolveInitializerList(resolver, (AST::InitializerList*)expression);
	case AST::ExpressionType::Identifier:
		return ResolveIdentifier(resolver, (AST::Identifier*)expression);
	case AST::ExpressionType::Compound:
		return ResolveCompoundExpression(resolver, (AST::CompoundExpression*)expression);
	case AST::ExpressionType::Tuple:
		return ResolveTupleExpression(resolver, (AST::TupleExpression*)expression);

	case AST::ExpressionType::FunctionCall:
		return ResolveFunctionCall(resolver, (AST::FunctionCall*)expression);
	case AST::ExpressionType::SubscriptOperator:
		return ResolveSubscriptOperator(resolver, (AST::SubscriptOperator*)expression);
	case AST::ExpressionType::DotOperator:
		return ResolveDotOperator(resolver, (AST::DotOperator*)expression);
	case AST::ExpressionType::Typecast:
		return ResolveCast(resolver, (AST::Typecast*)expression);
	case AST::ExpressionType::Sizeof:
		return ResolveSizeof(resolver, (AST::Sizeof*)expression);
	case AST::ExpressionType::Malloc:
		return ResolveMalloc(resolver, (AST::Malloc*)expression);

	case AST::ExpressionType::UnaryOperator:
		return ResolveUnaryOperator(resolver, (AST::UnaryOperator*)expression);
	case AST::ExpressionType::BinaryOperator:
		return ResolveBinaryOperator(resolver, (AST::BinaryOperator*)expression);
	case AST::ExpressionType::TernaryOperator:
		return ResolveTernaryOperator(resolver, (AST::TernaryOperator*)expression);

	default:
		SnekAssert(false);
		return false;
	}
}

static bool ResolveStatement(Resolver* resolver, AST::Statement* statement);

static bool ResolveCompoundStatement(Resolver* resolver, AST::CompoundStatement* statement)
{
	bool result = true;

	resolver->pushScope("compound", statement->location);
	for (int i = 0; i < statement->statements.size; i++)
	{
		result = ResolveStatement(resolver, statement->statements[i]) && result;
	}
	resolver->popScope(statement->end);

	return result;
}

static bool ResolveExprStatement(Resolver* resolver, AST::ExpressionStatement* statement)
{
	bool result = ResolveExpression(resolver, statement->expression);
	if (statement->expression->type == AST::ExpressionType::FunctionCall)
	{
		AST::FunctionCall* functionCall = (AST::FunctionCall*)statement->expression;
		/*
		if (functionCall->function && functionCall->function->returnType && functionCall->function->returnType->typeKind != AST::TypeKind::Void)
		{
			SnekError(resolver->context, statement->location, "Return value of function call must not be dropped");
			return false;
		}
		*/
	}
	return result;
}

static bool ResolveVarDeclStatement(Resolver* resolver, AST::VariableDeclaration* statement)
{
	bool result = true;

	if (statement->varTypeAST)
	{
		result = ResolveType(resolver, statement->varTypeAST) && result;
		statement->varType = statement->varTypeAST->typeID;
	}
	for (int i = 0; i < statement->declarators.size; i++)
	{
		AST::VariableDeclarator* declarator = statement->declarators[i];

		// Check if variable with that name already exists in the current function
		if (Variable* variableWithSameName = resolver->findLocalVariableInScope(declarator->name, resolver->scope, true))
		{
			AST::Element* declaration = variableWithSameName->declaration;
			SnekErrorLoc(resolver->context, statement->location, "Variable '%s' already defined at %s:%d:%d", declarator->name, declaration->file->name, declaration->location.line, declaration->location.col);
			result = false;
		}

		if (declarator->value)
		{
			resolver->expectedType = statement->varType;
			if (ResolveExpression(resolver, declarator->value))
			{
				resolver->expectedType = nullptr;

				if (!statement->varType)
					statement->varType = declarator->value->valueType;

				if (declarator->value->type == AST::ExpressionType::InitializerList && statement->varType->typeKind == AST::TypeKind::Array)
				{
					AST::InitializerList* initializerList = (AST::InitializerList*)declarator->value;
					TypeID arrayType = statement->varType;
					initializerList->initializerType = arrayType;
					initializerList->valueType = arrayType;
				}
				// why was this here again?
				//else if (declarator->value->valueType && declarator->value->valueType->typeKind == AST::TypeKind::Pointer && CompareTypes(declarator->value->valueType->pointerType.elementType, statement->varType))
				//	;
				else if (!CanConvertImplicit(declarator->value->valueType, statement->varType, declarator->value->isConstant()))
				{
					SnekErrorLoc(resolver->context, statement->location, "Can't assign value of type '%s' to variable of type '%s'", GetTypeString(declarator->value->valueType), GetTypeString(statement->varType));
					result = false;
					//SnekError(resolver->context, statement->location, ERROR_CODE_INVALID_CAST, "Can't assign ");
				}
			}
			else
			{
				resolver->expectedType = nullptr;

				result = false;
			}
		}
		else
		{
			if (!statement->varType)
			{
				SnekErrorLoc(resolver->context, statement->location, "Variable '%s' with inferred type must have a default value", declarator->name);
			}
		}

		Variable* variable = new Variable(declarator->file, _strdup(declarator->name), _strdup(declarator->name), statement->varType, declarator->value, statement->isConstant, AST::Visibility::Null);
		declarator->variable = variable;
		resolver->registerLocalVariable(variable, statement);
	}

	return result;
}

static bool ResolveIfStatement(Resolver* resolver, AST::IfStatement* statement)
{
	bool result = true;

	result = ResolveExpression(resolver, statement->condition) && result;
	if (statement->condition->valueType && !(
		statement->condition->valueType->typeKind == AST::TypeKind::Boolean ||
		statement->condition->valueType->typeKind == AST::TypeKind::Integer ||
		statement->condition->valueType->typeKind == AST::TypeKind::Pointer ||
		statement->condition->valueType->typeKind == AST::TypeKind::Optional
		))
	{
		SnekErrorLoc(resolver->context, statement->condition->location, "If statement condition needs to be of boolean, integer or pointer type, is '%s'", GetTypeString(statement->condition->valueType));
		result = false;
	}

	if (result)
	{
		result = ResolveStatement(resolver, statement->thenStatement) && result;
		if (statement->elseStatement)
			result = ResolveStatement(resolver, statement->elseStatement) && result;
	}

	return result;
}

static bool ResolveWhileLoop(Resolver* resolver, AST::WhileLoop* statement)
{
	bool result = true;

	resolver->pushScope("while", statement->location);
	resolver->scope->branchDst = statement;

	result = ResolveExpression(resolver, statement->condition) && result;
	result = ResolveStatement(resolver, statement->body) && result;

	resolver->popScope(statement->end);

	return result;
}

static bool ResolveForLoop(Resolver* resolver, AST::ForLoop* statement)
{
	bool result = true;

	resolver->pushScope("for", statement->location);
	resolver->scope->branchDst = statement;

	if (!statement->iteratorName)
	{
		if (statement->initStatement)
			result = ResolveStatement(resolver, statement->initStatement) && result;
		if (statement->conditionExpr)
			result = ResolveExpression(resolver, statement->conditionExpr) && result;
		if (statement->iterateExpr)
			result = ResolveExpression(resolver, statement->iterateExpr) && result;

		/*
		if (statement->deltaValue)
		{
			result = ResolveExpression(resolver, statement->deltaValue) && result;

			if (statement->deltaValue->isConstant())
			{
				bool constantFoldSuccess;
				statement->delta = (int)ConstantFoldInt(resolver, statement->deltaValue, constantFoldSuccess);
				//statement->delta = (int)((AST::IntegerLiteral*)statement->deltaValue)->value;
			}
			else
			{
				SnekError(resolver->context, statement->deltaValue->location, ERROR_CODE_FOR_LOOP_SYNTAX, "For loop step value must be a constant integer");
				result = false;
			}
		}
		else
		{
			statement->delta = 1;
		}
		*/

		//statement->iterator = RegisterLocalVariable(resolver, GetIntegerType(32, true), statement->startValue, statement->iteratorName, false, resolver->file, statement->location);
		//Variable* iterator = new Variable(statement->file, _strdup(statement->iteratorName->name), _strdup(statement->iteratorName->name), GetIntegerType(32, true), statement->startValue, false, AST::Visibility::Null);
		//statement->iterator = iterator;
		//resolver->registerLocalVariable(iterator, statement->iteratorName);
	}
	else
	{
		SnekAssert(statement->iteratorName);
		SnekAssert(statement->container);

		result = ResolveExpression(resolver, statement->container) && result;

		TypeID iteratorType = nullptr;
		if (statement->container->valueType->typeKind == AST::TypeKind::Array)
		{
			iteratorType = statement->container->valueType->arrayType.elementType;
		}
		else
		{
			// TODO error
			SnekAssert(false);
		}

		Variable* variable = new Variable(resolver->currentFile, _strdup(statement->iteratorName), _strdup(statement->iteratorName), iteratorType, nullptr, true, AST::Visibility::Null);
		resolver->registerLocalVariable(variable, statement);
	}

	result = ResolveStatement(resolver, statement->body) && result;

	resolver->popScope(statement->end);

	return result;
}

static bool ResolveBreak(Resolver* resolver, AST::Break* statement)
{
	Scope* scope = resolver->scope;
	while (!scope->branchDst)
	{
		if (scope->parent)
			scope = scope->parent;
		else
			break;
	}
	if (scope->branchDst)
	{
		statement->branchDst = scope->branchDst;
		return true;
	}
	else
	{
		SnekErrorLoc(resolver->context, statement->location, "No loop structure to break out of");
		return false;
	}
}

static bool ResolveContinue(Resolver* resolver, AST::Continue* statement)
{
	Scope* scope = resolver->scope;
	while (!scope->branchDst)
	{
		if (scope->parent)
			scope = scope->parent;
		else
			break;
	}
	if (scope->branchDst)
	{
		statement->branchDst = scope->branchDst;
		return true;
	}
	else
	{
		SnekErrorLoc(resolver->context, statement->location, "No loop structure to continue");
		return false;
	}
}

static bool ResolveReturn(Resolver* resolver, AST::Return* statement)
{
	if (statement->value)
	{
		resolver->expectedType = resolver->currentFunction->returnType->typeID;
		if (ResolveExpression(resolver, statement->value))
		{
			resolver->expectedType = nullptr;

			AST::Function* currentFunction = resolver->currentFunction;
			TypeID returnType = currentFunction->returnType ? currentFunction->returnType->typeID : GetVoidType();

			if (CanConvertImplicit(statement->value->valueType, returnType, statement->value->isLiteral()))
			{
				return true;
			}
			else
			{
				SnekErrorLoc(resolver->context, statement->location, "Can't return value of type %s from function with return type %s", GetTypeString(statement->value->valueType), GetTypeString(returnType));
				return false;
			}
		}
		else
		{
			resolver->expectedType = nullptr;
			return false;
		}
	}

	return true;
}

static bool ResolveAssert(Resolver* resolver, AST::Assert* statement)
{
	if (ResolveExpression(resolver, statement->condition))
	{
		if (!(
			statement->condition->valueType->typeKind == AST::TypeKind::Boolean ||
			statement->condition->valueType->typeKind == AST::TypeKind::Integer ||
			statement->condition->valueType->typeKind == AST::TypeKind::Pointer ||
			statement->condition->valueType->typeKind == AST::TypeKind::Optional
			))
		{
			SnekErrorLoc(resolver->context, statement->condition->location, "If statement condition needs to be of boolean, integer or pointer type");
			return false;
		}

		if (statement->message)
		{
			if (ResolveExpression(resolver, statement->message))
			{
				if (statement->message->valueType->typeKind != AST::TypeKind::String)
				{
					SnekErrorLoc(resolver->context, statement->message->location, "Second argument to assertion (message) must be of type string");
					return false;
				}
			}
		}

		return true;
	}
	return false;
}

static bool ResolveFree(Resolver* resolver, AST::Free* statement)
{
	bool result = true;

	for (int i = 0; i < statement->values.size; i++)
	{
		result = ResolveExpression(resolver, statement->values[i]) && result;
	}

	return result;
}

static bool ResolveStatement(Resolver* resolver, AST::Statement* statement)
{
	AST::Element* lastElement = resolver->currentElement;
	resolver->currentElement = statement;
	defer _(nullptr, [=](...) { resolver->currentElement = lastElement; });

	switch (statement->type)
	{
	case AST::StatementType::NoOp:
		return true;
	case AST::StatementType::Compound:
		return ResolveCompoundStatement(resolver, (AST::CompoundStatement*)statement);
	case AST::StatementType::Expression:
		return ResolveExprStatement(resolver, (AST::ExpressionStatement*)statement);
	case AST::StatementType::VariableDeclaration:
		return ResolveVarDeclStatement(resolver, (AST::VariableDeclaration*)statement);
	case AST::StatementType::If:
		return ResolveIfStatement(resolver, (AST::IfStatement*)statement);
	case AST::StatementType::While:
		return ResolveWhileLoop(resolver, (AST::WhileLoop*)statement);
	case AST::StatementType::For:
		return ResolveForLoop(resolver, (AST::ForLoop*)statement);
	case AST::StatementType::Break:
		return ResolveBreak(resolver, (AST::Break*)statement);
	case AST::StatementType::Continue:
		return ResolveContinue(resolver, (AST::Continue*)statement);
	case AST::StatementType::Return:
		return ResolveReturn(resolver, (AST::Return*)statement);
	case AST::StatementType::Assert:
		return ResolveAssert(resolver, (AST::Assert*)statement);
	case AST::StatementType::Free:
		return ResolveFree(resolver, (AST::Free*)statement);

	default:
		SnekAssert(false);
		return false;
	}
}

static AST::Visibility GetVisibilityFromFlags(AST::DeclarationFlags flags)
{
	if (HasFlag(flags, AST::DeclarationFlags::Public))
		return AST::Visibility::Public;
	else if (HasFlag(flags, AST::DeclarationFlags::Private))
		return AST::Visibility::Private;
	else
		return AST::Visibility::Public;
}

static bool CheckEntrypointDecl(Resolver* resolver, AST::Function* function)
{
	if (function->visibility != AST::Visibility::Public)
	{
		SnekErrorLoc(resolver->context, function->location, "Entry point must be public");
		return false;
	}
	if (!(!function->returnType || function->returnType->typeKind == AST::TypeKind::Void || function->returnType->typeKind == AST::TypeKind::Integer && function->returnType->typeID->integerType.bitWidth == 32 && function->returnType->typeID->integerType.isSigned))
	{
		SnekErrorLoc(resolver->context, function->location, "Entry point must return void or int");
		return false;
	}
	if (function->paramTypes.size > 0)
	{
		SnekErrorLoc(resolver->context, function->location, "Entry point must not have parameters");
		return false;
	}
	return true;
}

static bool ResolveFunctionHeader(Resolver* resolver, AST::Function* decl)
{
	AST::Element* lastElement = resolver->currentElement;
	resolver->currentElement = decl;
	defer _(nullptr, [=](...) { resolver->currentElement = lastElement; });

	AST::Function* lastFunction = resolver->currentFunction;
	resolver->currentFunction = decl;

	bool result = true;

	decl->visibility = GetVisibilityFromFlags(decl->flags);

	if (strcmp(decl->name, ENTRY_POINT_NAME) == 0)
	{
		decl->isEntryPoint = true;
		resolver->entryPoint = decl;
	}

	if (decl->isGeneric)
	{
	}
	else
	{
		if (decl->returnType)
			result = ResolveType(resolver, decl->returnType) && result;

		bool defaultArgumentExists = false;
		for (int i = 0; i < decl->paramTypes.size; i++)
		{
			result = ResolveType(resolver, decl->paramTypes[i]) && result;

			if (AST::Expression* paramValue = decl->paramValues[i])
			{
				result = result && ResolveExpression(resolver, paramValue);

				defaultArgumentExists = true;

				if (!paramValue->isConstant())
				{
					SnekErrorLoc(resolver->context, paramValue->location, "Default value for parameter '%s' must be a constant", decl->paramNames[i]);
					result = false;
				}
			}
			else
			{
				if (defaultArgumentExists)
				{
					SnekErrorLoc(resolver->context, decl->paramTypes[i]->location, "Parameter '%s' is following a parameter with a default argument and therefore must also have a default argument");
					result = false;
				}
			}
		}

		if (decl->varArgs && decl->varArgsTypeAST)
		{
			result = ResolveType(resolver, decl->varArgsTypeAST) && result;
			decl->varArgsType = decl->varArgsTypeAST->typeID;
		}

		TypeID returnType = nullptr;
		int numParams = decl->paramTypes.size;
		TypeID* paramTypes = new TypeID[numParams];
		for (int i = 0; i < numParams; i++)
		{
			paramTypes[i] = decl->paramTypes[i]->typeID;
		}

		if (decl->returnType)
			returnType = decl->returnType->typeID;
		else if (decl->bodyExpression)
		{
			result = ResolveExpression(resolver, decl->bodyExpression) && result;
			returnType = decl->bodyExpression->valueType;
		}
		else
		{
			returnType = GetVoidType();
		}

		decl->functionType = GetFunctionType(returnType, numParams, paramTypes, decl->varArgs, decl->varArgsType, false, nullptr, decl);

		if (decl->isEntryPoint)
		{
			result = CheckEntrypointDecl(resolver, decl) && result;
		}

		if (result)
		{
			char* mangledName = MangleFunctionName(decl);
			List<AST::Function*> overloadsWithSameName;
			resolver->findFunctionsInModule(decl->name, decl->file->module, overloadsWithSameName);
			for (int i = 0; i < overloadsWithSameName.size; i++)
			{
				if (overloadsWithSameName[i]->mangledName && strcmp(mangledName, overloadsWithSameName[i]->mangledName) == 0 && overloadsWithSameName[i]->body && decl->body)
				{
					SnekErrorLoc(resolver->context, decl->location, "Function '%s' of type %s already defined at %s", decl->name, GetTypeString(decl->functionType), SourceLocationToString(overloadsWithSameName[i]->location));
					result = false;
					break;
				}
			}
			decl->mangledName = mangledName;
		}
	}

	resolver->currentFunction = lastFunction;

	return result;
}

static bool ResolveFunction(Resolver* resolver, AST::Function* decl)
{
	AST::Element* lastElement = resolver->currentElement;
	resolver->currentElement = decl;
	defer _(nullptr, [=](...) { resolver->currentElement = lastElement; });

	bool result = true;

	if (HasFlag(decl->flags, AST::DeclarationFlags::Extern))
	{
		if (decl->body)
		{
			SnekErrorLoc(resolver->context, decl->location, "Extern function '%s' cannot have a body", decl->name);
			return false;
		}
	}

	if (decl->isGeneric)
	{
	}
	else
	{
		if (decl->body)
		{
			AST::Function* lastFunction = resolver->currentFunction;
			resolver->currentFunction = decl;

			resolver->pushScope(decl->name, decl->location);

			//decl->paramVariables.resize(decl->paramTypes.size);

			for (int i = 0; i < decl->paramTypes.size; i++)
			{
				Variable* variable = new Variable(decl->file, _strdup(decl->paramNames[i]), _strdup(decl->paramNames[i]), decl->paramTypes[i]->typeID, nullptr, false, AST::Visibility::Null);
				//decl->paramVariables[i] = variable;
				resolver->registerLocalVariable(variable, decl->paramTypes[i]);
				//RegisterLocalVariable(resolver, decl->paramTypes[i]->typeID, NULL, decl->paramNames[i], false, resolver->file, decl->paramTypes[i]->location);
			}

			if (decl->varArgs)
			{
				TypeID varArgsArrayType = GetArrayType(decl->varArgsType, -1);
				Variable* varArgsVariable = new Variable(decl->file, _strdup(decl->varArgsName), _strdup(decl->varArgsName), varArgsArrayType, nullptr, false, AST::Visibility::Null);
				resolver->registerLocalVariable(varArgsVariable, decl->varArgsTypeAST);
			}

			result = ResolveStatement(resolver, decl->body) && result;

			resolver->popScope(decl->endLocation);

			resolver->currentFunction = lastFunction;
		}
	}

	return result;
}

static bool ResolveStructHeader(Resolver* resolver, AST::Struct* decl)
{
	AST::Element* lastElement = resolver->currentElement;
	resolver->currentElement = decl;
	defer _(nullptr, [=](...) { resolver->currentElement = lastElement; });

	decl->visibility = GetVisibilityFromFlags(decl->flags);

	if (decl->isGeneric)
	{
	}
	else
	{
		decl->mangledName = MangleStructName(decl);
		decl->type = GetStructType(decl->mangledName, decl);
	}

	return true;
}

static bool ResolveStructField(Resolver* resolver, AST::StructField* field, TypeID* fieldType, const char** fieldName)
{
	if (field->isStruct)
	{
		if (field->name && fieldType)
		{
			TypeID* structFieldTypes = new TypeID[field->structFields.size];
			for (int i = 0; i < field->structFields.size; i++)
			{
				ResolveStructField(resolver, field->structFields[i], &structFieldTypes[i], nullptr);
			}
			*fieldType = GetStructType(field->structFields.size, structFieldTypes);
		}
		else
		{
			for (int i = 0; i < field->structFields.size; i++)
			{
				ResolveStructField(resolver, field->structFields[i], nullptr, nullptr);
			}
		}
	}
	else if (field->isUnion)
	{
		for (int i = 0; i < field->unionFields.size; i++)
		{
			ResolveStructField(resolver, field->unionFields[i], nullptr, nullptr);
		}
	}
	else if (ResolveType(resolver, field->type))
	{
		if (fieldType)
			*fieldType = field->type->typeID;
	}
	else
	{
		return false;
	}

	if (fieldName)
		*fieldName = field->name;

	return true;
}

static bool ResolveStruct(Resolver* resolver, AST::Struct* decl)
{
	AST::Element* lastElement = resolver->currentElement;
	resolver->currentElement = decl;
	defer _(nullptr, [=](...) { resolver->currentElement = lastElement; });

	bool result = true;

	if (decl->isGeneric)
	{
	}
	else
	{
		decl->type->structType.hasBody = decl->hasBody;

		if (decl->hasBody)
		{
			AST::Struct* lastStruct = resolver->currentStruct;
			resolver->currentStruct = decl;

			int numFields = decl->fields.size;
			TypeID* fieldTypes = new TypeID[numFields];
			const char** fieldNames = new const char* [numFields];
			for (int i = 0; i < numFields; i++)
			{
				ResolveStructField(resolver, decl->fields[i], &fieldTypes[i], &fieldNames[i]);
			}
			decl->type->structType.numFields = numFields;
			decl->type->structType.fieldTypes = fieldTypes;
			decl->type->structType.fieldNames = fieldNames;

			resolver->currentStruct = lastStruct;
		}
		else
		{
			decl->type->structType.numFields = 0;
			decl->type->structType.fieldTypes = NULL;
			decl->type->structType.fieldNames = NULL;
		}
	}

	return result;
}

static bool ResolveClassHeader(Resolver* resolver, AST::Class* decl)
{
	AST::Element* lastElement = resolver->currentElement;
	resolver->currentElement = decl;
	defer _(nullptr, [=](...) { resolver->currentElement = lastElement; });

	decl->visibility = GetVisibilityFromFlags(decl->flags);

	if (decl->isGeneric)
	{
	}
	else
	{
		decl->mangledName = MangleClassName(decl);
		decl->type = GetClassType(decl->mangledName, decl);
	}

	return true;
}

static bool ResolveClassMethodHeader(Resolver* resolver, AST::Method* method, AST::Class* decl)
{
	AST::Element* lastElement = resolver->currentElement;
	resolver->currentElement = decl;
	defer _(nullptr, [=](...) { resolver->currentElement = lastElement; });

	bool result = true;

	method->mangledName = _strdup(method->name);

	result = ResolveType(resolver, method->returnType) && result;
	for (int i = 0; i < method->paramTypes.size; i++)
	{
		result = ResolveType(resolver, method->paramTypes[i]) && result;
	}

	int numParams = method->paramTypes.size;
	TypeID returnType = method->returnType->typeID;
	TypeID* paramTypes = new TypeID[numParams];
	for (int i = 0; i < numParams; i++)
	{
		paramTypes[i] = method->paramTypes[i]->typeID;
	}

	if (method->varArgsTypeAST)
	{
		result = ResolveType(resolver, method->varArgsTypeAST) && result;
		method->varArgsType = method->varArgsTypeAST->typeID;
	}

	method->instanceType = decl->type;
	method->functionType = GetFunctionType(returnType, numParams, paramTypes, method->varArgs, method->varArgsType, true, decl->type, method);

	return result;
}

static bool ResolveClassConstructorHeader(Resolver* resolver, AST::Constructor* constructor, AST::Class* decl)
{
	AST::Element* lastElement = resolver->currentElement;
	resolver->currentElement = decl;
	defer _(nullptr, [=](...) { resolver->currentElement = lastElement; });

	constructor->name = _strdup(decl->type->classType.name);
	constructor->mangledName = _strdup(constructor->name);

	bool result = true;

	result = ResolveType(resolver, constructor->returnType) && result;
	for (int i = 0; i < constructor->paramTypes.size; i++)
	{
		result = ResolveType(resolver, constructor->paramTypes[i]) && result;
	}

	int numParams = constructor->paramTypes.size;
	TypeID returnType = constructor->returnType->typeID;
	TypeID* paramTypes = new TypeID[numParams];
	for (int i = 0; i < numParams; i++)
	{
		paramTypes[i] = constructor->paramTypes[i]->typeID;
	}

	if (constructor->varArgsTypeAST)
	{
		result = ResolveType(resolver, constructor->varArgsTypeAST) && result;
		constructor->varArgsType = constructor->varArgsTypeAST->typeID;
	}

	constructor->instanceType = decl->type;
	constructor->functionType = GetFunctionType(returnType, numParams, paramTypes, constructor->varArgs, constructor->varArgsType, true, decl->type, constructor);

	return result;
}

static bool ResolveClassProcedureHeaders(Resolver* resolver, AST::Class* decl)
{
	AST::Element* lastElement = resolver->currentElement;
	resolver->currentElement = decl;
	defer _(nullptr, [=](...) { resolver->currentElement = lastElement; });

	bool result = true;

	int numMethods = decl->methods.size;
	for (int i = 0; i < numMethods; i++)
	{
		AST::Method* method = decl->methods[i];
		result = ResolveClassMethodHeader(resolver, method, decl) && result;
	}

	if (decl->constructor)
	{
		result = ResolveClassConstructorHeader(resolver, decl->constructor, decl) && result;
	}

	return result;
}

static bool ResolveClass(Resolver* resolver, AST::Class* decl)
{
	AST::Element* lastElement = resolver->currentElement;
	resolver->currentElement = decl;
	defer _(nullptr, [=](...) { resolver->currentElement = lastElement; });

	bool result = true;

	if (decl->isGeneric)
	{
	}
	else
	{
		AST::Class* lastClass = resolver->currentClass;
		resolver->currentClass = decl;

		int numFields = decl->fields.size;
		TypeID* fieldTypes = new TypeID[numFields];
		const char** fieldNames = new const char* [numFields];
		for (int i = 0; i < numFields; i++)
		{
			AST::ClassField* field = decl->fields[i];
			if (ResolveType(resolver, field->type))
			{
				fieldTypes[i] = decl->fields[i]->type->typeID;
			}
			else
			{
				result = false;
			}
			fieldNames[i] = decl->fields[i]->name;
		}
		decl->type->classType.numFields = numFields;
		decl->type->classType.fieldTypes = fieldTypes;
		decl->type->classType.fieldNames = fieldNames;

		resolver->currentClass = lastClass;
	}

	return result;
}

static bool ResolveClassMethod(Resolver* resolver, AST::Function* decl)
{
	AST::Element* lastElement = resolver->currentElement;
	resolver->currentElement = decl;
	defer _(nullptr, [=](...) { resolver->currentElement = lastElement; });

	bool result = true;

	AST::Function* lastFunction = resolver->currentFunction;
	resolver->currentFunction = decl;

	resolver->pushScope(decl->name, decl->location);

	//decl->paramVariables.resize(decl->paramTypes.size);

	//decl->instanceVariable = RegisterLocalVariable(resolver, decl->instanceType, NULL, "this", false, resolver->file, decl->location);
	Variable* instanceVariable = new Variable(decl->file, _strdup("this"), _strdup("this"), decl->returnType->typeID, nullptr, false, AST::Visibility::Null);
	decl->instanceVariable = instanceVariable;
	resolver->registerLocalVariable(instanceVariable, decl);

	for (int i = 0; i < decl->paramTypes.size; i++)
	{
		//Variable* variable = RegisterLocalVariable(resolver, decl->paramTypes[i]->typeID, NULL, decl->paramNames[i], false, resolver->file, decl->paramTypes[i]->location);
		Variable* variable = new Variable(decl->file, _strdup(decl->paramNames[i]), _strdup(decl->paramNames[i]), decl->paramTypes[i]->typeID, nullptr, false, AST::Visibility::Null);
		//decl->paramVariables[i] = variable;
		resolver->registerLocalVariable(variable, decl->paramTypes[i]);
	}

	if (decl->varArgs)
	{
		TypeID varArgsArrayType = GetArrayType(decl->varArgsType, -1);
		Variable* varArgsVariable = new Variable(decl->file, _strdup(decl->varArgsName), _strdup(decl->varArgsName), varArgsArrayType, nullptr, false, AST::Visibility::Null);
		resolver->registerLocalVariable(varArgsVariable, decl->varArgsTypeAST);
	}

	result = ResolveStatement(resolver, decl->body) && result;

	resolver->popScope(decl->endLocation);

	resolver->currentFunction = lastFunction;

	return result;
}

static bool ResolveClassConstructor(Resolver* resolver, AST::Constructor* decl)
{
	AST::Element* lastElement = resolver->currentElement;
	resolver->currentElement = decl;
	defer _(nullptr, [=](...) { resolver->currentElement = lastElement; });

	bool result = true;

	AST::Function* lastFunction = resolver->currentFunction;
	resolver->currentFunction = decl;

	resolver->pushScope(decl->name, decl->location);

	//decl->paramVariables.resize(decl->paramTypes.size);

	//decl->instanceVariable = RegisterLocalVariable(resolver, decl->instanceType, NULL, "this", false, resolver->file, decl->location);
	Variable* instanceVariable = new Variable(decl->file, _strdup("this"), _strdup("this"), decl->instanceType, nullptr, false, AST::Visibility::Null);
	decl->instanceVariable = instanceVariable;
	resolver->registerLocalVariable(instanceVariable, decl);
	for (int i = 0; i < decl->paramTypes.size; i++)
	{
		//Variable* variable = RegisterLocalVariable(resolver, decl->paramTypes[i]->typeID, NULL, decl->paramNames[i], false, resolver->file, decl->location);
		Variable* variable = new Variable(decl->file, _strdup(decl->paramNames[i]), _strdup(decl->paramNames[i]), decl->paramTypes[i]->typeID, nullptr, false, AST::Visibility::Null);
		//decl->paramVariables[i] = variable;
		resolver->registerLocalVariable(variable, decl->paramTypes[i]);
	}

	if (decl->varArgs)
	{
		TypeID varArgsArrayType = GetArrayType(decl->varArgsType, -1);
		Variable* varArgsVariable = new Variable(decl->file, _strdup(decl->varArgsName), _strdup(decl->varArgsName), varArgsArrayType, nullptr, false, AST::Visibility::Null);
		resolver->registerLocalVariable(varArgsVariable, decl->varArgsTypeAST);
	}

	result = ResolveStatement(resolver, decl->body) && result;

	resolver->popScope(decl->endLocation);

	resolver->currentFunction = lastFunction;

	return result;
}

static bool ResolveClassProcedures(Resolver* resolver, AST::Class* decl)
{
	AST::Element* lastElement = resolver->currentElement;
	resolver->currentElement = decl;
	defer _(nullptr, [=](...) { resolver->currentElement = lastElement; });

	bool result = true;

	int numMethods = decl->methods.size;
	for (int i = 0; i < numMethods; i++)
	{
		AST::Function* method = decl->methods[i];
		result = ResolveClassMethod(resolver, method) && result;
	}

	if (decl->constructor)
	{
		result = ResolveClassConstructor(resolver, decl->constructor) && result;
	}

	return result;
}

static bool ResolveTypedefHeader(Resolver* resolver, AST::Typedef* decl)
{
	AST::Element* lastElement = resolver->currentElement;
	resolver->currentElement = decl;
	defer _(nullptr, [=](...) { resolver->currentElement = lastElement; });

	decl->visibility = GetVisibilityFromFlags(decl->flags);
	decl->type = GetAliasType(decl->name, decl);

	bool result = true;

	result = ResolveType(resolver, decl->alias) && result;
	decl->type->aliasType.alias = decl->alias->typeID;

	return true;
}

static bool ResolveTypedef(Resolver* resolver, AST::Typedef* decl)
{
	AST::Element* lastElement = resolver->currentElement;
	resolver->currentElement = decl;
	defer _(nullptr, [=](...) { resolver->currentElement = lastElement; });

	/*
	bool result = true;

	result = ResolveType(resolver, decl->alias) && result;
	decl->type->aliasType.alias = decl->alias->typeID;

	return result;
	*/
	return true;
}

static bool ResolveEnumHeader(Resolver* resolver, AST::Enum* decl)
{
	AST::Element* lastElement = resolver->currentElement;
	resolver->currentElement = decl;
	defer _(nullptr, [=](...) { resolver->currentElement = lastElement; });

	decl->visibility = GetVisibilityFromFlags(decl->flags);
	decl->type = GetAliasType(decl->name, decl);
	decl->type->aliasType.alias = GetIntegerType(32, true);

	bool result = true;

	for (int i = 0; i < decl->values.size; i++)
	{
		decl->values[i]->declaration = decl;
		if (decl->values[i]->value)
		{
			if (!ResolveExpression(resolver, decl->values[i]->value))
			{
				result = false;
			}
		}
	}

	return true;
}

static bool ResolveEnum(Resolver* resolver, AST::Enum* decl)
{
	return true;
}

static bool ResolveExprdefHeader(Resolver* resolver, AST::Macro* decl)
{
	AST::Element* lastElement = resolver->currentElement;
	resolver->currentElement = decl;
	defer _(nullptr, [=](...) { resolver->currentElement = lastElement; });

	decl->visibility = GetVisibilityFromFlags(decl->flags);

	return true;
}

static bool ResolveGlobalHeader(Resolver* resolver, AST::GlobalVariable* decl)
{
	AST::Element* lastElement = resolver->currentElement;
	resolver->currentElement = decl;
	defer _(nullptr, [=](...) { resolver->currentElement = lastElement; });

	bool result = true;

	result = ResolveType(resolver, decl->varType) && result;

	AST::Visibility visibility = GetVisibilityFromFlags(decl->flags);
	decl->visibility = visibility;
	for (int i = 0; i < decl->declarators.size; i++)
	{
		AST::VariableDeclarator* declarator = decl->declarators[i];
		//decl->variable = RegisterGlobalVariable(resolver, decl->type->typeID, decl->value, decl->name, decl->flags & DECL_FLAG_CONSTANT, visibility, resolver->file, decl);
		Variable* variable = new Variable(decl->file, _strdup(declarator->name), MangleGlobalName(decl, declarator), decl->varType->typeID, declarator->value, HasFlag(decl->flags, AST::DeclarationFlags::Constant), visibility);
		resolver->registerGlobalVariable(variable, decl);
		declarator->variable = variable;
	}

	return result;
}

static bool ResolveGlobal(Resolver* resolver, AST::GlobalVariable* decl)
{
	AST::Element* lastElement = resolver->currentElement;
	resolver->currentElement = decl;
	defer _(nullptr, [=](...) { resolver->currentElement = lastElement; });

	bool result = true;

	for (int i = 0; i < decl->declarators.size; i++)
	{
		AST::VariableDeclarator* declarator = decl->declarators[i];
		if (declarator->value)
		{
			if (ResolveExpression(resolver, declarator->value))
			{
				if (!CanConvertImplicit(declarator->value->valueType, decl->varType->typeID, declarator->value->isConstant()))
				{
					SnekErrorLoc(resolver->context, decl->location, "Can't initialize global variable '%s' of type %s with value of type %s", declarator->name, GetTypeString(decl->varType->typeID), GetTypeString(declarator->value->valueType));
					result = false;
				}
				if (HasFlag(decl->flags, AST::DeclarationFlags::Extern))
				{
					SnekErrorLoc(resolver->context, decl->location, "Extern global variable '%s' cannot have an initializer", declarator->name);
					result = false;
				}
				else if (!declarator->value->isConstant())
				{
					SnekErrorLoc(resolver->context, decl->location, "Initializer of global variable '%s' must be a constant value", declarator->name);
					result = false;
				}
				if (declarator->value->type == AST::ExpressionType::InitializerList)
				{
					AST::InitializerList* initializerList = (AST::InitializerList*)declarator->value;
					initializerList->initializerType = decl->varType->typeID;
					initializerList->valueType = decl->varType->typeID;
				}
			}
			else
			{
				result = false;
			}
		}
	}

	return result;
}

static bool ResolveModuleDecl(Resolver* resolver, AST::ModuleDeclaration* decl)
{
	AST::Element* lastElement = resolver->currentElement;
	resolver->currentElement = decl;
	defer _(nullptr, [=](...) { resolver->currentElement = lastElement; });

	AST::Module* parent = resolver->globalModule;
	for (int i = 0; i < decl->identifier.namespaces.size; i++)
	{
		const char* name = decl->identifier.namespaces[i];
		AST::Module* module = FindModule(resolver, name, parent);
		if (!module)
		{
			module = CreateModule(resolver, name, parent);
		}
		parent = module;
	}

	//decl->module = CreateModule(resolver, decl->file->name, parent);
	decl->module = parent;

	return true;
}

static void AddModuleDependency(AST::File* file, AST::Module* module, bool recursive)
{
	if (!file->dependencies.contains(module))
		file->dependencies.add(module);
	if (recursive)
	{
		for (int i = 0; i < module->children.size; i++)
		{
			AddModuleDependency(file, module->children[i], recursive);
		}
	}
}

static bool ResolveImport(Resolver* resolver, AST::Import* decl)
{
	AST::Element* lastElement = resolver->currentElement;
	resolver->currentElement = decl;
	defer _(nullptr, [=](...) { resolver->currentElement = lastElement; });

	bool result = true;

	for (int i = 0; i < decl->imports.size; i++)
	{
		AST::ModuleIdentifier& identifier = decl->imports[i];
		AST::Module* parent = resolver->globalModule;
		for (int j = 0; j < identifier.namespaces.size; j++)
		{
			const char* name = identifier.namespaces[j];
			if (strcmp(name, "*") == 0)
			{
				if (j == identifier.namespaces.size - 1)
				{
					for (int k = 0; k < parent->children.size; k++)
					{
						AddModuleDependency(resolver->currentFile, parent->children[k], false);
					}
					break;
				}
				else
				{
					SnekErrorLoc(resolver->context, decl->location, "For bulk imports, * must be at the end of the import declaration");
					result = false;
					break;
				}
			}
			else if (strcmp(name, "**") == 0)
			{
				if (j == identifier.namespaces.size - 1)
				{
					AddModuleDependency(resolver->currentFile, parent, true);
					break;
				}
				else
				{
					SnekErrorLoc(resolver->context, decl->location, "For recursive bulk imports, ** must be at the end of the import declaration");
					result = false;
					break;
				}
			}
			else
			{
				AST::Module* module = nullptr;

				if (!module)
				{
					module = FindModule(resolver, name, parent);
				}

				if (module)
				{
					if (j == identifier.namespaces.size - 1)
					{
						AddModuleDependency(resolver->currentFile, module, false);
						break;
					}
					else
					{
						parent = module;
					}
				}
				else
				{
					char module_name[64];
					module_name[0] = '\0';
					for (int k = 0; k <= j; k++)
					{
						strcat(module_name, identifier.namespaces[k]);
						if (k < j)
							strcat(module_name, ".");
					}

					SnekErrorLoc(resolver->context, decl->location, "Unknown module '%s'", module_name);
					result = false;
					parent = NULL;
					break;
				}
			}
		}
	}

	return result;
}

static bool ResolveModuleHeaders(Resolver* resolver)
{
	bool result = true;

	for (int i = 0; i < resolver->asts.size; i++)
	{
		AST::File* ast = resolver->asts[i];
		resolver->currentFile = ast;

		if (ast->moduleDecl)
		{
			result = ResolveModuleDecl(resolver, ast->moduleDecl) && result;
			ast->module = ast->moduleDecl->module;
		}
		else
		{
			//ast->module = CreateModule(resolver, ast->name, resolver->globalModule);
			ast->module = resolver->globalModule;
		}
		if (ast->namespaceDecl)
		{
			//result = ResolveNamespaceDecl(resolver, ast->namespaceDecl) && result;
			ast->nameSpace = ast->namespaceDecl->name;
		}

		ast->module->files.add(ast);
	}

	/*
	AST::Module* consoleModule = FindModule(resolver, "console", FindModule(resolver, "snek", resolver->globalModule));
	for (int i = 0; i < resolver->asts.size; i++)
	{
		AST::File* ast = resolver->asts[i];
		if (ast->module != consoleModule)
			AddModuleDependency(ast, consoleModule, false);
	}
	*/

	for (int i = 0; i < resolver->asts.size; i++)
	{
		AST::File* ast = resolver->asts[i];
		resolver->currentFile = ast;
		for (int j = 0; j < ast->imports.size; j++)
		{
			result = ResolveImport(resolver, ast->imports[j]) && result;
		}
	}
	for (int i = 0; i < resolver->asts.size; i++)
	{
		AST::File* ast = resolver->asts[i];
		resolver->currentFile = ast;
		for (int j = 0; j < ast->globals.size; j++)
		{
			bool isPrimitiveConstant = HasFlag(ast->globals[j]->flags, AST::DeclarationFlags::Constant) && IsPrimitiveType(ast->globals[j]->varType->typeKind);
			if (isPrimitiveConstant)
				result = ResolveGlobalHeader(resolver, ast->globals[j]) && result;
		}
	}
	for (int i = 0; i < resolver->asts.size; i++)
	{
		AST::File* ast = resolver->asts[i];
		resolver->currentFile = ast;
		for (int j = 0; j < ast->enums.size; j++)
		{
			result = ResolveEnumHeader(resolver, ast->enums[j]) && result;
		}
	}
	for (int i = 0; i < resolver->asts.size; i++)
	{
		AST::File* ast = resolver->asts[i];
		resolver->currentFile = ast;
		for (int j = 0; j < ast->structs.size; j++)
		{
			result = ResolveStructHeader(resolver, ast->structs[j]) && result;
		}
	}
	for (int i = 0; i < resolver->asts.size; i++)
	{
		AST::File* ast = resolver->asts[i];
		resolver->currentFile = ast;
		for (int j = 0; j < ast->classes.size; j++)
		{
			result = ResolveClassHeader(resolver, ast->classes[j]) && result;
		}
	}
	for (int i = 0; i < resolver->asts.size; i++)
	{
		AST::File* ast = resolver->asts[i];
		resolver->currentFile = ast;
		for (int j = 0; j < ast->typedefs.size; j++)
		{
			result = ResolveTypedefHeader(resolver, ast->typedefs[j]) && result;
		}
	}
	for (int i = 0; i < resolver->asts.size; i++)
	{
		AST::File* ast = resolver->asts[i];
		resolver->currentFile = ast;
		for (int j = 0; j < ast->macros.size; j++)
		{
			result = ResolveExprdefHeader(resolver, ast->macros[j]) && result;
		}
	}
	for (int i = 0; i < resolver->asts.size; i++)
	{
		AST::File* ast = resolver->asts[i];
		resolver->currentFile = ast;
		for (int j = 0; j < ast->functions.size; j++)
		{
			result = ResolveFunctionHeader(resolver, ast->functions[j]) && result;
		}
		for (int j = 0; j < ast->classes.size; j++)
		{
			result = ResolveClassProcedureHeaders(resolver, ast->classes[j]) && result;
		}
	}
	for (int i = 0; i < resolver->asts.size; i++)
	{
		AST::File* ast = resolver->asts[i];
		resolver->currentFile = ast;
		for (int j = 0; j < ast->globals.size; j++)
		{
			bool isPrimitiveConstant = HasFlag(ast->globals[j]->flags, AST::DeclarationFlags::Constant) && IsPrimitiveType(ast->globals[j]->varType->typeKind);
			if (!isPrimitiveConstant)
				result = ResolveGlobalHeader(resolver, ast->globals[j]) && result;
		}
	}

	return result;
}

static bool ResolveModules(Resolver* resolver)
{
	bool result = true;

	for (int i = 0; i < resolver->asts.size; i++)
	{
		AST::File* ast = resolver->asts[i];
		resolver->currentFile = ast;
		for (int j = 0; j < ast->globals.size; j++)
		{
			bool isPrimitiveConstant = HasFlag(ast->globals[j]->flags, AST::DeclarationFlags::Constant) && IsPrimitiveType(ast->globals[j]->varType->typeKind);
			if (isPrimitiveConstant)
				result = ResolveGlobal(resolver, ast->globals[j]) && result;
		}
	}

	for (int i = 0; i < resolver->asts.size; i++)
	{
		AST::File* ast = resolver->asts[i];
		resolver->currentFile = ast;
		for (int j = 0; j < ast->enums.size; j++)
		{
			result = ResolveEnum(resolver, ast->enums[j]) && result;
		}
	}
	for (int i = 0; i < resolver->asts.size; i++)
	{
		AST::File* ast = resolver->asts[i];
		resolver->currentFile = ast;
		for (int j = 0; j < ast->structs.size; j++)
		{
			result = ResolveStruct(resolver, ast->structs[j]) && result;
		}
	}
	for (int i = 0; i < resolver->asts.size; i++)
	{
		AST::File* ast = resolver->asts[i];
		resolver->currentFile = ast;
		for (int j = 0; j < ast->classes.size; j++)
		{
			result = ResolveClass(resolver, ast->classes[j]) && result;
		}
	}
	for (int i = 0; i < resolver->asts.size; i++)
	{
		AST::File* ast = resolver->asts[i];
		resolver->currentFile = ast;
		for (int j = 0; j < ast->typedefs.size; j++)
		{
			result = ResolveTypedef(resolver, ast->typedefs[j]) && result;
		}
	}
	if (!result) // Return early if types have not been resolved
		return result;

	for (int i = 0; i < resolver->asts.size; i++)
	{
		AST::File* ast = resolver->asts[i];
		resolver->currentFile = ast;
		for (int j = 0; j < ast->functions.size; j++)
		{
			result = ResolveFunction(resolver, ast->functions[j]) && result;
		}
		for (int j = 0; j < ast->classes.size; j++)
		{
			result = ResolveClassProcedures(resolver, ast->classes[j]) && result;
		}
	}
	for (int i = 0; i < resolver->asts.size; i++)
	{
		AST::File* ast = resolver->asts[i];
		resolver->currentFile = ast;
		for (int j = 0; j < ast->globals.size; j++)
		{
			bool isPrimitiveConstant = HasFlag(ast->globals[j]->flags, AST::DeclarationFlags::Constant) && IsPrimitiveType(ast->globals[j]->varType->typeKind);
			if (!isPrimitiveConstant)
				result = ResolveGlobal(resolver, ast->globals[j]) && result;
		}
	}

	return result;
}

Resolver::Resolver(CGLCompiler* context, List<AST::File*>& asts)
	: context(context), asts(asts)
{
	InitTypeData();

	globalModule = CreateModule(this, "", nullptr);
}

Resolver::~Resolver()
{
	//delete globalModule;
}

bool Resolver::run()
{
	bool result = true;

	currentFile = nullptr;

	currentFunction = nullptr;
	currentElement = nullptr;

	globalScope = new Scope();
	scope = globalScope;

	result = ResolveModuleHeaders(this) && result;
	result = ResolveModules(this) && result;

	if (!entryPoint && !context->staticLibrary && !context->sharedLibrary)
	{
		SnekError(context, "No entry point 'main'");
		result = false;
	}

	return result;
}

Scope* Resolver::pushScope(const char* name, AST::SourceLocation& location)
{
	Scope* newScope = new Scope();
	newScope->parent = scope;
	newScope->name = name;
	newScope->start = location;
	scope->children.add(newScope);
	scope = newScope;
	return newScope;
}

void Resolver::popScope(AST::SourceLocation& location)
{
	SnekAssert(location.line != 0 && location.col != 0);
	Scope* oldScope = scope;
	oldScope->end = location;
	scope = oldScope->parent;
}

AST::File* Resolver::findFileByName(const char* name)
{
	for (int i = 0; i < asts.size; i++)
	{
		AST::File* file = asts[i];
		if (strcmp(file->name, name) == 0)
			return file;
	}
	return nullptr;
}

bool Resolver::isFunctionVisible(const AST::Function* function, AST::Module* currentModule)
{
	return function->file->module == currentModule || function->visibility >= AST::Visibility::Public;
}
