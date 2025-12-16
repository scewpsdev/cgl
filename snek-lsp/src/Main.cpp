#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Store documents in memory
std::unordered_map<std::string, std::string> documents;

// Very dumb lexer (replace later)
std::vector<std::string> keywords = { "fn", "let", "struct", "if", "else", "while", "return" };

void send(json msg)
{
    std::string body = msg.dump();
    std::cout << "Content-Length: " << body.size() << "\r\n\r\n" << body;
    std::cout.flush();
}

json readMessage()
{
    std::string line;
    int length = 0;

    while (std::getline(std::cin, line)) {
        if (line.rfind("Content-Length:", 0) == 0) {
            length = std::stoi(line.substr(15));
        }
        if (line == "\r") break;
    }

    std::string content(length, '\0');
    std::cin.read((char*)content.data(), length);

    return json::parse(content);
}

int main()
{
    while (true)
    {
        json request = readMessage();
        std::string method = request.value("method", "");

        /* ========== INITIALIZE ========== */
        if (method == "initialize")
        {
            json result = {
                {"capabilities", {
                    {"textDocumentSync", 1},
                    {"completionProvider", { {"resolveProvider", false} }},
                    {"semanticTokensProvider", {
                        {"legend", {
                            {"tokenTypes", {"keyword", "variable", "function"}},
                            {"tokenModifiers", json::array()}
                        }},
                        {"full", true}
                    }}
                }}
            };

            send({
                {"jsonrpc", "2.0"},
                {"id", request["id"]},
                {"result", result}
                });
        }

        /* ========== OPEN FILE ========== */
        else if (method == "textDocument/didOpen")
        {
            auto uri = request["params"]["textDocument"]["uri"];
            auto text = request["params"]["textDocument"]["text"];
            documents[uri] = text;
        }

        /* ========== CHANGE FILE ========== */
        else if (method == "textDocument/didChange")
        {
            auto uri = request["params"]["textDocument"]["uri"];
            auto text = request["params"]["contentChanges"][0]["text"];
            documents[uri] = text;
        }

        /* ========== COMPLETION ========== */
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

            send({
                {"jsonrpc", "2.0"},
                {"id", request["id"]},
                {"result", {
                    {"isIncomplete", false},
                    {"items", items}
                }}
                });
        }

        /* ========== SEMANTIC TOKENS ========== */
        else if (method == "textDocument/semanticTokens/full")
        {
            auto uri = request["params"]["textDocument"]["uri"];
            std::string text = documents[uri];

            std::vector<int> data;
            int line = 0;
            int col = 0;

            std::istringstream stream(text);
            std::string word;

            while (stream >> word)
            {
                for (auto& k : keywords)
                {
                    if (word == k)
                    {
                        // deltaLine, deltaStart, length, tokenType, tokenModifiers
                        data.push_back(line);
                        data.push_back(col);
                        data.push_back((int)word.length());
                        data.push_back(0); // keyword
                        data.push_back(0);
                    }
                }
                col += (int)word.length() + 1;
                if (word.find("\n") != std::string::npos)
                {
                    line++;
                    col = 0;
                }
            }

            send({
                {"jsonrpc","2.0"},
                {"id", request["id"]},
                {"result", {
                    {"data", data}
                }}
                });
        }
    }
}
