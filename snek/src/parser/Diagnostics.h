#pragma once

#include "utils/List.h"

#include <stdint.h>


enum DiagnosticStage : uint8_t
{
	DIAGNOSTICS_PARSER_STAGE,
	DIAGNOSTICS_TYPECHECK_STAGE,
};

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
	uint8_t stage;

	int padding0;
};

struct Diagnostics
{
	Arena* arena;
	DiagnosticStage stage;

	List<DiagnosticsItem> items;
};


void initDiagnostics(Diagnostics* diagnostics, Arena* arena);
void destroyDiagnostics(Diagnostics* diagnostics);

void resetDiagnostics(Diagnostics* diagnostics, DiagnosticStage stage);

void logMessage(Diagnostics* diagnostics, const char* message, int startLine, int startCol, int endLine, int endCol, uint8_t severity);
