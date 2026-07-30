#pragma once

#include "Type.h"


struct TypeSystem
{
	Type errorType;
	Type primitiveTypes[TYPE_COUNT];
};


void initTypeSystem(TypeSystem* types);
