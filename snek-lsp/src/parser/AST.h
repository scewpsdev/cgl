#pragma once

#include "Lexer.h"
#include "utils/Arena.h"
#include "utils/StringView.h"

#include <stdint.h>


enum NodeType : uint8_t
{
	NODE_NULL = 0,

	NODE_PRIMITIVE_TYPE,
	NODE_NAMED_TYPE,
	NODE_STRUCT_TYPE,
	NODE_UNION_TYPE,
	NODE_POINTER_TYPE,
	NODE_OPTIONAL_TYPE,
	NODE_FUNCTION_TYPE,
	NODE_TUPLE_TYPE,
	NODE_ARRAY_TYPE,

	NODE_INT_LITERAL,
	NODE_FLOAT_LITERAL,
	NODE_STRING_LITERAL,
	NODE_CHAR_LITERAL,
	NODE_TRUE,
	NODE_FALSE,
	NODE_NULL_LITERAL,
	NODE_IDENTIFIER,
	NODE_COMPOUND_EXPRESSION,
	NODE_EXPRESSION_LIST,
	NODE_BINARY_OPERATOR,
	NODE_CAST,
	NODE_PREFIX_OPERATOR,
	NODE_POSTFIX_OPERATOR,
	NODE_FUNCTION_CALL,
	NODE_ARRAY_SUBSCRIPT,
	NODE_MEMBER_ACCESS,
	NODE_TERNARY_CONDITION,

	NODE_BLOCK_STATEMENT,
	NODE_IF,
	NODE_WHILE,
	NODE_FOR,
	NODE_RETURN,
	NODE_BREAK,
	NODE_CONTINUE,
	NODE_DEFER,
	NODE_VARIABLE_DECLARATION,
	NODE_ASSIGNMENT,
	NODE_EXPRESSION_STATEMENT,

	NODE_FIELD,
	NODE_PARAMETER,

	NODE_STRUCT,
	NODE_ENUM,
	NODE_UNION,
	NODE_TYPEDEF,
	NODE_FUNCTION,
	NODE_GLOBAL_VARIABLE,
	NODE_MACRO,
	NODE_IMPORT,

	NODE_COUNT
};

enum StorageSpecifier : uint8_t
{
	STORAGE_NULL = 0,

	STORAGE_CONSTANT = 1 << 0,
	STORAGE_EXTERN = 1 << 1,
	STORAGE_DLLEXPORT = 1 << 2,
	STORAGE_DLLIMPORT = 1 << 3,
	STORAGE_PRIVATE = 1 << 4,
	STORAGE_PACKED = 1 << 5,
	STORAGE_NOMANGLE = 1 << 6,
};

struct NodeBase
{
	uint8_t type;
	int start, end;
};

enum TypeKind : uint8_t
{
	TYPE_NULL = 0,

	TYPE_VOID,

	TYPE_INT_START,
	TYPE_INT8,
	TYPE_INT16,
	TYPE_INT32,
	TYPE_INT64,
	TYPE_UINT8,
	TYPE_UINT16,
	TYPE_UINT32,
	TYPE_UINT64,
	TYPE_INT_END,

	TYPE_FLOAT_BEGIN,
	TYPE_FLOAT,
	TYPE_DOUBLE,
	TYPE_FLOAT_END,

	TYPE_BOOL,
	TYPE_ANY,
	TYPE_STRING,
	TYPE_STRUCT,
	TYPE_UNION,
	TYPE_POINTER,
	TYPE_OPTIONAL,
	TYPE_FUNCTION,
	TYPE_TUPLE,
	TYPE_ARRAY,
};


struct Expression;

struct Type : NodeBase
{
	uint8_t typeKind;
};

struct NamedType : Type
{
	StringView name;
};

struct VariableDeclarator
{
	StringView name;
	Expression* value;
};

struct Field : NodeBase
{
	Type* type;
	VariableDeclarator* declarators;
	int numDeclarators;
};

struct StructType : Type
{
	Field** fields;
	int numFields;
};

struct UnionType : Type
{
	Field** fields;
	int numFields;
};

struct PointerType : Type
{
	Type* elementType;
};

struct OptionalType : Type
{
	Type* elementType;
};

struct Parameter : NodeBase
{
	Type* type;
	StringView name;
	bool variadic;
};

struct FunctionType : Type
{
	Parameter** params;
	int numParams;
	Type* returnType;
};

struct TupleType : Type
{
	Type** elementTypes;
	int numElementTypes;
};

struct ArrayType : Type
{
	Type* elementType;
	Expression* size;
};


struct Expression : NodeBase
{
};

struct IntLiteral : Expression
{
	StringView value;
};

struct FloatLiteral : Expression
{
	StringView value;
};

struct StringLiteral : Expression
{
	StringView value;
};

struct CharLiteral : Expression
{
	StringView value;
};

struct Identifier : Expression
{
	StringView name;
};

struct CompoundExpression : Expression
{
	Expression* value;
};

struct ExpressionList : Expression
{
	Expression** values;
	int numValues;
};

enum OperatorType : uint8_t
{
	OPERATOR_NULL = 0,

	OPERATOR_INCREMENT_POSTFIX,
	OPERATOR_DECREMENT_POSTFIX,
	OPERATOR_FUNCTION_CALL,
	OPERATOR_ARRAY_SUBSCRIPT,
	OPERATOR_MEMBER_ACCESS,

	OPERATOR_INCREMENT_PREFIX,
	OPERATOR_DECREMENT_PREFIX,
	OPERATOR_PLUS_PREFIX,
	OPERATOR_MINUS_PREFIX,
	OPERATOR_LOGICAL_NOT,
	OPERATOR_BITWISE_NOT,
	OPERATOR_DEREFERENCE,
	OPERATOR_ADDRESS,

	OPERATOR_MULTIPLY,
	OPERATOR_DIVIDE,
	OPERATOR_MODULO,

	OPERATOR_ADD,
	OPERATOR_SUBTRACT,

	OPERATOR_BITSHIFT_LEFT,
	OPERATOR_BITSHIFT_RIGHT,

	OPERATOR_CAST,

	OPERATOR_LESS,
	OPERATOR_LESS_EQUALS,
	OPERATOR_GREATER,
	OPERATOR_GREATER_EQUALS,

	OPERATOR_EQUALS,
	OPERATOR_NOT_EQUALS,

	OPERATOR_BITWISE_AND,

	OPERATOR_BITWISE_XOR,

	OPERATOR_BITWISE_OR,

	OPERATOR_LOGICAL_AND,

	OPERATOR_LOGICAL_OR,

	OPERATOR_TERNARY_CONDITION,

	OPERATOR_ASSIGN,
	OPERATOR_ADD_ASSIGN,
	OPERATOR_SUBTRACT_ASSIGN,
	OPERATOR_MULTIPLY_ASSIGN,
	OPERATOR_DIVIDE_ASSIGN,
	OPERATOR_MODULO_ASSIGN,
	OPERATOR_BITSHIFT_LEFT_ASSIGN,
	OPERATOR_BITSHIFT_RIGHT_ASSIGN,
	OPERATOR_BITWISE_AND_ASSIGN,
	OPERATOR_BITWISE_XOR_ASSIGN,
	OPERATOR_BITWISE_OR_ASSIGN,
	OPERATOR_LOGICAL_AND_ASSIGN,
	OPERATOR_LOGICAL_OR_ASSIGN,

	OPERATOR_COUNT
};

struct BinaryOperator : Expression
{
	uint8_t op;
	Expression* left, * right;
};

struct Cast : Expression
{
	Expression* expression;
	Type* targetType;
};

struct PrefixOperator : Expression
{
	uint8_t op;
	Expression* expression;
};

struct PostfixOperator : Expression
{
	uint8_t op;
	Expression* expression;
};

struct FunctionCall : Expression
{
	Expression* expression;
	Expression** args;
	int numArgs;
};

struct ArraySubscript : Expression
{
	Expression* expression;
	Expression** args;
	int numArgs;
};

struct MemberAccess : Expression
{
	Expression* expression;
	StringView name;
	int64_t index;
};

struct TernaryCondition : Expression
{
	Expression* condition;
	Expression* then, * else_;
};


struct Statement : NodeBase
{
};

struct BlockStatement : Statement
{
	Statement** statements;
	int numStatements;
};

struct If : Statement
{
	Expression* condition;
	Statement* then, * else_;
};

struct While : Statement
{
	Expression* condition;
	Statement* then;
};

struct For : Statement
{
	StringView iteratorName;
	Expression* startValue;
	OperatorType compareType;
	Expression* compareValue;
	Statement* body;
};

struct Return : Statement
{
	Expression* value;
};

struct Defer : Statement
{
	Statement* body;
};

struct VariableDeclaration : Statement
{
	Type* type;
	uint32_t storage;
	VariableDeclarator* declarators;
	int numDeclarators;
};

struct Assignment : Statement
{
	OperatorType op;
	Expression* expression;
	Expression* value;
};

struct ExpressionStatement : Statement
{
	Expression* expression;
};


struct Struct : NodeBase
{
	StringView name;
	uint32_t storage;

	Field** fields;
	int numFields;
};

struct EnumValue
{
	StringView name;
	Expression* value;
};

struct Enum : NodeBase
{
	StringView name;
	uint32_t storage;

	EnumValue* values;
	int numValues;
};

struct Union : NodeBase
{
	StringView name;
	uint32_t storage;

	Field** fields;
	int numFields;
};

struct Typedef : NodeBase
{
	StringView name;
	uint32_t storage;
	Type* value;
};

struct Function : NodeBase
{
	StringView name;
	uint32_t storage;

	Parameter** params;
	int numParams;
	Type* returnType;

	Statement** statements;
	int numStatements;
	Expression* value;
};

struct GlobalVariable : NodeBase
{
	Type* type;
	uint32_t storage;
	VariableDeclarator* declarators;
	int numDeclarators;
};

struct Macro : NodeBase
{
	StringView name;
	uint32_t storage;
};

struct Import : NodeBase
{
	StringView name;
};

struct Node
{
	union {
		struct {
			uint8_t type;
			int start, end;
		};

		// types

		Type primitiveType;
		NamedType namedType;
		StructType structType;
		UnionType unionType;
		PointerType pointerType;
		OptionalType optionalType;
		FunctionType functionType;
		TupleType tupleType;
		ArrayType arrayType;

		IntLiteral intLiteral;
		FloatLiteral floatLiteral;
		StringLiteral stringLiteral;
		CharLiteral charLiteral;
		Identifier identifier;
		CompoundExpression compoundExpression;
		ExpressionList expressionList;
		BinaryOperator binaryOperator;
		PrefixOperator prefixOperator;
		PostfixOperator postfixOperator;
		FunctionCall functionCall;
		ArraySubscript arraySubscript;
		MemberAccess memberAccess;
		Cast cast;

		BlockStatement blockStatement;
		If if_;
		While while_;
		For for_;
		Return return_;
		Defer defer;
		VariableDeclaration variableDeclaration;
		Assignment assignment;
		ExpressionStatement expressionStatement;

		Field field;
		Parameter parameter;

		// declarations

		Struct struct_;
		Enum enum_;
		Union union_;
		Typedef typedef_;
		Function function;
		GlobalVariable globalVariable;
		Macro macro;
		Import import;
	};
};

struct SymbolEntry
{
	uint32_t key;
	Node* value;
};

struct SymbolTable
{
	SymbolEntry* slots;
	int capacity;
	int count;

	Arena* arena;
};

struct Scope
{
	Scope* parent;
	SymbolTable symbols;
};

struct AST
{
	Node** declarations;
	int numDeclarations;

	Scope* globalScope;
};

typedef void(*ASTVisitor_t)(Node* node, void* userPtr);


void initAST(AST* ast);
void destroyAST(AST* ast);

void initNode(Node* node, uint8_t type, int start);
void initType(Type* type, uint8_t nodeType, uint8_t typeKind, int start);

void initSymbolTable(SymbolTable* symbols, int capacity, Arena* arena);
bool insertSymbol(SymbolTable* symbols, StringView identifier, Node* node);
Node* lookupSymbol(SymbolTable* symbols, StringView identifier);

void initScope(Scope* scope, Scope* parent, bool isGlobal, Arena* arena);

void traverseAST(AST* ast, ASTVisitor_t visitor, void* userPtr);
