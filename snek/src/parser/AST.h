#pragma once

#include "Lexer.h"
#include "TypeKind.h"
#include "utils/Arena.h"
#include "utils/StringView.h"
#include "codegen/Value.h"

#include <stdint.h>


enum NodeType : uint8_t
{
	NODE_NULL = 0,

	NODE_ERROR,
	NODE_ERROR_TYPE,
	NODE_ERROR_EXPRESSION,
	NODE_ERROR_STATEMENT,

	NODE_TYPE_START,
	NODE_PRIMITIVE_TYPE,
	NODE_NAMED_TYPE,
	NODE_STRUCT_TYPE,
	NODE_UNION_TYPE,
	NODE_POINTER_TYPE,
	NODE_OPTIONAL_TYPE,
	NODE_FUNCTION_TYPE,
	NODE_TUPLE_TYPE,
	NODE_ARRAY_TYPE,
	NODE_TYPE_END,

	NODE_EXPRESSION_START,
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
	NODE_UNARY_OPERATOR,
	NODE_FUNCTION_CALL,
	NODE_ARRAY_SUBSCRIPT,
	NODE_MEMBER_ACCESS,
	NODE_TERNARY_CONDITION,
	NODE_CAST,
	NODE_EXPRESSION_END,

	NODE_STATEMENT_START,
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
	NODE_STATEMENT_END,

	NODE_FIELD,
	NODE_PARAMETER,
	NODE_ENUM_VALUE,

	NODE_STRUCT,
	NODE_UNION,
	NODE_ENUM,
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
	NodeType type;
	int start, end;
};


typedef uint64_t FileHandle;

struct Node;
struct Type;
struct Expression;
struct Scope;
struct Symbol;
struct FunctionOverload;

struct TypeNode : NodeBase
{
	TypeKind typeKind;

	Type* inferredType;
};

struct ErrorType : TypeNode
{
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
	TypeNode* variableType;
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
	TypeNode* paramType;
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

struct ErrorExpression : Expression
{
};

struct IntLiteral : Expression
{
	StringView value;
	uint64_t intValue;
	bool negative;
	int base;
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

struct SymbolHandle
{
	FileHandle file;
	uint32_t symbol;
};

struct Identifier : Expression
{
	StringView name;

	Symbol* resolvedSymbol;
	SymbolHandle resolvedSymbolHandle;
	int functionOverloadID;
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
	OperatorType op;
	Expression* left, * right;
};

struct UnaryOperator : Expression
{
	OperatorType op;
	Expression* operand;
};

struct FunctionCall : Expression
{
	Expression* expression;
	Expression** args;
	int numArgs;
};

struct ArraySubscript : Expression
{
	Expression* operand;
	Expression** args;
	int numArgs;
};

struct MemberAccess : Expression
{
	Expression* operand;
	StringView name;
	int64_t index;

	Symbol* resolvedSymbol;
	SymbolHandle resolvedSymbolHandle;
	int functionOverloadID;
	Value memberFunctionInstance;
};

struct TernaryCondition : Expression
{
	Expression* condition;
	Expression* then, * else_;
};

struct Cast : Expression
{
	Expression* expression;
	Expression* expression2;
	TypeNode* targetType;
	bool implicit;
};


struct Statement : NodeBase
{
};

struct ErrorStatement : Statement
{
};

struct BlockStatement : Statement
{
	Statement** statements;
	int numStatements;

	Scope* scope;
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
	bool equals;
	Expression* compareValue;
	Statement* body;

	Scope* scope;
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

	Type* structType;
};

struct Union : NodeBase
{
	StringView name;
	uint32_t storage;

	Field** fields;
	int numFields;

	Type* unionType;
};

struct EnumValue : NodeBase
{
	StringView name;
	Expression* value;

	int64_t intValue;
};

struct Enum : NodeBase
{
	StringView name;
	uint32_t storage;

	TypeNode* valueType;
	EnumValue** values;
	int numValues;

	Type* enumType;
};

struct Typedef : NodeBase
{
	StringView name;
	uint32_t storage;
	TypeNode* value;

	Type* aliasType;
};

struct Function : NodeBase
{
	StringView name;
	uint32_t storage;

	Parameter** params;
	int numParams;
	TypeNode* returnType;

	bool hasBody;
	Statement** statements;
	int numStatements;
	Expression* value;

	Type* functionType;
	StringView mangledName;

	Scope* scope;
};

struct GlobalVariable : NodeBase
{
	TypeNode* variableType;
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
	StringView* path;
	int pathCount;

	FileHandle fileHandle;
};

struct Node
{
	union {
		struct {
			NodeType type;
			int start, end;
		};

		// types

		ErrorType errorType;
		TypeNode primitiveType;
		NamedType namedType;
		StructType structType;
		UnionType unionType;
		PointerType pointerType;
		OptionalType optionalType;
		FunctionType functionType;
		TupleType tupleType;
		ArrayType arrayType;

		ErrorExpression errorExpression;
		IntLiteral intLiteral;
		FloatLiteral floatLiteral;
		StringLiteral stringLiteral;
		CharLiteral charLiteral;
		Identifier identifier;
		CompoundExpression compoundExpression;
		ExpressionList expressionList;
		BinaryOperator binaryOperator;
		UnaryOperator unaryOperator;
		FunctionCall functionCall;
		ArraySubscript arraySubscript;
		MemberAccess memberAccess;
		TernaryCondition ternaryCondition;
		Cast cast;

		ErrorStatement errorStatement;
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
		EnumValue enumValue;

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

struct FunctionOverload
{
	Function* declaration;
};

struct FunctionSet
{
	FunctionOverload* overloads;
	int count;
	int capacity;
};

struct Symbol
{
	uint32_t key;
	StringView name;
	SymbolType type;
	FileHandle file;

	union
	{
		Node* declaration;
		FunctionSet functionSet;
	};
};

struct SymbolTable
{
	Symbol* slots;
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
	Import** imports;
	int numImports;

	Scope* globalScope;
};

typedef bool(*ASTVisitor_t)(Node* nodeRef, Scope* scope, void* userPtr);


void initNode(Node* node, NodeType type, int start);
void initType(TypeNode* type, NodeType nodeType, TypeKind typeKind, int start);

void initSymbolTable(SymbolTable* symbols, int capacity, Arena* arena);
bool insertSymbol(SymbolTable* symbols, StringView identifier, SymbolType type, Node* declaration, FileHandle file);
Symbol* lookupSymbol(SymbolTable* symbols, StringView identifier);
Symbol* lookupSymbol(SymbolTable* symbols, uint32_t h);

Symbol* getIdentifierSymbol(Identifier* identifier);
Symbol* getMemberAccessSymbol(MemberAccess* member);

int getFieldIndex(StringView name, int numFields, StringView* fieldNames);
int getEnumValue(StringView name, int numValues, EnumValue** values);
VariableDeclarator* getDeclarator(VariableDeclaration* variable, StringView name);
VariableDeclarator* getDeclarator(GlobalVariable* variable, StringView name);

bool isConstant(Expression* expression);
bool isLValue(Expression* expression);
bool constantFold(Expression* expression, int64_t* value);

void resetExpression(Expression** ref);
void resetAST(AST* ast);

void traverseAST(AST* ast, ASTVisitor_t visitor, void* userPtr);

void getLocalPathFromModuleName(char* path, StringView* parts, int numParts);
