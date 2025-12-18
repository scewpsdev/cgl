#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <Windows.h>
#include <stdio.h>
#include <nlohmann/json.hpp>

#include "Document.h"

#include "cgl/CGLCompiler.h"


using json = nlohmann::json;


enum TokenType
{
	TOKEN_TYPE_KEYWORD,
	TOKEN_TYPE_VARIABLE,
	TOKEN_TYPE_FUNCTION,

	TOKEN_TYPE_LAST
};

static const char* tokenTypeIdentifiers[TOKEN_TYPE_LAST] = {
	"keyword",
	"variable",
	"function",
};


std::vector<std::string> keywords = { "fn", "let", "struct", "if", "else", "while", "return" };

std::unordered_map<std::string, Document> documents;


void send(json msg)
{
	std::string body = msg.dump();
	//fprintf(stdout, "Content-Length: %d\r\n\r\n%s", (int)body.size(), body.c_str());
	// had to get rid of the \r for some reason, it would print 4 newlines and not accept the message
	fprintf(stdout, "Content-Length: %d\n\n%s", (int)body.size(), body.c_str());
	fprintf(stderr, "%s", body.c_str());
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
	std::cerr << "Starting lsp server" << std::endl;

	while (true)
	{
		json request = readMessage();
		std::string method = request.value("method", "");

		std::cerr << "Received message of type " << method << std::endl;

		// Requests

		if (method == "initialize")
		{
			json tokenLegend = {
				{"tokenTypes", json::array()},
				{"tokenModifiers", json::array()},
			};
			for (int i = 0; i < TOKEN_TYPE_LAST; i++)
			{
				tokenLegend["tokenTypes"][i] = tokenTypeIdentifiers[i];
			}

			json result = {
				{"capabilities", {
					{"textDocumentSync", 1},
					{"hoverProvider", true},
					{"completionProvider", {
						{"resolveProvider", false}
					}},
					{"semanticTokensProvider", {
						{"legend", tokenLegend},
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
		else if (method == "textDocument/hover")
		{
			json result = CreateHoverResult("abc");
			sendResponse(request["id"], result);
		}
		else if (method == "textDocument/completion")
		{
			json items = json::array();

			for (auto& k : keywords)
			{
				items.push_back({
					{"label", k},
					{"kind", 14}  // keyword
					});
			}

			sendResponse(request["id"], {
				{"isIncomplete", false},
				{"items", items}
				});
		}
		else if (method == "textDocument/semanticTokens/full")
		{
			std::string uri = request["params"]["textDocument"]["uri"];
			Document& document = documents[uri];

			CGLCompiler compiler;
			compiler.init(nullptr);
			compiler.addFile(uri.c_str(), "testmodule", document.text.c_str());
			compiler.compile();

			SourceFile& sourceFile = compiler.sourceFiles[0];

			std::vector<int> data;
			int line = 0;
			int col = 0;
			int lastLine = 0, lastCol = 0;

			std::istringstream stream(document.text);
			std::string word;

			while (stream >> word)
			{
				for (auto& k : keywords)
				{
					if (word == k)
					{
						// deltaLine, deltaStart, length, tokenType, tokenModifiers
						data.push_back(line - lastLine);
						data.push_back(line == lastLine ? col - lastCol : col);
						data.push_back((int)word.length());
						data.push_back(0); // keyword
						data.push_back(0);

						lastLine = line;
						lastCol = col;
					}
				}
				col += (int)word.length() + 1;
				if (word.find("\n") != std::string::npos)
				{
					line++;
					col = 0;
				}
			}

			sendResponse(request["id"], {
				{"data", data}
				});
		}

		// Notifications

		else if (method == "textDocument/didOpen")
		{
			auto params = request["params"];
			auto textDocument = params["textDocument"];
			std::string uri = textDocument["uri"];
			std::string text = textDocument["text"];

			Document document = {};
			document.text = text;
			documents[uri] = document;

			fprintf(stderr, "Opened text document %s\n", uri.c_str());
			fprintf(stderr, "%s\n", text.c_str());
		}
		else if (method == "textDocument/didChange")
		{
			auto params = request["params"];
			auto textDocument = params["textDocument"];
			std::string uri = textDocument["uri"];
			int version = textDocument["version"];

			json contentChanges = params["contentChanges"];
			for (int i = 0; i < (int)contentChanges.array().size(); i++)
			{
				json changeEvent = contentChanges[i];
				std::string text = changeEvent["text"];

				Document& document = documents[uri];
				document.text = text;

				fprintf(stderr, "Changed text document %s\n", uri.c_str());
				fprintf(stderr, "%s\n", text.c_str());
			}
		}
	}
}
