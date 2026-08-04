#include "StringView.h"

#include <string.h>
#include <stdlib.h>


char& StringView::operator[](int idx)
{
	if (!(idx < length))
		__debugbreak();
	return ptr[idx];
}

StringView CreateString(const char* start, const char* end)
{
	StringView str = {};
	str.ptr = (char*)start;
	str.length = (int)(end - start);
	return str;
}

StringView CreateString(const char* start, int length)
{
	StringView str = {};
	str.ptr = (char*)start;
	str.length = length;
	return str;
}

StringView CreateString(const char* ptr)
{
	StringView str = {};
	str.ptr = (char*)ptr;
	str.length = (int)strlen(ptr);
	return str;
}

StringView copy(StringView from)
{
	StringView str = {};
	str.length = from.length;
	str.ptr = (char*)malloc(from.length + 1);
	memcpy(str.ptr, from.ptr, from.length);
	str.ptr[str.length] = 0;
	return str;
}

bool compareString(StringView a, StringView b)
{
	if (a.length != b.length)
		return false;

	return strncmp(a.ptr, b.ptr, a.length) == 0;
}

bool compareString(StringView a, const char* b)
{
	int len = (int)strlen(b);
	if (a.length != len)
		return false;

	return strncmp(a.ptr, b, len) == 0;
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

