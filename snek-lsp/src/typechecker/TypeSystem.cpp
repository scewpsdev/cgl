#include "TypeSystem.h"


void initTypeSystem(TypeSystem* types)
{
	types->errorType = { .typeKind = TYPE_NULL };

	types->primitiveTypes[TYPE_VOID] = { .typeKind = TYPE_VOID };
	types->primitiveTypes[TYPE_INT8] = { .typeKind = TYPE_INT8 };
	types->primitiveTypes[TYPE_INT16] = { .typeKind = TYPE_INT16 };
	types->primitiveTypes[TYPE_INT32] = { .typeKind = TYPE_INT32 };
	types->primitiveTypes[TYPE_INT64] = { .typeKind = TYPE_INT64 };
	types->primitiveTypes[TYPE_UINT8] = { .typeKind = TYPE_UINT8 };
	types->primitiveTypes[TYPE_UINT16] = { .typeKind = TYPE_UINT16 };
	types->primitiveTypes[TYPE_UINT32] = { .typeKind = TYPE_UINT32 };
	types->primitiveTypes[TYPE_UINT64] = { .typeKind = TYPE_UINT64 };
	types->primitiveTypes[TYPE_FLOAT] = { .typeKind = TYPE_FLOAT };
	types->primitiveTypes[TYPE_DOUBLE] = { .typeKind = TYPE_DOUBLE };
	types->primitiveTypes[TYPE_BOOL] = { .typeKind = TYPE_BOOL };
	types->primitiveTypes[TYPE_ANY] = { .typeKind = TYPE_ANY };
	types->primitiveTypes[TYPE_STRING] = { .typeKind = TYPE_STRING };
}
