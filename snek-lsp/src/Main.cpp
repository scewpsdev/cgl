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


using json = nlohmann::json;


struct LSPToken
{
	Token token;
	int type;
	int modifiers;
};


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

static void OnCompilerMessage(CGLCompiler* context, MessageType msgType, const char* filename, int line, int col, const char* msg, ...)
{
	va_list args;
	va_start(args, msg);
	vfprintf(stderr, msg, args);
	va_end(args);
	fprintf(stderr, "\n");
}

static int LSPTokenComparator(LSPToken const* a, LSPToken const* b)
{
	return (a->token.line < b->token.line || a->token.line == b->token.line && a->token.col < b->token.col) ? -1 :
		a->token.line == b->token.line && a->token.col == b->token.col ? 0 : 1;
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
			json result = {
				{"capabilities", {
					{"textDocumentSync", 1},
					{"hoverProvider", true},
					{"completionProvider", {
						{"resolveProvider", false}
					}},
					{"semanticTokensProvider", {
						{"legend", {
							{"tokenTypes", {"keyword", "string", "number", "operator", "function", "struct", "type"}},
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

			uint64_t beforeParse = GetTimeNS();

			CGLCompiler compiler;
			compiler.init(OnCompilerMessage);
			SourceFile sourceFile = {};
			sourceFile.filename = uri.c_str();
			sourceFile.name = "testfile";
			sourceFile.source = document.text.c_str();

			Lexer lexer(&compiler, sourceFile.filename, sourceFile.source);
			Parser parser(&compiler);

			std::vector<LSPToken> tokens;

			auto addToken = [&tokens](Token token, int type, int modifiers)
				{
					tokens.push_back({ token, type, modifiers });
				};

			while (LexerHasNext(&lexer))
			{
				Token token = LexerNext(&lexer);

				if (token.keywordType != KEYWORD_TYPE_NULL)
				{
					if (token.keywordType > KEYWORD_TYPE_BUILTIN_TYPES_START && token.keywordType < KEYWORD_TYPE_BUILTIN_TYPES_END)
						addToken(token, 6 /*type*/, 0);
					else
						addToken(token, 0 /*keyword*/, 0);
				}
				else
				{
					switch (token.type)
					{
					case TOKEN_TYPE_STRING_LITERAL:
					case TOKEN_TYPE_CHAR_LITERAL:
						addToken(token, 1 /*string*/, 0);
						break;
					case TOKEN_TYPE_STRING_LITERAL_MULTILINE:
						break;
					case TOKEN_TYPE_FLOAT_LITERAL:
					case TOKEN_TYPE_DOUBLE_LITERAL:
					case TOKEN_TYPE_INT_LITERAL:
						addToken(token, 2 /*number*/, 0);
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
						addToken(token, 3 /*operator*/, 0);
						break;
					case TOKEN_TYPE_IDENTIFIER:
						break;
					default:
						break;
					}
				}
			}

			AST::File* ast = parser.run(sourceFile);
			for (int i = 0; i < ast->functions.size; i++)
			{
				AST::Function* function = ast->functions[i];
				addToken(function->nameToken, 4 /*function*/, 0);
			}
			for (int i = 0; i < ast->structs.size; i++)
			{
				AST::Struct* strct = ast->structs[i];
				addToken(strct->nameToken, 5 /*struct*/, 0);
			}

			qsort(tokens.data(), tokens.size(), sizeof(LSPToken), (_CoreCrtNonSecureSearchSortCompareFunction)LSPTokenComparator);

			std::vector<int> data;
			int lastLine = 0, lastCol = 0;
			for (int i = 0; i < (int)tokens.size(); i++)
			{
				LSPToken token = tokens[i];
				int line = token.token.line - 1;
				int col = token.token.col - 1;
				int len = token.token.len;

				// deltaLine, deltaStart, length, tokenType, tokenModifiers
				data.push_back(line - lastLine);
				data.push_back(line == lastLine ? col - lastCol : col);
				data.push_back(len);
				data.push_back(token.type);
				data.push_back(token.modifiers);

				lastLine = line;
				lastCol = col;
			}

			uint64_t afterParse = GetTimeNS();

			float ms = (afterParse - beforeParse) / 1e6f;
			fprintf(stderr, "parsed '%s' in %.3fms\n", uri.c_str(), ms);

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
