#include "StringView.h"

#include "Log.h"

#include <string.h>
#include <stdlib.h>


char& StringView::operator[](int idx)
{
	SnekAssert(idx < length);
	return ptr[idx];
}

StringView CreateString(const char* start, const char* end)
{
	StringView str = {};
	str.ptr = (char*)start;
	str.length = (int)(end - start);
	return str;
}

char* substring(const char* str, int offset, int length)
{
	if (length == -1) length = (int)strlen(str) - offset;
	char* buffer = (char*)malloc(length + 1);
	memcpy(buffer, str + offset, length);
	buffer[length] = 0;
	return buffer;
}

char* concat(const char* str1, const char* str2)
{
	int len1 = (int)strlen(str1);
	int len2 = (int)strlen(str2);
	char* buffer = (char*)malloc(len1 + len2 + 1);
	memcpy(buffer, str1, len1);
	memcpy(buffer + len1, str2, len2);
	buffer[len1 + len2] = 0;
	return buffer;
}

char* concatDelete(const char* str1, const char* str2)
{
	char* result = concat(str1, str2);
	free((void*)str1);
	free((void*)str2);
	return result;
}

