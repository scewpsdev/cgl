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

// isalpha crashes when passing Ö
static bool isAlpha(char c)
{
	return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
}

static bool isDigit(char c)
{
	return c >= '0' && c <= '9';
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

void Document::init(std::string uri, const char* localPath, const std::string& text, TypeSystem* types, GlobalBlockPool* blockPool)
{
	this->uri = uri;
	this->localPath = _strdup(localPath);
	this->fileHandle = getFileHandle(localPath);

	textToLines(text, lines);

	open = false;

	lastChange = GetTimeNS();

	state = DOCUMENT_STATE_UNPARSED;
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

	char* prefix = substring(lines[startLine], 0, locationToLineOffset(lines[startLine], startCol));
	char* suffix = substring(lines[endLine], locationToLineOffset(lines[endLine], endCol));

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

	// preliminary line offset update in case we need this before the parser runs
	int currentOffset = 0;
	for (int i = 0; i < lines.size; i++)
	{
		file->lineOffsets.add(currentOffset);
		currentOffset += (int)strlen(lines[i]) + 1;
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

static void getStringRange(StringView str, File* file, int* start, int* end)
{
	*start = (int)(str.ptr - file->src);
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
		getStringRange(identifier->name, data->file, &start, &end);
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
		getStringRange(member->name, data->file, &start, &end);

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
		getStringRange(for_->iteratorName, data->file, &start, &end);
		data->lspTokens->add({ start, end - start, LSP_TOKEN_VARIABLE, 0 });
	}
	else if (node->type == NODE_STRUCT)
	{
		Struct* struct_ = &node->struct_;
		int start, end;
		getStringRange(struct_->name, data->file, &start, &end);
		data->lspTokens->add({ start, end - start, LSP_TOKEN_STRUCT, LSP_TOKEN_MODIFIER_DECLARATION });
	}
	else if (node->type == NODE_ENUM)
	{
		Enum* enum_ = &node->enum_;
		int start, end;
		getStringRange(enum_->name, data->file, &start, &end);
		data->lspTokens->add({ start, end - start, LSP_TOKEN_ENUM, LSP_TOKEN_MODIFIER_DECLARATION });

		for (int i = 0; i < enum_->numValues; i++)
		{
			int start, end;
			getStringRange(enum_->values[i]->name, data->file, &start, &end);
			data->lspTokens->add({ start, end - start, LSP_TOKEN_ENUM_VALUE, 0 });
		}
	}
	else if (node->type == NODE_TYPEDEF)
	{
		Typedef* typedef_ = &node->typedef_;
		int start, end;
		getStringRange(typedef_->name, data->file, &start, &end);
		data->lspTokens->add({ start, end - start, LSP_TOKEN_TYPE, LSP_TOKEN_MODIFIER_DECLARATION });
	}
	else if (node->type == NODE_FUNCTION)
	{
		Function* function = &node->function;
		int start, end;
		getStringRange(function->name, data->file, &start, &end);
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
			getStringRange(field->declarators[i].name, data->file, &start, &end);
			data->lspTokens->add({ start, end - start, LSP_TOKEN_PROPERTY, 0 });
		}
	}
	else if (node->type == NODE_PARAMETER)
	{
		Parameter* parameter = &node->parameter;
		int start, end;
		getStringRange(parameter->name, data->file, &start, &end);
		data->lspTokens->add({ start, end - start, LSP_TOKEN_PARAMETER, 0 });
	}
	else if (node->type == NODE_ENUM_VALUE)
	{
		EnumValue* enumValue = &node->enumValue;
		int start, end;
		getStringRange(enumValue->name, data->file, &start, &end);
		data->lspTokens->add({ start, end - start, LSP_TOKEN_ENUM_VALUE, 0 });
	}
	else if (node->type == NODE_GLOBAL_VARIABLE)
	{
		GlobalVariable* globalVariable = &node->globalVariable;
		for (int i = 0; i < globalVariable->numDeclarators; i++)
		{
			int start, end;
			getStringRange(globalVariable->declarators[i].name, data->file, &start, &end);
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

	ASTVisitorData visitorData = {};
	visitorData.file = file;
	visitorData.lspTokens = &lspTokens;
	traverseAST(&file->ast, (ASTVisitor_t)getNodeTokens, &visitorData);

	qsort(lspTokens.buffer, lspTokens.size, sizeof(LSPToken), (_CoreCrtNonSecureSearchSortCompareFunction)LSPTokenComparator);

	int lastLine = 0, lastCol = 0;
	for (int i = 0; i < lspTokens.size; i++)
	{
		LSPToken token = lspTokens[i];
		SourceLocation location = getSourceLocation(file, token.offset);
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
	File* file;
	int line, col;
	Node* node;
	Scope* scope;
};

static bool searchForNode(Node* node, Scope* scope, void* userPtr)
{
	NodeSearchParams* params = (NodeSearchParams*)userPtr;

	SnekAssert(node->end >= node->start);
	SourceLocation start = getSourceLocation(params->file, node->start);
	SourceLocation end = getSourceLocation(params->file, node->end);

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
	NodeSearchParams params = {};
	params.file = file;
	params.line = line;
	params.col = col;

	traverseAST(&file->ast, searchForNode, &params);

	*node = params.node;
	*scope = params.scope;
}

static bool isInRangeOfString(File* file, int line, int col, StringView str)
{
	SourceLocation start = getSourceLocation(file, (int)(str.ptr - file->src));
	SourceLocation end = getSourceLocation(file, (int)(str.ptr - file->src) + str.length);

	if (line >= start.line && line <= end.line)
	{
		bool matchesStart = line == start.line && col >= start.col || line > start.line;
		bool matchesEnd = line == end.line && col <= end.col || line < end.line;
		return matchesStart && matchesEnd;
	}

	return false;
}

static bool getDeclarationNameIdentifier(File* file, Node* node, int line, int col, StringView* identifier)
{
	if (node->type == NODE_STRUCT)
	{
		Struct* struct_ = &node->struct_;
		*identifier = struct_->name;
		return true;
	}
	else if (node->type == NODE_UNION)
	{
		Union* union_ = &node->union_;
		*identifier = union_->name;
		return true;
	}
	else if (node->type == NODE_ENUM)
	{
		Enum* enum_ = &node->enum_;
		*identifier = enum_->name;
		return true;
	}
	else if (node->type == NODE_TYPEDEF)
	{
		Typedef* typedef_ = &node->typedef_;
		*identifier = typedef_->name;
		return true;
	}
	else if (node->type == NODE_FUNCTION)
	{
		Function* function = &node->function;
		*identifier = function->name;
		return true;
	}
	else if (node->type == NODE_GLOBAL_VARIABLE)
	{
		GlobalVariable* variable = &node->globalVariable;
		for (int i = 0; i < variable->numDeclarators; i++)
		{
			VariableDeclarator* declarator = &variable->declarators[i];
			if (isInRangeOfString(file, line, col, declarator->name))
			{
				*identifier = declarator->name;
				return true;
			}
		}
	}
	else if (node->type == NODE_MACRO)
	{
		Macro* macro = &node->macro;
		*identifier = macro->name;
		return true;
	}
	return false;
}

Symbol* Document::getNodeSymbol(Node* node, int line, int col, int* overloadIdx)
{
	if (node)
	{
		if (node->type == NODE_IDENTIFIER)
		{
			Identifier* identifier = &node->identifier;
			*overloadIdx = identifier->functionOverloadID;
			return getIdentifierSymbol(identifier);
		}
		else if (node->type == NODE_MEMBER_ACCESS)
		{
			MemberAccess* member = &node->memberAccess;
			*overloadIdx = member->functionOverloadID;
			return getMemberAccessSymbol(member);
		}
		else if (node->type == NODE_NAMED_TYPE)
		{
			NamedType* namedType = &node->namedType;
			return getNamedTypeSymbol(namedType);
		}
		else
		{
			StringView declarationName;
			if (getDeclarationNameIdentifier(file, node, line, col, &declarationName))
			{
				if (Symbol* symbol = lookupSymbol(&file->ast.globalScope->symbols, declarationName))
				{
					if (symbol->type == SYMBOL_FUNCTION_SET)
					{
						SnekAssert(node->type == NODE_FUNCTION);
						for (int i = 0; symbol->functionSet.count; i++)
						{
							if (symbol->functionSet.overloads[i].declaration == &node->function)
							{
								*overloadIdx = i;
								break;
							}
						}
						SnekAssert((*overloadIdx) != -1);
					}
					return symbol;
				}
			}
		}
	}

	return nullptr;
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
				{"kind", completionItem},
				{"data", {
					{"symbol_id", std::to_string(symbol->key)},
					{"file_id", std::to_string((uint64_t)symbol->file)}
				}}
				});
		}
	}
}

static void scanScopeForMemberFunction(Scope* scope, Type* operandType, nlohmann::json& items)
{
	for (int i = 0; i < scope->symbols.capacity; i++)
	{
		Symbol* symbol = &scope->symbols.slots[i];
		if (symbol->key)
		{
			if (symbol->type == SYMBOL_FUNCTION_SET)
			{
				for (int j = 0; j < symbol->functionSet.count; j++)
				{
					Function* overload = symbol->functionSet.overloads[j].declaration;

					if (overload->numParams >= 1)
					{
						Type* paramType = overload->params[0]->paramType->inferredType;

						if (compareTypes(paramType, operandType) || paramType->typeKind == TYPE_POINTER && compareTypes(paramType->pointer.elementType, operandType))
						{
							StringView name = symbol->name;
							std::string nameStr = std::string(name.ptr, name.length);

							CompletionItemType completionItem = COMPLETION_ITEM_FUNCTION;

							items.push_back({
								{"label", nameStr },
								{"kind", completionItem},
								{"data", {
									{"symbol_id", std::to_string(symbol->key)},
									{"file_id", std::to_string((uint64_t)symbol->file)}
								}}
								});
						}
					}
				}
			}
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

	for (int i = 0; i < file->dependencies.size; i++)
	{
		if (File* dependency = getFileFromHandleLSP(file->dependencies[i]))
		{
			scanScopeForItems(dependency->ast.globalScope, items);
		}
	}
}

void Document::autocomplete(Node* node, char triggerCharacter, json& items)
{
	if (triggerCharacter == '.')
	{
		if (node->type > NODE_EXPRESSION_START && node->type < NODE_EXPRESSION_END)
		{
			Expression* expression = (Expression*)node;
			if (Type* operandType = expression->inferredType)
			{
				if (operandType->typeKind == TYPE_POINTER)
					operandType = operandType->pointer.elementType;

				if (operandType->typeKind == TYPE_STRUCT)
				{
					Struct* declaration = operandType->struct_.declaration;
					Symbol* symbol = declaration->symbol;

					int fieldID = 0;
					for (int i = 0; i < declaration->numFields; i++)
					{
						Field* field = declaration->fields[i];
						for (int j = 0; j < field->numDeclarators; j++)
						{
							items.push_back({
								{"label", std::string(field->declarators[j].name.ptr, field->declarators[j].name.length) },
								{"kind", COMPLETION_ITEM_FIELD},
								{"data", {
									{"type_id", std::to_string((uint64_t)operandType)},
									{"field_id", fieldID }
								}}
								});

							fieldID++;
						}
					}
				}
				else if (operandType->typeKind == TYPE_UNION)
				{
					Union* declaration = operandType->union_.declaration;
					Symbol* symbol = declaration->symbol;

					int fieldID = 0;
					for (int i = 0; i < declaration->numFields; i++)
					{
						Field* field = declaration->fields[i];
						for (int j = 0; j < field->numDeclarators; j++)
						{
							items.push_back({
								{"label", std::string(field->declarators[j].name.ptr, field->declarators[j].name.length) },
								{"kind", COMPLETION_ITEM_FIELD},
								{"data", {
									{"type_id", std::to_string((uint64_t)operandType)},
									{"field_id", fieldID }
								}}
								});

							fieldID++;
						}
					}
				}
				else if (operandType->typeKind == TYPE_STRING)
				{
					items.push_back({
						{"label", "data"},
						{"kind", COMPLETION_ITEM_FIELD},
						{"data", {
							{"type_id", std::to_string((uint64_t)operandType)},
							{"field_id", 0 }
						}}
						});

					items.push_back({
						{"label", "length"},
						{"kind", COMPLETION_ITEM_FIELD},
						{"data", {
							{"type_id", std::to_string((uint64_t)operandType)},
							{"field_id", 1 }
						}}
						});
				}
				else if (operandType->typeKind == TYPE_ARRAY)
				{
					items.push_back({
						{"label", "data"},
						{"kind", COMPLETION_ITEM_FIELD},
						{"data", {
							{"type_id", std::to_string((uint64_t)operandType)},
							{"field_id", 0 }
						}}
						});

					items.push_back({
						{"label", "length"},
						{"kind", COMPLETION_ITEM_FIELD},
						{"data", {
							{"type_id", std::to_string((uint64_t)operandType)},
							{"field_id", 1 }
						}}
						});
				}
				else if (operandType->typeKind == TYPE_ANY)
				{
					items.push_back({
						{"label", "value"},
						{"kind", COMPLETION_ITEM_FIELD},
						{"data", {
							{"type_id", std::to_string((uint64_t)operandType)},
							{"field_id", 0 }
						}}
						});

					items.push_back({
						{"label", "type"},
						{"kind", COMPLETION_ITEM_FIELD},
						{"data", {
							{"type_id", std::to_string((uint64_t)operandType)},
							{"field_id", 1 }
						}}
						});
				}
				else if (operandType->typeKind == TYPE_TYPE)
				{
					SnekAssert(expression->type == NODE_IDENTIFIER);

					Identifier* typeName = (Identifier*)expression;
					if (Symbol* symbol = getIdentifierSymbol(typeName))
					{
						if (symbol->declaration->type == NODE_ENUM)
						{
							Enum* enum_ = &symbol->declaration->enum_;
							for (int i = 0; i < enum_->numValues; i++)
							{
								EnumValue* enumValue = enum_->values[i];

								items.push_back({
									{"label", std::string(enumValue->name.ptr, enumValue->name.length)},
									{"kind", COMPLETION_ITEM_ENUM_MEMBER},
									{"data", {
										{"type_id", std::to_string((uint64_t)operandType)},
										{"field_id", i }
									}}
									});
							}
						}
					}
				}

				scanScopeForMemberFunction(file->ast.globalScope, operandType, items);

				for (int i = 0; i < file->dependencies.size; i++)
				{
					if (File* dependency = getFileFromHandleLSP(file->dependencies[i]))
					{
						scanScopeForMemberFunction(dependency->ast.globalScope, operandType, items);
					}
				}
			}
		}
	}
}

void Document::getSymbols(json& items)
{
	AST* ast = &file->ast;

	for (int i = 0; i < ast->numDeclarations; i++)
	{
		Node* declaration = ast->declarations[i];

		SourceLocation start, end;
		getSourceLocation(file, declaration, &start, &end);

		SourceLocation selectionStart, selectionEnd;

		std::string name;
		int kind = 0;

		if (declaration->type == NODE_STRUCT)
		{
			Struct* struct_ = &declaration->struct_;

			name = std::string(struct_->name.ptr, struct_->name.length);
			kind = SYMBOL_KIND_STRUCT;

			getSourceLocation(file, struct_->name, &selectionStart, &selectionEnd);
		}
		else if (declaration->type == NODE_UNION)
		{
			Union* union_ = &declaration->union_;

			name = std::string(union_->name.ptr, union_->name.length);
			kind = SYMBOL_KIND_STRUCT;

			getSourceLocation(file, union_->name, &selectionStart, &selectionEnd);
		}
		else if (declaration->type == NODE_ENUM)
		{
			Enum* enum_ = &declaration->enum_;

			name = std::string(enum_->name.ptr, enum_->name.length);
			kind = SYMBOL_KIND_ENUM;

			getSourceLocation(file, enum_->name, &selectionStart, &selectionEnd);
		}
		else if (declaration->type == NODE_TYPEDEF)
		{
			Typedef* typedef_ = &declaration->typedef_;

			name = std::string(typedef_->name.ptr, typedef_->name.length);
			kind = SYMBOL_KIND_STRUCT;

			getSourceLocation(file, typedef_->name, &selectionStart, &selectionEnd);
		}
		else if (declaration->type == NODE_FUNCTION)
		{
			Function* function = &declaration->function;

			name = std::string(function->name.ptr, function->name.length);
			kind = SYMBOL_KIND_FUNCTION;

			getSourceLocation(file, function->name, &selectionStart, &selectionEnd);
		}
		else if (declaration->type == NODE_GLOBAL_VARIABLE)
		{
			GlobalVariable* variable = &declaration->globalVariable;

			if (variable->numDeclarators)
			{
				name = std::string(variable->declarators[0].name.ptr, variable->declarators[0].name.length);
				kind = variable->storage & STORAGE_CONSTANT ? SYMBOL_KIND_CONSTANT : SYMBOL_KIND_VARIABLE;

				getSourceLocation(file, variable->declarators[0].name, &selectionStart, &selectionEnd);
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
	AST* ast = &file->ast;

	for (int i = 0; i < ast->numDeclarations; i++)
	{
		Node* declaration = ast->declarations[i];

		SourceLocation start, end;
		getSourceLocation(file, declaration, &start, &end);

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

				getSourceLocation(file, struct_->name, &selectionStart, &selectionEnd);
			}
		}
		else if (declaration->type == NODE_UNION)
		{
			Union* union_ = &declaration->union_;

			if (query.length() == 0 && items.size() < 2000 || matchQuery(union_->name, query))
			{
				name = std::string(union_->name.ptr, union_->name.length);
				kind = SYMBOL_KIND_STRUCT;

				getSourceLocation(file, union_->name, &selectionStart, &selectionEnd);
			}
		}
		else if (declaration->type == NODE_ENUM)
		{
			Enum* enum_ = &declaration->enum_;

			if (query.length() && items.size() < 2000 == 0 || matchQuery(enum_->name, query))
			{
				name = std::string(enum_->name.ptr, enum_->name.length);
				kind = SYMBOL_KIND_ENUM;

				getSourceLocation(file, enum_->name, &selectionStart, &selectionEnd);
			}
		}
		else if (declaration->type == NODE_TYPEDEF)
		{
			Typedef* typedef_ = &declaration->typedef_;

			if (query.length() && items.size() < 2000 == 0 || matchQuery(typedef_->name, query))
			{
				name = std::string(typedef_->name.ptr, typedef_->name.length);
				kind = SYMBOL_KIND_STRUCT;

				getSourceLocation(file, typedef_->name, &selectionStart, &selectionEnd);
			}
		}
		else if (declaration->type == NODE_FUNCTION)
		{
			Function* function = &declaration->function;

			if (query.length() && items.size() < 2000 == 0 || matchQuery(function->name, query))
			{
				name = std::string(function->name.ptr, function->name.length);
				kind = SYMBOL_KIND_FUNCTION;

				getSourceLocation(file, function->name, &selectionStart, &selectionEnd);
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

					getSourceLocation(file, variable->declarators[0].name, &selectionStart, &selectionEnd);
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

static Symbol* resolveSymbol(File* file, Scope* currentScope, StringView identifier)
{
	Scope* scope = currentScope;
	while (scope)
	{
		if (Symbol* symbol = lookupSymbol(&scope->symbols, identifier))
		{
			return symbol;
		}
		scope = scope->parent;
	}

	for (int i = 0; i < file->dependencies.size; i++)
	{
		FileHandle dependency = file->dependencies[i];
		if (File* file = getFileFromHandleLSP(dependency))
		{
			if (Symbol* symbol = lookupSymbol(&file->ast.globalScope->symbols, identifier))
			{
				return symbol;
			}
		}
	}

	return nullptr;
}

bool Document::getFunctionSignature(int line, int col, json& signatures, int& activeSignature, int& activeParameter)
{
	const char* lineStr = lines[line];

	int functionNameStart = -1, functionNameEnd = -1;

	int parenLevel = 0;
	int commas = 0;
	for (int i = col - 1; i >= 0; i--)
	{
		char c = lineStr[i];
		if (functionNameEnd != -1)
		{
			bool identifier = isAlpha(c) || isDigit(c) || c == '_';
			if (!identifier)
			{
				functionNameStart = i + 1;
				break;
			}
		}
		else if (c == ',')
			commas++;
		else if (c == '(' && parenLevel == 0 && i - 1 >= 0)
			functionNameEnd = i;
		else if (c == ')')
		{
			if (i == col - 1)
				return false;
			else
				parenLevel++;
		}
	}

	activeParameter = commas;

	if (functionNameStart != -1 && functionNameEnd != -1)
	{
		SourceLocation functionNameLocation = {};
		functionNameLocation.line = line;
		functionNameLocation.col = functionNameStart;

		functionNameStart += file->lineOffsets[line];
		functionNameEnd += file->lineOffsets[line];

		StringView functionName = getRangedString(functionNameStart, functionNameEnd, file);

		Node* node;
		Scope* scope;
		getNodeAtPosition(functionNameLocation.line, functionNameLocation.col, &node, &scope);

		Symbol* symbol = nullptr;
		if (node && node->type == NODE_IDENTIFIER)
		{
			symbol = getIdentifierSymbol(&node->identifier);
			activeSignature = node->identifier.functionOverloadID;
		}

		if (!symbol)
		{
			symbol = resolveSymbol(file, scope, functionName);
			activeSignature = 0;
		}

		if (symbol)
		{
			if (symbol->type == SYMBOL_FUNCTION_SET)
			{
				for (int i = 0; i < symbol->functionSet.count; i++)
				{
					FunctionOverload* overload = &symbol->functionSet.overloads[i];
					Function* function = overload->declaration;

					std::stringstream functionLabel;
					functionLabel << std::string(function->name.ptr, function->name.length) << '(';

					json parameters = json::array();

					for (int j = 0; j < function->numParams; j++)
					{
						Type* paramType = function->params[j]->paramType->inferredType;
						std::string paramName = std::string(function->params[j]->name.ptr, function->params[j]->name.length);

						std::string paramLabel = std::string(paramType->name.ptr, paramType->name.length) + ' ' + paramName;

						parameters.push_back({ { "label", paramLabel } });

						functionLabel << paramLabel;
						if (j < function->numParams - 1)
							functionLabel << ", ";
					}

					functionLabel << ')';

					if (function->returnType)
					{
						Type* returnType = function->returnType->inferredType;
						functionLabel << ' ' << std::string(returnType->name.ptr, returnType->name.length);
					}

					signatures.push_back({
						{"label", functionLabel.str()},
						{"parameters", parameters},
						});
				}

				return true;
			}
		}
	}

	return false;
}

static Document* getDocument(FileHandle file)
{
	for (int i = 0; i < documents.size; i++)
	{
		if (documents[i]->file->handle == file)
			return documents[i];
	}
	return nullptr;
}

bool Document::getDefinitionLocation(int line, int col, json& location)
{
	Node* node = nullptr;
	Scope* scope = nullptr;
	getNodeAtPosition(line, col, &node, &scope);

	Symbol* symbol = nullptr;
	int overloadIdx = -1;
	if (node)
		symbol = getNodeSymbol(node, line, col, &overloadIdx);

	if (symbol)
	{
		Document* symbolDocument = getDocument(symbol->file);
		SourceLocation start = {}, end = {};

		if (symbol->type == SYMBOL_VARIABLE || symbol->type == SYMBOL_TYPE)
		{
			getSourceLocation(symbolDocument->file, symbol->declaration, &start, &end);
		}
		else if (symbol->type == SYMBOL_FUNCTION_SET)
		{
			SnekAssert(overloadIdx != -1);
			FunctionOverload* functionOverload = &symbol->functionSet.overloads[overloadIdx];
			getSourceLocation(symbolDocument->file, (Node*)functionOverload->declaration, &start, &end);
		}

		if (start.filename)
		{
			location = {
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
			};

			return true;
		}
	}

	return false;
}

bool Document::getHoverInfo(int line, int col, json& result, SourceLocation& start, SourceLocation& end)
{
	Node* node = nullptr;
	Scope* scope = nullptr;
	getNodeAtPosition(line, col, &node, &scope);

	Symbol* symbol = nullptr;
	int overloadIdx = -1;
	if (node)
		symbol = getNodeSymbol(node, line, col, &overloadIdx);

	if (symbol)
	{
		std::string detail;
		if (getSymbolInfoMarkdown(symbol, result, detail))
		{
			getSourceLocation(file, node, &start, &end);
			return true;
		}
	}
	else if (node && node->type == NODE_MEMBER_ACCESS)
	{
		MemberAccess* member = &node->memberAccess;
		if (isInRangeOfString(file, line, col, member->name))
		{
			Type* operandType = member->operand->inferredType;
			int fieldID = getFieldIndex(operandType, member->name, member->operand);
			std::string detail;
			if (getFieldInfoMarkdown(operandType, fieldID, result, detail))
			{
				getSourceLocation(file, member->name, &start, &end);
				return true;
			}
		}
	}

	return false;
}

struct ScanReferencesParams
{
	Document* document;
	Symbol* symbol;
	json* locations;
};

static bool scanReferences(Node* node, Scope* scope, void* userPtr)
{
	ScanReferencesParams* params = (ScanReferencesParams*)userPtr;

	SourceLocation start, end;
	getSourceLocation(params->document->file, node, &start, &end);

	if (node->type == NODE_IDENTIFIER)
	{
		Identifier* identifier = &node->identifier;
		if (getIdentifierSymbol(identifier) == params->symbol)
		{
			params->locations->push_back({
				{"uri", params->document->uri},
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
	}
	else if (node->type == NODE_MEMBER_ACCESS)
	{
		MemberAccess* member = &node->memberAccess;
		if (getMemberAccessSymbol(member) == params->symbol)
		{
			params->locations->push_back({
				{"uri", params->document->uri},
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
	}
	else if (node->type == NODE_NAMED_TYPE)
	{
		NamedType* namedType = &node->namedType;
		if (getNamedTypeSymbol(namedType) == params->symbol)
		{
			params->locations->push_back({
				{"uri", params->document->uri},
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
	}

	return true;
}

void Document::findAllReferences(Symbol* symbol, json& locations)
{
	ScanReferencesParams params = {};
	params.document = this;
	params.symbol = symbol;
	params.locations = &locations;

	traverseAST(&file->ast, scanReferences, &params);
}

struct ScanReferencesRenameParams
{
	Document* document;
	Symbol* symbol;
	std::string newText;
	json* locations;
};

static bool scanReferencesForRename(Node* node, Scope* scope, void* userPtr)
{
	ScanReferencesRenameParams* params = (ScanReferencesRenameParams*)userPtr;

	if (node->type == NODE_IDENTIFIER)
	{
		Identifier* identifier = &node->identifier;
		if (getIdentifierSymbol(identifier) == params->symbol)
		{
			SourceLocation start, end;
			getSourceLocation(params->document->file, node, &start, &end);

			params->locations->push_back({
				{"newText", params->newText},
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
	}
	else if (node->type == NODE_MEMBER_ACCESS)
	{
		MemberAccess* member = &node->memberAccess;
		if (getMemberAccessSymbol(member) == params->symbol)
		{
			SourceLocation start, end;
			getSourceLocation(params->document->file, member->name, &start, &end);

			params->locations->push_back({
				{"newText", params->newText},
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
	}
	else if (node->type == NODE_NAMED_TYPE)
	{
		NamedType* namedType = &node->namedType;
		if (getNamedTypeSymbol(namedType) == params->symbol)
		{
			SourceLocation start, end;
			getSourceLocation(params->document->file, node, &start, &end);

			params->locations->push_back({
				{"newText", params->newText},
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
	}

	return true;
}

void Document::rename(Symbol* symbol, int overloadIdx, std::string newText, int line, int col, json& changes)
{
	ScanReferencesRenameParams params = {};
	params.document = this;
	params.newText = newText;
	params.symbol = symbol;
	params.locations = &changes;

	traverseAST(&file->ast, scanReferencesForRename, &params);

	Document* symbolDocument = getDocument(symbol->file);
	Node* declaration = symbol->type == SYMBOL_FUNCTION_SET ? (Node*)symbol->functionSet.overloads[overloadIdx].declaration : symbol->declaration;

	if (symbolDocument == this)
	{
		StringView declarationName;
		if (getDeclarationNameIdentifier(file, declaration, line, col, &declarationName))
		{
			SourceLocation start, end;
			getSourceLocation(symbolDocument->file, declarationName, &start, &end);

			changes.push_back({
				{"newText", newText},
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
	}
}
