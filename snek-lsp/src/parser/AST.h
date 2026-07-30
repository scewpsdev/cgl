#pragma once

#include "Lexer.h"
#include "TypeKind.h"
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


struct Node;
struct Expression;
struct Type;

struct TypeNode : NodeBase
{
	uint8_t typeKind;

	Type* inferredType;
};

struct NamedType : TypeNode
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
	TypeNode* type;
	VariableDeclarator* declarators;
	int numDeclarators;
};

struct StructType : TypeNode
{
	Field** fields;
	int numFields;
};

struct UnionType : TypeNode
{
	Field** fields;
	int numFields;
};

struct PointerType : TypeNode
{
	TypeNode* elementType;
};

struct OptionalType : TypeNode
{
	TypeNode* elementType;
};

struct Parameter : NodeBase
{
	TypeNode* type;
	StringView name;
	bool variadic;
};

struct FunctionType : TypeNode
{
	Parameter** params;
	int numParams;
	TypeNode* returnType;
};

struct TupleType : TypeNode
{
	TypeNode** elementTypes;
	int numElementTypes;
};

struct ArrayType : TypeNode
{
	TypeNode* elementType;
	Expression* size;
};


struct Expression : NodeBase
{
	Type* inferredType;
};

struct IntLiteral : Expression
{
	StringView value;
	uint64_t intValue;
	bool negative;
};

struct FloatLiteral : Expression
{
	StringView value;
	double floatValue;
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

	Node* resolvedSymbol;
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
	TypeNode* targetType;
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
	TypeNode* variableType;
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
	TypeNode* value;
};

struct Function : NodeBase
{
	StringView name;
	uint32_t storage;

	Parameter** params;
	int numParams;
	TypeNode* returnType;

	Statement** statements;
	int numStatements;
	Expression* value;

	Type* functionType;
};

struct GlobalVariable : NodeBase
{
	TypeNode* type;
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

		TypeNode primitiveType;
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

enum SymbolType : uint8_t
{
	SYMBOL_NULL = 0,

	SYMBOL_VARIABLE,
	SYMBOL_TYPE,
	SYMBOL_FUNCTION_SET,
	SYMBOL_MACRO,
};

struct SymbolEntry
{
	uint32_t key;
	uint8_t type;

	union
	{
		Node* declaration;
		struct {
			Node** overloads;
			int count;
			int capacity;
		} functionSet;
	};
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

	Struct** structs;
	int numStructs;
	Enum** enums;
	int numEnums;
	Union** unions;
	int numUnions;
	Typedef** typedefs;
	int numTypedefs;
	Function** functions;
	int numFunctions;
	Macro** macros;
	int numMacros;
	GlobalVariable** globalVariables;
	int numGlobalVariables;
};

typedef void(*ASTVisitor_t)(Node* node, void* userPtr);


void initAST(AST* ast);
void destroyAST(AST* ast);

void initNode(Node* node, uint8_t type, int start);
void initType(TypeNode* type, uint8_t nodeType, uint8_t typeKind, int start);

void initSymbolTable(SymbolTable* symbols, int capacity, Arena* arena);
bool insertSymbol(SymbolTable* symbols, StringView identifier, SymbolType type, Node* declaration);
SymbolEntry* lookupSymbol(SymbolTable* symbols, StringView identifier);

void initScope(Scope* scope, Scope* parent, bool isGlobal, Arena* arena);

void traverseAST(AST* ast, ASTVisitor_t visitor, void* userPtr);
