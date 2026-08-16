#pragma once

#include <string>
#include <vector>
#include <mutex>

#include <nlohmann/json.hpp>

#include "File.h"


enum LSPTokenType
{
	LSP_TOKEN_NAMESPACE,
	LSP_TOKEN_TYPE,
	LSP_TOKEN_CLASS,
	LSP_TOKEN_ENUM,
	LSP_TOKEN_INTERFACE,
	LSP_TOKEN_STRUCT,
	LSP_TOKEN_TYPE_PARAMETER,
	LSP_TOKEN_PARAMETER,
	LSP_TOKEN_VARIABLE,
	LSP_TOKEN_PROPERTY,
	LSP_TOKEN_ENUM_VALUE,
	LSP_TOKEN_EVENT,
	LSP_TOKEN_FUNCTION,
	LSP_TOKEN_METHOD,
	LSP_TOKEN_MACRO,
	LSP_TOKEN_KEYWORD,
	LSP_TOKEN_MODIFIER,
	LSP_TOKEN_COMMENT,
	LSP_TOKEN_STRING,
	LSP_TOKEN_NUMBER,
	LSP_TOKEN_REGEXP,
	LSP_TOKEN_OPERATOR,
};

enum LSPTokenModifier
{
	LSP_TOKEN_MODIFIER_DECLARATION,
	LSP_TOKEN_MODIFIER_DEFINITION,
	LSP_TOKEN_MODIFIER_READONLY,
	LSP_TOKEN_MODIFIER_STATIC,
	LSP_TOKEN_MODIFIER_DEPRECATED,
};

struct LSPToken
{
	int offset;
	int length;
	int type;
	int modifiers;
};

enum CompletionItemType
{
	COMPLETION_ITEM_TEXT = 1,
	COMPLETION_ITEM_METHOD = 2,
	COMPLETION_ITEM_FUNCTION = 3,
	COMPLETION_ITEM_CONSTRUCTOR = 4,
	COMPLETION_ITEM_FIELD = 5,
	COMPLETION_ITEM_VARIABLE = 6,
	COMPLETION_ITEM_CLASS = 7,
	COMPLETION_ITEM_INTERFACE = 8,
	COMPLETION_ITEM_MODULE = 9,
	COMPLETION_ITEM_PROPERTY = 10,
	COMPLETION_ITEM_UNIT = 11,
	COMPLETION_ITEM_VALUE = 12,
	COMPLETION_ITEM_ENUM = 13,
	COMPLETION_ITEM_KEYWORD = 14,
	COMPLETION_ITEM_SNIPPET = 15,
	COMPLETION_ITEM_COLOR = 16,
	COMPLETION_ITEM_FILE = 17,
	COMPLETION_ITEM_REFERENCE = 18,
	COMPLETION_ITEM_FOLDER = 19,
	COMPLETION_ITEM_ENUM_MEMBER = 20,
	COMPLETION_ITEM_CONSTANT = 21,
	COMPLETION_ITEM_STRUCT = 22,
	COMPLETION_ITEM_EVENT = 23,
	COMPLETION_ITEM_OPERATOR = 24,
	COMPLETION_ITEM_TYPE_PARAMETER = 25,
};

enum SymbolKind
{
	SYMBOL_KIND_FILE = 1,
	SYMBOL_KIND_MODULE = 2,
	SYMBOL_KIND_NAMESPACE = 3,
	SYMBOL_KIND_PACKAGE = 4,
	SYMBOL_KIND_CLASS = 5,
	SYMBOL_KIND_METHOD = 6,
	SYMBOL_KIND_PROPERTY = 7,
	SYMBOL_KIND_FIELD = 8,
	SYMBOL_KIND_CONSTRUCTOR = 9,
	SYMBOL_KIND_ENUM = 10,
	SYMBOL_KIND_INTERFACE = 11,
	SYMBOL_KIND_FUNCTION = 12,
	SYMBOL_KIND_VARIABLE = 13,
	SYMBOL_KIND_CONSTANT = 14,
	SYMBOL_KIND_STRING = 15,
	SYMBOL_KIND_NUMBER = 16,
	SYMBOL_KIND_BOOLEAN = 17,
	SYMBOL_KIND_ARRAY = 18,
	SYMBOL_KIND_OBJECT = 19,
	SYMBOL_KIND_KEY = 20,
	SYMBOL_KIND_NULL = 21,
	SYMBOL_KIND_ENUM_MEMBER = 22,
	SYMBOL_KIND_STRUCT = 23,
	SYMBOL_KIND_EVENT = 24,
	SYMBOL_KIND_OPERATOR = 25,
	SYMBOL_KIND_TYPE_PARAMETER = 26,
};

struct DocumentPiece
{
	int start, length;
	std::string text;
};

enum DocumentState
{
	DOCUMENT_STATE_NULL = 0,

	DOCUMENT_STATE_UNPARSED,
	DOCUMENT_STATE_PARSED,
	DOCUMENT_STATE_TYPECHECKED,
};

struct TypeSystem;
struct GlobalBlockPool;

struct Document
{
	std::string uri;
	std::string localPath;
	List<char*> lines;
	std::mutex linesMutex;

	bool open;
	DocumentState state;
	bool needsTypeCheck;
	uint64_t lastChange = 0;

	std::string text;
	std::mutex astMutex;
	File file;


	void init(const std::string& text, TypeSystem* types, GlobalBlockPool* blockPool);
	void onOpen(std::string& text);
	void onChange(int startLine, int startCol, int endLine, int endCol, std::string& text);
	void getTokens(std::vector<int>& data);

	void getNodeAtPosition(int line, int col, Node** node, Scope** scope);
	void autocomplete(Scope* scope, nlohmann::json& items);

	void getSymbols(nlohmann::json& items);
	void getWorkspaceSymbols(const std::string& query, nlohmann::json& items);
};


File* getFileFromHandle(FileHandle fileHandle);
