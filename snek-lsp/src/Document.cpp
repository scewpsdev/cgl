#include "Document.h"

#include "Platform.h"
#include "utils/Hash.h"
#include "typechecker/TypeSystem.h"

#include <iostream>
#include <sstream>
#include <set>


using json = nlohmann::json;


extern List<Document*> documents;


static int LSPTokenComparator(LSPToken const* a, LSPToken const* b)
{
	return a->offset < b->offset ? -1 : a->offset == b->offset ? 0 : 1;
}

static void textToLines(const std::string& text, List<char*>& lines)
{
	if (text == "")
	{
		lines.add(_strdup(""));
	}
	else
	{
		std::istringstream stream(text);
		std::string line;
		while (std::getline(stream, line))
		{
			lines.add(_strdup(line.c_str()));
		}

		if (text.back() == '\n')
		{
			lines.add(_strdup(""));
		}
	}
}

void Document::init(const std::string& text, TypeSystem* types, GlobalBlockPool* blockPool)
{
	textToLines(text, lines);

	open = false;

	lastChange = GetTimeNS();

	state = DOCUMENT_STATE_UNPARSED;

	initFile(&file, localPath.c_str(), blockPool);
}

void Document::onOpen(std::string& text)
{
	linesMutex.lock();

	for (int i = 0; i < lines.size; i++)
	{
		free(lines[i]);
	}
	lines.clear();

	textToLines(text, lines);

	open = true;

	linesMutex.unlock();

	lastChange = GetTimeNS();
}

void Document::onChange(int startLine, int startCol, int endLine, int endCol, std::string& text)
{
	List<char*> changeLines;

	textToLines(text, changeLines);

	char* prefix = substring(lines[startLine], 0, startCol);
	char* suffix = substring(lines[endLine], endCol);

	changeLines[0] = concatDelete(prefix, changeLines[0]);
	changeLines[changeLines.size - 1] = concatDelete(changeLines[changeLines.size - 1], suffix);

	linesMutex.lock();

	int linesToRemove = endLine - startLine + 1;
	for (int i = 0; i < linesToRemove; i++)
	{
		free(lines[startLine]);
		lines.removeAt(startLine);
	}

	for (int i = 0; i < changeLines.size; i++)
	{
		lines.insert(startLine + i, changeLines[i]);
	}

	linesMutex.unlock();

	FreeList(&changeLines);

	lastChange = GetTimeNS();
}

static void getCoordFromOffset(int offset, const char* src, int* line, int* col)
{
	*line = 0;
	*col = 0;
	for (int i = 0; i < offset; i++)
	{
		if (src[i] == '\n')
		{
			*line += 1;
			*col = 0;
		}
		else
		{
			*col += 1;
		}
	}
}

struct ASTVisitorData
{
	File* file;
	List<LSPToken>* lspTokens;
};

static void getStringRange(StringView str, Parser* parser, int* start, int* end)
{
	*start = (int)(str.ptr - parser->lexer.src);
	*end = *start + str.length;
}

static bool getNodeTokens(Node* node, Scope* scope, ASTVisitorData* data)
{
	if (!node)
		return false;

	if (node->type == NODE_NAMED_TYPE)
	{
		NamedType* namedType = &node->namedType;
		if (Type* type = namedType->inferredType)
		{
			if (type->typeKind == TYPE_STRUCT)
				data->lspTokens->add({ node->start, node->end - node->start, LSP_TOKEN_STRUCT, 0 });
			else if (type->typeKind == TYPE_UNION)
				data->lspTokens->add({ node->start, node->end - node->start, LSP_TOKEN_STRUCT, 0 });
			else if (type->typeKind == TYPE_ENUM)
				data->lspTokens->add({ node->start, node->end - node->start, LSP_TOKEN_ENUM, 0 });
			else if (type->typeKind != TYPE_NULL)
				data->lspTokens->add({ node->start, node->end - node->start, LSP_TOKEN_TYPE, 0 });
		}
	}
	else if (node->type == NODE_IDENTIFIER)
	{
		Identifier* identifier = &node->identifier;

		int start, end;
		getStringRange(identifier->name, &data->file->parser, &start, &end);
		int len = end - start;

		Symbol* symbol = getIdentifierSymbol(identifier);

		if (symbol)
		{
			if (symbol->type == SYMBOL_VARIABLE)
			{
				Node* node = symbol->declaration;
				if (node->type == NODE_VARIABLE_DECLARATION)
				{
					data->lspTokens->add({ start, len, LSP_TOKEN_VARIABLE, 0 });
				}
				else if (node->type == NODE_GLOBAL_VARIABLE)
				{
					int modifiers = LSP_TOKEN_MODIFIER_STATIC;
					if (node->globalVariable.storage & STORAGE_CONSTANT)
						modifiers |= LSP_TOKEN_MODIFIER_READONLY;
					data->lspTokens->add({ start, len, LSP_TOKEN_VARIABLE, modifiers });
				}
				else if (node->type == NODE_PARAMETER)
				{
					data->lspTokens->add({ start, len, LSP_TOKEN_PARAMETER, 0 });
				}
				else if (node->type == NODE_FOR)
				{
					data->lspTokens->add({ start, len, LSP_TOKEN_VARIABLE, 0 });
				}
			}
			else if (symbol->type == SYMBOL_FUNCTION_SET)
			{
				data->lspTokens->add({ start, len, LSP_TOKEN_FUNCTION, 0 });
			}
			else if (symbol->type == SYMBOL_TYPE)
			{
				Node* declaration = symbol->declaration;
				if (declaration->type == NODE_STRUCT)
					data->lspTokens->add({ start, len, LSP_TOKEN_STRUCT, 0 });
				else if (declaration->type == NODE_UNION)
					data->lspTokens->add({ start, len, LSP_TOKEN_STRUCT, 0 });
				else if (declaration->type == NODE_ENUM)
					data->lspTokens->add({ start, len, LSP_TOKEN_ENUM, 0 });
			}
		}
	}
	else if (node->type == NODE_MEMBER_ACCESS)
	{
		MemberAccess* member = &node->memberAccess;

		int start, end;
		getStringRange(member->name, &data->file->parser, &start, &end);

		Type* operandType = member->operand->inferredType;
		if (operandType && operandType->typeKind == TYPE_TYPE)
		{
			SnekAssert(member->operand->type == NODE_IDENTIFIER);
			Identifier* typeName = (Identifier*)member->operand;
			if (Symbol* symbol = getIdentifierSymbol(typeName))
			{
				if (symbol->declaration->type == NODE_ENUM)
					data->lspTokens->add({ start, end - start, LSP_TOKEN_ENUM_VALUE, 0 });
			}
		}
		else
		{
			data->lspTokens->add({ start, end - start, LSP_TOKEN_PROPERTY, 0 });
		}
	}
	else if (node->type == NODE_FOR)
	{
		For* for_ = &node->for_;
		int start, end;
		getStringRange(for_->iteratorName, &data->file->parser, &start, &end);
		data->lspTokens->add({ start, end - start, LSP_TOKEN_VARIABLE, 0 });
	}
	else if (node->type == NODE_STRUCT)
	{
		Struct* struct_ = &node->struct_;
		int start, end;
		getStringRange(struct_->name, &data->file->parser, &start, &end);
		data->lspTokens->add({ start, end - start, LSP_TOKEN_STRUCT, LSP_TOKEN_MODIFIER_DECLARATION });
	}
	else if (node->type == NODE_ENUM)
	{
		Enum* enum_ = &node->enum_;
		int start, end;
		getStringRange(enum_->name, &data->file->parser, &start, &end);
		data->lspTokens->add({ start, end - start, LSP_TOKEN_ENUM, LSP_TOKEN_MODIFIER_DECLARATION });

		for (int i = 0; i < enum_->numValues; i++)
		{
			int start, end;
			getStringRange(enum_->values[i]->name, &data->file->parser, &start, &end);
			data->lspTokens->add({ start, end - start, LSP_TOKEN_ENUM_VALUE, 0 });
		}
	}
	else if (node->type == NODE_TYPEDEF)
	{
		Typedef* typedef_ = &node->typedef_;
		int start, end;
		getStringRange(typedef_->name, &data->file->parser, &start, &end);
		data->lspTokens->add({ start, end - start, LSP_TOKEN_TYPE, LSP_TOKEN_MODIFIER_DECLARATION });
	}
	else if (node->type == NODE_FUNCTION)
	{
		Function* function = &node->function;
		int start, end;
		getStringRange(function->name, &data->file->parser, &start, &end);
		data->lspTokens->add({ start, end - start, LSP_TOKEN_FUNCTION, LSP_TOKEN_MODIFIER_DECLARATION });
	}
	else if (node->type == NODE_IMPORT)
	{
		//
	}
	else if (node->type == NODE_FIELD)
	{
		Field* field = &node->field;
		for (int i = 0; i < field->numDeclarators; i++)
		{
			int start, end;
			getStringRange(field->declarators[i].name, &data->file->parser, &start, &end);
			data->lspTokens->add({ start, end - start, LSP_TOKEN_PROPERTY, 0 });
		}
	}
	else if (node->type == NODE_PARAMETER)
	{
		Parameter* parameter = &node->parameter;
		int start, end;
		getStringRange(parameter->name, &data->file->parser, &start, &end);
		data->lspTokens->add({ start, end - start, LSP_TOKEN_PARAMETER, 0 });
	}
	else if (node->type == NODE_ENUM_VALUE)
	{
		EnumValue* enumValue = &node->enumValue;
		int start, end;
		getStringRange(enumValue->name, &data->file->parser, &start, &end);
		data->lspTokens->add({ start, end - start, LSP_TOKEN_ENUM_VALUE, 0 });
	}
	else if (node->type == NODE_GLOBAL_VARIABLE)
	{
		GlobalVariable* globalVariable = &node->globalVariable;
		for (int i = 0; i < globalVariable->numDeclarators; i++)
		{
			int start, end;
			getStringRange(globalVariable->declarators[i].name, &data->file->parser, &start, &end);
			data->lspTokens->add({ start, end - start, LSP_TOKEN_VARIABLE, LSP_TOKEN_MODIFIER_DECLARATION });
		}
	}

	return true;
}

void Document::getTokens(std::vector<int>& data)
{
	if (state < DOCUMENT_STATE_TYPECHECKED)
		return;

	List<LSPToken> lspTokens;

	astMutex.lock();

	ASTVisitorData visitorData = {};
	visitorData.file = &file;
	visitorData.lspTokens = &lspTokens;
	traverseAST(&file.ast, (ASTVisitor_t)getNodeTokens, &visitorData);

	astMutex.unlock();

	qsort(lspTokens.buffer, lspTokens.size, sizeof(LSPToken), (_CoreCrtNonSecureSearchSortCompareFunction)LSPTokenComparator);

	int lastLine = 0, lastCol = 0;
	for (int i = 0; i < lspTokens.size; i++)
	{
		LSPToken token = lspTokens[i];
		SourceLocation location = getSourceLocation(&file.parser.lexer, token.offset);
		int len = token.length;

		// deltaLine, deltaStart, length, tokenType, tokenModifiers
		data.push_back(location.line - lastLine);
		data.push_back(location.line == lastLine ? location.col - lastCol : location.col);
		data.push_back(len);
		data.push_back(token.type);
		data.push_back(token.modifiers);

		lastLine = location.line;
		lastCol = location.col;
	}

	FreeList(&lspTokens);
}


struct NodeSearchParams
{
	Lexer* lexer;
	int line, col;
	Node* node;
	Scope* scope;
};

static bool searchForNode(Node* node, Scope* scope, void* userPtr)
{
	NodeSearchParams* params = (NodeSearchParams*)userPtr;

	SourceLocation start = getSourceLocation(params->lexer, node->start);
	SourceLocation end = getSourceLocation(params->lexer, node->end);

	if (params->line >= start.line && params->line <= end.line)
	{
		bool matchesStart = params->line == start.line && params->col >= start.col || params->line > start.line;
		bool matchesEnd = params->line == end.line && params->col <= end.col || params->line < end.line;
		if (matchesStart && matchesEnd)
		{
			if (node->type == NODE_FUNCTION)
				scope = node->function.scope;
			else if (node->type == NODE_FOR)
				scope = node->for_.scope;
			else if (node->type == NODE_BLOCK_STATEMENT)
				scope = node->blockStatement.scope;

			params->node = node;
			params->scope = scope;

			return true;
		}
	}

	return false;
}

void Document::getNodeAtPosition(int line, int col, Node** node, Scope** scope)
{
	AST* ast = &file.ast;

	NodeSearchParams params = {};
	params.lexer = &file.parser.lexer;
	params.line = line;
	params.col = col;

	traverseAST(ast, searchForNode, &params);

	*node = params.node;
	*scope = params.scope;
}

static void scanScopeForItems(Scope* scope, nlohmann::json& items)
{
	for (int i = 0; i < scope->symbols.capacity; i++)
	{
		Symbol* symbol = &scope->symbols.slots[i];
		if (symbol->key)
		{
			StringView name = symbol->name;
			std::string nameStr = std::string(name.ptr, name.length);

			CompletionItemType completionItem = COMPLETION_ITEM_TEXT;
			if (symbol->type == SYMBOL_VARIABLE)
			{
				completionItem = COMPLETION_ITEM_VARIABLE;
				Node* declaration = symbol->declaration;
				if (declaration->type == NODE_GLOBAL_VARIABLE)
				{
					GlobalVariable* globalVariable = &declaration->globalVariable;
					if (globalVariable->storage & STORAGE_CONSTANT)
						completionItem = COMPLETION_ITEM_CONSTANT;
				}
			}
			else if (symbol->type == SYMBOL_TYPE)
			{
				Node* declaration = symbol->declaration;
				if (declaration->type == NODE_STRUCT)
				{
					completionItem = COMPLETION_ITEM_STRUCT;
				}
				else if (declaration->type == NODE_UNION)
				{
					completionItem = COMPLETION_ITEM_STRUCT;
				}
				else if (declaration->type == NODE_ENUM)
				{
					completionItem = COMPLETION_ITEM_ENUM;
				}
				else if (declaration->type == NODE_TYPEDEF)
				{
					completionItem = COMPLETION_ITEM_STRUCT;
				}
			}
			else if (symbol->type == SYMBOL_FUNCTION_SET)
			{
				completionItem = COMPLETION_ITEM_FUNCTION;
			}
			else if (symbol->type == SYMBOL_MACRO)
			{
				completionItem = COMPLETION_ITEM_METHOD;
			}

			items.push_back({
				{"label", nameStr },
				{"kind", completionItem}  // keyword
				});
		}
	}
}

void Document::autocomplete(Scope* scope, json& items)
{
	while (scope)
	{
		scanScopeForItems(scope, items);
		scope = scope->parent;
	}

	for (int i = 0; i < file.dependencies.size; i++)
	{
		if (File* dependency = getFileFromHandle(file.dependencies[i]))
		{
			scanScopeForItems(dependency->ast.globalScope, items);
		}
	}
}

void Document::getSymbols(json& items)
{
	AST* ast = &file.ast;

	for (int i = 0; i < ast->numDeclarations; i++)
	{
		Node* declaration = ast->declarations[i];

		SourceLocation start, end;
		getSourceLocation(&file.parser, declaration, &start, &end);

		SourceLocation selectionStart, selectionEnd;

		std::string name;
		int kind = 0;

		if (declaration->type == NODE_STRUCT)
		{
			Struct* struct_ = &declaration->struct_;

			name = std::string(struct_->name.ptr, struct_->name.length);
			kind = SYMBOL_KIND_STRUCT;

			getSourceLocation(&file.parser, struct_->name, &selectionStart, &selectionEnd);
		}
		else if (declaration->type == NODE_UNION)
		{
			Union* union_ = &declaration->union_;

			name = std::string(union_->name.ptr, union_->name.length);
			kind = SYMBOL_KIND_STRUCT;

			getSourceLocation(&file.parser, union_->name, &selectionStart, &selectionEnd);
		}
		else if (declaration->type == NODE_ENUM)
		{
			Enum* enum_ = &declaration->enum_;

			name = std::string(enum_->name.ptr, enum_->name.length);
			kind = SYMBOL_KIND_ENUM;

			getSourceLocation(&file.parser, enum_->name, &selectionStart, &selectionEnd);
		}
		else if (declaration->type == NODE_TYPEDEF)
		{
			Typedef* typedef_ = &declaration->typedef_;

			name = std::string(typedef_->name.ptr, typedef_->name.length);
			kind = SYMBOL_KIND_STRUCT;

			getSourceLocation(&file.parser, typedef_->name, &selectionStart, &selectionEnd);
		}
		else if (declaration->type == NODE_FUNCTION)
		{
			Function* function = &declaration->function;

			name = std::string(function->name.ptr, function->name.length);
			kind = SYMBOL_KIND_FUNCTION;

			getSourceLocation(&file.parser, function->name, &selectionStart, &selectionEnd);
		}
		else if (declaration->type == NODE_GLOBAL_VARIABLE)
		{
			GlobalVariable* variable = &declaration->globalVariable;

			if (variable->numDeclarators)
			{
				name = std::string(variable->declarators[0].name.ptr, variable->declarators[0].name.length);
				kind = variable->storage & STORAGE_CONSTANT ? SYMBOL_KIND_CONSTANT : SYMBOL_KIND_VARIABLE;

				getSourceLocation(&file.parser, variable->declarators[0].name, &selectionStart, &selectionEnd);
			}
		}
		else if (declaration->type == NODE_MACRO)
		{
		}
		else if (declaration->type == NODE_IMPORT)
		{
		}

		if (kind)
		{
			json range = {
				{"start", {
					{"line", start.line},
					{"character", start.col}
				}},
				{"end", {
					{"line", end.line},
					{"character", end.col}
				}}
			};

			json selectionRange = {
				{"start", {
					{"line", selectionStart.line},
					{"character", selectionStart.col}
				}},
				{"end", {
					{"line", selectionEnd.line},
					{"character", selectionEnd.col}
				}}
			};

			items.push_back({
					{"name", name},
					{"kind", kind},
					{"range", range},
					{"selectionRange", selectionRange}
				});
		}
	}
}

static char toLower(char c)
{
	if (c >= 'A' && c <= 'Z')
		return c + ('a' - 'A');
	return c;
}

static bool matchQuery(StringView name, const std::string& query)
{
	if (query.length() > name.length)
		return false;

	for (int i = 0; i <= name.length - query.length(); i++)
	{
		bool match = true;

		for (int j = 0; j < query.length(); j++)
		{
			if (toLower(name[i + j]) != toLower(query[j]))
			{
				match = false;
				break;
			}
		}

		if (match)
			return true;
	}

	return false;
}

void Document::getWorkspaceSymbols(const std::string& query, json& items)
{
	AST* ast = &file.ast;

	for (int i = 0; i < ast->numDeclarations; i++)
	{
		Node* declaration = ast->declarations[i];

		SourceLocation start, end;
		getSourceLocation(&file.parser, declaration, &start, &end);

		SourceLocation selectionStart, selectionEnd;

		std::string name;
		int kind = 0;

		if (declaration->type == NODE_STRUCT)
		{
			Struct* struct_ = &declaration->struct_;

			if (query.length() == 0 && items.size() < 2000 || matchQuery(struct_->name, query))
			{
				name = std::string(struct_->name.ptr, struct_->name.length);
				kind = SYMBOL_KIND_STRUCT;

				getSourceLocation(&file.parser, struct_->name, &selectionStart, &selectionEnd);
			}
		}
		else if (declaration->type == NODE_UNION)
		{
			Union* union_ = &declaration->union_;

			if (query.length() == 0 && items.size() < 2000 || matchQuery(union_->name, query))
			{
				name = std::string(union_->name.ptr, union_->name.length);
				kind = SYMBOL_KIND_STRUCT;

				getSourceLocation(&file.parser, union_->name, &selectionStart, &selectionEnd);
			}
		}
		else if (declaration->type == NODE_ENUM)
		{
			Enum* enum_ = &declaration->enum_;

			if (query.length() && items.size() < 2000 == 0 || matchQuery(enum_->name, query))
			{
				name = std::string(enum_->name.ptr, enum_->name.length);
				kind = SYMBOL_KIND_ENUM;

				getSourceLocation(&file.parser, enum_->name, &selectionStart, &selectionEnd);
			}
		}
		else if (declaration->type == NODE_TYPEDEF)
		{
			Typedef* typedef_ = &declaration->typedef_;

			if (query.length() && items.size() < 2000 == 0 || matchQuery(typedef_->name, query))
			{
				name = std::string(typedef_->name.ptr, typedef_->name.length);
				kind = SYMBOL_KIND_STRUCT;

				getSourceLocation(&file.parser, typedef_->name, &selectionStart, &selectionEnd);
			}
		}
		else if (declaration->type == NODE_FUNCTION)
		{
			Function* function = &declaration->function;

			if (query.length() && items.size() < 2000 == 0 || matchQuery(function->name, query))
			{
				name = std::string(function->name.ptr, function->name.length);
				kind = SYMBOL_KIND_FUNCTION;

				getSourceLocation(&file.parser, function->name, &selectionStart, &selectionEnd);
			}
		}
		else if (declaration->type == NODE_GLOBAL_VARIABLE)
		{
			GlobalVariable* variable = &declaration->globalVariable;

			if (variable->numDeclarators)
			{
				if (query.length() && items.size() < 2000 == 0 || matchQuery(variable->declarators[0].name, query))
				{
					name = std::string(variable->declarators[0].name.ptr, variable->declarators[0].name.length);
					kind = variable->storage & STORAGE_CONSTANT ? SYMBOL_KIND_CONSTANT : SYMBOL_KIND_VARIABLE;

					getSourceLocation(&file.parser, variable->declarators[0].name, &selectionStart, &selectionEnd);
				}
			}
		}
		else if (declaration->type == NODE_MACRO)
		{
		}
		else if (declaration->type == NODE_IMPORT)
		{
		}

		if (kind)
		{
			json location = {
				{"uri", uri},
				{"range", {
					{"start", {
						{"line", start.line},
						{"character", start.col}
					}},
					{"end", {
						{"line", end.line},
						{"character", end.col}
					}}
				}}
			};

			items.push_back({
					{"name", name},
					{"kind", kind},
					{"location", location},
					{"containerName", this->localPath}
				});
		}
	}
}
