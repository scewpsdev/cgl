#include "platform.h"


// has to match table in sys_print
#define SYS_INPUT  1
#define SYS_OUTPUT 2
#define SYS_ERROR  3

// has to match table in sys_open_file
#define SYS_FILE_WRITE (0x1)
#define SYS_FILE_READ  (0x2)

#define STD_INPUT_HANDLE      ((DWORD)-10)
#define STD_OUTPUT_HANDLE     ((DWORD)-11)
#define STD_ERROR_HANDLE      ((DWORD)-12)
#define NULL                  ((void*)0)
#define GENERIC_WRITE         0x40000000L
#define GENERIC_READ          0x80000000L
#define CREATE_ALWAYS         2
#define OPEN_EXISTING         3
#define FILE_SHARE_READ       0x00000001
#define FILE_ATTRIBUTE_NORMAL 0x00000080
#define INVALID_HANDLE_VALUE  ((HANDLE)(SIZE_T)-1)
#define INVALID_FILE_SIZE     ((DWORD)0xFFFFFFFF)


typedef unsigned int	   DWORD;
typedef unsigned int	   UINT;
typedef int				   BOOL;
typedef void*              HANDLE;
typedef unsigned long long SIZE_T;
typedef long long          LARGE_INTEGER;


DLLIMPORT HANDLE STDCALL GetProcessHeap(void);
DLLIMPORT void*  STDCALL HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes);
DLLIMPORT void*  STDCALL HeapReAlloc(HANDLE hHeap, DWORD dwFlags, void* lpMem, SIZE_T dwBytes);
DLLIMPORT BOOL   STDCALL HeapFree(HANDLE hHeap, DWORD dwFlags, void* lpMem);
DLLIMPORT char*  STDCALL GetCommandLineA(void);
DLLIMPORT void   STDCALL ExitProcess(UINT uExitCode);

DLLIMPORT HANDLE STDCALL GetStdHandle(DWORD nStdHandle);
DLLIMPORT BOOL   STDCALL WriteFile(HANDLE hFile, const void* lpBuffer, DWORD nNumberOfBytesToWrite, DWORD* lpNumberOfBytesWritten, void* lpOverlapped);
DLLIMPORT BOOL   STDCALL ReadFile(HANDLE hFile, void* lpBuffer, DWORD nNumberOfBytesToRead, DWORD* lpNumberOfBytesRead, void* lpOverlapped);
DLLIMPORT HANDLE CreateFileA(const char* lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, void* lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
DLLIMPORT BOOL   GetFileSizeEx(HANDLE hFile, LARGE_INTEGER* lpFileSize);
DLLIMPORT BOOL   CloseHandle(HANDLE hObject);


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
    }
    else {
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


static int parse_cmd_args(char* arg_str, SIZE_T len, string* args) {
    int num_args = 0;
    SIZE_T arg_start = 0;
    BOOL in_string = 0;
    for (SIZE_T i = 0; i < len + 1; i++) {
        char c = i < len ? arg_str[i] : 0;
        if (c == ' ' && !in_string || c == 0) {
            SIZE_T arg_len = i - arg_start;
            if (arg_len > 0) {
                if (args) {
                    BOOL is_quoted = arg_str[arg_start] == '"' && arg_str[i - 1] == '"';
                    if (is_quoted) {
                        arg_start++;
                        arg_len -= 2;
                    }
                    args[num_args] = (string) { &arg_str[arg_start], arg_len };
                }
                num_args++;
            }
            arg_start = i + 1;
        } else if (c == '"') {
            in_string = !in_string;
        }
    }
    return num_args;
}

int _main(int argc, string* argv);
void _start(void) {
    char* arg_str = GetCommandLineA();
    SIZE_T arg_len = 0;
    while (arg_str[arg_len])
        arg_len++;

    int argc = parse_cmd_args(arg_str, arg_len, NULL);
    string args[argc];
    parse_cmd_args(arg_str, arg_len, args);

    int exit_code = _main(argc, args);

    ExitProcess(exit_code);
}

void sys_exit(unsigned int exit_code) {
    ExitProcess(exit_code);
}

void* sys_memory(void* ptr, unsigned long long new_size) {
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

void* sys_open_file(string path, unsigned int access) {
    char path_terminated[512];
    memcpy(path_terminated, path.data, path.size);
    path_terminated[path.size] = '\0';

    DWORD accessDword = 0;
    if (access & SYS_FILE_WRITE)
        accessDword |= GENERIC_WRITE;
    if (access & SYS_FILE_READ)
        accessDword |= GENERIC_READ;

    DWORD disposition = access & SYS_FILE_READ ? OPEN_EXISTING : CREATE_ALWAYS;

    HANDLE file = CreateFileA(path_terminated, accessDword, FILE_SHARE_READ, NULL, disposition, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE)
        return NULL;

    return (void*)file;
}

void sys_close_file(void* file) {
    CloseHandle((HANDLE)file);
}

unsigned long long sys_get_file_size(void* file) {
    LARGE_INTEGER file_size;
    BOOL success = GetFileSizeEx(file, &file_size);
    if (!success)
        return 0;
    return file_size;
}

unsigned long long sys_write_file(void* file, const unsigned char* buffer, unsigned long long size) {
    unsigned long long chunk_size = 1024 * 1024 * 1024;
    unsigned long long total_bytes_written = 0;

    while (size) {
        unsigned long long bytes_to_write = size > chunk_size ? chunk_size : size;

        DWORD bytes_written = 0;
        BOOL success = WriteFile((HANDLE)file, buffer + total_bytes_written, (DWORD)bytes_to_write, &bytes_written, NULL);
        if (!success)
            return 0;

        size = size > chunk_size ? size - chunk_size : 0;
        total_bytes_written += bytes_written;
    }

    return total_bytes_written;
}

unsigned long long sys_read_file(void* file, void* buffer, unsigned long long size) {
    unsigned long long chunk_size = 1024 * 1024 * 1024;
    unsigned long long total_bytes_read = 0;

    while (size) {
        unsigned long long bytes_to_read = size > chunk_size ? chunk_size : size;

        DWORD bytes_read = 0;
        BOOL success = ReadFile((HANDLE)file, buffer + total_bytes_read, (DWORD)bytes_to_read, &bytes_read, NULL);
        if (!success)
            return 0;

        size = size > chunk_size ? size - chunk_size : 0;
        total_bytes_read += bytes_read;
    }

    return total_bytes_read;
}

void* sys_get_std_file(unsigned int stream) {
    DWORD handles[] = {
        0,
        STD_INPUT_HANDLE,
        STD_OUTPUT_HANDLE,
        STD_ERROR_HANDLE
    };
    return GetStdHandle(handles[stream]);
}

void sys_debug_trap() {
    __asm__ volatile("int3");
}
