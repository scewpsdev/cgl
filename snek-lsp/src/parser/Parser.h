#pragma once


struct AST;
struct Arena;
struct Diagnostics;


void parse(AST* ast, Arena* arena, Diagnostics* diagnostics, const char* filename, const char* src, int length);
