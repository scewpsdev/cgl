#include "cgl/CGLCompiler.h"

#include "lib/libtcc/libtcc.h"

#include "cgl/semantics/Variable.h"
#include "cgl/semantics/Function.h"


struct Scope
{
	Scope* parent = nullptr;
};

class CodegenTCC
{
	CGLCompiler* context;
	AST::File* file;

	List<Variable*> importedGlobals;
	List<AST::Function*> importedFunctions;
	std::map<TypeID, std::string> optionalTypes;
	std::map<TypeID, std::string> tupleTypes;
	std::map<TypeID, std::string> arrayTypes;

	std::stringstream builtinDefinitionStream;
	std::stringstream types;
	std::stringstream constants;
	std::stringstream globals;
	std::stringstream functionDeclarations;
	std::stringstream functions;

	std::stringstream* instructionStream = nullptr;

	AST::Function* currentFunction = nullptr;
	Scope* scope = nullptr;

	int unnamedLocalId = 0;
	int unnamedGlobalId = 0;

	int indentation = 0;


	void newLine(std::stringstream& stream)
	{
		stream << "\n";
		for (int i = 0; i < indentation; i++)
			stream << "\t";
	}

	void newLine()
	{
		newLine(*instructionStream);
	}

	bool isWhitespace(char c)
	{
		return c == ' ' || c == '\t' || c == '\n' || c == '\r';
	}

	void stepBackWhitespace()
	{
		instructionStream->seekg(-1, std::ios::end);
		char c = instructionStream->get();
		if (isWhitespace(c))
		{
			instructionStream->seekp(-1, std::ios::end);
		}
		instructionStream->seekg(0, std::ios::end);
	}

	void stepBackPastWhitespace()
	{
		int i = 1;
		while (true)
		{
			instructionStream->seekg(-i, std::ios::end);
			//instructionStream->seekp(instructionStream->cur-1);
			char c;
			instructionStream->read(&c, 1);
			//char c = instructionStream->get();
			if (isWhitespace(c))
				i++;
			else
			{
				instructionStream->seekp(-i + 1, std::ios::end);
				instructionStream->seekg(0, std::ios::end);
				return;
			}
		}
	}

	void newGlobalName(char name[8])
	{
		name[0] = '_';
		name[1] = '_';
		name[2] = 'G';
		sprintf(&name[3], "%d", unnamedGlobalId++);
	}

	std::string newLocalName()
	{
		return "__" + std::to_string(unnamedLocalId++);
	}

	Scope* pushScope()
	{
		Scope* newScope = new Scope();
		newScope->parent = scope;
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

	std::string genTypeArray(TypeID type)
	{
		auto it = arrayTypes.find(type);
		if (it == arrayTypes.end())
		{
			std::string name = "__" + mangleType(type);

			if (type->arrayType.length != -1)
			{
				std::string str = "typedef struct{" + genType(type->arrayType.elementType) + " buffer[" + std::to_string(type->arrayType.length) + "];}" + name + ";";
				types << str << std::endl;
				//return "struct{" + genType(type->arrayType.elementType) + " buffer[" + std::to_string(type->arrayType.length) + "];}";
			}
			else
			{
				std::string str = "typedef struct{" + genType(type->arrayType.elementType) + "* ptr;long length;}" + name + ";";
				types << str << std::endl;
				//return "struct{" + genType(type->arrayType.elementType) + "* ptr;long length;}";
			}

			arrayTypes.emplace(type, name);
			return name;
		}
		return arrayTypes[type];
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
			return genTypeArray(type);
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

	std::string castValue(std::string expression, TypeID valueType, TypeID type, AST::Expression* ast = nullptr)
	{
		if (CompareTypes(valueType, type))
			return expression;
		else if (valueType->typeKind == AST::TypeKind::Integer && type->typeKind == AST::TypeKind::Integer)
		{
			return "(" + genType(type) + ")" + expression;
		}
		else if (valueType->typeKind == AST::TypeKind::FloatingPoint && type->typeKind == AST::TypeKind::Integer)
		{
			return "(" + genType(type) + ")" + expression;
		}
		else if (valueType->typeKind == AST::TypeKind::Integer && type->typeKind == AST::TypeKind::FloatingPoint)
		{
			return "(" + genType(type) + ")" + expression;
		}
		else if (valueType->typeKind == AST::TypeKind::Pointer && valueType->pointerType.elementType->typeKind == AST::TypeKind::Void &&
			type->typeKind == AST::TypeKind::Optional)
		{
			return "(" + genType(type) + ")" + expression;
		}
		else if (valueType->typeKind != AST::TypeKind::Optional && type->typeKind == AST::TypeKind::Optional &&
			CompareTypes(type->optionalType.elementType, valueType))
		{
			return "(" + genType(type) + "){" + expression + ",1}";
		}
		// Boolean conversions
		else if (type->typeKind == AST::TypeKind::Boolean && valueType->typeKind == AST::TypeKind::Integer)
		{
			return expression;
		}
		else if (type->typeKind == AST::TypeKind::Boolean && valueType->typeKind == AST::TypeKind::Pointer)
		{
			return expression;
		}
		else if (type->typeKind == AST::TypeKind::Boolean && valueType->typeKind == AST::TypeKind::Optional)
		{
			return expression + ".hasValue";
		}
		else if (valueType->typeKind == AST::TypeKind::Pointer &&
			valueType->pointerType.elementType->typeKind == AST::TypeKind::Integer &&
			valueType->pointerType.elementType->integerType.bitWidth == 8 &&
			type->typeKind == AST::TypeKind::String)
		{
			if (ast && ast->type == AST::ExpressionType::StringLiteral)
				return "(string){" + expression + "," + std::to_string(((AST::StringLiteral*)ast)->length) + "}";
			else
				return "(string){" + expression + ",strlen(" + expression + ")}";
		}
		// Conversions
		else if (valueType->typeKind == AST::TypeKind::String &&
			type->typeKind == AST::TypeKind::Integer)
		{
			return "__sto" + std::string(type->integerType.isSigned ? "i" : "u") + std::to_string(type->integerType.bitWidth) + "(" + expression + ")";
		}
		else if (valueType->typeKind == AST::TypeKind::Integer &&
			type->typeKind == AST::TypeKind::String)
		{
			return "__itos(" + expression + ")";
		}
		else
		{
			SnekAssert(false);
			return "";
		}
	}

	std::string castValue(AST::Expression* expression, TypeID type)
	{
		return castValue(genExpression(expression), expression->valueType, type, expression);
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

	void writeStringLiteral(const char* str, int length, std::stringstream& stream)
	{
		stream << "\"";
		for (int i = 0; i < length; i++)
		{
			char c = str[i];
			if (c == '\n')
				stream << "\\n";
			else if (c == '\r')
				stream << "\\r";
			else if (c == '\t')
				stream << "\\t";
			else if (c == '\\')
				stream << "\\\\";
			else if (c == '\0')
				stream << "\\0";
			else if (c == '"')
				stream << "\\\"";
			else
				stream << c;
		}
		stream << "\"";
	}

	void writeStringLiteral(const char* str, std::stringstream& stream)
	{
		writeStringLiteral(str, (int)strlen(str), stream);
	}

	std::string genExpressionStringLiteral(AST::StringLiteral* expression)
	{
		char globalName[8];
		newGlobalName(globalName);

		constants << "static char " << globalName << "[]=";
		writeStringLiteral(expression->value, expression->length, constants);
		constants << ";\n";

		return "(string){" + std::string(globalName) + "," + std::to_string(expression->length) + "}";
	}

	void importGlobal(Variable* variable)
	{
		std::stringstream* parentStream = instructionStream;
		instructionStream = &globals;

		*instructionStream << "extern " << genType(variable->type) << " " << variable->mangledName << ";" << std::endl;

		instructionStream = parentStream;
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

	void importFunction(AST::Function* function)
	{
		std::stringstream* parentStream = instructionStream;
		instructionStream = &functionDeclarations;

		*instructionStream << "extern " << genType(function->functionType->functionType.returnType) << " " << function->mangledName << "(";

		for (int i = 0; i < function->functionType->functionType.numParams; i++)
		{
			*instructionStream << genType(function->functionType->functionType.paramTypes[i]) << " " << function->paramNames[i];
			if (i < function->paramTypes.size - 1)
				*instructionStream << ",";
		}

		*instructionStream << ");" << std::endl;

		instructionStream = parentStream;
	}

	std::string getFunctionValue(AST::Function* function)
	{
		if (file != function->file)
		{
			if (!importedFunctions.contains(function))
			{
				importFunction(function);
				importedFunctions.add(function);
			}
		}
		return function->mangledName;
	}

	std::string genExpressionInitializerList(AST::InitializerList* expression)
	{
		SnekAssert(expression->initializerType);
		if (expression->initializerType->typeKind == AST::TypeKind::Array)
		{
			if (expression->initializerType->arrayType.length != -1)
			{
				std::stringstream result;
				result << "{";
				for (int i = 0; i < expression->values.size; i++)
				{
					result << castValue(expression->values[i], expression->initializerType->arrayType.elementType);
					if (i < expression->values.size - 1)
						result << ",";
				}
				result << "}";
				return result.str();
			}
			else
			{
				char globalName[8];
				newGlobalName(globalName);
				constants << "static " << genType(expression->initializerType->arrayType.elementType) << " " << globalName << "[]={";
				for (int i = 0; i < expression->values.size; i++)
				{
					constants << castValue(expression->values[i], expression->initializerType->arrayType.elementType);
					if (i < expression->values.size - 1)
						constants << ",";
				}
				constants << "};" << std::endl;

				return "{" + std::string(globalName) + "," + std::to_string(expression->values.size) + "}";
			}
		}
		else
		{
			return "";
		}
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
			return getFunctionValue(expression->functions[0]);
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
		result << "(" << genType(expression->valueType) << "){";
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
			//auto parentStream = currentStream;
			std::stringstream callStream;
			//currentStream = &callStream;

			std::string returnValue = "";

			SnekAssert(expression->callee->valueType->typeKind == AST::TypeKind::Function);
			if (expression->callee->valueType->functionType.returnType->typeKind != AST::TypeKind::Void)
			{
				returnValue = newLocalName();
				callStream << genType(expression->callee->valueType->functionType.returnType) << " " << returnValue << "=";
			}

			std::string callee = genExpression(expression->callee);
			callStream << callee << "(";
			for (int i = 0; i < expression->arguments.size; i++)
			{
				std::string arg = castValue(expression->arguments[i], expression->callee->valueType->functionType.paramTypes[i]);
				callStream << arg;
				if (i < expression->arguments.size - 1)
					callStream << ",";
			}
			callStream << ");";
			newLine(callStream);

			//currentStream = parentStream;

			*instructionStream << callStream.str();

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
			if (expression->operand->valueType->arrayType.length != -1)
				return operand + ".buffer[" + genExpression(expression->arguments[0]) + "]";
			else
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

	std::string genExpressionUnaryOperator(AST::UnaryOperator* expression)
	{
		switch (expression->operatorType)
		{
		case AST::UnaryOperatorType::Not:
			SnekAssert(!expression->position);
			return "!" + genExpression(expression->operand);
		case AST::UnaryOperatorType::Negate:
			SnekAssert(!expression->position);
			return "-" + genExpression(expression->operand);
		case AST::UnaryOperatorType::Reference:
			SnekAssert(!expression->position);
			return "&" + genExpression(expression->operand);
		case AST::UnaryOperatorType::Dereference:
			SnekAssert(!expression->position);
			return "*" + genExpression(expression->operand);
		case AST::UnaryOperatorType::Increment:
			if (!expression->position)
				return "++" + genExpression(expression->operand);
			else
				return genExpression(expression->operand) + "++";
		case AST::UnaryOperatorType::Decrement:
			if (!expression->position)
				return "--" + genExpression(expression->operand);
			else
				return genExpression(expression->operand) + "--";
		default:
			SnekAssert(false);
			return "";
		}
		return "";
	}

	std::string genExpressionBinaryOperator(AST::BinaryOperator* expression)
	{
		auto left = genExpression(expression->left);
		auto right = genExpression(expression->right);

		switch (expression->operatorType)
		{
		case AST::BinaryOperatorType::Add:
			return castValue(left, expression->left->valueType, expression->valueType) + "+" + castValue(right, expression->right->valueType, expression->valueType);
		case AST::BinaryOperatorType::Subtract:
			return castValue(left, expression->left->valueType, expression->valueType) + "-" + castValue(right, expression->right->valueType, expression->valueType);
		case AST::BinaryOperatorType::Multiply:
			return castValue(left, expression->left->valueType, expression->valueType) + "*" + castValue(right, expression->right->valueType, expression->valueType);
		case AST::BinaryOperatorType::Divide:
			return castValue(left, expression->left->valueType, expression->valueType) + "/" + castValue(right, expression->right->valueType, expression->valueType);
		case AST::BinaryOperatorType::Modulo:
			return castValue(left, expression->left->valueType, expression->valueType) + "%" + castValue(right, expression->right->valueType, expression->valueType);
		case AST::BinaryOperatorType::Equals:
		{
			if (expression->left->valueType->typeKind == AST::TypeKind::Optional &&
				expression->right->valueType->typeKind == AST::TypeKind::Pointer && expression->right->valueType->pointerType.elementType->typeKind == AST::TypeKind::Void)
			{
				SnekAssert(dynamic_cast<AST::NullLiteral*>(expression->right));
				return "!" + left + ".hasValue";
			}
			else
			{
				return left + "==" + right;
			}
		}
		case AST::BinaryOperatorType::DoesNotEqual:
			if (expression->left->valueType->typeKind == AST::TypeKind::Optional &&
				expression->right->valueType->typeKind == AST::TypeKind::Pointer && expression->right->valueType->pointerType.elementType->typeKind == AST::TypeKind::Void)
			{
				SnekAssert(dynamic_cast<AST::NullLiteral*>(expression->right));
				return left + ".hasValue";
			}
			else
			{
				return left + "!=" + right;
			}
		case AST::BinaryOperatorType::LessThan:
			return left + "<" + right;
		case AST::BinaryOperatorType::GreaterThan:
			return left + ">" + right;
		case AST::BinaryOperatorType::LessThanEquals:
			return left + "<=" + right;
		case AST::BinaryOperatorType::GreaterThanEquals:
			return left + ">=" + right;
		case AST::BinaryOperatorType::LogicalAnd:
			return left + "&&" + right;
		case AST::BinaryOperatorType::LogicalOr:
			return left + "||" + right;
		case AST::BinaryOperatorType::BitwiseAnd:
		case AST::BinaryOperatorType::BitwiseOr:
		case AST::BinaryOperatorType::BitwiseXor:
		case AST::BinaryOperatorType::BitshiftLeft:
		case AST::BinaryOperatorType::BitshiftRight:
		case AST::BinaryOperatorType::Assignment:
		{
			*instructionStream << left << "=" << castValue(right, expression->right->valueType, expression->left->valueType) << ";";
			newLine();
			return left;
		}
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
		case AST::BinaryOperatorType::NullCoalescing:
		{
			return left + ".hasValue?" + left + ".value:" + castValue(right, expression->right->valueType, expression->left->valueType->optionalType.elementType);
		}
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
			return genExpressionInitializerList((AST::InitializerList*)expression);
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
			return genExpressionUnaryOperator((AST::UnaryOperator*)expression);
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
		//*instructionStream << "{";
		//indentation++;
		//newLine();

		//pushScope();
		for (int i = 0; i < statement->statements.size; i++)
		{
			genStatement(statement->statements[i]);
		}
		//popScope();

		//instructionStream->seekp(-1, instructionStream->cur);
		//indentation--;
		//newLine();
		//*instructionStream << "}";
	}

	char getLastNonWhitespaceChar(std::stringstream& stream)
	{
		int i = 1;
		while (true)
		{
			instructionStream->seekg(-i++, std::ios::end);
			char c = instructionStream->get();
			if (!isWhitespace(c))
			{
				instructionStream->seekg(0, std::ios::beg);
				return c;
			}
		}
		SnekAssert(false);
		return 0;
	}

	void genStatementExpression(AST::ExpressionStatement* statement)
	{
		genExpression(statement->expression);
		/*
		*instructionStream << genExpression(statement->expression);
		auto c = getLastNonWhitespaceChar(*instructionStream);
		if (c != ';')
		{
			*instructionStream << ";";
			newLine();
		}
		*/
	}

	void genStatementVariableDeclaration(AST::VariableDeclaration* statement)
	{
		//std::stringstream* parentStream = instructionStream;
		std::stringstream stream;
		//instructionStream = &stream;

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

		//instructionStream = parentStream;
		*instructionStream << stream.str();
		newLine();
	}

	void genStatementIf(AST::IfStatement* statement)
	{
		std::stringstream* parentStream = instructionStream;
		std::stringstream stream;
		instructionStream = &stream;

		auto condition = genExpression(statement->condition);

		stream << "if(" << castValue(condition, statement->condition->valueType, GetBoolType()) << "){";

		indentation++;
		newLine();

		genStatement(statement->thenStatement);
		if (statement->elseStatement)
		{
			if (statement->elseStatement->type == AST::StatementType::If)
			{
				stepBackWhitespace();
				indentation--;

				stream << "}else ";
				genStatement(statement->elseStatement);
			}
			else
			{
				stepBackWhitespace();
				indentation--;
				//newLine();

				stream << "}else{";

				indentation++;
				newLine();

				genStatement(statement->elseStatement);

				stepBackWhitespace();
				indentation--;
				//newLine();

				stream << "}";
			}
		}
		else
		{
			stepBackWhitespace();
			indentation--;
			//newLine();

			stream << "}";
		}

		instructionStream = parentStream;
		*instructionStream << stream.str();
		newLine();
	}

	void genStatementFor(AST::ForLoop* statement)
	{
		std::stringstream* parentStream = instructionStream;
		std::stringstream stream;
		instructionStream = &stream;

		/*
		*currentStream
			<< "for(int " << statement->iteratorName->name << "=" << genExpression(statement->startValue) << ";"
			<< statement->iteratorName->name << "<" << genExpression(statement->endValue) << ";"
			<< statement->iteratorName->name << "++)";
			*/

		*instructionStream << "for(";

		if (!statement->iteratorName)
		{
			if (statement->initStatement)
			{
				genStatement(statement->initStatement);
				stepBackPastWhitespace();
				//instructionStream->seekp(-2, instructionStream->cur);
			}
			else
			{
				*instructionStream << ";";
			}
			//*currentStream << ";";
			if (statement->conditionExpr)
				*instructionStream << genExpression(statement->conditionExpr);
			*instructionStream << ";";
			if (statement->iterateExpr)
				*instructionStream << genExpression(statement->iterateExpr);
			*instructionStream << "){";

			indentation++;
			newLine();
		}
		else
		{
			std::string container = genExpression(statement->container);

			*instructionStream << "int __it=0;__it<";
			if (statement->container->valueType->typeKind == AST::TypeKind::Array)
			{
				if (statement->container->valueType->arrayType.length != -1)
					*instructionStream << statement->container->valueType->arrayType.length;
				else
					*instructionStream << container << ".length";
			}
			else
			{
				SnekAssert(false);
			}

			*instructionStream << ";__it++){";

			indentation++;
			newLine();

			*instructionStream << genType(statement->container->valueType->arrayType.elementType) << " const " << statement->iteratorName << "=";

			if (statement->container->valueType->typeKind == AST::TypeKind::Array)
			{
				if (statement->container->valueType->arrayType.length != -1)
					*instructionStream << container << ".buffer[__it]";
				else
					*instructionStream << container << ".ptr[__it]";
			}
			else
			{
				SnekAssert(false);
			}

			*instructionStream << ";";
			newLine();
		}

		genStatement(statement->body);

		stepBackWhitespace();
		indentation--;
		//newLine();

		*instructionStream << "}";

		instructionStream = parentStream;
		*instructionStream << stream.str();
		newLine();
	}

	void genStatementReturn(AST::Return* statement)
	{
		if (statement->value)
		{
			SnekAssert(currentFunction->functionType->functionType.returnType->typeKind != AST::TypeKind::Void);
			std::string value = castValue(statement->value, currentFunction->functionType->functionType.returnType);
			*instructionStream << "return " << value << ";";
			newLine();
		}
		else
		{
			*instructionStream << "return;";
			newLine();
		}
	}

	void genStatementAssert(AST::Assert* statement)
	{
		*instructionStream << "if(!(" << castValue(statement->condition, GetBoolType()) << ")){";
		indentation++;
		newLine();

		if (statement->message)
		{
			char message[128] = {};
			sprintf(message, "Assertion failed at %s:%d:%d: \"%%s\"\n", statement->file->name, statement->location.line, statement->location.col);

			*instructionStream << "printf(";
			writeStringLiteral(message, *instructionStream);
			*instructionStream << ", " << genExpression(statement->message) << ".ptr);";
		}
		else
		{
			char message[128] = {};
			sprintf(message, "Assertion failed at %s:%d:%d\n", statement->file->name, statement->location.line, statement->location.col);

			*instructionStream << "printf(";
			writeStringLiteral(message, *instructionStream);
			*instructionStream << ");";
		}
		newLine();

		*instructionStream << "__asm__ volatile(\"int $0x03\");";
		indentation--;
		newLine();

		*instructionStream << "}";
		newLine();
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
		case AST::StatementType::Assert:
			genStatementAssert((AST::Assert*)statement);
			break;
		case AST::StatementType::Free:
		default:
			SnekAssert(false);
			break;
		}
	}

	void genFunction(AST::Function* function)
	{
		std::stringstream* parentStream = instructionStream;

		std::stringstream functionStream;
		instructionStream = &functionStream;

		if (HasFlag(function->flags, AST::DeclarationFlags::Extern))
			*instructionStream << "extern ";

		if (function->isEntryPoint)
		{
			*instructionStream << "int";
		}
		else
		{
			*instructionStream << genType(function->functionType->functionType.returnType);
		}

		*instructionStream << " " << function->mangledName << "(";
		for (int i = 0; i < function->functionType->functionType.numParams; i++)
		{
			*instructionStream << genType(function->functionType->functionType.paramTypes[i]) << " " << function->paramNames[i];
			if (i < function->functionType->functionType.numParams - 1)
				*instructionStream << ",";
		}
		*instructionStream << ")";

		unnamedLocalId = 0;

		if (function->body)
		{
			*instructionStream << "{";
			indentation++;
			newLine();
			pushScope();

			currentFunction = function;

			genStatement(function->body);
			if (function->isEntryPoint)
				*instructionStream << " return 0; ";

			currentFunction = nullptr;

			popScope();
			stepBackWhitespace();
			//instructionStream->seekp(-1, instructionStream->cur);
			indentation--;
			//newLine();
			*instructionStream << "}";
		}
		else if (function->bodyExpression)
		{
			*instructionStream << "{";
			pushScope();

			*instructionStream << "return " << genExpression(function->bodyExpression) << ";";

			popScope();
			*instructionStream << "}";
		}
		else
		{
			*instructionStream << ";";
		}
		*instructionStream << "\n\n";

		instructionStream = parentStream;

		functions << functionStream.str();
	}

	void genTypedef(AST::Typedef* td)
	{
		std::stringstream* parentStream = instructionStream;

		std::stringstream typedefStream;
		instructionStream = &typedefStream;

		*instructionStream << "typedef " << genType(td->alias) << " " << td->name << ";";

		instructionStream = parentStream;

		types << typedefStream.str();
	}

	void genGlobal(AST::GlobalVariable* global)
	{
		std::stringstream* parentStream = instructionStream;

		std::stringstream globalStream;
		instructionStream = &globalStream;

		if (HasFlag(global->flags, AST::DeclarationFlags::Extern))
			*instructionStream << "extern ";

		*instructionStream << genType(global->varType->typeID);

		for (int i = 0; i < global->declarators.size; i++)
		{
			AST::VariableDeclarator* declarator = global->declarators[i];
			*instructionStream << " " << declarator->variable->mangledName;
			if (declarator->value)
				*instructionStream << "=" << genExpression(declarator->value);
			if (i < global->declarators.size - 1)
				*instructionStream << ",";
		}
		*instructionStream << ";" << std::endl;

		instructionStream = parentStream;

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

		builtinDefinitionStream << "int printf(char* fmt, ...);\n";

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
		if (functionDeclarations.seekg(0, std::ios::end).tellg() > 0)
			fileStream << functionDeclarations.str() << std::endl << std::endl;
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

	tcc_add_include_path(tcc, "D:\\Repositories\\tcc\\include\\");
	tcc_add_library_path(tcc, "D:\\Repositories\\tcc\\lib\\");
	//tcc_add_include_path(tcc, "C:\\Users\\faris\\Documents\\Dev\\tcc\\include\\");
	//tcc_add_library_path(tcc, "C:\\Users\\faris\\Documents\\Dev\\tcc\\lib\\");
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
