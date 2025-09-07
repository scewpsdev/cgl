#include "cgl.h"


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#ifdef _WIN32
#include <windows.h>
#endif


#define	SIGTRAP	5 // debug break


#ifdef __TINYC__
void __debugbreak()
{
	raise(SIGTRAP);
}
#endif

void __assertmsg(int x, const char* msg)
{
	if (!x)
	{
		fprintf(stderr, "%s\n", msg);
#ifdef RUNTIME_STACK_TRACE
		__print_trace();
#endif
		exit(1);
	}
}


void* __alloc(u64 size)
{
	void* ptr = malloc(size);
	memset(ptr, 0, size);
	return ptr;
}

void __free(void* ptr)
{
	free(ptr);
}


i64 __stoi64(string s)
{
	i64 value = 0;
	i32 base = 10;
	i32 offset = 0;
	bool negative = 0;
	if (s.length > 2 && s.ptr[0] == '0' && (s.ptr[1] == 'b' || s.ptr[1] == 'o' || s.ptr[1] == 'x'))
	{
		offset = 2;
		if (s.ptr[1] == 'b')
			base = 2;
		else if (s.ptr[1] == 'o')
			base = 8;
		else if (s.ptr[1] == 'x')
			base = 16;
	}
	for (int i = offset; i < s.length; i++)
	{
		if (i == offset && s.ptr[i] == '-')
			negative = 1;
		else if (s.ptr[i] == '_')
			;
		else if (s.ptr[i] >= '0' && s.ptr[i] <= '9')
			value = value * base + (s.ptr[i] - '0');
		else if ((s.ptr[i] >= 'A' && s.ptr[i] <= 'F' || s.ptr[i] >= 'a' && s.ptr[i] <= 'f') && base == 16)
			value = value * base + (s.ptr[i] - 'a' + 10);
	}

	if (negative)
		value *= -1;

	return value;
}

i32 __stoi32(string s) { return (i32)__stoi64(s); }
i16 __stoi16(string s) { return (i16)__stoi64(s); }
i8 __stoi8(string s) { return (i8)__stoi64(s); }

u64 __stou64(string s)
{
	i64 value = 0;
	i32 base = 10;
	i32 offset = 0;
	if (s.length > 2 && s.ptr[0] == '0' && (s.ptr[1] == 'b' || s.ptr[1] == 'o' || s.ptr[1] == 'x'))
	{
		offset = 2;
		if (s.ptr[1] == 'b')
			base = 2;
		else if (s.ptr[1] == 'o')
			base = 8;
		else if (s.ptr[1] == 'x')
			base = 16;
	}

	for (int i = s.length - 1; i >= offset; i--)
	{
		if (s.ptr[i] == '_')
			;
		else if (s.ptr[i] >= '0' && s.ptr[i] <= '9')
			value = value * base + (s.ptr[i] - '0');
		else if ((s.ptr[i] >= 'A' && s.ptr[i] <= 'F' || s.ptr[i] >= 'a' && s.ptr[i] <= 'f') && base == 16)
			value = value * base + (s.ptr[i] - 'a' + 10);
	}

	return value;
}

u32 __stou32(string s) { return (u32)__stou64(s); }
u16 __stou16(string s) { return (u16)__stou64(s); }
u8 __stou8(string s) { return (u8)__stou64(s); }

string __itos(i64 n)
{
	char* buffer = malloc(21);
	int i = 0;

	bool isNeg = n < 0;

	i64 n1 = isNeg ? -n : n;

	while (n1 != 0)
	{
		buffer[i++] = n1 % 10 + '0';
		n1 = n1 / 10;
	}

	if (isNeg)
		buffer[i++] = '-';

	buffer[i] = '\0';

	for (int t = 0; t < i / 2; t++)
	{
		buffer[t] ^= buffer[i - t - 1];
		buffer[i - t - 1] ^= buffer[t];
		buffer[t] ^= buffer[i - t - 1];
	}

	if (n == 0)
	{
		buffer[0] = '0';
		buffer[1] = '\0';
		i = 1;
	}

	return (string) { buffer, i };
}

string __utos(u64 n)
{
	char* buffer = malloc(20);
	int i = 0;

	while (n != 0)
	{
		buffer[i++] = n % 10 + '0';
		n = n / 10;
	}

	buffer[i] = '\0';

	for (int t = 0; t < i / 2; t++)
	{
		buffer[t] ^= buffer[i - t - 1];
		buffer[i - t - 1] ^= buffer[t];
		buffer[t] ^= buffer[i - t - 1];
	}

	if (n == 0)
	{
		buffer[0] = '0';
		buffer[1] = '\0';
		i = 1;
	}

	return (string) { buffer, i };
}

i32 __cstrl(const char* str)
{
	return (i32)strlen(str);
}

void* __loadDllFunc(const char* dllName, const char* funcName)
{
	void* library = LoadLibraryA(dllName);
	if (!library)
	{
		fprintf(stderr, "Failed to load %s\n", dllName);
		__debugbreak();
		exit(-1);
	}

	void* func = GetProcAddress(library, funcName);
	if (!func)
	{
		fprintf(stderr, "Failed to load Dll function %s:%s\n", dllName, funcName);
		__debugbreak();
		exit(-1);
	}

	return func;
}

// DEBUGGING
#ifdef RUNTIME_STACK_TRACE

#define MAX_TRACE_DEPTH 256

typedef struct {
	const char* func;
	const char* file;
	int line;
} TraceFrame;

static TraceFrame g_trace_stack[MAX_TRACE_DEPTH];
static int g_trace_stack_top = 0;

void __push_trace(const char* func, const char* file, int line) {
	if (g_trace_stack_top < MAX_TRACE_DEPTH) {
		g_trace_stack[g_trace_stack_top++] = (TraceFrame){ func, file, line };
	}
}

void __pop_trace(void) {
	if (g_trace_stack_top > 0) {
		g_trace_stack_top--;
	}
}

void __print_trace(void) {
	fprintf(stderr, "\n=== Stack Trace ===\n");
	for (int i = g_trace_stack_top - 1; i >= 0; --i) {
		TraceFrame* f = &g_trace_stack[i];
		fprintf(stderr, "  at %s (%s:%d)\n", f->func, f->file, f->line);
	}
	fprintf(stderr, "============================\n");
}

void __panic(const char* msg) {
	fprintf(stderr, "\nPanic: %s\n", msg);
	__print_trace();
	exit(1);
}

static void __signal_handler(int sig) {
	fprintf(stderr, "\nCrash: Signal %d\n", sig);
	__print_trace();
	exit(1);
}

#ifdef _WIN32

LONG WINAPI rain__seh_filter(EXCEPTION_POINTERS* ExceptionInfo) {
	fprintf(stderr, "\nCrash (Windows SEH): Code 0x%08X\n",
		ExceptionInfo->ExceptionRecord->ExceptionCode);
	__print_trace();
	return EXCEPTION_EXECUTE_HANDLER;
}

void __setup_crash_handler(void) {
	SetUnhandledExceptionFilter(rain__seh_filter);
	signal(SIGFPE, __signal_handler);
	signal(SIGILL, __signal_handler);
	signal(SIGABRT, __signal_handler);
}
#else
void __setup_crash_handler(void) {
	signal(SIGSEGV, __signal_handler);
	signal(SIGFPE, __signal_handler);
	signal(SIGILL, __signal_handler);
	signal(SIGABRT, __signal_handler);
}
#endif

__attribute__((unused))
static void __trace_pop_guard(int* guard) {
	(void)guard;
	__pop_trace();
}

#endif