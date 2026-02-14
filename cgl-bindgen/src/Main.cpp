#include <stdio.h>

#include "cgl/Platform.h"
#include "cgl/CGLCompiler.h"
#include "cgl/parser/lexer.h"
#include "cgl/parser/Parser.h"
#include "cgl/semantics/Resolver.h"
#include "cgl/semantics/Variable.h"

#include "tcc/libtcc.h"
#include "tcc/tcc.h"
#undef free

#include <tree_sitter/api.h>

#include <unordered_map>
#include <string>
#include <iostream>
#include <istream>
#include <fstream>
#include <set>


static std::unordered_map<uint32_t, std::pair<TSNode, int>> typedefs;
static std::set<uint32_t> defines;


extern "C" const TSLanguage* tree_sitter_c(void);


static bool isHeaderIncluded(const char* header, const List<char*>& headers)
{
	for (int i = 0; i < headers.size; i++)
	{
		if (strcmp(headers[i], header) == 0)
			return true;
	}
	return false;
}

static bool isStandardHeader(const char* header)
{
	static const char* standardHeaders[] = {
		"assert.h",
		"conio.h",
		"ctype.h",
		"dir.h",
		"direct.h",
		"dirent.h",
		"dos.h",
		"errno.h",
		"excpt.h",
		"fcntl.h",
		"fenv.h",
		"float.h",
		"inttypes.h",
		"io.h",
		"limits.h",
		"locale.h",
		"malloc.h",
		"math.h",
		"mem.h",
		"memory.h",
		"process.h",
		"setjmp.h",
		"share.h",
		"signal.h",
		"stdarg.h",
		"stdbool.h",
		"stddef.h",
		"stdint.h",
		"stdio.h",
		"stdlib.h",
		"string.h",
		"tchar.h",
		"time.h",
		"vadefs.h",
		"values.h",
		"varargs.h",
		"wchar.h",
		"wctype.h",
	};

	int numStandardHeaders = sizeof(standardHeaders) / sizeof(standardHeaders[0]);
	for (int i = 0; i < numStandardHeaders; i++)
	{
		if (strcmp(header, standardHeaders[i]) == 0)
			return true;
	}
	return false;
}

static std::string trim(std::string& str)
{
	str.erase(str.find_last_not_of(' ') + 1);  // Suffixing spaces
	str.erase(0, str.find_first_not_of(' '));  // Prefixing spaces
	return str;
}

static uint32_t hashString(const std::string& str)
{
	uint32_t hash = 7;
	for (int i = 0; i < (int)str.length(); i++)
	{
		char c = str[i];
		hash = hash * 31 + c;
	}

	return hash;
}

static bool parseInt(const char* str, int64_t* result, bool* isUnsigned)
{
	int64_t value = 0;
	bool isNegative = false;
	*isUnsigned = true;
	int base = 10;

	for (int i = 0; i < (int)strlen(str); i++)
	{
		char c = tolower(str[i]);
		if (c == '-')
		{
			SnekAssert(i == 0);
			isNegative = true;
		}
		else if (c == '_')
		{
			;
		}
		else if (isDigit(c))
		{
			value = value * base + (c - '0');
		}
		else if (isAlpha(c) && c >= 'a' && c <= 'f' && base == 16)
		{
			value = value * base + (c - 'a' + 10);
		}
		else if (isAlpha(c) && c >= 'A' && c <= 'F' && base == 16)
		{
			value = value * base + (c - 'A' + 10);
		}
		else if (base == 10 && c == 'b')
		{
			if (value == 0 && (i == 1 || i == 2 && isNegative))
				base = 2;
			else
				return false;
		}
		else if (base == 10 && c == 'o')
		{
			if (value == 0 && (i == 1 || i == 2 && isNegative))
				base = 8;
			else
				return false;
		}
		else if (base == 10 && c == 'x')
		{
			if (value == 0 && (i == 1 || i == 2 && isNegative))
				base = 16;
			else
				return false;
		}
		else if (c == 'u')
		{
			*isUnsigned = true;
		}
		else if (c == 'l')
		{

		}
		else
		{
			return false;
		}
	}

	if (isNegative)
	{
		value *= -1;
		*isUnsigned = false;
	}

	*result = value;
	return true;
}

static bool parseFloat(const char* str, double* result, bool* isDouble)
{
	char processed[32] = {};
	int len = 0;

	*isDouble = true;

	auto min = [](int a, int b) { return  a < b ? a : b; };

	for (int i = 0; i < min((int)strlen(str), 31); i++)
	{
		char c = str[i];
		if (c != '_' && (c != 'f' && c != 'F'))
			processed[len++] = c;
		if (c == 'f' || c == 'F')
			*isDouble = false;
	}
	processed[len] = 0;

	double value = atof(processed);

	if (value != 0)
	{
		*result = value;
		return true;
	}

	return false;
}

static std::string parseValue(std::string str, std::string* type)
{
	auto lineComment = str.find("//");
	if (lineComment != std::string::npos)
		str = str.substr(0, lineComment);

	auto comment = str.find("/*");
	if (comment != std::string::npos)
	{
		auto commentEnd = str.find("*/", comment + 2);
		if (commentEnd != std::string::npos)
		{
			str = str.substr(0, comment) + str.substr(commentEnd + 2);
		}
	}

	str = trim(str);

	int64_t i;
	bool isUnsigned;
	if (parseInt(str.c_str(), &i, &isUnsigned))
	{
		int bitWidth = 32;
		bool isSigned = !isUnsigned;
		if (i < 0)
		{
			isSigned = true;
			if (i > INT32_MAX || i < INT32_MIN)
				bitWidth = 64;
		}
		else
		{
			if (i > UINT32_MAX)
				bitWidth = 64;
		}

		*type = (isSigned ? "int" : "uint") + std::to_string(bitWidth);

		return str;
	}

	double f;
	bool isDouble;
	if (parseFloat(str.c_str(), &f, &isDouble))
	{
		*type = isDouble ? "double" : "float";

		return str;
	}

	return "";
}

static void parseConstants(const char* path, FILE* file)
{
	List<char*> headers;
	headers.add(_strdup(path));

	while (headers.size > 0)
	{
		std::ifstream stream(headers[0]);
		free(headers[0]);
		headers.removeAt(0);

		std::string line;

		for (std::string line; std::getline(stream, line);)
		{
			if (line.starts_with("#define "))
			{
				line = line.substr(8);
				auto space = line.find(' ');
				if (space != std::string::npos)
				{
					std::string name = line.substr(0, space);
					uint32_t nameHash = hashString(name);
					if (defines.find(nameHash) == defines.end())
					{
						KeywordType keywordType = getKeywordType(name.c_str(), (int)name.length());
						auto paren = name.find('(');
						if (keywordType == KeywordType::KEYWORD_TYPE_NULL && paren == std::string::npos)
						{
							std::string valueStr = line.substr(space + 1);
							std::string type;
							std::string value = parseValue(valueStr, &type);
							if (value != "")
							{
								fprintf(file, "const %s %s = %s;\n", type.c_str(), name.c_str(), value.c_str());
							}
						}
						defines.emplace(nameHash);
					}
				}
			}
			else if (line.starts_with("#include "))
			{
				line = line.substr(9);
				line = line.substr(1, line.size() - 2);

				if (!isHeaderIncluded(line.c_str(), headers) && !isStandardHeader(line.c_str()))
					headers.add(_strdup(line.c_str()));
			}
		}
	}
}

static bool preprocess(const char* path, const char* out)
{
	TCCState* tcc = tcc_new();

	tcc_set_lib_path(tcc, "D:\\Dev\\2023\\CGL\\cgl\\lib\\libtcc");

	tcc_add_include_path(tcc, "D:\\Dev\\2023\\CGL\\cgl-bindgen\\src\\tcc");
	tcc_add_include_path(tcc, ".");

	// apparently both tcc_set_lib_path and tcc_add_library_path are needed to find libtcc1-64.a?
	// also a copy named libtcc1.a has to be provided. amazing.
	tcc_add_library_path(tcc, "D:\\Dev\\2023\\CGL\\cgl\\lib\\libtcc");

	tcc_set_options(tcc, "-E -nostdinc -nostdlib");
	tcc_add_include_path(tcc, "D:\\Dev\\2023\\CGL\\cgl-bindgen\\lib\\stubs");

	/*
	tcc_add_include_path(tcc, "D:\\Dev\\2023\\CGL\\cgl\\lib\\libtcc\\include");

	tcc_add_include_path(tcc, "D:\\Dev\\2023\\CGL\\cgl\\lib");
	tcc_add_include_path(tcc, "D:\\Dev\\2023\\CGL\\cgl\\lib\\libtcc\\include");
	tcc_add_include_path(tcc, "D:\\Dev\\2023\\CGL\\cgl\\lib\\libtcc\\include\\winapi");

	*/

	//tcc_define_symbol(tcc, "_WIN32", nullptr);
	tcc_define_symbol(tcc, "SDL_DECLSPEC", "__declspec(dllimport)");

	tcc_set_output_type(tcc, TCC_OUTPUT_PREPROCESS);
	tcc->ppfp = fopen(out, "wb");

	char buffer[1024] = "";
	sprintf(buffer, "#include \"%s\"\n", path);

	int result = tcc_compile_string(tcc, buffer);
	if (result != 0)
		return false;

	fclose(tcc->ppfp);
	tcc_delete(tcc);

	return true;
}

static char* ReadText(const char* path)
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

static TSNode findNode(TSNode node, const char* type, bool recursive = false)
{
	for (int i = 0; i < ts_node_child_count(node); i++)
	{
		TSNode child = ts_node_child(node, i);
		const char* childType = ts_node_type(child);
		if (strcmp(type, childType) == 0)
			return child;
		else if (recursive)
		{
			TSNode n = findNode(child, type, recursive);
			if (!ts_node_is_null(n))
				return n;
		}
	}
	return {};
}

static bool hasNode(TSNode node, const char* type, bool recursive = false)
{
	for (int i = 0; i < ts_node_child_count(node); i++)
	{
		TSNode child = ts_node_child(node, i);
		if (strcmp(type, ts_node_type(child)) == 0)
			return true;
		else if (recursive)
		{
			if (hasNode(child, type, recursive))
				return true;
		}
	}
	return false;
}

static void printNode(TSNode node, int indentation = 0)
{
	for (int i = 0; i < indentation; i++)
		printf("\t");
	printf("%s\n", ts_node_type(node));
	for (int i = 0; i < ts_node_child_count(node); i++)
	{
		printNode(ts_node_child(node, i), indentation + 1);
	}
}

static bool nodeIsValue(TSNode node, const char* value, const char* src)
{
	int nameStart = ts_node_start_byte(node);
	int nameEnd = ts_node_end_byte(node);
	int nameLength = nameEnd - nameStart;
	const char* name = &src[nameStart];

	int len = (int)strlen(value);

	return len == nameLength && strncmp(name, value, len) == 0;
}

static uint32_t hashNodeValue(TSNode node, const char* src)
{
	int nameStart = ts_node_start_byte(node);
	int nameEnd = ts_node_end_byte(node);

	uint32_t hash = 7;
	for (int i = nameStart; i != nameEnd; i++)
	{
		char c = src[i];
		hash = hash * 31 + c;
	}

	return hash;
}

static void outputNodeValue(TSNode node, const char* src, FILE* file)
{
	int nameStart = ts_node_start_byte(node);
	int nameEnd = ts_node_end_byte(node);
	int nameLength = nameEnd - nameStart;
	const char* name = &src[nameStart];
	fprintf(file, "%.*s", nameLength, name);

	if (nameLength == 8 && strncmp(name, "SDL_Quit", 8) == 0)
	{
		int a = 5;
	}
}

static TSNode unwrapPointer(TSNode node, const char* type, int* pointers)
{
	*pointers = 0;

	TSNode pointer = findNode(node, "pointer_declarator");

	while (!ts_node_is_null(pointer) && strcmp(ts_node_type(pointer), "pointer_declarator") == 0)
	{
		(*pointers)++;
		TSNode newPointer = findNode(pointer, "pointer_declarator");
		if (ts_node_is_null(newPointer))
			return findNode(pointer, type); //ts_node_child(pointer, 1);
		else
			pointer = newPointer;
	}

	return pointer;
}

static TSNode unwrapPointer(TSNode node, int* pointers)
{
	*pointers = 0;

	TSNode pointer = findNode(node, "pointer_declarator");

	while (!ts_node_is_null(pointer) && strcmp(ts_node_type(pointer), "pointer_declarator") == 0)
	{
		(*pointers)++;
		TSNode newPointer = findNode(pointer, "pointer_declarator");
		if (ts_node_is_null(newPointer))
			return ts_node_child(pointer, 1);
		else
			pointer = newPointer;
	}

	return pointer;
}

static void parseTypeIdentifier(TSNode node, int pointers, const char* src, FILE* file, int indentation = 0, bool returnType = false);
static void parseField(TSNode field, const char* src, FILE* file, int indentation = 0);

static void parseAnonymousStruct(TSNode node, const char* src, FILE* file, int indentation)
{
	TSNode fields = findNode(node, "field_declaration_list");

	fprintf(file, "struct {\n");

	for (int i = 0; i < ts_node_child_count(fields); i++)
	{
		TSNode field = ts_node_child(fields, i);
		const char* fieldType = ts_node_type(field);
		if (strcmp(fieldType, "field_declaration") == 0)
		{
			parseField(field, src, file, indentation + 1);
			/*
			int pointers = 0;
			TSNode identifier = unwrapPointer(field, &pointers);
			if (ts_node_is_null(identifier))
				identifier = ts_node_child(field, 1);

			for (int j = 0; j < indentation + 1; j++)
				fprintf(file, "\t");
			parseTypeIdentifier(field, pointers, src, file, indentation + 1);
			fprintf(file, " ");
			outputNodeValue(identifier, src, file);
			fprintf(file, ";\n");
			*/
		}
	}

	for (int j = 0; j < indentation; j++)
		fprintf(file, "\t");
	fprintf(file, "}");
}

static void parseUnion(TSNode node, const char* src, FILE* file, int indentation)
{
	TSNode fields = findNode(node, "field_declaration_list");

	const char* n = &src[ts_node_start_byte(node)];

	fprintf(file, "union {\n");

	for (int i = 0; i < ts_node_child_count(fields); i++)
	{
		TSNode field = ts_node_child(fields, i);
		const char* fieldType = ts_node_type(field);
		if (strcmp(fieldType, "field_declaration") == 0)
		{
			parseField(field, src, file, indentation + 1);
			/*
			int pointers = 0;
			TSNode identifier = unwrapPointer(field, &pointers);
			if (ts_node_is_null(identifier))
				identifier = ts_node_child(field, 1);

			for (int j = 0; j < indentation + 1; j++)
				fprintf(file, "\t");
			parseTypeIdentifier(field, pointers, src, file, indentation + 1);
			fprintf(file, " ");
			outputNodeValue(identifier, src, file);
			fprintf(file, ";\n");
			*/
		}
	}

	for (int j = 0; j < indentation; j++)
		fprintf(file, "\t");
	fprintf(file, "}");
}

static void parseTypeIdentifier(TSNode node, int pointers, const char* src, FILE* file, int indentation, bool returnType)
{
	TSNode type = findNode(node, "primitive_type");

	TSNode sizedType = findNode(node, "sized_type_specifier");
	if (ts_node_is_null(type) && !ts_node_is_null(sizedType) && (nodeIsValue(ts_node_child(sizedType, 0), "short", src) || nodeIsValue(ts_node_child(sizedType, 0), "long", src)))
		type = sizedType;

	if (ts_node_is_null(type))
		type = findNode(node, "type_identifier");
	if (ts_node_is_null(type))
	{
		TSNode structSpecifier = findNode(node, "struct_specifier");
		if (!ts_node_is_null(structSpecifier))
		{
			if (!ts_node_is_null(findNode(structSpecifier, "field_declaration_list")))
			{
				parseAnonymousStruct(structSpecifier, src, file, indentation);
				return;
			}
			else
			{
				type = structSpecifier;
			}
		}
	}
	if (ts_node_is_null(type))
	{
		TSNode unionSpecifier = findNode(node, "union_specifier");
		if (!ts_node_is_null(unionSpecifier))
		{
			parseUnion(unionSpecifier, src, file, indentation);
			return;
		}
	}

	if (ts_node_is_null(type) && !ts_node_is_null(sizedType))
		type = sizedType;

	if (!ts_node_is_null(type))
	{
		int nameStart = ts_node_start_byte(type);
		int nameEnd = ts_node_end_byte(type);
		int nameLength = nameEnd - nameStart;
		const char* name = &src[nameStart];

		uint32_t nameHash = hashNodeValue(type, src);

		if (nameLength == 4 && strncmp(name, "void", 4) == 0 && pointers)
			fprintf(file, "byte");
		else if (nameLength == 4 && strncmp(name, "void", 4) == 0 && !pointers && returnType)
			;
		else if (typedefs.find(nameHash) != typedefs.end())
		{
			TSNode value = {};
			int nameLength = 0;
			const char* name = nullptr;

			while (typedefs.find(nameHash) != typedefs.end())
			{
				std::pair pair = typedefs[nameHash];
				value = pair.first;
				pointers += pair.second;

				int nameStart = ts_node_start_byte(value);
				int nameEnd = ts_node_end_byte(value);
				nameLength = nameEnd - nameStart;
				name = &src[nameStart];

				nameHash = hashNodeValue(value, src);
			}

			if (nameLength > 7 && strncmp(name, "struct", 6) == 0)
			{
				name += 7;
				nameLength -= 7;
			}

			fprintf(file, "%.*s", nameLength, name);

			//outputNodeValue(value, src, file);
		}
		else
		{
			int nameStart = ts_node_start_byte(type);
			int nameEnd = ts_node_end_byte(type);
			int nameLength = nameEnd - nameStart;
			const char* name = &src[nameStart];

			if (nameLength > 7 && strncmp(name, "struct", 6) == 0)
			{
				name += 7;
				nameLength -= 7;
			}
			else
			{
				bool unsignd = false;
				if (nameLength > 9 && strncmp(name, "unsigned ", 9) == 0)
				{
					name += 9;
					nameLength -= 9;
					unsignd = true;
				}
				if (nameLength > 5 && strncmp(name, "long ", 5) == 0)
				{
					name += 5;
					nameLength -= 5;
				}

				if (unsignd)
				{
					name -= 1;
					nameLength += 1;
					((char*)name)[0] = 'u';
				}
			}

			fprintf(file, "%.*s", nameLength, name);
		}

		for (int i = 0; i < pointers; i++)
		{
			fprintf(file, "*");
		}
	}
}

static TSNode _getType(TSNode node)
{
	TSNode type = findNode(node, "primitive_type");
	if (ts_node_is_null(type))
		type = findNode(node, "type_identifier");
	if (ts_node_is_null(type))
		type = findNode(node, "sized_type_specifier");
	if (ts_node_is_null(type))
	{
		TSNode structSpecifier = findNode(node, "struct_specifier");
		if (!ts_node_is_null(structSpecifier))
			type = _getType(structSpecifier);
	}
	return type;
}

static void parseParameter(TSNode node, int idx, const char* src, FILE* file)
{
	int pointers;
	TSNode identifier = unwrapPointer(node, &pointers);
	if (ts_node_is_null(identifier))
		identifier = ts_node_child(node, 1);

	if (!ts_node_is_null(identifier) && strcmp(ts_node_type(identifier), "array_declarator") == 0)
	{
		pointers++;
		identifier = findNode(identifier, "identifier");
	}

	parseTypeIdentifier(node, pointers, src, file);

	if (!ts_node_is_null(identifier))
	{
		fprintf(file, " ");
		outputNodeValue(identifier, src, file);
	}
	else
	{
		fprintf(file, " _%d", idx);
	}
}

static void parseParameterList(TSNode parameters, const char* src, FILE* file)
{
	int numParams = 0;
	for (int i = 0; i < ts_node_child_count(parameters); i++)
	{
		TSNode parameter = ts_node_child(parameters, i);

		if (strcmp(ts_node_type(parameter), "parameter_declaration") == 0)
		{
			if (numParams > 0)
				fprintf(file, ", ");

			parseParameter(parameter, numParams, src, file);
			numParams++;
		}
	}
}

static void parseFunction(TSNode node, const char* src, FILE* file)
{
	int pointers = 0;
	TSNode declarator = unwrapPointer(node, "function_declarator", &pointers);
	if (ts_node_is_null(declarator))
		declarator = findNode(node, "function_declarator");

	bool dllimport = false;

	TSNode attribute = findNode(node, "attribute_specifier");
	if (!ts_node_is_null(attribute))
	{
		TSNode attributeArgs = findNode(attribute, "argument_list");
		if (!ts_node_is_null(attributeArgs))
		{
			for (int i = 0; i < ts_node_child_count(attributeArgs); i++)
			{
				TSNode attributeArg = ts_node_child(attributeArgs, i);
				if (strcmp(ts_node_type(attributeArg), "identifier") == 0)
				{
					if (nodeIsValue(attributeArg, "dllimport", src))
					{
						dllimport = true;
					}
				}
			}
		}
	}

	if (!ts_node_is_null(declarator))
	{
		TSNode name = findNode(declarator, "identifier");
		TSNode parameters = findNode(declarator, "parameter_list");

		if (dllimport)
			fprintf(file, "dllimport ");
		fprintf(file, "externc func ");
		outputNodeValue(name, src, file);
		fprintf(file, "(");

		if (ts_node_child_count(parameters) == 3 && nodeIsValue(ts_node_child(parameters, 1), "void", src))
		{
		}
		else
		{
			parseParameterList(parameters, src, file);
		}

		fprintf(file, ") ");

		parseTypeIdentifier(node, pointers, src, file, 0, true);
		/*
		if (!(nodeIsValue(returnType, "void", src) && !pointers))
		{
			fprintf(file, " ");
			outputType(returnType, pointers, src, file);
		}
		*/

		fprintf(file, ";\n");
	}
}

static void parseDeclaration(TSNode node, const char* src, FILE* file)
{
	if (hasNode(node, "function_declarator", true))
	{
		parseFunction(node, src, file);
	}
	else
	{
		SnekAssert(false);
	}
}

static void parseField(TSNode field, const char* src, FILE* file, int indentation)
{
	// NOTE this could break if the function returns a pointer since it will be buried inside the pointer declarator node
	TSNode funcType = findNode(field, "function_declarator");

	int pointers = 0;
	TSNode identifier = unwrapPointer(field, &pointers);
	if (ts_node_is_null(identifier))
		identifier = ts_node_child(field, 1);

	for (int i = 0; i < indentation; i++)
		fprintf(file, "\t");
	parseTypeIdentifier(field, pointers, src, file, indentation);

	if (!ts_node_is_null(funcType))
	{
		SnekAssert(identifier.id == funcType.id);

		TSNode fieldName = findNode(funcType, "parenthesized_declarator");
		fieldName = findNode(fieldName, "pointer_declarator");
		fieldName = findNode(fieldName, "field_identifier");
		identifier = fieldName;

		TSNode params = findNode(funcType, "parameter_list");

		fprintf(file, "(");
		parseParameterList(params, src, file);
		fprintf(file, ")");
	}

	if (strcmp(ts_node_type(identifier), "array_declarator") == 0)
	{
		TSNode arrayLength = findNode(identifier, "number_literal");
		if (ts_node_is_null(arrayLength))
			arrayLength = findNode(identifier, "identifier"); // most likely a constant used as array size
		identifier = findNode(identifier, "field_identifier");

		fprintf(file, "[");
		outputNodeValue(arrayLength, src, file);
		fprintf(file, "]");
	}

	fprintf(file, " ");
	outputNodeValue(identifier, src, file);

	// additional names
	if (ts_node_child_count(field) >= 2 && strcmp(ts_node_type(ts_node_child(field, 2)), ",") == 0)
	{
		for (int i = 3; i < ts_node_child_count(field); i++)
		{
			TSNode child = ts_node_child(field, i);
			if (strcmp(ts_node_type(child), "field_identifier") == 0)
			{
				fprintf(file, ", ");
				outputNodeValue(child, src, file);
			}
		}
	}

	fprintf(file, ";\n");
}

static void parseStruct(TSNode node, TSNode name, const char* src, FILE* file)
{
	TSNode fields = findNode(node, "field_declaration_list");

	const char* n = &src[ts_node_start_byte(node)];
	if (strncmp(n, "struct SDL_TextEditingCandidatesEvent", strlen("struct SDL_TextEditingCandidatesEvent")) == 0)
	{
		int a = 5;
	}

	fprintf(file, "struct ");
	outputNodeValue(name, src, file);
	fprintf(file, " {\n");

	for (int i = 0; i < ts_node_child_count(fields); i++)
	{
		TSNode field = ts_node_child(fields, i);
		const char* fieldType = ts_node_type(field);
		if (strcmp(fieldType, "field_declaration") == 0)
		{
			parseField(field, src, file, 1);
		}
	}

	fprintf(file, "}\n");
}

static void parseUnion(TSNode node, TSNode name, const char* src, FILE* file)
{
	TSNode fields = findNode(node, "field_declaration_list");

	const char* n = &src[ts_node_start_byte(node)];
	if (strncmp(n, "struct SDL_TextEditingCandidatesEvent", strlen("struct SDL_TextEditingCandidatesEvent")) == 0)
	{
		int a = 5;
	}

	fprintf(file, "union ");
	outputNodeValue(name, src, file);
	fprintf(file, " {\n");

	for (int i = 0; i < ts_node_child_count(fields); i++)
	{
		TSNode field = ts_node_child(fields, i);
		const char* fieldType = ts_node_type(field);
		if (strcmp(fieldType, "field_declaration") == 0)
		{
			parseField(field, src, file, 1);
		}
	}

	fprintf(file, "}\n");
}

static void parseEnum(TSNode node, TSNode name, const char* src, FILE* file)
{
	TSNode values = findNode(node, "enumerator_list");

	fprintf(file, "enum ");
	outputNodeValue(name, src, file);
	fprintf(file, " {\n");

	for (int i = 0; i < ts_node_child_count(values); i++)
	{
		TSNode field = ts_node_child(values, i);
		const char* fieldType = ts_node_type(field);
		if (strcmp(fieldType, "enumerator") == 0)
		{
			TSNode name = findNode(field, "identifier");
			TSNode value = findNode(field, "number_literal");

			fprintf(file, "\t");
			outputNodeValue(name, src, file);
			if (!ts_node_is_null(value))
			{
				fprintf(file, " = ");
				outputNodeValue(value, src, file);
			}

			fprintf(file, ",\n");
		}
	}

	fprintf(file, "}\n");
}

static void parseType(TSNode node, const char* src, FILE* file)
{
	if (hasNode(node, "typedef"))
	{
		int pointers = 0;
		TSNode name = unwrapPointer(node, &pointers);
		if (ts_node_is_null(name))
			name = ts_node_child(node, 2 + ts_node_child_count(node) - 4);

		if (strcmp(ts_node_type(name), "array_declarator") == 0)
		{
			// array typedef, might be a static assert hack (SDL3 uses this)
			// not supported for now
			return;
		}

		if (hasNode(node, "struct_specifier"))
		{
			TSNode value = findNode(node, "struct_specifier");

			if (hasNode(value, "field_declaration_list"))
			{
				parseStruct(value, name, src, file);
			}
			else if (strcmp(ts_node_type(name), "pointer_declarator") != 0)
			{
				// opaque struct
				fprintf(file, "struct ");
				outputNodeValue(name, src, file);
				fprintf(file, ";\n");
			}
		}
		else if (hasNode(node, "union_specifier"))
		{
			TSNode value = findNode(node, "union_specifier");

			if (hasNode(value, "field_declaration_list"))
			{
				parseUnion(value, name, src, file);
				/*
				fprintf(file, "typedef ");
				outputNodeValue(name, src, file);
				fprintf(file, " : ");
				parseUnion(value, src, file, 0);
				fprintf(file, ";\n");
				*/
			}
			else
			{
				// opaque union, only used for pointers.
				// why does this thing even exist? C???
				fprintf(file, "struct ");
				parseTypeIdentifier(node, 0, src, file);
				fprintf(file, ";\n");
			}
		}
		else if (hasNode(node, "enum_specifier"))
		{
			TSNode value = findNode(node, "enum_specifier");
			parseEnum(value, name, src, file);
		}
		else if (/*hasNode(node, "function_declarator")*/ strcmp(ts_node_type(name), "function_declarator") == 0)
		{
			//TSNode value = findNode(node, "function_declarator");
			//SnekAssert(value.id == name.id);
			TSNode value = name;

			TSNode returnType = ts_node_child(node, 1 + ts_node_child_count(node) - 4);

			TSNode fieldName = findNode(value, "parenthesized_declarator");
			fieldName = findNode(fieldName, "pointer_declarator");
			fieldName = findNode(fieldName, "type_identifier");
			name = fieldName;

			const char* n = &src[ts_node_start_byte(name)];
			const char* t = ts_node_type(name);
			if (strncmp(n, "SDL_malloc_func", strlen("SDL_malloc_func")) == 0)
			{
				int a = 5;
			}

			TSNode params = findNode(value, "parameter_list");

			fprintf(file, "typedef ");
			outputNodeValue(name, src, file);
			fprintf(file, " : ");
			parseTypeIdentifier(node, pointers, src, file);

			fprintf(file, "(");
			parseParameterList(params, src, file);
			fprintf(file, ");\n");
		}
		else
		{
			TSNode value = ts_node_child(node, 1 + ts_node_child_count(node) - 4);

			fprintf(file, "typedef ");
			outputNodeValue(name, src, file);
			fprintf(file, " : ");
			parseTypeIdentifier(node, pointers, src, file);
			fprintf(file, ";\n");

			/*
			TSNode value = ts_node_child(node, 1 + ts_node_child_count(node) - 4);
			uint32_t nameHash = hashNodeValue(name, src);
			typedefs.emplace(nameHash, std::make_pair(value, pointers));
			*/
		}

		/*
		TSNode value = ts_node_child(node, 1);
		TSNode name = ts_node_child(node, 2);

		const char* valueType = ts_node_type(value);
		if (strcmp(valueType, "struct_specifier") == 0)
		{
			if (hasNode(value, "field_declaration_list"))
			{
				parseStruct(value, name, src, file);
			}
			else if (strcmp(ts_node_type(name), "pointer_declarator") != 0)
			{
				// opaque struct
				fprintf(file, "struct ");
				outputNodeValue(name, src, file);
				fprintf(file, ";\n");
			}
		}
		else if (strcmp(valueType, "enum_specifier") == 0)
		{
			parseEnum(value, src, file);
		}
		else if (strcmp(valueType, "union_specifier") == 0)
		{
			if (hasNode(value, "field_declaration_list"))
			{
				fprintf(file, "typedef ");
				outputNodeValue(name, src, file);
				fprintf(file, " : ");
				parseUnion(value, src, file, 0);
				fprintf(file, ";\n");
			}
			else
			{
				// opaque union, only used for pointers.
				// why does this thing even exist? C???
				fprintf(file, "struct ");
				parseTypeIdentifier(node, 0, src, file);
				fprintf(file, ";\n");
			}
		}
		else
		{
			uint32_t nameHash = hashNodeValue(name, src);
			typedefs.emplace(nameHash, value);
		}
		*/

		/*
		int valueStart = ts_node_start_byte(value);
		int valueEnd = ts_node_end_byte(value);
		int valueLength = valueEnd - valueStart;
		const char* valueStr = &src[valueStart];

		if (valueLength > 7 && strncmp(valueStr, "struct", 6) == 0)
		{
			valueStr += 7;
			valueLength -= 7;
			fprintf(file, "struct %.*s;\n", valueLength, valueStr);
		}
		else if (valueLength > 5 && strncmp(valueStr, "enum", 4) == 0)
		{
			valueStr += 5;
			valueLength -= 5;
			fprintf(file, "enum %.*s;\n", valueLength, valueStr);
		}
		else
		{
			uint32_t nameHash = hashNodeValue(name, src);
			typedefs.emplace(nameHash, value);
		}*/

		/*
		TSNode funcType = findNode(field, "function_declarator");

		int pointers = 0;
		TSNode identifier = unwrapPointer(field, &pointers);
		if (ts_node_is_null(identifier))
			identifier = ts_node_child(field, 1);

		for (int i = 0; i < indentation; i++)
			fprintf(file, "\t");
		parseTypeIdentifier(field, pointers, src, file, indentation);

		if (!ts_node_is_null(funcType))
		{
			SnekAssert(identifier.id == funcType.id);

			TSNode fieldName = findNode(funcType, "parenthesized_declarator");
			fieldName = findNode(fieldName, "pointer_declarator");
			fieldName = findNode(fieldName, "field_identifier");
			identifier = fieldName;

			TSNode params = findNode(funcType, "parameter_list");

			fprintf(file, "(");
			parseParameterList(params, src, file);
			fprintf(file, ")");
		}
		*/
	}
}

static bool parse(const char* path, FILE* outFile)
{
	TSParser* parser = ts_parser_new();
	ts_parser_set_language(parser, tree_sitter_c());

	char* src = ReadText(path);
	TSTree* tree = ts_parser_parse_string(
		parser,
		NULL,
		src,
		strlen(src)
	);

	TSNode rootNode = ts_tree_root_node(tree);

	TSNode arrayNode = ts_node_named_child(rootNode, 0);
	TSNode numberNode = ts_node_named_child(arrayNode, 0);

	//assert(strcmp(ts_node_type(root_node), "document") == 0);
	//assert(strcmp(ts_node_type(array_node), "array") == 0);
	//assert(strcmp(ts_node_type(number_node), "number") == 0);

	//assert(ts_node_child_count(root_node) == 1);
	//assert(ts_node_child_count(array_node) == 5);
	//assert(ts_node_named_child_count(array_node) == 2);
	//assert(ts_node_child_count(number_node) == 0);

	for (int i = 0; i < ts_node_child_count(rootNode); i++)
	{
		TSNode node = ts_node_child(rootNode, i);
		const char* type = ts_node_type(node);

		if (strcmp(type, "preproc_call") != 0)
			printNode(node);

		if (strcmp(type, "declaration") == 0)
		{
			parseDeclaration(node, src, outFile);
		}
		else if (strcmp(type, "type_definition") == 0)
		{
			parseType(node, src, outFile);
		}
		else if (strcmp(type, "preproc_call") == 0)
		{

		}
		else if (strcmp(type, "function_definition") == 0)
		{

		}
		else
		{
			const char* n = &src[ts_node_start_byte(node)];
			int a = 5;
		}
	}

	delete src;

	ts_tree_delete(tree);
	ts_parser_delete(parser);

	return true;
}

int main(int argc, char* argv[])
{
	if (argc >= 3)
	{
		const char* path = argv[1];
		const char* out = argv[2];

		char intermediate[256];
		sprintf(intermediate, out);
		char* dot = strrchr(intermediate, '.');
		sprintf(dot, ".i");

		FILE* outFile = fopen(out, "w");

		parseConstants(path, outFile);

		if (preprocess(path, intermediate))
		{
			if (parse(intermediate, outFile))
			{
				fclose(outFile);
				return 0;
			}
		}

		fclose(outFile);
	}
	return 1;
}