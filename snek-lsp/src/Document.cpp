#include "Document.h"

#include "Platform.h"
#include "utils/Hash.h"

#include <iostream>
#include <sstream>
#include <set>


using namespace nlohmann;


static int LSPTokenComparator(LSPToken const* a, LSPToken const* b)
{
	return (a->token.line < b->token.line || a->token.line == b->token.line && a->token.col < b->token.col) ? -1 :
		a->token.line == b->token.line && a->token.col == b->token.col ? 0 : 1;
}

void Document::init(const std::string& text)
{
	std::istringstream stream(text);
	std::string line;
	while (std::getline(stream, line))
	{
		lines.add(_strdup(line.c_str()));
	}

	lastChange = GetTimeNS();
}

void Document::onChange(int startLine, int startCol, int endLine, int endCol, std::string& text)
{
	List<std::string> changeLines;

	if (text == "")
	{
		changeLines.add("");
	}
	else
	{
		std::istringstream stream(text);
		std::string line;
		while (std::getline(stream, line))
		{
			changeLines.add(line);
		}
	}

	std::string prefix = std::string(lines[startLine]).substr(0, startCol);
	std::string suffix = std::string(lines[endLine]).substr(endCol);

	changeLines[0] = prefix + changeLines[0];
	changeLines[changeLines.size - 1] = changeLines[changeLines.size - 1] + suffix;

	linesMutex.lock();

	int linesToRemove = endLine - startLine + 1;
	for (int i = 0; i < linesToRemove; i++)
	{
		free(lines[startLine]);
		lines.removeAt(startLine);
	}

	for (int i = 0; i < changeLines.size; i++)
	{
		lines.insert(startLine + i, _strdup(changeLines[i].c_str()));
	}

	linesMutex.unlock();

	FreeList(&changeLines);

	lastChange = GetTimeNS();
}

void Document::getTokens(std::vector<int>& data)
{
	std::vector<LSPToken> lspTokens;

	auto addToken = [&lspTokens](Token token, int type, int modifiers)
		{
			lspTokens.push_back({ token, type, modifiers });
		};


	std::set<uint32_t> functionNames;
	std::set<uint32_t> structNames;
	std::set<uint32_t> enumNames;

	tokensMutex.lock();

	for (int i = 0; i < tokens.size; i++)
	{
		Token token = tokens[i];

		if (token.type == TOKEN_FUNCTION)
		{
			functionNames.insert(hash(tokens[++i].text));
		}
		else if (token.type = TOKEN_STRUCT)
		{
			structNames.insert(hash(tokens[++i].text));
		}
		else if (token.type = TOKEN_ENUM)
		{
			enumNames.insert(hash(tokens[++i].text));
		}
		/*
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
			case TOKEN_TYPE_COMMENT:
				addToken(token, LSP_TOKEN_COMMENT, 0);
				break;
			default:
				break;
			}
		}
		*/
	}

	for (int i = 0; i < tokens.size; i++)
	{
		Token token = tokens[i];
		uint32_t h = hash(token.text);

		if (functionNames.find(h) != functionNames.end())
		{
			addToken(token, LSP_TOKEN_FUNCTION, 0);
		}
		else if (structNames.find(h) != structNames.end())
		{
			addToken(token, LSP_TOKEN_STRUCT, 0);
		}
		else if (enumNames.find(h) != enumNames.end())
		{
			addToken(token, LSP_TOKEN_ENUM, 0);
		}
	}

	tokensMutex.unlock();

	/*
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
	for (int i = 0; i < ast->typedefs.size; i++)
	{
		AST::Typedef* def = ast->typedefs[i];
		addToken(def->nameToken, LSP_TOKEN_TYPE, 0);
	}
	for (int i = 0; i < ast->enums.size; i++)
	{
		AST::Enum* en = ast->enums[i];
		addToken(en->nameToken, LSP_TOKEN_ENUM, 0);
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
	*/

	qsort(lspTokens.data(), lspTokens.size(), sizeof(LSPToken), (_CoreCrtNonSecureSearchSortCompareFunction)LSPTokenComparator);

	int lastLine = 0, lastCol = 0;
	for (int i = 0; i < (int)lspTokens.size(); i++)
	{
		LSPToken token = lspTokens[i];
		int line = token.token.line - 1;
		int col = token.token.col - 1;
		int len = token.token.text.length;

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
