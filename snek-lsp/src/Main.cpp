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

		fprintf(stderr, "error %s:%d:%d: %s\n", document->localPath, diagnostics->items[i].startLine + 1, diagnostics->items[i].startCol + 1, diagnostics->items[i].message);
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

		std::string srcStr = stream.str();
		document->tmpFile = (File*)calloc(1, sizeof(File));
		initFile(document->tmpFile, document->localPath, document->uri.c_str(), srcStr.c_str(), (int)srcStr.size(), &blockPool);

		initParser(&document->parser, document->tmpFile);

		parse(&document->parser, &document->tmpFile->ast);

		resolveDependencies(&document->parser, document->tmpFile);
	}

	uint64_t afterParse = GetTimeNS();
	float parseMs = (afterParse - beforeParse) / 1e6f;
	fprintf(stderr, "parsed %d documents in %.3fms\n", documents.size, parseMs);

	uint64_t beforeTypeCheck = GetTimeNS();

	for (int i = 0; i < documents.size; i++)
	{
		Document* document = documents[i];

		initTypeTable(&document->tmpFile->typeTable, &document->tmpFile->arena, 64);
		initTypeChecker(&document->typeChecker, &document->tmpFile->arena, &document->tmpFile->scratch, &document->tmpFile->diagnostics, &types);

		symbolCollection(&document->typeChecker, document->tmpFile);
	}

	for (int i = 0; i < documents.size; i++)
	{
		Document* document = documents[i];

		symbolResolution(&document->typeChecker, document->tmpFile);
	}

	for (int i = 0; i < documents.size; i++)
	{
		Document* document = documents[i];

		typeCheckFunctions(&document->typeChecker, document->tmpFile);

		document->state = DOCUMENT_STATE_TYPECHECKED;
		document->needsReparse = false;
	}

	for (int i = 0; i < documents.size; i++)
	{
		Document* document = documents[i];
		sendDiagnosticsNotification(&document->tmpFile->diagnostics, document);
	}

	// swap ast
	for (int i = 0; i < documents.size; i++)
	{
		Document* document = documents[i];

		document->astMutex.lock();

		if (document->file)
			destroyFile(document->file);
		document->file = document->tmpFile;
		document->tmpFile = nullptr;

		document->astMutex.unlock();
	}

	sendRequest("workspace/semanticTokens/refresh", nullptr);

	uint64_t afterTypeCheck = GetTimeNS();
	float typeCheckMs = (afterTypeCheck - beforeTypeCheck) / 1e6f;
	fprintf(stderr, "typechecked %d documents in %.3fms\n", documents.size, typeCheckMs);
	fprintf(stderr, "memory blocks allocated: %d\n", blockPool.blockCount);
	fprintf(stderr, "type table size: %d/%d\n", types.typeTable.count, types.typeTable.capacity);
}

static Document* OpenDocument(std::string uri, std::string text)
{
	namespace fs = std::filesystem;

	fprintf(stderr, "Opening document %s\n", uri.c_str());

	fs::path mainFileDirectory = fs::path(mainFilePath).parent_path();
	fs::path localPath = fs::relative(URIToPath(uri), mainFileDirectory);
	std::string localPathStr = localPath.string();

	Document* document = (Document*)calloc(1, sizeof(Document));
	document->init(uri, localPathStr.c_str(), text, &types, &blockPool);

	documents.add(document);
	uriMap.emplace(document->uri, documents.size - 1);

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
		File* file = documents[i]->tmpFile ? documents[i]->tmpFile : documents[i]->file;
		if (file->handle == fileHandle)
			return file;
	}
	return nullptr;
}

bool isFileLoaded(FileHandle fileHandle)
{
	for (int i = 0; i < documents.size; i++)
	{
		if (documents[i]->fileHandle == fileHandle)
			return true;
	}
	return false;
}

File* getFileFromHandleLSP(FileHandle fileHandle)
{
	for (int i = 0; i < documents.size; i++)
	{
		if (documents[i]->file && documents[i]->file->handle == fileHandle)
			return documents[i]->file;
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
		if (documents[i]->file && documents[i]->file->handle == file)
			return documents[i];
	}
	return nullptr;
}

static bool containsFile(List<Document*>& documents, FileHandle file)
{
	for (int i = 0; i < documents.size; i++)
	{
		if (documents[i]->file && documents[i]->file->handle == file)
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
			if (document->state < DOCUMENT_STATE_TYPECHECKED)
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

		parseList.clear();

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

			if (document->file)
			{
				for (int j = 0; j < document->file->dependencies.size; j++)
				{
					FileHandle dependency = document->file->dependencies[j];
					if (containsFile(parseList, dependency))
					{
						document->needsReparse = true;
						break;
					}
				}
			}
		}

		for (int i = 0; i < documents.size; i++)
		{
			Document* document = documents[i];

			if (document->needsReparse && document->file && areDependenciesParsed(document->file))
			{
				if (!parseList.contains(document))
					parseList.add(document);
			}
		}

		if (parseList.size > 0)
			Parse(parseList);
		else
			SleepMS(10);
	}
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

static void waitForDocumentParse(Document* document)
{
	for (int i = 0; i < 500; i++)
	{
		document->astMutex.lock();
		bool parsed = document->file != nullptr && document->tmpFile == nullptr;
		document->astMutex.unlock();
		if (parsed)
			return;
		else
			SleepMS(10);
	}

	SnekAssert(false);
}

bool getSymbolInfoMarkdown(Symbol* symbol, json& result, std::string& detail)
{
	File* file = getFileFromHandleLSP(symbol->file);

	std::string markdown;

	std::stringstream stream;

	writeFilePath(file->localPath, stream);
	stream << '.';
	stream.write(symbol->name.ptr, symbol->name.length);
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
				StringView valueStr = getRangedString(declarator->value->start, declarator->value->end, file);
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
				StringView valueStr = getRangedString(declarator->value->start, declarator->value->end, file);
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

			if (struct_->hasBody)
			{
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
			else
			{
				stream << ';';
			}
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

			if (union_->hasBody)
			{
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
			else
			{
				stream << ';';
			}
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
			{"kind", "markdown"},
			{"value", markdown}
		};
	}

	return true;
}

bool getFieldInfoMarkdown(Type* type, int fieldID, json& result, std::string& detail)
{
	std::string markdown;

	std::stringstream stream;

	if (type->typeKind == TYPE_STRUCT)
	{
		Struct* declaration = type->struct_.declaration;
		Symbol* symbol = declaration->symbol;
		Document* document = getDocument(symbol->file);

		StringView fieldName = type->struct_.fieldNames[fieldID];

		writeFilePath(document->localPath, stream);
		stream << '.';
		stream.write(type->name.ptr, type->name.length);
		stream << '.';
		stream.write(fieldName.ptr, fieldName.length);
		detail = stream.str();
		stream.str("");
		stream.clear();

		Type* fieldType = type->struct_.fieldTypes[fieldID];

		stream.write(fieldType->name.ptr, fieldType->name.length);
		stream << ' ';
		stream.write(type->name.ptr, type->name.length);
		stream << '.';
		stream.write(fieldName.ptr, fieldName.length);

		int fieldOffset = type->struct_.fieldOffsets[fieldID];
		if (fieldOffset != -1)
			stream << " @offset(" << fieldOffset << ')';

		stream << ';';

		markdown = stream.str();
	}
	else if (type->typeKind == TYPE_UNION)
	{
		Union* declaration = type->union_.declaration;
		Symbol* symbol = declaration->symbol;
		Document* document = getDocument(symbol->file);

		StringView fieldName = type->union_.fieldNames[fieldID];

		writeFilePath(document->localPath, stream);
		stream << '.';
		stream.write(type->name.ptr, type->name.length);
		stream << '.';
		stream.write(fieldName.ptr, fieldName.length);
		detail = stream.str();
		stream.str("");
		stream.clear();

		Type* fieldType = type->union_.fieldTypes[fieldID];

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

		StringView name = declaration->values[fieldID]->name;

		writeFilePath(document->localPath, stream);
		stream << '.';
		stream.write(type->name.ptr, type->name.length);
		stream << '.';
		stream.write(name.ptr, name.length);
		detail = stream.str();
		stream.str("");
		stream.clear();

		stream.write(type->name.ptr, type->name.length);
		stream << ' ';
		stream.write(name.ptr, name.length);
		stream << " = ";

		EnumValue* enumValue = declaration->values[fieldID];
		StringView valueStr = getRangedString(enumValue->value->start, enumValue->value->end, document->file);
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
			{"kind", "markdown"},
			{"value", markdown}
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
						{"hoverProvider", true},
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
					waitForDocumentParse(document);
					document->astMutex.lock();

					std::vector<int> data;
					document->getTokens(data);

					document->astMutex.unlock();

					sendResponse(id, {
						{"data", data}
						});
				}
				else
				{
					fprintf(stderr, "Document %s not found\n", uri.c_str());
					sendErrorResponse(id, -32801);
				}
			}
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

				json items = json::array();

				Node* node = nullptr;
				Scope* scope = nullptr;

				waitForDocumentParse(document);
				document->astMutex.lock();

				uint64_t beforeComplete = GetTimeNS();

				if (triggerCharacter)
				{
					document->getNodeAtPosition(line, character - 2, &node, &scope);
					if (node)
						document->autocomplete(node, triggerCharacter, items);
				}
				else
				{
					document->getNodeAtPosition(line, character - 1, &node, &scope);
					if (scope)
						document->autocomplete(scope, items);
				}

				uint64_t afterComplete = GetTimeNS();

				document->astMutex.unlock();

				float ms = (afterComplete - beforeComplete) / 1e6f;
				fprintf(stderr, "autocomplete in %.3fms\n", ms);

				sendResponse(request["id"], {
					{"isIncomplete", false},
					{"items", items}
					});
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

					Document* document = getDocument(fileHandle);

					waitForDocumentParse(document);
					document->astMutex.lock();

					File* file = getFileFromHandleLSP(fileHandle);

					Symbol* symbol = nullptr;
					if (file)
						symbol = lookupSymbol(&file->ast.globalScope->symbols, symbolHandle);

					std::string detail;
					json markdown;
					if (symbol && getSymbolInfoMarkdown(symbol, markdown, detail))
					{
						sendResponse(request["id"], {
							{"label", label},
							{"kind", kind},
							{"detail", detail},
							{"documentation", markdown}
							});
					}
					else
					{
						sendErrorResponse(request["id"], -32801);
					}

					document->astMutex.unlock();
				}
				else if (data.contains("type_id") && data.contains("field_id"))
				{
					std::string typeIDStr = data["type_id"];
					int fieldID = data["field_id"];

					Type* type = (Type*)std::stoull(typeIDStr);

					std::string detail;
					json markdown;
					if (getFieldInfoMarkdown(type, fieldID, markdown, detail))
					{
						sendResponse(request["id"], {
							{"label", label},
							{"kind", kind},
							{"detail", detail},
							{"documentation", markdown}
							});
					}
					else
					{
						sendErrorResponse(request["id"], -32801);
					}
				}
			}
			else if (method == "textDocument/hover")
			{
				json params = request["params"];
				std::string uri = params["textDocument"]["uri"];

				Document* document = GetDocument(uri);

				json position = params["position"];
				int line = position["line"];
				int character = position["character"];

				SourceLocation start, end;
				json result;
				if (document->getHoverInfo(line, character, result, start, end))
				{
					sendResponse(request["id"], {
						{"contents", result },
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
				else
				{
					sendErrorResponse(request["id"], -32801);
				}
			}
			else if (method == "textDocument/documentSymbol")
			{
				json params = request["params"];
				std::string uri = params["textDocument"]["uri"];

				Document* document = GetDocument(uri);

				json result = json::array();

				waitForDocumentParse(document);
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

				waitForDocumentParse(document);
				document->astMutex.lock();

				json result;
				if (document->getDefinitionLocation(line, character, result))
				{
					sendResponse(request["id"], result);
				}
				else
				{
					sendErrorResponse(request["id"], -32801);
				}

				document->astMutex.unlock();
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

				waitForDocumentParse(document);
				document->astMutex.lock();

				Node* node = nullptr;
				Scope* scope = nullptr;
				document->getNodeAtPosition(line, character, &node, &scope);

				Symbol* symbol = nullptr;
				int overloadIdx = -1;
				if (node)
					symbol = document->getNodeSymbol(node, line, character, &overloadIdx);

				if (symbol)
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
						getSourceLocation(symbolDocument->file, declaration, &start, &end);

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

				document->astMutex.unlock();
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

				waitForDocumentParse(document);
				document->astMutex.lock();

				Node* node = nullptr;
				Scope* scope = nullptr;
				document->getNodeAtPosition(line, character, &node, &scope);

				Symbol* symbol = nullptr;
				int overloadIdx = -1;
				if (node)
					symbol = document->getNodeSymbol(node, line, character, &overloadIdx);

				document->astMutex.unlock();

				if (symbol)
				{
					json changes = json::object();

					for (int i = 0; i < documents.size; i++)
					{
						json fileChanges = json::array();

						waitForDocumentParse(documents[i]);
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

				json position = params["position"];
				int line = position["line"];
				int character = position["character"];

				json signatures = json::array();
				int activeSignature = 0;
				int activeParameter = 0;

				waitForDocumentParse(document);
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
			else if (method == "workspace/symbol")
			{
				json params = request["params"];
				std::string query = params["query"];

				uint64_t beforeComplete = GetTimeNS();

				json result = json::array();

				for (int i = 0; i < documents.size; i++)
				{
					Document* document = documents[i];

					waitForDocumentParse(document);
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
