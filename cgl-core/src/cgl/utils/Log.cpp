#include "pch.h"
#include "log.h"

#include "cgl/CGLCompiler.h"


MessageCallback_t GetMsgCallback(CGLCompiler* context)
{
	return context->msgCallback;
}
