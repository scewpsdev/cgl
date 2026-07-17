#pragma once


struct StringView
{
	char* ptr;
	int length;


	char& operator[](int idx);
};


StringView CreateString(const char* start, const char* end);
StringView CreateString(const char* start, int length);

char* substring(const char* str, int offset, int length = -1);
char* concat(const char* str1, const char* str2);
char* concatDelete(const char* str1, const char* str2);
