#include "Document.h"

#include "Platform.h"
#include "utils/Hash.h"
#include "typechecker/TypeSystem.h"

#include <iostream>
#include <sstream>
#include <set>


using namespace nlohmann;


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

void Document::init(const std::string& text, GlobalBlockPool* blockPool)
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

static void getNodeTokens(Node* node, Scope* scope, ASTVisitorData* data)
{
	if (!node)
		return;

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

		SymbolEntry* symbol = getIdentifierSymbol(identifier);

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

		Type* operandType = member->expression->inferredType;
		if (operandType->typeKind == TYPE_TYPE)
		{
			SnekAssert(member->expression->type == NODE_IDENTIFIER);
			Identifier* typeName = (Identifier*)member->expression;
			if (SymbolEntry* symbol = getIdentifierSymbol(typeName))
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

File* getFileFromHandle(FileHandle fileHandle)
{
	for (int i = 0; i < documents.size; i++)
	{
		if (documents[i]->file.handle == fileHandle)
			return &documents[i]->file;
	}
	return nullptr;
}
