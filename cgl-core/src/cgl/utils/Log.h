#pragma once

#include <stdio.h>


#if defined(_DEBUG)
#define SnekAssert(condition) { if (!(condition)) { fprintf(stderr, "Assertion %s failed at %s line %d", #condition, __FILE__, __LINE__); __debugbreak(); } }
#define SnekAssertMsg(condition, message) { if (!(condition)) { fprintf(stderr, "Assertion %s failed at %s line %d: %s", #condition, __FILE__, __LINE__, message); __debugbreak(); } }
#else
#define SnekAssert(condition)
#define SnekAssertMsg(condition, message)
#endif

#define SnekTrace(context, msg, ...) GetMsgCallback(context)(context, MESSAGE_TYPE_INFO, NULL, 0, 0, msg, ##__VA_ARGS__)
#define SnekWarnLoc(context, location, msg, ...) GetMsgCallback(context)(context, MESSAGE_TYPE_WARNING, (location).filename, (location).line, (location).col, msg, ##__VA_ARGS__)
#define SnekErrorLoc(context, location, msg, ...) GetMsgCallback(context)(context, MESSAGE_TYPE_ERROR, (location).filename, (location).line, (location).col, msg, ##__VA_ARGS__)
#define SnekError(context, msg, ...) GetMsgCallback(context)(context, MESSAGE_TYPE_ERROR, NULL, 0, 0, msg, ##__VA_ARGS__)
#define SnekFatal(context, msg, ...) GetMsgCallback(context)(context, MESSAGE_TYPE_FATAL_ERROR, NULL, 0, 0, msg, ##__VA_ARGS__)


enum MessageType
{
	MESSAGE_TYPE_NULL = 0,

	MESSAGE_TYPE_INFO,
	MESSAGE_TYPE_WARNING,
	MESSAGE_TYPE_ERROR,
	MESSAGE_TYPE_FATAL_ERROR,

	MESSAGE_TYPE_MAX,
};

class CGLCompiler;
typedef void(*MessageCallback_t)(CGLCompiler* context, MessageType msgType, const char* filename, int line, int col, const char* msg, ...);


MessageCallback_t GetMsgCallback(CGLCompiler* context);
