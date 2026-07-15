#pragma once

#include "Log.h"


struct StringView
{
	char* ptr;
	int length;


	char& operator[](int idx)
	{
		SnekAssert(idx < length);
		return ptr[idx];
	}
};


StringView CreateString(const char* start, const char* end)
{
	StringView str = {};
	str.ptr = (char*)start;
	str.length = (int)(end - start);
}
