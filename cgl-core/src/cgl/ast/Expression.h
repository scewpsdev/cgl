#pragma once

#include "Element.h"
#include "Type.h"

#include "cgl/semantics/Type.h"


struct Variable;

namespace AST
{
	struct Module;
	struct Function;
	struct EnumValue;

	struct StructField;
	struct ClassField;


	enum class ExpressionType : uint8_t
	{
		Null = 0,

		IntegerLiteral,
		FloatingPointLiteral,
		BooleanLiteral,
		CharacterLiteral,
		NullLiteral,
		StringLiteral,
		InitializerList,
		Identifier,
		Compound,
		Tuple,

		FunctionCall,
		SubscriptOperator,
		DotOperator,
		Typecast,
		Sizeof,
		Malloc,

		UnaryOperator,
		BinaryOperator,
		TernaryOperator,
	};

	enum class UnaryOperatorType : uint8_t
	{
		Null = 0,

		Not,
		Negate,
		Reference,
		Dereference,

		Increment,
		Decrement,
	};

	enum class BinaryOperatorType : uint8_t
	{
		Null = 0,

		Add,
		Subtract,
		Multiply,
		Divide,
		Modulo,

		Equals,
		DoesNotEqual,
		LessThan,
		GreaterThan,
		LessThanEquals,
		GreaterThanEquals,
		LogicalAnd,
		LogicalOr,
		BitwiseAnd,
		BitwiseOr,
		BitwiseXor,
		BitshiftLeft,
		BitshiftRight,

		Assignment,
		PlusEquals,
		MinusEquals,
		TimesEquals,
		DividedByEquals,
		ModuloEquals,
		BitwiseAndEquals,
		BitwiseOrEquals,
		BitwiseXorEquals,
		BitshiftLeftEquals,
		BitshiftRightEquals,
		ReferenceAssignment,

		NullCoalescing,
		Ternary, // Technically not a binary operator, but it makes sense to have it here
	};

	enum class BuiltinTypeProperty : uint8_t
	{
		Null,

		Int8Min,
		Int8Max,
		Int16Min,
		Int16Max,
		Int32Min,
		Int32Max,
		Int64Min,
		Int64Max,

		UInt8Min,
		UInt8Max,
		UInt16Min,
		UInt16Max,
		UInt32Min,
		UInt32Max,
		UInt64Min,
		UInt64Max,
	};

	struct Expression : Element
	{
		ExpressionType type;

		TypeID valueType;
		bool lvalue;


		Expression(File* file, const SourceLocation& location, ExpressionType type);

		virtual Element* copy() override = 0;
		virtual bool isConstant();
		virtual bool isLiteral();
	};

	struct IntegerLiteral : Expression
	{
		int64_t value = 0;


		IntegerLiteral(File* file, const SourceLocation& location, int64_t value);

		virtual Element* copy() override;
		virtual bool isConstant() override;
		virtual bool isLiteral() override;
	};

	struct FloatingPointLiteral : Expression
	{
		double value = 0.0;
		char valueStr[32] = {};
		bool isDouble = false;


		FloatingPointLiteral(File* file, const SourceLocation& location, double value, const char* valueStr, bool isDouble);

		virtual Element* copy() override;
		virtual bool isConstant() override;
		virtual bool isLiteral() override;
	};

	struct BooleanLiteral : Expression
	{
		bool value;


		BooleanLiteral(File* file, const SourceLocation& location, bool value);

		virtual Element* copy() override;
		virtual bool isConstant() override;
		virtual bool isLiteral() override;
	};

	struct CharacterLiteral : Expression
	{
		uint32_t value;


		CharacterLiteral(File* file, const SourceLocation& location, uint32_t value);

		virtual Element* copy() override;
		virtual bool isConstant() override;
		virtual bool isLiteral() override;
	};

	struct NullLiteral : Expression
	{
		NullLiteral(File* file, const SourceLocation& location);

		virtual Element* copy() override;
		virtual bool isConstant() override;
		virtual bool isLiteral() override;
	};

	struct StringLiteral : Expression
	{
		char* value;
		int length;


		StringLiteral(File* file, const SourceLocation& location, char* value, int length);
		virtual ~StringLiteral();

		virtual Element* copy() override;
		virtual bool isConstant() override;
		virtual bool isLiteral() override;
	};

	struct InitializerList : Expression
	{
		Type* initializerTypeAST;
		List<Expression*> values;

		TypeID initializerType = nullptr;


		InitializerList(File* file, const SourceLocation& location, Type* initializerTypeAST, const List<Expression*>& values);
		virtual ~InitializerList();

		virtual Element* copy() override;
		virtual bool isConstant() override;
		virtual bool isLiteral() override;
	};

	struct Identifier : Expression
	{
		char* name;

		Variable* variable = nullptr;
		Expression* exprdefValue = nullptr;
		EnumValue* enumValue = nullptr;

		List<Function*> functions;


		Identifier(File* file, const SourceLocation& location, char* name);
		virtual ~Identifier();

		virtual Element* copy() override;
		virtual bool isConstant() override;
	};

	struct CompoundExpression : Expression
	{
		Expression* value;


		CompoundExpression(File* file, const SourceLocation& location, Expression* value);
		virtual ~CompoundExpression();

		virtual Element* copy() override;
		virtual bool isConstant() override;
	};

	struct TupleExpression : Expression
	{
		List<Expression*> values;


		TupleExpression(File* file, const SourceLocation& location, const List<Expression*>& values);
		virtual ~TupleExpression();

		virtual Element* copy() override;
		virtual bool isConstant() override;
	};

	struct FunctionCall : Expression
	{
		Expression* callee;
		List<Expression*> arguments;

		bool hasGenericArgs;
		List<Type*> genericArgsAST;
		List<TypeID> genericArgs;

		Function* function = nullptr;
		bool isMethodCall = false;
		Expression* methodInstance = nullptr;

		bool isCast = false;
		TypeID castDstType = nullptr;


		FunctionCall(File* file, const SourceLocation& location, Expression* callee, const List<Expression*>& arguments, bool hasGenericArgs, const List<Type*>& genericArgs);
		virtual ~FunctionCall();

		virtual Element* copy() override;
	};

	struct SubscriptOperator : Expression
	{
		Expression* operand;
		List<Expression*> arguments;

		Function* operatorOverload = nullptr;


		SubscriptOperator(File* file, const SourceLocation& location, Expression* operand, const List<Expression*>& arguments);
		virtual ~SubscriptOperator();

		virtual Element* copy() override;
	};

	struct DotOperator : Expression
	{
		Expression* operand;
		char* name = nullptr;
		int fieldIndex = -1;

		Module* module = nullptr;
		Variable* namespacedVariable = nullptr;

		List<Function*> namespacedFunctions;

		BuiltinTypeProperty builtinTypeProperty = BuiltinTypeProperty::Null;

		// structs
		StructField* structField = nullptr;
		Function* classMethod = nullptr;
		ValueHandle methodInstance = nullptr;

		// classes
		ClassField* classField = nullptr;

		List<Function*> methods;

		// enums
		EnumValue* enumValue = nullptr;


		DotOperator(File* file, const SourceLocation& location, Expression* operand, int index);
		DotOperator(File* file, const SourceLocation& location, Expression* operand, char* name);
		virtual ~DotOperator();

		virtual Element* copy() override;
	};

	struct Typecast : Expression
	{
		Expression* value;
		Type* dstType;


		Typecast(File* file, const SourceLocation& location, Expression* value, Type* dstType);
		virtual ~Typecast();

		virtual Element* copy() override;
	};

	struct Sizeof : Expression
	{
		Type* dstType;


		Sizeof(File* file, const SourceLocation& location, Type* dstType);
		virtual ~Sizeof();

		virtual Element* copy() override;
	};

	struct Malloc : Expression
	{
		Type* dstType;
		Expression* count;
		bool malloc;

		bool hasArguments;
		List<Expression*> arguments;


		Malloc(File* file, const SourceLocation& location, Type* dstType, Expression* count, bool malloc, bool hasArguments, const List<Expression*>& arguments);
		virtual ~Malloc();

		virtual Element* copy() override;
	};

	struct UnaryOperator : Expression
	{
		Expression* operand;
		UnaryOperatorType operatorType;
		bool position;


		UnaryOperator(File* file, const SourceLocation& location, Expression* operand, UnaryOperatorType operatorType, bool position);
		virtual ~UnaryOperator();

		virtual Element* copy() override;
		virtual bool isConstant() override;
	};

	struct BinaryOperator : Expression
	{
		Expression* left, * right;
		BinaryOperatorType operatorType;

		TypeID opAssignResultingType;


		BinaryOperator(File* file, const SourceLocation& location, Expression* left, Expression* right, BinaryOperatorType operatorType);
		virtual ~BinaryOperator();

		virtual Element* copy() override;
		virtual bool isConstant() override;
	};

	struct TernaryOperator : Expression
	{
		Expression* condition, * thenValue, * elseValue;


		TernaryOperator(File* file, const SourceLocation& location, Expression* condition, Expression* thenValue, Expression* elseValue);
		virtual ~TernaryOperator();

		virtual Element* copy() override;
		virtual bool isConstant() override;
	};
}
