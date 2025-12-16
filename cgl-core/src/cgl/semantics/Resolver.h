#pragma once

#include "Type.h"

#include "cgl/ast/Declaration.h"
#include "cgl/utils/List.h"

#include <map>


struct Variable;

namespace AST
{
	struct Element;
	struct Module;
	struct File;
	struct Function;
	struct Statement;
	struct Expression;
}

struct Scope
{
	Scope* parent = NULL;
	const char* name;

	AST::Statement* branchDst;

	List<Variable*> localVariables;
};

class CGLCompiler;

struct Resolver
{
	CGLCompiler* context;
	List<AST::File*>& asts;

	AST::Function* entryPoint = nullptr;

	AST::Module* globalModule = nullptr;

	AST::File* currentFile = nullptr;
	AST::Element* currentElement = nullptr;
	AST::Function* currentFunction = nullptr;
	AST::Struct* currentStruct = nullptr;
	AST::Class* currentClass = nullptr;
	//AstStatement* currentStatement;
	//AstExpression* currentExpression;

	Scope* scope = nullptr;

	TypeID expectedType = nullptr;


	Resolver(CGLCompiler* context, List<AST::File*>& asts);
	~Resolver();

	bool run();

	Scope* pushScope(const char* name);
	void popScope();

	AST::File* findFileByName(const char* name);

	void registerLocalVariable(Variable* variable, AST::Element* declaration);
	void registerGlobalVariable(Variable* variable, AST::GlobalVariable* global);

	TypeID getGenericTypeArgument(const char* name);

	Variable* findLocalVariableInScope(const char* name, Scope* scope, bool recursive);
	Variable* findGlobalVariableInFile(const char* name, AST::File* file);
	Variable* findGlobalVariableInModule(const char* name, AST::Module* module, AST::Module* current);
	Variable* findVariable(const char* name);

	AST::Function* findFunctionInFile(const char* name, AST::File* file, TypeID instanceType = nullptr);
	AST::Function* findFunctionInModule(const char* name, AST::Module* module, TypeID instanceType = nullptr);
	AST::Function* findFunction(const char* name, TypeID instanceType = nullptr);
	bool findFunctionsInFile(const char* name, AST::File* file, List<AST::Function*>& functions);
	bool findFunctionsInModule(const char* name, AST::Module* module, List<AST::Function*>& functions);
	bool findFunctions(const char* name, List<AST::Function*>& functions);

	AST::Function* findUnaryOperatorOverloadInFile(TypeID operandType, AST::UnaryOperatorType operatorOverload, AST::File* file);
	AST::Function* findUnaryOperatorOverloadInModule(TypeID operandType, AST::UnaryOperatorType operatorOverload, AST::Module* module);
	AST::Function* findUnaryOperatorOverload(TypeID operandType, AST::UnaryOperatorType operatorOverload);

	AST::Function* findBinaryOperatorOverloadInFile(TypeID left, TypeID right, AST::BinaryOperatorType operatorOverload, AST::File* file);
	AST::Function* findBinaryOperatorOverloadInModule(TypeID left, TypeID right, AST::BinaryOperatorType operatorOverload, AST::Module* module);
	AST::Function* findBinaryOperatorOverload(TypeID left, TypeID right, AST::BinaryOperatorType operatorOverload);

	int getFunctionOverloadScore(const AST::Function* function, const List<AST::Expression*>& arguments);
	void chooseFunctionOverload(List<AST::Function*>& functions, const List<AST::Expression*>& arguments, AST::Expression* methodInstance);

	AST::Enum* findEnumInFile(const char* name, AST::File* file);
	AST::Enum* findEnumInModule(const char* name, AST::Module* module);
	AST::Enum* findEnum(const char* name);

	AST::EnumValue* findEnumValueInFile(const char* name, AST::File* file);
	AST::EnumValue* findEnumValueInModule(const char* name, AST::Module* module);
	AST::EnumValue* findEnumValue(const char* name);

	AST::Struct* findStructInFile(const char* name, AST::File* file);
	AST::Struct* findStructInModule(const char* name, AST::Module* module);
	AST::Struct* findStruct(const char* name);

	AST::Typedef* findTypedefInFile(const char* name, AST::File* file);
	AST::Typedef* findTypedefInModule(const char* name, AST::Module* module);
	AST::Typedef* findTypedef(const char* name);

	AST::Macro* findExprdefInFile(const char* name, AST::File* file);
	AST::Macro* findExprdefInModule(const char* name, AST::Module* module);
	AST::Macro* findExprdef(const char* name);

	bool isFunctionVisible(const AST::Function* function, AST::Module* currentModule);
};


bool ResolveType(Resolver* resolver, AST::Type* type);

TypeID DeduceGenericArg(AST::Type* type, TypeID argType, const AST::Function* function);
