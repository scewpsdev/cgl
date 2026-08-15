#pragma once

#include "utils/StringView.h"


struct Type;

struct Value
{
	union {
		char name[32];
		StringView identifier;
	};
	Type* type;
	bool lvalue;
	bool isIdentifier;
};
