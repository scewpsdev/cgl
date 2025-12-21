#include "Document.h"

#include "cgl/CGLCompiler.h"
#include "cgl/Platform.h"

#include "cgl/semantics/Variable.h"

#include <stdarg.h>


using namespace nlohmann;


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

void Document::reparse(std::vector<int>& data)
{
	uint64_t beforeParse = GetTimeNS();

	if (compiler)
		delete compiler;
	if (lexer)
		delete lexer;
	if (parser)
		delete parser;
	if (resolver)
		delete resolver;

	compiler = new CGLCompiler();
	compiler->init(OnCompilerMessage);
	SourceFile sourceFile = {};
	sourceFile.filename = uri.c_str();
	sourceFile.name = "testfile";
	sourceFile.source = text.c_str();

	lexer = new Lexer(compiler, sourceFile.filename, sourceFile.source);
	parser = new Parser(compiler);

	std::vector<LSPToken> tokens;

	auto addToken = [&tokens](Token token, int type, int modifiers)
		{
			tokens.push_back({ token, type, modifiers });
		};

	while (LexerHasNext(lexer))
	{
		Token token = LexerNext(lexer);

		if (token.keywordType != KEYWORD_TYPE_NULL)
		{
			if (token.keywordType > KEYWORD_TYPE_BUILTIN_TYPES_START && token.keywordType < KEYWORD_TYPE_BUILTIN_TYPES_END)
				addToken(token, LSP_TOKEN_TYPE, 0);
			else
				addToken(token, LSP_TOKEN_KEYWORD, 0);
		}
		else
		{
			switch (token.type)
			{
			case TOKEN_TYPE_STRING_LITERAL:
			case TOKEN_TYPE_CHAR_LITERAL:
				addToken(token, LSP_TOKEN_STRING, 0);
				break;
			case TOKEN_TYPE_STRING_LITERAL_MULTILINE:
				break;
			case TOKEN_TYPE_FLOAT_LITERAL:
			case TOKEN_TYPE_DOUBLE_LITERAL:
			case TOKEN_TYPE_INT_LITERAL:
				addToken(token, LSP_TOKEN_NUMBER, 0);
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
				addToken(token, LSP_TOKEN_OPERATOR, 0);
				break;
			case TOKEN_TYPE_IDENTIFIER:
				break;
			default:
				break;
			}
		}
	}

	ast = parser->run(sourceFile);

	for (int i = 0; i < ast->functions.size; i++)
	{
		AST::Function* function = ast->functions[i];
		addToken(function->nameToken, LSP_TOKEN_FUNCTION, 0);
	}
	for (int i = 0; i < ast->structs.size; i++)
	{
		AST::Struct* strct = ast->structs[i];
		addToken(strct->nameToken, LSP_TOKEN_STRUCT, 0);
	}

	List<AST::File*> asts;
	asts.add(ast);
	resolver = new Resolver(compiler, asts);
	resolver->run();

	for (int i = 0; i < resolver->identifiers.size; i++)
	{
		AST::Identifier* identifier = resolver->identifiers[i];
		int type = identifier->functions.size ? LSP_TOKEN_FUNCTION :
			//identifier->variable ? LSP_TOKEN_VARIABLE :
			identifier->enumValue ? LSP_TOKEN_ENUM_VALUE :
			identifier->exprdefValue ? LSP_TOKEN_MACRO :
			identifier->enumDecl ? LSP_TOKEN_ENUM :
			identifier->module ? LSP_TOKEN_NAMESPACE :
			identifier->builtinType ? LSP_TOKEN_TYPE :
			-1;
		if (type != -1)
			addToken(identifier->nameToken, type, 0);
	}
	for (int i = 0; i < resolver->namedTypes.size; i++)
	{
		AST::NamedType* namedType = resolver->namedTypes[i];
		int type = namedType->typeID->typeKind == AST::TypeKind::Struct ? LSP_TOKEN_STRUCT :
			namedType->typeID->typeKind == AST::TypeKind::Class ? LSP_TOKEN_CLASS :
			namedType->declaration->type == AST::DeclarationType::Enumeration ? LSP_TOKEN_ENUM :
			LSP_TOKEN_TYPE;
		if (type != -1)
			addToken(namedType->nameToken, type, 0);
	}

	qsort(tokens.data(), tokens.size(), sizeof(LSPToken), (_CoreCrtNonSecureSearchSortCompareFunction)LSPTokenComparator);

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
}

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

void Document::autocomplete(json& items, int line, int col)
{
	if (resolver)
	{
		if (Scope* scope = FindScopeAtSourceLocation(resolver->globalScope, line, col))
		{
			fprintf(stderr, "Found completion scope at %d, %d\n", scope->start.line, scope->start.col);
			ProcessCompletionScope(scope, items, resolver);
		}

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
}
