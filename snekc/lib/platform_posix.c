#include "platform.h"

// Define basic types since windows headers are absent
#define NULL ((void*)0)
typedef unsigned long long SIZE_T;

// has to match table in sys_open_file
#define SYS_FILE_WRITE (0x1)
#define SYS_FILE_READ  (0x2)

/* ============================================================================
 * 1. Linux x86_64 System Call Table & Constants
 * ============================================================================ */
#define SYS_read      0
#define SYS_write     1
#define SYS_open      2
#define SYS_close     3
#define SYS_stat      4
#define SYS_fstat     5
#define SYS_mmap      9
#define SYS_munmap    11
#define SYS_mremap    25
#define SYS_exit      60

#define O_RDONLY      00
#define O_WRONLY      01
#define O_RDWR        02
#define O_CREAT       0100
#define O_TRUNC       01000

#define PROT_READ     0x1
#define PROT_WRITE    0x2
#define MAP_PRIVATE   0x02
#define MAP_ANONYMOUS 0x20
#define MREMAP_MAYMOVE 1

 /* ============================================================================
  * 2. High-Performance Raw Inline Syscall Wrapper
  * ============================================================================ */
static inline long long sys_call(long long num, long long a1, long long a2, long long a3, long long a4, long long a5, long long a6) {
    long long ret;
    __asm__ volatile (
        "movq %1, %%rax\n\t"
        "movq %2, %%rdi\n\t"
        "movq %3, %%rsi\n\t"
        "movq %4, %%rdx\n\t"
        "movq %5, %%r10\n\t" // Linux system calls use r10 instead of rcx
        "movq %6, %%r8\n\t"
        "movq %7, %%r9\n\t"
        "syscall\n\t"
        "movq %%rax, %0"
        : "=r"(ret)
        : "g"(num), "g"(a1), "g"(a2), "g"(a3), "g"(a4), "g"(a5), "g"(a6)
        : "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9", "rcx", "r11", "memory"
        );
    return ret;
}

/* ============================================================================
 * 3. Standard C Freestanding Language Compliance Functions
 * ============================================================================ */
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

// FIX: Map alloca cleanly to TCC's built-in macro identifier to eliminate signature collision crashes
#define alloca(size) __builtin_alloca(size)

/* ============================================================================
 * 4. Command Line Argument Parsing Configuration
 * ============================================================================ */

int _main(int argc, string* argv);

__attribute__((naked))
void _start(void) {
    void** stack_ptr;
    // Extract the raw parameters straight from the Stack Pointer context
    __asm__("movq %%rsp, %0" : "=r" (stack_ptr));

    long long raw_argc = *(long long*)stack_ptr;
    char** raw_argv = (char**)(stack_ptr + 1);

    // Turn stack parameters into your custom 'string' array tokens
    string args[raw_argc];
    for (int i = 0; i < raw_argc; i++) {
        args[i].data = raw_argv[i];
        SIZE_T len = 0;
        while (raw_argv[i][len]) len++;
        args[i].size = len;
    }

    int exit_code = _main((int)raw_argc, args);
    sys_exit(exit_code);
}

void sys_exit(unsigned int exit_code) {
    sys_call(SYS_exit, exit_code, 0, 0, 0, 0, 0);
    while (1);
}

/* ============================================================================
 * 5. Memory Subsystem (Sized Heap Allocator over mmap)
 * ============================================================================ */
void* sys_memory(void* ptr, unsigned long long new_size) {
    if (new_size == 0) {
        if (ptr) {
            unsigned long long* raw_block = (unsigned long long*)ptr - 1;
            unsigned long long size = *raw_block;
            sys_call(SYS_munmap, (long long)raw_block, size, 0, 0, 0, 0);
        }
        return NULL;
    }

    if (ptr == NULL) {
        unsigned long long total_size = new_size + sizeof(unsigned long long);
        long long res = sys_call(SYS_mmap, 0, total_size, (PROT_READ | PROT_WRITE), (MAP_PRIVATE | MAP_ANONYMOUS), -1, 0);
        if (res < 0) return NULL;

        unsigned long long* raw_block = (unsigned long long*)res;
        *raw_block = total_size;
        return (void*)(raw_block + 1);
    }

    unsigned long long* old_block = (unsigned long long*)ptr - 1;
    unsigned long long old_total_size = *old_block;
    unsigned long long new_total_size = new_size + sizeof(unsigned long long);

    long long res = sys_call(SYS_mremap, (long long)old_block, old_total_size, new_total_size, MREMAP_MAYMOVE, 0, 0);
    if (res < 0) return NULL;

    unsigned long long* new_block = (unsigned long long*)res;
    *new_block = new_total_size;
    return (void*)(new_block + 1);
}

/* ============================================================================
 * 6. File System Engine Implementation
 * ============================================================================ */
void* sys_open_file(string path, unsigned int access) {
    char path_terminated[512];
    if (path.size >= 512) return NULL;
    memcpy(path_terminated, path.data, path.size);
    path_terminated[path.size] = '\0';

    int flags = 0;
    if ((access & SYS_FILE_READ) && (access & SYS_FILE_WRITE)) flags |= O_RDWR;
    else if (access & SYS_FILE_WRITE) flags |= O_WRONLY;
    else flags |= O_RDONLY;

    if (access & SYS_FILE_WRITE) flags |= (O_CREAT | O_TRUNC);

    long long fd = sys_call(SYS_open, (long long)path_terminated, flags, 0664, 0, 0, 0);
    if (fd < 0) return NULL;
    return (void*)fd;
}

void sys_close_file(void* file) {
    long long fd = (long long)file;
    if (fd >= 0) sys_call(SYS_close, fd, 0, 0, 0, 0, 0);
}

unsigned long long sys_get_file_size(void* file) {
    long long fd = (long long)file;
    if (fd < 0) return 0;

    unsigned char stat_buf[144];
    long long res = sys_call(SYS_fstat, fd, (long long)stat_buf, 0, 0, 0, 0);
    if (res < 0) return 0;

    return *(unsigned long long*) & stat_buf[48]; // Offset 48 tracks file size fields
}

unsigned long long sys_write_file(void* file, const unsigned char* buffer, unsigned long long size) {
    long long fd = (long long)file;
    if (fd < 0 || !buffer) return 0;

    long long written = sys_call(SYS_write, fd, (long long)buffer, size, 0, 0, 0);
    return (written < 0) ? 0 : (unsigned long long)written;
}

unsigned long long sys_read_file(void* file, void* buffer, unsigned long long size) {
    long long fd = (long long)file;
    if (fd < 0 || !buffer) return 0;

    long long read_bytes = sys_call(SYS_read, fd, (long long)buffer, size, 0, 0, 0);
    return (read_bytes < 0) ? 0 : (unsigned long long)read_bytes;
}

void* sys_get_std_file(unsigned int stream) {
    if (stream >= 1 && stream <= 3) {
        return (void*)(unsigned long long)(stream - 1);
    }
    return (void*)-1;
}

void sys_debug_trap() {
    __asm__ volatile("int3");
}
