#include "Parser.h"

#include "Lexer.h"
#include "AST.h"
#include "Diagnostics.h"

#include "utils/Log.h"
#include "utils/Arena.h"
#include "utils/List.h"

#include <stdarg.h>


struct Parser
{
	Lexer lexer;
	int cursor;

	Token lookahead[3];
	int lookaheadState[3];
	int lookaheadCount;

	Scope* currentScope;

	Arena* arena;
	List<Node*> scratch;

	Diagnostics* diagnostics;
};

static void initParser(Parser* parser, const char* filename, const char* src, int length, Arena* arena, Diagnostics* diagnostics)
{
	parser->lexer = {};
	initLexer(&parser->lexer, filename, src, length, arena, diagnostics);

	parser->arena = arena;
	parser->diagnostics = diagnostics;
	parser->lookaheadCount = 0;
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

static SourceLocation getSourceLocation(Parser* parser)
{
	return getSourceLocation(&parser->lexer, parser->cursor);
}

static SourceLocation getSourceLocation(Parser* parser, Token token)
{
	return getSourceLocation(&parser->lexer, token.offset);
}

static void getSourceLocation(Parser* parser, Token token, SourceLocation* start, SourceLocation* end)
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
		return token;
	}

	Token token = nextToken(&parser->lexer);
	parser->cursor = parser->lexer.cursor;
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
	if (outToken) *outToken = token;

	if (token.type == type)
	{
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

static Type* parseBasicType(Parser* parser)
{
	Token token = peekToken(parser);
	int start, end;
	getTokenRange(token, &start, &end);

	if (token.type == TOKEN_VOID)
	{
		nextToken(parser);

		Type* type = parser->arena->alloc<Type>();
		initType(type, TYPE_VOID, start);
		type->end = end;
		return type;
	}
	else if (token.type == TOKEN_INT8 || token.type == TOKEN_INT16 || token.type == TOKEN_INT32 || token.type == TOKEN_INT64 || token.type == TOKEN_INT128)
	{
		nextToken(parser);

		const int BITWIDTHS[] = { 8, 16, 32, 64, 128 };

		IntegerType* type = parser->arena->alloc<IntegerType>();
		initType((Type*)type, TYPE_INTEGER, start);
		type->end = end;
		type->bitWidth = BITWIDTHS[token.type - TOKEN_INT8];
		type->isSigned = true;
		return type;
	}
	else if (token.type == TOKEN_UINT8 || token.type == TOKEN_UINT16 || token.type == TOKEN_UINT32 || token.type == TOKEN_UINT64 || token.type == TOKEN_UINT128)
	{
		nextToken(parser);

		const int BITWIDTHS[] = { 8, 16, 32, 64, 128 };

		IntegerType* type = parser->arena->alloc<IntegerType>();
		initType((Type*)type, TYPE_INTEGER, start);
		type->end = end;
		type->bitWidth = BITWIDTHS[token.type - TOKEN_UINT8];
		type->isSigned = false;
		return type;
	}
	else if (token.type == TOKEN_FLOAT16 || token.type == TOKEN_FLOAT32 || token.type == TOKEN_FLOAT64 || token.type == TOKEN_FLOAT80 || token.type == TOKEN_FLOAT128)
	{
		nextToken(parser);

		const int BITWIDTHS[] = { 16, 32, 64, 80, 128 };

		FloatType* type = parser->arena->alloc<FloatType>();
		initType((Type*)type, TYPE_FLOAT, start);
		type->end = end;
		type->bitWidth = BITWIDTHS[token.type - TOKEN_FLOAT16];
		return type;
	}
	else if (token.type == TOKEN_BOOL)
	{
		nextToken(parser);

		Type* type = parser->arena->alloc<Type>();
		initType(type, TYPE_BOOL, start);
		type->end = end;
		return type;
	}
	else if (token.type == TOKEN_ANY)
	{
		nextToken(parser);

		Type* type = parser->arena->alloc<Type>();
		initType(type, TYPE_ANY, start);
		type->end = end;
		return type;
	}
	else if (token.type == TOKEN_STRING)
	{
		nextToken(parser);

		Type* type = parser->arena->alloc<Type>();
		initType(type, TYPE_STRING, start);
		type->end = end;
		return type;
	}
	else if (token.type == TOKEN_IDENTIFIER)
	{
		nextToken(parser);

		NamedType* type = parser->arena->alloc<NamedType>();
		initType((Type*)type, TYPE_NAMED, start);
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
			PointerType* type = parser->arena->alloc<PointerType>();
			initType((Type*)type, TYPE_POINTER, basicType->start);
			type->end = token.offset + token.length;
			type->elementType = basicType;
			return type;
		}
		else if (nextIs(parser, TOKEN_QUESTION, &token) && token.offset == basicType->end)
		{
			OptionalType* type = parser->arena->alloc<OptionalType>();
			initType((Type*)type, TYPE_OPTIONAL, basicType->start);
			type->end = token.offset + token.length;
			type->elementType = basicType;
			return type;
		}
		else
		{
			return basicType;
		}
	}
	return nullptr;
}

Struct* parseStruct(Parser* parser)
{
	int start = parser->cursor;

	nextToken(parser); // struct

	Token identifier;
	if (!expectToken(parser, TOKEN_IDENTIFIER, &identifier))
		return nullptr;

	Struct* struct_ = parser->arena->alloc<Struct>();
	initNode((Node*)struct_, NODE_STRUCT, start);
	struct_->name = getTokenString(identifier, parser);

	if (nextIs(parser, ';'))
	{
		nextToken(parser);
	}
	else if (expectToken(parser, '{'))
	{
		skipPastTokenNested(parser, '{', '}');
	}

	struct_->end = parser->cursor;

	return struct_;
}

Enum* parseEnum(Parser* parser)
{
	int start = parser->cursor;

	nextToken(parser); // enum

	Token identifier;
	if (!expectToken(parser, TOKEN_IDENTIFIER, &identifier))
		return nullptr;

	Enum* enum_ = parser->arena->alloc<Enum>();
	initNode((Node*)enum_, NODE_ENUM, start);
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

Union* parseUnion(Parser* parser)
{
	int start = parser->cursor;

	nextToken(parser); // union

	Token identifier;
	if (!expectToken(parser, TOKEN_IDENTIFIER, &identifier))
		return nullptr;

	Union* union_ = parser->arena->alloc<Union>();
	initNode((Node*)union_, NODE_UNION, start);
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

Typedef* parseTypedef(Parser* parser)
{
	int start = parser->cursor;

	nextToken(parser); // typedef

	Token identifier;
	if (!expectToken(parser, TOKEN_IDENTIFIER, &identifier))
		return nullptr;

	Typedef* typedef_ = parser->arena->alloc<Typedef>();
	initNode((Node*)typedef_, NODE_TYPEDEF, start);
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

Function* parseFunction(Parser* parser)
{
	int start = parser->cursor;

	nextToken(parser); // func

	Token identifier;
	if (!expectToken(parser, TOKEN_IDENTIFIER, &identifier))
		return nullptr;

	Function* function = parser->arena->alloc<Function>();
	initNode((Node*)function, NODE_FUNCTION, start);
	function->name = getTokenString(identifier, parser);

	if (expectToken(parser, '('))
	{
		skipPastTokenNested(parser, '(', ')');
	}

	// skip return type
	while (nextIs(parser, TOKEN_IDENTIFIER) || nextIsKeyword(parser) || nextIs(parser, TOKEN_ASTERISK))
	{
		nextToken(parser);
	}

	if (nextIs(parser, ';'))
	{
		nextToken(parser);
	}
	else if (expectToken(parser, '{'))
	{
		skipPastTokenNested(parser, '{', '}');
	}

	function->end = parser->cursor;

	return function;
}

Macro* parseMacro(Parser* parser)
{
	int start = parser->cursor;

	nextToken(parser); // macro

	Token identifier;
	if (!expectToken(parser, TOKEN_IDENTIFIER, &identifier))
		return nullptr;

	Macro* macro = parser->arena->alloc<Macro>();
	initNode((Node*)macro, NODE_MACRO, start);
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
	int start = parser->scratch.size;

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
			if (Struct* struct_ = parseStruct(parser))
				parser->scratch.add((Node*)struct_);
		}
		else if (token.type == TOKEN_ENUM)
		{
			if (Enum* enum_ = parseEnum(parser))
				parser->scratch.add((Node*)enum_);
		}
		else if (token.type == TOKEN_UNION)
		{
			if (Union* union_ = parseUnion(parser))
				parser->scratch.add((Node*)union_);
		}
		else if (token.type == TOKEN_TYPEDEF)
		{
			if (Typedef* typedef_ = parseTypedef(parser))
				parser->scratch.add((Node*)typedef_);
		}
		else if (token.type == TOKEN_FUNCTION)
		{
			if (Function* function = parseFunction(parser))
				parser->scratch.add((Node*)function);
		}
		else if (token.type == TOKEN_MACRO)
		{
			if (Macro* macro = parseMacro(parser))
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
		else
		{
			skipPastToken(parser, ';');
		}
	}

	int count = parser->scratch.size - start;
	if (count > 0)
	{
		ast->numDeclarations = count;
		ast->declarations = parser->arena->alloc<Node*>(count);
		memcpy(ast->declarations, &parser->scratch[start], count * sizeof(Node*));
	}

	parser->scratch.resize(start);
}

void parse(AST* ast, Arena* arena, Diagnostics* diagnostics, const char* filename, const char* src, int length)
{
	Parser parser = {};
	initParser(&parser, filename, src, length, arena, diagnostics);

	ast->globalScope = arena->alloc<Scope>();
	initScope(ast->globalScope, nullptr, true, arena);
	parser.currentScope = ast->globalScope;

	parseFile(&parser, ast);

	for (int i = 0; i < ast->numDeclarations; i++)
	{
		Node* declaration = ast->declarations[i];

		StringView identifier = {};
		if (declaration->type == NODE_STRUCT)
			identifier = declaration->struct_.name;
		else if (declaration->type == NODE_ENUM)
			identifier = declaration->enum_.name;
		else if (declaration->type == NODE_UNION)
			identifier = declaration->union_.name;
		else if (declaration->type == NODE_TYPEDEF)
			identifier = declaration->typedef_.name;
		else if (declaration->type == NODE_FUNCTION)
			identifier = declaration->function.name;
		else if (declaration->type == NODE_MACRO)
			identifier = declaration->macro.name;

		if (identifier.ptr)
			insertSymbol(&parser.currentScope->symbols, identifier, declaration);
	}
}
