#pragma once

#include <stdio.h>


char* ReadText(const char* path);
int WriteText(const char* txt, const char* path);
int WriteText(const char* txt, FILE* file);
const char* LocalFilePath(const char* path);
void CreateDirectories(const char* path);
