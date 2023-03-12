#include "pch.h"
#include "Type.h"

#include "Declaration.h"
#include "Expression.h"


namespace AST
{
	Type::Type(File* file, const SourceLocation& location, TypeKind typeKind)
		: Element(file, location), typeKind(typeKind)
	{
	}

	VoidType::VoidType(File* file, const SourceLocation& location)
		: Type(file, location, TypeKind::Void)
	{
	}

	Element* VoidType::copy()
	{
		return new VoidType(file, location);
	}

	IntegerType::IntegerType(File* file, const SourceLocation& location, int bitWidth, bool isSigned)
		: Type(file, location, TypeKind::Integer), bitWidth(bitWidth), isSigned(isSigned)
	{
	}

	Element* AST::IntegerType::copy()
	{
		return new IntegerType(file, location, bitWidth, isSigned);
	}

	FloatingPointType::FloatingPointType(File* file, const SourceLocation& location, int bitWidth)
		: Type(file, location, TypeKind::FloatingPoint), bitWidth(bitWidth)
	{
	}

	Element* FloatingPointType::copy()
	{
		return new FloatingPointType(file, location, bitWidth);
	}

	BooleanType::BooleanType(File* file, const SourceLocation& location)
		: Type(file, location, TypeKind::Boolean)
	{
	}

	Element* BooleanType::copy()
	{
		return new BooleanType(file, location);
	}

	NamedType::NamedType(File* file, const SourceLocation& location, char* name, bool hasGenericArgs, const List<Type*>& genericArgs)
		: Type(file, location, TypeKind::NamedType), name(name), hasGenericArgs(hasGenericArgs), genericArgs(genericArgs)
	{
	}

	NamedType::~NamedType()
	{
		if (hasGenericArgs)
		{
			for (int i = 0; i < genericArgs.size; i++)
			{
				if (genericArgs[i])
					delete genericArgs[i];
			}
			DestroyList(genericArgs);
			delete declaration;
		}
	}

	Element* NamedType::copy()
	{
		List<Type*> genericArgsCopy = {};
		if (hasGenericArgs)
		{
			genericArgsCopy = CreateList<Type*>(genericArgs.size);
			for (int i = 0; i < genericArgs.size; i++)
				genericArgsCopy.add((Type*)genericArgs[i]->copy());
		}

		return new NamedType(file, location, _strdup(name), hasGenericArgs, genericArgsCopy);
	}

	PointerType::PointerType(File* file, const SourceLocation& location, Type* elementType)
		: Type(file, location, TypeKind::Pointer), elementType(elementType)
	{
	}

	PointerType::~PointerType()
	{
		if (elementType)
			delete elementType;
	}

	Element* PointerType::copy()
	{
		return new PointerType(file, location, (Type*)elementType->copy());
	}

	OptionalType::OptionalType(File* file, const SourceLocation& location, Type* elementType)
		: Type(file, location, TypeKind::Optional), elementType(elementType)
	{
	}

	OptionalType::~OptionalType()
	{
		if (elementType)
			delete elementType;
	}

	Element* OptionalType::copy()
	{
		return new OptionalType(file, location, (Type*)elementType->copy());
	}

	FunctionType::FunctionType(File* file, const SourceLocation& location, Type* returnType, const List<Type*>& paramTypes, bool varArgs)
		: Type(file, location, TypeKind::Function), returnType(returnType), paramTypes(paramTypes), varArgs(varArgs)
	{
	}

	FunctionType::~FunctionType()
	{
		if (returnType)
			delete returnType;
		for (int i = 0; i < paramTypes.size; i++)
		{
			if (paramTypes[i])
				delete paramTypes[i];
		}
		DestroyList(paramTypes);
	}

	Element* FunctionType::copy()
	{
		List<Type*> paramTypesCopy = CreateList<Type*>(paramTypes.size);
		for (int i = 0; i < paramTypes.size; i++)
			paramTypesCopy.add((Type*)paramTypes[i]->copy());

		return new FunctionType(file, location, (Type*)returnType->copy(), paramTypesCopy, varArgs);
	}

	TupleType::TupleType(File* file, const SourceLocation& location, const List<Type*>& valueTypes)
		: Type(file, location, TypeKind::Tuple), valueTypes(valueTypes)
	{
	}

	TupleType::~TupleType()
	{
		for (Type* type : valueTypes)
		{
			delete type;
		}
		DestroyList(valueTypes);
	}

	Element* TupleType::copy()
	{
		List<Type*> valueTypesCopy;
		valueTypesCopy.reserve(valueTypes.size);

		for (Type* type : valueTypes)
		{
			valueTypesCopy.add((Type*)type->copy());
		}

		return new TupleType(file, location, valueTypesCopy);
	}

	ArrayType::ArrayType(File* file, const SourceLocation& location, Type* elementType, Expression* length)
		: Type(file, location, TypeKind::Array), elementType(elementType), length(length)
	{
	}

	ArrayType::~ArrayType()
	{
		if (elementType)
			delete elementType;
		if (length)
			delete length;
	}

	Element* ArrayType::copy()
	{
		return new ArrayType(file, location, (Type*)elementType->copy(), length ? (Expression*)length->copy() : nullptr);
	}

	StringType::StringType(File* file, const SourceLocation& location, Expression* length)
		: Type(file, location, TypeKind::String), length(length)
	{
	}

	Element* StringType::copy()
	{
		return new StringType(file, location, length ? (Expression*)length->copy() : nullptr);
	}
}
