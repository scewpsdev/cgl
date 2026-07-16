#include "Parser.h"

#include "Lexer.h"
#include "AST.h"

#include "utils/Log.h"
#include "utils/Arena.h"
#include "utils/List.h"


struct Parser
{
	Lexer lexer;
	LexerState state;

	Token lookahead[3];
	LexerState lookaheadState[3];
	int lookaheadCount;

	Arena* arena;
	List<Node*> scratch;
};

static void initParser(Parser* parser, const char* filename, const char* src, int length)
{
	parser->lexer = {};
	initLexer(&parser->lexer, filename, src, length);

	parser->lookaheadCount = 0;
}

static SourceLocation getSourceLocation(Parser* parser)
{
	return { parser->lexer.filename, parser->state.line, parser->state.col };
}

static Token nextToken(Parser* parser)
{
	if (parser->lookaheadCount > 0)
	{
		Token token = parser->lookahead[0];
		parser->lookahead[0] = parser->lookahead[1];
		parser->lookahead[1] = parser->lookahead[2];

		LexerState state = parser->lookaheadState[0];
		parser->lookaheadState[0] = parser->lookaheadState[1];
		parser->lookaheadState[1] = parser->lookaheadState[2];

		parser->lookaheadCount--;
		parser->state = state;
		return token;
	}

	Token token = nextToken(&parser->lexer);
	parser->state = parser->lexer.state;
	return token;
}

static Token peekToken(Parser* parser, int count = 0)
{
	SnekAssert(count <= 3);
	while (parser->lookaheadCount <= count)
	{
		parser->lookahead[parser->lookaheadCount] = nextToken(&parser->lexer);
		parser->lookaheadState[parser->lookaheadCount] = parser->lexer.state;
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

	// todo use token type string
	SnekErrorLoc(getSourceLocation(parser), "Expected token %d", (int)type);
	return false;
}

static bool nextIs(Parser* parser, int type)
{
	return peekToken(parser).type == type;
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
	struct_->name = identifier.text;

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
	enum_->name = identifier.text;

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
	union_->name = identifier.text;

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
	typedef_->name = identifier.text;

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
	function->name = identifier.text;

	if (expectToken(parser, '('))
	{
		skipPastTokenNested(parser, '(', ')');
	}

	// skip return type
	while (nextIs(parser, TOKEN_IDENTIFIER) || nextIs(parser, TOKEN_OP_ASTERISK))
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
	macro->name = identifier.text;

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
			parser->scratch.add((Node*)parseStruct(parser));
		}
		else if (token.type == TOKEN_ENUM)
		{
			parser->scratch.add((Node*)parseEnum(parser));
		}
		else if (token.type == TOKEN_UNION)
		{
			parser->scratch.add((Node*)parseUnion(parser));
		}
		else if (token.type == TOKEN_TYPEDEF)
		{
			parser->scratch.add((Node*)parseTypedef(parser));
		}
		else if (token.type == TOKEN_FUNCTION)
		{
			parser->scratch.add((Node*)parseFunction(parser));
		}
		else if (token.type == TOKEN_MACRO)
		{
			parser->scratch.add((Node*)parseMacro(parser));
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
		ast->declarations = parser->arena->alloc<Node*>(count);
		memcpy(ast->declarations, &parser->scratch[start], count * sizeof(Node*));
	}

	parser->scratch.resize(start);
}

void parse(AST* ast, const char* filename, const char* src, int length)
{
	Parser parser = {};
	initParser(&parser, filename, src, length);

	initAST(ast);

	parser.arena = &ast->arena;

	parseFile(&parser, ast);

	initSymbolTable(&ast->symbols, ast->numDeclarations, &ast->arena);
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
