#pragma once

#include <stdio.h>


#ifndef _WIN32
#define _strdup strdup
#define __min(a, b) ((a) < (b) ? (a) : (b))
#define __max(a, b) ((a) > (b) ? (a) : (b))
#endif


void __debugbreak();

#if defined(_DEBUG)
#define SnekAssert(condition) { if (!(condition)) { fprintf(stderr, "Assertion (%s) failed at %s line %d", #condition, __FILE__, __LINE__); __debugbreak(); } }
#define SnekAssertMsg(condition, message) { if (!(condition)) { fprintf(stderr, "Assertion (%s) failed at %s line %d: %s", #condition, __FILE__, __LINE__, message); __debugbreak(); } }
#else
#define SnekAssert(condition)
#define SnekAssertMsg(condition, message)
#endif
