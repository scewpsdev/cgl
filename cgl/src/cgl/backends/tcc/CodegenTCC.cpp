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
	std::map<TypeID, std::string> optionalTypes;
	std::map<TypeID, std::string> tupleTypes;

	std::stringstream builtinDefinitionStream;
	std::stringstream types;
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

	std::string fpPrecisionToString(FloatingPointPrecision precision)
	{
		switch (precision)
		{
		case FloatingPointPrecision::Half: return "16";
		case FloatingPointPrecision::Single: return "32";
		case FloatingPointPrecision::Double: return "64";
		case FloatingPointPrecision::Decimal: return "80";
		case FloatingPointPrecision::Quad: return "128";
		default:
			SnekAssert(false);
			return "";
		}
	}

	std::string mangleType(TypeID type)
	{
		switch (type->typeKind)
		{
		case AST::TypeKind::Void:
			return "v";
		case AST::TypeKind::Integer:
			return std::string(type->integerType.isSigned ? "i" : "u") + std::to_string(type->integerType.bitWidth);
		case AST::TypeKind::FloatingPoint:
			return "f" + fpPrecisionToString(type->fpType.precision);
		case AST::TypeKind::Boolean:
			return "b";
		case AST::TypeKind::Struct:
			return "x" + std::string(type->structType.name);
		case AST::TypeKind::Class:
			return "X" + std::string(type->classType.name);
		case AST::TypeKind::Alias:
			return mangleType(type->aliasType.alias);
		case AST::TypeKind::Pointer:
			return "p" + mangleType(type->pointerType.elementType);
		case AST::TypeKind::Optional:
			return "o" + mangleType(type->optionalType.elementType);
		case AST::TypeKind::Function:
		{
			std::string result = "F" + mangleType(type->functionType.returnType) + std::to_string(type->functionType.numParams);
			for (int i = 0; i < type->functionType.numParams; i++)
				result = result + mangleType(type->functionType.paramTypes[i]);
			return result;
		}
		case AST::TypeKind::Tuple:
		{
			std::string result = "t" + std::to_string(type->tupleType.numValues);
			for (int i = 0; i < type->tupleType.numValues; i++)
				result = result + mangleType(type->tupleType.valueTypes[i]);
			return result;
		}
		case AST::TypeKind::Array:
			return "a" + mangleType(type->arrayType.elementType);
		case AST::TypeKind::String:
			return "s";
		default:
			SnekAssert(false);
			return "";
		}
	}

	std::string genTypeVoid()
	{
		return "void";
	}

	std::string genTypeInteger(TypeID type)
	{
		return (type->integerType.isSigned ? "i" : "u") + std::to_string(type->integerType.bitWidth);
	}

	std::string genTypeFloatingPoint(TypeID type)
	{
		switch (type->fpType.precision)
		{
		case FloatingPointPrecision::Half:
			return "f32";
		case FloatingPointPrecision::Single:
			return "f32";
		case FloatingPointPrecision::Double:
			return "f64";
		case FloatingPointPrecision::Decimal:
			return "f64";
		case FloatingPointPrecision::Quad:
			return "f64";
		default:
			SnekAssert(false);
			return "";
		}
	}

	std::string genTypeBoolean()
	{
		return "bool";
	}

	std::string genTypePointer(TypeID type)
	{
		return genType(type->pointerType.elementType) + "*";
	}

	std::string genTypeOptional(TypeID type)
	{
		auto it = optionalTypes.find(type);
		if (it == optionalTypes.end())
		{
			std::string name = "__" + mangleType(type);
			std::string str = "typedef struct{" + genType(type->optionalType.elementType) + " value;u8 hasValue;}" + name + ";";
			types << str << std::endl;
			optionalTypes.emplace(type, name);
			return name;
		}
		return optionalTypes[type];
	}

	std::string genTypeTuple(TypeID type)
	{
		auto it = tupleTypes.find(type);
		if (it == tupleTypes.end())
		{
			std::string name = "__" + mangleType(type);
			std::string str = "typedef struct{";
			for (int i = 0; i < type->tupleType.numValues; i++)
			{
				str += genType(type->tupleType.valueTypes[i]) + " _" + std::to_string(i) + ";";
			}
			str += "}" + name + ";";
			types << str << std::endl;
			tupleTypes.emplace(type, name);
			return name;
		}
		return tupleTypes[type];
	}

	std::string genTypeString()
	{
		return "string";
	}

	std::string genType(TypeID type)
	{
		switch (type->typeKind)
		{
		case AST::TypeKind::Void:
			return genTypeVoid();
		case AST::TypeKind::Integer:
			return genTypeInteger(type);
		case AST::TypeKind::FloatingPoint:
			return genTypeFloatingPoint(type);
		case AST::TypeKind::Boolean:
			return genTypeBoolean();
		case AST::TypeKind::NamedType:
		case AST::TypeKind::Struct:
		case AST::TypeKind::Class:
		case AST::TypeKind::Alias:
		case AST::TypeKind::Pointer:
			return genTypePointer(type);
		case AST::TypeKind::Optional:
			return genTypeOptional(type);
		case AST::TypeKind::Function:
		case AST::TypeKind::Tuple:
			return genTypeTuple(type);
		case AST::TypeKind::Array:
		case AST::TypeKind::String:
			return genTypeString();
		default:
			SnekAssert(false);
			return "";
		}
	}

	std::string genType(AST::Type* type)
	{
		return genType(type->typeID);
	}

	std::string castValue(AST::Expression* expression, TypeID type)
	{
		if (CompareTypes(expression->valueType, type))
			return genExpression(expression);
		else if (expression->valueType->typeKind == AST::TypeKind::Integer && type->typeKind == AST::TypeKind::Integer)
		{
			return "(" + genType(type) + ")" + genExpression(expression);
		}
		else if (expression->valueType->typeKind == AST::TypeKind::FloatingPoint && type->typeKind == AST::TypeKind::Integer)
		{
			return "(" + genType(type) + ")" + genExpression(expression);
		}
		else if (expression->valueType->typeKind == AST::TypeKind::Integer && type->typeKind == AST::TypeKind::FloatingPoint)
		{
			return "(" + genType(type) + ")" + genExpression(expression);
		}
		else if (expression->valueType->typeKind == AST::TypeKind::Pointer && expression->valueType->pointerType.elementType->typeKind == AST::TypeKind::Void &&
			type->typeKind == AST::TypeKind::Optional)
		{
			return "(" + genType(type) + ")" + genExpression(expression);
		}
		else if (expression->valueType->typeKind != AST::TypeKind::Optional && type->typeKind == AST::TypeKind::Optional &&
			CompareTypes(type->optionalType.elementType, expression->valueType))
		{
			return "(" + genType(type) + "){" + genExpression(expression) + ",1}";
		}
		// Boolean conversions
		else if (type->typeKind == AST::TypeKind::Boolean && expression->valueType->typeKind == AST::TypeKind::Integer)
		{
			return genExpression(expression);
		}
		else if (type->typeKind == AST::TypeKind::Boolean && expression->valueType->typeKind == AST::TypeKind::Pointer)
		{
			return genExpression(expression);
		}
		else if (type->typeKind == AST::TypeKind::Boolean && expression->valueType->typeKind == AST::TypeKind::Optional)
		{
			return genExpression(expression) + ".hasValue";
		}
		else if (expression->valueType->typeKind == AST::TypeKind::Pointer &&
			expression->valueType->pointerType.elementType->typeKind == AST::TypeKind::Integer &&
			expression->valueType->pointerType.elementType->integerType.bitWidth == 8 &&
			type->typeKind == AST::TypeKind::String)
		{
			std::string expr = genExpression(expression);

			if (expression->type == AST::ExpressionType::StringLiteral)
				return "(string){" + expr + "," + std::to_string(((AST::StringLiteral*)expression)->length) + "}";
			else
				return "(string){" + expr + ",strlen(" + expr + ")}";
		}
		// Conversions
		else if (expression->valueType->typeKind == AST::TypeKind::String &&
			type->typeKind == AST::TypeKind::Integer)
		{
			return "__sto" + std::string(type->integerType.isSigned ? "i" : "u") + std::to_string(type->integerType.bitWidth) + "(" + genExpression(expression) + ")";
		}
		else if (expression->valueType->typeKind == AST::TypeKind::Integer &&
			type->typeKind == AST::TypeKind::String)
		{
			return "__itos(" + genExpression(expression) + ")";
		}
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
		return expression->valueStr;
	}

	std::string genExpressionBooleanLiteral(AST::BooleanLiteral* expression)
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

	std::string genExpressionNullLiteral(AST::NullLiteral* expression)
	{
		return "{0}";
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

		return "(string){" + std::string(globalName) + "," + std::to_string(expression->length) + "}";
	}

	void importGlobal(Variable* variable)
	{
		std::stringstream* parentStream = currentStream;
		currentStream = &globals;

		*currentStream << "extern " << genType(variable->type) << " " << variable->mangledName << ";" << std::endl;

		currentStream = parentStream;
	}

	std::string getVariableValue(Variable* variable)
	{
		if (file != variable->file)
		{
			if (!importedGlobals.contains(variable))
			{
				importGlobal(variable);
				importedGlobals.add(variable);
			}
		}
		return variable->mangledName;
	}

	std::string genExpressionIdentifier(AST::Identifier* expression)
	{
		if (expression->variable)
		{
			return getVariableValue(expression->variable);
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

	std::string genExpressionCompound(AST::CompoundExpression* expression)
	{
		return "(" + genExpression(expression->value) + ")";
	}

	std::string genExpressionTuple(AST::TupleExpression* expression)
	{
		std::stringstream result;
		result << "{";
		for (int i = 0; i < expression->values.size; i++)
		{
			AST::Expression* value = expression->values[i];
			result << genExpression(value);
			if (i < expression->values.size - 1)
				result << ",";
		}
		result << "}";
		return result.str();
	}

	std::string genExpressionFunctionCall(AST::FunctionCall* expression)
	{
		if (expression->isCast)
		{
			SnekAssert(expression->arguments.size == 1);
			return castValue(expression->arguments[0], expression->castDstType);
		}
		else
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
				std::string arg = castValue(expression->arguments[i], expression->callee->valueType->functionType.paramTypes[i]);
				*currentStream << arg;
				if (i < expression->arguments.size - 1)
					*currentStream << ",";
			}
			*currentStream << ")";

			currentStream = parentStream;

			*currentStream << callStream.str();

			return returnValue;
		}
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
			return getVariableValue(expression->namespacedVariable);
		else if (expression->namespacedFunctions.size > 0)
			return expression->namespacedFunctions[0]->mangledName;
		else if (expression->builtinTypeProperty != AST::BuiltinTypeProperty::Null)
		{
			switch (expression->builtinTypeProperty)
			{
			case AST::BuiltinTypeProperty::Int8Min: return "__INT8_MIN";
			case AST::BuiltinTypeProperty::Int8Max: return "__INT8_MAX";
			case AST::BuiltinTypeProperty::Int16Min: return "__INT16_MIN";
			case AST::BuiltinTypeProperty::Int16Max: return "__INT16_MAX";
			case AST::BuiltinTypeProperty::Int32Min: return "__INT32_MIN";
			case AST::BuiltinTypeProperty::Int32Max: return "__INT32_MAX";
			case AST::BuiltinTypeProperty::Int64Min: return "__INT64_MIN";
			case AST::BuiltinTypeProperty::Int64Max: return "__INT64_MAX";
			case AST::BuiltinTypeProperty::UInt8Min: return "__UINT8_MIN";
			case AST::BuiltinTypeProperty::UInt8Max: return "__UINT8_MAX";
			case AST::BuiltinTypeProperty::UInt16Min: return "__UINT16_MIN";
			case AST::BuiltinTypeProperty::UInt16Max: return "__UINT16_MAX";
			case AST::BuiltinTypeProperty::UInt32Min: return "__UINT32_MIN";
			case AST::BuiltinTypeProperty::UInt32Max: return "__UINT32_MAX";
			case AST::BuiltinTypeProperty::UInt64Min: return "__UINT64_MIN";
			case AST::BuiltinTypeProperty::UInt64Max: return "__UINT64_MAX";
			default:
				SnekAssert(false);
				return "";
			}
		}
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
		else if (expression->fieldIndex != -1)
		{
			if (expression->operand->valueType->typeKind == AST::TypeKind::Optional)
			{
				if (expression->fieldIndex == 0)
					return genExpression(expression->operand) + ".value";
				else if (expression->fieldIndex == 1)
					return genExpression(expression->operand) + ".hasValue";
				else
				{
					SnekAssert(false);
					return "";
				}
			}
			else if (expression->operand->valueType->typeKind == AST::TypeKind::Tuple)
			{
				SnekAssert(expression->fieldIndex >= 0 && expression->fieldIndex < expression->operand->valueType->tupleType.numValues);
				return genExpression(expression->operand) + "._" + std::to_string(expression->fieldIndex);
			}
			else if (expression->operand->valueType->typeKind == AST::TypeKind::Array)
			{
				if (expression->fieldIndex == 0)
					return genExpression(expression->operand) + ".length";
				else if (expression->fieldIndex == 1)
					return genExpression(expression->operand) + ".buffer";
				else
				{
					SnekAssert(false);
					return "";
				}
			}
			else if (expression->operand->valueType->typeKind == AST::TypeKind::String)
			{
				if (expression->fieldIndex == 0)
					return genExpression(expression->operand) + ".length";
				else if (expression->fieldIndex == 1)
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
		else
		{
			SnekAssert(false);
			return "";
		}
	}

	std::string genExpressionTypecast(AST::Typecast* expression)
	{
		return castValue(expression->value, expression->dstType->typeID);
	}

	std::string compareValues(AST::Expression* left, AST::Expression* right)
	{
		if (left->valueType->typeKind == AST::TypeKind::Optional &&
			right->valueType->typeKind == AST::TypeKind::Pointer && right->valueType->pointerType.elementType->typeKind == AST::TypeKind::Void)
		{
			SnekAssert(dynamic_cast<AST::NullLiteral*>(right));
			return "!(" + genExpression(left) + ".hasValue)";
		}
		else
		{
			return genExpression(left) + "==" + genExpression(right);
		}
	}

	std::string genExpressionBinaryOperator(AST::BinaryOperator* expression)
	{
		switch (expression->operatorType)
		{
		case AST::BinaryOperatorType::Add:
			return castValue(expression->left, expression->valueType) + "+" + castValue(expression->right, expression->valueType);
		case AST::BinaryOperatorType::Subtract:
			return castValue(expression->left, expression->valueType) + "-" + castValue(expression->right, expression->valueType);
		case AST::BinaryOperatorType::Multiply:
			return castValue(expression->left, expression->valueType) + "*" + castValue(expression->right, expression->valueType);
		case AST::BinaryOperatorType::Divide:
			return castValue(expression->left, expression->valueType) + "/" + castValue(expression->right, expression->valueType);
		case AST::BinaryOperatorType::Modulo:
			return castValue(expression->left, expression->valueType) + "%" + castValue(expression->right, expression->valueType);
		case AST::BinaryOperatorType::Equals:
			return compareValues(expression->left, expression->right);
		case AST::BinaryOperatorType::DoesNotEqual:
			return "!(" + compareValues(expression->left, expression->right) + ")";
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
			return genExpressionBooleanLiteral((AST::BooleanLiteral*)expression);
		case AST::ExpressionType::CharacterLiteral:
			return genExpressionCharacterLiteral((AST::CharacterLiteral*)expression);
		case AST::ExpressionType::NullLiteral:
			return genExpressionNullLiteral((AST::NullLiteral*)expression);
		case AST::ExpressionType::StringLiteral:
			return genExpressionStringLiteral((AST::StringLiteral*)expression);
		case AST::ExpressionType::InitializerList:
		case AST::ExpressionType::Identifier:
			return genExpressionIdentifier((AST::Identifier*)expression);
		case AST::ExpressionType::Compound:
			return genExpressionCompound((AST::CompoundExpression*)expression);
		case AST::ExpressionType::Tuple:
			return genExpressionTuple((AST::TupleExpression*)expression);
		case AST::ExpressionType::FunctionCall:
			return genExpressionFunctionCall((AST::FunctionCall*)expression);
		case AST::ExpressionType::SubscriptOperator:
			return genExpressionSubscriptOperator((AST::SubscriptOperator*)expression);
		case AST::ExpressionType::DotOperator:
			return genExpressionDotOperator((AST::DotOperator*)expression);
		case AST::ExpressionType::Typecast:
			return genExpressionTypecast((AST::Typecast*)expression);
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

		stream << genType(statement->varType) << " ";

		if (statement->isConstant)
			stream << "const ";

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
			else
			{
				stream << "={0}";
			}
			if (i < statement->declarators.size - 1)
				stream << ",";
		}
		stream << ";";

		currentStream = parentStream;
		*currentStream << stream.str();
		newLine();
	}

	void genStatementIf(AST::IfStatement* statement)
	{
		std::stringstream* parentStream = currentStream;
		std::stringstream stream;
		currentStream = &stream;

		stream << "if(" << castValue(statement->condition, GetBoolType()) << ")";
		genStatement(statement->thenStatement);
		if (statement->elseStatement)
		{
			//newLine();
			stream << " else ";
			genStatement(statement->elseStatement);
		}

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
			genStatementIf((AST::IfStatement*)statement);
			break;
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
				*currentStream << genType(function->returnType);
			else
				*currentStream << "void";
		}

		*currentStream << " " << function->mangledName << "(";
		for (int i = 0; i < function->paramTypes.size; i++)
		{
			*currentStream << genType(function->paramTypes[i]) << " " << function->paramNames[i];
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

	void genTypedef(AST::Typedef* td)
	{
		std::stringstream* parentStream = currentStream;

		std::stringstream typedefStream;
		currentStream = &typedefStream;

		*currentStream << "typedef " << genType(td->alias) << " " << td->name << ";";

		currentStream = parentStream;

		types << typedefStream.str();
	}

	void genGlobal(AST::GlobalVariable* global)
	{
		std::stringstream* parentStream = currentStream;

		std::stringstream globalStream;
		currentStream = &globalStream;

		if (HasFlag(global->flags, AST::DeclarationFlags::Extern))
			*currentStream << "extern ";

		*currentStream << genType(global->varType->typeID);

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

		globals << globalStream.str();
	}

public:
	CodegenTCC(CGLCompiler* context, AST::File* file)
		: context(context), file(file)
	{
	}

	std::string genFile(AST::File* file)
	{
		builtinDefinitionStream << "// " << file->name << "\n";

		builtinDefinitionStream << "typedef char i8;\n";
		builtinDefinitionStream << "typedef short i16;\n";
		builtinDefinitionStream << "typedef int i32;\n";
		builtinDefinitionStream << "typedef long long i64;\n";
		builtinDefinitionStream << "typedef unsigned char u8;\n";
		builtinDefinitionStream << "typedef unsigned short u16;\n";
		builtinDefinitionStream << "typedef unsigned int u32;\n";
		builtinDefinitionStream << "typedef unsigned long long u64;\n";
		builtinDefinitionStream << "typedef float f32;\n";
		builtinDefinitionStream << "typedef double f64;\n";
		builtinDefinitionStream << "typedef _Bool bool;\n";
		builtinDefinitionStream << "typedef struct { char* ptr; long length; } string;\n";

		builtinDefinitionStream << "#define __INT8_MIN -0x80\n";
		builtinDefinitionStream << "#define __INT8_MAX 0x7f\n";
		builtinDefinitionStream << "#define __INT16_MIN -0x8000\n";
		builtinDefinitionStream << "#define __INT16_MAX 0x7fff\n";
		builtinDefinitionStream << "#define __INT32_MIN -0x80000000\n";
		builtinDefinitionStream << "#define __INT32_MAX 0x7fffffff\n";
		builtinDefinitionStream << "#define __INT64_MIN -0x8000000000000000\n";
		builtinDefinitionStream << "#define __INT64_MAX 0x7fffffffffffffff\n";
		builtinDefinitionStream << "#define __UINT8_MIN 0\n";
		builtinDefinitionStream << "#define __UINT8_MAX 0xff\n";
		builtinDefinitionStream << "#define __UINT16_MIN 0\n";
		builtinDefinitionStream << "#define __UINT16_MAX 0xffff\n";
		builtinDefinitionStream << "#define __UINT32_MIN 0\n";
		builtinDefinitionStream << "#define __UINT32_MAX 0xffffffff\n";
		builtinDefinitionStream << "#define __UINT64_MIN 0\n";
		builtinDefinitionStream << "#define __UINT64_MAX 0xffffffffffffffff\n";

		builtinDefinitionStream << "i8 __stoi8(string s);\n";
		builtinDefinitionStream << "i16 __stoi16(string s);\n";
		builtinDefinitionStream << "i32 __stoi32(string s);\n";
		builtinDefinitionStream << "i64 __stoi64(string s);\n";
		builtinDefinitionStream << "u8 __stou8(string s);\n";
		builtinDefinitionStream << "u16 __stou16(string s);\n";
		builtinDefinitionStream << "u32 __stou32(string s);\n";
		builtinDefinitionStream << "u64 __stou64(string s);\n";
		builtinDefinitionStream << "string __itos(i64 i);\n";
		builtinDefinitionStream << "string __utos(u64 n);\n";

		for (AST::Function* function : file->functions)
		{
			genFunction(function);
		}
		for (AST::Typedef* td : file->typedefs)
		{
			genTypedef(td);
		}
		for (AST::GlobalVariable* global : file->globals)
		{
			genGlobal(global);
		}

		std::stringstream fileStream;
		if (builtinDefinitionStream.seekg(0, std::ios::end).tellg() > 0)
			fileStream << builtinDefinitionStream.str() << std::endl << std::endl;
		if (types.seekg(0, std::ios::end).tellg() > 0)
			fileStream << types.str() << std::endl << std::endl;
		if (constants.seekg(0, std::ios::end).tellg() > 0)
			fileStream << constants.str() << std::endl << std::endl;
		if (globals.seekg(0, std::ios::end).tellg() > 0)
			fileStream << globals.str() << std::endl << std::endl;
		if (functions.seekg(0, std::ios::end).tellg() > 0)
			fileStream << functions.str();

		return fileStream.str();
	}
};

static char* ReadText(const char* path)
{
	if (FILE* file = fopen(path, "r"))
	{
		fseek(file, 0, SEEK_END);
		long numBytes = ftell(file);
		fseek(file, 0, SEEK_SET);

		char* buffer = new char[numBytes + 1];
		memset(buffer, 0, numBytes);
		fread(buffer, 1, numBytes, file);
		fclose(file);
		buffer[numBytes] = 0;

		return buffer;
	}
	return nullptr;
}

int CGLCompiler::run(int argc, char* argv[])
{
	TCCState* tcc = tcc_new();

	tcc_add_include_path(tcc, "C:\\Users\\faris\\Documents\\Dev\\tcc\\include\\");
	tcc_add_library_path(tcc, "C:\\Users\\faris\\Documents\\Dev\\tcc\\lib\\");
	tcc_add_library(tcc, "tcc1-64");

	tcc_set_output_type(tcc, TCC_OUTPUT_MEMORY);

	{
		char* builtinSrc = ReadText("cgl.c");
		tcc_compile_string(tcc, builtinSrc);
		delete builtinSrc;
	}

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
