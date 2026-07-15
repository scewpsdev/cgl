#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <thread>
#include <stdio.h>
#include <stdarg.h>

#include <nlohmann/json.hpp>

#include "Document.h"
#include "Platform.h"

#include "utils/List.h"


using json = nlohmann::json;


std::string rootPath;
List<Document*> documents;
std::map<std::string, int> uriMap;

//CGLCompiler* compiler;


void send(json msg)
{
	std::string body = msg.dump();
	//fprintf(stdout, "Content-Length: %d\r\n\r\n%s", (int)body.size(), body.c_str());
	// had to get rid of the \r for some reason, it would print 4 newlines and not accept the message
	fprintf(stdout, "Content-Length: %d\n\n%s", (int)body.size(), body.c_str());
	fflush(stdout);
	//fprintf(stderr, "Content-Length: %d\n\n%s\n", (int)body.size(), body.c_str());
}

void sendResponse(int id, json result)
{
	send({
		{"jsonrpc", "2.0"},
		{"id", id},
		{"result", result}
		});
}

int requestIdCounter = 1;
int sendRequest(std::string method, json params)
{
	int id = requestIdCounter++;
	send({
		{"jsonrpc", "2.0"},
		{"id", id},
		{"method", method},
		{"params", params}
		});
	return id;
}

json readMessage()
{
	std::string line;
	int length = 0;

	while (std::getline(std::cin, line)) {
		if (line.rfind("Content-Length:", 0) == 0) {
			length = std::stoi(line.substr(15));
		}
		if (line == "") break;
	}

	std::string content(length, '\0');
	std::cin.read((char*)content.data(), length);

	return content != "" ? json::parse(content) : json{};
}

static json CreateHoverResult(std::string contents)
{
	return {
		{"contents", contents}
	};
}

char* ReadText(const char* path)
{
	if (FILE* file = fopen(path, "rb"))
	{
		fseek(file, 0, SEEK_END);
		long numBytes = ftell(file);
		fseek(file, 0, SEEK_SET);

		char* buffer = new char[numBytes + 1];
		memset(buffer, 0, numBytes);
		numBytes = (long)fread(buffer, 1, numBytes, file);
		fclose(file);
		buffer[numBytes] = 0;

		return buffer;
	}
	return nullptr;
}

/*
static bool IsInRange(const AST::SourceLocation& a, const AST::SourceLocation& b, int line, int col)
{
	return (line > a.line || line == a.line && col >= a.col) && (line < b.line || line == b.line && col <= b.col);
}

static Scope* FindScopeAtSourceLocation(Scope* scope, int line, int col)
{
	Scope* result = nullptr;
	if (IsInRange(scope->start, scope->end, line, col))
		result = scope;

	for (int i = 0; i < scope->children.size; i++)
	{
		if (Scope* childScope = FindScopeAtSourceLocation(scope->children[i], line, col))
			result = childScope;
	}

	return result;
}

static void ProcessCompletionScope(Scope* scope, json& items, Resolver* resolver)
{
	for (int i = 0; i < scope->localVariables.size; i++)
	{
		items.push_back({
			{"label", scope->localVariables[i]->name},
			{"kind", COMPLETION_ITEM_VARIABLE},
			});
	}

	if (scope->parent != resolver->globalScope)
	{
		ProcessCompletionScope(scope->parent, items, resolver);
	}
}

static void autocompleteAST(AST::File* ast, Resolver* resolver, json& items)
{
	for (int i = 0; i < ast->globals.size; i++)
	{
		bool isConstant = (int)ast->globals[i]->flags & (int)AST::DeclarationFlags::Constant;
		for (int j = 0; j < ast->globals[i]->declarators.size; j++)
		{
			items.push_back({
				{"label", ast->globals[i]->declarators[j]->name},
				{"kind", isConstant ? COMPLETION_ITEM_CONSTANT : COMPLETION_ITEM_VARIABLE},
				});
		}
	}

	for (int i = 0; i < ast->functions.size; i++)
	{
		items.push_back({
			{"label", ast->functions[i]->name},
			{"kind", COMPLETION_ITEM_FUNCTION},
			});
	}

	for (int i = 0; i < ast->structs.size; i++)
	{
		items.push_back({
			{"label", ast->structs[i]->name},
			{"kind", COMPLETION_ITEM_STRUCT},
			});
	}

	for (int i = 0; i < ast->classes.size; i++)
	{
		items.push_back({
			{"label", ast->classes[i]->name},
			{"kind", COMPLETION_ITEM_CLASS},
			});
	}

	for (int i = 0; i < ast->typedefs.size; i++)
	{
		items.push_back({
			{"label", ast->typedefs[i]->name},
			{"kind", COMPLETION_ITEM_STRUCT},
			});
	}

	for (int i = 0; i < ast->enums.size; i++)
	{
		items.push_back({
			{"label", ast->enums[i]->name},
			{"kind", COMPLETION_ITEM_ENUM},
			});

		for (int j = 0; j < ast->enums[i]->values.size; j++)
		{
			items.push_back({
				{"label", ast->enums[i]->values[j]->name},
				{"kind", COMPLETION_ITEM_ENUM_MEMBER},
				});
		}
	}

	for (int i = 0; i < ast->classes.size; i++)
	{
		items.push_back({
			{"label", ast->classes[i]->name},
			{"kind", COMPLETION_ITEM_CLASS},
			});
	}

	for (int i = 0; i < ast->macros.size; i++)
	{
		items.push_back({
			{"label", ast->macros[i]->name},
			{"kind", COMPLETION_ITEM_VALUE},
			});
	}

	for (int i = 0; i < ast->imports.size; i++)
	{
		for (int j = 0; j < ast->imports[i]->imports.size; j++)
		{
			items.push_back({
				{"label", ast->imports[i]->imports[j].namespaces[0]},
				{"kind", COMPLETION_ITEM_VALUE},
				});
		}
	}
}

static void autocomplete(AST::File* currentFile, Resolver* resolver, json& items, int line, int col)
{
	if (Scope* scope = FindScopeAtSourceLocation(resolver->globalScope, line, col))
	{
		fprintf(stderr, "Found completion scope at %d, %d\n", scope->start.line, scope->start.col);
		ProcessCompletionScope(scope, items, resolver);
	}

	autocompleteAST(currentFile, compiler->resolver, items);
	for (int i = 0; i < compiler->resolver->asts.size; i++)
	{
		if (compiler->resolver->asts[i] != currentFile)
			autocompleteAST(compiler->resolver->asts[i], compiler->resolver, items);
	}
}
*/

void printCapabilities(const json& j, const std::string& prefix = "") {
	// Handle nested objects
	if (j.is_object()) {
		for (auto& [key, value] : j.items()) {
			printCapabilities(value, prefix + (prefix.empty() ? "" : ".") + key);
		}
	}
	// Handle arrays
	else if (j.is_array()) {
		std::cerr << prefix << ": [ ";
		for (const auto& item : j) {
			if (item.is_primitive()) {
				std::cerr << item << " ";
			}
			else {
				std::cerr << "{...} "; // Simplify nested objects inside arrays
			}
		}
		std::cerr << "]\n";
	}
	// Handle primitive values (bool, string, number, null)
	else {
		std::cerr << prefix << ": " << j << "\n";
	}
}

void Parse(Document* document)
{
	std::ostringstream stream;

	document->linesMutex.lock();

	for (int i = 0; i < document->lines.size; i++)
	{
		stream << document->lines[i];
		if (i < document->lines.size - 1)
			stream << '\n';
	}

	document->linesMutex.unlock();

	document->text = stream.str();

	uint64_t beforeParse = GetTimeNS();

	document->tokensMutex.lock();
	document->tokens.clear();

	Lexer lexer = Lexer(nullptr, nullptr, document->text.c_str());

	while (LexerHasNext(&lexer))
	{
		Token token = LexerNext(&lexer);
		document->tokens.add(token);
	}

	// parse here

	document->tokensMutex.unlock();

	uint64_t afterParse = GetTimeNS();
	float ms = (afterParse - beforeParse) / 1e6f;
	fprintf(stderr, "parsed in %.3fms\n", ms);

	sendRequest("workspace/semanticTokens/refresh", nullptr);
}

void ParserThread()
{
	bool running = true;
	while (running)
	{
		uint64_t now = GetTimeNS();

		for (int i = 0; i < documents.size; i++)
		{
			Document* document = documents[i];

			const int parseDelay = 500;

			if (document->lastChange && (now - document->lastChange) / 1e6 >= parseDelay)
			{
				Parse(document);
				document->lastChange = 0;
			}
		}

		SleepMS(10);
	}
}

int main()
{
	SleepMS(5000);
	std::cerr << "Starting lsp server" << std::endl;

	std::thread parserThread(ParserThread);
	parserThread.detach();

	while (true)
	{
		json request = readMessage();
		if (request.is_null())
			break;

		std::string method = request.value("method", "");

		std::cerr << "Received message of type " << method << std::endl;

		if (method == "initialize")
		{
			json params = request["params"];

			//printCapabilities(params["capabilities"]);

			json root = params["rootPath"];
			if (strcmp(root.type_name(), "string") == 0)
			{
				rootPath = params["rootPath"];
				fprintf(stderr, "Root path: %s\n", rootPath.c_str());
			}

			json tokenTypes = json::array();
			tokenTypes[LSP_TOKEN_NAMESPACE] = "namespace";
			tokenTypes[LSP_TOKEN_TYPE] = "type";
			tokenTypes[LSP_TOKEN_CLASS] = "class";
			tokenTypes[LSP_TOKEN_ENUM] = "enum";
			tokenTypes[LSP_TOKEN_INTERFACE] = "interface";
			tokenTypes[LSP_TOKEN_STRUCT] = "struct";
			tokenTypes[LSP_TOKEN_TYPE_PARAMETER] = "typeParameter";
			tokenTypes[LSP_TOKEN_PARAMETER] = "parameter";
			tokenTypes[LSP_TOKEN_VARIABLE] = "variable";
			tokenTypes[LSP_TOKEN_PROPERTY] = "property";
			tokenTypes[LSP_TOKEN_ENUM_VALUE] = "enumMember";
			tokenTypes[LSP_TOKEN_EVENT] = "event";
			tokenTypes[LSP_TOKEN_FUNCTION] = "function";
			tokenTypes[LSP_TOKEN_METHOD] = "method";
			tokenTypes[LSP_TOKEN_MACRO] = "macro";
			tokenTypes[LSP_TOKEN_KEYWORD] = "keyword";
			tokenTypes[LSP_TOKEN_MODIFIER] = "modifier";
			tokenTypes[LSP_TOKEN_COMMENT] = "comment";
			tokenTypes[LSP_TOKEN_STRING] = "string";
			tokenTypes[LSP_TOKEN_NUMBER] = "number";
			tokenTypes[LSP_TOKEN_REGEXP] = "regexp";
			tokenTypes[LSP_TOKEN_OPERATOR] = "operator";

			json tokenModifiers = json::array();
			tokenModifiers[LSP_TOKEN_MODIFIER_DECLARATION] = "declaration";
			tokenModifiers[LSP_TOKEN_MODIFIER_DEFINITION] = "definition";
			tokenModifiers[LSP_TOKEN_MODIFIER_READONLY] = "readonly";
			tokenModifiers[LSP_TOKEN_MODIFIER_STATIC] = "static";
			tokenModifiers[LSP_TOKEN_MODIFIER_DEPRECATED] = "deprecated";

			json result = {
				{"capabilities", {
					{"textDocumentSync", 2},
					{"hoverProvider", true},
					{"completionProvider", {
						{"resolveProvider", false}
					}},
					{"semanticTokensProvider", {
						{"legend", {
							{"tokenTypes", tokenTypes},
							{"tokenModifiers", tokenModifiers},
						}},
						{"full", true},
					}},
					/*
					{"workspace", {
						{"workspaceFolders", {
							{"supported", true},
							{"changeNotifications", true}
						}}
					}},
					*/
				}},
				{"serverInfo", {
					{"name", "Snek Language Server"},
					{"version", "0.0.1"},
				}}
			};

			sendResponse(request["id"], result);
		}

		// Notifications

		else if (method == "textDocument/didOpen")
		{
			auto params = request["params"];
			auto textDocument = params["textDocument"];
			std::string uri = textDocument["uri"];
			std::string text = textDocument["text"];

			Document* document = new Document();
			document->uri = uri;
			documents.add(document);
			uriMap.emplace(uri, documents.size - 1);

			document->init(text);
		}
		else if (method == "textDocument/didChange")
		{
			auto params = request["params"];
			auto textDocument = params["textDocument"];
			std::string uri = textDocument["uri"];
			int version = textDocument["version"];

			Document* document = documents[uriMap[uri]];

			json contentChanges = params["contentChanges"];
			for (int i = 0; i < (int)contentChanges.size(); i++)
			{
				json changeEvent = contentChanges[i];

				json range = changeEvent["range"];
				json rangeStart = range["start"];
				json rangeEnd = range["end"];

				int startLine = rangeStart["line"];
				int startCol = rangeStart["character"];

				int endLine = rangeEnd["line"];
				int endCol = rangeEnd["character"];

				std::string text = changeEvent["text"];

				document->onChange(startLine, startCol, endLine, endCol, text);
			}
		}

		// Requests

		else if (method == "textDocument/semanticTokens/full")
		{
			std::string uri = request["params"]["textDocument"]["uri"];
			Document* document = documents[uriMap[uri]];

			std::vector<int> data;

			document->getTokens(data);

			sendResponse(request["id"], {
				{"data", data}
				});
		}
		else if (method == "textDocument/hover")
		{
			json result = CreateHoverResult("abc");
			sendResponse(request["id"], result);
		}
		else if (method == "textDocument/completion")
		{
			json params = request["params"];
			std::string uri = params["textDocument"]["uri"];

			Document* document = documents[uriMap[uri]];

			json position = params["position"];
			int line = position["line"] + 1;
			int character = position["character"] + 1;

			int triggerKind = 0;
			std::string triggerCharacter = "";
			if (params.contains("context"))
			{
				json context = params["context"];
				triggerKind = context["triggerKind"];
				if (triggerKind == 2) // triggerCharacter
					triggerCharacter = context["triggerCharacter"];
			}

			// normal identifier completion
			if (triggerKind >= 0 && triggerKind <= 2) // invoke
			{
				json items = json::array();

				/*
				for (auto& pair : keywords)
				{
					std::string keyword = pair.first;
					items.push_back({
						{"label", keyword },
						{"kind", COMPLETION_ITEM_KEYWORD}  // keyword
						});
				}
				*/

				// TODO autocomplete using all parsed asts
				//std::filesystem::path path = std::filesystem::path(uri);
				//std::string name = path.stem().string();
				//AST::File* ast = compiler->getASTByName(name.c_str());
				//autocomplete(ast, compiler->resolver, items, line, character);

				if (triggerKind == 2)
				{
					fprintf(stderr, "trigger character %c\n", triggerCharacter[0]);
				}

				sendResponse(request["id"], {
					{"isIncomplete", false},
					{"items", items}
					});
			}
			else
			{
				sendResponse(request["id"], {
					{"isIncomplete", false},
					{"items", json::array()}
					});
			}
		}
	}
}
