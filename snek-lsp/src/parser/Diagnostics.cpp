#include "Diagnostics.h"

#include "utils/Arena.h"


void initDiagnostics(Diagnostics* diagnostics, Arena* arena)
{
	diagnostics->arena = arena;
}

void destroyDiagnostics(Diagnostics* diagnostics)
{
	FreeList(&diagnostics->items);
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
	diagnostics->items.add(item);
}
