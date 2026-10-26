#include "platform.h"


#define STD_OUTPUT_HANDLE ((DWORD)-11)
#define NULL ((void*)0)


DLLIMPORT HANDLE STDCALL GetStdHandle(DWORD nStdHandle);
DLLIMPORT BOOL   STDCALL WriteFile(HANDLE hFile, const void* lpBuffer, DWORD nNumberOfBytesToWrite, DWORD* lpNumberOfBytesWritten, void* lpOverlapped);
DLLIMPORT HANDLE STDCALL GetProcessHeap(void);
DLLIMPORT void*  STDCALL HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes);
DLLIMPORT void*  STDCALL HeapReAlloc(HANDLE hHeap, DWORD dwFlags, void* lpMem, SIZE_T dwBytes);
DLLIMPORT BOOL   STDCALL HeapFree(HANDLE hHeap, DWORD dwFlags, void* lpMem);
DLLIMPORT void   STDCALL ExitProcess(UINT uExitCode);


static int local_strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

void sys_exit(UINT exit_code) {
    ExitProcess(exit_code);
}

void sys_print(char* buffer, DWORD length) {
    if (buffer && length > 0) {
        HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD bytes_written;
        WriteFile(stdout_handle, buffer, length, &bytes_written, NULL);
    }
}

void* sys_memory(void* ptr, SIZE_T new_size) {
    HANDLE heap = GetProcessHeap();

    if (new_size == 0) {
        if (ptr) {
            HeapFree(heap, 0, ptr);
        }
        return NULL;
    }
    if (ptr == NULL) {
        return HeapAlloc(heap, 0, new_size);
    }
    return HeapReAlloc(heap, 0, ptr, new_size);
}

void* memset(void* dst, int val, SIZE_T len) {
    unsigned char* ptr = (unsigned char*)dst;
    while (len) {
        *ptr = (unsigned char)val;
        ptr++;
        len--;
    }
    return dst;
}

void* memcpy(void* dst, const void* src, SIZE_T len) {
    char* dst_ = (char*)dst;
    char* src_ = (char*)src;
    while (len) {
        *dst_ = *src_;
        dst_++;
        src_++;
        len--;
    }
    return dst;
}

void* memmove(void* dst, const void* src, SIZE_T len) {
    char* dst_ = (char*)dst;
    char* src_ = (char*)src;
    if (dst < src) {
        while (len) {
            *dst_++ = *src_++;
            len--;
        }
    } else {
        dst_ = dst_ + len;
        src_ = src_ + len;
        while (len) {
            *--dst_ = *--src_;
            len--;
        }
    }
    return dst;
}

int memcmp(const void* a, const void* b, SIZE_T len) {
    const unsigned char* s1 = (const unsigned char*)a;
    const unsigned char* s2 = (const unsigned char*)b;
    while (len) {
        if (*s1 != *s2) {
            return *s1 - *s2;
        }
        s1++;
        s2++;
        len--;
    }
    return 0;
}

void* alloca(SIZE_T size) {
    size = (size + 15) & ~15;

    void* result;
    __asm__ volatile (
        "subq %1, %%rsp\n\t"
        "movq %%rsp, %0"
        : "=r" (result)
        : "r" (size)
    );

    return result;
}
