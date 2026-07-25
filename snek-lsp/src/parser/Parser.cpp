#include "Parser.h"

#include "Lexer.h"
#include "AST.h"
#include "Diagnostics.h"

#include "utils/Log.h"
#include "utils/Arena.h"
#include "utils/List.h"

#include <stdarg.h>


void initParser(Parser* parser, const char* filename, const char* src, int length, Arena* arena, Diagnostics* diagnostics)
{
	parser->lexer = {};
	initLexer(&parser->lexer, filename, src, length, arena, diagnostics);

	parser->arena = arena;
	parser->diagnostics = diagnostics;
	parser->lookaheadCount = 0;
	parser->lastTokenEnd = 0;

	initScratchBuffer(&parser->scratch, 16);
}

void destroyParser(Parser* parser)
{
	destroyScratchBuffer(&parser->scratch);
	destroyLexer(&parser->lexer);
}

static Scope* pushScope(Parser* parser)
{
	Scope* scope = parser->arena->alloc<Scope>();
	scope->parent = parser->currentScope;

	bool isGlobal = false;
	initSymbolTable(&scope->symbols, isGlobal ? 1024 : 16, parser->arena);

	parser->currentScope = scope;
}

static void popScope(Parser* parser)
{
	parser->currentScope = parser->currentScope->parent;
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

static Token peekToken(Parser* parser, int count = 0)
{
	SnekAssert(count <= 3);
	while (parser->lookaheadCount <= count)
	{
		parser->lookahead[parser->lookaheadCount] = nextToken(&parser->lexer);
		parser->lookaheadState[parser->lookaheadCount] = parser->lexer.cursor;
		parser->lookaheadCount++;
	}

	return parser->lookahead[count];
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
	getSourceLocation(parser, token, &start, &end);

	if (type < TOKEN_FIRST)
		error(parser, start, end, "Expected token %c", (int)type);
	else
		// todo use token type string
		error(parser, start, end, "Expected token %d", (int)type);
	return false;
}

static bool nextIs(Parser* parser, int type, Token* outToken = nullptr)
{
	Token token = peekToken(parser);
	if (outToken) *outToken = token;
	return token.type == type;
}

static bool nextIsKeyword(Parser* parser)
{
	TokenType type = peekToken(parser).type;
	return type >= TOKEN_KEYWORD_BEGIN && type <= TOKEN_KEYWORD_END;
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
	while (level > 0)
	{
		Token token = nextToken(parser);
		if (token.type == openType)
			level++;
		else if (token.type == closeType)
			level--;
	}
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

	int count = parser->scratch.count<T>(mark);
	if (count > 0)
	{
		nodes = parser->arena->alloc<T>(count);
		memcpy(nodes, parser->scratch.memory + mark, count * sizeof(T));
	}

	*outCount = count;
	parser->scratch.release(mark);

	return nodes;
}

static Type* parseBasicType(Parser* parser)
{
	Token token = peekToken(parser);
	int start, end;
	getTokenRange(token, &start, &end);

	if (token.type == TOKEN_VOID)
	{
		nextToken(parser);

		Type* type = parser->arena->alloc<Type>();
		initType(type, NODE_PRIMITIVE_TYPE, TYPE_VOID, start);
		type->end = end;
		return type;
	}
	else if (token.type == TOKEN_INT8 || token.type == TOKEN_INT16 || token.type == TOKEN_INT32 || token.type == TOKEN_INT64)
	{
		nextToken(parser);

		uint8_t typeKind = TYPE_INT8 + (token.type - TOKEN_INT8);

		Type* type = parser->arena->alloc<Type>();
		initType(type, NODE_PRIMITIVE_TYPE, typeKind, start);
		type->end = end;
		return type;
	}
	else if (token.type == TOKEN_UINT8 || token.type == TOKEN_UINT16 || token.type == TOKEN_UINT32 || token.type == TOKEN_UINT64)
	{
		nextToken(parser);

		uint8_t typeKind = TYPE_UINT8 + (token.type - TOKEN_UINT8);

		Type* type = parser->arena->alloc<Type>();
		initType(type, NODE_PRIMITIVE_TYPE, typeKind, start);
		type->end = end;
		return type;
	}
	else if (token.type == TOKEN_FLOAT32 || token.type == TOKEN_FLOAT64)
	{
		nextToken(parser);

		uint8_t typeKind = token.type == TOKEN_FLOAT32 ? TYPE_FLOAT : TYPE_DOUBLE;

		Type* type = parser->arena->alloc<Type>();
		initType(type, NODE_PRIMITIVE_TYPE, typeKind, start);
		type->end = end;
		return type;
	}
	else if (token.type == TOKEN_BOOL)
	{
		nextToken(parser);

		Type* type = parser->arena->alloc<Type>();
		initType(type, NODE_PRIMITIVE_TYPE, TYPE_BOOL, start);
		type->end = end;
		return type;
	}
	else if (token.type == TOKEN_ANY)
	{
		nextToken(parser);

		Type* type = parser->arena->alloc<Type>();
		initType(type, NODE_PRIMITIVE_TYPE, TYPE_ANY, start);
		type->end = end;
		return type;
	}
	else if (token.type == TOKEN_STRING)
	{
		nextToken(parser);

		Type* type = parser->arena->alloc<Type>();
		initType(type, NODE_PRIMITIVE_TYPE, TYPE_STRING, start);
		type->end = end;
		return type;
	}
	else if (token.type == TOKEN_IDENTIFIER)
	{
		nextToken(parser);

		NamedType* type = parser->arena->alloc<NamedType>();
		initType((Type*)type, NODE_NAMED_TYPE, 0, start);
		type->end = end;
		type->name = getTokenString(token, parser);
		return type;
	}
	else
	{
		return nullptr;
	}
}

Type* parseType(Parser* parser)
{
	if (Type* basicType = parseBasicType(parser))
	{
		Token token;
		if (nextIs(parser, TOKEN_ASTERISK, &token) && token.offset == basicType->end)
		{
			nextToken(parser);

			PointerType* type = parser->arena->alloc<PointerType>();
			initType((Type*)type, NODE_POINTER_TYPE, TYPE_POINTER, basicType->start);
			type->end = token.offset + token.length;
			type->elementType = basicType;

			return type;
		}
		else if (nextIs(parser, TOKEN_QUESTION, &token) && token.offset == basicType->end)
		{
			nextToken(parser);

			OptionalType* type = parser->arena->alloc<OptionalType>();
			initType((Type*)type, NODE_OPTIONAL_TYPE, TYPE_OPTIONAL, basicType->start);
			type->end = token.offset + token.length;
			type->elementType = basicType;

			return type;
		}
		else if (nextIs(parser, '[', &token) && token.offset == basicType->end)
		{
			nextToken(parser);

			Token constantSizeToken;
			if (!expectToken(parser, TOKEN_INT_LITERAL, &constantSizeToken))
				return nullptr;

			if (!expectToken(parser, ']'))
				return nullptr;

			int end = constantSizeToken.offset + constantSizeToken.length;

			StringView constantSizeStr = getTokenString(constantSizeToken, parser);
			int64_t constantSize = strtoll(constantSizeStr.ptr, nullptr, 10);

			ArrayType* type = parser->arena->alloc<ArrayType>();
			initType((Type*)type, NODE_ARRAY_TYPE, TYPE_ARRAY, basicType->start);
			type->end = end;
			type->elementType = basicType;
			type->constantSize = constantSize;

			return type;
		}
		else
		{
			return basicType;
		}
	}
	return nullptr;
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
		intLiteral->end = parser->cursor;

		return intLiteral;
	}
	else if (nextIs(parser, TOKEN_STRING_LITERAL, &token))
	{
		nextToken(parser);

		StringLiteral* stringLiteral = parser->arena->alloc<StringLiteral>();
		initNode((Node*)stringLiteral, NODE_STRING_LITERAL, start);
		stringLiteral->value = CreateString(&parser->lexer.src[token.offset + 1], token.length - 2);
		stringLiteral->end = parser->cursor;

		return stringLiteral;
	}
	else if (nextIs(parser, TOKEN_STRING_LITERAL_MULTILINE, &token))
	{
		nextToken(parser);

		StringLiteral* stringLiteral = parser->arena->alloc<StringLiteral>();
		initNode((Node*)stringLiteral, NODE_STRING_LITERAL, start);
		stringLiteral->value = CreateString(&parser->lexer.src[token.offset + 4], token.length - 8);
		stringLiteral->end = parser->cursor;

		return stringLiteral;
	}
	else if (nextIs(parser, TOKEN_IDENTIFIER, &token))
	{
		nextToken(parser);

		Identifier* identifier = parser->arena->alloc<Identifier>();
		initNode((Node*)identifier, NODE_IDENTIFIER, start);
		identifier->name = getTokenString(token, parser);
		identifier->end = parser->cursor;

		return identifier;
	}

	return nullptr;
}

static OperatorType peekBinaryOperatorType(Parser* parser, int* numTokens)
{
	Token token = peekToken(parser);
	*numTokens = 1;

	if (token.type == TOKEN_PLUS)
	{
		return OPERATOR_ADD;
	}
	else if (token.type == TOKEN_MINUS)
	{
		return OPERATOR_SUBTRACT;
	}
	else if (token.type == TOKEN_ASTERISK)
	{
		return OPERATOR_MULTIPLY;
	}
	else if (token.type == TOKEN_SLASH)
	{
		return OPERATOR_DIVIDE;
	}
	else if (token.type == TOKEN_PERCENT)
	{
		return OPERATOR_MODULO;
	}
	else if (token.type == TOKEN_LESS_THAN)
	{
		if (peekToken(parser, 1).type == TOKEN_LESS_THAN)
		{
			*numTokens = 2;
			return OPERATOR_BITSHIFT_LEFT;
		}
		else
		{
			return OPERATOR_LESS;
		}
	}
	else if (token.type == TOKEN_GREATER_THAN)
	{
		if (peekToken(parser, 1).type == TOKEN_GREATER_THAN)
		{
			*numTokens = 2;
			return OPERATOR_BITSHIFT_RIGHT;
		}
		else
		{
			return OPERATOR_GREATER;
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
	case OPERATOR_SUBSCRIPT:
	case OPERATOR_MEMBER:
		return 1;

	case OPERATOR_INCREMENT_PREFIX:
	case OPERATOR_DECREMENT_PREFIX:
	case OPERATOR_PLUS_PREFIX:
	case OPERATOR_MINUS_PREFIX:
	case OPERATOR_LOGICAL_NOT:
	case OPERATOR_BITWISE_NOT:
	case OPERATOR_CAST:
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

	case OPERATOR_LESS:
	case OPERATOR_LESS_EQUALS:
	case OPERATOR_GREATER:
	case OPERATOR_GREATER_EQUALS:
		return 6;

	case OPERATOR_EQUALS:
	case OPERATOR_NOT_EQUALS:
		return 7;

	case OPERATOR_BITWISE_AND:
		return 8;

	case OPERATOR_BITWISE_XOR:
		return 9;

	case OPERATOR_BITWISE_OR:
		return 10;

	case OPERATOR_LOGICAL_AND:
		return 11;

	case OPERATOR_LOGICAL_OR:
		return 12;

	case OPERATOR_TERNARY_CONDITION:
		return 13;

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
		return 14;

	default:
		return 1000;
	}
}

Expression* parseBinaryOperator(Parser* parser, OperatorType lastOperatorType)
{
	if (Expression* left = parseAtom(parser))
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

				if (Expression* right = parseBinaryOperator(parser, operatorType))
				{
					BinaryOperator* op = parser->arena->alloc<BinaryOperator>();
					initNode((Node*)op, NODE_BINARY_OPERATOR, left->start);
					op->op = operatorType;
					op->left = left;
					op->right = right;
					op->end = parser->cursor;

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
	return parseBinaryOperator(parser, OPERATOR_NULL);
}

Statement* parseStatement(Parser* parser)
{
	int start = parser->cursor;

	if (nextIs(parser, '{'))
	{
		nextToken(parser);

		BlockStatement* block = parser->arena->alloc<BlockStatement>();
		initNode((Node*)block, NODE_BLOCK_STATEMENT, start);

		int mark = parser->scratch.mark();

		while (!nextIs(parser, '}'))
		{
			if (Statement* statement = parseStatement(parser))
			{
				parser->scratch.add(statement);
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

		nextToken(parser);

		block->statements = copyFromScratchBuffer<Statement*>(parser, mark, &block->numStatements);
		block->end = parser->cursor;

		return block;
	}
	if (Expression* expression = parseExpression(parser))
	{
		expectToken(parser, ';');

		ExpressionStatement* expressionStatement = parser->arena->alloc<ExpressionStatement>();
		initNode((Node*)expressionStatement, NODE_EXPRESSION_STATEMENT, start);
		expressionStatement->end = parser->cursor;
		expressionStatement->expression = expression;
		return expressionStatement;
	}

	return nullptr;
}

Field* parseField(Parser* parser)
{
	Type* type = parseType(parser);

	Token nameToken;
	if (!expectToken(parser, TOKEN_IDENTIFIER, &nameToken))
		return nullptr;

	if (!expectToken(parser, ';'))
	{
		skipPastToken(parser, ';');
		return nullptr;
	}

	int end = parser->lastTokenEnd;

	Field* field = parser->arena->alloc<Field>();
	initNode((Node*)field, NODE_FIELD, type->start);
	field->end = end;
	field->type = type;
	field->name = getTokenString(nameToken, parser);

	return field;
}

Struct* parseStruct(Parser* parser, uint32_t storage, int start)
{
	nextToken(parser); // struct

	Token identifier;
	if (!expectToken(parser, TOKEN_IDENTIFIER, &identifier))
		return nullptr;

	Struct* struct_ = parser->arena->alloc<Struct>();
	initNode((Node*)struct_, NODE_STRUCT, start);
	struct_->storage = storage;
	struct_->name = getTokenString(identifier, parser);

	if (nextIs(parser, ';'))
	{
		nextToken(parser);
	}
	else if (expectToken(parser, '{'))
	{
		int mark = parser->scratch.mark();

		while (!nextIs(parser, '}'))
		{
			if (Field* field = parseField(parser))
				parser->scratch.add((Node*)field);
		}

		struct_->fields = copyFromScratchBuffer<Field*>(parser, mark, &struct_->numFields);

		nextToken(parser);
	}

	struct_->end = parser->cursor;

	return struct_;
}

Enum* parseEnum(Parser* parser, uint32_t storage, int start)
{
	nextToken(parser); // enum

	Token identifier;
	if (!expectToken(parser, TOKEN_IDENTIFIER, &identifier))
		return nullptr;

	Enum* enum_ = parser->arena->alloc<Enum>();
	initNode((Node*)enum_, NODE_ENUM, start);
	enum_->storage = storage;
	enum_->name = getTokenString(identifier, parser);

	if (nextIs(parser, ';'))
	{
		nextToken(parser);
	}
	else if (expectToken(parser, '{'))
	{
		skipPastTokenNested(parser, '{', '}');
	}

	enum_->end = parser->cursor;

	return enum_;
}

Union* parseUnion(Parser* parser, uint32_t storage, int start)
{
	nextToken(parser); // union

	Token identifier;
	if (!expectToken(parser, TOKEN_IDENTIFIER, &identifier))
		return nullptr;

	Union* union_ = parser->arena->alloc<Union>();
	initNode((Node*)union_, NODE_UNION, start);
	union_->storage = storage;
	union_->name = getTokenString(identifier, parser);

	if (nextIs(parser, ';'))
	{
		nextToken(parser);
	}
	else if (expectToken(parser, '{'))
	{
		skipPastTokenNested(parser, '{', '}');
	}

	union_->end = parser->cursor;

	return union_;
}

Typedef* parseTypedef(Parser* parser, uint32_t storage, int start)
{
	nextToken(parser); // typedef

	Token identifier;
	if (!expectToken(parser, TOKEN_IDENTIFIER, &identifier))
		return nullptr;

	Typedef* typedef_ = parser->arena->alloc<Typedef>();
	initNode((Node*)typedef_, NODE_TYPEDEF, start);
	typedef_->storage = storage;
	typedef_->name = getTokenString(identifier, parser);

	if (nextIs(parser, ';'))
	{
		nextToken(parser);
	}
	else if (expectToken(parser, '{'))
	{
		skipPastTokenNested(parser, '{', '}');
	}

	typedef_->end = parser->cursor;

	return typedef_;
}

Parameter* parseParameter(Parser* parser)
{
	Type* type = parseType(parser);
	if (!type)
		return nullptr;

	Token nameToken;
	if (!expectToken(parser, TOKEN_IDENTIFIER, &nameToken))
		return nullptr;

	int end = parser->lastTokenEnd;

	Parameter* parameter = parser->arena->alloc<Parameter>();
	initNode((Node*)parameter, NODE_PARAMETER, type->start);
	parameter->end = end;
	parameter->type = type;
	parameter->name = getTokenString(nameToken, parser);

	return parameter;
}

Function* parseFunction(Parser* parser, uint32_t storage, int start)
{
	nextToken(parser); // func

	Token identifier;
	if (!expectToken(parser, TOKEN_IDENTIFIER, &identifier))
		return nullptr;

	if (!expectToken(parser, '('))
	{
		skipPastToken(parser, '{');
		skipPastTokenNested(parser, '{', '}');
		return nullptr;
	}

	Function* function = parser->arena->alloc<Function>();
	initNode((Node*)function, NODE_FUNCTION, start);
	function->storage = storage;
	function->name = getTokenString(identifier, parser);

	int mark = parser->scratch.mark();

	bool next = !nextIs(parser, ')');
	while (next)
	{
		if (Parameter* parameter = parseParameter(parser))
			parser->scratch.add((Node*)parameter);
		next = nextIs(parser, ',');
		if (next) nextToken(parser);
	}

	if (!expectToken(parser, ')'))
		skipPastToken(parser, ')');

	function->params = copyFromScratchBuffer<Parameter*>(parser, mark, &function->numParams);

	if (!nextIs(parser, '{'))
	{
		function->returnType = parseType(parser);
	}

	if (nextIs(parser, ';'))
	{
		nextToken(parser);
	}
	else
	{
		function->body = parseStatement(parser);
	}

	function->end = parser->cursor;

	return function;
}

GlobalVariable* parseGlobalVariable(Parser* parser, Type* type, uint32_t storage, int start)
{
	int mark = parser->scratch.mark();

	Token identifier;
	bool next = nextIs(parser, TOKEN_IDENTIFIER, &identifier);
	while (next)
	{
		if (!expectToken(parser, TOKEN_IDENTIFIER, &identifier))
		{
			parser->scratch.release(mark);
			skipPastToken(parser, ';');
			return nullptr;
		}

		VariableDeclarator declarator = {};
		declarator.name = getTokenString(identifier, parser);
		declarator.value = nullptr;

		if (nextIs(parser, TOKEN_EQUALS))
		{
			nextToken(parser);
			declarator.value = parseExpression(parser);

			if (!declarator.value)
			{
				error(parser, getSourceLocation(parser), "Expected an expression");
			}
		}

		parser->scratch.add(declarator);

		next = nextIs(parser, ',');
		if (next)
			nextToken(parser);
	}

	expectToken(parser, ';');

	int end = parser->lastTokenEnd;

	GlobalVariable* globalVariable = parser->arena->alloc<GlobalVariable>();
	initNode((Node*)globalVariable, NODE_GLOBAL_VARIABLE, start);
	globalVariable->storage = storage;
	globalVariable->type = type;
	globalVariable->end = end;

	globalVariable->declarators = copyFromScratchBuffer<VariableDeclarator>(parser, mark, &globalVariable->numDeclarators);

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

	macro->end = parser->cursor;

	return macro;
}

void parseFile(Parser* parser, AST* ast)
{
	int start = parser->cursor;

	int mark = parser->scratch.mark();

	Token token = {};
	while ((token = peekToken(parser)).type)
	{
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
				parser->scratch.add((Node*)struct_);
		}
		else if (token.type == TOKEN_ENUM)
		{
			if (Enum* enum_ = parseEnum(parser, storage, start))
				parser->scratch.add((Node*)enum_);
		}
		else if (token.type == TOKEN_UNION)
		{
			if (Union* union_ = parseUnion(parser, storage, start))
				parser->scratch.add((Node*)union_);
		}
		else if (token.type == TOKEN_TYPEDEF)
		{
			if (Typedef* typedef_ = parseTypedef(parser, storage, start))
				parser->scratch.add((Node*)typedef_);
		}
		else if (token.type == TOKEN_FUNCTION)
		{
			if (Function* function = parseFunction(parser, storage, start))
				parser->scratch.add((Node*)function);
		}
		else if (token.type == TOKEN_MACRO)
		{
			if (Macro* macro = parseMacro(parser, storage, start))
				parser->scratch.add((Node*)macro);
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
			skipPastToken(parser, ';');
		}
		else if (Type* type = parseType(parser))
		{
			if (GlobalVariable* globalVariable = parseGlobalVariable(parser, type, storage, start))
				parser->scratch.add((Node*)globalVariable);
		}
		else
		{
			skipPastToken(parser, ';');
		}
	}

	ast->declarations = copyFromScratchBuffer<Node*>(parser, mark, &ast->numDeclarations);
}

void parse(Parser* parser, AST* ast, Arena* arena)
{
	ast->globalScope = arena->alloc<Scope>();
	initScope(ast->globalScope, nullptr, true, arena);
	parser->currentScope = ast->globalScope;

	parseFile(parser, ast);

	for (int i = 0; i < ast->numDeclarations; i++)
	{
		Node* declaration = ast->declarations[i];

		StringView identifier = {};
		if (declaration->type == NODE_STRUCT)
			insertSymbol(&parser->currentScope->symbols, declaration->struct_.name, declaration);
		else if (declaration->type == NODE_ENUM)
			insertSymbol(&parser->currentScope->symbols, declaration->enum_.name, declaration);
		else if (declaration->type == NODE_UNION)
			insertSymbol(&parser->currentScope->symbols, declaration->union_.name, declaration);
		else if (declaration->type == NODE_TYPEDEF)
			insertSymbol(&parser->currentScope->symbols, declaration->typedef_.name, declaration);
		else if (declaration->type == NODE_FUNCTION)
			insertSymbol(&parser->currentScope->symbols, declaration->function.name, declaration);
		else if (declaration->type == NODE_GLOBAL_VARIABLE)
		{
			for (int i = 0; i < declaration->globalVariable.numDeclarators; i++)
			{
				insertSymbol(&parser->currentScope->symbols, declaration->globalVariable.declarators[i].name, declaration);
			}
		}
		else if (declaration->type == NODE_MACRO)
			insertSymbol(&parser->currentScope->symbols, declaration->macro.name, declaration);
	}
}
