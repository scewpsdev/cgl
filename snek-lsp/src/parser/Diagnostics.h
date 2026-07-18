#pragma once

#include "utils/List.h"

#include <stdint.h>


enum DiagnosticSeverity : uint8_t
{
	DIAGNOSTICS_ERROR = 1,
	DIAGNOSTICS_WARNING = 2,
	DIAGNOSTICS_INFO = 3,
	DIAGNOSTICS_HINT = 4,
};

struct Arena;

struct DiagnosticsItem
{
	const char* message;
	int startLine, startCol;
	int endLine, endCol;
	uint8_t severity;

	uint8_t padding0;
	int padding1;
};

struct Diagnostics
{
	Arena* arena;
	List<DiagnosticsItem> items;
};


void initDiagnostics(Diagnostics* diagnostics, Arena* arena);
void destroyDiagnostics(Diagnostics* diagnostics);

void logMessage(Diagnostics* diagnostics, const char* message, int startLine, int startCol, int endLine, int endCol, uint8_t severity);
