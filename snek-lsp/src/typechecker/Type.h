#pragma once

#include "parser/TypeKind.h"


struct Type
{
	uint8_t typeKind;
	union {
		struct {

		} struct_;

		struct {

		} union_;

		struct {
			Type* elementType;
		} pointer;

		struct {
			Type* elementType;
		} optional;

		struct {

		} function;

		struct {

		} tuple;

		struct {
			Type* elementType;
			uint64_t size;
		} array;
	};
};
