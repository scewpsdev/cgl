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

void sys_init() {
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
