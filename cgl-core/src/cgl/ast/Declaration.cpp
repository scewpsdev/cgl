#include "pch.h"
#include "Declaration.h"

#include "../semantics/Resolver.h"


namespace AST
{
	DeclarationFlags operator|(DeclarationFlags flag0, DeclarationFlags flag1)
	{
		return (DeclarationFlags)((uint16_t)flag0 | (uint16_t)flag1);
	}

	DeclarationFlags operator&(DeclarationFlags flag0, DeclarationFlags flag1)
	{
		return (DeclarationFlags)((uint16_t)flag0 & (uint16_t)flag1);
	}

	Declaration::Declaration(File* file, const SourceLocation& location, DeclarationType type, DeclarationFlags flags)
		: Element(file, location), type(type), flags(flags)
	{
	}

	Function::Function(File* file, const SourceLocation& location, DeclarationFlags flags, const SourceLocation& endLocation, char* name, Type* returnType, const List<Type*>& paramTypes, const List<char*>& paramNames, const List<Expression*>& paramValues, bool varArgs, Type* varArgsTypeAST, char* varArgsName, Statement* body, bool isGeneric, const List<char*>& genericParams)
		: Declaration(file, location, DeclarationType::Function, flags), endLocation(endLocation), name(name), returnType(returnType), paramTypes(paramTypes), paramNames(paramNames), paramValues(paramValues), varArgs(varArgs), varArgsTypeAST(varArgsTypeAST), varArgsName(varArgsName), body(body), isGeneric(isGeneric), genericParams(genericParams)
	{
	}

	Function::~Function()
	{
		if (name)
			delete name;
		if (returnType)
			delete returnType;
		for (int i = 0; i < paramTypes.size; i++)
		{
			if (paramTypes[i])
				delete paramTypes[i];
		}
		DestroyList(paramTypes);
		for (int i = 0; i < paramNames.size; i++)
		{
			if (paramNames[i])
				delete paramNames[i];
		}
		DestroyList(paramNames);
		if (body)
			delete body;

		if (isGeneric || isGenericInstance)
		{
			for (int i = 0; i < genericParams.size; i++)
			{
				if (genericParams[i])
					delete genericParams[i];
			}
			DestroyList(genericParams);
		}
	}

	Element* Function::copy()
	{
		List<Type*> paramTypesCopy = CreateList<Type*>(paramTypes.size);
		for (int i = 0; i < paramTypes.size; i++)
			paramTypesCopy.add((Type*)paramTypes[i]->copy());

		List<char*> paramNamesCopy = CreateList<char*>(paramNames.size);
		for (int i = 0; i < paramNames.size; i++)
			paramNamesCopy.add(_strdup(paramNames[i]));

		List<Expression*> paramValuesCopy = CreateList<Expression*>(paramValues.size);
		for (int i = 0; i < paramValues.size; i++)
			paramValuesCopy.add(paramValues[i] ? (Expression*)paramValues[i]->copy() : nullptr);

		List<char*> genericParamsCopy = {};
		if (isGeneric || isGenericInstance)
		{
			genericParamsCopy = CreateList<char*>(genericParams.size);
			for (int i = 0; i < genericParams.size; i++)
				genericParamsCopy.add(_strdup(genericParams[i]));
		}

		Function* decl = new Function(file, location, flags, endLocation, _strdup(name), returnType ? (Type*)returnType->copy() : nullptr, paramTypesCopy, paramNamesCopy, paramValuesCopy, varArgs, varArgsTypeAST ? (Type*)varArgsTypeAST->copy() : nullptr, varArgsName ? _strdup(varArgsName) : nullptr, body ? (Statement*)body->copy() : nullptr, isGeneric, genericParamsCopy);
		decl->dllImport = dllImport;
		return decl;
	}

	int Function::getNumRequiredParams()
	{
		int i = 0;
		for (; i < paramTypes.size; i++)
		{
			if (paramValues[i])
				break;
		}
		return i;
	}

	bool Function::isGenericParam(int idx, TypeID argType) const
	{
		if (isGenericInstance || isGeneric)
		{
			if (idx >= 0 && idx < paramTypes.size)
			{
				AST::Type* paramType = paramTypes[idx];
				if (TypeID type = DeduceGenericArg(paramType, argType, this))
					return true;
			}
		}
		return false;
	}

	TypeID Function::getGenericTypeArgument(const char* name)
	{
		if (isGenericInstance)
		{
			for (int i = 0; i < genericParams.size; i++)
			{
				if (strcmp(genericParams[i], name) == 0)
					return genericTypeArguments[i];
			}
		}
		return nullptr;
	}

	Function* Function::getGenericInstance(const List<TypeID>& genericArgs)
	{
		for (Function* instance : genericInstances)
		{
			bool matching = true;
			for (int i = 0; i < genericArgs.size; i++)
			{
				if (!CompareTypes(genericArgs[i], instance->genericTypeArguments[i]))
				{
					matching = false;
					break;
				}
			}
			if (matching)
				return instance;
		}
		return nullptr;
	}

	Method::Method(File* file, const SourceLocation& location, DeclarationFlags flags, const SourceLocation& endLocation, char* name, Type* returnType, const List<Type*>& paramTypes, const List<char*>& paramNames, const List<Expression*>& paramValues, bool varArgs, Type* varArgsTypeAST, char* varArgsName, Statement* body, bool isGeneric, const List<char*>& genericParams)
		: Function(file, location, flags, endLocation, name, returnType, paramTypes, paramNames, paramValues, varArgs, varArgsTypeAST, varArgsName, body, isGeneric, genericParams)
	{
		type = DeclarationType::ClassMethod;
	}

	Method::~Method()
	{
	}

	Element* Method::copy()
	{
		List<Type*> paramTypesCopy = CreateList<Type*>(paramTypes.size);
		for (int i = 0; i < paramTypes.size; i++)
			paramTypesCopy.add((Type*)paramTypes[i]->copy());

		List<char*> paramNamesCopy = CreateList<char*>(paramNames.size);
		for (int i = 0; i < paramNames.size; i++)
			paramNamesCopy.add(_strdup(paramNames[i]));

		List<Expression*> paramValuesCopy = CreateList<Expression*>(paramValues.size);
		for (int i = 0; i < paramValues.size; i++)
			paramValuesCopy.add(paramValues[i] ? (Expression*)paramValues[i]->copy() : nullptr);

		List<char*> genericParamsCopy = {};
		if (isGeneric)
		{
			genericParamsCopy = CreateList<char*>(genericParams.size);
			for (int i = 0; i < genericParams.size; i++)
				genericParamsCopy.add(_strdup(genericParams[i]));
		}

		return new Method(file, location, flags, endLocation, _strdup(name), (Type*)returnType->copy(), paramTypesCopy, paramNamesCopy, paramValuesCopy, varArgs, varArgsTypeAST ? (Type*)varArgsTypeAST->copy() : nullptr, varArgsName ? _strdup(varArgsName) : nullptr, (Statement*)body->copy(), isGeneric, genericParamsCopy);
	}

	Constructor::Constructor(File* file, const SourceLocation& location, DeclarationFlags flags, const SourceLocation& endLocation, char* name, Type* returnType, const List<Type*>& paramTypes, const List<char*>& paramNames, const List<Expression*>& paramValues, bool varArgs, Type* varArgsTypeAST, char* varArgsName, Statement* body, bool isGeneric, const List<char*>& genericParams)
		: Method(file, location, flags, endLocation, name, returnType, paramTypes, paramNames, paramValues, varArgs, varArgsTypeAST, varArgsName, body, isGeneric, genericParams)
	{
		type = DeclarationType::ClassConstructor;
	}

	Constructor::~Constructor()
	{
	}

	Element* Constructor::copy()
	{
		List<Type*> paramTypesCopy = CreateList<Type*>(paramTypes.size);
		for (int i = 0; i < paramTypes.size; i++)
			paramTypesCopy.add((Type*)paramTypes[i]->copy());

		List<char*> paramNamesCopy = CreateList<char*>(paramNames.size);
		for (int i = 0; i < paramNames.size; i++)
			paramNamesCopy.add(_strdup(paramNames[i]));

		List<Expression*> paramValuesCopy = CreateList<Expression*>(paramValues.size);
		for (int i = 0; i < paramValues.size; i++)
			paramValuesCopy.add(paramValues[i] ? (Expression*)paramValues[i]->copy() : nullptr);

		List<char*> genericParamsCopy = {};
		if (isGeneric)
		{
			genericParamsCopy = CreateList<char*>(genericParams.size);
			for (int i = 0; i < genericParams.size; i++)
				genericParamsCopy.add(_strdup(genericParams[i]));
		}

		return new Constructor(file, location, flags, endLocation, _strdup(name), (Type*)returnType->copy(), paramTypesCopy, paramNamesCopy, paramValuesCopy, varArgs, varArgsTypeAST ? (Type*)varArgsTypeAST->copy() : nullptr, varArgsName ? _strdup(varArgsName) : nullptr, (Statement*)body->copy(), isGeneric, genericParamsCopy);
	}

	StructField::StructField(File* file, const SourceLocation& location, Type* type, char* name, int index)
		: Element(file, location), type(type), name(name), index(index)
	{
	}

	StructField::~StructField()
	{
		if (type)
			delete type;
		if (name)
			delete name;
	}

	Element* StructField::copy()
	{
		StructField* field = new StructField(file, location, (Type*)type->copy(), _strdup(name), index);
		/*
		if (isStruct)
		{
			field->isStruct = true;
			field->structFields = CopyList(structFields);
		}
		else if (isUnion)
		{
			field->isUnion = true;
			field->unionFields = CopyList(unionFields);
		}
		*/
		return field;
	}

	Struct::Struct(File* file, const SourceLocation& location, DeclarationFlags flags, char* name, bool hasBody, const List<StructField*>& fields, bool isGeneric, const List<char*>& genericParams)
		: Declaration(file, location, DeclarationType::Struct, flags), name(name), hasBody(hasBody), fields(fields), isGeneric(isGeneric), genericParams(genericParams)
	{
	}

	Struct::~Struct()
	{
		if (name)
			delete name;
		for (int i = 0; i < fields.size; i++)
		{
			if (fields[i])
				delete fields[i];
		}
		DestroyList(fields);

		if (isGeneric || isGenericInstance)
		{
			for (int i = 0; i < genericParams.size; i++)
			{
				if (genericParams[i])
					delete genericParams[i];
			}
			DestroyList(genericParams);
		}
	}

	Element* Struct::copy()
	{
		List<StructField*> fieldsCopy = CreateList<StructField*>(fields.size);
		for (int i = 0; i < fields.size; i++)
			fieldsCopy.add((StructField*)fields[i]->copy());

		List<char*> genericParamsCopy = {};
		if (isGeneric || isGenericInstance)
		{
			genericParamsCopy = CreateList<char*>(genericParams.size);
			for (int i = 0; i < genericParams.size; i++)
				genericParamsCopy.add(_strdup(genericParams[i]));
		}

		return new Struct(file, location, flags, _strdup(name), hasBody, fieldsCopy, isGeneric, genericParamsCopy);
	}

	StructField* CheckStructField(StructField* field, const char* name)
	{
		if (field->name && strcmp(field->name, name) == 0)
			return field;
		if (field->type->typeKind == AST::TypeKind::Struct && !field->name)
		{
			AST::StructType* structType = (AST::StructType*)field->type->typeID->structType.anonDeclaration;
			for (int i = 0; i < structType->fields.size; i++)
			{
				if (StructField* structField = CheckStructField(structType->fields[i], name))
					return structField;
			}
		}
		if (field->type->typeKind == AST::TypeKind::Union && !field->name)
		{
			AST::UnionType* unionType = (AST::UnionType*)field->type->typeID->unionType.anonDeclaration;
			for (int i = 0; i < unionType->fields.size; i++)
			{
				if (StructField* unionField = CheckStructField(unionType->fields[i], name))
					return unionField;
			}
		}
		return nullptr;
	}

	StructField* Struct::getFieldWithName(const char* name, int* index)
	{
		for (int i = 0; i < fields.size; i++)
		{
			if (StructField* field = CheckStructField(fields[i], name))
			{
				if (index)
					*index = i;
				return field;
			}
		}
		return nullptr;
	}

	TypeID Struct::getGenericTypeArgument(const char* name)
	{
		if (isGenericInstance)
		{
			for (int i = 0; i < genericParams.size; i++)
			{
				if (strcmp(genericParams[i], name) == 0)
					return genericTypeArguments[i];
			}
		}
		return nullptr;
	}

	Struct* Struct::getGenericInstance(const List<Type*>& genericArgs)
	{
		for (Struct* instance : genericInstances)
		{
			bool matching = true;
			for (int i = 0; i < genericArgs.size; i++)
			{
				if (!CompareTypes(genericArgs[i]->typeID, instance->genericTypeArguments[i]))
				{
					matching = false;
					break;
				}
			}
			if (matching)
				return instance;
		}
		return nullptr;
	}

	ClassField::ClassField(File* file, const SourceLocation& location, Type* type, char* name, int index)
		: Element(file, location), type(type), name(name), index(index)
	{
	}

	ClassField::~ClassField()
	{
		if (type)
			delete type;
		if (name)
			delete name;
	}

	Element* ClassField::copy()
	{
		return new ClassField(file, location, (Type*)type->copy(), _strdup(name), index);
	}

	Class::Class(File* file, const SourceLocation& location, DeclarationFlags flags, char* name, const List<ClassField*>& fields, const List<Method*>& methods, Constructor* constructor, bool isGeneric, const List<char*>& genericParams)
		: Declaration(file, location, DeclarationType::Class, flags), name(name), fields(fields), methods(methods), constructor(constructor), isGeneric(isGeneric), genericParams(genericParams)
	{
	}

	Class::~Class()
	{
		if (name)
			delete name;
		for (int i = 0; i < fields.size; i++)
		{
			if (fields[i])
				delete fields[i];
		}
		DestroyList(fields);
		for (int i = 0; i < methods.size; i++)
		{
			if (methods[i])
				delete methods[i];
		}
		DestroyList(methods);
		if (constructor)
			delete constructor;
	}

	Element* Class::copy()
	{
		List<ClassField*> fieldsCopy = CreateList<ClassField*>(fields.size);
		for (int i = 0; i < fields.size; i++)
			fieldsCopy.add((ClassField*)fields[i]->copy());

		List<Method*> methodsCopy = CreateList<Method*>(methods.size);
		for (int i = 0; i < methods.size; i++)
			methodsCopy.add((Method*)methods[i]->copy());

		List<char*> genericParamsCopy = {};
		if (isGeneric || isGenericInstance)
		{
			genericParamsCopy = CreateList<char*>(genericParams.size);
			for (int i = 0; i < genericParams.size; i++)
				genericParamsCopy.add(_strdup(genericParams[i]));
		}

		return new Class(file, location, flags, _strdup(name), fieldsCopy, methodsCopy, constructor ? (Constructor*)constructor->copy() : nullptr, isGeneric, genericParamsCopy);
	}

	TypeID Class::getGenericTypeArgument(const char* name)
	{
		if (isGenericInstance)
		{
			for (int i = 0; i < genericParams.size; i++)
			{
				if (strcmp(genericParams[i], name) == 0)
					return genericTypeArguments[i];
			}
		}
		return nullptr;
	}

	Class* Class::getGenericInstance(const List<Type*>& genericArgs)
	{
		for (Class* instance : genericInstances)
		{
			bool matching = true;
			for (int i = 0; i < genericArgs.size; i++)
			{
				if (!CompareTypes(genericArgs[i]->typeID, instance->genericTypeArguments[i]))
				{
					matching = false;
					break;
				}
			}
			if (matching)
				return instance;
		}
		return nullptr;
	}

	Typedef::Typedef(File* file, const SourceLocation& location, DeclarationFlags flags, char* name, Type* alias)
		: Declaration(file, location, DeclarationType::Typedef, flags), name(name), alias(alias)
	{
	}

	Typedef::~Typedef()
	{
		if (name)
			delete name;
		if (alias)
			delete alias;
	}

	Element* Typedef::copy()
	{
		return new Typedef(file, location, flags, _strdup(name), (Type*)alias->copy());
	}

	EnumValue::EnumValue(File* file, const SourceLocation& location, char* name, Expression* value, int idx)
		: Element(file, location), name(name), value(value), idx(idx)
	{
	}

	EnumValue::~EnumValue()
	{
		if (name)
			delete name;
		if (value)
			delete value;
	}

	Element* EnumValue::copy()
	{
		return new EnumValue(file, location, _strdup(name), (Expression*)value->copy(), idx);
	}

	Enum::Enum(File* file, const SourceLocation& location, DeclarationFlags flags, char* name, Type* alias, const List<EnumValue*>& values)
		: Declaration(file, location, DeclarationType::Enumeration, flags), name(name), alias(alias), values(values)
	{
		for (int i = 0; i < values.size; i++)
			values[i]->declaration = this;
	}

	Enum::~Enum()
	{
		if (name)
			delete name;
		if (alias)
			delete alias;
		for (int i = 0; i < values.size; i++)
		{
			if (values[i])
				delete values[i];
		}
		DestroyList(values);
	}

	Element* Enum::copy()
	{
		List<EnumValue*> valuesCopy = CreateList<EnumValue*>(values.size);
		for (int i = 0; i < values.size; i++)
			valuesCopy.add((EnumValue*)values[i]->copy());

		return new Enum(file, location, flags, _strdup(name), (Type*)alias->copy(), valuesCopy);
	}

	Macro::Macro(File* file, const SourceLocation& location, DeclarationFlags flags, char* name, Expression* alias)
		: Declaration(file, location, DeclarationType::Macro, flags), name(name), alias(alias)
	{
	}

	Macro::~Macro()
	{
		if (name)
			delete name;
		if (alias)
			delete alias;
	}

	Element* Macro::copy()
	{
		return new Macro(file, location, flags, _strdup(name), (Expression*)alias->copy());
	}

	GlobalVariable::GlobalVariable(File* file, const SourceLocation& location, DeclarationFlags flags, Type* varType, List<VariableDeclarator*>& declarators)
		: Declaration(file, location, DeclarationType::GlobalVariable, flags), varType(varType), declarators(declarators)
	{
	}

	GlobalVariable::~GlobalVariable()
	{
		if (varType)
			delete varType;
		for (int i = 0; i < declarators.size; i++)
		{
			if (declarators[i])
				delete declarators[i];
		}
		DestroyList(declarators);
	}

	Element* GlobalVariable::copy()
	{
		List<VariableDeclarator*> declaratorsCopy = CreateList<VariableDeclarator*>(declarators.size);
		for (int i = 0; i < declarators.size; i++)
			declaratorsCopy.add((VariableDeclarator*)declarators[i]->copy());

		GlobalVariable* decl = new GlobalVariable(file, location, flags, (Type*)varType->copy(), declaratorsCopy);
		decl->dllImport = dllImport;
		return decl;
	}

	ModuleDeclaration::ModuleDeclaration(File* file, const SourceLocation& location, DeclarationFlags flags, ModuleIdentifier identifier)
		: Declaration(file, location, DeclarationType::Module, flags), identifier(identifier)
	{
	}

	ModuleDeclaration::~ModuleDeclaration()
	{
		for (int i = 0; i < identifier.namespaces.size; i++)
		{
			if (identifier.namespaces[i])
				delete identifier.namespaces[i];
		}
		DestroyList(identifier.namespaces);
	}

	Element* ModuleDeclaration::copy()
	{
		List<char*> namespacesCopy = CreateList<char*>(identifier.namespaces.size);
		for (int i = 0; i < identifier.namespaces.size; i++)
			namespacesCopy.add(_strdup(identifier.namespaces[i]));

		return new ModuleDeclaration(file, location, flags, { namespacesCopy });
	}

	NamespaceDeclaration::NamespaceDeclaration(File* file, const SourceLocation& location, DeclarationFlags flags, char* name)
		: Declaration(file, location, DeclarationType::Namespace, flags), name(name)
	{
	}

	NamespaceDeclaration::~NamespaceDeclaration()
	{
		if (name)
			delete name;
	}

	Element* NamespaceDeclaration::copy()
	{
		return new NamespaceDeclaration(file, location, flags, _strdup(name));
	}

	Import::Import(File* file, const SourceLocation& location, DeclarationFlags flags, const List<ModuleIdentifier>& imports)
		: Declaration(file, location, DeclarationType::Import, flags), imports(imports)
	{
	}

	Import::~Import()
	{
		for (int i = 0; i < imports.size; i++)
		{
			for (int j = 0; j < imports[i].namespaces.size; j++)
			{
				if (imports[i].namespaces[j])
					delete imports[i].namespaces[j];
			}
			DestroyList(imports[i].namespaces);
		}
		DestroyList(imports);
	}

	Element* Import::copy()
	{
		List<ModuleIdentifier> importsCopy = CreateList<ModuleIdentifier>(imports.size);
		for (int i = 0; i < imports.size; i++)
		{
			List<char*> namespacesCopy = CreateList<char*>(imports[i].namespaces.size);
			for (int j = 0; j < imports[i].namespaces.size; j++)
				namespacesCopy.add(_strdup(imports[i].namespaces[j]));

			importsCopy.add({ namespacesCopy });
		}

		return new Import(file, location, flags, importsCopy);
	}
}
