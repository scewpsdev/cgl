#include "Document.h"

#include "Platform.h"
#include "utils/Hash.h"

#include <iostream>
#include <sstream>
#include <set>


using namespace nlohmann;


extern List<Document*> documents;


static int LSPTokenComparator(LSPToken const* a, LSPToken const* b)
{
	return a->token.offset < b->token.offset ? -1 : a->token.offset == b->token.offset ? 0 : 1;
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

	hasAST = false;
}

void Document::onChange(int startLine, int startCol, int endLine, int endCol, std::string& text)
{
	List<char*> changeLines;

	if (text == "")
	{
		changeLines.add(_strdup(""));
	}
	else
	{
		std::istringstream stream(text);
		std::string line;
		while (std::getline(stream, line))
		{
			changeLines.add(_strdup(line.c_str()));
		}

		if (text.back() == '\n')
		{
			changeLines.add(_strdup(""));
		}
	}

	char* prefix = substring(lines[startLine], 0, startCol);
	char* suffix = substring(lines[endLine], endCol);

	changeLines[0] = concatDelete(prefix, changeLines[0]);
	changeLines[changeLines.size - 1] = concatDelete(changeLines[changeLines.size - 1], suffix);

	linesMutex.lock();

	int linesToRemove = endLine - startLine + 1;
	for (int i = 0; i < linesToRemove; i++)
	{
		free(lines[startLine]);
		lines.removeAt(startLine);
	}

	for (int i = 0; i < changeLines.size; i++)
	{
		lines.insert(startLine + i, changeLines[i]);
	}

	linesMutex.unlock();

	FreeList(&changeLines);

	lastChange = GetTimeNS();
}

static void getCoordFromOffset(int offset, const char* src, int* line, int* col)
{
	*line = 0;
	*col = 0;
	for (int i = 0; i < offset; i++)
	{
		if (src[i] == '\n')
		{
			*line += 1;
			*col = 0;
		}
		else
		{
			*col += 1;
		}
	}
}

static Node* resolveSymbol(StringView identifier)
{
	for (int i = 0; i < documents.size; i++)
	{
		if (Node* symbol = lookupSymbol(&documents[i]->ast.globalScope->symbols, identifier))
			return symbol;
	}
	return nullptr;
}

void Document::getTokens(std::vector<int>& data)
{
	std::vector<LSPToken> lspTokens;

	auto addToken = [&lspTokens](Token token, int type, int modifiers)
		{
			lspTokens.push_back({ token, type, modifiers });
		};


	std::map<uint32_t, Node*> nameMap;

	astMutex.lock();

	if (hasAST)
	{
		for (int i = 0; i < tokens.size; i++)
		{
			if (tokens[i].type == TOKEN_IDENTIFIER)
			{
				StringView identifier = getTokenString(tokens[i], text.c_str());
				if (Node* node = resolveSymbol(identifier))
				{
					int type = 0, modifiers = 0;

					if (node->type == NODE_STRUCT)
						type = LSP_TOKEN_STRUCT;
					else if (node->type == NODE_ENUM)
						type = LSP_TOKEN_ENUM;
					else if (node->type == NODE_UNION)
						type = LSP_TOKEN_STRUCT;
					else if (node->type == NODE_TYPEDEF)
						type = LSP_TOKEN_STRUCT;
					else if (node->type == NODE_FUNCTION)
						type = LSP_TOKEN_FUNCTION;
					else if (node->type == NODE_MACRO)
						type = LSP_TOKEN_MACRO;

					if (type)
					{
						addToken(tokens[i], type, modifiers);
					}
				}
			}
		}
	}

	astMutex.unlock();

	qsort(lspTokens.data(), lspTokens.size(), sizeof(LSPToken), (_CoreCrtNonSecureSearchSortCompareFunction)LSPTokenComparator);

	int lastLine = 0, lastCol = 0;
	for (int i = 0; i < (int)lspTokens.size(); i++)
	{
		LSPToken token = lspTokens[i];
		int line, col;
		getCoordFromOffset(token.token.offset, text.c_str(), &line, &col);
		int len = token.token.length;

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
