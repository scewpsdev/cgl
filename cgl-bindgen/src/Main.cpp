#include <stdio.h>

#include "cgl/Platform.h"
#include "cgl/CGLCompiler.h"
#include "cgl/parser/lexer.h"
#include "cgl/parser/Parser.h"
#include "cgl/semantics/Resolver.h"
#include "cgl/semantics/Variable.h"

#include "tcc/libtcc.h"
#include "tcc/tcc.h"

#include <tree_sitter/api.h>

#include <unordered_map>


static std::unordered_map<uint32_t, TSNode> typedefs;


extern "C" const TSLanguage* tree_sitter_c(void);


static void outputSymbols(TCCState* s1, FILE* file)
{
	for (int i = 0; i < total_idents; i++)
	{
		TokenSym* tok = table_ident[i];

		continue;
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
}

static TSNode unwrapPointer(TSNode node, int* pointers)
{
	TSNode pointer = findNode(node, "pointer_declarator");

	*pointers = 0;
	while (!ts_node_is_null(pointer) && strcmp(ts_node_type(pointer), "pointer_declarator") == 0)
	{
		(*pointers)++;
		pointer = ts_node_child(pointer, 1);
	}

	return pointer;
}

static bool parseTypeIdentifier(TSNode node, int pointers, const char* src, FILE* file);

static void parseUnion(TSNode node, const char* src, FILE* file)
{
	TSNode fields = findNode(node, "field_declaration_list");

	fprintf(file, "union {\n");

	for (int i = 0; i < ts_node_child_count(fields); i++)
	{
		TSNode field = ts_node_child(fields, i);
		const char* fieldType = ts_node_type(field);
		if (strcmp(fieldType, "field_declaration") == 0)
		{
			//TSNode type = getType(field);
			int pointers = 0;
			TSNode identifier = unwrapPointer(field, &pointers);
			if (ts_node_is_null(identifier))
				identifier = ts_node_child(field, 1);

			fprintf(file, "\t");
			parseTypeIdentifier(field, pointers, src, file);
			fprintf(file, " ");
			outputNodeValue(identifier, src, file);
			fprintf(file, ";\n");
		}
	}

	fprintf(file, "}\n");
}

static bool parseTypeIdentifier(TSNode node, int pointers, const char* src, FILE* file)
{
	TSNode type = findNode(node, "primitive_type");

	TSNode sizedType = findNode(node, "sized_type_specifier");

	if (ts_node_is_null(type))
		type = findNode(node, "type_identifier");
	if (ts_node_is_null(type))
	{
		TSNode structSpecifier = findNode(node, "struct_specifier");
		if (!ts_node_is_null(structSpecifier))
		{
			if (parseTypeIdentifier(structSpecifier, pointers, src, file))
				return true;
		}
	}
	if (ts_node_is_null(type))
	{
		TSNode unionSpecifier = findNode(node, "union_specifier");
		if (!ts_node_is_null(unionSpecifier))
		{
			parseUnion(unionSpecifier, src, file);
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
		else if (typedefs.find(nameHash) != typedefs.end())
		{
			TSNode value = typedefs[nameHash];

			int nameStart = ts_node_start_byte(value);
			int nameEnd = ts_node_end_byte(value);
			int nameLength = nameEnd - nameStart;
			const char* name = &src[nameStart];

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

		return true;
	}

	return false;
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

static void parseFunction(TSNode node, const char* src, FILE* file)
{
	int pointers = 0;
	TSNode declarator = unwrapPointer(node, &pointers);

	if (!ts_node_is_null(declarator))
	{
		TSNode name = findNode(declarator, "identifier");
		TSNode parameters = findNode(declarator, "parameter_list");

		fprintf(file, "externc func ");
		outputNodeValue(name, src, file);
		fprintf(file, "(");

		if (ts_node_child_count(parameters) == 3 && nodeIsValue(ts_node_child(parameters, 1), "void", src))
		{
		}
		else
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

		fprintf(file, ")");

		parseTypeIdentifier(node, pointers, src, file);
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

static void parseStruct(TSNode node, TSNode name, const char* src, FILE* file)
{
	TSNode fields = findNode(node, "field_declaration_list");

	fprintf(file, "struct ");
	outputNodeValue(name, src, file);
	fprintf(file, " {\n");

	for (int i = 0; i < ts_node_child_count(fields); i++)
	{
		TSNode field = ts_node_child(fields, i);
		const char* fieldType = ts_node_type(field);
		if (strcmp(fieldType, "field_declaration") == 0)
		{
			//TSNode type = getType(field);
			int pointers = 0;
			TSNode identifier = unwrapPointer(field, &pointers);
			if (ts_node_is_null(identifier))
				identifier = ts_node_child(field, 1);

			fprintf(file, "\t");
			parseTypeIdentifier(field, pointers, src, file);
			fprintf(file, " ");
			outputNodeValue(identifier, src, file);
			fprintf(file, ";\n");
		}
	}

	fprintf(file, "}\n");
}

static void parseEnum(TSNode node, const char* src, FILE* file)
{

}

static void parseType(TSNode node, const char* src, FILE* file)
{
	if (hasNode(node, "typedef"))
	{
		if (ts_node_child_count(node) == 4)
		{
			TSNode value = ts_node_child(node, 1);
			TSNode name = ts_node_child(node, 2);

			const char* valueType = ts_node_type(value);
			if (strcmp(valueType, "struct_specifier") == 0 && hasNode(value, "field_declaration_list"))
			{
				parseStruct(value, name, src, file);

				/*
				{
					// opaque struct
					fprintf(file, "struct ");
					outputNodeValue(name, src, file);
					fprintf(file, ";\n");
				}
				*/
			}
			else if (strcmp(valueType, "enum_specifier") == 0)
			{
				parseEnum(value, src, file);
			}
			else
			{
				uint32_t nameHash = hashNodeValue(name, src);
				typedefs.emplace(nameHash, value);
			}

			/*int valueStart = ts_node_start_byte(value);
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
		}
	}
}

static bool parse(const char* path, const char* out)
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

	FILE* outFile = fopen(out, "w");

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

		//printNode(node);
		if (strcmp(type, "declaration") == 0)
		{
			parseDeclaration(node, src, outFile);
		}
		else if (strcmp(type, "type_definition") == 0)
		{
			parseType(node, src, outFile);
		}
	}

	fclose(outFile);

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

		if (preprocess(path, intermediate))
		{
			if (parse(intermediate, out))
			{
				return 0;
			}
		}
	}
	return 1;
}