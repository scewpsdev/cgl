#pragma once

#include "cgl/ast/Element.h"
#include "cgl/ast/Module.h"
#include "cgl/ast/File.h"
#include "cgl/ast/Declaration.h"

#include "cgl/semantics/Type.h"


struct Variable
{
	AST::File* file;
	char* name;
	bool isConstant;
	AST::Visibility visibility;
	AST::Expression* value;

	AST::Element* declaration = nullptr;

	char* mangledName = nullptr;
	TypeID type = nullptr;

	ValueHandle allocHandle = nullptr;


	Variable(AST::File* file, char* name, char* mangledName, TypeID type, AST::Expression* value, bool isConstant, AST::Visibility visibility);
	~Variable();
};


AST::EnumValue* FindEnumValue(Resolver* resolver, const char* name);

AST::Struct* FindStruct(Resolver* resolver, const char* name);

AST::Class* FindClass(Resolver* resolver, const char* name);

AST::Typedef* FindTypedef(Resolver* resolver, const char* name);

AST::Enum* FindEnum(Resolver* resolver, const char* name);

AST::Macro* FindMacro(Resolver* resolver, const char* name);
