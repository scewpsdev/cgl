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
	return a->offset < b->offset ? -1 : a->offset == b->offset ? 0 : 1;
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

static void getNodeTokens(Node* node, List<LSPToken>* lspTokens)
{
	if (node->type == NODE_NAMED_TYPE)
	{
		if (Node* type = resolveSymbol(node->namedType.name))
		{
			LSPTokenType tokenType = type->type == NODE_STRUCT ? LSP_TOKEN_STRUCT : LSP_TOKEN_TYPE;
			lspTokens->add({ node->start, node->end - node->start, tokenType, 0 });
		}
	}
}

void Document::getTokens(std::vector<int>& data)
{
	List<LSPToken> lspTokens;

	astMutex.lock();

	if (hasAST)
	{
		traverseAST(&ast, (ASTVisitor_t)getNodeTokens, &lspTokens);
	}

	astMutex.unlock();

	qsort(lspTokens.buffer, lspTokens.size, sizeof(LSPToken), (_CoreCrtNonSecureSearchSortCompareFunction)LSPTokenComparator);

	int lastLine = 0, lastCol = 0;
	for (int i = 0; i < lspTokens.size; i++)
	{
		LSPToken token = lspTokens[i];
		int line, col;
		getCoordFromOffset(token.offset, text.c_str(), &line, &col);
		int len = token.length;

		// deltaLine, deltaStart, length, tokenType, tokenModifiers
		data.push_back(line - lastLine);
		data.push_back(line == lastLine ? col - lastCol : col);
		data.push_back(len);
		data.push_back(token.type);
		data.push_back(token.modifiers);

		lastLine = line;
		lastCol = col;
	}

	FreeList(&lspTokens);
}
