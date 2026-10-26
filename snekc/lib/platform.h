#pragma once


typedef unsigned int	   DWORD;
typedef unsigned int	   UINT;
typedef int				   BOOL;
typedef void*			   HANDLE;
typedef unsigned long long SIZE_T;

void sys_init();
void sys_exit(UINT exit_code);
void sys_print(char* buffer, DWORD length);
void* sys_memory(void* ptr, SIZE_T new_size);

#define __bounds_check(x, file, line, col) // __assertmsg(x, "Bounds check failed: " #x " at " file ":" #line ":" #col)


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

#if defined __EMSCRIPTEN__ || defined __TINYC__
typedef _Bool bool;
#endif

typedef struct { char* data; u64 length; } string;
typedef struct {
    union {
        i64 int_;
        double float_;
        bool bool_;
        void* ptr;
    };
    i32 type;
} any;
