#include "Lexer.h"

#include "Keywords.h"
#include "Diagnostics.h"
#include "utils/Arena.h"
#include "utils/Log.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <map>
#include <string>


void initLexer(Lexer* lexer, const char* filename, const char* src, int length, Arena* arena, Diagnostics* diagnostics)
{
	lexer->filename = filename;
	lexer->src = src;
	lexer->length = length;
	lexer->arena = arena;
	lexer->diagnostics = diagnostics;
	lexer->cursor = 0;

	lexer->lineOffsets.add(0);
	for (int i = 0; i < length; i++)
	{
		if (src[i] == '\n')
			lexer->lineOffsets.add(i + 1);
	}
}

void destroyLexer(Lexer* lexer)
{
	FreeList(&lexer->lineOffsets);
}

SourceLocation getSourceLocation(Lexer* lexer, int offset)
{
	for (int i = 0; i < lexer->lineOffsets.size - 1; i++)
	{
		if (lexer->lineOffsets[i + 1] > offset)
		{
			int line = i;
			int col = offset - lexer->lineOffsets[i];
			return { lexer->filename, line, col };
		}
	}
	int line = lexer->lineOffsets.size - 1;
	int col = offset - lexer->lineOffsets[lexer->lineOffsets.size - 1];
	return { lexer->filename, line, col };
}

StringView getTokenString(Token token, const char* src)
{
	return CreateString(&src[token.offset], token.length);
}

static void error(Lexer* lexer, int start, int end, const char* fmt, ...)
{
	if (!lexer->diagnostics) return;

	SourceLocation startLocation = getSourceLocation(lexer, start);
	SourceLocation endLocation = getSourceLocation(lexer, end);

	va_list args;
	va_start(args, fmt);

	int length = vsnprintf(nullptr, 0, fmt, args);
	char* msg = (char*)lexer->arena->alloc(length + 1);
	vsnprintf(msg, length + 1, fmt, args);

	va_end(args);

	logMessage(lexer->diagnostics, msg, startLocation.line, startLocation.col, endLocation.line, endLocation.col, DIAGNOSTICS_ERROR);
}

static char peekCharacter(Lexer* lexer, int offset = 0)
{
	if (lexer->cursor >= lexer->length)
		return 0;
	return lexer->src[lexer->cursor + offset];
}

static char nextCharacter(Lexer* lexer)
{
	if (lexer->cursor >= lexer->length)
		return 0;
	return lexer->src[lexer->cursor++];
}

// replacement for isalpha because that one crashes when passing ö
static bool isAlpha(char c)
{
	return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
}

static bool isDigit(char c)
{
	return c >= '0' && c <= '9';
}

static bool nextIsWhitespace(Lexer* lexer)
{
	char c = peekCharacter(lexer, 0);
	return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\b';
}

static bool nextIsComment(Lexer* lexer)
{
	return peekCharacter(lexer, 0) == '/' && (peekCharacter(lexer, 1) == '/' || peekCharacter(lexer, 1) == '*');
}

static void skipComment(Lexer* lexer)
{
	nextCharacter(lexer);
	char c2 = nextCharacter(lexer);
	if (c2 == '/')
		while (peekCharacter(lexer) && nextCharacter(lexer) != '\n');
	else if (c2 == '*')
	{
		int level = 1;
		while (peekCharacter(lexer) && level > 0)
		{
			char c = nextCharacter(lexer);
			if (c == '/' && nextCharacter(lexer) == '*')
				level++;
			else if (c == '*' && nextCharacter(lexer) == '/')
				level--;
		}
	}
	else
	{
		SnekAssertMsg(false, "SkipComment error");
	}
}

static void skipWhitespace(Lexer* lexer)
{
	while (nextIsWhitespace(lexer) || nextIsComment(lexer))
	{
		if (nextIsComment(lexer))
			skipComment(lexer);
		else
			nextCharacter(lexer);
	}
}

static bool readStringLiteral(Lexer* lexer, Token* token)
{
	if (peekCharacter(lexer, 0) != '"')
		return false;

	token->offset = lexer->cursor;

	bool multiline = peekCharacter(lexer, 1) == '"' && peekCharacter(lexer, 2) == '"';

	if (multiline)
	{
		token->type = TOKEN_STRING_LITERAL_MULTILINE;

		nextCharacter(lexer); // "
		nextCharacter(lexer); // "
		nextCharacter(lexer); // "

		char c;
		do
		{
			c = nextCharacter(lexer);
		} while (c != '\n');

		int start = lexer->cursor;
		int end = lexer->cursor;

		while (peekCharacter(lexer))
		{
			if (peekCharacter(lexer, 0) == '\n' || peekCharacter(lexer, 0) == '\r')
			{
				end = lexer->cursor;
			}

			char c = nextCharacter(lexer);

			if (c == '"' && peekCharacter(lexer, 0) == '"' && peekCharacter(lexer, 1) == '"')
			{
				nextCharacter(lexer); // "
				nextCharacter(lexer); // "

				token->length = end - start;
				skipWhitespace(lexer);
				return true;
			}
		}

		error(lexer, start, end, "Unterminated multiline string literal");
		return true;
	}
	else
	{
		token->type = TOKEN_STRING_LITERAL;

		int start = lexer->cursor;

		nextCharacter(lexer);

		while (peekCharacter(lexer))
		{
			char c = nextCharacter(lexer);

			if (c == '"')
			{
				int end = lexer->cursor;
				token->length = end - start;
				skipWhitespace(lexer);
				return true;
			}
		}

		int end = lexer->cursor;
		error(lexer, start, end, "Unterminated string literal");
		return true;
	}
}

static bool readCharLiteral(Lexer* lexer, Token* token)
{
	if (peekCharacter(lexer, 0) != '\'')
		return false;

	int start = lexer->cursor;

	token->type = TOKEN_CHAR_LITERAL;
	token->offset = start;

	nextCharacter(lexer); // '

	while (peekCharacter(lexer))
	{
		char c = nextCharacter(lexer);

		if (c == '\'')
		{
			int end = lexer->cursor;
			token->length = end - start;
			skipWhitespace(lexer);
			return true;
		}
	}

	int end = lexer->cursor;
	error(lexer, start, end, "Unterminated character literal");
	return true;
}

static bool readNumberLiteral(Lexer* lexer, Token* token)
{
	char c = tolower(peekCharacter(lexer, 0));
	char c2 = tolower(peekCharacter(lexer, 1));
	bool negative = c == '-';
	if (!isDigit(c) && !(negative && isDigit(c2)))
		return false;

	int start = lexer->cursor;
	token->offset = start;

	bool fp = false;
	bool isDouble = true;
	bool hex = false;

	if (negative)
		nextCharacter(lexer); // -

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
		nextCharacter(lexer);
		if (c == 'x')
			hex = true;
		if (c == '.' || (!hex && c == 'f'))
			fp = true;
		if (!hex && c == 'f')
			isDouble = false;

		c = tolower(peekCharacter(lexer, 0));
		c2 = tolower(peekCharacter(lexer, 1));
	}

	int end = lexer->cursor;

	token->type = fp ? isDouble ? TOKEN_DOUBLE_LITERAL : TOKEN_FLOAT_LITERAL : TOKEN_INT_LITERAL;
	token->length = end - start;

	skipWhitespace(lexer);

	return true;
}

static bool readPunctuation(Lexer* lexer, Token* token)
{
	char c = peekCharacter(lexer, 0);
	bool punctuation = c == '.'
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
	if (!punctuation)
		return false;

	int start = lexer->cursor;

	token->type = (TokenType)nextCharacter(lexer);
	token->offset = start;

	int end = lexer->cursor;
	token->length = lexer->cursor - start;

	skipWhitespace(lexer);

	return true;
}

static bool readOperator(Lexer* lexer, Token* token)
{
	char c = peekCharacter(lexer, 0);
	bool op = c == '+'
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
	if (!op)
		return false;

	int start = lexer->cursor;

	token->offset = start;

	nextCharacter(lexer);

	switch (c)
	{
	case '+': token->type = TOKEN_PLUS; break;
	case '-': token->type = TOKEN_MINUS; break;
	case '*': token->type = TOKEN_ASTERISK; break;
	case '/': token->type = TOKEN_SLASH; break;
	case '%': token->type = TOKEN_PERCENT; break;
	case '&': token->type = TOKEN_AMPERSAND; break;
	case '|': token->type = TOKEN_OR; break;
	case '^': token->type = TOKEN_CARET; break;
	case '?': token->type = TOKEN_QUESTION; break;
	case '!': token->type = TOKEN_EXCLAMATION; break;
	case '=': token->type = TOKEN_EQUALS; break;
	case '<': token->type = TOKEN_LESS_THAN; break;
	case '>': token->type = TOKEN_GREATER_THAN; break;
	default:
		assert(false && "ReadOperator");
		break;
	}

	int end = lexer->cursor;
	token->length = end - start;

	skipWhitespace(lexer);

	return true;
}

static TokenType getKeywordType(const char* str, int len)
{
	switch (len)
	{
	case 2:
		switch (str[0])
		{
		case 'a':
			if (str[1] == 's') return TOKEN_AS;
			break;
		case 'i':
			if (str[1] == 'f') return TOKEN_IF;
			break;
		}
		break;

	case 3:
		switch (str[0])
		{
		case 'a':
			if (str[1] == 'n' && str[2] == 'y') return TOKEN_ANY;
			break;
		case 'f':
			if (str[1] == 'o' && str[2] == 'r') return TOKEN_FOR;
			break;
		case 'i':
			if (str[1] == 'n' && str[2] == 't') return TOKEN_INT32;
			break;
		case 'l':
			if (str[1] == 'e' && str[2] == 't') return TOKEN_LET;
			break;
		case 'n':
			if (str[1] == 'e' && str[2] == 'w') return TOKEN_MALLOC;
			break;
		case 'v':
			if (str[1] == 'a' && str[2] == 'r') return TOKEN_VARIABLE;
			break;
		}
		break;

	case 4:
		switch (str[0])
		{
		case 'b':
			if (str[1] == 'o' && str[2] == 'o' && str[3] == 'l') return TOKEN_BOOL;
			if (str[1] == 'y' && str[2] == 't' && str[3] == 'e') return TOKEN_UINT8;
			break;
		case 'c':
			if (str[1] == 'h' && str[2] == 'a' && str[3] == 'r') return TOKEN_INT8;
			break;
		case 'e':
			if (str[1] == 'l' && str[2] == 's' && str[3] == 'e') return TOKEN_ELSE;
			if (str[1] == 'n' && str[2] == 'u' && str[3] == 'm') return TOKEN_ENUM;
			break;
		case 'f':
			if (str[1] == 'u' && str[2] == 'n' && str[3] == 'c') return TOKEN_FUNCTION;
			break;
		case 'i':
			if (str[1] == 'n' && str[2] == 't' && str[3] == '8') return TOKEN_INT8;
			break;
		case 'l':
			if (str[1] == 'o' && str[2] == 'n' && str[3] == 'g') return TOKEN_INT64;
			break;
		case 'n':
			if (str[1] == 'u' && str[2] == 'l' && str[3] == 'l') return TOKEN_NULL_KEYWORD;
			break;
		case 's':
			if (str[1] == 'n' && str[2] == 'e' && str[3] == 'w') return TOKEN_STACKNEW;
			break;
		case 't':
			if (str[1] == 'r' && str[2] == 'u' && str[3] == 'e') return TOKEN_TRUE;
			break;
		case 'u':
			if (str[1] == 'i' && str[2] == 'n' && str[3] == 't') return TOKEN_UINT32;
			break;
		case 'v':
			if (str[1] == 'o' && str[2] == 'i' && str[3] == 'd') return TOKEN_VOID;
			break;
		}
		break;

	case 5:
		switch (str[0])
		{
		case 'b':
			if (str[1] == 'r' && str[2] == 'e' && str[3] == 'a' && str[4] == 'k') return TOKEN_BREAK;
			break;
		case 'c':
			if (str[1] == 'l' && str[2] == 'a' && str[3] == 's' && str[4] == 's') return TOKEN_CLASS;
			if (str[1] == 'o' && str[2] == 'n' && str[3] == 's' && str[4] == 't') return TOKEN_CONSTANT;
			break;
		case 'd':
			if (str[1] == 'e' && str[2] == 'f' && str[3] == 'e' && str[4] == 'r') return TOKEN_DEFER;
			break;
		case 'f':
			if (str[1] == 'a' && str[2] == 'l' && str[3] == 's' && str[4] == 'e') return TOKEN_FALSE;
			if (str[1] == 'l' && str[2] == 'o' && str[3] == 'a' && str[4] == 't') return TOKEN_FLOAT32;
			break;
		case 'i':
			if (str[1] == 'n' && str[2] == 't' && str[3] == '1' && str[4] == '6') return TOKEN_INT16;
			if (str[1] == 'n' && str[2] == 't' && str[3] == '3' && str[4] == '2') return TOKEN_INT32;
			if (str[1] == 'n' && str[2] == 't' && str[3] == '6' && str[4] == '4') return TOKEN_INT64;
			break;
		case 'm':
			if (str[1] == 'a' && str[2] == 'c' && str[3] == 'r' && str[4] == 'o') return TOKEN_MACRO;
			break;
		case 's':
			if (str[1] == 'h' && str[2] == 'o' && str[3] == 'r' && str[4] == 't') return TOKEN_INT16;
			break;
		case 'u':
			if (str[1] == 'c' && str[2] == 'h' && str[3] == 'a' && str[4] == 'r') return TOKEN_UINT8;
			if (str[1] == 'i' && str[2] == 'n' && str[3] == 't' && str[4] == '8') return TOKEN_UINT8;
			if (str[1] == 'l' && str[2] == 'o' && str[3] == 'n' && str[4] == 'g') return TOKEN_UINT64;
			if (str[1] == 'n' && str[2] == 'i' && str[3] == 'o' && str[4] == 'n') return TOKEN_UNION;
			break;
		case 'w':
			if (str[1] == 'h' && str[2] == 'i' && str[3] == 'l' && str[4] == 'e') return TOKEN_WHILE;
			break;
		}
		break;

	case 6:
		switch (str[0])
		{
		case 'a':
			if (str[1] == 'l' && str[2] == 'l' && str[3] == 'o' && str[4] == 'c' && str[5] == 'a') return TOKEN_ALLOCA;
			if (str[1] == 's' && str[2] == 's' && str[3] == 'e' && str[4] == 'r' && str[5] == 't') return TOKEN_ASSERT;
			break;
		case 'd':
			if (str[1] == 'e' && str[2] == 'l' && str[3] == 'e' && str[4] == 't' && str[5] == 'e') return TOKEN_FREE;
			if (str[1] == 'o' && str[2] == 'u' && str[3] == 'b' && str[4] == 'l' && str[5] == 'e') return TOKEN_FLOAT64;
			break;
		case 'i':
			if (str[1] == 'm' && str[2] == 'p' && str[3] == 'o' && str[4] == 'r' && str[5] == 't') return TOKEN_IMPORT;
			break;
		case 'm':
			if (str[1] == 'e' && str[2] == 't' && str[3] == 'h' && str[4] == 'o' && str[5] == 'd') return TOKEN_METHOD;
			break;
		case 'p':
			if (str[1] == 'a' && str[2] == 'c' && str[3] == 'k' && str[4] == 'e' && str[5] == 'd') return TOKEN_PACKED;
			if (str[1] == 'u' && str[2] == 'b' && str[3] == 'l' && str[4] == 'i' && str[5] == 'c') return TOKEN_PUBLIC;
			break;
		case 'r':
			if (str[1] == 'e' && str[2] == 't' && str[3] == 'u' && str[4] == 'r' && str[5] == 'n') return TOKEN_RETURN;
			break;
		case 's':
			if (str[1] == 'i' && str[2] == 'z' && str[3] == 'e' && str[4] == 'o' && str[5] == 'f') return TOKEN_SIZEOF;
			if (str[1] == 't' && str[2] == 'a' && str[3] == 't' && str[4] == 'i' && str[5] == 'c') return TOKEN_STATIC;
			if (str[1] == 't' && str[2] == 'r' && str[3] == 'i' && str[4] == 'n' && str[5] == 'g') return TOKEN_STRING;
			if (str[1] == 't' && str[2] == 'r' && str[3] == 'u' && str[4] == 'c' && str[5] == 't') return TOKEN_STRUCT;
			break;
		case 'u':
			if (str[1] == 'i' && str[2] == 'n' && str[3] == 't' && str[4] == '1' && str[5] == '6') return TOKEN_UINT16;
			if (str[1] == 'i' && str[2] == 'n' && str[3] == 't' && str[4] == '3' && str[5] == '2') return TOKEN_UINT32;
			if (str[1] == 'i' && str[2] == 'n' && str[3] == 't' && str[4] == '6' && str[5] == '4') return TOKEN_UINT64;
			if (str[1] == 's' && str[2] == 'h' && str[3] == 'o' && str[4] == 'r' && str[5] == 't') return TOKEN_UINT16;
			break;
		}
		break;

	case 7:
		switch (str[0])
		{
		case 'e':
			if (str[1] == 'x' && str[2] == 't' && str[3] == 'e' && str[4] == 'r' && str[5] == 'n' && str[6] == 'c') return TOKEN_EXTERN;
			break;
		case 'f':
			if (str[1] == 'l' && str[2] == 'o' && str[3] == 'a' && str[4] == 't' && str[5] == '3' && str[6] == '2') return TOKEN_FLOAT32;
			if (str[1] == 'l' && str[2] == 'o' && str[3] == 'a' && str[4] == 't' && str[5] == '6' && str[6] == '4') return TOKEN_FLOAT64;
			break;
		case 'p':
			if (str[1] == 'a' && str[2] == 'c' && str[3] == 'k' && str[4] == 'a' && str[5] == 'g' && str[6] == 'e') return TOKEN_MODULE;
			break;
		case 't':
			if (str[1] == 'y' && str[2] == 'p' && str[3] == 'e' && str[4] == 'd' && str[5] == 'e' && str[6] == 'f') return TOKEN_TYPEDEF;
			break;
		}
		break;

	case 8:
		switch (str[0])
		{
		case 'c':
			if (str[1] == 'o' && str[2] == 'n' && str[3] == 't' && str[4] == 'i' && str[5] == 'n' && str[6] == 'u' && str[7] == 'e') return TOKEN_CONTINUE;
			break;
		case 'i':
			if (str[1] == 'n' && str[2] == 't' && str[3] == 'e' && str[4] == 'r' && str[5] == 'n' && str[6] == 'a' && str[7] == 'l') return TOKEN_PRIVATE;
			break;
		case 'n':
			if (str[1] == 'o' && str[2] == 'm' && str[3] == 'a' && str[4] == 'n' && str[5] == 'g' && str[6] == 'l' && str[7] == 'e') return TOKEN_NOMANGLE;
			break;
		}
		break;

	case 9:
		switch (str[0])
		{
		case 'd':
			if (str[1] == 'l' && str[2] == 'l' && str[3] == 'e' && str[4] == 'x' && str[5] == 'p' && str[6] == 'o' && str[7] == 'r' && str[8] == 't') return TOKEN_DLLEXPORT;
			if (str[1] == 'l' && str[2] == 'l' && str[3] == 'i' && str[4] == 'm' && str[5] == 'p' && str[6] == 'o' && str[7] == 'r' && str[8] == 't') return TOKEN_DLLIMPORT;
			break;
		case 'n':
			if (str[1] == 'a' && str[2] == 'm' && str[3] == 'e' && str[4] == 's' && str[5] == 'p' && str[6] == 'a' && str[7] == 'c' && str[8] == 'e') return TOKEN_NAMESPACE;
			break;
		}
		break;

	default:
		break;
	}
	return TOKEN_IDENTIFIER;
}

static bool readIdentifier(Lexer* lexer, Token* token)
{
	char c = peekCharacter(lexer, 0);
	bool identifier = isAlpha(c) || c == '_';
	if (!identifier)
		return false;

	int start = lexer->cursor;
	token->offset = start;

	while (isAlpha(c) || isDigit(c) || c == '_')
	{
		nextCharacter(lexer);
		c = peekCharacter(lexer);
	}

	int end = lexer->cursor;
	int length = end - start;
	token->type = getKeywordType(&lexer->src[start], length);
	token->length = length;

	skipWhitespace(lexer);

	return true;
}

Token nextToken(Lexer* lexer)
{
	skipWhitespace(lexer);

	if (lexer->cursor >= lexer->length)
		return {};

	Token token = {};
	if (readStringLiteral(lexer, &token))
		return token;
	if (readCharLiteral(lexer, &token))
		return token;
	if (readNumberLiteral(lexer, &token))
		return token;
	if (readPunctuation(lexer, &token))
		return token;
	if (readOperator(lexer, &token))
		return token;
	if (readIdentifier(lexer, &token))
		return token;

	error(lexer, lexer->cursor, lexer->cursor + 1, "Undefined character '%c' (%d)", nextCharacter(lexer));

	return nextToken(lexer);
}
