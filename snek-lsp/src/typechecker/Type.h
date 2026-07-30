#pragma once

#include "parser/TypeKind.h"


struct Type
{
	uint8_t typeKind;
	union {
		struct {
			char* name;
			int numFields;
			Type** fieldTypes;
		} struct_;

		struct {
			char* name;
			int numFields;
			Type** fieldTypes;
		} union_;

		struct {
			Type* elementType;
		} pointer;

		struct {
			Type* elementType;
		} optional;

		struct {
			Type* returnType;
			int numParams;
			Type** paramTypes;
		} function;

		struct {
			Type* elementType;
			uint64_t size;
		} array;
	};
};
