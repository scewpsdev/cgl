#pragma once


struct AST;


void parse(AST* ast, const char* filename, const char* src, int length);
