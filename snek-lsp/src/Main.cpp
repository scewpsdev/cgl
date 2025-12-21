#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <thread>
#include <stdio.h>

#include <nlohmann/json.hpp>

#include "Document.h"

#include "cgl/Platform.h"
#include "cgl/CGLCompiler.h"
#include "cgl/parser/lexer.h"
#include "cgl/parser/Parser.h"
#include "cgl/semantics/Resolver.h"


using json = nlohmann::json;


std::vector<std::string> keywords = { "fn", "let", "struct", "if", "else", "while", "return" };

std::unordered_map<std::string, Document> documents;


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
			document.reparse(data);

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

			json position = params["position"];
			int line = position["line"] + 1;
			int character = position["character"] + 1;

			json context = params["context"];
			int triggerKind = context["triggerKind"];
			std::string triggerCharacter;
			if (triggerKind == 2) // triggerCharacter
				triggerCharacter = context["triggerCharacter"];

			// normal identifier completion
			if (triggerKind == 1) // invoke
			{
				json items = json::array();

				for (auto& k : keywords)
				{
					items.push_back({
						{"label", k},
						{"kind", COMPLETION_ITEM_KEYWORD}  // keyword
						});
				}

				document.autocomplete(items, line, character);

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
		}
	}
}
