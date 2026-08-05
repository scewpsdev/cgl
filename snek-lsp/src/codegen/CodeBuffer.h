#pragma once

#include "utils/StringView.h"


struct Arena;

struct CodeBuffer
{
	char* data;
	int capacity;
	int count;

	Arena* arena;


	void emitString(const char* str);
	void emitString(StringView str);
	void emitChar(char c);
};


void initCodeBuffer(CodeBuffer* buffer, Arena* arena, int initialCapacity);
void resetCodeBuffer(CodeBuffer* buffer);

void emitString(CodeBuffer* buffer, const char* str);
void emitString(CodeBuffer* buffer, StringView str);
void emitChar(CodeBuffer* buffer, char c);
