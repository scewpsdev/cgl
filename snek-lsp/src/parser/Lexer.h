#pragma once

#include "utils/StringView.h"
#include "utils/List.h"

#include <string.h>


enum TokenType : int
{
	TOKEN_NULL = 0,

	TOKEN_FIRST = 256,

	TOKEN_STRING_LITERAL,
	TOKEN_STRING_LITERAL_MULTILINE,
	TOKEN_CHAR_LITERAL,
	TOKEN_FLOAT_LITERAL,
	TOKEN_DOUBLE_LITERAL,
	TOKEN_INT_LITERAL,


	TOKEN_OP_BEGIN,

	TOKEN_PLUS,
	TOKEN_MINUS,
	TOKEN_ASTERISK,
	TOKEN_SLASH,
	TOKEN_PERCENT,
	TOKEN_AMPERSAND,
	TOKEN_OR,
	TOKEN_CARET,
	TOKEN_QUESTION,
	TOKEN_EXCLAMATION,
	TOKEN_EQUALS,
	TOKEN_LESS_THAN,
	TOKEN_GREATER_THAN,

	TOKEN_OP_END,


	TOKEN_KEYWORD_BEGIN,

	TOKEN_MODULE,
	TOKEN_NAMESPACE,
	TOKEN_IMPORT,
	TOKEN_VARIABLE,
	TOKEN_LET,
	TOKEN_FUNCTION,
	TOKEN_STRUCT,
	TOKEN_CLASS,
	TOKEN_TYPEDEF,
	TOKEN_MACRO,
	TOKEN_METHOD,
	TOKEN_ENUM,
	TOKEN_UNION,

	TOKEN_PUBLIC,
	TOKEN_PRIVATE,
	TOKEN_STATIC,
	TOKEN_CONSTANT,
	TOKEN_EXTERN,
	TOKEN_DLLEXPORT,
	TOKEN_DLLIMPORT,
	TOKEN_PACKED,
	TOKEN_NOMANGLE,

	TOKEN_IF,
	TOKEN_ELSE,
	TOKEN_FOR,
	TOKEN_WHILE,
	TOKEN_RETURN,
	TOKEN_BREAK,
	TOKEN_CONTINUE,
	TOKEN_DEFER,
	TOKEN_ASSERT,

	TOKEN_AS,
	TOKEN_SIZEOF,
	TOKEN_ALLOCA,
	TOKEN_MALLOC,
	TOKEN_STACKNEW,
	TOKEN_FREE,

	TOKEN_TRUE,
	TOKEN_FALSE,
	TOKEN_NULL_KEYWORD,

	TOKEN_VOID,

	TOKEN_INT8,
	TOKEN_INT16,
	TOKEN_INT32,
	TOKEN_INT64,

	TOKEN_UINT8,
	TOKEN_UINT16,
	TOKEN_UINT32,
	TOKEN_UINT64,

	TOKEN_BOOL,
	TOKEN_STRING,

	TOKEN_FLOAT32,
	TOKEN_FLOAT64,

	TOKEN_ANY,

	TOKEN_KEYWORD_END,


	TOKEN_IDENTIFIER,


	TOKEN_COUNT,
};

struct Token
{
	TokenType type;
	int offset;
	int length;
};

struct SourceLocation
{
	const char* filename;
	int line, col;
};

struct Arena;
struct Diagnostics;

struct Lexer
{
	const char* filename;
	const char* src;
	int length;
	int cursor;

	List<int> lineOffsets;

	Arena* arena;
	Diagnostics* diagnostics;
};


void initLexer(Lexer* lexer, const char* filename, const char* src, int length, Arena* arena, Diagnostics* diagnostics);
SourceLocation getSourceLocation(Lexer* lexer, int offset);
StringView getTokenString(Token token, const char* src);

Token nextToken(Lexer* lexer);
