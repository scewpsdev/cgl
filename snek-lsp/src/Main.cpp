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
Arena globalArena;
TypeSystem types;
Codegen codegen;

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
		unsigned char uc = (unsigned char)c;
		switch (uc) {
		case '"':  escaped << "\\\""; break;
		case '\\': escaped << "\\\\"; break;
		case '\b': escaped << "\\b";  break;
		case '\f': escaped << "\\f";  break;
		case '\n': escaped << "\\n";  break;
		case '\r': escaped << "\\r";  break;
		case '\t': escaped << "\\t";  break;
		default:
			// Handle control characters (ASCII 0 to 31) using \u00xx format
			if (uc < 32 || uc == 127) {
				escaped << "\\u"
					<< std::hex << std::setw(4) << std::setfill('0')
					<< uc
					<< std::dec;
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

		document->astMutex.lock();

		document->text = stream.str();
		stream.clear();

		resetArena(&document->file.arena);
		resetScratchBuffer(&document->file.scratch);
		resetDiagnostics(&document->file.diagnostics, DIAGNOSTICS_PARSER_STAGE);

		document->file.diagnostics.stage = DIAGNOSTICS_PARSER_STAGE;

		document->file.ast = {};

		document->file.dependencies.clear();

		if (document->file.parser.arena)
			destroyParser(&document->file.parser);
		initParser(&document->file.parser, document->uri.c_str(), document->text.c_str(), (int)document->text.size(), &document->file.arena, &document->file.scratch, &document->file.diagnostics);

		parse(&document->file.parser, &document->file.ast);
		document->state = DOCUMENT_STATE_PARSED;
		document->needsTypeCheck = true;
		document->lastChange = 0;

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
			//clearInternedTypes(&document->file, &types);
			resetAST(&document->file.ast);

			resetDiagnostics(&document->file.diagnostics, DIAGNOSTICS_TYPECHECK_STAGE);
		}

		document->file.diagnostics.stage = DIAGNOSTICS_TYPECHECK_STAGE;

		if (document->file.typeChecker.arena)
			destroyTypeChecker(&document->file.typeChecker);

		initTypeTable(&document->file.typeTable, &document->file.arena, 64);

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
			emitFile(&codegen, &document->file, document->localPath.c_str(), outPath.c_str());
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

File* getFileFromHandle(FileHandle fileHandle)
{
	for (int i = 0; i < documents.size; i++)
	{
		if (documents[i]->file.handle == fileHandle)
			return &documents[i]->file;
	}
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
		else
		{
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

static bool waitForDocumentTypeCheck(Document* document, int timeoutMs)
{
	int numIterations = timeoutMs / 10;
	for (int i = 0; i < numIterations; i++)
	{
		if (document->state == DOCUMENT_STATE_TYPECHECKED && document->lastChange == 0)
			return true;
		SleepMS(10);
	}
	return false;
}

static void getFunctionDetailString(Function* function, std::stringstream& stream)
{
	if (function->storage & STORAGE_DLLEXPORT)
		stream << "dllexport ";
	else if (function->storage & STORAGE_DLLIMPORT)
		stream << "dllimport ";
	else if (function->storage & STORAGE_PRIVATE)
		stream << "private ";
	if (function->storage & STORAGE_EXTERN)
		stream << "externc ";
	if (function->storage & STORAGE_NOMANGLE)
		stream << "nomangle ";

	if (stream.tellp())
		stream << "\n";

	stream << "func ";
	stream.write(function->name.ptr, function->name.length);
	stream << '(';

	if (function->numParams)
		stream << '\n';

	for (int j = 0; j < function->numParams; j++)
	{
		Type* paramType = function->params[j]->paramType->inferredType;
		std::string paramName = std::string(function->params[j]->name.ptr, function->params[j]->name.length);

		std::string paramTypeStr = std::string(paramType->name.ptr, paramType->name.length);

		stream << '\t' << paramTypeStr;

		if (function->functionType->function.variadic && j == function->numParams - 1)
		{
			stream << "...";
		}

		stream << ' ' << paramName;
		if (j < function->numParams - 1)
			stream << ",\n";
	}

	if (function->numParams)
		stream << '\n';

	stream << ')';

	if (function->functionType->function.returnType)
	{
		Type* returnType = function->functionType->function.returnType;
		stream << ' ' << std::string(returnType->name.ptr, returnType->name.length);
	}

	stream << ';';
}

static void writeFilePath(const std::string& localPath, std::stringstream& stream)
{
	size_t lastSlash = localPath.find_last_of("/\\");
	size_t lastDot = localPath.find_last_of('.');

	size_t end = localPath.length();
	if (lastDot != std::string::npos && (lastSlash == std::string::npos || lastDot > lastSlash)) {
		end = lastDot;
	}

	for (size_t i = 0; i < end; i++)
	{
		const char& c = localPath[i];
		if (c == '/' || c == '\\')
			stream << '.';
		else
			stream << c;
	}
}

static bool resolveCompletionItem(std::string label, int kind, uint32_t symbolHandle, FileHandle fileHandle, json& result)
{
	if (File* file = getFileFromHandle(fileHandle))
	{
		Document* document = getDocument(fileHandle);

		if (Symbol* symbol = lookupSymbol(&file->ast.globalScope->symbols, symbolHandle))
		{
			std::string detail;
			std::string markdown;

			std::stringstream stream;

			writeFilePath(document->localPath, stream);
			stream << '.' << label;
			detail = stream.str();
			stream.str("");
			stream.clear();

			if (symbol->type == SYMBOL_VARIABLE)
			{
				if (symbol->declaration->type == NODE_VARIABLE_DECLARATION)
				{
					VariableDeclaration* variable = &symbol->declaration->variableDeclaration;
					VariableDeclarator* declarator = getDeclarator(variable, symbol->name);

					Type* type = variable->variableType->inferredType;

					if (variable->storage & STORAGE_DLLEXPORT)
						stream << "dllexport ";
					else if (variable->storage & STORAGE_DLLIMPORT)
						stream << "dllimport ";
					else if (variable->storage & STORAGE_PRIVATE)
						stream << "private ";
					if (variable->storage & STORAGE_EXTERN)
						stream << "externc ";
					if (variable->storage & STORAGE_CONSTANT)
						stream << "const ";
					if (variable->storage & STORAGE_NOMANGLE)
						stream << "nomangle ";

					if (stream.tellp())
						stream << "\n";

					stream.write(type->name.ptr, type->name.length);
					stream << ' ';
					stream.write(symbol->name.ptr, symbol->name.length);

					if (variable->storage & STORAGE_CONSTANT && declarator->value)
					{
						StringView valueStr = getRangedString(declarator->value->start, declarator->value->end, &file->parser);
						stream << " = ";
						stream.write(valueStr.ptr, valueStr.length);
					}

					markdown = stream.str();
				}
				else if (symbol->declaration->type == NODE_GLOBAL_VARIABLE)
				{
					GlobalVariable* variable = &symbol->declaration->globalVariable;
					VariableDeclarator* declarator = getDeclarator(variable, symbol->name);

					Type* type = variable->variableType->inferredType;

					if (variable->storage & STORAGE_DLLEXPORT)
						stream << "dllexport ";
					else if (variable->storage & STORAGE_DLLIMPORT)
						stream << "dllimport ";
					else if (variable->storage & STORAGE_PRIVATE)
						stream << "private ";
					if (variable->storage & STORAGE_EXTERN)
						stream << "externc ";
					if (variable->storage & STORAGE_CONSTANT)
						stream << "const ";
					if (variable->storage & STORAGE_NOMANGLE)
						stream << "nomangle ";

					if (stream.tellp())
						stream << "\n";

					stream.write(type->name.ptr, type->name.length);
					stream << ' ';
					stream.write(symbol->name.ptr, symbol->name.length);

					if (variable->storage & STORAGE_CONSTANT && declarator->value)
					{
						StringView valueStr = getRangedString(declarator->value->start, declarator->value->end, &file->parser);
						stream << " = ";
						stream.write(valueStr.ptr, valueStr.length);
					}

					markdown = stream.str();
				}
				else if (symbol->declaration->type == NODE_PARAMETER)
				{
					Parameter* parameter = &symbol->declaration->parameter;

					Type* type = parameter->paramType->inferredType;

					stream.write(type->name.ptr, type->name.length);
					stream << ' ';
					stream.write(symbol->name.ptr, symbol->name.length);

					/*
					if (parameter->value)
					{
						StringView valueStr = getRangedString(declarator->value->start, declarator->value->end, &file->parser);
						stream << " = ";
						stream.write(valueStr.ptr, valueStr.length);
					}
					*/

					markdown = stream.str();
				}
				else if (symbol->declaration->type == NODE_FOR)
				{
					For* for_ = &symbol->declaration->for_;

					detail = "for iterator " + detail;

					stream << "int32 ";
					stream.write(for_->iteratorName.ptr, for_->iteratorName.length);

					markdown = stream.str();
				}
			}
			else if (symbol->type == SYMBOL_TYPE)
			{
				if (symbol->declaration->type == NODE_STRUCT)
				{
					Struct* struct_ = &symbol->declaration->struct_;

					if (struct_->storage & STORAGE_PRIVATE)
						stream << "private ";
					if (struct_->storage & STORAGE_PACKED)
						stream << "packed ";
					if (struct_->storage & STORAGE_NOMANGLE)
						stream << "nomangle ";

					if (stream.tellp())
						stream << "\n";

					stream << "struct ";

					stream.write(struct_->name.ptr, struct_->name.length);
					stream << " {\n";

					for (int i = 0; i < struct_->numFields; i++)
					{
						Field* field = struct_->fields[i];
						Type* fieldType = field->variableType->inferredType;

						stream << "\t";
						stream.write(fieldType->name.ptr, fieldType->name.length);
						stream << ' ';

						for (int j = 0; j < field->numDeclarators; j++)
						{
							stream.write(field->declarators[j].name.ptr, field->declarators[j].name.length);

							if (field->declarators[j].hasOffset)
							{
								stream << " @offset(" << field->declarators[j].offset << ')';
							}

							if (j < field->numDeclarators - 1)
								stream << ", ";
						}

						stream << ";\n";
					}

					stream << '}';
				}
				else if (symbol->declaration->type == NODE_UNION)
				{
					Union* union_ = &symbol->declaration->union_;

					if (union_->storage & STORAGE_PRIVATE)
						stream << "private ";
					if (union_->storage & STORAGE_PACKED)
						stream << "packed ";
					if (union_->storage & STORAGE_NOMANGLE)
						stream << "nomangle ";

					if (stream.tellp())
						stream << "\n";

					stream << "union ";

					stream.write(union_->name.ptr, union_->name.length);
					stream << " {\n";

					for (int i = 0; i < union_->numFields; i++)
					{
						Field* field = union_->fields[i];
						Type* fieldType = field->variableType->inferredType;

						stream << "\t";
						stream.write(fieldType->name.ptr, fieldType->name.length);
						stream << ' ';

						for (int j = 0; j < field->numDeclarators; j++)
						{
							stream.write(field->declarators[j].name.ptr, field->declarators[j].name.length);
							if (j < field->numDeclarators - 1)
								stream << ", ";
						}

						stream << ";\n";
					}

					stream << '}';
				}
				else if (symbol->declaration->type == NODE_ENUM)
				{
					Enum* enum_ = &symbol->declaration->enum_;

					if (enum_->storage & STORAGE_PRIVATE)
						stream << "private ";
					if (enum_->storage & STORAGE_PACKED)
						stream << "packed ";
					if (enum_->storage & STORAGE_NOMANGLE)
						stream << "nomangle ";

					if (stream.tellp())
						stream << "\n";

					stream << "enum ";

					stream.write(enum_->name.ptr, enum_->name.length);

					if (enum_->valueType)
					{
						Type* valueType = enum_->valueType->inferredType;
						stream << " = ";
						stream.write(valueType->name.ptr, valueType->name.length);
					}

					stream << ';';
				}
				else if (symbol->declaration->type == NODE_TYPEDEF)
				{
					Typedef* typedef_ = &symbol->declaration->typedef_;

					if (typedef_->storage & STORAGE_PRIVATE)
						stream << "private ";
					if (typedef_->storage & STORAGE_PACKED)
						stream << "packed ";
					if (typedef_->storage & STORAGE_NOMANGLE)
						stream << "nomangle ";

					if (stream.tellp())
						stream << "\n";

					stream << "type ";

					stream.write(typedef_->name.ptr, typedef_->name.length);

					Type* valueType = typedef_->value->inferredType;
					stream << " = ";
					stream.write(valueType->name.ptr, valueType->name.length);

					stream << ';';
				}

				markdown = stream.str();
			}
			else if (symbol->type == SYMBOL_FUNCTION_SET && symbol->functionSet.count)
			{
				if (symbol->functionSet.count > 1)
				{
					detail = detail + "(...) : " + std::to_string(symbol->functionSet.count) + " overloads";
				}

				for (int i = 0; i < symbol->functionSet.count; i++)
				{
					getFunctionDetailString(symbol->functionSet.overloads[i].declaration, stream);
					if (i < symbol->functionSet.count - 1)
						stream << "\n";
				}
				markdown = stream.str();
			}

			if (detail != "" || markdown != "")
			{
				if (markdown != "")
				{
					markdown = "```sneklang\n" + markdown + "\n```";
				}

				result = {
					{"label", label},
					{"kind", kind},
					{"detail", detail},
					{"documentation", {
						{"kind", "markdown"},
						{"value", markdown}
					}},
					{"data", {
						{"symbol_id", std::to_string(symbolHandle)},
						{"file_id", std::to_string((uint64_t)fileHandle)}
					}}
				};
			}

			return true;
		}
	}

	return false;
}

static bool resolveCompletionItem(std::string label, int kind, Type* type, int fieldID, json& result)
{
	std::string detail;
	std::string markdown;

	std::stringstream stream;

	if (type->typeKind == TYPE_STRUCT)
	{
		Struct* declaration = type->struct_.declaration;
		Symbol* symbol = declaration->symbol;
		Document* document = getDocument(symbol->file);

		writeFilePath(document->localPath, stream);
		stream << '.';
		stream.write(type->name.ptr, type->name.length);
		stream << '.' << label;
		detail = stream.str();
		stream.str("");
		stream.clear();

		Type* fieldType = type->struct_.fieldTypes[fieldID];
		StringView fieldName = type->struct_.fieldNames[fieldID];

		stream.write(fieldType->name.ptr, fieldType->name.length);
		stream << ' ';
		stream.write(type->name.ptr, type->name.length);
		stream << '.';
		stream.write(fieldName.ptr, fieldName.length);
		stream << ';';

		markdown = stream.str();
	}
	else if (type->typeKind == TYPE_UNION)
	{
		Union* declaration = type->union_.declaration;
		Symbol* symbol = declaration->symbol;
		Document* document = getDocument(symbol->file);

		writeFilePath(document->localPath, stream);
		stream << '.';
		stream.write(type->name.ptr, type->name.length);
		stream << '.' << label;
		detail = stream.str();
		stream.str("");
		stream.clear();

		Type* fieldType = type->union_.fieldTypes[fieldID];
		StringView fieldName = type->union_.fieldNames[fieldID];

		stream.write(fieldType->name.ptr, fieldType->name.length);
		stream << ' ';
		stream.write(type->name.ptr, type->name.length);
		stream << '.';
		stream.write(fieldName.ptr, fieldName.length);
		stream << ';';

		markdown = stream.str();
	}
	else if (type->typeKind == TYPE_ENUM)
	{
		Enum* declaration = type->enum_.declaration;
		Symbol* symbol = declaration->symbol;
		Document* document = getDocument(symbol->file);

		writeFilePath(document->localPath, stream);
		stream << '.';
		stream.write(type->name.ptr, type->name.length);
		stream << '.' << label;
		detail = stream.str();
		stream.str("");
		stream.clear();

		stream.write(type->name.ptr, type->name.length);
		stream << ' ' << label << " = ";

		EnumValue* enumValue = declaration->values[fieldID];
		StringView valueStr = getRangedString(enumValue->value->start, enumValue->value->end, &document->file.parser);
		stream.write(valueStr.ptr, valueStr.length);

		stream << ';';

		markdown = stream.str();
	}
	else if (type->typeKind == TYPE_STRING)
	{
		if (fieldID == 0)
			stream << "char* string.ptr;";
		else if (fieldID == 1)
			stream << "int string.length;";

		markdown = stream.str();
	}
	else if (type->typeKind == TYPE_ARRAY)
	{
		if (fieldID == 0)
		{
			stream.write(type->array.elementType->name.ptr, type->array.elementType->name.length);
			stream << "* ";
			stream.write(type->name.ptr, type->name.length);
			stream << ".data;";
		}
		else if (fieldID == 1)
		{
			stream << "int ";
			stream.write(type->name.ptr, type->name.length);
			stream << ".length;";

			stream << "int array.length;";
		}

		markdown = stream.str();
	}
	else if (type->typeKind == TYPE_ANY)
	{
		if (fieldID == 0)
			stream << "byte* any.value;";
		else if (fieldID == 1)
			stream << "int any.type;";

		markdown = stream.str();
	}
	else if (type->typeKind == TYPE_OPTIONAL)
	{
		stream.write(type->optional.elementType->name.ptr, type->optional.elementType->name.length);
		stream << ' ';
		stream.write(type->name.ptr, type->name.length);
		stream << ".value;";

		markdown = stream.str();
	}

	if (detail != "" || markdown != "")
	{
		if (markdown != "")
		{
			markdown = "```sneklang\n" + markdown + "\n```";
		}

		result = {
			{"label", label},
			{"kind", kind},
			{"detail", detail},
			{"documentation", {
				{"kind", "markdown"},
				{"value", markdown}
			}}
		};

		return true;
	}

	return false;
}

int main()
{
	SleepMS(5000);
	fprintf(stderr, "Starting LSP Server\n");

	initGlobalBlockPool(&blockPool, 16);
	initArena(&globalArena, &blockPool);
	initTypeSystem(&types, &globalArena);
	initCodegen(&codegen, &types, &globalArena);

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
						{"documentSymbolProvider", true},
						{"workspaceSymbolProvider", true},
						{"definitionProvider", true},
						{"referencesProvider", true},
						{"renameProvider", true},
						//{"hoverProvider", true},
						{"signatureHelpProvider", {
							{"triggerCharacters", {"(", ","}},
							{"retriggerCharacters", {",", ")"}}
						}},
						{"completionProvider", {
							{"resolveProvider", true},
							{"triggerCharacters", {"."}}
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
				int line = position["line"];
				int character = position["character"];

				int triggerKind = 0;
				char triggerCharacter = 0;
				if (params.contains("context"))
				{
					json context = params["context"];
					triggerKind = context["triggerKind"];
					if (triggerKind == 2) // triggerCharacter
					{
						std::string triggerCharacterStr = context["triggerCharacter"];
						triggerCharacter = triggerCharacterStr[0];
					}
				}

				// normal identifier completion
				if (triggerKind >= 0 && triggerKind <= 2) // invoke
				{
					if (waitForDocumentTypeCheck(document, 200))
					{
						json items = json::array();

						Node* node = nullptr;
						Scope* scope = nullptr;

						document->astMutex.lock();

						uint64_t beforeComplete = GetTimeNS();

						document->getNodeAtPosition(line, character - (triggerCharacter ? 2 : 1), &node, &scope);

						if (node)
						{
							document->autocomplete(node, scope, triggerCharacter, items);
						}

						uint64_t afterComplete = GetTimeNS();

						document->astMutex.unlock();

						float ms = (afterComplete - beforeComplete) / 1e6f;
						fprintf(stderr, "autocomplete in %.3fms\n", ms);

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
				else
				{
					sendResponse(request["id"], {
						{"isIncomplete", false},
						{"items", json::array()}
						});
				}
			}
			else if (method == "completionItem/resolve")
			{
				json params = request["params"];

				std::string label = params["label"];
				int kind = params["kind"];

				json data = params["data"];

				if (data.contains("symbol_id") && data.contains("file_id"))
				{
					std::string symbolHandleStr = data["symbol_id"];
					std::string fileHandleStr = data["file_id"];

					uint32_t symbolHandle = std::stoul(symbolHandleStr);
					FileHandle fileHandle = std::stoull(fileHandleStr);

					json result;
					if (resolveCompletionItem(label, kind, symbolHandle, fileHandle, result))
					{
						sendResponse(request["id"], result);
					}
					else
					{
						sendErrorResponse(request["id"], -32801);
					}
				}
				else if (data.contains("type_id") && data.contains("field_id"))
				{
					std::string typeIDStr = data["type_id"];
					int fieldID = data["field_id"];

					Type* type = (Type*)std::stoull(typeIDStr);

					json result;
					if (resolveCompletionItem(label, kind, type, fieldID, result))
					{
						sendResponse(request["id"], result);
					}
					else
					{
						sendErrorResponse(request["id"], -32801);
					}
				}
			}
			else if (method == "textDocument/documentSymbol")
			{
				json params = request["params"];
				std::string uri = params["textDocument"]["uri"];

				Document* document = GetDocument(uri);

				json result = json::array();

				document->astMutex.lock();

				document->getSymbols(result);

				document->astMutex.unlock();

				sendResponse(request["id"], result);
			}
			else if (method == "textDocument/definition")
			{
				json params = request["params"];
				std::string uri = params["textDocument"]["uri"];

				Document* document = GetDocument(uri);

				json position = params["position"];
				int line = position["line"];
				int character = position["character"];

				json result;
				if (document->getDefinitionLocation(line, character, result))
				{
					sendResponse(request["id"], result);
				}
				else
				{
					sendErrorResponse(request["id"], -32801);
				}
			}
			else if (method == "textDocument/references")
			{
				json params = request["params"];
				std::string uri = params["textDocument"]["uri"];

				Document* document = GetDocument(uri);

				json position = params["position"];
				int line = position["line"];
				int character = position["character"];

				bool includeDeclaration = params["context"]["includeDeclaration"];

				int overloadIdx = -1;
				if (Symbol* symbol = document->getSymbolAtPosition(line, character, &overloadIdx))
				{
					json locations = json::array();

					for (int i = 0; i < documents.size; i++)
					{
						documents[i]->astMutex.lock();

						documents[i]->findAllReferences(symbol, locations);

						documents[i]->astMutex.unlock();
					}

					if (includeDeclaration)
					{
						Node* declaration = symbol->type == SYMBOL_FUNCTION_SET ? (Node*)symbol->functionSet.overloads[overloadIdx].declaration : symbol->declaration;
						Document* symbolDocument = getDocument(symbol->file);

						SourceLocation start, end;
						getSourceLocation(&symbolDocument->file.parser, declaration, &start, &end);

						locations.push_back({
							{"uri", symbolDocument->uri},
							{"range", {
								{"start", {
									{"line", start.line},
									{"character", start.col},
								}},
								{"end", {
									{"line", end.line},
									{"character", end.col},
								}},
							}}
							});
					}

					sendResponse(request["id"], locations);
				}
				else
				{
					sendErrorResponse(request["id"], -32801);
				}
			}
			else if (method == "textDocument/rename")
			{
				json params = request["params"];
				std::string uri = params["textDocument"]["uri"];

				Document* document = GetDocument(uri);

				json position = params["position"];
				int line = position["line"];
				int character = position["character"];

				std::string newName = params["newName"];

				int overloadIdx = -1;
				if (Symbol* symbol = document->getSymbolAtPosition(line, character, &overloadIdx))
				{
					json changes = json::object();

					for (int i = 0; i < documents.size; i++)
					{
						json fileChanges = json::array();

						documents[i]->astMutex.lock();

						documents[i]->rename(symbol, overloadIdx, newName, line, character, fileChanges);

						documents[i]->astMutex.unlock();

						if (fileChanges.size())
							changes.push_back({ documents[i]->uri, fileChanges });
					}

					sendResponse(request["id"], {
						{"changes", changes},
						});
				}
				else
				{
					sendErrorResponse(request["id"], -32801);
				}
			}
			else if (method == "textDocument/signatureHelp")
			{
				json params = request["params"];
				std::string uri = params["textDocument"]["uri"];

				Document* document = GetDocument(uri);

				if (waitForDocumentTypeCheck(document, 200))
				{
					json position = params["position"];
					int line = position["line"];
					int character = position["character"];

					json signatures = json::array();
					int activeSignature = 0;
					int activeParameter = 0;

					document->astMutex.lock();

					if (document->getFunctionSignature(line, character, signatures, activeSignature, activeParameter))
					{
						json result = {
							{"signatures", signatures},
							{"activeSignature", activeSignature},
							{"activeParameter", activeParameter},
						};

						sendResponse(request["id"], result);
					}
					else
					{
						sendResponse(request["id"], nullptr);
					}

					document->astMutex.unlock();
				}
				else
				{
					sendResponse(request["id"], nullptr);
				}
			}
			else if (method == "workspace/symbol")
			{
				json params = request["params"];
				std::string query = params["query"];

				uint64_t beforeComplete = GetTimeNS();

				json result = json::array();

				for (int i = 0; i < documents.size; i++)
				{
					Document* document = documents[i];

					document->astMutex.lock();

					document->getWorkspaceSymbols(query, result);

					document->astMutex.unlock();
				}

				sendResponse(request["id"], result);

				uint64_t afterComplete = GetTimeNS();

				float ms = (afterComplete - beforeComplete) / 1e6f;
				fprintf(stderr, "workspace symbols: %d in %.3fms\n", (int)result.size(), ms);
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
