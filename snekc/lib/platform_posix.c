#include "platform.h"

// Define basic types since windows headers are absent
#define NULL ((void*)0)
typedef unsigned long long SIZE_T;

typedef struct timespec {
    long long tv_sec;
    long long tv_nsec;
} timespec;

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
#define SYS_nanosleep 35
#define SYS_sendfile  40
#define SYS_exit      60
#define SYS_clock_gettime 228

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

void sys_debug_trap() {
    __asm__ volatile("int3");
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

unsigned long long sys_get_file_modified_time(const char* filepath) {
    unsigned char stat_buf[144];
    for (int i = 0; i < 144; i++) stat_buf[i] = 0;

    long long res = sys_call(SYS_stat, (long long)filepath, (long long)stat_buf, 0, 0, 0, 0);
    if (res < 0) return 0;

    // On x86_64 Linux, struct stat stores mtime seconds at offset 88 
    // and mtime nanoseconds at offset 96.
    unsigned long long sec = *(unsigned long long*) & stat_buf[88];
    unsigned long long nsec = *(unsigned long long*) & stat_buf[96];

    return (sec * 1000000000ULL) + nsec;
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

int sys_copy_file(const char* src_path, const char* dst_path) {
    // 1. Open the source file for reading
    long long src_fd = sys_call(SYS_open, (long long)src_path, O_RDONLY, 0, 0, 0, 0);
    if (src_fd < 0) return 0;

    // 2. Get the source file's size using your existing helper
    unsigned long long file_size = sys_get_file_size((void*)src_fd);

    // 3. Open or create the destination file (0664 = rw-rw-r-- permissions)
    long long dst_fd = sys_call(SYS_open, (long long)dst_path, (O_WRONLY | O_CREAT | O_TRUNC), 0664, 0, 0, 0);
    if (dst_fd < 0) {
        sys_call(SYS_close, src_fd, 0, 0, 0, 0, 0);
        return 0;
    }

    // 4. Transfer the bytes directly inside the kernel
    // Note: sendfile transfers up to ~2GB (0x7ffff000 bytes) per call, so we loop until done
    unsigned long long total_copied = 0;
    int success = 1;

    while (total_copied < file_size) {
        unsigned long long remaining = file_size - total_copied;
        unsigned long long chunk = remaining > 0x7ffff000ULL ? 0x7ffff000ULL : remaining;

        long long sent = sys_call(SYS_sendfile, dst_fd, src_fd, 0, chunk, 0, 0);
        if (sent <= 0) {
            success = 0;
            break;
        }
        total_copied += (unsigned long long)sent;
    }

    // 5. Close both file descriptors
    sys_call(SYS_close, dst_fd, 0, 0, 0, 0, 0);
    sys_call(SYS_close, src_fd, 0, 0, 0, 0, 0);

    return success;
}

double sys_time_seconds() {
    timespec time = {0};
    long long res = sys_call(SYS_clock_gettime, CLOCK_MONOTONIC, (long long)&time, 0, 0, 0, 0);
    if (res < 0) return 0.0;
    return (double)time.tv_sec + ((double)time.tv_nsec / 1e9);
}

void sys_sleep(unsigned int milliseconds) {
    timespec req;
    req.tv_sec = milliseconds / 1000;
    req.tv_nsec = (milliseconds % 1000) * 1000000LL;

    // nanosleep(const struct timespec *req, struct timespec *rem)
    sys_call(SYS_nanosleep, (long long)&req, 0, 0, 0, 0, 0);
}


/* ============================================================================
 * 1. Minimal 64-Bit ELF Structures & Constants
 * ============================================================================ */
#define PROT_EXEC  0x4
#define MAP_FIXED  0x10

#define PT_LOAD    1
#define PT_DYNAMIC 2

#define DT_NULL    0
#define DT_STRTAB  5
#define DT_SYMTAB  6
#define DT_SYMENT  11

typedef struct {
    unsigned char e_ident[16];
    unsigned short e_type;
    unsigned short e_machine;
    unsigned int   e_version;
    unsigned long long e_entry;
    unsigned long long e_phoff;
    unsigned long long e_shoff;
    unsigned int   e_flags;
    unsigned short e_ehsize;
    unsigned short e_phentsize;
    unsigned short e_phnum;
    unsigned short e_shentsize;
    unsigned short e_shnum;
    unsigned short e_shstrndx;
} Elf64_Ehdr;

typedef struct {
    unsigned int   p_type;
    unsigned int   p_flags;
    unsigned long long p_offset;
    unsigned long long p_vaddr;
    unsigned long long p_paddr;
    unsigned long long p_filesz;
    unsigned long long p_memsz;
    unsigned long long p_align;
} Elf64_Phdr;

typedef struct {
    long long d_tag;
    unsigned long long d_val;
} Elf64_Dyn;

typedef struct {
    unsigned int   st_name;
    unsigned char  st_info;
    unsigned char  st_other;
    unsigned short st_shndx;
    unsigned long long st_value;
    unsigned long long st_size;
} Elf64_Sym;

// Internal handle to track mapped base address and dynamic tables
typedef struct {
    unsigned char* base_addr;
    unsigned long long map_size;
    Elf64_Sym* symtab;
    const char* strtab;
    unsigned long long syment_size;
} LinuxDynamicLib;

static int local_strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

/* ============================================================================
 * 2. Freestanding ELF Shared Object Loader
 * ============================================================================ */
void* sys_load_library(const char* path) {
    // 1. Open and read the entire .so file into a temporary buffer
    long long fd = sys_call(SYS_open, (long long)path, O_RDONLY, 0, 0, 0, 0);
    if (fd < 0) return NULL;

    unsigned long long file_size = sys_get_file_size((void*)fd);
    if (file_size < sizeof(Elf64_Ehdr)) {
        sys_call(SYS_close, fd, 0, 0, 0, 0, 0);
        return NULL;
    }

    unsigned char* file_buf = (unsigned char*)sys_memory(NULL, file_size);
    if (!file_buf) {
        sys_call(SYS_close, fd, 0, 0, 0, 0, 0);
        return NULL;
    }

    sys_call(SYS_read, fd, (long long)file_buf, file_size, 0, 0, 0);
    sys_call(SYS_close, fd, 0, 0, 0, 0, 0);

    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)file_buf;
    // Verify ELF Magic bytes: 0x7F 'E' 'L' 'F'
    if (ehdr->e_ident[0] != 0x7F || ehdr->e_ident[1] != 'E' ||
        ehdr->e_ident[2] != 'L' || ehdr->e_ident[3] != 'F') {
        sys_memory(file_buf, 0);
        return NULL;
    }

    Elf64_Phdr* phdrs = (Elf64_Phdr*)(file_buf + ehdr->e_phoff);

    // 2. Calculate total virtual address span required by all PT_LOAD segments
    unsigned long long min_vaddr = (unsigned long long) - 1;
    unsigned long long max_vaddr = 0;

    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdrs[i].p_type == PT_LOAD) {
            if (phdrs[i].p_vaddr < min_vaddr) min_vaddr = phdrs[i].p_vaddr;
            unsigned long long end = phdrs[i].p_vaddr + phdrs[i].p_memsz;
            if (end > max_vaddr) max_vaddr = end;
        }
    }

    unsigned long long total_span = max_vaddr - min_vaddr;
    if (total_span == 0) {
        sys_memory(file_buf, 0);
        return NULL;
    }

    // 3. Reserve a contiguous, executable virtual memory region via mmap
    long long base = sys_call(SYS_mmap, 0, total_span,
        (PROT_READ | PROT_WRITE | PROT_EXEC),
        (MAP_PRIVATE | MAP_ANONYMOUS), -1, 0);
    if (base < 0) {
        sys_memory(file_buf, 0);
        return NULL;
    }

    unsigned char* base_ptr = (unsigned char*)base;
    Elf64_Dyn* dyn_section = NULL;

    // 4. Copy segment data into the mapped virtual memory
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdrs[i].p_type == PT_LOAD) {
            unsigned char* dest = base_ptr + (phdrs[i].p_vaddr - min_vaddr);
            unsigned char* src = file_buf + phdrs[i].p_offset;
            memcpy(dest, src, phdrs[i].p_filesz);

            // Zero out any remaining .bss space (memsz > filesz)
            if (phdrs[i].p_memsz > phdrs[i].p_filesz) {
                memset(dest + phdrs[i].p_filesz, 0, phdrs[i].p_memsz - phdrs[i].p_filesz);
            }
        }
        else if (phdrs[i].p_type == PT_DYNAMIC) {
            dyn_section = (Elf64_Dyn*)(base_ptr + (phdrs[i].p_vaddr - min_vaddr));
        }
    }

    sys_memory(file_buf, 0); // Free raw disk buffer

    if (!dyn_section) {
        sys_call(SYS_munmap, (long long)base_ptr, total_span, 0, 0, 0, 0);
        return NULL;
    }

    // 5. Parse PT_DYNAMIC table to locate .dynsym and .dynstr
    LinuxDynamicLib* lib = (LinuxDynamicLib*)sys_memory(NULL, sizeof(LinuxDynamicLib));
    lib->base_addr = base_ptr - min_vaddr; // Adjust base offset if min_vaddr > 0
    lib->map_size = total_span;
    lib->symtab = NULL;
    lib->strtab = NULL;
    lib->syment_size = sizeof(Elf64_Sym);

    for (int i = 0; dyn_section[i].d_tag != DT_NULL; i++) {
        if (dyn_section[i].d_tag == DT_STRTAB) {
            // Note: Depending on linker flags, d_val may be an absolute offset or relative RVA
            unsigned long long offset = dyn_section[i].d_val;
            lib->strtab = (offset < total_span) ? (const char*)(base_ptr + offset) : (const char*)offset;
        }
        else if (dyn_section[i].d_tag == DT_SYMTAB) {
            unsigned long long offset = dyn_section[i].d_val;
            lib->symtab = (offset < total_span) ? (Elf64_Sym*)(base_ptr + offset) : (Elf64_Sym*)offset;
        }
        else if (dyn_section[i].d_tag == DT_SYMENT) {
            lib->syment_size = dyn_section[i].d_val;
        }
    }

    return (void*)lib;
}

void* sys_get_proc_address(void* lib_handle, const char* proc_name) {
    LinuxDynamicLib* lib = (LinuxDynamicLib*)lib_handle;
    if (!lib || !lib->symtab || !lib->strtab || !proc_name) return NULL;

    // Walk the symbol table until we hit the string table boundary
    unsigned char* sym_ptr = (unsigned char*)lib->symtab;
    while (sym_ptr < (unsigned char*)lib->strtab) {
        Elf64_Sym* sym = (Elf64_Sym*)sym_ptr;
        if (sym->st_name != 0 && sym->st_value != 0) {
            const char* name = lib->strtab + sym->st_name;
            if (local_strcmp(name, proc_name) == 0) {
                return (void*)(lib->base_addr + sym->st_value);
            }
        }
        sym_ptr += lib->syment_size;
    }

    return NULL;
}

void sys_free_library(void* lib_handle) {
    LinuxDynamicLib* lib = (LinuxDynamicLib*)lib_handle;
    if (lib) {
        if (lib->base_addr) {
            sys_call(SYS_munmap, (long long)lib->base_addr, lib->map_size, 0, 0, 0, 0);
        }
        sys_memory(lib, 0);
    }
}
