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

#include "cgl/Platform.h"
#include "cgl/CGLCompiler.h"
#include "cgl/parser/lexer.h"
#include "cgl/parser/Parser.h"
#include "cgl/semantics/Resolver.h"
#include "cgl/semantics/Variable.h"


using json = nlohmann::json;


std::string rootPath;
std::unordered_map<std::string, Document> documents;

CGLCompiler* compiler;


void send(json msg)
{
	std::string body = msg.dump();
	//fprintf(stdout, "Content-Length: %d\r\n\r\n%s", (int)body.size(), body.c_str());
	// had to get rid of the \r for some reason, it would print 4 newlines and not accept the message
	fprintf(stdout, "Content-Length: %d\n\n%s", (int)body.size(), body.c_str());
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

	return json::parse(content);
}

static json CreateHoverResult(std::string contents)
{
	return {
		{"contents", contents}
	};
}

static void OnCompilerMessage(CGLCompiler* context, MessageType msgType, const char* filename, int line, int col, const char* msg, ...)
{
	static const char* const MSG_TYPE_NAMES[MESSAGE_TYPE_MAX] = {
		"<null>",
		"info",
		"warning",
		"error",
		"fatal error",
	};

	static char message[4192] = {};
	message[0] = 0;

	if (filename)
		sprintf(message + strlen(message), "%s:%d:%d: ", filename, line, col);

	if (msgType != MESSAGE_TYPE_INFO)
		sprintf(message + strlen(message), "%s: ", MSG_TYPE_NAMES[msgType]);

	va_list args;
	va_start(args, msg);
	vsprintf(message + strlen(message), msg, args);
	va_end(args);

	fprintf(stderr, "%s\n", message);
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

static void reparse()
{
	uint64_t beforeParse = GetTimeNS();

	if (compiler)
	{
		compiler->terminate();
		delete compiler;
	}

	compiler = new CGLCompiler();
	compiler->init(OnCompilerMessage);

	std::vector<std::filesystem::path> sourceFilesInWorkspace;

	for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(rootPath))
	{
		if (dirEntry.is_regular_file())
		{
			std::filesystem::path file = dirEntry.path();
			if (file.extension().string() == ".src")
			{
				sourceFilesInWorkspace.push_back(file);
			}
		}
	}

	for (auto& file : sourceFilesInWorkspace)
	{
		std::string filepath = file.string();
		std::string name = file.stem().string();
		char* src = ReadText(filepath.c_str());
		compiler->addFile(_strdup(filepath.c_str()), _strdup(name.c_str()), src);
	}

	/*
	for (auto& pair : documents)
	{
		Document* document = &pair.second;
		compiler->addFile(document->uri.c_str(), document->uri.c_str(), document->text.c_str());
	}
	*/

	compiler->compile();

	uint64_t afterParse = GetTimeNS();

	float ms = (afterParse - beforeParse) / 1e6f;
	fprintf(stderr, "parsed %d files in %.3fms\n", (int)sourceFilesInWorkspace.size(), ms);
}

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

int main()
{
	//std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	std::cerr << "Starting lsp server" << std::endl;

	while (true)
	{
		json request = readMessage();
		std::string method = request.value("method", "");

		std::cerr << "Received message of type " << method << std::endl;

		// Requests

		if (method == "initialize")
		{
			rootPath = request["params"]["rootPath"];
			fprintf(stderr, "Root path: %s\n", rootPath.c_str());

			json tokenTypes = json::array();
			tokenTypes[LSP_TOKEN_NAMESPACE] = "namespace";
			tokenTypes[LSP_TOKEN_TYPE] = "type";
			tokenTypes[LSP_TOKEN_CLASS] = "class";
			tokenTypes[LSP_TOKEN_ENUM] = "enum";
			tokenTypes[LSP_TOKEN_STRUCT] = "struct";
			tokenTypes[LSP_TOKEN_VARIABLE] = "variable";
			tokenTypes[LSP_TOKEN_ENUM_VALUE] = "enumMember";
			tokenTypes[LSP_TOKEN_FUNCTION] = "function";
			tokenTypes[LSP_TOKEN_MACRO] = "macro";
			tokenTypes[LSP_TOKEN_KEYWORD] = "keyword";
			tokenTypes[LSP_TOKEN_COMMENT] = "comment";
			tokenTypes[LSP_TOKEN_STRING] = "string";
			tokenTypes[LSP_TOKEN_NUMBER] = "number";
			tokenTypes[LSP_TOKEN_OPERATOR] = "operator";

			json result = {
				{"capabilities", {
					{"textDocumentSync", 1},
					{"hoverProvider", true},
					{"completionProvider", {
						{"resolveProvider", false}
					}},
					{"semanticTokensProvider", {
						{"legend", {
							{"tokenTypes", tokenTypes},
							{"tokenModifiers", json::array()},
						}},
						{"full", true},
					}},
				}},
				{"serverInfo", {
					{"name", "Snek Language Server"},
					{"version", "0.0.1"},
				}}
			};

			sendResponse(request["id"], result);
		}
		else if (method == "textDocument/semanticTokens/full")
		{
			std::string uri = request["params"]["textDocument"]["uri"];
			Document& document = documents[uri];

			std::vector<int> data;

			if (compiler)
			{
				std::filesystem::path path = std::filesystem::path(uri);
				std::string name = path.stem().string();
				if (AST::File* ast = compiler->getASTByName(name.c_str()))
				{
					document.getTokens(ast, compiler, data);
				}
			}

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

			Document& document = documents[uri];

			std::filesystem::path path = std::filesystem::path(uri);
			std::string name = path.stem().string();
			AST::File* ast = compiler->getASTByName(name.c_str());

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

				for (auto& pair : keywords)
				{
					std::string keyword = pair.first;
					items.push_back({
						{"label", keyword },
						{"kind", COMPLETION_ITEM_KEYWORD}  // keyword
						});
				}

				// TODO autocomplete using all parsed asts
				autocomplete(ast, compiler->resolver, items, line, character);

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

		// Notifications

		else if (method == "textDocument/didOpen")
		{
			auto params = request["params"];
			auto textDocument = params["textDocument"];
			std::string uri = textDocument["uri"];
			std::string text = textDocument["text"];

			documents.emplace(uri, Document{ uri, text });

			reparse();

			//fprintf(stderr, "Opened text document %s\n", uri.c_str());
			//fprintf(stderr, "%s\n", text.c_str());
		}
		else if (method == "textDocument/didChange")
		{
			auto params = request["params"];
			auto textDocument = params["textDocument"];
			std::string uri = textDocument["uri"];
			int version = textDocument["version"];

			json contentChanges = params["contentChanges"];
			for (int i = 0; i < (int)contentChanges.size(); i++)
			{
				json changeEvent = contentChanges[i];
				std::string text = changeEvent["text"];

				Document& document = documents[uri];
				document.text = text;

				//fprintf(stderr, "Changed text document %s\n", uri.c_str());
				//fprintf(stderr, "%s\n", text.c_str());
			}

			reparse();
		}
	}
}
