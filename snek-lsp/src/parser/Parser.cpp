#include "Parser.h"

#include "Lexer.h"
#include "AST.h"

#include "utils/Log.h"
#include "utils/List.h"


struct Parser
{
	Lexer lexer;

	Token lookahead[3];
	int lookaheadCount;

	List<Node*> scratch;
};

static void initParser(Parser* parser, const char* filename, const char* src, int length)
{
	parser->lexer = {};
	initLexer(&parser->lexer, filename, src, length);

	parser->lookaheadCount = 0;
}

static Token nextToken(Parser* parser)
{
	if (parser->lookaheadCount > 0)
	{
		Token token = parser->lookahead[0];
		parser->lookahead[0] = parser->lookahead[1];
		parser->lookahead[1] = parser->lookahead[2];
		parser->lookaheadCount--;
		return token;
	}
	return nextToken(&parser->lexer);
}

static Token peekToken(Parser* parser, int count = 0)
{
	SnekAssert(count <= 3);
	while (parser->lookaheadCount <= count)
	{
		parser->lookahead[parser->lookaheadCount++] = nextToken(&parser->lexer);
	}
	return parser->lookahead[count];
}

Struct* parseStruct(Parser* parser)
{
	
}

Enum* parseEnum(Parser* parser)
{

}

Function* parseFunction(Parser* parser)
{

}

void parseFile(Parser* parser, AST* ast)
{
	int start = parser->scratch.size;

	Token token = {};
	while ((token = peekToken(parser)).type)
	{
		if (token.type == TOKEN_STRUCT)
		{
			parser->scratch.add((Node*)parseStruct(parser));
		}
		else if (token.type == TOKEN_ENUM)
		{
			parser->scratch.add((Node*)parseEnum(parser));
		}
		else if (token.type == TOKEN_FUNCTION)
		{
			parser->scratch.add((Node*)parseFunction(parser));
		}
		else
		{
			nextToken(parser);
		}
	}

	int count = parser->scratch.size - start;
	if (count > 0)
	{
		ast->declarations = ast->arena.alloc<Node*>(count);
		memcpy(ast->declarations, &parser->scratch[start], count * sizeof(Node*));
	}

	parser->scratch.resize(start);
}

void parse(AST* ast, const char* filename, const char* src, int length)
{
	Parser parser = {};
	initParser(&parser, filename, src, length);

	initAST(ast);

	parseFile(&parser, ast);
}
