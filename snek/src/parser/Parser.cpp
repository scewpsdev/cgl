#include "Parser.h"

#include "File.h"
#include "Lexer.h"
#include "AST.h"
#include "Diagnostics.h"

#include "utils/Log.h"
#include "utils/Arena.h"
#include "utils/List.h"

#include <stdarg.h>


void initParser(Parser* parser, const char* filename, const char* src, int length, Arena* arena, ScratchBuffer* scratch, Diagnostics* diagnostics)
{
	*parser = {};

	initLexer(&parser->lexer, filename, src, length, arena, diagnostics);

	parser->arena = arena;
	parser->scratch = scratch;
	parser->diagnostics = diagnostics;
	parser->lookaheadCount = 0;
	parser->lastTokenEnd = 0;
}

void destroyParser(Parser* parser)
{
	destroyLexer(&parser->lexer);
}

SourceLocation getSourceLocation(Parser* parser)
{
	return getSourceLocation(&parser->lexer, parser->cursor);
}

SourceLocation getSourceLocation(Parser* parser, Token token)
{
	return getSourceLocation(&parser->lexer, token.offset);
}

void getSourceLocation(Parser* parser, Token token, SourceLocation* start, SourceLocation* end)
{
	*start = getSourceLocation(&parser->lexer, token.offset);
	*end = getSourceLocation(&parser->lexer, token.offset + token.length);
}

static void getTokenRange(Token token, int* start, int* end)
{
	*start = token.offset;
	*end = token.offset + token.length;
}

static StringView getTokenString(Token token, Parser* parser)
{
	return getTokenString(token, parser->lexer.src);
}

static StringView getRangedString(int start, int end, Parser* parser)
{
	return CreateString(parser->lexer.src + start, end - start);
}

static const char* getTokenTypeName(int type)
{
	static const char* tokenNames[TOKEN_COUNT];

	tokenNames[TOKEN_STRING_LITERAL] = "string literal";
	tokenNames[TOKEN_STRING_LITERAL_MULTILINE] = "multiline string literal";
	tokenNames[TOKEN_CHAR_LITERAL] = "character literal";
	tokenNames[TOKEN_FLOAT_LITERAL] = "floating point literal";
	tokenNames[TOKEN_INT_LITERAL] = "integer literal";
	tokenNames[TOKEN_MODULE] = "'module'";
	tokenNames[TOKEN_NAMESPACE] = "'namespace'";
	tokenNames[TOKEN_IMPORT] = "'import'";
	tokenNames[TOKEN_FUNCTION] = "'func'";
	tokenNames[TOKEN_STRUCT] = "'struct'";
	tokenNames[TOKEN_CLASS] = "'class'";
	tokenNames[TOKEN_TYPEDEF] = "'type'";
	tokenNames[TOKEN_MACRO] = "'macro'";
	tokenNames[TOKEN_ENUM] = "'enum'";
	tokenNames[TOKEN_UNION] = "'union'";
	tokenNames[TOKEN_PUBLIC] = "'public'";
	tokenNames[TOKEN_PRIVATE] = "'private'";
	tokenNames[TOKEN_STATIC] = "'static'";
	tokenNames[TOKEN_CONSTANT] = "'const'";
	tokenNames[TOKEN_EXTERN] = "'extern'";
	tokenNames[TOKEN_DLLEXPORT] = "'dllexport'";
	tokenNames[TOKEN_DLLIMPORT] = "'dllimport'";
	tokenNames[TOKEN_PACKED] = "'packed'";
	tokenNames[TOKEN_NOMANGLE] = "'nomangle'";
	tokenNames[TOKEN_IF] = "'if'";
	tokenNames[TOKEN_ELSE] = "'else'";
	tokenNames[TOKEN_FOR] = "'for'";
	tokenNames[TOKEN_WHILE] = "'while'";
	tokenNames[TOKEN_RETURN] = "'return'";
	tokenNames[TOKEN_BREAK] = "'break'";
	tokenNames[TOKEN_CONTINUE] = "'continue'";
	tokenNames[TOKEN_DEFER] = "'defer'";
	tokenNames[TOKEN_ASSERT] = "'assert'";
	tokenNames[TOKEN_AS] = "'as'";
	tokenNames[TOKEN_SIZEOF] = "'sizeof'";
	tokenNames[TOKEN_ALIGNOF] = "'alignof'";
	tokenNames[TOKEN_TRUE] = "'true'";
	tokenNames[TOKEN_FALSE] = "'false'";
	tokenNames[TOKEN_NULL_KEYWORD] = "'null'";
	tokenNames[TOKEN_VOID] = "'void'";
	tokenNames[TOKEN_INT8] = "'char'";
	tokenNames[TOKEN_INT16] = "'short'";
	tokenNames[TOKEN_INT32] = "'int'";
	tokenNames[TOKEN_INT64] = "'long'";
	tokenNames[TOKEN_UINT8] = "'byte'";
	tokenNames[TOKEN_UINT16] = "'ushort'";
	tokenNames[TOKEN_UINT32] = "'uint'";
	tokenNames[TOKEN_UINT64] = "'ulong'";
	tokenNames[TOKEN_BOOL] = "'bool'";
	tokenNames[TOKEN_STRING] = "'string'";
	tokenNames[TOKEN_FLOAT32] = "'float'";
	tokenNames[TOKEN_FLOAT64] = "'double'";
	tokenNames[TOKEN_ANY] = "'any'";
	tokenNames[TOKEN_IDENTIFIER] = "identifier";

	return tokenNames[type];
}

static void error(Parser* parser, SourceLocation start, SourceLocation end, const char* fmt, ...)
{
	if (!parser->diagnostics) return;

	va_list args;
	va_start(args, fmt);

	int length = vsnprintf(nullptr, 0, fmt, args);
	char* msg = (char*)parser->arena->alloc(length + 1);
	vsnprintf(msg, length + 1, fmt, args);

	va_end(args);

	logMessage(parser->diagnostics, msg, start.line, start.col, end.line, end.col, DIAGNOSTICS_ERROR);
}

static void error(Parser* parser, SourceLocation location, const char* fmt, ...)
{
	if (!parser->diagnostics) return;

	va_list args;
	va_start(args, fmt);

	int length = vsnprintf(nullptr, 0, fmt, args);
	char* msg = (char*)parser->arena->alloc(length + 1);
	vsnprintf(msg, length + 1, fmt, args);

	va_end(args);

	logMessage(parser->diagnostics, msg, location.line, location.col, location.line, location.col + 1, DIAGNOSTICS_ERROR);
}

void error(Parser* parser, Node* node, const char* fmt, ...)
{
	if (!parser->diagnostics) return;

	va_list args;
	va_start(args, fmt);

	int length = vsnprintf(nullptr, 0, fmt, args);
	char* msg = (char*)parser->arena->alloc(length + 1);
	vsnprintf(msg, length + 1, fmt, args);

	va_end(args);

	SourceLocation start = getSourceLocation(&parser->lexer, node->start);
	SourceLocation end = getSourceLocation(&parser->lexer, node->end);

	logMessage(parser->diagnostics, msg, start.line, start.col, end.line, end.col, DIAGNOSTICS_ERROR);
}

static bool hasNext(Parser* parser)
{
	return parser->cursor < parser->lexer.length;
}

static Token nextToken(Parser* parser)
{
	if (parser->lookaheadCount > 0)
	{
		Token token = parser->lookahead[0];
		parser->lookahead[0] = parser->lookahead[1];
		parser->lookahead[1] = parser->lookahead[2];

		int state = parser->lookaheadState[0];
		parser->lookaheadState[0] = parser->lookaheadState[1];
		parser->lookaheadState[1] = parser->lookaheadState[2];

		parser->lookaheadCount--;
		parser->cursor = state;
		parser->lastTokenEnd = token.offset + token.length;
		return token;
	}

	Token token = nextToken(&parser->lexer);
	parser->cursor = parser->lexer.cursor;
	parser->lastTokenEnd = token.offset + token.length;
	return token;
}

static Token peekToken(Parser* parser, int offset = 0)
{
	SnekAssert(offset <= 3);
	while (parser->lookaheadCount <= offset)
	{
		parser->lookahead[parser->lookaheadCount] = nextToken(&parser->lexer);
		parser->lookaheadState[parser->lookaheadCount] = parser->lexer.cursor;
		parser->lookaheadCount++;
	}

	if (parser->lookahead[0].offset > parser->cursor)
		parser->cursor = parser->lookahead[0].offset;

	return parser->lookahead[offset];
}

static void rewind(Parser* parser, int cursor)
{
	parser->lexer.cursor = cursor;
	parser->cursor = cursor;
	parser->lookaheadCount = 0;
	parser->lastTokenEnd = cursor;
}

static bool expectToken(Parser* parser, int type, Token* outToken = nullptr)
{
	Token token = peekToken(parser);

	if (token.type == type)
	{
		if (outToken) *outToken = token;
		nextToken(parser);
		return true;
	}

	SourceLocation start, end;
	if (token.type)
	{
		getSourceLocation(parser, token, &start, &end);
	}
	else
	{
		start = getSourceLocation(parser);
		end = start;
	}

	if (type < TOKEN_FIRST)
		error(parser, start, end, "Expected token '%c'", (int)type);
	else
		error(parser, start, end, "Expected %s", getTokenTypeName(type));

	if (outToken) *outToken = {};

	return false;
}

static bool nextIs(Parser* parser, int type, Token* outToken = nullptr)
{
	Token token = peekToken(parser);
	if (outToken) *outToken = token;
	return token.type == type;
}

static bool nextIs(Parser* parser, int offset, int type, Token* outToken = nullptr)
{
	Token token = peekToken(parser, offset);
	if (outToken) *outToken = token;
	return token.type == type;
}

static bool nextIsKeyword(Parser* parser, Token* outToken = nullptr)
{
	Token token = peekToken(parser);
	if (outToken) *outToken = token;
	return token.type >= TOKEN_KEYWORD_BEGIN && token.type <= TOKEN_KEYWORD_END;
}

static void skipPastToken(Parser* parser, int type)
{
	Token token;
	while ((token = nextToken(parser)).type && token.type != type)
	{
	}
}

static void skipPastTokenNested(Parser* parser, int openType, int closeType)
{
	int level = 1;
	while (hasNext(parser) && level > 0)
	{
		Token token = nextToken(parser);
		if (token.type == openType)
			level++;
		else if (token.type == closeType)
			level--;
	}
}

static int64_t getConstantInt(StringView str)
{
	char* endPtr;
	int64_t value = strtoll(str.ptr, &endPtr, 10);
	SnekAssert(endPtr == str.ptr + str.length);
	return value;
}

static StorageSpecifier getStorageSpecifier(TokenType tokenType)
{
	switch (tokenType)
	{
	case TOKEN_CONSTANT: return STORAGE_CONSTANT;
	case TOKEN_EXTERN: return STORAGE_EXTERN;
	case TOKEN_DLLEXPORT: return STORAGE_DLLEXPORT;
	case TOKEN_DLLIMPORT: return STORAGE_DLLIMPORT;
	case TOKEN_PRIVATE: return STORAGE_PRIVATE;
	case TOKEN_PACKED: return STORAGE_PACKED;
	case TOKEN_NOMANGLE: return STORAGE_NOMANGLE;
	default: return STORAGE_NULL;
	}
}

template<typename T>
static T* copyFromScratchBuffer(Parser* parser, int mark, int* outCount)
{
	T* nodes = nullptr;

	int count = parser->scratch->count<T>(mark);
	if (count > 0)
	{
		nodes = parser->arena->alloc<T>(count);
		memcpy(nodes, parser->scratch->memory + mark, count * sizeof(T));
	}

	*outCount = count;

	return nodes;
}


TypeNode* parseType(Parser* parser);
Expression* parseExpression(Parser* parser);
Statement* parseStatement(Parser* parser);
Field* parseField(Parser* parser);
Parameter* parseParameter(Parser* parser);

static ErrorType* getErrorType(Parser* parser, int start)
{
	ErrorType* node = parser->arena->alloc<ErrorType>();
	initNode((Node*)node, NODE_ERROR_TYPE, start);
	node->end = start;
	return node;
}

static ErrorExpression* getErrorExpression(Parser* parser, int start)
{
	ErrorExpression* node = parser->arena->alloc<ErrorExpression>();
	initNode((Node*)node, NODE_ERROR_EXPRESSION, start);
	node->end = start;
	return node;
}

static ErrorStatement* getErrorStatement(Parser* parser, int start)
{
	ErrorStatement* node = parser->arena->alloc<ErrorStatement>();
	initNode((Node*)node, NODE_ERROR_STATEMENT, start);
	node->end = start;
	return node;
}

static Node* getErrorNode(Parser* parser, int start)
{
	Node* node = parser->arena->alloc<Node>();
	initNode(node, NODE_ERROR, start);
	node->end = start;
	return node;
}


static TypeNode* parseBasicType(Parser* parser)
{
	Token token = peekToken(parser);
	int start, end;
	getTokenRange(token, &start, &end);

	if (token.type == TOKEN_VOID)
	{
		nextToken(parser);

		TypeNode* type = parser->arena->alloc<TypeNode>();
		initType(type, NODE_PRIMITIVE_TYPE, TYPE_VOID, start);
		type->end = end;
		return type;
	}
	else if (token.type == TOKEN_INT8 || token.type == TOKEN_INT16 || token.type == TOKEN_INT32 || token.type == TOKEN_INT64)
	{
		nextToken(parser);

		TypeKind typeKind = (TypeKind)(TYPE_INT8 + (token.type - TOKEN_INT8));

		TypeNode* type = parser->arena->alloc<TypeNode>();
		initType(type, NODE_PRIMITIVE_TYPE, typeKind, start);
		type->end = end;
		return type;
	}
	else if (token.type == TOKEN_UINT8 || token.type == TOKEN_UINT16 || token.type == TOKEN_UINT32 || token.type == TOKEN_UINT64)
	{
		nextToken(parser);

		TypeKind typeKind = (TypeKind)(TYPE_UINT8 + (token.type - TOKEN_UINT8));

		TypeNode* type = parser->arena->alloc<TypeNode>();
		initType(type, NODE_PRIMITIVE_TYPE, typeKind, start);
		type->end = end;
		return type;
	}
	else if (token.type == TOKEN_FLOAT32 || token.type == TOKEN_FLOAT64)
	{
		nextToken(parser);

		TypeKind typeKind = token.type == TOKEN_FLOAT32 ? TYPE_FLOAT : TYPE_DOUBLE;

		TypeNode* type = parser->arena->alloc<TypeNode>();
		initType(type, NODE_PRIMITIVE_TYPE, typeKind, start);
		type->end = end;
		return type;
	}
	else if (token.type == TOKEN_BOOL)
	{
		nextToken(parser);

		TypeNode* type = parser->arena->alloc<TypeNode>();
		initType(type, NODE_PRIMITIVE_TYPE, TYPE_BOOL, start);
		type->end = end;
		return type;
	}
	else if (token.type == TOKEN_ANY)
	{
		nextToken(parser);

		TypeNode* type = parser->arena->alloc<TypeNode>();
		initType(type, NODE_PRIMITIVE_TYPE, TYPE_ANY, start);
		type->end = end;
		return type;
	}
	else if (token.type == TOKEN_STRING)
	{
		nextToken(parser);

		TypeNode* type = parser->arena->alloc<TypeNode>();
		initType(type, NODE_PRIMITIVE_TYPE, TYPE_STRING, start);
		type->end = end;
		return type;
	}
	else if (token.type == TOKEN_IDENTIFIER)
	{
		nextToken(parser);

		NamedType* type = parser->arena->alloc<NamedType>();
		initType((TypeNode*)type, NODE_NAMED_TYPE, TYPE_NULL, start);
		type->end = end;
		type->name = getTokenString(token, parser);
		return type;
	}
	else if (token.type == TOKEN_FUNCTION && peekToken(parser, 1).type == '(')
	{
		nextToken(parser);

		FunctionType* type = parser->arena->alloc<FunctionType>();
		initType((TypeNode*)type, NODE_FUNCTION_TYPE, TYPE_FUNCTION, start);

		int mark = parser->scratch->mark();

		nextToken(parser); // (

		bool next = !nextIs(parser, ')');
		while (next)
		{
			Parameter* param = parseParameter(parser);
			parser->scratch->add(param);

			next = nextIs(parser, ',');
			if (next)
				nextToken(parser);
		}

		if (!expectToken(parser, ')'))
			skipPastToken(parser, ')');

		type->params = copyFromScratchBuffer<Parameter*>(parser, mark, &type->numParams);

		parser->scratch->release(mark);

		if (nextIs(parser, '-') && nextIs(parser, 1, '>'))
		{
			nextToken(parser);
			nextToken(parser);
			type->returnType = parseType(parser);
		}

		type->end = parser->lastTokenEnd;

		return type;
	}
	else if (token.type == TOKEN_STRUCT)
	{
		nextToken(parser);

		StructType* type = parser->arena->alloc<StructType>();
		initType((TypeNode*)type, NODE_STRUCT_TYPE, TYPE_STRUCT, start);

		if (expectToken(parser, '{'))
		{
			int mark = parser->scratch->mark();

			while (!nextIs(parser, '}'))
			{
				if (Field* field = parseField(parser))
					parser->scratch->add((Node*)field);
				else
					skipPastToken(parser, ';');
			}

			type->fields = copyFromScratchBuffer<Field*>(parser, mark, &type->numFields);

			parser->scratch->release(mark);

			expectToken(parser, '}');
		}

		return type;
	}
	else if (token.type == TOKEN_UNION)
	{
		nextToken(parser);

		UnionType* type = parser->arena->alloc<UnionType>();
		initType((TypeNode*)type, NODE_UNION_TYPE, TYPE_UNION, start);

		if (expectToken(parser, '{'))
		{
			int mark = parser->scratch->mark();

			while (!nextIs(parser, '}'))
			{
				if (Field* field = parseField(parser))
					parser->scratch->add((Node*)field);
				else
					skipPastToken(parser, ';');
			}

			type->fields = copyFromScratchBuffer<Field*>(parser, mark, &type->numFields);

			parser->scratch->release(mark);

			expectToken(parser, '}');
		}

		return type;
	}
	else
	{
		return nullptr;
	}
}

static TypeNode* parseComplexType(Parser* parser, TypeNode* basicType)
{
	Token token;
	if (nextIs(parser, '*', &token) && token.offset == basicType->end)
	{
		nextToken(parser);

		PointerType* type = parser->arena->alloc<PointerType>();
		initType((TypeNode*)type, NODE_POINTER_TYPE, TYPE_POINTER, basicType->start);
		type->end = token.offset + token.length;
		type->elementType = basicType;

		return parseComplexType(parser, type);
	}
	else if (nextIs(parser, '?', &token) && token.offset == basicType->end)
	{
		nextToken(parser);

		OptionalType* type = parser->arena->alloc<OptionalType>();
		initType((TypeNode*)type, NODE_OPTIONAL_TYPE, TYPE_OPTIONAL, basicType->start);
		type->end = token.offset + token.length;
		type->elementType = basicType;

		return parseComplexType(parser, type);
	}
	else if (nextIs(parser, '[', &token) && token.offset == basicType->end)
	{
		nextToken(parser);

		Expression* size = nullptr;
		if (!nextIs(parser, ']'))
			size = parseExpression(parser);

		expectToken(parser, ']');

		ArrayType* type = parser->arena->alloc<ArrayType>();
		initType((TypeNode*)type, NODE_ARRAY_TYPE, TYPE_ARRAY, basicType->start);
		type->elementType = basicType;
		type->size = size;
		type->end = parser->lastTokenEnd;

		return parseComplexType(parser, type);
	}
	else
	{
		return basicType;
	}
}

TypeNode* parseType(Parser* parser)
{
	if (TypeNode* basicType = parseBasicType(parser))
	{
		return parseComplexType(parser, basicType);
	}
	return nullptr;
}

static Expression** parseExpressionList(Parser* parser, Expression* firstExpression, int* numExpressions)
{
	int mark = parser->scratch->mark();

	bool next = !nextIs(parser, ')');

	if (firstExpression)
	{
		parser->scratch->add(firstExpression);

		next = nextIs(parser, ',');
		if (next)
			nextToken(parser);
	}

	while (next)
	{
		if (Expression* expression = parseExpression(parser))
			parser->scratch->add(expression);
		else
			parser->scratch->add(getErrorExpression(parser, parser->cursor));

		next = nextIs(parser, ',');
		if (next)
			nextToken(parser);
	}

	Expression** expressions = copyFromScratchBuffer<Expression*>(parser, mark, numExpressions);

	parser->scratch->release(mark);

	return expressions;
}

Expression* parseAtom(Parser* parser)
{
	int start = parser->cursor;

	Token token;
	if (nextIs(parser, TOKEN_INT_LITERAL, &token))
	{
		nextToken(parser);

		IntLiteral* intLiteral = parser->arena->alloc<IntLiteral>();
		initNode((Node*)intLiteral, NODE_INT_LITERAL, start);
		intLiteral->value = getTokenString(token, parser);
		intLiteral->end = parser->lastTokenEnd;

		return intLiteral;
	}
	else if (nextIs(parser, TOKEN_FLOAT_LITERAL, &token))
	{
		nextToken(parser);

		FloatLiteral* floatLiteral = parser->arena->alloc<FloatLiteral>();
		initNode((Node*)floatLiteral, NODE_FLOAT_LITERAL, start);
		floatLiteral->value = getTokenString(token, parser);
		floatLiteral->end = parser->lastTokenEnd;

		return floatLiteral;
	}
	else if (nextIs(parser, TOKEN_STRING_LITERAL, &token))
	{
		nextToken(parser);

		StringLiteral* stringLiteral = parser->arena->alloc<StringLiteral>();
		initNode((Node*)stringLiteral, NODE_STRING_LITERAL, start);
		stringLiteral->value = CreateString(&parser->lexer.src[token.offset + 1], token.length - 2);
		stringLiteral->end = parser->lastTokenEnd;

		return stringLiteral;
	}
	else if (nextIs(parser, TOKEN_STRING_LITERAL_MULTILINE, &token))
	{
		nextToken(parser);

		StringLiteral* stringLiteral = parser->arena->alloc<StringLiteral>();
		initNode((Node*)stringLiteral, NODE_STRING_LITERAL, start);
		stringLiteral->value = CreateString(&parser->lexer.src[token.offset + 4], token.length - 8);
		stringLiteral->end = parser->lastTokenEnd;

		return stringLiteral;
	}
	else if (nextIs(parser, TOKEN_CHAR_LITERAL, &token))
	{
		nextToken(parser);

		CharLiteral* charLiteral = parser->arena->alloc<CharLiteral>();
		initNode((Node*)charLiteral, NODE_CHAR_LITERAL, start);
		charLiteral->value = CreateString(&parser->lexer.src[token.offset + 1], token.length - 2);
		charLiteral->end = parser->lastTokenEnd;

		return charLiteral;
	}
	else if (nextIs(parser, TOKEN_TRUE))
	{
		nextToken(parser);

		Expression* expression = parser->arena->alloc<Expression>();
		initNode((Node*)expression, NODE_TRUE, start);
		expression->end = parser->lastTokenEnd;

		return expression;
	}
	else if (nextIs(parser, TOKEN_FALSE))
	{
		nextToken(parser);

		Expression* expression = parser->arena->alloc<Expression>();
		initNode((Node*)expression, NODE_FALSE, start);
		expression->end = parser->lastTokenEnd;

		return expression;
	}
	else if (nextIs(parser, TOKEN_NULL_KEYWORD))
	{
		nextToken(parser);

		Expression* expression = parser->arena->alloc<Expression>();
		initNode((Node*)expression, NODE_NULL_LITERAL, start);
		expression->end = parser->lastTokenEnd;

		return expression;
	}
	else if (nextIs(parser, TOKEN_IDENTIFIER, &token) || nextIsKeyword(parser))
	{
		nextToken(parser);

		Identifier* identifier = parser->arena->alloc<Identifier>();
		initNode((Node*)identifier, NODE_IDENTIFIER, start);
		identifier->name = getTokenString(token, parser);
		identifier->end = parser->lastTokenEnd;

		return identifier;
	}
	else if (nextIs(parser, '('))
	{
		nextToken(parser);

		if (Expression* expression = parseExpression(parser))
		{
			if (nextIs(parser, ','))
			{
				ExpressionList* expressionList = parser->arena->alloc<ExpressionList>();
				initNode((Node*)expressionList, NODE_EXPRESSION_LIST, start);

				expressionList->values = parseExpressionList(parser, expression, &expressionList->numValues);

				if (!expectToken(parser, ')'))
					skipPastToken(parser, ')');

				expressionList->end = parser->lastTokenEnd;

				return expressionList;
			}
			else
			{
				if (!expectToken(parser, ')'))
					skipPastToken(parser, ')');

				CompoundExpression* compound = parser->arena->alloc<CompoundExpression>();
				initNode((Node*)compound, NODE_COMPOUND_EXPRESSION, start);
				compound->value = expression;
				compound->end = parser->lastTokenEnd;

				return compound;
			}
		}
		else
		{
			error(parser, getSourceLocation(parser), "Expression expected");
			skipPastToken(parser, ')');
			return getErrorExpression(parser, start);
		}
	}

	return nullptr;
}

static OperatorType peekBinaryOperatorType(Parser* parser, int* numTokens)
{
	Token token = peekToken(parser);
	Token token2 = peekToken(parser, 1);

	*numTokens = 1;

	if (token.type == TOKEN_AS)
	{
		return OPERATOR_CAST;
	}
	else if (token.type == '*' && token2.type != '=')
	{
		return OPERATOR_MULTIPLY;
	}
	else if (token.type == '/' && token2.type != '=')
	{
		return OPERATOR_DIVIDE;
	}
	else if (token.type == '%' && token2.type != '=')
	{
		return OPERATOR_MODULO;
	}
	else if (token.type == '+' && token2.type != '=')
	{
		return OPERATOR_ADD;
	}
	else if (token.type == '-' && token2.type != '=')
	{
		return OPERATOR_SUBTRACT;
	}
	else if (token.type == '<')
	{
		Token token3 = peekToken(parser, 2);
		if (token2.type == '<' && token3.type != '=')
		{
			*numTokens = 2;
			return OPERATOR_BITSHIFT_LEFT;
		}
		else if (token2.type == '=')
		{
			*numTokens = 2;
			return OPERATOR_LESS_EQUALS;
		}
		else
		{
			return OPERATOR_LESS;
		}
	}
	else if (token.type == '>')
	{
		Token token3 = peekToken(parser, 2);
		if (token2.type == '>' && token3.type != '=')
		{
			*numTokens = 2;
			return OPERATOR_BITSHIFT_RIGHT;
		}
		else if (token2.type == '=')
		{
			*numTokens = 2;
			return OPERATOR_GREATER_EQUALS;
		}
		else
		{
			return OPERATOR_GREATER;
		}
	}
	else if (token.type == '=' && peekToken(parser, 1).type == '=')
	{
		*numTokens = 2;
		return OPERATOR_EQUALS;
	}
	else if (token.type == '!' && peekToken(parser, 1).type == '=')
	{
		*numTokens = 2;
		return OPERATOR_NOT_EQUALS;
	}
	else if (token.type == '&')
	{
		Token token3 = peekToken(parser, 2);
		if (token2.type == '&' && token3.type != '=')
		{
			*numTokens = 2;
			return OPERATOR_LOGICAL_AND;
		}
		else if (token2.type != '=')
		{
			return OPERATOR_BITWISE_AND;
		}
	}
	else if (token.type == '|')
	{
		Token token3 = peekToken(parser, 2);
		if (token2.type == '|' && token3.type != '=')
		{
			*numTokens = 2;
			return OPERATOR_LOGICAL_OR;
		}
		else if (token2.type != '=')
		{
			return OPERATOR_BITWISE_OR;
		}
	}
	else if (token.type == '^' && token2.type != '=')
	{
		return OPERATOR_BITWISE_XOR;
	}

	return OPERATOR_NULL;
}

static OperatorType peekAssignmentOperatorType(Parser* parser, int* numTokens)
{
	Token token = peekToken(parser);
	*numTokens = 2;

	if (token.type == '=')
	{
		*numTokens = 1;
		return OPERATOR_ASSIGN;
	}
	else
	{
		Token token2 = peekToken(parser, 1);
		if (token.type == '+' && token2.type == '=')
		{
			return OPERATOR_ADD_ASSIGN;
		}
		else if (token.type == '-' && token2.type == '=')
		{
			return OPERATOR_SUBTRACT_ASSIGN;
		}
		else if (token.type == '*' && token2.type == '=')
		{
			return OPERATOR_MULTIPLY_ASSIGN;
		}
		else if (token.type == '/' && token2.type == '=')
		{
			return OPERATOR_DIVIDE_ASSIGN;
		}
		else if (token.type == '%' && token2.type == '=')
		{
			return OPERATOR_MODULO_ASSIGN;
		}
		else if (token.type == '&' && token2.type == '=')
		{
			return OPERATOR_BITWISE_AND_ASSIGN;
		}
		else if (token.type == '^' && token2.type == '=')
		{
			return OPERATOR_BITWISE_XOR_ASSIGN;
		}
		else if (token.type == '|' && token2.type == '=')
		{
			return OPERATOR_BITWISE_OR_ASSIGN;
		}
		else
		{
			Token token3 = peekToken(parser, 2);
			if (token.type == '<' && token2.type == '<' && token3.type == '=')
			{
				return OPERATOR_BITSHIFT_LEFT_ASSIGN;
			}
			else if (token.type == '>' && token2.type == '>' && token3.type == '=')
			{
				return OPERATOR_BITSHIFT_RIGHT_ASSIGN;
			}
			else if (token.type == '&' && token2.type == '&' && token3.type == '=')
			{
				return OPERATOR_LOGICAL_AND_ASSIGN;
			}
			else if (token.type == '|' && token2.type == '|' && token3.type == '=')
			{
				return OPERATOR_LOGICAL_OR_ASSIGN;
			}
		}
	}

	return OPERATOR_NULL;
}

static int getOperatorPrecedence(OperatorType operatorType)
{
	switch (operatorType)
	{
	case OPERATOR_INCREMENT_POSTFIX:
	case OPERATOR_DECREMENT_POSTFIX:
	case OPERATOR_FUNCTION_CALL:
	case OPERATOR_ARRAY_SUBSCRIPT:
	case OPERATOR_MEMBER_ACCESS:
		return 1;

	case OPERATOR_INCREMENT_PREFIX:
	case OPERATOR_DECREMENT_PREFIX:
	case OPERATOR_PLUS_PREFIX:
	case OPERATOR_MINUS_PREFIX:
	case OPERATOR_LOGICAL_NOT:
	case OPERATOR_BITWISE_NOT:
	case OPERATOR_DEREFERENCE:
	case OPERATOR_ADDRESS:
		return 2;

	case OPERATOR_MULTIPLY:
	case OPERATOR_DIVIDE:
	case OPERATOR_MODULO:
		return 3;

	case OPERATOR_ADD:
	case OPERATOR_SUBTRACT:
		return 4;

	case OPERATOR_BITSHIFT_LEFT:
	case OPERATOR_BITSHIFT_RIGHT:
		return 5;

	case OPERATOR_CAST:
		return 6;

	case OPERATOR_LESS:
	case OPERATOR_LESS_EQUALS:
	case OPERATOR_GREATER:
	case OPERATOR_GREATER_EQUALS:
		return 7;

	case OPERATOR_EQUALS:
	case OPERATOR_NOT_EQUALS:
		return 8;

	case OPERATOR_BITWISE_AND:
		return 9;

	case OPERATOR_BITWISE_XOR:
		return 10;

	case OPERATOR_BITWISE_OR:
		return 11;

	case OPERATOR_LOGICAL_AND:
		return 12;

	case OPERATOR_LOGICAL_OR:
		return 13;

	case OPERATOR_TERNARY_CONDITION:
		return 14;

	case OPERATOR_ASSIGN:
	case OPERATOR_ADD_ASSIGN:
	case OPERATOR_SUBTRACT_ASSIGN:
	case OPERATOR_MULTIPLY_ASSIGN:
	case OPERATOR_DIVIDE_ASSIGN:
	case OPERATOR_MODULO_ASSIGN:
	case OPERATOR_BITSHIFT_LEFT_ASSIGN:
	case OPERATOR_BITSHIFT_RIGHT_ASSIGN:
	case OPERATOR_BITWISE_AND_ASSIGN:
	case OPERATOR_BITWISE_XOR_ASSIGN:
	case OPERATOR_BITWISE_OR_ASSIGN:
		return 15;

	default:
		return 1000;
	}
}

static OperatorType parsePostfixOperatorType(Parser* parser)
{
	Token token = peekToken(parser);

	if (token.type == '(')
	{
		nextToken(parser);
		return OPERATOR_FUNCTION_CALL;
	}
	else if (token.type == '[')
	{
		nextToken(parser);
		return OPERATOR_ARRAY_SUBSCRIPT;
	}
	else if (token.type == '.')
	{
		nextToken(parser);
		return OPERATOR_MEMBER_ACCESS;
	}
	else
	{
		Token token2 = peekToken(parser, 1);
		if (token.type == '+' && token2.type == '+')
		{
			nextToken(parser);
			nextToken(parser);
			return OPERATOR_INCREMENT_POSTFIX;
		}
		else if (token.type == '-' && token2.type == '-')
		{
			nextToken(parser);
			nextToken(parser);
			return OPERATOR_DECREMENT_POSTFIX;
		}

		return OPERATOR_NULL;
	}
}

static TypeKind getPrimitiveTypeKind(TokenType tokenType)
{
	if (tokenType == TOKEN_VOID)
		return TYPE_VOID;
	if (tokenType == TOKEN_INT8)
		return TYPE_INT8;
	if (tokenType == TOKEN_INT16)
		return TYPE_INT16;
	if (tokenType == TOKEN_INT32)
		return TYPE_INT32;
	if (tokenType == TOKEN_INT64)
		return TYPE_INT64;
	if (tokenType == TOKEN_UINT8)
		return TYPE_UINT8;
	if (tokenType == TOKEN_UINT16)
		return TYPE_UINT16;
	if (tokenType == TOKEN_UINT32)
		return TYPE_UINT32;
	if (tokenType == TOKEN_UINT64)
		return TYPE_UINT64;
	if (tokenType == TOKEN_BOOL)
		return TYPE_BOOL;
	if (tokenType == TOKEN_STRING)
		return TYPE_STRING;
	if (tokenType == TOKEN_FLOAT32)
		return TYPE_FLOAT;
	if (tokenType == TOKEN_FLOAT64)
		return TYPE_DOUBLE;
	if (tokenType == TOKEN_ANY)
		return TYPE_ANY;
	return TYPE_NULL;
}

Expression* parsePostfixOperator(Parser* parser)
{
	if (Expression* expression = parseAtom(parser))
	{
		while (OperatorType operatorType = parsePostfixOperatorType(parser))
		{
			if (operatorType == OPERATOR_FUNCTION_CALL)
			{
				TypeNode* castType = nullptr;
				if (expression->type == NODE_IDENTIFIER)
				{
					StringView name = ((Identifier*)expression)->name;
					TokenType keywordType = getKeywordType(name.ptr, name.length);
					if (TypeKind typeKind = getPrimitiveTypeKind(keywordType))
					{
						static_assert(sizeof(TypeNode) <= sizeof(Expression));

						castType = (TypeNode*)expression;
						initType(castType, NODE_PRIMITIVE_TYPE, typeKind, expression->start);
						expression = nullptr;
					}
				}

				if (castType)
				{
					Cast* cast = parser->arena->alloc<Cast>();
					initNode((Node*)cast, NODE_CAST, castType->start);
					cast->targetType = castType;

					cast->expression = parseExpression(parser);

					if (castType->typeKind == TYPE_STRING)
					{
						if (nextIs(parser, ','))
						{
							nextToken(parser);
							cast->expression2 = parseExpression(parser);
						}
					}

					expectToken(parser, ')');

					cast->end = parser->lastTokenEnd;

					expression = cast;
				}
				else
				{
					FunctionCall* functionCall = parser->arena->alloc<FunctionCall>();
					initNode((Node*)functionCall, NODE_FUNCTION_CALL, expression->start);
					functionCall->expression = expression;

					functionCall->args = parseExpressionList(parser, nullptr, &functionCall->numArgs);

					expectToken(parser, ')');

					functionCall->end = parser->lastTokenEnd;

					expression = functionCall;
				}
			}
			else if (operatorType == OPERATOR_ARRAY_SUBSCRIPT)
			{
				ArraySubscript* subscript = parser->arena->alloc<ArraySubscript>();
				initNode((Node*)subscript, NODE_ARRAY_SUBSCRIPT, expression->start);
				subscript->operand = expression;

				subscript->args = parseExpressionList(parser, nullptr, &subscript->numArgs);

				expectToken(parser, ']');

				subscript->end = parser->lastTokenEnd;

				expression = subscript;
			}
			else if (operatorType == OPERATOR_MEMBER_ACCESS)
			{
				MemberAccess* member = parser->arena->alloc<MemberAccess>();
				initNode((Node*)member, NODE_MEMBER_ACCESS, expression->start);
				member->operand = expression;

				if (nextIs(parser, TOKEN_IDENTIFIER) || nextIsKeyword(parser))
				{
					Token identifier = nextToken(parser);
					member->name = getTokenString(identifier, parser);
					member->index = -1;
				}
				else if (nextIs(parser, TOKEN_INT_LITERAL))
				{
					Token index = nextToken(parser);
					member->index = getConstantInt(getTokenString(index, parser));
				}
				else
				{
					Token token = nextToken(parser);
					SourceLocation start, end;
					getSourceLocation(parser, token, &start, &end);
					error(parser, start, end, "Identifier or integer expected");
				}

				member->end = parser->lastTokenEnd;

				expression = member;
			}
			else
			{
				UnaryOperator* op = parser->arena->alloc<UnaryOperator>();
				initNode((Node*)op, NODE_UNARY_OPERATOR, expression->start);
				op->op = operatorType;
				op->operand = expression;
				op->end = parser->lastTokenEnd;

				expression = op;
			}
		}

		return expression;
	}

	return nullptr;
}

static OperatorType parsePrefixOperatorType(Parser* parser)
{
	Token token = peekToken(parser);
	if (token.type == '!')
	{
		nextToken(parser);
		return OPERATOR_LOGICAL_NOT;
	}
	else if (token.type == '~')
	{
		nextToken(parser);
		return OPERATOR_BITWISE_NOT;
	}
	else if (token.type == '*')
	{
		nextToken(parser);
		return OPERATOR_DEREFERENCE;
	}
	else if (token.type == '&')
	{
		nextToken(parser);
		return OPERATOR_ADDRESS;
	}
	else
	{
		Token token2 = peekToken(parser, 1);

		if (token.type == '+')
		{
			if (token2.type == '+')
			{
				nextToken(parser);
				nextToken(parser);
				return OPERATOR_INCREMENT_PREFIX;
			}
			else
			{
				nextToken(parser);
				return OPERATOR_PLUS_PREFIX;
			}
		}
		else if (token.type == '-')
		{
			if (token2.type == '-')
			{
				nextToken(parser);
				nextToken(parser);
				return OPERATOR_DECREMENT_PREFIX;
			}
			else
			{
				nextToken(parser);
				return OPERATOR_MINUS_PREFIX;
			}
		}

		return OPERATOR_NULL;
	}
}

Expression* parsePrefixOperator(Parser* parser)
{
	int start = parser->cursor;

	if (OperatorType operatorType = parsePrefixOperatorType(parser))
	{
		Expression* expression = parsePrefixOperator(parser);
		if (!expression)
		{
			error(parser, getSourceLocation(parser), "Expression expected");
			expression = getErrorExpression(parser, parser->cursor);
		}

		UnaryOperator* op = parser->arena->alloc<UnaryOperator>();
		initNode((Node*)op, NODE_UNARY_OPERATOR, start);
		op->op = operatorType;
		op->operand = expression;
		op->end = parser->lastTokenEnd;

		return op;
	}

	return parsePostfixOperator(parser);
}

Expression* parseBinaryOperator(Parser* parser, OperatorType lastOperatorType)
{
	if (Expression* left = parsePrefixOperator(parser))
	{
		int numOperatorTokens;
		while (OperatorType operatorType = peekBinaryOperatorType(parser, &numOperatorTokens))
		{
			if (getOperatorPrecedence(operatorType) < getOperatorPrecedence(lastOperatorType))
			{
				int operatorStart = parser->cursor;
				for (int i = 0; i < numOperatorTokens; i++)
					nextToken(parser);
				int operatorEnd = parser->cursor;

				if (operatorType == OPERATOR_CAST)
				{
					TypeNode* type = parseType(parser);
					if (!type)
					{
						error(parser, getSourceLocation(parser), "Type expected");
						type = getErrorType(parser, parser->cursor);
					}

					Cast* cast = parser->arena->alloc<Cast>();
					initNode((Node*)cast, NODE_CAST, left->start);
					cast->expression = left;
					cast->targetType = type;
					cast->end = parser->lastTokenEnd;

					left = cast;
				}
				else if (Expression* right = parseBinaryOperator(parser, operatorType))
				{
					BinaryOperator* op = parser->arena->alloc<BinaryOperator>();
					initNode((Node*)op, NODE_BINARY_OPERATOR, left->start);
					op->op = operatorType;
					op->left = left;
					op->right = right;
					op->end = parser->lastTokenEnd;

					left = op;
				}
				else
				{
					StringView operatorString = getRangedString(operatorStart, operatorEnd, parser);
					error(parser, getSourceLocation(parser), "Expected expression after operator %.*s", operatorString.length, operatorString.ptr);
					return left;
				}
			}
			else
			{
				break;
			}
		}

		return left;
	}

	return nullptr;
}

Expression* parseExpression(Parser* parser)
{
	Expression* expression = parseBinaryOperator(parser, OPERATOR_NULL);

	if (nextIs(parser, '?'))
	{
		nextToken(parser);

		Expression* then = parseExpression(parser);

		expectToken(parser, ':');

		Expression* else_ = parseExpression(parser);

		TernaryCondition* ternary = parser->arena->alloc<TernaryCondition>();
		initNode((Node*)ternary, NODE_TERNARY_CONDITION, expression->start);
		ternary->condition = expression;
		ternary->then = then;
		ternary->else_ = else_;
		ternary->end = parser->lastTokenEnd;

		return ternary;
	}

	return expression;
}

static Statement** parseCodeBlock(Parser* parser, int endToken, int* numStatements)
{
	int mark = parser->scratch->mark();

	while (!nextIs(parser, 0) && !nextIs(parser, endToken))
	{
		if (Statement* statement = parseStatement(parser))
		{
			parser->scratch->add(statement);
		}
		else
		{
			Token token = nextToken(parser);
			StringView tokenStr = getTokenString(token, parser);
			SourceLocation start, end;
			getSourceLocation(parser, token, &start, &end);
			error(parser, start, end, "Unexpected token '%.*s'", tokenStr.length, tokenStr.ptr);
		}
	}

	Statement** statements = copyFromScratchBuffer<Statement*>(parser, mark, numStatements);

	parser->scratch->release(mark);

	return statements;
}

Statement* parseStatement(Parser* parser)
{
	int start = parser->cursor;

	uint32_t storage = 0;
	while (StorageSpecifier storageSpecifier = getStorageSpecifier(peekToken(parser).type))
	{
		storage |= storageSpecifier;
		nextToken(parser);
	}

	if (nextIs(parser, '{'))
	{
		nextToken(parser);

		BlockStatement* block = parser->arena->alloc<BlockStatement>();
		initNode((Node*)block, NODE_BLOCK_STATEMENT, start);

		block->statements = parseCodeBlock(parser, '}', &block->numStatements);

		expectToken(parser, '}');

		block->end = parser->lastTokenEnd;

		return block;
	}
	else if (nextIs(parser, TOKEN_IF))
	{
		nextToken(parser);

		If* if_ = parser->arena->alloc<If>();
		initNode((Node*)if_, NODE_IF, start);

		if (Expression* condition = parseExpression(parser))
		{
			if_->condition = condition;
		}
		else
		{
			error(parser, getSourceLocation(parser), "Expression expected");
			if_->condition = getErrorExpression(parser, parser->cursor);
		}

		if (Statement* then = parseStatement(parser))
		{
			if_->then = then;

			if (nextIs(parser, TOKEN_ELSE))
			{
				nextToken(parser);
				if (Statement* else_ = parseStatement(parser))
					if_->else_ = else_;
				else
				{
					error(parser, getSourceLocation(parser), "Statement expected");
				}
			}
		}
		else
		{
			error(parser, getSourceLocation(parser), "Statement expected");
			if_->then = getErrorStatement(parser, parser->cursor);
		}


		if_->end = parser->lastTokenEnd;

		return if_;
	}
	else if (nextIs(parser, TOKEN_WHILE))
	{
		nextToken(parser);

		While* while_ = parser->arena->alloc<While>();
		initNode((Node*)while_, NODE_WHILE, start);

		if (Expression* condition = parseExpression(parser))
		{
			while_->condition = condition;

			if (Statement* then = parseStatement(parser))
			{
				while_->then = then;
			}
			else
			{
				error(parser, getSourceLocation(parser), "Statement expected");
				while_->then = getErrorStatement(parser, parser->cursor);
			}
		}
		else
		{
			error(parser, getSourceLocation(parser), "Expression expected");
		}

		while_->end = parser->lastTokenEnd;

		return while_;
	}
	else if (nextIs(parser, TOKEN_FOR))
	{
		nextToken(parser);

		For* for_ = parser->arena->alloc<For>();
		initNode((Node*)for_, NODE_FOR, start);

		expectToken(parser, '(');

		Token iteratorName;
		if (expectToken(parser, TOKEN_IDENTIFIER, &iteratorName))
		{
			for_->iteratorName = getTokenString(iteratorName, parser);
		}

		expectToken(parser, ',');

		Expression* startValue = parseExpression(parser);
		if (!startValue)
		{
			error(parser, getSourceLocation(parser), "Expression expected");
		}
		for_->startValue = startValue;

		expectToken(parser, ',');

		bool equals = false;
		if (nextIs(parser, '='))
		{
			nextToken(parser);
			equals = true;
		}
		for_->equals = equals;

		Expression* compareValue = parseExpression(parser);
		if (!compareValue)
		{
			error(parser, getSourceLocation(parser), "Expression expected");
		}
		for_->compareValue = compareValue;

		if (!expectToken(parser, ')'))
			skipPastToken(parser, ')');

		Statement* body = parseStatement(parser);
		if (!body)
		{
			error(parser, getSourceLocation(parser), "Statement expected");
		}
		for_->body = body;

		for_->end = parser->lastTokenEnd;

		return for_;
	}
	else if (nextIs(parser, TOKEN_RETURN))
	{
		nextToken(parser);

		Expression* value = nullptr;
		if (!nextIs(parser, ';'))
		{
			value = parseExpression(parser);
			if (!value)
			{
				error(parser, getSourceLocation(parser), "Expression expected");
			}
		}

		expectToken(parser, ';');

		Return* return_ = parser->arena->alloc<Return>();
		initNode((Node*)return_, NODE_RETURN, start);
		return_->value = value;
		return_->end = parser->lastTokenEnd;

		return return_;
	}
	else if (nextIs(parser, TOKEN_BREAK))
	{
		nextToken(parser);

		Statement* break_ = parser->arena->alloc<Statement>();
		initNode((Node*)break_, NODE_BREAK, start);
		break_->end = parser->lastTokenEnd;

		return break_;
	}
	else if (nextIs(parser, TOKEN_CONTINUE))
	{
		nextToken(parser);

		Statement* continue_ = parser->arena->alloc<Statement>();
		initNode((Node*)continue_, NODE_CONTINUE, start);
		continue_->end = parser->lastTokenEnd;

		return continue_;
	}
	else if (nextIs(parser, TOKEN_DEFER))
	{
		nextToken(parser);

		Statement* body = parseStatement(parser);
		if (!body)
		{
			error(parser, getSourceLocation(parser), "Statement expected");
		}

		Defer* defer = parser->arena->alloc<Defer>();
		initNode((Node*)defer, NODE_DEFER, start);
		defer->body = body;
		defer->end = parser->lastTokenEnd;

		return defer;
	}
	else
	{
		int start = parser->cursor;

		if (TypeNode* type = parseType(parser))
		{
			if (nextIs(parser, TOKEN_IDENTIFIER) || nextIsKeyword(parser))
			{
				int mark = parser->scratch->mark();

				bool next = true;
				while (next)
				{
					Token identifier = nextToken(parser);

					VariableDeclarator declarator = {};
					declarator.name = getTokenString(identifier, parser);
					declarator.value = nullptr;

					if (nextIs(parser, '='))
					{
						nextToken(parser);
						declarator.value = parseExpression(parser);

						if (!declarator.value)
						{
							error(parser, getSourceLocation(parser), "Expression expected");
						}
					}

					parser->scratch->add(declarator);

					next = nextIs(parser, ',');
					if (next)
						nextToken(parser);
				}

				expectToken(parser, ';');

				VariableDeclaration* variableDeclaration = parser->arena->alloc<VariableDeclaration>();
				initNode((Node*)variableDeclaration, NODE_VARIABLE_DECLARATION, start);
				variableDeclaration->storage = storage;
				variableDeclaration->variableType = type;
				variableDeclaration->end = parser->lastTokenEnd;

				variableDeclaration->declarators = copyFromScratchBuffer<VariableDeclarator>(parser, mark, &variableDeclaration->numDeclarators);

				parser->scratch->release(mark);

				return variableDeclaration;
			}
			else
			{
				rewind(parser, start);
			}
		}

		if (Expression* expression = parseExpression(parser))
		{
			int numOperatorTokens;
			if (OperatorType operatorType = peekAssignmentOperatorType(parser, &numOperatorTokens))
			{
				for (int i = 0; i < numOperatorTokens; i++)
					nextToken(parser);

				Assignment* assignment = parser->arena->alloc<Assignment>();
				initNode((Node*)assignment, NODE_ASSIGNMENT, expression->start);
				assignment->op = operatorType;
				assignment->expression = expression;

				assignment->value = parseExpression(parser);

				expectToken(parser, ';');

				return assignment;
			}
			else
			{
				expectToken(parser, ';');

				ExpressionStatement* expressionStatement = parser->arena->alloc<ExpressionStatement>();
				initNode((Node*)expressionStatement, NODE_EXPRESSION_STATEMENT, start);
				expressionStatement->end = parser->lastTokenEnd;
				expressionStatement->expression = expression;
				return expressionStatement;
			}
		}
	}

	return nullptr;
}

Field* parseField(Parser* parser)
{
	TypeNode* type = parseType(parser);
	if (!type)
		return nullptr;

	Field* field = parser->arena->alloc<Field>();
	initNode((Node*)field, NODE_FIELD, type->start);
	field->variableType = type;

	bool hasName = (nextIs(parser, TOKEN_IDENTIFIER) || nextIsKeyword(parser)) && nextIs(parser, 1, ';');
	if ((type->typeKind == TYPE_STRUCT || type->typeKind == TYPE_UNION) && !hasName)
	{
		field->end = parser->lastTokenEnd;
		return field;
	}

	int mark = parser->scratch->mark();

	Token identifier;
	bool next = nextIs(parser, TOKEN_IDENTIFIER, &identifier) || nextIsKeyword(parser, &identifier);
	while (next)
	{
		if (nextIsKeyword(parser, &identifier))
		{
			nextToken(parser);
		}
		else if (!expectToken(parser, TOKEN_IDENTIFIER, &identifier))
		{
			parser->scratch->release(mark);
			skipPastToken(parser, ';');
			return nullptr;
		}

		VariableDeclarator declarator = {};
		declarator.name = getTokenString(identifier, parser);
		declarator.value = nullptr;

		if (nextIs(parser, '='))
		{
			nextToken(parser);
			declarator.value = parseExpression(parser);

			if (!declarator.value)
			{
				error(parser, getSourceLocation(parser), "Expression expected");
			}
		}

		parser->scratch->add(declarator);

		next = nextIs(parser, ',');
		if (next)
			nextToken(parser);
	}

	if (!expectToken(parser, ';'))
		skipPastToken(parser, ';');

	field->declarators = copyFromScratchBuffer<VariableDeclarator>(parser, mark, &field->numDeclarators);

	parser->scratch->release(mark);

	field->end = parser->lastTokenEnd;

	return field;
}

Struct* parseStruct(Parser* parser, uint32_t storage, int start)
{
	nextToken(parser); // struct

	Struct* struct_ = parser->arena->alloc<Struct>();
	initNode((Node*)struct_, NODE_STRUCT, start);
	struct_->storage = storage;

	Token identifier;
	if (!expectToken(parser, TOKEN_IDENTIFIER, &identifier))
	{
		struct_->end = parser->lastTokenEnd;
		return struct_;
	}

	struct_->name = getTokenString(identifier, parser);

	if (nextIs(parser, ';'))
	{
		nextToken(parser);
	}
	else if (expectToken(parser, '{'))
	{
		int mark = parser->scratch->mark();

		while (hasNext(parser) && !nextIs(parser, '}'))
		{
			if (Field* field = parseField(parser))
				parser->scratch->add((Node*)field);
			else
				skipPastToken(parser, ';');
		}

		struct_->fields = copyFromScratchBuffer<Field*>(parser, mark, &struct_->numFields);

		parser->scratch->release(mark);

		nextToken(parser);
	}

	struct_->end = parser->lastTokenEnd;

	return struct_;
}

Union* parseUnion(Parser* parser, uint32_t storage, int start)
{
	nextToken(parser); // union

	Union* union_ = parser->arena->alloc<Union>();
	initNode((Node*)union_, NODE_UNION, start);
	union_->storage = storage;

	Token identifier;
	if (!expectToken(parser, TOKEN_IDENTIFIER, &identifier))
	{
		union_->end = parser->lastTokenEnd;
		return union_;
	}

	union_->name = getTokenString(identifier, parser);

	if (nextIs(parser, ';'))
	{
		nextToken(parser);
	}
	else if (expectToken(parser, '{'))
	{
		int mark = parser->scratch->mark();

		while (hasNext(parser) && !nextIs(parser, '}'))
		{
			if (Field* field = parseField(parser))
				parser->scratch->add((Node*)field);
			else
				skipPastToken(parser, ';');
		}

		union_->fields = copyFromScratchBuffer<Field*>(parser, mark, &union_->numFields);

		parser->scratch->release(mark);

		nextToken(parser);
	}

	union_->end = parser->lastTokenEnd;

	return union_;
}

Enum* parseEnum(Parser* parser, uint32_t storage, int start)
{
	nextToken(parser); // enum

	Enum* enum_ = parser->arena->alloc<Enum>();
	initNode((Node*)enum_, NODE_ENUM, start);
	enum_->storage = storage;

	Token identifier;
	expectToken(parser, TOKEN_IDENTIFIER, &identifier);
	enum_->name = getTokenString(identifier, parser);

	if (nextIs(parser, '='))
	{
		nextToken(parser);
		enum_->valueType = parseType(parser);
	}

	expectToken(parser, '{');

	int mark = parser->scratch->mark();

	while (nextIs(parser, TOKEN_IDENTIFIER))
	{
		Token identifier = nextToken(parser);

		EnumValue* enumValue = parser->arena->alloc<EnumValue>();
		initNode((Node*)enumValue, NODE_ENUM_VALUE, start);
		enumValue->name = getTokenString(identifier, parser);

		if (nextIs(parser, '='))
		{
			nextToken(parser);
			enumValue->value = parseExpression(parser);
		}

		parser->scratch->add(enumValue);

		if (nextIs(parser, ','))
			nextToken(parser);
	}

	enum_->values = copyFromScratchBuffer<EnumValue*>(parser, mark, &enum_->numValues);

	parser->scratch->release(mark);

	expectToken(parser, '}');

	enum_->end = parser->lastTokenEnd;

	return enum_;
}

Typedef* parseTypedef(Parser* parser, uint32_t storage, int start)
{
	nextToken(parser); // typedef

	Typedef* typedef_ = parser->arena->alloc<Typedef>();
	initNode((Node*)typedef_, NODE_TYPEDEF, start);
	typedef_->storage = storage;

	Token identifier;
	expectToken(parser, TOKEN_IDENTIFIER, &identifier);
	typedef_->name = getTokenString(identifier, parser);

	if (nextIs(parser, '='))
	{
		nextToken(parser);
		typedef_->value = parseType(parser);
	}

	if (!expectToken(parser, ';'))
		skipPastToken(parser, ';');

	typedef_->end = parser->lastTokenEnd;

	return typedef_;
}

Parameter* parseParameter(Parser* parser)
{
	TypeNode* type = parseType(parser);
	if (!type)
		return nullptr;

	Parameter* parameter = parser->arena->alloc<Parameter>();
	initNode((Node*)parameter, NODE_PARAMETER, parser->cursor);
	parameter->paramType = type;

	bool variadic = false;
	if (nextIs(parser, '.') && nextIs(parser, 1, '.') && nextIs(parser, 2, '.'))
	{
		nextToken(parser);
		nextToken(parser);
		nextToken(parser);
		variadic = true;
	}

	Token nameToken;
	if (nextIs(parser, TOKEN_IDENTIFIER) || nextIsKeyword(parser))
		nameToken = nextToken(parser);
	else
		expectToken(parser, TOKEN_IDENTIFIER, &nameToken);

	parameter->name = getTokenString(nameToken, parser);
	parameter->variadic = variadic;
	parameter->end = parser->lastTokenEnd;

	return parameter;
}

Function* parseFunction(Parser* parser, uint32_t storage, int start)
{
	nextToken(parser); // func

	Function* function = parser->arena->alloc<Function>();
	initNode((Node*)function, NODE_FUNCTION, start);
	function->storage = storage;

	Token identifier;
	expectToken(parser, TOKEN_IDENTIFIER, &identifier);
	function->name = getTokenString(identifier, parser);

	if (expectToken(parser, '('))
	{
		int mark = parser->scratch->mark();

		bool next = !nextIs(parser, ')');
		while (next)
		{
			if (Parameter* parameter = parseParameter(parser))
				parser->scratch->add((Node*)parameter);
			next = nextIs(parser, ',');
			if (next) nextToken(parser);
		}

		if (!expectToken(parser, ')'))
			skipPastToken(parser, ')');

		function->params = copyFromScratchBuffer<Parameter*>(parser, mark, &function->numParams);

		parser->scratch->release(mark);
	}
	else
	{
		skipPastToken(parser, '(');
		skipPastTokenNested(parser, '(', ')');

		function->numParams = 0;
		function->params = nullptr;
	}

	if (nextIs(parser, '=') && nextIs(parser, 1, '>'))
	{
		nextToken(parser);
		nextToken(parser);
		function->hasBody = true;
		function->value = parseExpression(parser);
		expectToken(parser, ';');
	}
	else
	{
		if (!nextIs(parser, '{') && !nextIs(parser, ';'))
		{
			function->returnType = parseType(parser);
		}

		if (nextIs(parser, ';'))
		{
			nextToken(parser);
		}
		else
		{
			expectToken(parser, '{');
			function->hasBody = true;
			function->statements = parseCodeBlock(parser, '}', &function->numStatements);
			expectToken(parser, '}');
		}
	}

	function->end = parser->lastTokenEnd;

	return function;
}

GlobalVariable* parseGlobalVariable(Parser* parser, TypeNode* type, uint32_t storage, int start)
{
	int mark = parser->scratch->mark();

	Token identifier;
	bool next = nextIs(parser, TOKEN_IDENTIFIER, &identifier);
	while (next)
	{
		if (!expectToken(parser, TOKEN_IDENTIFIER, &identifier))
		{
			parser->scratch->release(mark);
			skipPastToken(parser, ';');
			return nullptr;
		}

		VariableDeclarator declarator = {};
		declarator.name = getTokenString(identifier, parser);
		declarator.value = nullptr;

		if (nextIs(parser, '='))
		{
			nextToken(parser);
			declarator.value = parseExpression(parser);

			if (!declarator.value)
			{
				error(parser, getSourceLocation(parser), "Expression expected");
			}
		}

		parser->scratch->add(declarator);

		next = nextIs(parser, ',');
		if (next)
			nextToken(parser);
	}

	if (!expectToken(parser, ';'))
		skipPastToken(parser, ';');

	GlobalVariable* globalVariable = parser->arena->alloc<GlobalVariable>();
	initNode((Node*)globalVariable, NODE_GLOBAL_VARIABLE, start);
	globalVariable->storage = storage;
	globalVariable->variableType = type;
	globalVariable->end = parser->lastTokenEnd;

	globalVariable->declarators = copyFromScratchBuffer<VariableDeclarator>(parser, mark, &globalVariable->numDeclarators);

	parser->scratch->release(mark);

	return globalVariable;
}

Macro* parseMacro(Parser* parser, uint32_t storage, int start)
{
	nextToken(parser); // macro

	Token identifier;
	if (!expectToken(parser, TOKEN_IDENTIFIER, &identifier))
		return nullptr;

	Macro* macro = parser->arena->alloc<Macro>();
	initNode((Node*)macro, NODE_MACRO, start);
	macro->storage = storage;
	macro->name = getTokenString(identifier, parser);

	if (nextIs(parser, ';'))
	{
		nextToken(parser);
	}
	else if (expectToken(parser, '{'))
	{
		skipPastTokenNested(parser, '{', '}');
	}

	macro->end = parser->lastTokenEnd;

	return macro;
}

Import* parseImport(Parser* parser, uint32_t storage, int start)
{
	nextToken(parser); // import

	Token identifier;
	if (!nextIs(parser, TOKEN_IDENTIFIER) && !nextIsKeyword(parser))
	{
		expectToken(parser, TOKEN_IDENTIFIER);
		return nullptr;
	}

	identifier = nextToken(parser);

	Import* import_ = parser->arena->alloc<Import>();
	initNode((Node*)import_, NODE_IMPORT, start);

	int mark = parser->scratch->mark();

	parser->scratch->add(getTokenString(identifier, parser));

	while (nextIs(parser, '.'))
	{
		nextToken(parser);

		Token identifier;
		if (!nextIs(parser, TOKEN_IDENTIFIER) && !nextIsKeyword(parser))
			break;

		identifier = nextToken(parser);

		parser->scratch->add(getTokenString(identifier, parser));
	}

	import_->path = copyFromScratchBuffer<StringView>(parser, mark, &import_->pathCount);

	parser->scratch->release(mark);

	if (!expectToken(parser, ';'))
		skipPastToken(parser, ';');

	import_->end = parser->lastTokenEnd;

	return import_;
}

void parseFile(Parser* parser, AST* ast)
{
	int mark = parser->scratch->mark();

	Token token = {};
	while ((token = peekToken(parser)).type)
	{
		int start = parser->cursor;

		uint32_t storage = 0;
		while (StorageSpecifier storageSpecifier = getStorageSpecifier(token.type))
		{
			storage |= storageSpecifier;
			nextToken(parser);
			token = peekToken(parser);
		}

		if (token.type == TOKEN_STRUCT)
		{
			if (Struct* struct_ = parseStruct(parser, storage, start))
			{
				parser->scratch->add((Node*)struct_);
				ast->numStructs++;
			}
		}
		else if (token.type == TOKEN_ENUM)
		{
			if (Enum* enum_ = parseEnum(parser, storage, start))
			{
				parser->scratch->add((Node*)enum_);
				ast->numEnums++;
			}
		}
		else if (token.type == TOKEN_UNION)
		{
			if (Union* union_ = parseUnion(parser, storage, start))
			{
				parser->scratch->add((Node*)union_);
				ast->numUnions++;
			}
		}
		else if (token.type == TOKEN_TYPEDEF)
		{
			if (Typedef* typedef_ = parseTypedef(parser, storage, start))
			{
				parser->scratch->add((Node*)typedef_);
				ast->numTypedefs++;
			}
		}
		else if (token.type == TOKEN_FUNCTION && peekToken(parser, 1).type == TOKEN_IDENTIFIER)
		{
			if (Function* function = parseFunction(parser, storage, start))
			{
				parser->scratch->add((Node*)function);
				ast->numFunctions++;
			}
		}
		else if (token.type == TOKEN_MACRO)
		{
			if (Macro* macro = parseMacro(parser, storage, start))
			{
				parser->scratch->add((Node*)macro);
				ast->numMacros++;
			}
		}
		else if (token.type == TOKEN_MODULE)
		{
			skipPastToken(parser, ';');
		}
		else if (token.type == TOKEN_NAMESPACE)
		{
			skipPastToken(parser, ';');
		}
		else if (token.type == TOKEN_IMPORT)
		{
			if (Import* import = parseImport(parser, storage, start))
			{
				parser->scratch->add((Node*)import);
				ast->numImports++;
			}
		}
		else if (TypeNode* type = parseType(parser))
		{
			if (GlobalVariable* globalVariable = parseGlobalVariable(parser, type, storage, start))
			{
				parser->scratch->add((Node*)globalVariable);
				ast->numGlobalVariables++;
			}
		}
		else
		{
			nextToken(parser);

			SourceLocation start, end;
			getSourceLocation(parser, token, &start, &end);
			StringView tokenStr = getTokenString(token, parser);
			error(parser, start, end, "Unexpected token '%.*s'", tokenStr.length, tokenStr.ptr);

			if (token.type == '{')
			{
				skipPastTokenNested(parser, '{', '}');
			}
		}
	}

	ast->declarations = copyFromScratchBuffer<Node*>(parser, mark, &ast->numDeclarations);

	parser->scratch->release(mark);
}

void parse(Parser* parser, AST* ast)
{
	parseFile(parser, ast);
}

void resolveDependencies(Parser* parser, File* file)
{
	for (int i = 0; i < file->ast.numDeclarations; i++)
	{
		Node* declaration = file->ast.declarations[i];
		if (declaration->type == NODE_IMPORT)
		{
			char buffer[256];
			getLocalPathFromModuleName(buffer, declaration->import.path, declaration->import.pathCount);
			FileHandle fileHandle = getFileHandle(buffer);
			if (getFileFromHandle(fileHandle))
				file->dependencies.add(fileHandle);
			else
			{
				error(parser, declaration, "Undefined module '%s'", buffer);
			}
		}
	}
}
