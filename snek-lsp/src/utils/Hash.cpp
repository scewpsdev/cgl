#include "Hash.h"

#include <string.h>


uint32_t hash(uint32_t i)
{
	i = (uint32_t)(i ^ (uint32_t)(61)) ^ (uint32_t)(i >> (uint32_t)(16));
	i *= (uint32_t)(9);
	i = i ^ (i >> 4);
	i *= (uint32_t)(0x27d4eb2d);
	i = i ^ (i >> 15);
	return i;
}

uint32_t hash(float f)
{
	return hash(*(uint32_t*)&f);
}

// FNV-1a
uint32_t hash(const char* str)
{
	uint32_t hash = 2166136261u;
	int i = 0;
	while (char c = str[i++])
	{
		hash ^= (uint8_t)c;
		hash *= 16777619u;
	}
	return hash;
}

uint32_t hash(StringView str)
{
	uint32_t hash = 2166136261u;
	for (int i = 0; i < str.length; i++)
	{
		hash ^= (uint8_t)str.ptr[i];
		hash *= 16777619u;
	}
	return hash;
}

uint32_t hash(const void* ptr)
{
	return (uint32_t)(uint64_t)ptr;
}

uint32_t hashCombine(uint32_t h0, uint32_t h1)
{
	return h0 * 19 + h1;
}
