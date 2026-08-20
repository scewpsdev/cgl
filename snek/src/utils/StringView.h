#pragma once


struct Arena;

struct StringView
{
	char* ptr;
	int length;


	char& operator[](int idx);
	bool startsWith(const char* str);
	bool endsWith(const char* str);
	StringView substring(int offset, int count = -1);
};


StringView CreateString(const char* start, const char* end);
StringView CreateString(const char* start, int length);
StringView CreateString(const char* ptr);

StringView copy(StringView from);
StringView copy(StringView from, Arena* arena);
void destroy(StringView str);

bool compareString(StringView a, StringView b);
bool compareString(StringView a, const char* b);

char* substring(const char* str, int offset, int length = -1);
char* concat(const char* str1, const char* str2);
char* concatDelete(const char* str1, const char* str2);
