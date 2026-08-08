#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <thread>
#include <stdio.h>
#include <stdarg.h>
#include <filesystem>

#include <nlohmann/json.hpp>

#include "Document.h"
#include "Platform.h"

#include "parser/Parser.h"
#include "typechecker/TypeSystem.h"

#include "utils/List.h"


using json = nlohmann::json;


std::string rootPath;
std::string mainFilePath;
std::thread parserThread;

GlobalBlockPool blockPool;
TypeSystem types;
List<Document*> documents;
std::map<std::string, int> uriMap;

//CGLCompiler* compiler;


void send(json msg)
{
	std::string body = msg.dump();
	fprintf(stdout, "Content-Length: %d\n\n%s", (int)body.size(), body.c_str());
	fflush(stdout);
}

void sendResponse(int id, json result)
{
	send({
		{"jsonrpc", "2.0"},
		{"id", id},
		{"result", result}
		});
}

void sendErrorResponse(int id, int code)
{
	send({
		{"jsonrpc", "2.0"},
		{"id", id},
		{"error", {
			{"code", code}
		}}
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

	//std::cerr << "Sent request of type " << method << std::endl;

	return id;
}

void sendNotification(std::string method, json params)
{
	send({
		{"jsonrpc", "2.0"},
		{"method", method},
		{"params", params}
		});

	//std::cerr << "Sent notification of type " << method << std::endl;
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

std::string escapeString(const std::string& input)
{
	std::ostringstream escaped;

	for (char c : input) {
		switch (c) {
		case '"':  escaped << "\\\""; break;
		case '\\': escaped << "\\\\"; break;
		case '\b': escaped << "\\b";  break;
		case '\f': escaped << "\\f";  break;
		case '\n': escaped << "\\n";  break;
		case '\r': escaped << "\\r";  break;
		case '\t': escaped << "\\t";  break;
		default:
			// Handle control characters (ASCII 0 to 31) using \u00xx format
			if (static_cast<unsigned char>(c) < 32) {
				escaped << "\\u"
					<< std::hex << std::setw(4) << std::setfill('0')
					<< static_cast<int>(c);
			}
			else {
				escaped << c;
			}
			break;
		}
	}

	return escaped.str();
}

static void sendDiagnosticsNotification(Diagnostics* diagnostics, Document* document)
{
	json diagnosticsItems = json::array();
	for (int i = 0; i < diagnostics->items.size; i++)
	{
		json range = {
			{"start", {
				{"line", diagnostics->items[i].startLine},
				{"character", diagnostics->items[i].startCol}
			}},
			{"end", {
				{"line", diagnostics->items[i].endLine},
				{"character", diagnostics->items[i].endCol}
			}}
		};
		json diagnosticsItem = {
			{"range", range},
			{"severity", (int)diagnostics->items[i].severity},
			{"message", escapeString(diagnostics->items[i].message)}
		};
		diagnosticsItems.push_back(diagnosticsItem);

		fprintf(stderr, "error %s:%d:%d: %s\n", document->uri.c_str(), diagnostics->items[i].startLine + 1, diagnostics->items[i].startCol + 1, diagnostics->items[i].message);
	}

	sendNotification("textDocument/publishDiagnostics", {
		{"uri", document->uri},
		{"diagnostics", diagnosticsItems}
		});
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

static std::string PathToURI(const char* path)
{
	namespace fs = std::filesystem;

	// 1. Get absolute path and normalize to generic forward slashes '/'
	fs::path p = fs::absolute(path).generic_string();
	std::string genericPath = p.string();

	// 2. Format Windows paths (e.g., "C:/path" -> "/c:/path")
	if (genericPath.size() > 1 && genericPath[1] == ':') {
		genericPath[0] = std::tolower(genericPath[0]); // Lowercase drive letter
		genericPath = "/" + genericPath;
	}

	// 3. Simple strict percent-encoding loop
	std::ostringstream uri;
	uri << "file://";

	for (unsigned char c : genericPath) {
		// Keep ONLY unreserved RFC 3986 path characters and standard '/' delimiters
		if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~' || c == '/') {
			uri << (char)c;
		}
		else {
			// Force lowercase hex output (%3a) to match your client output
			uri << '%' << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (int)c;
		}
	}

	return uri.str();
}

static std::string URIToPath(const std::string& uriStr)
{
	namespace fs = std::filesystem;

	// 1. Strip the "file://" scheme
	std::string prefix = "file://";
	std::string pathStr = uriStr;
	if (pathStr.rfind(prefix, 0) == 0) {
		pathStr = pathStr.substr(prefix.length());
	}

	// 2. Percent-decode the remaining string
	std::string decodedPath;
	decodedPath.reserve(pathStr.size());

	for (size_t i = 0; i < pathStr.size(); ++i) {
		if (pathStr[i] == '%' && i + 2 < pathStr.size()) {
			// Extract hex value
			std::string hexStr = pathStr.substr(i + 1, 2);
			unsigned char value = static_cast<unsigned char>(std::stoul(hexStr, nullptr, 16));
			decodedPath += value;
			i += 2; // Skip the hex characters
		}
		else {
			decodedPath += pathStr[i];
		}
	}

	// 3. Reformat Windows paths (e.g., "/c:/path" -> "c:/path" or "C:/path")
	// If it starts with '/' followed by a drive letter and ':'
	if (decodedPath.size() > 2 && decodedPath[0] == '/' && std::isalpha(decodedPath[1]) && decodedPath[2] == ':') {
		decodedPath = decodedPath.substr(1); // Remove leading slash

		// Optional: Capitalize drive letter to match native Windows preferences
		decodedPath[0] = std::toupper(decodedPath[0]);
	}

	// 4. Return as native path string
	return fs::path(decodedPath).make_preferred().string();
}

static char* GetModuleNameFromPath(const char* path)
{
	const char* forwardSlash = strrchr(path, '/');
	const char* backwardSlash = strrchr(path, '\\');
	const char* slash = (forwardSlash && backwardSlash) ? (const char*)__max((uint64_t)forwardSlash, (uint64_t)backwardSlash) : forwardSlash ? forwardSlash : backwardSlash ? backwardSlash : NULL;
	if (slash)
		slash++;
	else
		slash = path;

	const char* dot = strrchr(slash, '.');
	if (!dot)
		dot = slash + strlen(slash);

	int length = (int)(dot - slash);

	char* name = new char[length + 1];
	strncpy(name, slash, length);
	name[length] = 0;

	return name;
}

static const char* GetFilenameFromPath(const char* path)
{
	const char* forwardSlash = strrchr(path, '/');
	const char* backwardSlash = strrchr(path, '\\');
	const char* slash = (forwardSlash && backwardSlash) ? (const char*)__max((uint64_t)forwardSlash, (uint64_t)backwardSlash) : forwardSlash ? forwardSlash : backwardSlash ? backwardSlash : NULL;
	if (slash)
		slash++;
	else
		slash = path;

	return slash;
}

static char* GetFolderFromPath(const char* path)
{
	const char* forwardSlash = strrchr(path, '/');
	const char* backwardSlash = strrchr(path, '\\');
	const char* slash = (forwardSlash && backwardSlash) ? (const char*)__max((uint64_t)forwardSlash, (uint64_t)backwardSlash) : forwardSlash ? forwardSlash : backwardSlash ? backwardSlash : NULL;
	if (!slash)
		slash = path;

	int length = (int)(slash - path);

	if (length == 0)
	{
		char* folder = new char[2];
		strcpy(folder, ".");
		folder[1] = 0;
		return folder;
	}
	else
	{
		char* folder = new char[length + 1];
		strncpy(folder, path, length);
		folder[length] = 0;
		return folder;
	}
}

static const char* GetExtensionFromPath(const char* path)
{
	const char* ext = strrchr(path, '.');
	if (ext)
		ext++;
	else
		ext = path + strlen(path);

	return ext;
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

void Parse(List<Document*> documents)
{
	if (documents.size == 0)
		return;

	uint64_t beforeParse = GetTimeNS();

	for (int i = 0; i < documents.size; i++)
	{
		Document* document = documents[i];

		document->linesMutex.lock();

		std::ostringstream stream;
		for (int i = 0; i < document->lines.size; i++)
		{
			stream << document->lines[i];
			if (i < document->lines.size - 1)
				stream << '\n';
		}

		document->linesMutex.unlock();

		document->text = stream.str();
		stream.clear();

		document->astMutex.lock();

		clearInternedTypes(&document->file, &types);

		resetArena(&document->file.arena);
		resetScratchBuffer(&document->file.scratch);
		resetDiagnostics(&document->file.diagnostics);

		document->file.ast = {};

		document->file.dependencies.clear();

		if (document->file.parser.arena)
			destroyParser(&document->file.parser);
		initParser(&document->file.parser, document->uri.c_str(), document->text.c_str(), (int)document->text.size(), &document->file.arena, &document->file.scratch, &document->file.diagnostics);

		parse(&document->file.parser, &document->file.ast);
		document->state = DOCUMENT_STATE_PARSED;
		document->needsTypeCheck = true;

		resolveDependencies(&document->file.parser, &document->file);

		//sendDiagnosticsNotification(&document->diagnostics, document);

		document->astMutex.unlock();
	}

	uint64_t afterParse = GetTimeNS();
	float ms = (afterParse - beforeParse) / 1e6f;
	fprintf(stderr, "parsed %d documents in %.3fms\n", documents.size, ms);
}

void TypeCheck(List<Document*> documents)
{
	if (documents.size == 0)
		return;

	uint64_t beforeTypeCheck = GetTimeNS();

	for (int i = 0; i < documents.size; i++)
	{
		Document* document = documents[i];

		SnekAssert(document->state >= DOCUMENT_STATE_PARSED);

		document->astMutex.lock();

		if (document->state >= DOCUMENT_STATE_TYPECHECKED)
		{
			clearInternedTypes(&document->file, &types);
			resetAST(&document->file.ast);
		}

		if (document->file.typeChecker.arena)
			destroyTypeChecker(&document->file.typeChecker);

		initTypeChecker(&document->file.typeChecker, &document->file.arena, &document->file.scratch, &document->file.parser.lexer, &document->file.diagnostics, &types);

		symbolCollection(&document->file.typeChecker, &document->file);

		document->astMutex.unlock();
	}

	for (int i = 0; i < documents.size; i++)
	{
		Document* document = documents[i];

		document->astMutex.lock();

		symbolResolution(&document->file.typeChecker, &document->file);

		document->astMutex.unlock();
	}

	for (int i = 0; i < documents.size; i++)
	{
		Document* document = documents[i];

		document->astMutex.lock();

		for (int j = 0; j < document->file.ast.numFunctions; j++)
		{
			typeCheckFunction(&document->file.typeChecker, document->file.ast.functions[j], &document->file);
		}

		document->state = DOCUMENT_STATE_TYPECHECKED;
		document->needsTypeCheck = false;

		sendDiagnosticsNotification(&document->file.diagnostics, document);

		document->astMutex.unlock();

		if (document->file.diagnostics.items.size == 0)
		{
			std::string outPath = document->localPath;
			outPath = outPath.substr(0, outPath.find('.'));
			outPath = rootPath + "/tmp/" + outPath + ".c";
			emitFile(&document->file.codegen, &document->file.ast, document->localPath.c_str(), outPath.c_str());
		}
	}

	sendRequest("workspace/semanticTokens/refresh", nullptr);

	uint64_t afterTypeCheck = GetTimeNS();
	float ms = (afterTypeCheck - beforeTypeCheck) / 1e6f;
	fprintf(stderr, "typechecked %d documents in %.3fms\n", documents.size, ms);
	fprintf(stderr, "memory blocks allocated: %d\n", blockPool.blockCount);
	fprintf(stderr, "type table size: %d/%d\n", types.typeTable.count, types.typeTable.capacity);
}

static Document* OpenDocument(std::string uri, std::string text)
{
	namespace fs = std::filesystem;

	fprintf(stderr, "Opening document %s\n", uri.c_str());

	fs::path mainFileDirectory = fs::path(mainFilePath).parent_path();
	fs::path localPath = fs::relative(URIToPath(uri), mainFileDirectory);

	Document* document = new Document();
	document->uri = uri;
	document->localPath = localPath.string();

	documents.add(document);
	uriMap.emplace(document->uri, documents.size - 1);

	document->init(text, &types, &blockPool);

	return document;
}

static Document* GetDocument(std::string uri)
{
	auto it = uriMap.find(uri);
	if (it != uriMap.end())
		return documents[it->second];
	return nullptr;
}

static bool AddFile(const char* path)
{
	if (char* src = ReadText(path))
	{
		OpenDocument(PathToURI(path), std::string(src));
		delete[] src;
		return true;
	}
	return false;
}

static bool ScanSourceFolder(const char* folder, const char* extension, bool recursive, std::string* mainFilePath)
{
	if (!std::filesystem::exists(folder))
	{
		return false;
	}

	bool result = true;

	for (auto& de : std::filesystem::directory_iterator(folder))
	{
		std::u8string dePathStr = de.path().u8string();
		const char* dePath = (const char*)dePathStr.c_str();
		if (de.is_directory() && recursive)
		{
			result = ScanSourceFolder(dePath, extension, recursive, mainFilePath) && result;
		}
		else
		{
			const char* filename = GetFilenameFromPath(dePath);
			if (strcmp(filename, "main.src") == 0)
			{
				*mainFilePath = dePath;
			}

			const char* fileExtension = GetExtensionFromPath(dePath);
			if (fileExtension && strcmp(fileExtension, extension) == 0)
			{
				result = AddFile(dePath) && result;
			}
		}
	}

	return result;
}

static Document* getDocument(FileHandle file)
{
	for (int i = 0; i < documents.size; i++)
	{
		if (documents[i]->file.handle == file)
			return documents[i];
	}
	return nullptr;
}

static bool containsFile(List<Document*>& documents, FileHandle file)
{
	for (int i = 0; i < documents.size; i++)
	{
		if (documents[i]->file.handle == file)
			return true;
	}
	return false;
}

static bool areDependenciesParsed(File* file)
{
	for (int i = 0; i < file->dependencies.size; i++)
	{
		FileHandle dependency = file->dependencies[i];
		if (Document* document = getDocument(dependency))
		{
			if (document->state < DOCUMENT_STATE_PARSED)
				return false;
		}
	}
	return true;
}

void ParserThread()
{
	bool running = true;
	while (running)
	{
		uint64_t now = GetTimeNS();

		static List<Document*> parseList;
		static List<Document*> typeCheckList;

		parseList.clear();
		typeCheckList.clear();

		for (int i = 0; i < documents.size; i++)
		{
			Document* document = documents[i];

			const int parseDelay = 200;
			if (document->state == DOCUMENT_STATE_UNPARSED || document->lastChange && (now - document->lastChange) / 1e6 >= parseDelay)
			{
				parseList.add(document);
				document->lastChange = 0;
			}
		}

		for (int i = 0; i < documents.size; i++)
		{
			Document* document = documents[i];

			for (int j = 0; j < document->file.dependencies.size; j++)
			{
				FileHandle dependency = document->file.dependencies[j];
				if (containsFile(parseList, dependency))
				{
					document->needsTypeCheck = true;
					break;
				}
			}
		}

		Parse(parseList);

		for (int i = 0; i < documents.size; i++)
		{
			Document* document = documents[i];

			if (document->needsTypeCheck && areDependenciesParsed(&document->file))
			{
				if (!typeCheckList.contains(document))
					typeCheckList.add(document);
			}
		}

		TypeCheck(typeCheckList);

		SleepMS(10);
	}
}

int main()
{
	SleepMS(5000);
	fprintf(stderr, "Starting LSP Server\n");

	initGlobalBlockPool(&blockPool, 16);
	initTypeSystem(&types);

	while (true)
	{
		json request = readMessage();
		if (request.is_null())
			break;

		std::string method = request.value("method", "");

		if (method != "")
		{
			//fprintf(stderr, "Received message of type %s\n", method.c_str());

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
						//{"hoverProvider", true},
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

				ScanSourceFolder(rootPath.c_str(), "src", true, &mainFilePath);

				parserThread = std::thread(ParserThread);
				parserThread.detach();

				sendResponse(request["id"], result);
			}

			// Notifications

			else if (method == "textDocument/didOpen")
			{
				auto params = request["params"];
				auto textDocument = params["textDocument"];
				std::string uri = textDocument["uri"];
				std::string text = textDocument["text"];

				Document* document = GetDocument(uri);

				if (!document)
					document = OpenDocument(uri, text);

				document->onOpen(text);
			}
			else if (method == "textDocument/didChange")
			{
				auto params = request["params"];
				auto textDocument = params["textDocument"];
				std::string uri = textDocument["uri"];
				int version = textDocument["version"];

				if (Document* document = GetDocument(uri))
				{
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
				else
				{
					fprintf(stderr, "Document %s not found\n", uri.c_str());
				}
			}

			// Requests

			else if (method == "textDocument/semanticTokens/full")
			{
				std::string uri = request["params"]["textDocument"]["uri"];
				int id = request["id"];

				if (Document* document = GetDocument(uri))
				{
					if (document->lastChange == 0) // dont send tokens is document is outdated and needs to be reparsed
					{
						std::vector<int> data;
						document->getTokens(data);

						sendResponse(id, {
							{"data", data}
							});
					}
					else
					{
						sendErrorResponse(id, -32801);
					}
				}
				else
				{
					fprintf(stderr, "Document %s not found\n", uri.c_str());
				}
			}
			/*
			else if (method == "textDocument/hover")
			{
				json result = CreateHoverResult("abc");
				sendResponse(request["id"], result);
			}
			*/
			else if (method == "textDocument/completion")
			{
				json params = request["params"];
				std::string uri = params["textDocument"]["uri"];

				Document* document = GetDocument(uri);

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
		else
		{
			int id = request["id"];
			//fprintf(stderr, "Received response with id %d\n", id);
		}
	}

	destroyTypeSystem(&types);
}
