#include "Log.h"

#include <stdarg.h>


void LogMessage(MessageType msgType, const char* file, int line, int col, const char* msg, ...)
{
	va_list args;
	va_start(args, msg);
	
	char txt[256];
	vsprintf(txt, msg, args);

	va_end(args);

	const char* msgTypeStr[MESSAGE_TYPE_COUNT] = {
		"null",
		"info",
		"warning",
		"error",
		"fatal"
	};

	fprintf(stderr, "%s %s:%d:%d: %s", msgTypeStr[msgType], file, line, col, txt);
}
