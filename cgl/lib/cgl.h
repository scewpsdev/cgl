#pragma once


typedef char i8;
typedef short i16;
typedef int i32;
typedef long long i64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef float f32;
typedef double f64;
typedef _Bool bool;
typedef struct { char* ptr; long length; } string;
typedef struct { void* type; void* value; } any;

#define __INT8_MIN -0x80
#define __INT8_MAX 0x7f
#define __INT16_MIN -0x8000
#define __INT16_MAX 0x7fff
#define __INT32_MIN -0x80000000
#define __INT32_MAX 0x7fffffff
#define __INT64_MIN -0x8000000000000000
#define __INT64_MAX 0x7fffffffffffffff
#define __UINT8_MIN 0
#define __UINT8_MAX 0xff
#define __UINT16_MIN 0
#define __UINT16_MAX 0xffff
#define __UINT32_MIN 0
#define __UINT32_MAX 0xffffffff
#define __UINT64_MIN 0
#define __UINT64_MAX 0xffffffffffffffff

void __debugbreak();
void __assertmsg(int x, const char* msg);
#define assert(x, file, line, col) __assertmsg(x, "Assertion failed at " file ":" #line ":" #col ": " #x)

i8 __stoi8(string s);
i16 __stoi16(string s);
i32 __stoi32(string s);
i64 __stoi64(string s);
u8 __stou8(string s);
u16 __stou16(string s);
u32 __stou32(string s);
u64 __stou64(string s);
string __itos(i64 i);
string __utos(u64 n);

extern long long strlen(const char* str);

void* __alloc(u64 size);
void __free(void* ptr);
