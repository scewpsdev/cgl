#include "pch.h"
#include "Lexer.h"

#include "Keywords.h"
#include "cgl/utils/Log.h"

#include <assert.h>
#include <ctype.h>
#include <map>
#include <string>


static const Token NULL_TOKEN = {};

const std::map<std::string, KeywordType> keywords =
{
	{ KEYWORD_MODULE, KEYWORD_TYPE_MODULE },
	{ KEYWORD_NAMESPACE, KEYWORD_TYPE_NAMESPACE },
	{ KEYWORD_IMPORT, KEYWORD_TYPE_IMPORT },
	{ KEYWORD_VAR, KEYWORD_TYPE_VARIABLE },
	{ KEYWORD_LET, KEYWORD_TYPE_LET },
	{ KEYWORD_FUNC, KEYWORD_TYPE_FUNCTION },
	{ KEYWORD_STRUCT, KEYWORD_TYPE_STRUCT },
	{ KEYWORD_CLASS, KEYWORD_TYPE_CLASS },
	{ KEYWORD_TYPEDEF, KEYWORD_TYPE_TYPEDEF },
	{ KEYWORD_MACRO, KEYWORD_TYPE_MACRO },
	{ KEYWORD_METHOD, KEYWORD_TYPE_METHOD },
	{ KEYWORD_ENUM, KEYWORD_TYPE_ENUM },
	{ KEYWORD_UNION, KEYWORD_TYPE_UNION },

	{ KEYWORD_PUBLIC, KEYWORD_TYPE_PUBLIC },
	{ KEYWORD_PRIVATE, KEYWORD_TYPE_PRIVATE },
	{ KEYWORD_STATIC, KEYWORD_TYPE_STATIC },
	{ KEYWORD_CONST, KEYWORD_TYPE_CONSTANT },
	{ KEYWORD_EXTERN, KEYWORD_TYPE_EXTERN },
	{ KEYWORD_DLLEXPORT, KEYWORD_TYPE_DLLEXPORT },
	{ KEYWORD_DLLIMPORT, KEYWORD_TYPE_DLLIMPORT },
	{ KEYWORD_PACKED, KEYWORD_TYPE_PACKED },
	{ KEYWORD_NOMANGLE, KEYWORD_TYPE_NOMANGLE },

	{ KEYWORD_IF, KEYWORD_TYPE_IF },
	{ KEYWORD_ELSE, KEYWORD_TYPE_ELSE },
	{ KEYWORD_FOR, KEYWORD_TYPE_FOR },
	{ KEYWORD_WHILE, KEYWORD_TYPE_WHILE },
	{ KEYWORD_RETURN, KEYWORD_TYPE_RETURN },
	{ KEYWORD_BREAK, KEYWORD_TYPE_BREAK },
	{ KEYWORD_CONTINUE, KEYWORD_TYPE_CONTINUE },
	{ KEYWORD_DEFER, KEYWORD_TYPE_DEFER },
	{ KEYWORD_ASSERT, KEYWORD_TYPE_ASSERT},

	{ KEYWORD_AS, KEYWORD_TYPE_AS },
	{ KEYWORD_SIZEOF, KEYWORD_TYPE_SIZEOF },
	{ KEYWORD_ALLOCA, KEYWORD_TYPE_ALLOCA },
	{ KEYWORD_MALLOC, KEYWORD_TYPE_MALLOC },
	{ KEYWORD_STACKNEW, KEYWORD_TYPE_STACKNEW },
	{ KEYWORD_FREE, KEYWORD_TYPE_FREE },

	{ KEYWORD_TRUE, KEYWORD_TYPE_TRUE },
	{ KEYWORD_FALSE, KEYWORD_TYPE_FALSE },
	{ KEYWORD_NULL, KEYWORD_TYPE_NULL_KEYWORD },

	{ KEYWORD_VOID, KEYWORD_TYPE_VOID },

	{ KEYWORD_INT8, KEYWORD_TYPE_INT8 },
	{ KEYWORD_INT16, KEYWORD_TYPE_INT16 },
	{ KEYWORD_INT32, KEYWORD_TYPE_INT32 },
	{ KEYWORD_INT64, KEYWORD_TYPE_INT64 },
	{ KEYWORD_INT128, KEYWORD_TYPE_INT128 },

	{ KEYWORD_CHAR, KEYWORD_TYPE_INT8 },
	{ KEYWORD_SHORT, KEYWORD_TYPE_INT16 },
	{ KEYWORD_INT, KEYWORD_TYPE_INT32 },
	{ KEYWORD_LONG, KEYWORD_TYPE_INT64 },

	{ KEYWORD_UINT8, KEYWORD_TYPE_UINT8 },
	{ KEYWORD_UINT16, KEYWORD_TYPE_UINT16 },
	{ KEYWORD_UINT32, KEYWORD_TYPE_UINT32 },
	{ KEYWORD_UINT64, KEYWORD_TYPE_UINT64 },
	{ KEYWORD_UINT128, KEYWORD_TYPE_UINT128 },

	{ KEYWORD_BYTE, KEYWORD_TYPE_UINT8 },
	{ KEYWORD_UCHAR, KEYWORD_TYPE_UINT8 },
	{ KEYWORD_USHORT, KEYWORD_TYPE_UINT16 },
	{ KEYWORD_UINT, KEYWORD_TYPE_UINT32 },
	{ KEYWORD_ULONG, KEYWORD_TYPE_UINT64 },

	{ KEYWORD_BOOL, KEYWORD_TYPE_BOOL },

	{ KEYWORD_FLOAT16, KEYWORD_TYPE_FLOAT16 },
	{ KEYWORD_FLOAT32, KEYWORD_TYPE_FLOAT32 },
	{ KEYWORD_FLOAT64, KEYWORD_TYPE_FLOAT64 },
	{ KEYWORD_FLOAT80, KEYWORD_TYPE_FLOAT80 },
	{ KEYWORD_FLOAT128, KEYWORD_TYPE_FLOAT128 },

	{ KEYWORD_HALF, KEYWORD_TYPE_FLOAT16 },
	{ KEYWORD_FLOAT, KEYWORD_TYPE_FLOAT32 },
	{ KEYWORD_DOUBLE, KEYWORD_TYPE_FLOAT64 },
	{ KEYWORD_DECIMAL, KEYWORD_TYPE_FLOAT80 },
	{ KEYWORD_QUAD, KEYWORD_TYPE_FLOAT128 },

	{ KEYWORD_ANY, KEYWORD_TYPE_ANY },

	{ KEYWORD_STRING, KEYWORD_TYPE_STRING },
	{ KEYWORD_PTR, KEYWORD_TYPE_PTR },
	{ KEYWORD_ARRAY, KEYWORD_TYPE_ARRAY },
	{ KEYWORD_FUNC_TYPE, KEYWORD_TYPE_FUNC_TYPE },
};


Lexer::Lexer(CGLCompiler* context, const char* filename, const char* src)
	: context(context), filename(filename)
{
	input = CreateInput(src, filename);
}

Lexer::~Lexer()
{
	DestroyInput(&input);
}

// replacement for isalpha because that one crashes when passing ö
bool isAlpha(char c)
{
	return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
}

bool isDigit(char c)
{
	return c >= '0' && c <= '9';
}

static bool nextIsWhitespace(Lexer* lexer)
{
	char c = InputPeek(&lexer->input, 0);
	return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\b';
}

static bool nextIsComment(Lexer* lexer)
{
	return InputPeek(&lexer->input, 0) == '/' && (InputPeek(&lexer->input, 1) == '/' || InputPeek(&lexer->input, 1) == '*');
}

static bool nextIsStringLiteral(Lexer* lexer)
{
	return InputPeek(&lexer->input, 0) == '"';
}

static bool nextIsCharLiteral(Lexer* lexer)
{
	return InputPeek(&lexer->input, 0) == '\'';
}

static bool nextIsNumberLiteral(Lexer* lexer)
{
	char c1 = InputPeek(&lexer->input, 0);
	char c2 = InputPeek(&lexer->input, 1);
	return isDigit(c1) || (c1 == '-' && isDigit(c2));
}

static bool nextIsPunctuation(Lexer* lexer)
{
	char c = InputPeek(&lexer->input, 0);
	return c == '.'
		|| c == ','
		|| c == ':'
		|| c == ';'
		|| c == '#'
		|| c == '('
		|| c == ')'
		|| c == '{'
		|| c == '}'
		|| c == '['
		|| c == ']';
}

static bool nextIsOperator(Lexer* lexer)
{
	char c = InputPeek(&lexer->input, 0);
	return c == '+'
		|| c == '-'
		|| c == '*'
		|| c == '/'
		|| c == '%'
		|| c == '&'
		|| c == '|'
		|| c == '^'
		|| c == '<'
		|| c == '>'
		|| c == '!'
		|| c == '?'
		|| c == '$'
		|| c == '=';
}

static bool nextIsIdentifier(Lexer* lexer)
{
	char c = InputPeek(&lexer->input, 0);
	return isAlpha(c) || c == '_';
}

static void skipComment(Lexer* lexer)
{
	InputNext(&lexer->input);
	char c2 = InputNext(&lexer->input);
	if (c2 == '/')
		while (InputNext(&lexer->input) != '\n' && InputHasNext(&lexer->input));
	else if (c2 == '*')
	{
		int level = 1;
		while (level > 0 && InputHasNext(&lexer->input))
		{
			char c = InputNext(&lexer->input);
			if (c == '/' && InputNext(&lexer->input) == '*')
				level++;
			else if (c == '*' && InputNext(&lexer->input) == '/')
				level--;
		}
	}
	else
		assert(false && "SkipComment error");
}

static void skipWhitespace(Lexer* lexer)
{
	while (nextIsWhitespace(lexer) || nextIsComment(lexer))
	{
		if (nextIsComment(lexer))
			skipComment(lexer);
		else
			InputNext(&lexer->input);
	}
}

static Token readStringLiteral(Lexer* lexer)
{
	Token token = {};
	token.line = lexer->input.state.line;
	token.col = lexer->input.state.col;

	InputState inputState = lexer->input.state;

	bool multiline = false;
	if (InputPeek(&lexer->input, 1) == '"' && InputPeek(&lexer->input, 2) == '"')
		multiline = true;

	if (multiline)
	{
		token.type = TOKEN_TYPE_STRING_LITERAL_MULTILINE;

		token.str = &lexer->input.state.buffer[lexer->input.state.index];

		InputNext(&lexer->input); // "
		InputNext(&lexer->input); // "
		InputNext(&lexer->input); // "
		token.len += 3; // """

		char c;
		do
		{
			c = InputNext(&lexer->input);
		} while (c != '\n');

		InputState lastLineState = lexer->input.state;
		int lastLineTokenLen = 0;

		while (InputHasNext(&lexer->input))
		{
			if (InputPeek(&lexer->input, 0) == '\n' || InputPeek(&lexer->input, 0) == '\r')
			{
				lastLineState = lexer->input.state;
				lastLineTokenLen = token.len;
			}

			char c = InputNext(&lexer->input);

			if (c == '"' && InputPeek(&lexer->input, 0) == '"' && InputPeek(&lexer->input, 1) == '"')
			{
				InputNext(&lexer->input); // "
				InputNext(&lexer->input); // "

				token.len = lastLineTokenLen;
				return token;
			}
			else
			{
				token.len++;
			}
		}


		SnekErrorLoc(lexer->context, inputState, "Unterminated multiline string literal");
		return {};
	}
	else
	{
		token.type = TOKEN_TYPE_STRING_LITERAL;

		token.str = &lexer->input.state.buffer[lexer->input.state.index];

		InputNext(&lexer->input);
		token.len++; // "

		char c;
		while ((c = InputNext(&lexer->input)) != '"')
		{
			if (!InputHasNext(&lexer->input))
			{
				SnekErrorLoc(lexer->context, inputState, "Unterminated string literal");
				return {};
			}
			token.len++;
			if (c == '\\')
			{
				InputNext(&lexer->input);
				token.len++;
			}
		}

		token.len++; // "

		return token;
	}
}

static Token readCharLiteral(Lexer* lexer)
{
	Token token;
	token.type = TOKEN_TYPE_CHAR_LITERAL;
	token.line = lexer->input.state.line;
	token.col = lexer->input.state.col;

	InputNext(&lexer->input);
	token.str = &lexer->input.state.buffer[lexer->input.state.index];
	token.len = 0;

	char c;
	while ((c = InputNext(&lexer->input)) != '\'')
	{
		token.len++;
		if (c == '\\')
		{
			InputNext(&lexer->input);
			token.len++;
		}
	}

	return token;
}

static Token readNumberLiteral(Lexer* lexer)
{
	Token token;
	// token.type
	token.line = lexer->input.state.line;
	token.col = lexer->input.state.col;

	token.str = &lexer->input.state.buffer[lexer->input.state.index];
	token.len = 0;

	bool fp = false;
	bool isDouble = true;
	bool hex = false;

	if (InputPeek(&lexer->input, 0) == '-')
	{
		InputNext(&lexer->input);
		token.len++;
	}

	char c = tolower(InputPeek(&lexer->input, 0));
	char c2 = tolower(InputPeek(&lexer->input, 1));
	for (;
		isDigit(c) ||
		(!fp && c == '.' && (isAlpha(c2) || isDigit(c2))) ||
		c == 'x' || c == 'b' || c == 'o' ||
		(c >= 'a' && c <= 'f') ||
		c == 'e' || c == '-' ||
		c == '_' ||
		c == 'u' ||
		c == 'l';
		)
	{
		InputNext(&lexer->input);
		if (c == 'x')
			hex = true;
		if (c == '.' || (!hex && c == 'f'))
			fp = true;
		if (!hex && c == 'f')
			isDouble = false;
		token.len++;

		c = tolower(InputPeek(&lexer->input, 0));
		c2 = tolower(InputPeek(&lexer->input, 1));
	}

	token.type = fp ? isDouble ? TOKEN_TYPE_DOUBLE_LITERAL : TOKEN_TYPE_FLOAT_LITERAL : TOKEN_TYPE_INT_LITERAL;

	return token;
}

static Token readPunctuation(Lexer* lexer)
{
	InputState state = lexer->input.state;

	Token token = {};
	token.type = (TokenType)InputNext(&lexer->input);
	token.line = state.line;
	token.col = state.col;

	token.str = state.ptr; // &lexer->input.state.buffer[lexer->input.state.index];
	token.len = 1;

	return token;
}

static Token readOperator(Lexer* lexer)
{
	InputState state = lexer->input.state;

	Token token = {};
	token.line = lexer->input.state.line;
	token.col = lexer->input.state.col;

	token.str = state.ptr;
	token.len = 1;

	switch (InputNext(&lexer->input))
	{
	case '+': token.type = TOKEN_TYPE_OP_PLUS; break;
	case '-': token.type = TOKEN_TYPE_OP_MINUS; break;
	case '*': token.type = TOKEN_TYPE_OP_ASTERISK; break;
	case '/': token.type = TOKEN_TYPE_OP_SLASH; break;
	case '%': token.type = TOKEN_TYPE_OP_PERCENT; break;
	case '&': token.type = TOKEN_TYPE_OP_AMPERSAND; break;
	case '|': token.type = TOKEN_TYPE_OP_OR; break;
	case '^': token.type = TOKEN_TYPE_OP_CARET; break;
	case '?': token.type = TOKEN_TYPE_OP_QUESTION; break;
	case '!': token.type = TOKEN_TYPE_OP_EXCLAMATION; break;
	case '=': token.type = TOKEN_TYPE_OP_EQUALS; break;
	case '<': token.type = TOKEN_TYPE_OP_LESS_THAN; break;
	case '>': token.type = TOKEN_TYPE_OP_GREATER_THAN; break;
	default:
		assert(false && "ReadOperator");
		break;
	}

	return token;
}

KeywordType getKeywordType(const char* str, int len)
{
	for (const auto& pair : keywords) {
		if (pair.first.length() == len && strncmp(pair.first.c_str(), str, len) == 0)
			return pair.second;
	}
	return KEYWORD_TYPE_NULL;
}

static Token readIdentifier(Lexer* lexer)
{
	Token token;
	token.type = TOKEN_TYPE_IDENTIFIER;
	token.line = lexer->input.state.line;
	token.col = lexer->input.state.col;

	token.str = &lexer->input.state.buffer[lexer->input.state.index];
	token.len = 0;

	char c = 0;
	for (c = InputPeek(&lexer->input, 0); (isAlpha(c) || isDigit(c) || c == '_'); c = InputPeek(&lexer->input, 0))
	{
		InputNext(&lexer->input);
		token.len++;
	}

	if (KeywordType keywordType = getKeywordType(token.str, token.len))
		token.keywordType = keywordType;

	return token;
}

Token LexerNext(Lexer* lexer)
{
	skipWhitespace(lexer);

	if (InputHasNext(&lexer->input))
	{
		if (nextIsStringLiteral(lexer))
			return readStringLiteral(lexer);
		if (nextIsCharLiteral(lexer))
			return readCharLiteral(lexer);
		if (nextIsNumberLiteral(lexer))
			return readNumberLiteral(lexer);
		if (nextIsPunctuation(lexer))
			return readPunctuation(lexer);
		if (nextIsOperator(lexer))
			return readOperator(lexer);
		if (nextIsIdentifier(lexer))
			return readIdentifier(lexer);

		SnekErrorLoc(lexer->context, lexer->input.state, "Undefined character '%c' (%d)", InputNext(&lexer->input));
		lexer->failed = true;

		return LexerNext(lexer);
	}

	return NULL_TOKEN;
}

Token LexerPeek(Lexer* lexer, int offset)
{
	skipWhitespace(lexer);
	InputState state = lexer->input.state;

	for (int i = 0; i < offset; i++)
		LexerNext(lexer);

	Token token = LexerNext(lexer);
	lexer->input.state = state;

	return token;
}

bool LexerHasNext(Lexer* lexer)
{
	return LexerPeek(lexer).type != TOKEN_TYPE_NULL;
}

bool LexerNextIsWhitespace(Lexer* lexer)
{
	return nextIsWhitespace(lexer);
}

char* GetTokenString(Token token, int offset, int trim)
{
	int len = token.len - offset - trim;
	char* str = new char[len + 1];
	memcpy(str, token.str + offset, len);
	str[len] = 0;
	return str;
}
