#include "Document.h"

#include "cgl/CGLCompiler.h"
#include "cgl/Platform.h"


using namespace nlohmann;


static int LSPTokenComparator(LSPToken const* a, LSPToken const* b)
{
	return (a->token.line < b->token.line || a->token.line == b->token.line && a->token.col < b->token.col) ? -1 :
		a->token.line == b->token.line && a->token.col == b->token.col ? 0 : 1;
}

void Document::getTokens(AST::File* ast, CGLCompiler* compiler, std::vector<int>& data)
{
	std::vector<LSPToken> tokens;

	auto addToken = [&tokens](Token token, int type, int modifiers)
		{
			tokens.push_back({ token, type, modifiers });
		};

	Lexer lexer(compiler, uri.c_str(), text.c_str());

	while (LexerHasNext(&lexer))
	{
		Token token = LexerNext(&lexer);

		if (token.keywordType != KEYWORD_TYPE_NULL)
		{
			if (token.keywordType > KEYWORD_TYPE_BUILTIN_TYPES_START && token.keywordType < KEYWORD_TYPE_BUILTIN_TYPES_END)
				addToken(token, LSP_TOKEN_TYPE, 0);
			else
				addToken(token, LSP_TOKEN_KEYWORD, 0);
		}
		else
		{
			switch (token.type)
			{
			case TOKEN_TYPE_STRING_LITERAL:
			case TOKEN_TYPE_CHAR_LITERAL:
				addToken(token, LSP_TOKEN_STRING, 0);
				break;
			case TOKEN_TYPE_STRING_LITERAL_MULTILINE:
				break;
			case TOKEN_TYPE_FLOAT_LITERAL:
			case TOKEN_TYPE_DOUBLE_LITERAL:
			case TOKEN_TYPE_INT_LITERAL:
				addToken(token, LSP_TOKEN_NUMBER, 0);
				break;
			case TOKEN_TYPE_OP_BEGIN:
			case TOKEN_TYPE_OP_PLUS:
			case TOKEN_TYPE_OP_MINUS:
			case TOKEN_TYPE_OP_ASTERISK:
			case TOKEN_TYPE_OP_SLASH:
			case TOKEN_TYPE_OP_PERCENT:
			case TOKEN_TYPE_OP_AMPERSAND:
			case TOKEN_TYPE_OP_OR:
			case TOKEN_TYPE_OP_CARET:
			case TOKEN_TYPE_OP_QUESTION:
			case TOKEN_TYPE_OP_EXCLAMATION:
			case TOKEN_TYPE_OP_EQUALS:
			case TOKEN_TYPE_OP_LESS_THAN:
			case TOKEN_TYPE_OP_GREATER_THAN:
				addToken(token, LSP_TOKEN_OPERATOR, 0);
				break;
			case TOKEN_TYPE_IDENTIFIER:
				break;
			default:
				break;
			}
		}
	}

	for (int i = 0; i < ast->functions.size; i++)
	{
		AST::Function* function = ast->functions[i];
		addToken(function->nameToken, LSP_TOKEN_FUNCTION, 0);
	}
	for (int i = 0; i < ast->structs.size; i++)
	{
		AST::Struct* strct = ast->structs[i];
		addToken(strct->nameToken, LSP_TOKEN_STRUCT, 0);
	}

	for (int i = 0; i < ast->identifiers.size; i++)
	{
		AST::Identifier* identifier = ast->identifiers[i];
		int type = identifier->functions.size ? LSP_TOKEN_FUNCTION :
			//identifier->variable ? LSP_TOKEN_VARIABLE :
			identifier->enumValue ? LSP_TOKEN_ENUM_VALUE :
			identifier->exprdefValue ? LSP_TOKEN_MACRO :
			identifier->enumDecl ? LSP_TOKEN_ENUM :
			identifier->module ? LSP_TOKEN_NAMESPACE :
			identifier->builtinType ? LSP_TOKEN_TYPE :
			-1;
		if (type != -1)
			addToken(identifier->nameToken, type, 0);
	}
	for (int i = 0; i < ast->namedTypes.size; i++)
	{
		AST::NamedType* namedType = ast->namedTypes[i];
		if (namedType && namedType->typeID)
		{
			int type = namedType->typeID->typeKind == AST::TypeKind::Struct ? LSP_TOKEN_STRUCT :
				namedType->typeID->typeKind == AST::TypeKind::Class ? LSP_TOKEN_CLASS :
				namedType->declaration->type == AST::DeclarationType::Enumeration ? LSP_TOKEN_ENUM :
				LSP_TOKEN_TYPE;
			if (type != -1)
				addToken(namedType->nameToken, type, 0);
		}
	}

	qsort(tokens.data(), tokens.size(), sizeof(LSPToken), (_CoreCrtNonSecureSearchSortCompareFunction)LSPTokenComparator);

	int lastLine = 0, lastCol = 0;
	for (int i = 0; i < (int)tokens.size(); i++)
	{
		LSPToken token = tokens[i];
		int line = token.token.line - 1;
		int col = token.token.col - 1;
		int len = token.token.len;

		// deltaLine, deltaStart, length, tokenType, tokenModifiers
		data.push_back(line - lastLine);
		data.push_back(line == lastLine ? col - lastCol : col);
		data.push_back(len);
		data.push_back(token.type);
		data.push_back(token.modifiers);

		lastLine = line;
		lastCol = col;
	}
}
