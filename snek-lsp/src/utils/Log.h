#pragma once

#include <stdio.h>


#if defined(_DEBUG)
#define SnekAssert(condition) { if (!(condition)) { fprintf(stderr, "Assertion %s failed at %s line %d", #condition, __FILE__, __LINE__); __debugbreak(); } }
#define SnekAssertMsg(condition, message) { if (!(condition)) { fprintf(stderr, "Assertion %s failed at %s line %d: %s", #condition, __FILE__, __LINE__, message); __debugbreak(); } }
#else
#define SnekAssert(condition)
#define SnekAssertMsg(condition, message)
#endif
