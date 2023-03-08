#include "cgl/CGLCompiler.h"

#include "lib/libtcc/libtcc.h"

#include "cgl/semantics/Variable.h"
#include "cgl/semantics/Function.h"


struct Scope
{
	Scope* parent = nullptr;

	int unnamedLocalId = 0;
};

class CodegenTCC
{
	CGLCompiler* context;
	AST::File* file;

	List<Variable*> importedGlobals;

	std::stringstream builtinDefinitionStream;
	std::stringstream constants;
	std::stringstream globals;
	std::stringstream functions;

	std::stringstream* currentStream = nullptr;

	Scope* scope = nullptr;

	int unnamedGlobalId = 0;

	int indentation = 0;


	void newLine()
	{
		*currentStream << "\n";
		for (int i = 0; i < indentation; i++)
			*currentStream << "\t";
	}

	void newGlobalName(char name[8])
	{
		name[0] = '_';
		name[1] = 'G';
		sprintf(&name[2], "%d", unnamedGlobalId++);
	}

	std::string newLocalName()
	{
		return '_' + std::to_string(scope->unnamedLocalId++);
	}

	Scope* pushScope()
	{
		Scope* newScope = new Scope();
		newScope->parent = scope;
		newScope->unnamedLocalId = scope ? scope->unnamedLocalId : 0;
		scope = newScope;
		return newScope;
	}

	void popScope()
	{
		Scope* lastScope = scope;
		scope = lastScope->parent;
		delete lastScope;
	}

	void genTypeVoid()
	{
		*currentStream << "void";
	}

	void genTypeInteger(TypeID type)
	{
		*currentStream << (type->integerType.isSigned ? "i" : "u") << type->integerType.bitWidth;
	}

	void genTypeFloatingPoint(TypeID type)
	{
		switch (type->fpType.precision)
		{
		case FloatingPointPrecision::Half:
			*currentStream << "float";
			break;
		case FloatingPointPrecision::Single:
			*currentStream << "float";
			break;
		case FloatingPointPrecision::Double:
			*currentStream << "double";
			break;
		case FloatingPointPrecision::Decimal:
			*currentStream << "double";
			break;
		case FloatingPointPrecision::Quad:
			*currentStream << "double";
			break;
		default:
			SnekAssert(false);
			break;
		}
	}

	void genTypePointer(TypeID type)
	{
		genType(type->pointerType.elementType);
		*currentStream << "*";
	}

	void genTypeString()
	{
		*currentStream << "string";
	}

	void genType(TypeID type)
	{
		switch (type->typeKind)
		{
		case AST::TypeKind::Void:
			genTypeVoid();
			break;
		case AST::TypeKind::Integer:
			genTypeInteger(type);
			break;
		case AST::TypeKind::FloatingPoint:
			genTypeFloatingPoint(type);
			break;
		case AST::TypeKind::Boolean:
		case AST::TypeKind::NamedType:
		case AST::TypeKind::Struct:
		case AST::TypeKind::Class:
		case AST::TypeKind::Alias:
		case AST::TypeKind::Pointer:
			genTypePointer(type);
			break;
		case AST::TypeKind::Function:
		case AST::TypeKind::Array:
		case AST::TypeKind::String:
			genTypeString();
			break;
		default:
			SnekAssert(false);
			break;
		}
	}

	void genType(AST::Type* type)
	{
		genType(type->typeID);
	}

	std::string castValue(AST::Expression* expression, TypeID type)
	{
		if (expression->valueType->typeKind == AST::TypeKind::Pointer &&
			expression->valueType->pointerType.elementType->typeKind == AST::TypeKind::Integer &&
			expression->valueType->pointerType.elementType->integerType.bitWidth == 8 &&
			type->typeKind == AST::TypeKind::String)
		{
			std::string expr = genExpression(expression);

			if (expression->type == AST::ExpressionType::StringLiteral)
				return "(string){" + expr + "," + std::to_string(((AST::StringLiteral*)expression)->length) + "}";
			else
				return "(string){" + expr + ",__cstrlen(" + expr + ")}";
		}
		else if (CompareTypes(expression->valueType, type))
			return genExpression(expression);
		else
		{
			SnekAssert(false);
			return "";
		}
	}

	std::string genExpressionIntegerLiteral(AST::IntegerLiteral* expression)
	{
		return std::to_string(expression->value);
	}

	std::string genExpressionFloatingPointLiteral(AST::FloatingPointLiteral* expression)
	{
		return std::to_string(expression->value);
	}

	std::string genExpressionCharacterLiteral(AST::CharacterLiteral* expression)
	{
		if (expression->value == '\n')
			return "'\\n'";
		else if (expression->value == '\r')
			return "'\\r'";
		else if (expression->value == '\t')
			return "'\\t'";
		else if (expression->value == '\\')
			return "'\\\\'";
		else if (expression->value == '\0')
			return "'\\0'";
		else
			return std::to_string(expression->value);
	}

	std::string genExpressionStringLiteral(AST::StringLiteral* expression)
	{
		char globalName[8];
		newGlobalName(globalName);

		constants << "static char " << globalName << "[] = \"";
		for (int i = 0; i < expression->length; i++)
		{
			char c = expression->value[i];
			if (c == '\n')
				constants << "\n";
			else if (c == '\r')
				constants << "\r";
			else if (c == '\t')
				constants << "\t";
			else if (c == '\\')
				constants << "\\";
			else if (c == '\0')
				constants << "\0";
			else
				constants << c;
		}
		constants << "\";\n";

		return globalName;
	}

	void importGlobal(Variable* variable)
	{

	}

	std::string genExpressionIdentifier(AST::Identifier* expression)
	{
		if (expression->variable)
		{
			if (file != expression->variable->file)
			{
				if (!importedGlobals.contains(expression->variable))
				{
					importGlobal(expression->variable);
					importedGlobals.add(expression->variable);
				}
			}
			return expression->variable->mangledName;
		}
		else if (expression->functions.size > 0)
		{
			SnekAssert(expression->functions.size == 1);
			return expression->functions[0]->mangledName;
		}
		else
		{
			SnekAssert(false);
			return "";
		}
	}

	std::string genExpressionFunctionCall(AST::FunctionCall* expression)
	{
		auto parentStream = currentStream;
		std::stringstream callStream;
		currentStream = &callStream;

		std::string returnValue = "";

		SnekAssert(expression->callee->valueType->typeKind == AST::TypeKind::Function);
		if (expression->callee->valueType->functionType.returnType->typeKind != AST::TypeKind::Void)
		{
			returnValue = newLocalName();
			*currentStream << returnValue << "=";
		}

		std::string callee = genExpression(expression->callee);
		*currentStream << callee << "(";
		for (int i = 0; i < expression->arguments.size; i++)
		{
			std::string arg = genExpression(expression->arguments[i]);
			*currentStream << arg;
			if (i < expression->arguments.size - 1)
				*currentStream << ",";
		}
		*currentStream << ")";

		currentStream = parentStream;

		*currentStream << callStream.str();

		return returnValue;
	}

	std::string genExpressionSubscriptOperator(AST::SubscriptOperator* expression)
	{
		std::string operand = genExpression(expression->operand);
		if (expression->operand->valueType->typeKind == AST::TypeKind::Pointer)
		{
			SnekAssert(expression->arguments.size == 1);
			return operand + "[" + genExpression(expression->arguments[0]) + "]";
		}
		else if (expression->operand->valueType->typeKind == AST::TypeKind::Array)
		{
			SnekAssert(expression->arguments.size == 1);
			// Runtime check?
			return operand + ".ptr[" + genExpression(expression->arguments[0]) + "]";
		}
		else if (expression->operand->valueType->typeKind == AST::TypeKind::String)
		{
			SnekAssert(expression->arguments.size == 1);
			// Runtime check?
			return operand + ".ptr[" + genExpression(expression->arguments[0]) + "]";
		}
		else
		{
			SnekAssert(false);
			return "";
		}
	}

	std::string genExpressionDotOperator(AST::DotOperator* expression)
	{
		if (expression->module)
		{
			// TODO
			SnekAssert(false);
			return "";
		}
		else if (expression->namespacedVariable)
			return expression->namespacedVariable->mangledName;
		else if (expression->namespacedFunctions.size > 0)
			return expression->namespacedFunctions[0]->mangledName;
		else if (expression->structField)
		{
			if (expression->operand->valueType->typeKind == AST::TypeKind::Pointer && expression->operand->valueType->pointerType.elementType->typeKind == AST::TypeKind::Struct)
				return genExpression(expression->operand) + "->" + expression->structField->name;
			else if (expression->operand->valueType->typeKind == AST::TypeKind::Struct)
				return genExpression(expression->operand) + "." + expression->structField->name;
			else
			{
				SnekAssert(false);
				return "";
			}
		}
		else if (expression->classMethod)
			return expression->classMethod->mangledName;
		else if (expression->classField)
			return genExpression(expression->operand) + "->" + expression->classField->name;
		else if (expression->arrayField != -1)
		{
			if (expression->arrayField == 0)
				return genExpression(expression->operand) + ".length";
			else if (expression->arrayField == 1)
				return genExpression(expression->operand) + ".buffer";
			else
			{
				SnekAssert(false);
				return "";
			}
		}
		else if (expression->stringField != -1)
		{
			if (expression->stringField == 0)
				return genExpression(expression->operand) + ".length";
			else if (expression->stringField == 1)
				return genExpression(expression->operand) + ".ptr";
			else
			{
				SnekAssert(false);
				return "";
			}
		}
		else
		{
			SnekAssert(false);
			return "";
		}
	}

	std::string genExpressionBinaryOperator(AST::BinaryOperator* expression)
	{
		switch (expression->operatorType)
		{
		case AST::BinaryOperatorType::Add:
			return genExpression(expression->left) + "+" + genExpression(expression->right);
		case AST::BinaryOperatorType::Subtract:
		case AST::BinaryOperatorType::Multiply:
		case AST::BinaryOperatorType::Divide:
		case AST::BinaryOperatorType::Modulo:
		case AST::BinaryOperatorType::Equals:
		case AST::BinaryOperatorType::DoesNotEqual:
		case AST::BinaryOperatorType::LessThan:
		case AST::BinaryOperatorType::GreaterThan:
		case AST::BinaryOperatorType::LessThanEquals:
		case AST::BinaryOperatorType::GreaterThanEquals:
		case AST::BinaryOperatorType::LogicalAnd:
		case AST::BinaryOperatorType::LogicalOr:
		case AST::BinaryOperatorType::BitwiseAnd:
		case AST::BinaryOperatorType::BitwiseOr:
		case AST::BinaryOperatorType::BitwiseXor:
		case AST::BinaryOperatorType::BitshiftLeft:
		case AST::BinaryOperatorType::BitshiftRight:
		case AST::BinaryOperatorType::Assignment:
			return genExpression(expression->left) + "=" + castValue(expression->right, expression->left->valueType);
		case AST::BinaryOperatorType::PlusEquals:
		case AST::BinaryOperatorType::MinusEquals:
		case AST::BinaryOperatorType::TimesEquals:
		case AST::BinaryOperatorType::DividedByEquals:
		case AST::BinaryOperatorType::ModuloEquals:
		case AST::BinaryOperatorType::BitwiseAndEquals:
		case AST::BinaryOperatorType::BitwiseOrEquals:
		case AST::BinaryOperatorType::BitwiseXorEquals:
		case AST::BinaryOperatorType::BitshiftLeftEquals:
		case AST::BinaryOperatorType::BitshiftRightEquals:
		case AST::BinaryOperatorType::ReferenceAssignment:
		default:
			SnekAssert(false);
			return "";
		}
	}

	std::string genExpression(AST::Expression* expression)
	{
		switch (expression->type)
		{
		case AST::ExpressionType::IntegerLiteral:
			return genExpressionIntegerLiteral((AST::IntegerLiteral*)expression);
		case AST::ExpressionType::FloatingPointLiteral:
			return genExpressionFloatingPointLiteral((AST::FloatingPointLiteral*)expression);
		case AST::ExpressionType::BooleanLiteral:
		case AST::ExpressionType::CharacterLiteral:
			return genExpressionCharacterLiteral((AST::CharacterLiteral*)expression);
		case AST::ExpressionType::NullLiteral:
		case AST::ExpressionType::StringLiteral:
			return genExpressionStringLiteral((AST::StringLiteral*)expression);
		case AST::ExpressionType::InitializerList:
		case AST::ExpressionType::Identifier:
			return genExpressionIdentifier((AST::Identifier*)expression);
		case AST::ExpressionType::Compound:
		case AST::ExpressionType::FunctionCall:
			return genExpressionFunctionCall((AST::FunctionCall*)expression);
		case AST::ExpressionType::SubscriptOperator:
			return genExpressionSubscriptOperator((AST::SubscriptOperator*)expression);
		case AST::ExpressionType::DotOperator:
			return genExpressionDotOperator((AST::DotOperator*)expression);
		case AST::ExpressionType::Typecast:
		case AST::ExpressionType::Sizeof:
		case AST::ExpressionType::Malloc:
		case AST::ExpressionType::UnaryOperator:
		case AST::ExpressionType::BinaryOperator:
			return genExpressionBinaryOperator((AST::BinaryOperator*)expression);
		case AST::ExpressionType::TernaryOperator:
		default:
			SnekAssert(false);
			return "";
		}
	}

	void genStatementCompound(AST::CompoundStatement* statement)
	{
		*currentStream << "{";
		indentation++;
		newLine();

		pushScope();
		for (int i = 0; i < statement->statements.size; i++)
		{
			genStatement(statement->statements[i]);
		}
		popScope();

		indentation--;
		newLine();
		*currentStream << "}";
	}

	void genStatementExpression(AST::ExpressionStatement* statement)
	{
		*currentStream << genExpression(statement->expression) << ";";
		newLine();
	}

	void genStatementVariableDeclaration(AST::VariableDeclaration* statement)
	{
		std::stringstream* parentStream = currentStream;
		std::stringstream stream;
		currentStream = &stream;

		if (statement->isConstant)
			stream << "const ";
		genType(statement->varType);
		stream << " ";
		for (int i = 0; i < statement->declarators.size; i++)
		{
			AST::VariableDeclarator* declarator = statement->declarators[i];
			if (statement->varTypeAST)
			{
				AST::Type* type = statement->varTypeAST;
				while (type->typeKind == AST::TypeKind::Pointer)
				{
					stream << "*";
					type = ((AST::PointerType*)type)->elementType;
				}
			}
			stream << declarator->name;
			if (declarator->value)
			{
				stream << "=" << castValue(declarator->value, statement->varType);
			}
			if (i < statement->declarators.size - 1)
				stream << ",";
		}
		stream << ";";

		currentStream = parentStream;
		*currentStream << stream.str();
		newLine();
	}

	void genStatementFor(AST::ForLoop* statement)
	{
		std::stringstream* parentStream = currentStream;
		std::stringstream stream;
		currentStream = &stream;

		*currentStream
			<< "for(int " << statement->iteratorName->name << "=" << genExpression(statement->startValue) << ";"
			<< statement->iteratorName->name << "<" << genExpression(statement->endValue) << ";"
			<< statement->iteratorName->name << "++)";
		genStatement(statement->body);

		currentStream = parentStream;
		*currentStream << stream.str();
		newLine();
	}

	void genStatementReturn(AST::Return* statement)
	{
		*currentStream << "return";
		if (statement->value)
		{
			*currentStream << " ";
			genExpression(statement->value);
		}
		*currentStream << ";";
	}

	void genStatement(AST::Statement* statement)
	{
		switch (statement->type)
		{
		case AST::StatementType::NoOp:
			break;
		case AST::StatementType::Compound:
			genStatementCompound((AST::CompoundStatement*)statement);
			break;
		case AST::StatementType::Expression:
			genStatementExpression((AST::ExpressionStatement*)statement);
			break;
		case AST::StatementType::VariableDeclaration:
			genStatementVariableDeclaration((AST::VariableDeclaration*)statement);
			break;
		case AST::StatementType::If:
		case AST::StatementType::While:
		case AST::StatementType::For:
			genStatementFor((AST::ForLoop*)statement);
			break;
		case AST::StatementType::Break:
		case AST::StatementType::Continue:
		case AST::StatementType::Return:
			genStatementReturn((AST::Return*)statement);
			break;
		case AST::StatementType::Defer:
		case AST::StatementType::Free:
		default:
			SnekAssert(false);
			break;
		}
	}

	void genGlobal(AST::GlobalVariable* global)
	{
		std::stringstream* parentStream = currentStream;

		std::stringstream functionStream;
		currentStream = &functionStream;

		if (HasFlag(global->flags, AST::DeclarationFlags::Extern))
			*currentStream << "extern ";

		genType(global->varType->typeID);

		for (int i = 0; i < global->declarators.size; i++)
		{
			AST::VariableDeclarator* declarator = global->declarators[i];
			*currentStream << " " << declarator->variable->mangledName;
			if (declarator->value)
				*currentStream << "=" << genExpression(declarator->value);
			if (i < global->declarators.size - 1)
				*currentStream << ",";
		}
		*currentStream << ";" << std::endl;

		currentStream = parentStream;

		functions << functionStream.str();
	}

	void genFunction(AST::Function* function)
	{
		std::stringstream* parentStream = currentStream;

		std::stringstream functionStream;
		currentStream = &functionStream;

		if (HasFlag(function->flags, AST::DeclarationFlags::Extern))
			*currentStream << "extern ";

		if (function->isEntryPoint)
		{
			*currentStream << "int";
		}
		else
		{
			if (function->returnType)
				genType(function->returnType);
			else
				*currentStream << "void";
		}

		*currentStream << " " << function->mangledName << "(";
		for (int i = 0; i < function->paramTypes.size; i++)
		{
			genType(function->paramTypes[i]);
			*currentStream << " " << function->paramNames[i];
			if (i < function->paramTypes.size - 1)
				*currentStream << ",";
		}
		*currentStream << ")";

		if (function->body)
		{
			*currentStream << "{";
			pushScope();

			genStatement(function->body);
			if (function->isEntryPoint)
				*currentStream << " return 0; ";

			popScope();
			*currentStream << "}";
		}
		else
		{
			*currentStream << ";";
		}
		*currentStream << "\n\n";

		currentStream = parentStream;

		functions << functionStream.str();
	}

public:
	CodegenTCC(CGLCompiler* context, AST::File* file)
		: context(context), file(file)
	{
	}

	std::string genFile(AST::File* file)
	{
		builtinDefinitionStream << "typedef unsigned char u8;\n";
		builtinDefinitionStream << "typedef unsigned short u16;\n";
		builtinDefinitionStream << "typedef unsigned int u32;\n";
		builtinDefinitionStream << "typedef unsigned long long u64;\n";
		builtinDefinitionStream << "typedef char i8;\n";
		builtinDefinitionStream << "typedef short i16;\n";
		builtinDefinitionStream << "typedef int i32;\n";
		builtinDefinitionStream << "typedef long long i64;\n";
		builtinDefinitionStream << "typedef struct { char* ptr; long length; } string;\n";

		for (AST::GlobalVariable* global : file->globals)
		{
			genGlobal(global);
		}

		for (AST::Function* function : file->functions)
		{
			genFunction(function);
		}

		std::stringstream fileStream;
		fileStream << builtinDefinitionStream.str() << std::endl << std::endl;
		fileStream << constants.str() << std::endl << std::endl;
		fileStream << globals.str() << std::endl << std::endl;
		fileStream << functions.str();

		return fileStream.str();
	}
};

int CGLCompiler::run(int argc, char* argv[])
{
	TCCState* tcc = tcc_new();

	tcc_add_include_path(tcc, "D:\\Repositories\\tcc\\include\\");
	tcc_add_library_path(tcc, "D:\\Repositories\\tcc\\lib\\");
	tcc_add_library(tcc, "tcc1-64");

	tcc_set_output_type(tcc, TCC_OUTPUT_MEMORY);

	for (AST::File* file : asts)
	{
		CodegenTCC codegen(this, file);
		std::string cSrc = codegen.genFile(file);
		printf("%s\n", cSrc.c_str());
		if (tcc_compile_string(tcc, cSrc.c_str()) == -1)
		{
			SnekAssert(false);
		}
	}

	int result = tcc_run(tcc, argc, argv);

	tcc_delete(tcc);

	return result;
}
