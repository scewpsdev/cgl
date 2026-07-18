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

static bool nextIs(Parser* parser, int type)
{
	return peekToken(parser).type == type;
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

Struct* parseStruct(Parser* parser)
{
	SourceLocation start = getSourceLocation(parser);

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

	struct_->end = getSourceLocation(parser);

	return struct_;
}

Enum* parseEnum(Parser* parser)
{
	SourceLocation start = getSourceLocation(parser);

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

	enum_->end = getSourceLocation(parser);

	return enum_;
}

Union* parseUnion(Parser* parser)
{
	SourceLocation start = getSourceLocation(parser);

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

	union_->end = getSourceLocation(parser);

	return union_;
}

Typedef* parseTypedef(Parser* parser)
{
	SourceLocation start = getSourceLocation(parser);

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

	typedef_->end = getSourceLocation(parser);

	return typedef_;
}

Function* parseFunction(Parser* parser)
{
	SourceLocation start = getSourceLocation(parser);

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
	while (nextIs(parser, TOKEN_IDENTIFIER) || nextIsKeyword(parser) || nextIs(parser, TOKEN_OP_ASTERISK))
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

	function->end = getSourceLocation(parser);

	return function;
}

Macro* parseMacro(Parser* parser)
{
	SourceLocation start = getSourceLocation(parser);

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

	macro->end = getSourceLocation(parser);

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

	parseFile(&parser, ast);

	initSymbolTable(&ast->symbols, ast->numDeclarations, arena);
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
			insertSymbol(&ast->symbols, identifier, declaration);
	}
}
