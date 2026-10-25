#include "platform.h"


#define SYSTEM_CALLBACK __stdcall

typedef HANDLE(*FN_GetStdHandle)(DWORD);
typedef BOOL(*FN_WriteFile)(HANDLE, const void*, DWORD, DWORD*, void*);
typedef HANDLE(*FN_GetProcessHeap)(void);
typedef void* (*FN_HeapAlloc)(HANDLE, DWORD, SIZE_T);
typedef void* (*FN_HeapReAlloc)(HANDLE, DWORD, void*, SIZE_T);
typedef BOOL(*FN_HeapFree)(HANDLE, DWORD, void*);
typedef void(*FN_ExitProcess)(UINT);

#define STD_OUTPUT_HANDLE ((DWORD)-11)
#define NULL ((void*)0)


static FN_GetStdHandle pGetStdHandle = NULL;
static FN_WriteFile    pWriteFile = NULL;
static FN_GetProcessHeap pGetProcessHeap = NULL;
static FN_HeapAlloc    pHeapAlloc = NULL;
static FN_HeapReAlloc pHeapReAlloc = NULL;
static FN_HeapFree     pHeapFree = NULL;
static FN_ExitProcess  pExitProcess = NULL;


static int local_strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

void sys_init() {
    unsigned char* peb;
    // Read the PEB pointer from the GS register (64-bit Windows)
    __asm__("mov %%gs:0x60, %0" : "=r" (peb));

    // PEB->Ldr
    unsigned char* ldr = *(unsigned char**)(peb + 0x18);
    // Ldr->InMemoryOrderModuleList (Head)
    unsigned char* list_head = ldr + 0x20;
    unsigned char* current_link = *(unsigned char**)list_head;

    unsigned char* kernel32_base = NULL;

    // Loop through loaded modules. Typically, the 3rd or 4th module is kernel32.dll
    while (current_link != list_head) {
        // InMemoryOrderModuleList links point to the middle of LDR_DATA_TABLE_ENTRY
        unsigned char* module_entry = current_link - 0x10;
        unsigned char* base_address = *(unsigned char**)(module_entry + 0x30);
        void* dll_name_ptr = *(void**)(module_entry + 0x58); // FullDllName.Buffer

        if (base_address != NULL && dll_name_ptr != NULL) {
            // Check the MZ signature and PE signature to ensure it's a valid PE header
            unsigned int pe_offset = *(unsigned int*)(base_address + 0x3C);
            unsigned char* pe_header = base_address + pe_offset;

            // Look specifically for the Export Directory (Data Directory index 0)
            unsigned int export_rva = *(unsigned int*)(pe_header + 0x88);
            if (export_rva != 0) {
                unsigned char* exports = base_address + export_rva;
                unsigned int name_rva = *(unsigned int*)(exports + 0x20);
                if (name_rva != 0) {
                    const char* dll_name = (const char*)(base_address + name_rva);
                    // Match "KERNEL32.dll" or "kernel32.dll"
                    if (dll_name[0] == 'K' || dll_name[0] == 'k') {
                        kernel32_base = base_address;
                        break;
                    }
                }
            }
        }
        current_link = *(unsigned char**)current_link;
    }

    if (!kernel32_base) return;

    // Parse Kernel32's Export Address Table (EAT)
    unsigned int pe_offset = *(unsigned int*)(kernel32_base + 0x3C);
    unsigned char* exports = kernel32_base + *(unsigned int*)(kernel32_base + pe_offset + 0x88);

    unsigned int num_names = *(unsigned int*)(exports + 0x18);
    unsigned int funcs_rva = *(unsigned int*)(exports + 0x1C);
    unsigned int names_rva = *(unsigned int*)(exports + 0x20);
    unsigned int ords_rva = *(unsigned int*)(exports + 0x24);

    unsigned int* names = (unsigned int*)(kernel32_base + names_rva);
    unsigned int* funcs = (unsigned int*)(kernel32_base + funcs_rva);
    unsigned short* ords = (unsigned short*)(kernel32_base + ords_rva);

    // Resolve functions by string name matching
    for (unsigned int i = 0; i < num_names; i++) {
        const char* name = (const char*)(kernel32_base + names[i]);
        unsigned int func_rva = funcs[ords[i]];
        void* func_ptr = (void*)(kernel32_base + func_rva);

        if (local_strcmp(name, "GetStdHandle") == 0)      pGetStdHandle = (FN_GetStdHandle)func_ptr;
        else if (local_strcmp(name, "WriteFile") == 0)    pWriteFile = (FN_WriteFile)func_ptr;
        else if (local_strcmp(name, "GetProcessHeap") == 0) pGetProcessHeap = (FN_GetProcessHeap)func_ptr;
        else if (local_strcmp(name, "HeapAlloc") == 0)    pHeapAlloc = (FN_HeapAlloc)func_ptr;
        else if (local_strcmp(name, "HeapReAlloc") == 0)  pHeapReAlloc = (FN_HeapReAlloc)func_ptr;
        else if (local_strcmp(name, "HeapFree") == 0)     pHeapFree = (FN_HeapFree)func_ptr;
        else if (local_strcmp(name, "ExitProcess") == 0)  pExitProcess = (FN_ExitProcess)func_ptr;
    }
}

void sys_exit(UINT exit_code) {
    if (pExitProcess) pExitProcess(exit_code);
}

void sys_print(const char* buffer, DWORD length) {
    if (pGetStdHandle && pWriteFile && buffer && length > 0) {
        HANDLE stdout_handle = pGetStdHandle(STD_OUTPUT_HANDLE);
        DWORD bytes_written;
        pWriteFile(stdout_handle, buffer, length, &bytes_written, NULL);
    }
}

void* sys_memory(void* ptr, SIZE_T new_size) {
    if (!pGetProcessHeap || !pHeapAlloc) return NULL;
    HANDLE heap = pGetProcessHeap();

    if (new_size == 0) {
        if (ptr && pHeapFree) pHeapFree(heap, 0, ptr);
        return NULL;
    }
    if (ptr == NULL) {
        return pHeapAlloc(heap, 0, new_size);
    }
    return pHeapReAlloc ? pHeapReAlloc(heap, 0, ptr, new_size) : NULL;
}
