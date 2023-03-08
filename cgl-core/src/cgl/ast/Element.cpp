#include "pch.h"
#include "Element.h"

#include "cgl/parser/input.h"


namespace AST
{
	const SourceLocation SourceLocation::None;


	SourceLocation::SourceLocation()
	{
	}

	SourceLocation::SourceLocation(const InputState& inputState)
		: filename(inputState.filename), line(inputState.line), col(inputState.col)
	{
	}

	Element::Element(File* file, const SourceLocation& location)
		: file(file), location(location)
	{
	}
}
