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

struct ASTVisitorData
{
	Parser* parser;
	List<LSPToken>* lspTokens;
};

static void getStringRange(StringView str, Parser* parser, int* start, int* end)
{
	*start = (int)(str.ptr - parser->lexer.src);
	*end = *start + str.length;
}

static void getNodeTokens(Node* node, ASTVisitorData* data)
{
	if (node->type == NODE_STRUCT)
	{
		Struct* struct_ = &node->struct_;
		int start, end;
		getStringRange(struct_->name, data->parser, &start, &end);
		data->lspTokens->add({ start, end - start, LSP_TOKEN_STRUCT, 0 });
	}
	else if (node->type == NODE_GLOBAL_VARIABLE)
	{
		GlobalVariable* globalVariable = &node->globalVariable;
		for (int i = 0; i < globalVariable->numNames; i++)
		{
			int start, end;
			getStringRange(globalVariable->names[i], data->parser, &start, &end);
			data->lspTokens->add({ start, end - start, LSP_TOKEN_VARIABLE, 0 });
		}
	}
	else if (node->type == NODE_NAMED_TYPE)
	{
		if (Node* type = resolveSymbol(node->namedType.name))
		{
			LSPTokenType tokenType = type->type == NODE_STRUCT ? LSP_TOKEN_STRUCT : LSP_TOKEN_TYPE;
			data->lspTokens->add({ node->start, node->end - node->start, tokenType, 0 });
		}
	}
}

void Document::getTokens(std::vector<int>& data)
{
	if (!hasAST)
		return;

	List<LSPToken> lspTokens;

	astMutex.lock();

	ASTVisitorData visitorData = {};
	visitorData.parser = &parser;
	visitorData.lspTokens = &lspTokens;
	traverseAST(&ast, (ASTVisitor_t)getNodeTokens, &visitorData);

	astMutex.unlock();

	qsort(lspTokens.buffer, lspTokens.size, sizeof(LSPToken), (_CoreCrtNonSecureSearchSortCompareFunction)LSPTokenComparator);

	int lastLine = 0, lastCol = 0;
	for (int i = 0; i < lspTokens.size; i++)
	{
		LSPToken token = lspTokens[i];
		SourceLocation location = getSourceLocation(&parser.lexer, token.offset);
		int len = token.length;

		// deltaLine, deltaStart, length, tokenType, tokenModifiers
		data.push_back(location.line - lastLine);
		data.push_back(location.line == lastLine ? location.col - lastCol : location.col);
		data.push_back(len);
		data.push_back(token.type);
		data.push_back(token.modifiers);

		lastLine = location.line;
		lastCol = location.col;
	}

	FreeList(&lspTokens);
}
