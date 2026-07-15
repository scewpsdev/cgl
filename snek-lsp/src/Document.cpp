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

	astMutex.lock();

	// create tokens

	astMutex.unlock();

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
