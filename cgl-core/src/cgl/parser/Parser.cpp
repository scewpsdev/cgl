#include "pch.h"
#include "Parser.h"

#include "Lexer.h"
#include "cgl/ast/File.h"
#include "cgl/utils/Log.h"
#include "cgl/utils/Stringbuffer.h"

#include <string.h>
#include <stdlib.h>


static InputState GetInputState(Parser* parser)
{
	return parser->lexer->input.state;
}

static void SetInputState(Parser* parser, const InputState& inputState)
{
	parser->lexer->input.state = inputState;
}

static bool HasNext(Parser* parser)
{
	return LexerHasNext(parser->lexer);
}

static Token NextToken(Parser* parser)
{
	return LexerNext(parser->lexer);
}

static Token PeekToken(Parser* parser, int offset = 0)
{
	return LexerPeek(parser->lexer, offset);
}

static bool NextTokenIs(Parser* parser, int tokenType, int offset = 0)
{
	return PeekToken(parser, offset).type == tokenType;
}

static bool NextTokenIsWithoutSpaces(Parser* parser, int tokenType, int offset = 0)
{
	return !LexerNextIsWhitespace(parser->lexer) && PeekToken(parser, offset).type == tokenType;
}

static bool NextTokenIsKeyword(Parser* parser, int keywordType)
{
	Token tok = PeekToken(parser);
	return tok.type == TOKEN_TYPE_IDENTIFIER && tok.keywordType == keywordType;
}

static bool NextTokenIsKeyword(Parser* parser)
{
	Token tok = PeekToken(parser);
	return tok.type == TOKEN_TYPE_IDENTIFIER && tok.keywordType != KEYWORD_TYPE_NULL;
}

static bool NextTokenIsIdentifier(Parser* parser, const char* name)
{
	Token tok = PeekToken(parser);
	return tok.type == TOKEN_TYPE_IDENTIFIER && strlen(name) == tok.len && strncmp(tok.str, name, tok.len) == 0;
}

static void SkipWhitespace(Parser* parser)
{
	PeekToken(parser);
}

static bool SkipToken(Parser* parser, int tokenType)
{
	if (NextTokenIs(parser, tokenType))
	{
		NextToken(parser);
		return true;
	}
	else
	{
		Token tok = NextToken(parser);
		SnekError(parser->context, parser->lexer->input.state, "Expected token '%c'(%d), got '%.*s'", tokenType, tokenType, tok.len, tok.str);
		parser->failed = true;
		return false;
	}
}

static void SkipPastToken(Parser* parser, int tokenType)
{
	while (!NextTokenIs(parser, tokenType) && HasNext(parser))
	{
		NextToken(parser);
	}
	NextToken(parser); // ;
}

static void SkipPastStatement(Parser* parser)
{
	while (!NextTokenIs(parser, ';') && !NextTokenIs(parser, '}') && HasNext(parser))
	{
		NextToken(parser);
	}
	NextToken(parser); // ;
}

static AST::Type* ParseType(Parser* parser);
static AST::Expression* ParseExpression(Parser* parser, int prec = INT32_MAX);

static AST::Type* ParseElementType(Parser* parser)
{
	InputState inputState = GetInputState(parser);

	Token tok = NextToken(parser);
	if (tok.type == TOKEN_TYPE_IDENTIFIER)
	{
		switch (tok.keywordType)
		{
		case KEYWORD_TYPE_VOID:
			return new AST::VoidType(parser->module, inputState);

		case KEYWORD_TYPE_INT8:
			return new AST::IntegerType(parser->module, inputState, 8, true);
		case KEYWORD_TYPE_INT16:
			return new AST::IntegerType(parser->module, inputState, 16, true);
		case KEYWORD_TYPE_INT32:
			return new AST::IntegerType(parser->module, inputState, 32, true);
		case KEYWORD_TYPE_INT64:
			return new AST::IntegerType(parser->module, inputState, 64, true);

		case KEYWORD_TYPE_UINT8:
			return new AST::IntegerType(parser->module, inputState, 8, false);
		case KEYWORD_TYPE_UINT16:
			return new AST::IntegerType(parser->module, inputState, 16, false);
		case KEYWORD_TYPE_UINT32:
			return new AST::IntegerType(parser->module, inputState, 32, false);
		case KEYWORD_TYPE_UINT64:
			return new AST::IntegerType(parser->module, inputState, 64, false);

		case KEYWORD_TYPE_FLOAT16:
			return new AST::FloatingPointType(parser->module, inputState, 16);
		case KEYWORD_TYPE_FLOAT32:
			return new AST::FloatingPointType(parser->module, inputState, 32);
		case KEYWORD_TYPE_FLOAT64:
			return new AST::FloatingPointType(parser->module, inputState, 64);
		case KEYWORD_TYPE_FLOAT80:
			return new AST::FloatingPointType(parser->module, inputState, 80);
		case KEYWORD_TYPE_FLOAT128:
			return new AST::FloatingPointType(parser->module, inputState, 128);

		case KEYWORD_TYPE_BOOL:
			return new AST::BooleanType(parser->module, inputState);

		case KEYWORD_TYPE_STRING: {
			AST::Expression* length = nullptr;
			if (NextTokenIsWithoutSpaces(parser, '('))
			{
				NextToken(parser); // (
				length = ParseExpression(parser);
				SkipToken(parser, ')');
			}

			return new AST::StringType(parser->module, inputState, length);
		}

		case KEYWORD_TYPE_NULL: {
			char* name = GetTokenString(tok);

			bool hasGenericArgs = false;
			List<AST::Type*> genericArgs;

			if (NextTokenIsWithoutSpaces(parser, TOKEN_TYPE_OP_LESS_THAN))
			{
				NextToken(parser); // <

				bool upcomingType = !NextTokenIs(parser, TOKEN_TYPE_OP_GREATER_THAN);
				while (HasNext(parser) && upcomingType)
				{
					hasGenericArgs = true;
					if (AST::Type* genericArg = ParseType(parser))
					{
						genericArgs.add(genericArg);

						upcomingType = NextTokenIs(parser, ',');
						if (upcomingType)
							SkipToken(parser, ',');
					}
					else
					{
						SnekAssert(false);
					}
				}
				NextToken(parser); // >
			}

			return new AST::NamedType(parser->module, inputState, name, hasGenericArgs, genericArgs);
		}
		}
	}
	else if (tok.type == '(')
	{
		List<AST::Type*> valueTypes;
		while (!NextTokenIs(parser, ')'))
		{
			AST::Type* type = ParseType(parser);
			valueTypes.add(type);
			if (!NextTokenIs(parser, ')'))
				SkipToken(parser, ',');
		}
		NextToken(parser); // )

		return new AST::TupleType(parser->module, inputState, valueTypes);
	}

	SetInputState(parser, inputState);
	return NULL;
}

static AST::Type* ParseComplexType(Parser* parser, AST::Type* elementType)
{
	AST::SourceLocation location = elementType->location;

	if (NextTokenIs(parser, TOKEN_TYPE_OP_ASTERISK))
	{
		NextToken(parser); // *

		auto pointerType = new AST::PointerType(parser->module, location, elementType);

		return ParseComplexType(parser, pointerType);
	}
	else if (NextTokenIs(parser, TOKEN_TYPE_OP_QUESTION))
	{
		NextToken(parser); // ?

		auto optionalType = new AST::OptionalType(parser->module, location, elementType);

		return ParseComplexType(parser, optionalType);
	}
	else if (NextTokenIs(parser, '['))
	{
		NextToken(parser); // [
		AST::Expression* length = NULL;
		if (!NextTokenIs(parser, ']'))
		{
			length = ParseExpression(parser);
		}
		SkipToken(parser, ']');

		auto arrayType = new AST::ArrayType(parser->module, location, elementType, length);

		return ParseComplexType(parser, arrayType);
	}

	return elementType;
}

static AST::Type* ParseFunctionType(Parser* parser, AST::Type* elementType)
{
	InputState inputState = GetInputState(parser);
	AST::SourceLocation location = elementType->location;

	if (NextTokenIs(parser, '('))
	{
		NextToken(parser); // (

		List<AST::Type*> paramTypes = CreateList<AST::Type*>();
		bool varArgs = false;

		bool upcomingParamType = !NextTokenIs(parser, ')');
		while (upcomingParamType)
		{
			if (NextTokenIs(parser, '.'))
			{
				if (SkipToken(parser, '.') && SkipToken(parser, '.') && SkipToken(parser, '.'))
				{
					varArgs = true;
					upcomingParamType = false;
				}
				else
				{
					SnekError(parser->context, parser->lexer->input.state, "Variadic arguments need to be declared as '...'");
					parser->failed = true;
				}
			}
			else
			{
				if (AST::Type* paramType = ParseType(parser))
				{
					paramTypes.add(paramType);

					if (NextTokenIs(parser, TOKEN_TYPE_IDENTIFIER))
						NextToken(parser); // skip param names for function types for now

					upcomingParamType = NextTokenIs(parser, ',');
					if (upcomingParamType)
						SkipToken(parser, ',');
				}
				else
				{
					DestroyList(paramTypes);
					SetInputState(parser, inputState);
					return nullptr;
				}
			}
		}

		if (!NextTokenIs(parser, ')'))
		{
			DestroyList(paramTypes);
			SetInputState(parser, inputState);
			return nullptr;
		}

		SkipToken(parser, ')');

		auto functionType = new AST::FunctionType(parser->module, location, elementType, paramTypes, varArgs);

		return ParseComplexType(parser, functionType);
	}

	return elementType;
}

static AST::Type* ParseType(Parser* parser)
{
	InputState inputState = GetInputState(parser);

	if (AST::Type* elementType = ParseElementType(parser))
	{
		if (AST::Type* functionType = ParseFunctionType(parser, ParseComplexType(parser, elementType)))
			return functionType;
		else
			delete elementType;
	}

	SetInputState(parser, inputState);

	return nullptr;
}

static AST::Expression* ParseAtom(Parser* parser)
{
	InputState inputState = GetInputState(parser);

	if (NextTokenIs(parser, TOKEN_TYPE_INT_LITERAL))
	{
		Token token = NextToken(parser);
		char* str = GetTokenString(token);

		int64_t value = 0;
		bool isNegative = false;
		int base = 10;

		for (int i = 0; i < (int)strlen(str); i++)
		{
			if (str[i] == '-')
			{
				SnekAssert(i == 0);
				isNegative = true;
			}
			else if (str[i] == '_')
			{
				;
			}
			else if (isdigit(str[i]))
			{
				value = value * base + (str[i] - '0');
			}
			else if (isalpha(str[i]) && str[i] >= 'a' && str[i] <= 'f' && base == 16)
			{
				value = value * base + (str[i] - 'a' + 10);
			}
			else if (base == 10 && str[i] == 'b')
			{
				SnekAssert(value == 0);
				SnekAssert(i == 1 || i == 2 && isNegative);
				base = 2;
			}
			else if (base == 10 && str[i] == 'o')
			{
				SnekAssert(value == 0);
				SnekAssert(i == 1 || i == 2 && isNegative);
				base = 8;
			}
			else if (base == 10 && str[i] == 'x')
			{
				SnekAssert(value == 0);
				SnekAssert(i == 1 || i == 2 && isNegative);
				base = 16;
			}
			else
			{
				SnekAssert(false);
			}
		}

		if (isNegative)
			value *= -1;

		delete str;

		return new AST::IntegerLiteral(parser->module, inputState, value);
	}
	else if (NextTokenIs(parser, TOKEN_TYPE_FLOAT_LITERAL))
	{
		Token token = NextToken(parser);
		char* str = GetTokenString(token);

		char processed[32] = {};
		int len = 0;

		auto min = [](int a, int b) { return  a < b ? a : b; };

		for (int i = 0; i < min((int)strlen(str), 31); i++)
		{
			if (str[i] != '_')
				processed[len++] = str[i];
		}
		processed[len] = 0;

		double value = atof(processed);
		delete str;

		return new AST::FloatingPointLiteral(parser->module, inputState, value, processed);
	}
	else if (NextTokenIs(parser, TOKEN_TYPE_CHAR_LITERAL))
	{
		Token tok = NextToken(parser);

		char value = 0;
		for (int i = 0; i < tok.len; i++) {
			char c = tok.str[i];
			if (c == '\\' && i < tok.len - 1) {
				i++;
				switch (tok.str[i]) {
				case 'n': c = '\n'; break;
				case 'r': c = '\r'; break;
				case 't': c = '\t'; break;
				case 'b': c = '\b'; break;
				case '0': c = '\0'; break;
				case '\'': c = '\''; break;
				case '"': c = '"'; break;
				default:
					SnekWarn(parser->context, parser->lexer->input.state, "Undefined escape character '\\%c'", tok.str[i]);
					break;
				}
			}
			value = c;

			if (i < tok.len - 1) {
				SnekWarn(parser->context, parser->lexer->input.state, "Invalid character literal length: %d", tok.len - i);
			}
			break;
		}

		return new AST::CharacterLiteral(parser->module, inputState, value);
	}
	else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_TRUE))
	{
		NextToken(parser); // true

		return new AST::BooleanLiteral(parser->module, inputState, true);
	}
	else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_FALSE))
	{
		NextToken(parser); // false

		return new AST::BooleanLiteral(parser->module, inputState, false);
	}
	else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_NULL_KEYWORD))
	{
		NextToken(parser); // null

		return new AST::NullLiteral(parser->module, inputState);
	}
	else if (NextTokenIs(parser, TOKEN_TYPE_STRING_LITERAL) || NextTokenIs(parser, TOKEN_TYPE_STRING_LITERAL_MULTILINE))
	{
		bool raw = NextTokenIs(parser, TOKEN_TYPE_STRING_LITERAL);

		Token token = NextToken(parser);

		if (raw)
		{
			return new AST::StringLiteral(parser->module, inputState, GetTokenString(token), token.len);
		}
		else
		{
			StringBuffer buffer = CreateStringBuffer(8);
			for (int i = 0; i < token.len; i++)
			{
				char c = token.str[i];
				if (c == '\\')
				{
					switch (token.str[++i])
					{
					case 'n': c = '\n'; break;
					case 'r': c = '\r'; break;
					case 't': c = '\t'; break;
					case '\\': c = '\\'; break;
					case '0': c = '\0'; break;
					default:
						SnekWarn(parser->context, parser->lexer->input.state, "Undefined escape character '\\%c'", token.str[i]);
						break;
					}
				}
				buffer << c;
			}

			return new AST::StringLiteral(parser->module, inputState, buffer.buffer, buffer.length);
		}
	}
	else if (NextTokenIs(parser, TOKEN_TYPE_IDENTIFIER))
	{
		char* name = GetTokenString(NextToken(parser));
		return new AST::Identifier(parser->module, inputState, name);
	}
	else if (NextTokenIs(parser, '('))
	{
		NextToken(parser); // (

		if (AST::Expression* compoundValue = ParseExpression(parser)) // Compound
		{
			if (NextTokenIs(parser, ')'))
			{
				NextToken(parser); // )

				return new AST::CompoundExpression(parser->module, inputState, compoundValue);
			}
			else if (NextTokenIs(parser, ','))
			{
				List<AST::Expression*> values;
				values.add(compoundValue);

				while (NextTokenIs(parser, ','))
				{
					NextToken(parser); // ,
					AST::Expression* value = ParseExpression(parser);
					values.add(value);
				}

				SkipToken(parser, ')');

				return new AST::TupleExpression(parser->module, inputState, values);
			}
			else
			{
				Token tok = NextToken(parser);
				SnekError(parser->context, parser->lexer->input.state, "Expected ')': %.*s", tok.len, tok.str);
				parser->failed = true;
				return nullptr;
			}
		}
		else
		{
			Token tok = NextToken(parser);
			SnekError(parser->context, parser->lexer->input.state, "Expected an expression: %.*s", tok.len, tok.str);
			parser->failed = true;
			return nullptr;
		}
	}

	return NULL;
}

static AST::Expression* ParseArgumentOperator(Parser* parser, AST::Expression* expression)
{
	InputState inputState = GetInputState(parser);

	if (NextTokenIsWithoutSpaces(parser, TOKEN_TYPE_OP_LESS_THAN) || NextTokenIs(parser, '(')) // Function call
	{
		List<AST::Expression*> arguments = CreateList<AST::Expression*>();

		bool hasGenericArgs = false;
		List<AST::Type*> genericArgs;

		if (NextTokenIsWithoutSpaces(parser, TOKEN_TYPE_OP_LESS_THAN))
		{
			NextToken(parser); // <

			bool upcomingType = !NextTokenIs(parser, TOKEN_TYPE_OP_GREATER_THAN);
			while (HasNext(parser) && upcomingType)
			{
				hasGenericArgs = true;
				if (AST::Type* genericArg = ParseType(parser))
				{
					genericArgs.add(genericArg);

					upcomingType = NextTokenIs(parser, ',');
					if (upcomingType)
						SkipToken(parser, ',');
				}
				else
				{
					SnekAssert(false);
				}
			}
			NextToken(parser); // >
		}

		NextToken(parser); // (

		bool upcomingDeclarator = !NextTokenIs(parser, ')');
		while (HasNext(parser) && upcomingDeclarator)
		{
			if (AST::Expression* argument = ParseExpression(parser))
			{
				arguments.add(argument);

				upcomingDeclarator = NextTokenIs(parser, ',');
				if (upcomingDeclarator)
					SkipToken(parser, ',');
			}
			else
			{
				SnekAssert(false);
			}
		}

		SkipToken(parser, ')');

		auto expr = new AST::FunctionCall(parser->module, inputState, expression, arguments, hasGenericArgs, genericArgs);

		return ParseArgumentOperator(parser, expr);
	}
	else if (NextTokenIs(parser, '[')) // Subscript operator
	{
		NextToken(parser); // [

		List<AST::Expression*> arguments;

		bool upcomingDeclarator = !NextTokenIs(parser, ']');
		while (HasNext(parser) && upcomingDeclarator)
		{
			if (AST::Expression* argument = ParseExpression(parser))
			{
				arguments.add(argument);

				upcomingDeclarator = NextTokenIs(parser, ',');
			}
			else
			{
				Token tok = NextToken(parser);
				SnekError(parser->context, parser->lexer->input.state, "Expected an expression: %.*s", tok.len, tok.str);
				parser->failed = true;
			}
		}

		SkipToken(parser, ']');

		auto expr = new AST::SubscriptOperator(parser->module, inputState, expression, arguments);

		return ParseArgumentOperator(parser, expr);
	}
	else if (NextTokenIs(parser, '.') && !NextTokenIs(parser, '.', 1)) // Dot operator
	{
		NextToken(parser); // .

		AST::Expression* expr = nullptr;

		if (NextTokenIs(parser, TOKEN_TYPE_IDENTIFIER))
		{
			char* name = GetTokenString(NextToken(parser));
			expr = new AST::DotOperator(parser->module, inputState, expression, name);
		}
		else if (NextTokenIs(parser, TOKEN_TYPE_INT_LITERAL))
		{
			char* str = GetTokenString(NextToken(parser));
			int index = atoi(str);
			delete str;

			expr = new AST::DotOperator(parser->module, inputState, expression, index);
		}

		return ParseArgumentOperator(parser, expr);
	}

	return expression;
}

static AST::UnaryOperatorType ParsePrefixOperatorType(Parser* parser)
{
	InputState inputState = GetInputState(parser);

	Token tok = NextToken(parser);
	if (tok.type == TOKEN_TYPE_OP_EXCLAMATION)
		return AST::UnaryOperatorType::Not;
	else if (tok.type == TOKEN_TYPE_OP_MINUS)
		return AST::UnaryOperatorType::Negate;
	else if (tok.type == TOKEN_TYPE_OP_AMPERSAND)
		return AST::UnaryOperatorType::Reference;
	else if (tok.type == TOKEN_TYPE_OP_ASTERISK)
		return AST::UnaryOperatorType::Dereference;
	else if (tok.type == TOKEN_TYPE_OP_PLUS)
	{
		Token tok2 = NextToken(parser);
		if (tok2.type == TOKEN_TYPE_OP_PLUS)
			return AST::UnaryOperatorType::Increment;
	}
	else if (tok.type == TOKEN_TYPE_OP_MINUS)
	{
		Token tok2 = NextToken(parser);
		if (tok2.type == TOKEN_TYPE_OP_MINUS)
			return AST::UnaryOperatorType::Decrement;
	}

	SetInputState(parser, inputState);
	return AST::UnaryOperatorType::Null;
}

static AST::Expression* ParsePrefixOperator(Parser* parser)
{
	InputState inputState = GetInputState(parser);

	AST::UnaryOperatorType operatorType = ParsePrefixOperatorType(parser);

	if (AST::Expression* atom = ParseAtom(parser))
	{
		AST::Expression* expression = ParseArgumentOperator(parser, atom);

		if (operatorType != AST::UnaryOperatorType::Null)
		{
			return new AST::UnaryOperator(parser->module, inputState, expression, operatorType, false);
		}
		else
		{
			return expression;
		}
	}
	else
	{
		return nullptr;
	}
}

static AST::UnaryOperatorType ParsePostfixOperatorType(Parser* parser)
{
	InputState inputState = GetInputState(parser);

	Token tok = NextToken(parser);
	if (tok.type == TOKEN_TYPE_OP_PLUS)
	{
		Token tok2 = NextToken(parser);
		if (tok2.type == TOKEN_TYPE_OP_PLUS)
			return AST::UnaryOperatorType::Increment;
	}
	else if (tok.type == TOKEN_TYPE_OP_MINUS)
	{
		Token tok2 = NextToken(parser);
		if (tok2.type == TOKEN_TYPE_OP_MINUS)
			return AST::UnaryOperatorType::Decrement;
	}

	SetInputState(parser, inputState);
	return AST::UnaryOperatorType::Null;
}

static AST::Expression* ParsePostfixOperator(Parser* parser, AST::Expression* expression)
{
	InputState inputState = GetInputState(parser);

	AST::UnaryOperatorType operatorType = ParsePostfixOperatorType(parser);
	if (operatorType != AST::UnaryOperatorType::Null)
	{
		return new AST::UnaryOperator(parser->module, inputState, expression, operatorType, true);
	}
	return expression;
}

static AST::Expression* ParseBasicExpression(Parser* parser)
{
	InputState inputState = GetInputState(parser);

	//if (AST::Type* type = ParseElementType(parser))
	{
		/*
		if (NextTokenIs(parser, '{'))
		{
			NextToken(parser); // {

			List<AST::Expression*> values = CreateList<AST::Expression*>();

			bool upcomingValue = !NextTokenIs(parser, '}');
			while (upcomingValue && HasNext(parser))
			{
				AST::Expression* value = ParseExpression(parser);
				values.add(value);

				upcomingValue = NextTokenIs(parser, ',');
				if (upcomingValue)
					NextToken(parser); // ,
			}

			SkipToken(parser, '}');

			return new AST::InitializerList(parser->module, inputState, type, values);
		}
		else
		{
			SetInputState(parser, inputState);
		}
		*/
	}
	if (NextTokenIs(parser, '{'))
	{
		NextToken(parser); // {

		List<AST::Expression*> values = CreateList<AST::Expression*>();

		bool upcomingValue = !NextTokenIs(parser, '}');
		while (upcomingValue && HasNext(parser))
		{
			AST::Expression* value = ParseExpression(parser);
			values.add(value);

			upcomingValue = NextTokenIs(parser, ',');
			if (upcomingValue)
				NextToken(parser); // ,
		}

		SkipToken(parser, '}');

		return new AST::InitializerList(parser->module, inputState, nullptr, values);
	}
	else
	{
		SetInputState(parser, inputState);
	}
	if (NextTokenIs(parser, '('))
	{
		NextToken(parser); // (
		if (AST::Type* dstType = ParseType(parser)) // Cast
		{
			if (NextTokenIs(parser, ')'))
			{
				NextToken(parser); // )
				if (AST::Expression* value = ParsePrefixOperator(parser))
				{
					if (value = ParsePostfixOperator(parser, value))
					{
						return new AST::Typecast(parser->module, inputState, value, dstType);
					}
				}
			}
		}
	}
	else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_SIZEOF))
	{
		NextToken(parser); // sizeof

		AST::Type* sizedType = ParseType(parser);

		return new AST::Sizeof(parser->module, inputState, sizedType);
	}
	else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_MALLOC) || NextTokenIsKeyword(parser, KEYWORD_TYPE_STACKNEW))
	{
		Token tok = NextToken(parser); // malloc/snew

		AST::Type* type = ParseComplexType(parser, ParseElementType(parser));
		AST::Expression* count = nullptr;
		bool malloc = tok.keywordType == KEYWORD_TYPE_MALLOC;
		bool hasArguments = false;
		List<AST::Expression*> arguments = CreateList<AST::Expression*>();

		if (NextTokenIs(parser, '('))
		{
			NextToken(parser); // (

			hasArguments = true;

			bool upcomingDeclarator = !NextTokenIs(parser, ')');
			while (HasNext(parser) && upcomingDeclarator)
			{
				if (AST::Expression* argument = ParseExpression(parser))
				{
					arguments.add(argument);

					upcomingDeclarator = NextTokenIs(parser, ',');
					if (upcomingDeclarator)
						SkipToken(parser, ',');
				}
				else
				{
					Token tok = NextToken(parser);
					SnekError(parser->context, parser->lexer->input.state, "Expected an expression: %.*s", tok.len, tok.str);
					parser->failed = true;
				}
			}

			SkipToken(parser, ')');
		}

		if (NextTokenIs(parser, ':'))
		{
			NextToken(parser); // :
			count = ParseExpression(parser);
		}

		return new AST::Malloc(parser->module, inputState, type, count, malloc, hasArguments, arguments);
	}

	SetInputState(parser, inputState);
	if (AST::Expression* expr = ParsePrefixOperator(parser))
	{
		expr = ParsePostfixOperator(parser, expr);
		return expr;
	}

	return NULL;
}

static AST::BinaryOperatorType ParseBinaryTernaryOperatorType(Parser* parser)
{
	InputState inputState = GetInputState(parser);

	Token tok = NextToken(parser);
	Token tok2 = PeekToken(parser);
	if (tok.type == TOKEN_TYPE_OP_PLUS)
	{
		if (tok2.type == TOKEN_TYPE_OP_EQUALS)
		{
			NextToken(parser); // =
			return AST::BinaryOperatorType::PlusEquals;
		}
		else
			return AST::BinaryOperatorType::Add;
	}
	else if (tok.type == TOKEN_TYPE_OP_MINUS)
	{
		if (tok2.type == TOKEN_TYPE_OP_EQUALS)
		{
			NextToken(parser); // =
			return AST::BinaryOperatorType::MinusEquals;
		}
		else
			return AST::BinaryOperatorType::Subtract;
	}
	else if (tok.type == TOKEN_TYPE_OP_ASTERISK)
	{
		if (tok2.type == TOKEN_TYPE_OP_EQUALS)
		{
			NextToken(parser); // =
			return AST::BinaryOperatorType::TimesEquals;
		}
		else
			return AST::BinaryOperatorType::Multiply;
	}
	else if (tok.type == TOKEN_TYPE_OP_SLASH)
	{
		if (tok2.type == TOKEN_TYPE_OP_EQUALS)
		{
			NextToken(parser); // =
			return AST::BinaryOperatorType::DividedByEquals;
		}
		else
			return AST::BinaryOperatorType::Divide;
	}
	else if (tok.type == TOKEN_TYPE_OP_PERCENT)
	{
		if (tok2.type == TOKEN_TYPE_OP_EQUALS)
		{
			NextToken(parser); // =
			return AST::BinaryOperatorType::ModuloEquals;
		}
		else
			return AST::BinaryOperatorType::Modulo;
	}
	else if (tok.type == TOKEN_TYPE_OP_EQUALS)
	{
		if (tok2.type == TOKEN_TYPE_OP_EQUALS)
		{
			NextToken(parser); // =
			return AST::BinaryOperatorType::Equals;
		}
		else
			return AST::BinaryOperatorType::Assignment;
	}
	else if (tok.type == TOKEN_TYPE_OP_LESS_THAN)
	{
		if (tok2.type == TOKEN_TYPE_OP_LESS_THAN)
		{
			NextToken(parser); // <
			Token tok3 = PeekToken(parser, 1);
			if (tok3.type == TOKEN_TYPE_OP_EQUALS)
			{
				NextToken(parser); // =
				return AST::BinaryOperatorType::BitshiftLeftEquals;
			}
			else
			{
				return AST::BinaryOperatorType::BitshiftLeft;
			}
		}
		else if (tok2.type == TOKEN_TYPE_OP_EQUALS)
		{
			NextToken(parser); // =
			return AST::BinaryOperatorType::LessThanEquals;
		}
		else
		{
			return AST::BinaryOperatorType::LessThan;
		}
	}
	else if (tok.type == TOKEN_TYPE_OP_GREATER_THAN)
	{
		if (tok2.type == TOKEN_TYPE_OP_GREATER_THAN)
		{
			NextToken(parser); // >
			Token tok3 = PeekToken(parser, 1);
			if (tok3.type == TOKEN_TYPE_OP_EQUALS)
			{
				NextToken(parser); // =
				return AST::BinaryOperatorType::BitshiftRightEquals;
			}
			else
			{
				return AST::BinaryOperatorType::BitshiftRight;
			}
		}
		else if (tok2.type == TOKEN_TYPE_OP_EQUALS)
		{
			NextToken(parser); // =
			return AST::BinaryOperatorType::GreaterThanEquals;
		}
		else
		{
			return AST::BinaryOperatorType::GreaterThan;
		}
	}
	else if (tok.type == TOKEN_TYPE_OP_AMPERSAND)
	{
		if (tok2.type == TOKEN_TYPE_OP_AMPERSAND)
		{
			NextToken(parser); // &
			return AST::BinaryOperatorType::LogicalAnd;
		}
		else if (tok2.type == TOKEN_TYPE_OP_EQUALS)
		{
			NextToken(parser); // =
			return AST::BinaryOperatorType::BitwiseAndEquals;
		}
		else
			return AST::BinaryOperatorType::BitwiseAnd;
	}
	else if (tok.type == TOKEN_TYPE_OP_OR)
	{
		if (tok2.type == TOKEN_TYPE_OP_OR)
		{
			NextToken(parser); // |
			return AST::BinaryOperatorType::LogicalOr;
		}
		else if (tok2.type == TOKEN_TYPE_OP_EQUALS)
		{
			NextToken(parser); // =
			return AST::BinaryOperatorType::BitwiseOrEquals;
		}
		else
			return AST::BinaryOperatorType::BitwiseOr;
	}
	else if (tok.type == TOKEN_TYPE_OP_CARET)
	{
		if (tok2.type == TOKEN_TYPE_OP_EQUALS)
		{
			NextToken(parser); // =
			return AST::BinaryOperatorType::BitwiseXorEquals;
		}
		else
			return AST::BinaryOperatorType::BitwiseXor;
	}
	else if (tok.type == TOKEN_TYPE_OP_EXCLAMATION)
	{
		if (tok2.type == TOKEN_TYPE_OP_EQUALS)
		{
			NextToken(parser); // =
			return AST::BinaryOperatorType::DoesNotEqual;
		}
	}
	else if (tok.type == TOKEN_TYPE_OP_QUESTION)
	{
		if (tok2.type == TOKEN_TYPE_OP_QUESTION)
		{
			NextToken(parser); // ?
			return AST::BinaryOperatorType::NullCoalescing;
		}
		else
			return AST::BinaryOperatorType::Ternary;
	}

	SetInputState(parser, inputState);
	return AST::BinaryOperatorType::Null;
}

static int GetBinaryOperatorPrecedence(AST::BinaryOperatorType operatorType)
{
	switch (operatorType)
	{
	case AST::BinaryOperatorType::Multiply: return 3;
	case AST::BinaryOperatorType::Divide: return 3;
	case AST::BinaryOperatorType::Modulo: return 3;
	case AST::BinaryOperatorType::Add: return 4;
	case AST::BinaryOperatorType::Subtract: return 4;

	case AST::BinaryOperatorType::BitshiftLeft: return 5;
	case AST::BinaryOperatorType::BitshiftRight: return 5;

	case AST::BinaryOperatorType::LessThan: return 6;
	case AST::BinaryOperatorType::GreaterThan: return 6;
	case AST::BinaryOperatorType::LessThanEquals: return 6;
	case AST::BinaryOperatorType::GreaterThanEquals: return 6;

	case AST::BinaryOperatorType::Equals: return 7;
	case AST::BinaryOperatorType::DoesNotEqual: return 7;

	case AST::BinaryOperatorType::BitwiseAnd: return 8;
	case AST::BinaryOperatorType::BitwiseXor: return 9;
	case AST::BinaryOperatorType::BitwiseOr: return 10;
	case AST::BinaryOperatorType::LogicalAnd: return 11;
	case AST::BinaryOperatorType::LogicalOr: return 12;

	case AST::BinaryOperatorType::NullCoalescing: return 13;
	case AST::BinaryOperatorType::Ternary: return 13;

	case AST::BinaryOperatorType::Assignment: return 14;
	case AST::BinaryOperatorType::PlusEquals: return 14;
	case AST::BinaryOperatorType::MinusEquals: return 14;
	case AST::BinaryOperatorType::TimesEquals: return 14;
	case AST::BinaryOperatorType::DividedByEquals: return 14;
	case AST::BinaryOperatorType::ModuloEquals: return 14;
	case AST::BinaryOperatorType::BitshiftLeftEquals: return 14;
	case AST::BinaryOperatorType::BitshiftRightEquals: return 14;
	case AST::BinaryOperatorType::BitwiseAndEquals: return 14;
	case AST::BinaryOperatorType::BitwiseOrEquals: return 14;
	case AST::BinaryOperatorType::BitwiseXorEquals: return 14;
	case AST::BinaryOperatorType::ReferenceAssignment: return 14;

	default:
		SnekAssert(false);
		return INT32_MAX;
	}
}

static AST::Expression* ParseBinaryTernaryOperator(Parser* parser, AST::Expression* expression, int prec = INT32_MAX)
{
	InputState inputState = GetInputState(parser);

	AST::BinaryOperatorType operatorType = ParseBinaryTernaryOperatorType(parser);
	if (operatorType != AST::BinaryOperatorType::Null)
	{
		int operatorPrec = GetBinaryOperatorPrecedence(operatorType);
		if (operatorPrec < prec)
		{
			AST::Expression* result = NULL;
			if (operatorType == AST::BinaryOperatorType::Ternary)
			{
				AST::Expression* thenValue = ParseExpression(parser);
				SkipToken(parser, ':');
				AST::Expression* elseValue = ParseExpression(parser);

				result = new AST::TernaryOperator(parser->module, inputState, expression, thenValue, elseValue);
			}
			else
			{
				AST::Expression* left = expression;
				AST::Expression* right = ParseExpression(parser, operatorPrec);

				result = new AST::BinaryOperator(parser->module, inputState, left, right, operatorType);
			}
			return ParseBinaryTernaryOperator(parser, result, prec);
		}
		else
		{
			SetInputState(parser, inputState);
		}
	}

	return expression;
}

static AST::Expression* ParseExpression(Parser* parser, int prec)
{
	if (AST::Expression* expr = ParseBasicExpression(parser))
	{
		expr = ParseBinaryTernaryOperator(parser, expr, prec);
		return expr;
	}

	Token tok = NextToken(parser);
	SnekError(parser->context, parser->lexer->input.state, "Expected an expression: %.*s", tok.len, tok.str);
	SkipPastStatement(parser);
	parser->failed = true;
	return NULL;
}

static AST::Statement* ParseStatement(Parser* parser)
{
	SkipWhitespace(parser);
	InputState inputState = GetInputState(parser);

	if (NextTokenIs(parser, ';'))
	{
		NextToken(parser); // ;

		return new AST::NoOpStatement(parser->module, inputState);
	}
	else if (NextTokenIs(parser, '{'))
	{
		NextToken(parser); // {

		List<AST::Statement*> statements = CreateList<AST::Statement*>();

		while (HasNext(parser) && !NextTokenIs(parser, '}'))
		{
			AST::Statement* statement = ParseStatement(parser);
			if (statement)
				statements.add(statement);
		}

		NextToken(parser); // }

		return new AST::CompoundStatement(parser->module, inputState, statements);
	}
	else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_IF))
	{
		NextToken(parser); // if

		//SkipToken(parser, '(');
		AST::Expression* condition = ParseExpression(parser);
		//SkipToken(parser, ')');

		AST::Statement* thenStatement = ParseStatement(parser);
		AST::Statement* elseStatement = NULL;

		if (NextTokenIsKeyword(parser, KEYWORD_TYPE_ELSE))
		{
			NextToken(parser); // else
			elseStatement = ParseStatement(parser);
		}

		return new AST::IfStatement(parser->module, inputState, condition, thenStatement, elseStatement);
	}
	else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_WHILE))
	{
		NextToken(parser); // while

		//SkipToken(parser, '(');
		AST::Expression* condition = ParseExpression(parser);
		//SkipToken(parser, ')');

		AST::Statement* body = ParseStatement(parser);

		return new AST::WhileLoop(parser->module, inputState, condition, body);
	}
	else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_FOR))
	{
		NextToken(parser); // for

		//if (NextTokenIs(parser, '('))
		{
			//NextToken(parser); // (

			//bool includeEndValue = false;

			if (NextTokenIs(parser, '('))
			{
				SkipToken(parser, '(');

				AST::Statement* initStatement = nullptr;
				AST::Expression* conditionExpr = nullptr;
				AST::Expression* iterateExpr = nullptr;

				if (!NextTokenIs(parser, ';'))
					initStatement = ParseStatement(parser);
				//SkipToken(parser, ';');

				if (!NextTokenIs(parser, ';'))
					conditionExpr = ParseExpression(parser);
				SkipToken(parser, ';');

				if (!NextTokenIs(parser, ')'))
					iterateExpr = ParseExpression(parser);

				SkipToken(parser, ')');

				//SkipToken(parser, ',');

				//ParseExpression(parser);
				//SkipToken(parser, ',');
				/*
				SkipToken(parser, '.');
				SkipToken(parser, '.');
				if (NextTokenIs(parser, '.'))
				{
					NextToken(parser); // .
					includeEndValue = true;
				}
				*/

				/*
				AST::Expression* endValue = ParseExpression(parser);
				AST::Expression* deltaValue = nullptr;

				if (NextTokenIs(parser, ','))
				{
					NextToken(parser); // ,
					deltaValue = ParseExpression(parser);
				}
				*/

				//SkipToken(parser, ')');


				AST::Statement* body = ParseStatement(parser);

				/*
				if (iteratorName->type == AST::ExpressionType::Identifier)
				{
					return new AST::ForLoop(parser->module, inputState, (AST::Identifier*)iteratorName, startValue, endValue, deltaValue, includeEndValue, body);
				}
				else
				{
					SnekError(parser->context, parser->lexer->input.state, ERROR_CODE_FOR_LOOP_SYNTAX, "Last value in for loop header needs to be the iterator name");
					parser->failed = true;
				}
				*/

				return new AST::ForLoop(parser->module, inputState, initStatement, conditionExpr, iterateExpr, body);
			}
			else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_CONSTANT) || NextTokenIsKeyword(parser, KEYWORD_TYPE_VARIABLE))
			{
				bool isConstant = NextTokenIsKeyword(parser, KEYWORD_TYPE_CONSTANT);
				NextToken(parser); // const/var

				if (NextTokenIs(parser, TOKEN_TYPE_IDENTIFIER))
				{
					char* iteratorName = GetTokenString(NextToken(parser));

					SkipToken(parser, ':');

					AST::Expression* container = ParseExpression(parser);

					AST::Statement* body = ParseStatement(parser);

					return new AST::ForLoop(parser->module, inputState, iteratorName, container, body);
				}
				else
				{
					SnekError(parser->context, inputState, "Expected identifier");
					return nullptr;
				}
			}
			else
			{
				SnekError(parser->context, inputState, "Expected iterator or initializer statement");
				return nullptr;
			}
		}
		/*
		else
		{
			SnekError(parser->context, parser->lexer->input.state, ERROR_CODE_FOR_LOOP_SYNTAX, "For loop arguments must be enclosed in parentheses");
			parser->failed = true;
		}
		*/
	}
	else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_BREAK))
	{
		NextToken(parser); // break
		SkipToken(parser, ';');

		return new AST::Break(parser->module, inputState);
	}
	else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_CONTINUE))
	{
		NextToken(parser); // continue
		SkipToken(parser, ';');

		return new AST::Continue(parser->module, inputState);
	}
	else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_RETURN))
	{
		NextToken(parser); // return

		AST::Expression* value = nullptr;
		if (!NextTokenIs(parser, ';'))
			value = ParseExpression(parser);

		SkipToken(parser, ';');

		return new AST::Return(parser->module, inputState, value);
	}
	else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_ASSERT))
	{
		NextToken(parser); // assert
		SkipToken(parser, '(');

		AST::Expression* condition = ParseExpression(parser);
		AST::Expression* message = nullptr;

		if (!NextTokenIs(parser, ')'))
		{
			SkipToken(parser, ',');
			message = ParseExpression(parser);
		}

		SkipToken(parser, ')');
		SkipToken(parser, ';');

		return new AST::Assert(parser->module, inputState, condition, message);
	}
	else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_FREE))
	{
		NextToken(parser); // free

		List<AST::Expression*> values = CreateList<AST::Expression*>();

		bool upcomingValue = true;
		while (HasNext(parser) && upcomingValue)
		{
			AST::Expression* value = ParseExpression(parser);
			values.add(value);

			upcomingValue = NextTokenIs(parser, ',');
			if (upcomingValue)
				SkipToken(parser, ',');
		}

		SkipToken(parser, ';');

		return new AST::Free(parser->module, inputState, values);
	}
	else
	{
		bool isConstant = false;
		bool isAutoType = false;
		AST::Type* type = nullptr;

		if (NextTokenIsKeyword(parser, KEYWORD_TYPE_LET) || NextTokenIsKeyword(parser, KEYWORD_TYPE_CONSTANT))
		{
			NextToken(parser); // let/const
			isConstant = true;

			if (NextTokenIs(parser, ':'))
			{
				NextToken(parser); // :
				type = ParseType(parser);
			}
			else
			{
				isAutoType = true;
			}
		}
		else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_VARIABLE))
		{
			NextToken(parser); // var
			isAutoType = true;
		}
		else
		{
			type = ParseType(parser);
		}

		if (type || isAutoType)
		{
			if (NextTokenIs(parser, TOKEN_TYPE_IDENTIFIER))
			{
				List<AST::VariableDeclarator*> declarators;

				bool upcomingDeclarator = true;
				while (HasNext(parser) && upcomingDeclarator)
				{
					InputState inputState = GetInputState(parser);

					char* name = GetTokenString(NextToken(parser));
					AST::Expression* value = NULL;

					if (NextTokenIs(parser, TOKEN_TYPE_OP_EQUALS))
					{
						NextToken(parser); // =
						value = ParseExpression(parser);
					}

					upcomingDeclarator = NextTokenIs(parser, ',');
					if (upcomingDeclarator)
						SkipToken(parser, ',');

					AST::VariableDeclarator* declarator = new AST::VariableDeclarator(parser->module, inputState, name, value);
					declarators.add(declarator);
				}

				SkipToken(parser, ';');

				return new AST::VariableDeclaration(parser->module, inputState, type, isConstant, declarators);
			}
			else
			{
				SetInputState(parser, inputState);
			}
		}
	}
	if (AST::Expression* expression = ParseExpression(parser))
	{
		SkipToken(parser, ';');

		return new AST::ExpressionStatement(parser->module, inputState, expression);
	}

	SkipPastStatement(parser);
	return nullptr;
}

static AST::Declaration* ParseDeclaration(Parser* parser)
{
	InputState inputState = GetInputState(parser);

	AST::DeclarationFlags flags = AST::DeclarationFlags::None;

	while (NextTokenIsKeyword(parser) && HasNext(parser))
	{
		if (NextTokenIsKeyword(parser, KEYWORD_TYPE_EXTERN))
		{
			NextToken(parser); // extern
			flags = flags | AST::DeclarationFlags::Extern;
		}
		else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_DLLEXPORT))
		{
			NextToken(parser); // dllexport
			flags = flags | AST::DeclarationFlags::DllExport;
		}
		else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_DLLIMPORT))
		{
			NextToken(parser); // dllimport
			flags = flags | AST::DeclarationFlags::DllImport;
		}
		else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_CONSTANT))
		{
			NextToken(parser); // const
			flags = flags | AST::DeclarationFlags::Constant;
		}
		else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_PUBLIC))
		{
			NextToken(parser); // public
			flags = flags | AST::DeclarationFlags::Public;
		}
		else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_PRIVATE))
		{
			NextToken(parser); // private
			flags = flags | AST::DeclarationFlags::Private;
		}
		else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_PACKED))
		{
			NextToken(parser); // packed
			flags = flags | AST::DeclarationFlags::Packed;
		}
		else
		{
			break;
		}
	}

	if (NextTokenIsKeyword(parser, KEYWORD_TYPE_FUNCTION))
	{
		NextToken(parser); // func

		char* name = GetTokenString(NextToken(parser));

		bool isGeneric = false;
		List<char*> genericParams;

		if (NextTokenIs(parser, TOKEN_TYPE_OP_LESS_THAN)) // Generic types
		{
			NextToken(parser); // <

			isGeneric = true;
			genericParams = CreateList<char*>();

			bool hasNext = !NextTokenIs(parser, TOKEN_TYPE_OP_GREATER_THAN);
			while (hasNext)
			{
				if (NextTokenIs(parser, TOKEN_TYPE_IDENTIFIER))
				{
					char* genericParamName = GetTokenString(NextToken(parser));
					genericParams.add(genericParamName);

					hasNext = NextTokenIs(parser, ',');
					if (hasNext)
						NextToken(parser); // ,
				}
				else
				{
					SnekAssert(false); // TODO ERROR
				}
			}

			SkipToken(parser, TOKEN_TYPE_OP_GREATER_THAN);
		}

		SkipToken(parser, '(');

		List<AST::Type*> paramTypes;
		List<char*> paramNames;
		List<AST::Expression*> paramValues;
		bool varArgs = false;

		bool upcomingDeclarator = !NextTokenIs(parser, ')');
		while (HasNext(parser) && upcomingDeclarator)
		{
			if (NextTokenIs(parser, '.'))
			{
				if (SkipToken(parser, '.') && SkipToken(parser, '.') && SkipToken(parser, '.'))
				{
					varArgs = true;
					upcomingDeclarator = false;
				}
				else
				{
					SnekError(parser->context, parser->lexer->input.state, "Variadic arguments need to be declared as '...'");
					parser->failed = true;
				}
			}
			else if (AST::Type* paramType = ParseType(parser))
			{
				if (NextTokenIs(parser, TOKEN_TYPE_IDENTIFIER))
				{
					char* paramName = GetTokenString(NextToken(parser));

					paramTypes.add(paramType);
					paramNames.add(paramName);

					AST::Expression* paramValue = nullptr;
					if (NextTokenIs(parser, TOKEN_TYPE_OP_EQUALS))
					{
						NextToken(parser); // =
						paramValue = ParseExpression(parser);
					}
					paramValues.add(paramValue);

					upcomingDeclarator = NextTokenIs(parser, ',');
					if (upcomingDeclarator)
						SkipToken(parser, ',');
				}
				else
				{
					SnekError(parser->context, parser->lexer->input.state, "Function parameter declaration needs a name");
					parser->failed = true;
				}
			}
			else
			{
				Token tok = NextToken(parser);
				SnekError(parser->context, parser->lexer->input.state, "Unexpected token: '%.*s'", tok.len, tok.str);
				parser->failed = true;
			}
		}

		SkipToken(parser, ')');


		AST::Type* returnType = nullptr;
		if (PeekToken(parser).type == TokenType::TOKEN_TYPE_OP_MINUS && PeekToken(parser, 1).type == TokenType::TOKEN_TYPE_OP_GREATER_THAN)
		{
			NextToken(parser); NextToken(parser); // ->

			returnType = ParseType(parser);
		}


		AST::Statement* body = nullptr;

		if (NextTokenIs(parser, ';'))
		{
			NextToken(parser); // ;
		}
		else
		{
			body = ParseStatement(parser);
		}

		InputState endInputState = GetInputState(parser);

		return new AST::Function(parser->module, inputState, flags, endInputState, name, returnType, paramTypes, paramNames, paramValues, varArgs, body, isGeneric, genericParams);
	}
	else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_STRUCT))
	{
		NextToken(parser); // struct
		if (NextTokenIs(parser, TOKEN_TYPE_IDENTIFIER))
		{
			char* name = GetTokenString(NextToken(parser));
			bool hasBody = false;
			List<AST::StructField*> fields;

			bool isGeneric = false;
			List<char*> genericParams;

			if (NextTokenIs(parser, TOKEN_TYPE_OP_LESS_THAN)) // Generic types
			{
				NextToken(parser); // <

				isGeneric = true;
				genericParams = CreateList<char*>();

				bool hasNext = !NextTokenIs(parser, TOKEN_TYPE_OP_GREATER_THAN);
				while (hasNext)
				{
					if (NextTokenIs(parser, TOKEN_TYPE_IDENTIFIER))
					{
						char* genericParamName = GetTokenString(NextToken(parser));
						genericParams.add(genericParamName);

						hasNext = NextTokenIs(parser, ',');
						if (hasNext)
							NextToken(parser); // ,
					}
					else
					{
						SnekAssert(false); // TODO ERROR
					}
				}

				SkipToken(parser, TOKEN_TYPE_OP_GREATER_THAN);
			}

			if (NextTokenIs(parser, '{'))
			{
				NextToken(parser); // {

				hasBody = true;

				bool upcomingMember = !NextTokenIs(parser, '}');
				while (HasNext(parser) && upcomingMember)
				{
					InputState fieldInputState = GetInputState(parser);
					if (AST::Type* type = ParseType(parser))
					{
						bool upcomingField = true;

						while (upcomingField && HasNext(parser))
						{
							if (NextTokenIs(parser, TOKEN_TYPE_IDENTIFIER))
							{
								char* name = GetTokenString(NextToken(parser));
								AST::StructField* field = new AST::StructField(parser->module, fieldInputState, type, name, fields.size);
								fields.add(field);
							}
							else
							{
								Token tok = NextToken(parser);
								SnekError(parser->context, parser->lexer->input.state, "Expected a field name: %.*s", tok.len, tok.str);
								SkipPastToken(parser, ';');
								parser->failed = true;
								break;
							}

							upcomingField = NextTokenIs(parser, ',');
							if (upcomingField)
							{
								NextToken(parser); // ,
								type = (AST::Type*)type->copy();
							}
						}
					}
					else
					{
						Token tok = NextToken(parser);
						SnekError(parser->context, parser->lexer->input.state, "Expected a type: %.*s", tok.len, tok.str);
						SkipPastToken(parser, ';');
						parser->failed = true;
					}

					SkipToken(parser, ';');

					upcomingMember = !NextTokenIs(parser, '}');
				}
				SkipToken(parser, '}');
			}
			else
			{
				SkipToken(parser, ';');
			}

			return new AST::Struct(parser->module, inputState, flags, name, hasBody, fields, isGeneric, genericParams);
		}
	}
	else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_CLASS))
	{
		NextToken(parser); // class
		if (NextTokenIs(parser, TOKEN_TYPE_IDENTIFIER))
		{
			char* className = GetTokenString(NextToken(parser));
			List<AST::ClassField*> fields;
			List<AST::Method*> methods;
			AST::Constructor* constructor = nullptr;

			if (NextTokenIs(parser, '{'))
			{
				NextToken(parser); // {

				bool upcomingMember = !NextTokenIs(parser, '}');
				while (HasNext(parser) && upcomingMember)
				{
					InputState memberInputState = GetInputState(parser);

					if (AST::Type* type = ParseType(parser))
					{
						if (NextTokenIs(parser, TOKEN_TYPE_IDENTIFIER))
						{
							char* name = GetTokenString(NextToken(parser));

							if (NextTokenIs(parser, '('))
							{
								NextToken(parser); // (

								List<AST::Type*> paramTypes;
								List<char*> paramNames;
								List<AST::Expression*> paramValues;
								bool varArgs = false;

								bool upcomingDeclarator = !NextTokenIs(parser, ')');
								while (HasNext(parser) && upcomingDeclarator)
								{
									if (NextTokenIs(parser, '.'))
									{
										if (SkipToken(parser, '.') && SkipToken(parser, '.') && SkipToken(parser, '.'))
										{
											varArgs = true;
											upcomingDeclarator = false;
										}
										else
										{
											SnekError(parser->context, parser->lexer->input.state, "Variadic arguments need to be declared as '...'");
											parser->failed = true;
										}
									}
									else if (AST::Type* paramType = ParseType(parser))
									{
										if (NextTokenIs(parser, TOKEN_TYPE_IDENTIFIER))
										{
											char* paramName = GetTokenString(NextToken(parser));

											paramTypes.add(paramType);
											paramNames.add(paramName);

											AST::Expression* paramValue = nullptr;
											if (NextTokenIs(parser, TOKEN_TYPE_OP_EQUALS))
											{
												NextToken(parser); // =
												paramValue = ParseExpression(parser);
											}
											paramValues.add(paramValue);

											upcomingDeclarator = NextTokenIs(parser, ',');
											if (upcomingDeclarator)
												SkipToken(parser, ',');
										}
										else
										{
											SnekError(parser->context, parser->lexer->input.state, "Function parameter declaration needs a name");
											parser->failed = true;
										}
									}
									else
									{
										Token tok = NextToken(parser);
										SnekError(parser->context, parser->lexer->input.state, "Unexpected token: %.*s", tok.len, tok.str);
										parser->failed = true;
									}
								}

								SkipToken(parser, ')');

								AST::Statement* body = NULL;

								if (NextTokenIs(parser, ';'))
								{
									NextToken(parser); // ;
								}
								else
								{
									body = ParseStatement(parser);
								}

								InputState endInputState = GetInputState(parser);

								AST::Method* method = new AST::Method(parser->module, inputState, flags, endInputState, name, type, paramTypes, paramNames, paramValues, varArgs, body, false, {});
								methods.add(method);
							}
							else
							{
								bool upcomingField = true;

								while (upcomingField && HasNext(parser))
								{
									AST::ClassField* field = new AST::ClassField(parser->module, memberInputState, type, name, fields.size);
									fields.add(field);

									upcomingField = NextTokenIs(parser, ',');
									if (upcomingField)
									{
										NextToken(parser); // ,
										name = GetTokenString(NextToken(parser));
										type = (AST::Type*)type->copy();
									}
								}

								SkipToken(parser, ';');
							}
						}
						else
						{
							if (type->typeKind == AST::TypeKind::Function && ((AST::FunctionType*)type)->returnType->typeKind == AST::TypeKind::NamedType && strcmp(((AST::NamedType*)((AST::FunctionType*)type)->returnType)->name, className) == 0)
							{
								SetInputState(parser, memberInputState);

								// Constructor declaration

								NextToken(parser); // class name
								SkipToken(parser, '(');

								List<AST::Type*> paramTypes;
								List<char*> paramNames;
								List<AST::Expression*> paramValues;
								bool varArgs = false;

								bool upcomingDeclarator = !NextTokenIs(parser, ')');
								while (HasNext(parser) && upcomingDeclarator)
								{
									if (NextTokenIs(parser, '.'))
									{
										if (SkipToken(parser, '.') && SkipToken(parser, '.') && SkipToken(parser, '.'))
										{
											varArgs = true;
											upcomingDeclarator = false;
										}
										else
										{
											SnekError(parser->context, parser->lexer->input.state, "Variadic arguments need to be declared as '...'");
											parser->failed = true;
										}
									}
									else if (AST::Type* paramType = ParseType(parser))
									{
										if (NextTokenIs(parser, TOKEN_TYPE_IDENTIFIER))
										{
											char* paramName = GetTokenString(NextToken(parser));

											paramTypes.add(paramType);
											paramNames.add(paramName);

											AST::Expression* paramValue = nullptr;
											if (NextTokenIs(parser, TOKEN_TYPE_OP_EQUALS))
											{
												NextToken(parser); // =
												paramValue = ParseExpression(parser);
											}
											paramValues.add(paramValue);

											upcomingDeclarator = NextTokenIs(parser, ',');
											if (upcomingDeclarator)
												SkipToken(parser, ',');
										}
										else
										{
											SnekError(parser->context, parser->lexer->input.state, "Function parameter declaration needs a name");
											parser->failed = true;
										}
									}
									else
									{
										Token tok = NextToken(parser);
										SnekError(parser->context, parser->lexer->input.state, "Unexpected token: %.*s", tok.len, tok.str);
										parser->failed = true;
									}
								}

								SkipToken(parser, ')');

								AST::Statement* body = NULL;

								if (NextTokenIs(parser, ';'))
								{
									NextToken(parser); // ;
								}
								else
								{
									body = ParseStatement(parser);
								}

								InputState endInputState = GetInputState(parser);

								AST::Type* constructorReturnType = new AST::NamedType(parser->module, inputState, className, false, {});

								constructor = new AST::Constructor(parser->module, inputState, flags, endInputState, nullptr, constructorReturnType, paramTypes, paramNames, paramValues, varArgs, body, false, {});
							}
							else
							{
								Token tok = NextToken(parser);
								SnekError(parser->context, parser->lexer->input.state, "Expected a field name: %.*s", tok.len, tok.str);
								SkipPastToken(parser, ';');
								parser->failed = true;
							}
						}
					}
					else
					{
						Token tok = NextToken(parser);
						SnekError(parser->context, parser->lexer->input.state, "Expected a type: %.*s", tok.len, tok.str);
						SkipPastToken(parser, ';');
						parser->failed = true;
					}

					upcomingMember = !NextTokenIs(parser, '}');
				}
				SkipToken(parser, '}');
			}
			else
			{
				SkipToken(parser, ';');
			}

			return new AST::Class(parser->module, inputState, flags, className, fields, methods, constructor);
		}
	}
	else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_TYPEDEF))
	{
		NextToken(parser); // typedef

		char* name = GetTokenString(NextToken(parser));
		if (SkipToken(parser, ':'))
		{
			AST::Type* alias = ParseType(parser);

			SkipToken(parser, ';');

			return new AST::Typedef(parser->module, inputState, flags, name, alias);
		}
		else
		{
			SkipPastToken(parser, ';');
			return NULL;
		}
	}
	else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_ENUM))
	{
		NextToken(parser); // enum

		char* name = GetTokenString(NextToken(parser));
		AST::Type* alias = NULL;
		List<AST::EnumValue*> values;

		if (NextTokenIs(parser, ':'))
		{
			NextToken(parser); // :
			alias = ParseType(parser);
		}

		SkipToken(parser, '{');

		bool upcomingEntry = NextTokenIs(parser, TOKEN_TYPE_IDENTIFIER);

		while (upcomingEntry && HasNext(parser))
		{
			if (NextTokenIs(parser, TOKEN_TYPE_IDENTIFIER))
			{
				char* entryName = GetTokenString(NextToken(parser));
				AST::Expression* entryValue = NULL;
				if (NextTokenIs(parser, TOKEN_TYPE_OP_EQUALS))
				{
					NextToken(parser); // =
					entryValue = ParseExpression(parser);
				}

				AST::EnumValue* value = new AST::EnumValue(parser->module, inputState, entryName, entryValue);
				values.add(value);

				upcomingEntry = NextTokenIs(parser, ',') && !NextTokenIs(parser, '}', 1);
				if (NextTokenIs(parser, ','))
					NextToken(parser); // ,
			}
			else
			{
				// ERROR
			}
		}

		SkipToken(parser, '}');

		return new AST::Enum(parser->module, inputState, flags, name, alias, values);
	}
	else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_MACRO))
	{
		NextToken(parser); // exprdef

		char* name = GetTokenString(NextToken(parser));
		if (SkipToken(parser, TOKEN_TYPE_OP_EQUALS))
		{
			AST::Expression* expr = ParseExpression(parser);

			SkipToken(parser, ';');

			return new AST::Macro(parser->module, inputState, flags, name, expr);
		}
		else
		{
			SkipPastToken(parser, ';');
			return NULL;
		}
	}
	else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_MODULE))
	{
		NextToken(parser); // module
		if (NextTokenIs(parser, TOKEN_TYPE_IDENTIFIER))
		{
			AST::ModuleIdentifier identifier;

			bool upcomingNamespace = true;
			while (upcomingNamespace && HasNext(parser))
			{
				char* name = GetTokenString(NextToken(parser));
				identifier.namespaces.add(name);

				upcomingNamespace = NextTokenIs(parser, '.');
				if (upcomingNamespace)
					NextToken(parser); // .
			}
			SkipToken(parser, ';');

			return new AST::ModuleDeclaration(parser->module, inputState, flags, identifier);
		}
		else
		{
			Token tok = NextToken(parser);
			SnekError(parser->context, parser->lexer->input.state, "Expected a module name: %.*s", tok.len, tok.str);
			parser->failed = true;
			return NULL;
		}
	}
	else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_NAMESPACE))
	{
		NextToken(parser); // namespace
		char* nameSpace = GetTokenString(NextToken(parser));
		SkipToken(parser, ';');

		return new AST::NamespaceDeclaration(parser->module, inputState, flags, nameSpace);
	}
	else if (NextTokenIsKeyword(parser, KEYWORD_TYPE_IMPORT))
	{
		NextToken(parser); // import
		if (NextTokenIs(parser, TOKEN_TYPE_IDENTIFIER))
		{
			List<AST::ModuleIdentifier> imports;

			bool upcomingImport = true;
			while (upcomingImport && HasNext(parser))
			{
				AST::ModuleIdentifier identifier;

				bool upcomingModule = true;
				while (upcomingModule && HasNext(parser))
				{
					if (!(NextTokenIs(parser, TOKEN_TYPE_IDENTIFIER) || NextTokenIs(parser, TOKEN_TYPE_OP_ASTERISK)))
					{
						SkipPastToken(parser, ';');
						break;
					}

					char* name = GetTokenString(NextToken(parser));
					identifier.namespaces.add(name);

					upcomingModule = NextTokenIs(parser, '.');
					if (upcomingModule)
						NextToken(parser); // .
				}

				imports.add(identifier);

				upcomingImport = NextTokenIs(parser, ',');
				if (upcomingImport)
					NextToken(parser); // ,
			}
			SkipToken(parser, ';');

			return new AST::Import(parser->module, inputState, flags, imports);
		}
		else
		{
			Token tok = NextToken(parser);
			SnekError(parser->context, parser->lexer->input.state, "Expected a module name: %.*s", tok.len, tok.str);
			parser->failed = true;
			return NULL;
		}
	}
	else if (AST::Type* type = ParseType(parser))
	{
		if (NextTokenIs(parser, TOKEN_TYPE_IDENTIFIER))
		{
			// Global variable declaration

			char* name = GetTokenString(NextToken(parser));

			bool isGeneric = false;
			List<char*> genericParams;

			if (NextTokenIs(parser, TOKEN_TYPE_OP_LESS_THAN)) // Generic types
			{
				NextToken(parser); // <

				isGeneric = true;
				genericParams = CreateList<char*>();

				bool hasNext = !NextTokenIs(parser, TOKEN_TYPE_OP_GREATER_THAN);
				while (hasNext)
				{
					if (NextTokenIs(parser, TOKEN_TYPE_IDENTIFIER))
					{
						char* genericParamName = GetTokenString(NextToken(parser));
						genericParams.add(genericParamName);

						hasNext = NextTokenIs(parser, ',');
						if (hasNext)
							NextToken(parser); // ,
					}
					else
					{
						SnekAssert(false); // TODO ERROR
					}
				}

				SkipToken(parser, TOKEN_TYPE_OP_GREATER_THAN);
			}


			List<AST::VariableDeclarator*> declarators;

			SnekAssert(!isGeneric);

			bool upcomingDeclarator = true;
			while (HasNext(parser) && upcomingDeclarator)
			{
				InputState inputState = GetInputState(parser);

				AST::Expression* value = NULL;

				if (NextTokenIs(parser, TOKEN_TYPE_OP_EQUALS))
				{
					NextToken(parser); // =
					value = ParseExpression(parser);
				}

				upcomingDeclarator = NextTokenIs(parser, ',');
				if (upcomingDeclarator)
					SkipToken(parser, ',');

				AST::VariableDeclarator* declarator = new AST::VariableDeclarator(parser->module, inputState, name, value);
				declarators.add(declarator);
			}

			SkipToken(parser, ';');

			return new AST::GlobalVariable(parser->module, inputState, flags, type, declarators);
		}
	}

	Token token = NextToken(parser);
	SnekError(parser->context, parser->lexer->input.state, "Unexpected token '%.*s'", token.len, token.str);
	parser->failed = true;

	SkipPastToken(parser, ';');

	return nullptr;
}

Parser::Parser(CGLCompiler* context)
	: context(context)
{
}

Parser::~Parser()
{
}

AST::File* Parser::run(SourceFile& sourceFile)
{
	AST::File* file = new AST::File(sourceFile.name, 0, &sourceFile);

	module = file;
	lexer = new Lexer(context, sourceFile.filename, sourceFile.source);

	while (HasNext(this))
	{
		if (AST::Declaration* decl = ParseDeclaration(this))
		{
			switch (decl->type)
			{
			case AST::DeclarationType::Function:
				file->functions.add((AST::Function*)decl);
				break;
			case AST::DeclarationType::Struct:
				file->structs.add((AST::Struct*)decl);
				break;
			case AST::DeclarationType::Class:
				file->classes.add((AST::Class*)decl);
				break;
			case AST::DeclarationType::Typedef:
				file->typedefs.add((AST::Typedef*)decl);
				break;
			case AST::DeclarationType::Enumeration:
				file->enums.add((AST::Enum*)decl);
				break;
			case AST::DeclarationType::Macro:
				file->macros.add((AST::Macro*)decl);
				break;
			case AST::DeclarationType::GlobalVariable: {
				AST::GlobalVariable* global = (AST::GlobalVariable*)decl;
				file->globals.add(global);

				for (int i = 1; i < global->declarators.size; i++)
				{
					AST::VariableDeclarator* declarator = global->declarators[i];
					List<AST::VariableDeclarator*> declaratorList;
					declaratorList.add(declarator);
					AST::GlobalVariable* next = new AST::GlobalVariable(declarator->file, declarator->location, global->flags, (AST::Type*)global->varType->copy(), declaratorList);
					file->globals.add(next);
				}

				while (global->declarators.size > 1)
					global->declarators.removeAt(1);

				break;
			}
			case AST::DeclarationType::Module:
				file->moduleDecl = (AST::ModuleDeclaration*)decl;
				break;
			case AST::DeclarationType::Namespace:
				file->namespaceDecl = (AST::NamespaceDeclaration*)decl;
				break;
			case AST::DeclarationType::Import:
				file->imports.add((AST::Import*)decl);
				break;
			default:
				SnekAssert(false);
				break;
			}
		}
	}

	return file;
}
