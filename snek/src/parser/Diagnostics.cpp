#include "Diagnostics.h"

#include "utils/Arena.h"


void initDiagnostics(Diagnostics* diagnostics, Arena* arena)
{
	*diagnostics = {};
	diagnostics->arena = arena;
	diagnostics->stage = DIAGNOSTICS_PARSER_STAGE;
}

void destroyDiagnostics(Diagnostics* diagnostics)
{
	FreeList(&diagnostics->items);
}

void resetDiagnostics(Diagnostics* diagnostics, DiagnosticStage stage)
{
	for (int i = diagnostics->items.size - 1; i >= 0; i--)
	{
		if (diagnostics->items[i].stage >= stage)
			diagnostics->items.removeAt(i);
	}
}

void logMessage(Diagnostics* diagnostics, const char* message, int startLine, int startCol, int endLine, int endCol, uint8_t severity)
{
	DiagnosticsItem item = {};
	item.message = message;
	item.startLine = startLine;
	item.startCol = startCol;
	item.endLine = endLine;
	item.endCol = endCol;
	item.severity = severity;
	item.stage = diagnostics->stage;
	diagnostics->items.add(item);
}
