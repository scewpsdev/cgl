# 1 "<string>"
# 1 "<command line>" 1
# 14 "<command line>"
# 1 "D:/Dev/2023/CGL/cgl-bindgen/src/tcc/tccdefs.h" 1
# 14 "<command line>" 2
# 1 "<string>" 2

# 1 "SDL3/SDL.h" 1
# 35 "SDL3/SDL.h"
# 1 "./SDL3/SDL_stdinc.h" 1
# 50 "./SDL3/SDL_stdinc.h"
# 1 "./SDL3/SDL_platform_defines.h" 1
# 50 "./SDL3/SDL_stdinc.h" 2


# 1 "D:/Dev/2023/CGL/cgl-bindgen/lib/stubs/stdarg.h" 1
# 52 "./SDL3/SDL_stdinc.h" 2

# 1 "D:/Dev/2023/CGL/cgl-bindgen/lib/stubs/string.h" 1
# 53 "./SDL3/SDL_stdinc.h" 2

# 1 "D:/Dev/2023/CGL/cgl-bindgen/lib/stubs/wchar.h" 1
# 54 "./SDL3/SDL_stdinc.h" 2


# 1 "./SDL3/SDL_begin_code.h" 1
# 56 "./SDL3/SDL_stdinc.h" 2
# 83 "./SDL3/SDL_stdinc.h"
# 1 "D:/Dev/2023/CGL/cgl-bindgen/lib/stubs/stdint.h" 1
# 83 "./SDL3/SDL_stdinc.h" 2





# 1 "D:/Dev/2023/CGL/cgl-bindgen/lib/stubs/inttypes.h" 1
# 88 "./SDL3/SDL_stdinc.h" 2
# 100 "./SDL3/SDL_stdinc.h"
# 1 "D:/Dev/2023/CGL/cgl-bindgen/lib/stubs/stdbool.h" 1
# 100 "./SDL3/SDL_stdinc.h" 2
# 134 "./SDL3/SDL_stdinc.h"
void *alloca(uint64);
# 450 "./SDL3/SDL_stdinc.h"
typedef char Sint8;
# 459 "./SDL3/SDL_stdinc.h"
typedef byte Uint8;
# 468 "./SDL3/SDL_stdinc.h"
typedef short Sint16;
# 477 "./SDL3/SDL_stdinc.h"
typedef ushort Uint16;
# 486 "./SDL3/SDL_stdinc.h"
typedef int Sint32;
# 495 "./SDL3/SDL_stdinc.h"
typedef uint Uint32;
# 506 "./SDL3/SDL_stdinc.h"
typedef long Sint64;
# 517 "./SDL3/SDL_stdinc.h"
typedef ulong Uint64;
# 534 "./SDL3/SDL_stdinc.h"
typedef Sint64 SDL_Time;
# 817 "./SDL3/SDL_stdinc.h"
typedef int SDL_compile_time_assert_longlong_size64[(sizeof(long long) == 8) * 2 - 1];
# 1182 "./SDL3/SDL_stdinc.h"
typedef int SDL_compile_time_assert_bool_size[(sizeof(bool) == 1) * 2 - 1];
typedef int SDL_compile_time_assert_uint8_size[(sizeof(Uint8) == 1) * 2 - 1];
typedef int SDL_compile_time_assert_sint8_size[(sizeof(Sint8) == 1) * 2 - 1];
typedef int SDL_compile_time_assert_uint16_size[(sizeof(Uint16) == 2) * 2 - 1];
typedef int SDL_compile_time_assert_sint16_size[(sizeof(Sint16) == 2) * 2 - 1];
typedef int SDL_compile_time_assert_uint32_size[(sizeof(Uint32) == 4) * 2 - 1];
typedef int SDL_compile_time_assert_sint32_size[(sizeof(Sint32) == 4) * 2 - 1];
typedef int SDL_compile_time_assert_uint64_size[(sizeof(Uint64) == 8) * 2 - 1];
typedef int SDL_compile_time_assert_sint64_size[(sizeof(Sint64) == 8) * 2 - 1];

typedef int SDL_compile_time_assert_uint64_longlong[(sizeof(Uint64) <= sizeof(unsigned long long)) * 2 - 1];
typedef int SDL_compile_time_assert_size_t_longlong[(sizeof(uint64) <= sizeof(unsigned long long)) * 2 - 1];

typedef struct SDL_alignment_test
{
    Uint8 a;
    void *b;
} SDL_alignment_test;
typedef int SDL_compile_time_assert_struct_alignment[(sizeof(SDL_alignment_test) == (2 * sizeof(void *))) * 2 - 1];
typedef int SDL_compile_time_assert_two_s_complement[(((int)(~((int)(0)))) == ((int)(-1))) * 2 - 1];
# 1215 "./SDL3/SDL_stdinc.h"
typedef enum SDL_DUMMY_ENUM
{
    DUMMY_ENUM_VALUE
} SDL_DUMMY_ENUM;

typedef int SDL_compile_time_assert_enum[(sizeof(SDL_DUMMY_ENUM) == sizeof(int)) * 2 - 1];
# 1354 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport))  void *  SDL_malloc(uint64 size);
# 1379 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport))   void *  SDL_calloc(uint64 nmemb, uint64 size);
# 1419 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport))  void *  SDL_realloc(void *mem, uint64 size);
# 1439 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) void  SDL_free(void *mem);
# 1458 "./SDL3/SDL_stdinc.h"
typedef void *( *SDL_malloc_func)(uint64 size);
# 1479 "./SDL3/SDL_stdinc.h"
typedef void *( *SDL_calloc_func)(uint64 nmemb, uint64 size);
# 1500 "./SDL3/SDL_stdinc.h"
typedef void *( *SDL_realloc_func)(void *mem, uint64 size);
# 1518 "./SDL3/SDL_stdinc.h"
typedef void ( *SDL_free_func)(void *mem);
# 1537 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) void  SDL_GetOriginalMemoryFunctions(SDL_malloc_func *malloc_func,
                                                            SDL_calloc_func *calloc_func,
                                                            SDL_realloc_func *realloc_func,
                                                            SDL_free_func *free_func);
# 1559 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) void  SDL_GetMemoryFunctions(SDL_malloc_func *malloc_func,
                                                    SDL_calloc_func *calloc_func,
                                                    SDL_realloc_func *realloc_func,
                                                    SDL_free_func *free_func);
# 1590 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) bool  SDL_SetMemoryFunctions(SDL_malloc_func malloc_func,
                                                            SDL_calloc_func calloc_func,
                                                            SDL_realloc_func realloc_func,
                                                            SDL_free_func free_func);
# 1617 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport))  void *  SDL_aligned_alloc(uint64 alignment, uint64 size);
# 1635 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) void  SDL_aligned_free(void *mem);
# 1647 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_GetNumAllocations(void);
# 1662 "./SDL3/SDL_stdinc.h"
typedef struct SDL_Environment SDL_Environment;
# 1685 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) SDL_Environment *  SDL_GetEnvironment(void);
# 1707 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) SDL_Environment *  SDL_CreateEnvironment(bool populated);
# 1727 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) const char *  SDL_GetEnvironmentVariable(SDL_Environment *env, const char *name);
# 1748 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) char **  SDL_GetEnvironmentVariables(SDL_Environment *env);
# 1772 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) bool  SDL_SetEnvironmentVariable(SDL_Environment *env, const char *name, const char *value, bool overwrite);
# 1793 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) bool  SDL_UnsetEnvironmentVariable(SDL_Environment *env, const char *name);
# 1807 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) void  SDL_DestroyEnvironment(SDL_Environment *env);
# 1824 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) const char *  SDL_getenv(const char *name);
# 1848 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) const char *  SDL_getenv_unsafe(const char *name);
# 1866 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_setenv_unsafe(const char *name, const char *value, int overwrite);
# 1881 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_unsetenv_unsafe(const char *name);
# 1897 "./SDL3/SDL_stdinc.h"
typedef int ( *SDL_CompareCallback)(const void *a, const void *b);
# 1943 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) void  SDL_qsort(void *base, uint64 nmemb, uint64 size, SDL_CompareCallback compare);
# 1993 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) void *  SDL_bsearch(const void *key, const void *base, uint64 nmemb, uint64 size, SDL_CompareCallback compare);
# 2010 "./SDL3/SDL_stdinc.h"
typedef int ( *SDL_CompareCallback_r)(void *userdata, const void *a, const void *b);
# 2063 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) void  SDL_qsort_r(void *base, uint64 nmemb, uint64 size, SDL_CompareCallback_r compare, void *userdata);
# 2121 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) void *  SDL_bsearch_r(const void *key, const void *base, uint64 nmemb, uint64 size, SDL_CompareCallback_r compare, void *userdata);
# 2133 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_abs(int x);
# 2208 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_isalpha(int x);
# 2223 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_isalnum(int x);
# 2238 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_isblank(int x);
# 2253 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_iscntrl(int x);
# 2268 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_isdigit(int x);
# 2283 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_isxdigit(int x);
# 2301 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_ispunct(int x);
# 2323 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_isspace(int x);
# 2338 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_isupper(int x);
# 2353 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_islower(int x);
# 2372 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_isprint(int x);
# 2393 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_isgraph(int x);
# 2411 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_toupper(int x);
# 2429 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_tolower(int x);
# 2450 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) Uint16  SDL_crc16(Uint16 crc, const void *data, uint64 len);
# 2471 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) Uint32  SDL_crc32(Uint32 crc, const void *data, uint64 len);
# 2497 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) Uint32  SDL_murmur3_32(const void *data, uint64 len, Uint32 seed);
# 2517 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) void *  SDL_memcpy( void *dst,  const void *src, uint64 len);
# 2573 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) void *  SDL_memmove( void *dst,  const void *src, uint64 len);
# 2601 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) void *  SDL_memset( void *dst, int c, uint64 len);
# 2621 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) void *  SDL_memset4(void *dst, Uint32 val, uint64 dwords);
# 2703 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_memcmp(const void *s1, const void *s2, uint64 len);
# 2730 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) uint64  SDL_wcslen(const int16 *wstr);
# 2761 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) uint64  SDL_wcsnlen(const int16 *wstr, uint64 maxlen);
# 2788 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) uint64  SDL_wcslcpy( int16 *dst, const int16 *src, uint64 maxlen);
# 2817 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) uint64  SDL_wcslcat( int16 *dst, const int16 *src, uint64 maxlen);
# 2835 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int16 *  SDL_wcsdup(const int16 *wstr);
# 2855 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int16 *  SDL_wcsstr(const int16 *haystack, const int16 *needle);
# 2880 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int16 *  SDL_wcsnstr(const int16 *haystack, const int16 *needle, uint64 maxlen);
# 2899 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_wcscmp(const int16 *str1, const int16 *str2);
# 2930 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_wcsncmp(const int16 *str1, const int16 *str2, uint64 maxlen);
# 2960 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_wcscasecmp(const int16 *str1, const int16 *str2);
# 3002 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_wcsncasecmp(const int16 *str1, const int16 *str2, uint64 maxlen);
# 3029 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) long  SDL_wcstol(const int16 *str, int16 **endp, int base);
# 3049 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) uint64  SDL_strlen(const char *str);
# 3073 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) uint64  SDL_strnlen(const char *str, uint64 maxlen);
# 3102 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) uint64  SDL_strlcpy( char *dst, const char *src, uint64 maxlen);
# 3130 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) uint64  SDL_utf8strlcpy( char *dst, const char *src, uint64 dst_bytes);
# 3158 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) uint64  SDL_strlcat( char *dst, const char *src, uint64 maxlen);
# 3176 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport))  char *  SDL_strdup(const char *str);
# 3201 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport))  char *  SDL_strndup(const char *str, uint64 maxlen);
# 3222 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) char *  SDL_strrev(char *str);
# 3243 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) char *  SDL_strupr(char *str);
# 3264 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) char *  SDL_strlwr(char *str);
# 3284 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) char *  SDL_strchr(const char *str, int c);
# 3303 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) char *  SDL_strrchr(const char *str, int c);
# 3323 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) char *  SDL_strstr(const char *haystack, const char *needle);
# 3346 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) char *  SDL_strnstr(const char *haystack, const char *needle, uint64 maxlen);
# 3374 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) char *  SDL_strcasestr(const char *haystack, const char *needle);
# 3403 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) char *  SDL_strtok_r(char *str, const char *delim, char **saveptr);
# 3431 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) uint64  SDL_utf8strlen(const char *str);
# 3464 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) uint64  SDL_utf8strnlen(const char *str, uint64 bytes);
# 3492 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) char *  SDL_itoa(int value, char *str, int radix);
# 3520 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) char *  SDL_uitoa(unsigned int value, char *str, int radix);
# 3548 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) char *  SDL_ltoa(long value, char *str, int radix);
# 3576 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) char *  SDL_ultoa(unsigned long value, char *str, int radix);
# 3606 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) char *  SDL_lltoa(long long value, char *str, int radix);
# 3634 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) char *  SDL_ulltoa(unsigned long long value, char *str, int radix);
# 3658 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_atoi(const char *str);
# 3680 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) double  SDL_atof(const char *str);
# 3714 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) long  SDL_strtol(const char *str, char **endp, int base);
# 3747 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) unsigned long  SDL_strtoul(const char *str, char **endp, int base);
# 3782 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) long long  SDL_strtoll(const char *str, char **endp, int base);
# 3816 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) unsigned long long  SDL_strtoull(const char *str, char **endp, int base);
# 3846 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) double  SDL_strtod(const char *str, char **endp);
# 3866 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_strcmp(const char *str1, const char *str2);
# 3896 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_strncmp(const char *str1, const char *str2, uint64 maxlen);
# 3924 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_strcasecmp(const char *str1, const char *str2);
# 3964 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_strncasecmp(const char *str1, const char *str2, uint64 maxlen);
# 3982 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) char *  SDL_strpbrk(const char *str, const char *breakset);
# 4042 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) Uint32  SDL_StepUTF8(const char **pstr, uint64 *pslen);
# 4073 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) Uint32  SDL_StepBackUTF8(const char *start, const char **pstr);
# 4102 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) char *  SDL_UCS4ToUTF8(Uint32 codepoint, char *dst);
# 4119 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_sscanf(const char *text,  const char *fmt, ...) ;
# 4138 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_vsscanf(const char *text,  const char *fmt, byte* ap) ;
# 4171 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_snprintf( char *text, uint64 maxlen,  const char *fmt, ...) ;
# 4205 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_swprintf( int16 *text, uint64 maxlen,  const int16 *fmt, ...) ;
# 4225 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_vsnprintf( char *text, uint64 maxlen,  const char *fmt, byte* ap) ;
# 4246 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_vswprintf( int16 *text, uint64 maxlen,  const int16 *fmt, byte* ap) ;
# 4275 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_asprintf(char **strp,  const char *fmt, ...) ;
# 4294 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_vasprintf(char **strp,  const char *fmt, byte* ap) ;
# 4314 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) void  SDL_srand(Uint64 seed);
# 4348 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) Sint32  SDL_rand(Sint32 n);
# 4371 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) float  SDL_randf(void);
# 4394 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) Uint32  SDL_rand_bits(void);
# 4429 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) Sint32  SDL_rand_r(Uint64 *state, Sint32 n);
# 4453 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) float  SDL_randf_r(Uint64 *state);
# 4478 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) Uint32  SDL_rand_bits_r(Uint64 *state);
# 4532 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) double  SDL_acos(double x);
# 4562 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) float  SDL_acosf(float x);
# 4592 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) double  SDL_asin(double x);
# 4622 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) float  SDL_asinf(float x);
# 4654 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) double  SDL_atan(double x);
# 4686 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) float  SDL_atanf(float x);
# 4722 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) double  SDL_atan2(double y, double x);
# 4758 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) float  SDL_atan2f(float y, float x);
# 4786 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) double  SDL_ceil(double x);
# 4814 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) float  SDL_ceilf(float x);
# 4840 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) double  SDL_copysign(double x, double y);
# 4866 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) float  SDL_copysignf(float x, float y);
# 4894 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) double  SDL_cos(double x);
# 4922 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) float  SDL_cosf(float x);
# 4954 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) double  SDL_exp(double x);
# 4986 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) float  SDL_expf(float x);
# 5007 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) double  SDL_fabs(double x);
# 5028 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) float  SDL_fabsf(float x);
# 5056 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) double  SDL_floor(double x);
# 5084 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) float  SDL_floorf(float x);
# 5113 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) double  SDL_trunc(double x);
# 5142 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) float  SDL_truncf(float x);
# 5172 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) double  SDL_fmod(double x, double y);
# 5202 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) float  SDL_fmodf(float x, float y);
# 5216 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_isinf(double x);
# 5230 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_isinff(float x);
# 5244 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_isnan(double x);
# 5258 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_isnanf(float x);
# 5288 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) double  SDL_log(double x);
# 5317 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) float  SDL_logf(float x);
# 5347 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) double  SDL_log10(double x);
# 5377 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) float  SDL_log10f(float x);
# 5397 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) double  SDL_modf(double x, double *y);
# 5417 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) float  SDL_modff(float x, float *y);
# 5449 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) double  SDL_pow(double x, double y);
# 5481 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) float  SDL_powf(float x, float y);
# 5510 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) double  SDL_round(double x);
# 5539 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) float  SDL_roundf(float x);
# 5568 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) long  SDL_lround(double x);
# 5597 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) long  SDL_lroundf(float x);
# 5622 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) double  SDL_scalbn(double x, int n);
# 5647 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) float  SDL_scalbnf(float x, int n);
# 5675 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) double  SDL_sin(double x);
# 5703 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) float  SDL_sinf(float x);
# 5729 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) double  SDL_sqrt(double x);
# 5755 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) float  SDL_sqrtf(float x);
# 5785 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) double  SDL_tan(double x);
# 5815 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) float  SDL_tanf(float x);
# 5824 "./SDL3/SDL_stdinc.h"
typedef struct SDL_iconv_data_t *SDL_iconv_t;
# 5843 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) SDL_iconv_t  SDL_iconv_open(const char *tocode,
                                                   const char *fromcode);
# 5860 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) int  SDL_iconv_close(SDL_iconv_t cd);
# 5900 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) uint64  SDL_iconv(SDL_iconv_t cd, const char **inbuf,
                                         uint64 *inbytesleft, char **outbuf,
                                         uint64 *outbytesleft);
# 5937 "./SDL3/SDL_stdinc.h"
extern __attribute__((dllimport)) char *  SDL_iconv_string(const char *tocode,
                                               const char *fromcode,
                                               const char *inbuf,
                                               uint64 inbytesleft);
# 6104 "./SDL3/SDL_stdinc.h"
static inline bool SDL_size_mul_check_overflow(uint64 a, uint64 b, uint64 *ret)
{
    if (a != 0 && b > ((uint64) -1) / a) {
        return false;
    }
    *ret = a * b;
    return true;
}
# 6143 "./SDL3/SDL_stdinc.h"
static inline bool SDL_size_add_check_overflow(uint64 a, uint64 b, uint64 *ret)
{
    if (b > ((uint64) -1) - a) {
        return false;
    }
    *ret = a + b;
    return true;
}
# 6186 "./SDL3/SDL_stdinc.h"
typedef void (*SDL_FunctionPointer)(void);







# 1 "./SDL3/SDL_close_code.h" 1
# 6194 "./SDL3/SDL_stdinc.h" 2
# 35 "SDL3/SDL.h" 2

# 1 "./SDL3/SDL_assert.h" 1
# 70 "./SDL3/SDL_assert.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 70 "./SDL3/SDL_assert.h" 2
# 306 "./SDL3/SDL_assert.h"
typedef enum SDL_AssertState
{
    SDL_ASSERTION_RETRY,
    SDL_ASSERTION_BREAK,
    SDL_ASSERTION_ABORT,
    SDL_ASSERTION_IGNORE,
    SDL_ASSERTION_ALWAYS_IGNORE
} SDL_AssertState;
# 324 "./SDL3/SDL_assert.h"
typedef struct SDL_AssertData
{
    bool always_ignore;
    unsigned int trigger_count;
    const char *condition;
    const char *filename;
    int linenum;
    const char *function;
    const struct SDL_AssertData *next;
} SDL_AssertData;
# 350 "./SDL3/SDL_assert.h"
extern __attribute__((dllimport)) SDL_AssertState  SDL_ReportAssertion(SDL_AssertData *data,
                                                            const char *func,
                                                            const char *file, int line) ;
# 565 "./SDL3/SDL_assert.h"
typedef SDL_AssertState ( *SDL_AssertionHandler)(
                                 const SDL_AssertData *data, void *userdata);
# 591 "./SDL3/SDL_assert.h"
extern __attribute__((dllimport)) void  SDL_SetAssertionHandler(
                                            SDL_AssertionHandler handler,
                                            void *userdata);
# 612 "./SDL3/SDL_assert.h"
extern __attribute__((dllimport)) SDL_AssertionHandler  SDL_GetDefaultAssertionHandler(void);
# 637 "./SDL3/SDL_assert.h"
extern __attribute__((dllimport)) SDL_AssertionHandler  SDL_GetAssertionHandler(void **puserdata);
# 671 "./SDL3/SDL_assert.h"
extern __attribute__((dllimport)) const SDL_AssertData *  SDL_GetAssertionReport(void);
# 689 "./SDL3/SDL_assert.h"
extern __attribute__((dllimport)) void  SDL_ResetAssertionReport(void);






# 1 "./SDL3/SDL_close_code.h" 1
# 696 "./SDL3/SDL_assert.h" 2
# 36 "SDL3/SDL.h" 2

# 1 "./SDL3/SDL_asyncio.h" 1
# 109 "./SDL3/SDL_asyncio.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 109 "./SDL3/SDL_asyncio.h" 2
# 124 "./SDL3/SDL_asyncio.h"
typedef struct SDL_AsyncIO SDL_AsyncIO;






typedef enum SDL_AsyncIOTaskType
{
    SDL_ASYNCIO_TASK_READ,
    SDL_ASYNCIO_TASK_WRITE,
    SDL_ASYNCIO_TASK_CLOSE
} SDL_AsyncIOTaskType;






typedef enum SDL_AsyncIOResult
{
    SDL_ASYNCIO_COMPLETE,
    SDL_ASYNCIO_FAILURE,
    SDL_ASYNCIO_CANCELED
} SDL_AsyncIOResult;






typedef struct SDL_AsyncIOOutcome
{
    SDL_AsyncIO *asyncio;
    SDL_AsyncIOTaskType type;
    SDL_AsyncIOResult result;
    void *buffer;
    Uint64 offset;
    Uint64 bytes_requested;
    Uint64 bytes_transferred;
    void *userdata;
} SDL_AsyncIOOutcome;
# 183 "./SDL3/SDL_asyncio.h"
typedef struct SDL_AsyncIOQueue SDL_AsyncIOQueue;
# 224 "./SDL3/SDL_asyncio.h"
extern __attribute__((dllimport)) SDL_AsyncIO *  SDL_AsyncIOFromFile(const char *file, const char *mode);
# 241 "./SDL3/SDL_asyncio.h"
extern __attribute__((dllimport)) Sint64  SDL_GetAsyncIOSize(SDL_AsyncIO *asyncio);
# 279 "./SDL3/SDL_asyncio.h"
extern __attribute__((dllimport)) bool  SDL_ReadAsyncIO(SDL_AsyncIO *asyncio, void *ptr, Uint64 offset, Uint64 size, SDL_AsyncIOQueue *queue, void *userdata);
# 316 "./SDL3/SDL_asyncio.h"
extern __attribute__((dllimport)) bool  SDL_WriteAsyncIO(SDL_AsyncIO *asyncio, void *ptr, Uint64 offset, Uint64 size, SDL_AsyncIOQueue *queue, void *userdata);
# 365 "./SDL3/SDL_asyncio.h"
extern __attribute__((dllimport)) bool  SDL_CloseAsyncIO(SDL_AsyncIO *asyncio, bool flush, SDL_AsyncIOQueue *queue, void *userdata);
# 384 "./SDL3/SDL_asyncio.h"
extern __attribute__((dllimport)) SDL_AsyncIOQueue *  SDL_CreateAsyncIOQueue(void);
# 414 "./SDL3/SDL_asyncio.h"
extern __attribute__((dllimport)) void  SDL_DestroyAsyncIOQueue(SDL_AsyncIOQueue *queue);
# 440 "./SDL3/SDL_asyncio.h"
extern __attribute__((dllimport)) bool  SDL_GetAsyncIOResult(SDL_AsyncIOQueue *queue, SDL_AsyncIOOutcome *outcome);
# 484 "./SDL3/SDL_asyncio.h"
extern __attribute__((dllimport)) bool  SDL_WaitAsyncIOResult(SDL_AsyncIOQueue *queue, SDL_AsyncIOOutcome *outcome, Sint32 timeoutMS);
# 508 "./SDL3/SDL_asyncio.h"
extern __attribute__((dllimport)) void  SDL_SignalAsyncIOQueue(SDL_AsyncIOQueue *queue);
# 542 "./SDL3/SDL_asyncio.h"
extern __attribute__((dllimport)) bool  SDL_LoadFileAsync(const char *file, SDL_AsyncIOQueue *queue, void *userdata);






# 1 "./SDL3/SDL_close_code.h" 1
# 549 "./SDL3/SDL_asyncio.h" 2
# 37 "SDL3/SDL.h" 2

# 1 "./SDL3/SDL_atomic.h" 1
# 57 "./SDL3/SDL_atomic.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 57 "./SDL3/SDL_atomic.h" 2
# 82 "./SDL3/SDL_atomic.h"
typedef int SDL_SpinLock;
# 100 "./SDL3/SDL_atomic.h"
extern __attribute__((dllimport)) bool  SDL_TryLockSpinlock(SDL_SpinLock *lock);
# 117 "./SDL3/SDL_atomic.h"
extern __attribute__((dllimport)) void  SDL_LockSpinlock(SDL_SpinLock *lock);
# 136 "./SDL3/SDL_atomic.h"
extern __attribute__((dllimport)) void  SDL_UnlockSpinlock(SDL_SpinLock *lock);
# 195 "./SDL3/SDL_atomic.h"
extern __attribute__((dllimport)) void  SDL_MemoryBarrierReleaseFunction(void);
# 215 "./SDL3/SDL_atomic.h"
extern __attribute__((dllimport)) void  SDL_MemoryBarrierAcquireFunction(void);
# 405 "./SDL3/SDL_atomic.h"
typedef struct SDL_AtomicInt { int value; } SDL_AtomicInt;
# 425 "./SDL3/SDL_atomic.h"
extern __attribute__((dllimport)) bool  SDL_CompareAndSwapAtomicInt(SDL_AtomicInt *a, int oldval, int newval);
# 445 "./SDL3/SDL_atomic.h"
extern __attribute__((dllimport)) int  SDL_SetAtomicInt(SDL_AtomicInt *a, int v);
# 462 "./SDL3/SDL_atomic.h"
extern __attribute__((dllimport)) int  SDL_GetAtomicInt(SDL_AtomicInt *a);
# 483 "./SDL3/SDL_atomic.h"
extern __attribute__((dllimport)) int  SDL_AddAtomicInt(SDL_AtomicInt *a, int v);
# 550 "./SDL3/SDL_atomic.h"
typedef struct SDL_AtomicU32 { Uint32 value; } SDL_AtomicU32;
# 570 "./SDL3/SDL_atomic.h"
extern __attribute__((dllimport)) bool  SDL_CompareAndSwapAtomicU32(SDL_AtomicU32 *a, Uint32 oldval, Uint32 newval);
# 590 "./SDL3/SDL_atomic.h"
extern __attribute__((dllimport)) Uint32  SDL_SetAtomicU32(SDL_AtomicU32 *a, Uint32 v);
# 607 "./SDL3/SDL_atomic.h"
extern __attribute__((dllimport)) Uint32  SDL_GetAtomicU32(SDL_AtomicU32 *a);
# 625 "./SDL3/SDL_atomic.h"
extern __attribute__((dllimport)) Uint32  SDL_AddAtomicU32(SDL_AtomicU32 *a, int v);
# 646 "./SDL3/SDL_atomic.h"
extern __attribute__((dllimport)) bool  SDL_CompareAndSwapAtomicPointer(void **a, void *oldval, void *newval);
# 665 "./SDL3/SDL_atomic.h"
extern __attribute__((dllimport)) void *  SDL_SetAtomicPointer(void **a, void *v);
# 683 "./SDL3/SDL_atomic.h"
extern __attribute__((dllimport)) void *  SDL_GetAtomicPointer(void **a);







# 1 "./SDL3/SDL_close_code.h" 1
# 691 "./SDL3/SDL_atomic.h" 2
# 38 "SDL3/SDL.h" 2

# 1 "./SDL3/SDL_audio.h" 1
# 133 "./SDL3/SDL_audio.h"
# 1 "./SDL3/SDL_endian.h" 1
# 216 "./SDL3/SDL_endian.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 216 "./SDL3/SDL_endian.h" 2
# 253 "./SDL3/SDL_endian.h"
static inline Uint16 SDL_Swap16(Uint16 x)
{
  __asm__("xchgb %b0,%h0": "=abcd"(x):"0"(x));
    return x;
}
# 300 "./SDL3/SDL_endian.h"
static inline Uint32 SDL_Swap32(Uint32 x)
{
  __asm__("bswapl %0": "=r"(x):"0"(x));
    return x;
}
# 359 "./SDL3/SDL_endian.h"
static inline Uint64 SDL_Swap64(Uint64 x)
{
  __asm__("bswapq %0": "=r"(x):"0"(x));
    return x;
}
# 408 "./SDL3/SDL_endian.h"
static inline float SDL_SwapFloat(float x)
{
    union {
        float f;
        Uint32 ui32;
    } swapper;
    swapper.f = x;
    swapper.ui32 = SDL_Swap32(swapper.ui32);
    return swapper.f;
}
# 644 "./SDL3/SDL_endian.h"
# 1 "./SDL3/SDL_close_code.h" 1
# 644 "./SDL3/SDL_endian.h" 2
# 133 "./SDL3/SDL_audio.h" 2

# 1 "./SDL3/SDL_error.h" 1
# 54 "./SDL3/SDL_error.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 54 "./SDL3/SDL_error.h" 2
# 89 "./SDL3/SDL_error.h"
extern __attribute__((dllimport)) bool  SDL_SetError( const char *fmt, ...) ;
# 108 "./SDL3/SDL_error.h"
extern __attribute__((dllimport)) bool  SDL_SetErrorV( const char *fmt, byte* ap) ;
# 121 "./SDL3/SDL_error.h"
extern __attribute__((dllimport)) bool  SDL_OutOfMemory(void);
# 158 "./SDL3/SDL_error.h"
extern __attribute__((dllimport)) const char *  SDL_GetError(void);
# 172 "./SDL3/SDL_error.h"
extern __attribute__((dllimport)) bool  SDL_ClearError(void);
# 225 "./SDL3/SDL_error.h"
# 1 "./SDL3/SDL_close_code.h" 1
# 225 "./SDL3/SDL_error.h" 2
# 134 "./SDL3/SDL_audio.h" 2

# 1 "./SDL3/SDL_mutex.h" 1
# 47 "./SDL3/SDL_mutex.h"
# 1 "./SDL3/SDL_thread.h" 1
# 47 "./SDL3/SDL_thread.h"
# 1 "./SDL3/SDL_properties.h" 1
# 56 "./SDL3/SDL_properties.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 56 "./SDL3/SDL_properties.h" 2
# 66 "./SDL3/SDL_properties.h"
typedef Uint32 SDL_PropertiesID;






typedef enum SDL_PropertyType
{
    SDL_PROPERTY_TYPE_INVALID,
    SDL_PROPERTY_TYPE_POINTER,
    SDL_PROPERTY_TYPE_STRING,
    SDL_PROPERTY_TYPE_NUMBER,
    SDL_PROPERTY_TYPE_FLOAT,
    SDL_PROPERTY_TYPE_BOOLEAN
} SDL_PropertyType;
# 118 "./SDL3/SDL_properties.h"
extern __attribute__((dllimport)) SDL_PropertiesID  SDL_GetGlobalProperties(void);
# 134 "./SDL3/SDL_properties.h"
extern __attribute__((dllimport)) SDL_PropertiesID  SDL_CreateProperties(void);
# 155 "./SDL3/SDL_properties.h"
extern __attribute__((dllimport)) bool  SDL_CopyProperties(SDL_PropertiesID src, SDL_PropertiesID dst);
# 179 "./SDL3/SDL_properties.h"
extern __attribute__((dllimport)) bool  SDL_LockProperties(SDL_PropertiesID props);
# 192 "./SDL3/SDL_properties.h"
extern __attribute__((dllimport)) void  SDL_UnlockProperties(SDL_PropertiesID props);
# 216 "./SDL3/SDL_properties.h"
typedef void ( *SDL_CleanupPropertyCallback)(void *userdata, void *value);
# 247 "./SDL3/SDL_properties.h"
extern __attribute__((dllimport)) bool  SDL_SetPointerPropertyWithCleanup(SDL_PropertiesID props, const char *name, void *value, SDL_CleanupPropertyCallback cleanup, void *userdata);
# 270 "./SDL3/SDL_properties.h"
extern __attribute__((dllimport)) bool  SDL_SetPointerProperty(SDL_PropertiesID props, const char *name, void *value);
# 290 "./SDL3/SDL_properties.h"
extern __attribute__((dllimport)) bool  SDL_SetStringProperty(SDL_PropertiesID props, const char *name, const char *value);
# 307 "./SDL3/SDL_properties.h"
extern __attribute__((dllimport)) bool  SDL_SetNumberProperty(SDL_PropertiesID props, const char *name, Sint64 value);
# 324 "./SDL3/SDL_properties.h"
extern __attribute__((dllimport)) bool  SDL_SetFloatProperty(SDL_PropertiesID props, const char *name, float value);
# 341 "./SDL3/SDL_properties.h"
extern __attribute__((dllimport)) bool  SDL_SetBooleanProperty(SDL_PropertiesID props, const char *name, bool value);
# 356 "./SDL3/SDL_properties.h"
extern __attribute__((dllimport)) bool  SDL_HasProperty(SDL_PropertiesID props, const char *name);
# 372 "./SDL3/SDL_properties.h"
extern __attribute__((dllimport)) SDL_PropertyType  SDL_GetPropertyType(SDL_PropertiesID props, const char *name);
# 405 "./SDL3/SDL_properties.h"
extern __attribute__((dllimport)) void *  SDL_GetPointerProperty(SDL_PropertiesID props, const char *name, void *default_value);
# 429 "./SDL3/SDL_properties.h"
extern __attribute__((dllimport)) const char *  SDL_GetStringProperty(SDL_PropertiesID props, const char *name, const char *default_value);
# 451 "./SDL3/SDL_properties.h"
extern __attribute__((dllimport)) Sint64  SDL_GetNumberProperty(SDL_PropertiesID props, const char *name, Sint64 default_value);
# 473 "./SDL3/SDL_properties.h"
extern __attribute__((dllimport)) float  SDL_GetFloatProperty(SDL_PropertiesID props, const char *name, float default_value);
# 495 "./SDL3/SDL_properties.h"
extern __attribute__((dllimport)) bool  SDL_GetBooleanProperty(SDL_PropertiesID props, const char *name, bool default_value);
# 509 "./SDL3/SDL_properties.h"
extern __attribute__((dllimport)) bool  SDL_ClearProperty(SDL_PropertiesID props, const char *name);
# 528 "./SDL3/SDL_properties.h"
typedef void ( *SDL_EnumeratePropertiesCallback)(void *userdata, SDL_PropertiesID props, const char *name);
# 546 "./SDL3/SDL_properties.h"
extern __attribute__((dllimport)) bool  SDL_EnumerateProperties(SDL_PropertiesID props, SDL_EnumeratePropertiesCallback callback, void *userdata);
# 564 "./SDL3/SDL_properties.h"
extern __attribute__((dllimport)) void  SDL_DestroyProperties(SDL_PropertiesID props);






# 1 "./SDL3/SDL_close_code.h" 1
# 571 "./SDL3/SDL_properties.h" 2
# 47 "./SDL3/SDL_thread.h" 2






# 1 "D:/Dev/2023/CGL/cgl-bindgen/lib/stubs/process.h" 1
# 53 "./SDL3/SDL_thread.h" 2



# 1 "./SDL3/SDL_begin_code.h" 1
# 56 "./SDL3/SDL_thread.h" 2
# 71 "./SDL3/SDL_thread.h"
typedef struct SDL_Thread SDL_Thread;
# 85 "./SDL3/SDL_thread.h"
typedef Uint64 SDL_ThreadID;
# 98 "./SDL3/SDL_thread.h"
typedef SDL_AtomicInt SDL_TLSID;
# 111 "./SDL3/SDL_thread.h"
typedef enum SDL_ThreadPriority {
    SDL_THREAD_PRIORITY_LOW,
    SDL_THREAD_PRIORITY_NORMAL,
    SDL_THREAD_PRIORITY_HIGH,
    SDL_THREAD_PRIORITY_TIME_CRITICAL
} SDL_ThreadPriority;
# 127 "./SDL3/SDL_thread.h"
typedef enum SDL_ThreadState
{
    SDL_THREAD_UNKNOWN,
    SDL_THREAD_ALIVE,
    SDL_THREAD_DETACHED,
    SDL_THREAD_COMPLETE
} SDL_ThreadState;
# 143 "./SDL3/SDL_thread.h"
typedef int ( *SDL_ThreadFunction) (void *data);
# 340 "./SDL3/SDL_thread.h"
extern __attribute__((dllimport)) SDL_Thread *  SDL_CreateThreadRuntime(SDL_ThreadFunction fn, const char *name, void *data, SDL_FunctionPointer pfnBeginThread, SDL_FunctionPointer pfnEndThread);
# 356 "./SDL3/SDL_thread.h"
extern __attribute__((dllimport)) SDL_Thread *  SDL_CreateThreadWithPropertiesRuntime(SDL_PropertiesID props, SDL_FunctionPointer pfnBeginThread, SDL_FunctionPointer pfnEndThread);
# 378 "./SDL3/SDL_thread.h"
extern __attribute__((dllimport)) const char *  SDL_GetThreadName(SDL_Thread *thread);
# 398 "./SDL3/SDL_thread.h"
extern __attribute__((dllimport)) SDL_ThreadID  SDL_GetCurrentThreadID(void);
# 417 "./SDL3/SDL_thread.h"
extern __attribute__((dllimport)) SDL_ThreadID  SDL_GetThreadID(SDL_Thread *thread);
# 434 "./SDL3/SDL_thread.h"
extern __attribute__((dllimport)) bool  SDL_SetCurrentThreadPriority(SDL_ThreadPriority priority);
# 472 "./SDL3/SDL_thread.h"
extern __attribute__((dllimport)) void  SDL_WaitThread(SDL_Thread *thread, int *status);
# 487 "./SDL3/SDL_thread.h"
extern __attribute__((dllimport)) SDL_ThreadState  SDL_GetThreadState(SDL_Thread *thread);
# 525 "./SDL3/SDL_thread.h"
extern __attribute__((dllimport)) void  SDL_DetachThread(SDL_Thread *thread);
# 540 "./SDL3/SDL_thread.h"
extern __attribute__((dllimport)) void *  SDL_GetTLS(SDL_TLSID *id);
# 553 "./SDL3/SDL_thread.h"
typedef void ( *SDL_TLSDestructorCallback)(void *value);
# 581 "./SDL3/SDL_thread.h"
extern __attribute__((dllimport)) bool  SDL_SetTLS(SDL_TLSID *id, const void *value, SDL_TLSDestructorCallback destructor);
# 594 "./SDL3/SDL_thread.h"
extern __attribute__((dllimport)) void  SDL_CleanupTLS(void);






# 1 "./SDL3/SDL_close_code.h" 1
# 601 "./SDL3/SDL_thread.h" 2
# 47 "./SDL3/SDL_mutex.h" 2
# 274 "./SDL3/SDL_mutex.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 274 "./SDL3/SDL_mutex.h" 2
# 296 "./SDL3/SDL_mutex.h"
typedef struct SDL_Mutex SDL_Mutex;
# 320 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) SDL_Mutex *  SDL_CreateMutex(void);
# 346 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) void  SDL_LockMutex(SDL_Mutex *mutex) ;
# 369 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) bool  SDL_TryLockMutex(SDL_Mutex *mutex) ;
# 391 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) void  SDL_UnlockMutex(SDL_Mutex *mutex) ;
# 410 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) void  SDL_DestroyMutex(SDL_Mutex *mutex);
# 438 "./SDL3/SDL_mutex.h"
typedef struct SDL_RWLock SDL_RWLock;
# 482 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) SDL_RWLock *  SDL_CreateRWLock(void);
# 521 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) void  SDL_LockRWLockForReading(SDL_RWLock *rwlock) ;
# 554 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) void  SDL_LockRWLockForWriting(SDL_RWLock *rwlock) ;
# 581 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) bool  SDL_TryLockRWLockForReading(SDL_RWLock *rwlock) ;
# 613 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) bool  SDL_TryLockRWLockForWriting(SDL_RWLock *rwlock) ;
# 641 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) void  SDL_UnlockRWLock(SDL_RWLock *rwlock) ;
# 660 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) void  SDL_DestroyRWLock(SDL_RWLock *rwlock);
# 684 "./SDL3/SDL_mutex.h"
typedef struct SDL_Semaphore SDL_Semaphore;
# 710 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) SDL_Semaphore *  SDL_CreateSemaphore(Uint32 initial_value);
# 726 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) void  SDL_DestroySemaphore(SDL_Semaphore *sem);
# 748 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) void  SDL_WaitSemaphore(SDL_Semaphore *sem);
# 769 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) bool  SDL_TryWaitSemaphore(SDL_Semaphore *sem);
# 791 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) bool  SDL_WaitSemaphoreTimeout(SDL_Semaphore *sem, Sint32 timeoutMS);
# 806 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) void  SDL_SignalSemaphore(SDL_Semaphore *sem);
# 818 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) Uint32  SDL_GetSemaphoreValue(SDL_Semaphore *sem);
# 841 "./SDL3/SDL_mutex.h"
typedef struct SDL_Condition SDL_Condition;
# 859 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) SDL_Condition *  SDL_CreateCondition(void);
# 872 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) void  SDL_DestroyCondition(SDL_Condition *cond);
# 887 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) void  SDL_SignalCondition(SDL_Condition *cond);
# 902 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) void  SDL_BroadcastCondition(SDL_Condition *cond);
# 930 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) void  SDL_WaitCondition(SDL_Condition *cond, SDL_Mutex *mutex);
# 960 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) bool  SDL_WaitConditionTimeout(SDL_Condition *cond,
                                                SDL_Mutex *mutex, Sint32 timeoutMS);
# 975 "./SDL3/SDL_mutex.h"
typedef enum SDL_InitStatus
{
    SDL_INIT_STATUS_UNINITIALIZED,
    SDL_INIT_STATUS_INITIALIZING,
    SDL_INIT_STATUS_INITIALIZED,
    SDL_INIT_STATUS_UNINITIALIZING
} SDL_InitStatus;
# 1039 "./SDL3/SDL_mutex.h"
typedef struct SDL_InitState
{
    SDL_AtomicInt status;
    SDL_ThreadID thread;
    void *reserved;
} SDL_InitState;
# 1067 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) bool  SDL_ShouldInit(SDL_InitState *state);
# 1088 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) bool  SDL_ShouldQuit(SDL_InitState *state);
# 1107 "./SDL3/SDL_mutex.h"
extern __attribute__((dllimport)) void  SDL_SetInitialized(SDL_InitState *state, bool initialized);
# 1116 "./SDL3/SDL_mutex.h"
# 1 "./SDL3/SDL_close_code.h" 1
# 1116 "./SDL3/SDL_mutex.h" 2
# 135 "./SDL3/SDL_audio.h" 2


# 1 "./SDL3/SDL_iostream.h" 1
# 43 "./SDL3/SDL_iostream.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 43 "./SDL3/SDL_iostream.h" 2
# 53 "./SDL3/SDL_iostream.h"
typedef enum SDL_IOStatus
{
    SDL_IO_STATUS_READY,
    SDL_IO_STATUS_ERROR,
    SDL_IO_STATUS_EOF,
    SDL_IO_STATUS_NOT_READY,
    SDL_IO_STATUS_READONLY,
    SDL_IO_STATUS_WRITEONLY
} SDL_IOStatus;
# 71 "./SDL3/SDL_iostream.h"
typedef enum SDL_IOWhence
{
    SDL_IO_SEEK_SET,
    SDL_IO_SEEK_CUR,
    SDL_IO_SEEK_END
} SDL_IOWhence;
# 92 "./SDL3/SDL_iostream.h"
typedef struct SDL_IOStreamInterface
{

    Uint32 version;






    Sint64 ( *size)(void *userdata);







    Sint64 ( *seek)(void *userdata, Sint64 offset, SDL_IOWhence whence);
# 122 "./SDL3/SDL_iostream.h"
    uint64 ( *read)(void *userdata, void *ptr, uint64 size, SDL_IOStatus *status);
# 134 "./SDL3/SDL_iostream.h"
    uint64 ( *write)(void *userdata, const void *ptr, uint64 size, SDL_IOStatus *status);
# 145 "./SDL3/SDL_iostream.h"
    bool ( *flush)(void *userdata, SDL_IOStatus *status);
# 158 "./SDL3/SDL_iostream.h"
    bool ( *close)(void *userdata);

} SDL_IOStreamInterface;
# 170 "./SDL3/SDL_iostream.h"
typedef int SDL_compile_time_assert_SDL_IOStreamInterface_SIZE[((sizeof(void *) == 4 && sizeof(SDL_IOStreamInterface) == 28) || (sizeof(void *) == 8 && sizeof(SDL_IOStreamInterface) == 56)) * 2 - 1];
# 182 "./SDL3/SDL_iostream.h"
typedef struct SDL_IOStream SDL_IOStream;
# 278 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) SDL_IOStream *  SDL_IOFromFile(const char *file, const char *mode);
# 330 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) SDL_IOStream *  SDL_IOFromMem(void *mem, uint64 size);
# 381 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) SDL_IOStream *  SDL_IOFromConstMem(const void *mem, uint64 size);
# 411 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) SDL_IOStream *  SDL_IOFromDynamicMem(void);
# 446 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) SDL_IOStream *  SDL_OpenIO(const SDL_IOStreamInterface *iface, void *userdata);
# 478 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_CloseIO(SDL_IOStream *context);
# 491 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) SDL_PropertiesID  SDL_GetIOProperties(SDL_IOStream *context);
# 511 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) SDL_IOStatus  SDL_GetIOStatus(SDL_IOStream *context);
# 525 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) Sint64  SDL_GetIOSize(SDL_IOStream *context);
# 554 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) Sint64  SDL_SeekIO(SDL_IOStream *context, Sint64 offset, SDL_IOWhence whence);
# 574 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) Sint64  SDL_TellIO(SDL_IOStream *context);
# 604 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) uint64  SDL_ReadIO(SDL_IOStream *context, void *ptr, uint64 size);
# 640 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) uint64  SDL_WriteIO(SDL_IOStream *context, const void *ptr, uint64 size);
# 661 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) uint64  SDL_IOprintf(SDL_IOStream *context,  const char *fmt, ...)  ;
# 681 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) uint64  SDL_IOvprintf(SDL_IOStream *context,  const char *fmt, byte* ap) ;
# 701 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_FlushIO(SDL_IOStream *context);
# 727 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) void *  SDL_LoadFile_IO(SDL_IOStream *src, uint64 *datasize, bool closeio);
# 750 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) void *  SDL_LoadFile(const char *file, uint64 *datasize);
# 771 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_SaveFile_IO(SDL_IOStream *src, const void *data, uint64 datasize, bool closeio);
# 790 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_SaveFile(const char *file, const void *data, uint64 datasize);
# 816 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_ReadU8(SDL_IOStream *src, Uint8 *value);
# 835 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_ReadS8(SDL_IOStream *src, Sint8 *value);
# 858 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_ReadU16LE(SDL_IOStream *src, Uint16 *value);
# 881 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_ReadS16LE(SDL_IOStream *src, Sint16 *value);
# 904 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_ReadU16BE(SDL_IOStream *src, Uint16 *value);
# 927 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_ReadS16BE(SDL_IOStream *src, Sint16 *value);
# 950 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_ReadU32LE(SDL_IOStream *src, Uint32 *value);
# 973 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_ReadS32LE(SDL_IOStream *src, Sint32 *value);
# 996 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_ReadU32BE(SDL_IOStream *src, Uint32 *value);
# 1019 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_ReadS32BE(SDL_IOStream *src, Sint32 *value);
# 1042 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_ReadU64LE(SDL_IOStream *src, Uint64 *value);
# 1065 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_ReadS64LE(SDL_IOStream *src, Sint64 *value);
# 1088 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_ReadU64BE(SDL_IOStream *src, Uint64 *value);
# 1111 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_ReadS64BE(SDL_IOStream *src, Sint64 *value);
# 1133 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_WriteU8(SDL_IOStream *dst, Uint8 value);
# 1147 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_WriteS8(SDL_IOStream *dst, Sint8 value);
# 1166 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_WriteU16LE(SDL_IOStream *dst, Uint16 value);
# 1185 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_WriteS16LE(SDL_IOStream *dst, Sint16 value);
# 1203 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_WriteU16BE(SDL_IOStream *dst, Uint16 value);
# 1221 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_WriteS16BE(SDL_IOStream *dst, Sint16 value);
# 1240 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_WriteU32LE(SDL_IOStream *dst, Uint32 value);
# 1259 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_WriteS32LE(SDL_IOStream *dst, Sint32 value);
# 1277 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_WriteU32BE(SDL_IOStream *dst, Uint32 value);
# 1295 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_WriteS32BE(SDL_IOStream *dst, Sint32 value);
# 1314 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_WriteU64LE(SDL_IOStream *dst, Uint64 value);
# 1333 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_WriteS64LE(SDL_IOStream *dst, Sint64 value);
# 1351 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_WriteU64BE(SDL_IOStream *dst, Uint64 value);
# 1369 "./SDL3/SDL_iostream.h"
extern __attribute__((dllimport)) bool  SDL_WriteS64BE(SDL_IOStream *dst, Sint64 value);
# 1378 "./SDL3/SDL_iostream.h"
# 1 "./SDL3/SDL_close_code.h" 1
# 1378 "./SDL3/SDL_iostream.h" 2
# 137 "./SDL3/SDL_audio.h" 2


# 1 "./SDL3/SDL_begin_code.h" 1
# 139 "./SDL3/SDL_audio.h" 2
# 221 "./SDL3/SDL_audio.h"
typedef enum SDL_AudioFormat
{
    SDL_AUDIO_UNKNOWN   = 0x0000u,
    SDL_AUDIO_U8        = 0x0008u,

    SDL_AUDIO_S8        = 0x8008u,

    SDL_AUDIO_S16LE     = 0x8010u,

    SDL_AUDIO_S16BE     = 0x9010u,

    SDL_AUDIO_S32LE     = 0x8020u,

    SDL_AUDIO_S32BE     = 0x9020u,

    SDL_AUDIO_F32LE     = 0x8120u,

    SDL_AUDIO_F32BE     = 0x9120u,




    SDL_AUDIO_S16 = SDL_AUDIO_S16LE,
    SDL_AUDIO_S32 = SDL_AUDIO_S32LE,
    SDL_AUDIO_F32 = SDL_AUDIO_F32LE





} SDL_AudioFormat;
# 374 "./SDL3/SDL_audio.h"
typedef Uint32 SDL_AudioDeviceID;
# 405 "./SDL3/SDL_audio.h"
typedef struct SDL_AudioSpec
{
    SDL_AudioFormat format;
    int channels;
    int freq;
} SDL_AudioSpec;
# 452 "./SDL3/SDL_audio.h"
typedef struct SDL_AudioStream SDL_AudioStream;
# 478 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) int  SDL_GetNumAudioDrivers(void);
# 502 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) const char *  SDL_GetAudioDriver(int index);
# 518 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) const char *  SDL_GetCurrentAudioDriver(void);
# 547 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) SDL_AudioDeviceID *  SDL_GetAudioPlaybackDevices(int *count);
# 576 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) SDL_AudioDeviceID *  SDL_GetAudioRecordingDevices(int *count);
# 601 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) const char *  SDL_GetAudioDeviceName(SDL_AudioDeviceID devid);
# 636 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_GetAudioDeviceFormat(SDL_AudioDeviceID devid, SDL_AudioSpec *spec, int *sample_frames);
# 659 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) int *  SDL_GetAudioDeviceChannelMap(SDL_AudioDeviceID devid, int *count);
# 735 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) SDL_AudioDeviceID  SDL_OpenAudioDevice(SDL_AudioDeviceID devid, const SDL_AudioSpec *spec);
# 760 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_IsAudioDevicePhysical(SDL_AudioDeviceID devid);
# 774 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_IsAudioDevicePlayback(SDL_AudioDeviceID devid);
# 805 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_PauseAudioDevice(SDL_AudioDeviceID devid);
# 833 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_ResumeAudioDevice(SDL_AudioDeviceID devid);
# 855 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_AudioDevicePaused(SDL_AudioDeviceID devid);
# 878 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) float  SDL_GetAudioDeviceGain(SDL_AudioDeviceID devid);
# 913 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_SetAudioDeviceGain(SDL_AudioDeviceID devid, float gain);
# 934 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) void  SDL_CloseAudioDevice(SDL_AudioDeviceID devid);
# 974 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_BindAudioStreams(SDL_AudioDeviceID devid, SDL_AudioStream * const *streams, int num_streams);
# 995 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_BindAudioStream(SDL_AudioDeviceID devid, SDL_AudioStream *stream);
# 1016 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) void  SDL_UnbindAudioStreams(SDL_AudioStream * const *streams, int num_streams);
# 1032 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) void  SDL_UnbindAudioStream(SDL_AudioStream *stream);
# 1053 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) SDL_AudioDeviceID  SDL_GetAudioStreamDevice(SDL_AudioStream *stream);
# 1082 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) SDL_AudioStream *  SDL_CreateAudioStream(const SDL_AudioSpec *src_spec, const SDL_AudioSpec *dst_spec);
# 1106 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) SDL_PropertiesID  SDL_GetAudioStreamProperties(SDL_AudioStream *stream);
# 1127 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_GetAudioStreamFormat(SDL_AudioStream *stream, SDL_AudioSpec *src_spec, SDL_AudioSpec *dst_spec);
# 1167 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_SetAudioStreamFormat(SDL_AudioStream *stream, const SDL_AudioSpec *src_spec, const SDL_AudioSpec *dst_spec);
# 1183 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) float  SDL_GetAudioStreamFrequencyRatio(SDL_AudioStream *stream);
# 1211 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_SetAudioStreamFrequencyRatio(SDL_AudioStream *stream, float ratio);
# 1232 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) float  SDL_GetAudioStreamGain(SDL_AudioStream *stream);
# 1257 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_SetAudioStreamGain(SDL_AudioStream *stream, float gain);
# 1281 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) int *  SDL_GetAudioStreamInputChannelMap(SDL_AudioStream *stream, int *count);
# 1305 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) int *  SDL_GetAudioStreamOutputChannelMap(SDL_AudioStream *stream, int *count);
# 1365 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_SetAudioStreamInputChannelMap(SDL_AudioStream *stream, const int *chmap, int count);
# 1423 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_SetAudioStreamOutputChannelMap(SDL_AudioStream *stream, const int *chmap, int count);
# 1453 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_PutAudioStreamData(SDL_AudioStream *stream, const void *buf, int len);
# 1483 "./SDL3/SDL_audio.h"
typedef void ( *SDL_AudioStreamDataCompleteCallback)(void *userdata, const void *buf, int buflen);
# 1530 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_PutAudioStreamDataNoCopy(SDL_AudioStream *stream, const void *buf, int len, SDL_AudioStreamDataCompleteCallback callback, void *userdata);
# 1583 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_PutAudioStreamPlanarData(SDL_AudioStream *stream, const void * const *channel_buffers, int num_channels, int num_samples);
# 1613 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) int  SDL_GetAudioStreamData(SDL_AudioStream *stream, void *buf, int len);
# 1639 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) int  SDL_GetAudioStreamAvailable(SDL_AudioStream *stream);
# 1678 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) int  SDL_GetAudioStreamQueued(SDL_AudioStream *stream);
# 1699 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_FlushAudioStream(SDL_AudioStream *stream);
# 1720 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_ClearAudioStream(SDL_AudioStream *stream);
# 1744 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_PauseAudioStreamDevice(SDL_AudioStream *stream);
# 1767 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_ResumeAudioStreamDevice(SDL_AudioStream *stream);
# 1786 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_AudioStreamDevicePaused(SDL_AudioStream *stream);
# 1815 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_LockAudioStream(SDL_AudioStream *stream);
# 1834 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_UnlockAudioStream(SDL_AudioStream *stream);
# 1876 "./SDL3/SDL_audio.h"
typedef void ( *SDL_AudioStreamCallback)(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount);
# 1922 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_SetAudioStreamGetCallback(SDL_AudioStream *stream, SDL_AudioStreamCallback callback, void *userdata);
# 1971 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_SetAudioStreamPutCallback(SDL_AudioStream *stream, SDL_AudioStreamCallback callback, void *userdata);
# 1993 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) void  SDL_DestroyAudioStream(SDL_AudioStream *stream);
# 2056 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) SDL_AudioStream *  SDL_OpenAudioDeviceStream(SDL_AudioDeviceID devid, const SDL_AudioSpec *spec, SDL_AudioStreamCallback callback, void *userdata);
# 2093 "./SDL3/SDL_audio.h"
typedef void ( *SDL_AudioPostmixCallback)(void *userdata, const SDL_AudioSpec *spec, float *buffer, int buflen);
# 2147 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_SetAudioPostmixCallback(SDL_AudioDeviceID devid, SDL_AudioPostmixCallback callback, void *userdata);
# 2228 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_LoadWAV_IO(SDL_IOStream *src, bool closeio, SDL_AudioSpec *spec, Uint8 **audio_buf, Uint32 *audio_len);
# 2264 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_LoadWAV(const char *path, SDL_AudioSpec *spec, Uint8 **audio_buf, Uint32 *audio_len);
# 2300 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_MixAudio(Uint8 *dst, const Uint8 *src, SDL_AudioFormat format, Uint32 len, float volume);
# 2330 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) bool  SDL_ConvertAudioSamples(const SDL_AudioSpec *src_spec, const Uint8 *src_data, int src_len, const SDL_AudioSpec *dst_spec, Uint8 **dst_data, int *dst_len);
# 2343 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) const char *  SDL_GetAudioFormatName(SDL_AudioFormat format);
# 2359 "./SDL3/SDL_audio.h"
extern __attribute__((dllimport)) int  SDL_GetSilenceValueForFormat(SDL_AudioFormat format);







# 1 "./SDL3/SDL_close_code.h" 1
# 2367 "./SDL3/SDL_audio.h" 2
# 39 "SDL3/SDL.h" 2

# 1 "./SDL3/SDL_bits.h" 1
# 34 "./SDL3/SDL_bits.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 34 "./SDL3/SDL_bits.h" 2
# 66 "./SDL3/SDL_bits.h"
static inline int SDL_MostSignificantBitIndex32(Uint32 x)
{
# 92 "./SDL3/SDL_bits.h"
    const Uint32 b[] = {0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000};
    const int    S[] = {1, 2, 4, 8, 16};

    int msbIndex = 0;
    int i;

    if (x == 0) {
        return -1;
    }

    for (i = 4; i >= 0; i--)
    {
        if (x & b[i])
        {
            x >>= S[i];
            msbIndex |= S[i];
        }
    }

    return msbIndex;

}
# 133 "./SDL3/SDL_bits.h"
static inline bool SDL_HasExactlyOneBitSet32(Uint32 x)
{
    if (x && !(x & (x - 1))) {
        return true;
    }
    return false;
}






# 1 "./SDL3/SDL_close_code.h" 1
# 146 "./SDL3/SDL_bits.h" 2
# 40 "SDL3/SDL.h" 2

# 1 "./SDL3/SDL_blendmode.h" 1
# 36 "./SDL3/SDL_blendmode.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 36 "./SDL3/SDL_blendmode.h" 2
# 52 "./SDL3/SDL_blendmode.h"
typedef Uint32 SDL_BlendMode;
# 69 "./SDL3/SDL_blendmode.h"
typedef enum SDL_BlendOperation
{
    SDL_BLENDOPERATION_ADD              = 0x1,
    SDL_BLENDOPERATION_SUBTRACT         = 0x2,
    SDL_BLENDOPERATION_REV_SUBTRACT     = 0x3,
    SDL_BLENDOPERATION_MINIMUM          = 0x4,
    SDL_BLENDOPERATION_MAXIMUM          = 0x5
} SDL_BlendOperation;
# 88 "./SDL3/SDL_blendmode.h"
typedef enum SDL_BlendFactor
{
    SDL_BLENDFACTOR_ZERO                = 0x1,
    SDL_BLENDFACTOR_ONE                 = 0x2,
    SDL_BLENDFACTOR_SRC_COLOR           = 0x3,
    SDL_BLENDFACTOR_ONE_MINUS_SRC_COLOR = 0x4,
    SDL_BLENDFACTOR_SRC_ALPHA           = 0x5,
    SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA = 0x6,
    SDL_BLENDFACTOR_DST_COLOR           = 0x7,
    SDL_BLENDFACTOR_ONE_MINUS_DST_COLOR = 0x8,
    SDL_BLENDFACTOR_DST_ALPHA           = 0x9,
    SDL_BLENDFACTOR_ONE_MINUS_DST_ALPHA = 0xA
} SDL_BlendFactor;
# 189 "./SDL3/SDL_blendmode.h"
extern __attribute__((dllimport)) SDL_BlendMode  SDL_ComposeCustomBlendMode(SDL_BlendFactor srcColorFactor,
                                                                 SDL_BlendFactor dstColorFactor,
                                                                 SDL_BlendOperation colorOperation,
                                                                 SDL_BlendFactor srcAlphaFactor,
                                                                 SDL_BlendFactor dstAlphaFactor,
                                                                 SDL_BlendOperation alphaOperation);






# 1 "./SDL3/SDL_close_code.h" 1
# 201 "./SDL3/SDL_blendmode.h" 2
# 41 "SDL3/SDL.h" 2

# 1 "./SDL3/SDL_camera.h" 1
# 74 "./SDL3/SDL_camera.h"
# 1 "./SDL3/SDL_pixels.h" 1
# 88 "./SDL3/SDL_pixels.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 88 "./SDL3/SDL_pixels.h" 2
# 134 "./SDL3/SDL_pixels.h"
typedef enum SDL_PixelType
{
    SDL_PIXELTYPE_UNKNOWN,
    SDL_PIXELTYPE_INDEX1,
    SDL_PIXELTYPE_INDEX4,
    SDL_PIXELTYPE_INDEX8,
    SDL_PIXELTYPE_PACKED8,
    SDL_PIXELTYPE_PACKED16,
    SDL_PIXELTYPE_PACKED32,
    SDL_PIXELTYPE_ARRAYU8,
    SDL_PIXELTYPE_ARRAYU16,
    SDL_PIXELTYPE_ARRAYU32,
    SDL_PIXELTYPE_ARRAYF16,
    SDL_PIXELTYPE_ARRAYF32,

    SDL_PIXELTYPE_INDEX2
} SDL_PixelType;






typedef enum SDL_BitmapOrder
{
    SDL_BITMAPORDER_NONE,
    SDL_BITMAPORDER_4321,
    SDL_BITMAPORDER_1234
} SDL_BitmapOrder;






typedef enum SDL_PackedOrder
{
    SDL_PACKEDORDER_NONE,
    SDL_PACKEDORDER_XRGB,
    SDL_PACKEDORDER_RGBX,
    SDL_PACKEDORDER_ARGB,
    SDL_PACKEDORDER_RGBA,
    SDL_PACKEDORDER_XBGR,
    SDL_PACKEDORDER_BGRX,
    SDL_PACKEDORDER_ABGR,
    SDL_PACKEDORDER_BGRA
} SDL_PackedOrder;






typedef enum SDL_ArrayOrder
{
    SDL_ARRAYORDER_NONE,
    SDL_ARRAYORDER_RGB,
    SDL_ARRAYORDER_RGBA,
    SDL_ARRAYORDER_ARGB,
    SDL_ARRAYORDER_BGR,
    SDL_ARRAYORDER_BGRA,
    SDL_ARRAYORDER_ABGR
} SDL_ArrayOrder;






typedef enum SDL_PackedLayout
{
    SDL_PACKEDLAYOUT_NONE,
    SDL_PACKEDLAYOUT_332,
    SDL_PACKEDLAYOUT_4444,
    SDL_PACKEDLAYOUT_1555,
    SDL_PACKEDLAYOUT_5551,
    SDL_PACKEDLAYOUT_565,
    SDL_PACKEDLAYOUT_8888,
    SDL_PACKEDLAYOUT_2101010,
    SDL_PACKEDLAYOUT_1010102
} SDL_PackedLayout;
# 548 "./SDL3/SDL_pixels.h"
typedef enum SDL_PixelFormat
{
    SDL_PIXELFORMAT_UNKNOWN = 0,
    SDL_PIXELFORMAT_INDEX1LSB = 0x11100100u,

    SDL_PIXELFORMAT_INDEX1MSB = 0x11200100u,

    SDL_PIXELFORMAT_INDEX2LSB = 0x1c100200u,

    SDL_PIXELFORMAT_INDEX2MSB = 0x1c200200u,

    SDL_PIXELFORMAT_INDEX4LSB = 0x12100400u,

    SDL_PIXELFORMAT_INDEX4MSB = 0x12200400u,

    SDL_PIXELFORMAT_INDEX8 = 0x13000801u,

    SDL_PIXELFORMAT_RGB332 = 0x14110801u,

    SDL_PIXELFORMAT_XRGB4444 = 0x15120c02u,

    SDL_PIXELFORMAT_XBGR4444 = 0x15520c02u,

    SDL_PIXELFORMAT_XRGB1555 = 0x15130f02u,

    SDL_PIXELFORMAT_XBGR1555 = 0x15530f02u,

    SDL_PIXELFORMAT_ARGB4444 = 0x15321002u,

    SDL_PIXELFORMAT_RGBA4444 = 0x15421002u,

    SDL_PIXELFORMAT_ABGR4444 = 0x15721002u,

    SDL_PIXELFORMAT_BGRA4444 = 0x15821002u,

    SDL_PIXELFORMAT_ARGB1555 = 0x15331002u,

    SDL_PIXELFORMAT_RGBA5551 = 0x15441002u,

    SDL_PIXELFORMAT_ABGR1555 = 0x15731002u,

    SDL_PIXELFORMAT_BGRA5551 = 0x15841002u,

    SDL_PIXELFORMAT_RGB565 = 0x15151002u,

    SDL_PIXELFORMAT_BGR565 = 0x15551002u,

    SDL_PIXELFORMAT_RGB24 = 0x17101803u,

    SDL_PIXELFORMAT_BGR24 = 0x17401803u,

    SDL_PIXELFORMAT_XRGB8888 = 0x16161804u,

    SDL_PIXELFORMAT_RGBX8888 = 0x16261804u,

    SDL_PIXELFORMAT_XBGR8888 = 0x16561804u,

    SDL_PIXELFORMAT_BGRX8888 = 0x16661804u,

    SDL_PIXELFORMAT_ARGB8888 = 0x16362004u,

    SDL_PIXELFORMAT_RGBA8888 = 0x16462004u,

    SDL_PIXELFORMAT_ABGR8888 = 0x16762004u,

    SDL_PIXELFORMAT_BGRA8888 = 0x16862004u,

    SDL_PIXELFORMAT_XRGB2101010 = 0x16172004u,

    SDL_PIXELFORMAT_XBGR2101010 = 0x16572004u,

    SDL_PIXELFORMAT_ARGB2101010 = 0x16372004u,

    SDL_PIXELFORMAT_ABGR2101010 = 0x16772004u,

    SDL_PIXELFORMAT_RGB48 = 0x18103006u,

    SDL_PIXELFORMAT_BGR48 = 0x18403006u,

    SDL_PIXELFORMAT_RGBA64 = 0x18204008u,

    SDL_PIXELFORMAT_ARGB64 = 0x18304008u,

    SDL_PIXELFORMAT_BGRA64 = 0x18504008u,

    SDL_PIXELFORMAT_ABGR64 = 0x18604008u,

    SDL_PIXELFORMAT_RGB48_FLOAT = 0x1a103006u,

    SDL_PIXELFORMAT_BGR48_FLOAT = 0x1a403006u,

    SDL_PIXELFORMAT_RGBA64_FLOAT = 0x1a204008u,

    SDL_PIXELFORMAT_ARGB64_FLOAT = 0x1a304008u,

    SDL_PIXELFORMAT_BGRA64_FLOAT = 0x1a504008u,

    SDL_PIXELFORMAT_ABGR64_FLOAT = 0x1a604008u,

    SDL_PIXELFORMAT_RGB96_FLOAT = 0x1b10600cu,

    SDL_PIXELFORMAT_BGR96_FLOAT = 0x1b40600cu,

    SDL_PIXELFORMAT_RGBA128_FLOAT = 0x1b208010u,

    SDL_PIXELFORMAT_ARGB128_FLOAT = 0x1b308010u,

    SDL_PIXELFORMAT_BGRA128_FLOAT = 0x1b508010u,

    SDL_PIXELFORMAT_ABGR128_FLOAT = 0x1b608010u,


    SDL_PIXELFORMAT_YV12 = 0x32315659u,

    SDL_PIXELFORMAT_IYUV = 0x56555949u,

    SDL_PIXELFORMAT_YUY2 = 0x32595559u,

    SDL_PIXELFORMAT_UYVY = 0x59565955u,

    SDL_PIXELFORMAT_YVYU = 0x55595659u,

    SDL_PIXELFORMAT_NV12 = 0x3231564eu,

    SDL_PIXELFORMAT_NV21 = 0x3132564eu,

    SDL_PIXELFORMAT_P010 = 0x30313050u,

    SDL_PIXELFORMAT_EXTERNAL_OES = 0x2053454fu,


    SDL_PIXELFORMAT_MJPG = 0x47504a4du,
# 693 "./SDL3/SDL_pixels.h"
    SDL_PIXELFORMAT_RGBA32 = SDL_PIXELFORMAT_ABGR8888,
    SDL_PIXELFORMAT_ARGB32 = SDL_PIXELFORMAT_BGRA8888,
    SDL_PIXELFORMAT_BGRA32 = SDL_PIXELFORMAT_ARGB8888,
    SDL_PIXELFORMAT_ABGR32 = SDL_PIXELFORMAT_RGBA8888,
    SDL_PIXELFORMAT_RGBX32 = SDL_PIXELFORMAT_XBGR8888,
    SDL_PIXELFORMAT_XRGB32 = SDL_PIXELFORMAT_BGRX8888,
    SDL_PIXELFORMAT_BGRX32 = SDL_PIXELFORMAT_XRGB8888,
    SDL_PIXELFORMAT_XBGR32 = SDL_PIXELFORMAT_RGBX8888

} SDL_PixelFormat;






typedef enum SDL_ColorType
{
    SDL_COLOR_TYPE_UNKNOWN = 0,
    SDL_COLOR_TYPE_RGB = 1,
    SDL_COLOR_TYPE_YCBCR = 2
} SDL_ColorType;







typedef enum SDL_ColorRange
{
    SDL_COLOR_RANGE_UNKNOWN = 0,
    SDL_COLOR_RANGE_LIMITED = 1,
    SDL_COLOR_RANGE_FULL = 2
} SDL_ColorRange;







typedef enum SDL_ColorPrimaries
{
    SDL_COLOR_PRIMARIES_UNKNOWN = 0,
    SDL_COLOR_PRIMARIES_BT709 = 1,
    SDL_COLOR_PRIMARIES_UNSPECIFIED = 2,
    SDL_COLOR_PRIMARIES_BT470M = 4,
    SDL_COLOR_PRIMARIES_BT470BG = 5,
    SDL_COLOR_PRIMARIES_BT601 = 6,
    SDL_COLOR_PRIMARIES_SMPTE240 = 7,
    SDL_COLOR_PRIMARIES_GENERIC_FILM = 8,
    SDL_COLOR_PRIMARIES_BT2020 = 9,
    SDL_COLOR_PRIMARIES_XYZ = 10,
    SDL_COLOR_PRIMARIES_SMPTE431 = 11,
    SDL_COLOR_PRIMARIES_SMPTE432 = 12,
    SDL_COLOR_PRIMARIES_EBU3213 = 22,
    SDL_COLOR_PRIMARIES_CUSTOM = 31
} SDL_ColorPrimaries;
# 760 "./SDL3/SDL_pixels.h"
typedef enum SDL_TransferCharacteristics
{
    SDL_TRANSFER_CHARACTERISTICS_UNKNOWN = 0,
    SDL_TRANSFER_CHARACTERISTICS_BT709 = 1,
    SDL_TRANSFER_CHARACTERISTICS_UNSPECIFIED = 2,
    SDL_TRANSFER_CHARACTERISTICS_GAMMA22 = 4,
    SDL_TRANSFER_CHARACTERISTICS_GAMMA28 = 5,
    SDL_TRANSFER_CHARACTERISTICS_BT601 = 6,
    SDL_TRANSFER_CHARACTERISTICS_SMPTE240 = 7,
    SDL_TRANSFER_CHARACTERISTICS_LINEAR = 8,
    SDL_TRANSFER_CHARACTERISTICS_LOG100 = 9,
    SDL_TRANSFER_CHARACTERISTICS_LOG100_SQRT10 = 10,
    SDL_TRANSFER_CHARACTERISTICS_IEC61966 = 11,
    SDL_TRANSFER_CHARACTERISTICS_BT1361 = 12,
    SDL_TRANSFER_CHARACTERISTICS_SRGB = 13,
    SDL_TRANSFER_CHARACTERISTICS_BT2020_10BIT = 14,
    SDL_TRANSFER_CHARACTERISTICS_BT2020_12BIT = 15,
    SDL_TRANSFER_CHARACTERISTICS_PQ = 16,
    SDL_TRANSFER_CHARACTERISTICS_SMPTE428 = 17,
    SDL_TRANSFER_CHARACTERISTICS_HLG = 18,
    SDL_TRANSFER_CHARACTERISTICS_CUSTOM = 31
} SDL_TransferCharacteristics;
# 790 "./SDL3/SDL_pixels.h"
typedef enum SDL_MatrixCoefficients
{
    SDL_MATRIX_COEFFICIENTS_IDENTITY = 0,
    SDL_MATRIX_COEFFICIENTS_BT709 = 1,
    SDL_MATRIX_COEFFICIENTS_UNSPECIFIED = 2,
    SDL_MATRIX_COEFFICIENTS_FCC = 4,
    SDL_MATRIX_COEFFICIENTS_BT470BG = 5,
    SDL_MATRIX_COEFFICIENTS_BT601 = 6,
    SDL_MATRIX_COEFFICIENTS_SMPTE240 = 7,
    SDL_MATRIX_COEFFICIENTS_YCGCO = 8,
    SDL_MATRIX_COEFFICIENTS_BT2020_NCL = 9,
    SDL_MATRIX_COEFFICIENTS_BT2020_CL = 10,
    SDL_MATRIX_COEFFICIENTS_SMPTE2085 = 11,
    SDL_MATRIX_COEFFICIENTS_CHROMA_DERIVED_NCL = 12,
    SDL_MATRIX_COEFFICIENTS_CHROMA_DERIVED_CL = 13,
    SDL_MATRIX_COEFFICIENTS_ICTCP = 14,
    SDL_MATRIX_COEFFICIENTS_CUSTOM = 31
} SDL_MatrixCoefficients;






typedef enum SDL_ChromaLocation
{
    SDL_CHROMA_LOCATION_NONE = 0,
    SDL_CHROMA_LOCATION_LEFT = 1,
    SDL_CHROMA_LOCATION_CENTER = 2,
    SDL_CHROMA_LOCATION_TOPLEFT = 3
} SDL_ChromaLocation;
# 1011 "./SDL3/SDL_pixels.h"
typedef enum SDL_Colorspace
{
    SDL_COLORSPACE_UNKNOWN = 0,


    SDL_COLORSPACE_SRGB = 0x120005a0u,
# 1025 "./SDL3/SDL_pixels.h"
    SDL_COLORSPACE_SRGB_LINEAR = 0x12000500u,
# 1034 "./SDL3/SDL_pixels.h"
    SDL_COLORSPACE_HDR10 = 0x12002600u,







    SDL_COLORSPACE_JPEG = 0x220004c6u,







    SDL_COLORSPACE_BT601_LIMITED = 0x211018c6u,







    SDL_COLORSPACE_BT601_FULL = 0x221018c6u,







    SDL_COLORSPACE_BT709_LIMITED = 0x21100421u,







    SDL_COLORSPACE_BT709_FULL = 0x22100421u,







    SDL_COLORSPACE_BT2020_LIMITED = 0x21102609u,







    SDL_COLORSPACE_BT2020_FULL = 0x22102609u,







    SDL_COLORSPACE_RGB_DEFAULT = SDL_COLORSPACE_SRGB,
    SDL_COLORSPACE_YUV_DEFAULT = SDL_COLORSPACE_BT601_LIMITED
} SDL_Colorspace;
# 1112 "./SDL3/SDL_pixels.h"
typedef struct SDL_Color
{
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
} SDL_Color;







typedef struct SDL_FColor
{
    float r;
    float g;
    float b;
    float a;
} SDL_FColor;
# 1141 "./SDL3/SDL_pixels.h"
typedef struct SDL_Palette
{
    int ncolors;
    SDL_Color *colors;
    Uint32 version;
    int refcount;
} SDL_Palette;






typedef struct SDL_PixelFormatDetails
{
    SDL_PixelFormat format;
    Uint8 bits_per_pixel;
    Uint8 bytes_per_pixel;
    Uint8 padding[2];
    Uint32 Rmask;
    Uint32 Gmask;
    Uint32 Bmask;
    Uint32 Amask;
    Uint8 Rbits;
    Uint8 Gbits;
    Uint8 Bbits;
    Uint8 Abits;
    Uint8 Rshift;
    Uint8 Gshift;
    Uint8 Bshift;
    Uint8 Ashift;
} SDL_PixelFormatDetails;
# 1185 "./SDL3/SDL_pixels.h"
extern __attribute__((dllimport)) const char *  SDL_GetPixelFormatName(SDL_PixelFormat format);
# 1205 "./SDL3/SDL_pixels.h"
extern __attribute__((dllimport)) bool  SDL_GetMasksForPixelFormat(SDL_PixelFormat format, int *bpp, Uint32 *Rmask, Uint32 *Gmask, Uint32 *Bmask, Uint32 *Amask);
# 1227 "./SDL3/SDL_pixels.h"
extern __attribute__((dllimport)) SDL_PixelFormat  SDL_GetPixelFormatForMasks(int bpp, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);
# 1244 "./SDL3/SDL_pixels.h"
extern __attribute__((dllimport)) const SDL_PixelFormatDetails *  SDL_GetPixelFormatDetails(SDL_PixelFormat format);
# 1264 "./SDL3/SDL_pixels.h"
extern __attribute__((dllimport)) SDL_Palette *  SDL_CreatePalette(int ncolors);
# 1281 "./SDL3/SDL_pixels.h"
extern __attribute__((dllimport)) bool  SDL_SetPaletteColors(SDL_Palette *palette, const SDL_Color *colors, int firstcolor, int ncolors);
# 1295 "./SDL3/SDL_pixels.h"
extern __attribute__((dllimport)) void  SDL_DestroyPalette(SDL_Palette *palette);
# 1333 "./SDL3/SDL_pixels.h"
extern __attribute__((dllimport)) Uint32  SDL_MapRGB(const SDL_PixelFormatDetails *format, const SDL_Palette *palette, Uint8 r, Uint8 g, Uint8 b);
# 1372 "./SDL3/SDL_pixels.h"
extern __attribute__((dllimport)) Uint32  SDL_MapRGBA(const SDL_PixelFormatDetails *format, const SDL_Palette *palette, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
# 1400 "./SDL3/SDL_pixels.h"
extern __attribute__((dllimport)) void  SDL_GetRGB(Uint32 pixelvalue, const SDL_PixelFormatDetails *format, const SDL_Palette *palette, Uint8 *r, Uint8 *g, Uint8 *b);
# 1432 "./SDL3/SDL_pixels.h"
extern __attribute__((dllimport)) void  SDL_GetRGBA(Uint32 pixelvalue, const SDL_PixelFormatDetails *format, const SDL_Palette *palette, Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a);







# 1 "./SDL3/SDL_close_code.h" 1
# 1440 "./SDL3/SDL_pixels.h" 2
# 74 "./SDL3/SDL_camera.h" 2


# 1 "./SDL3/SDL_surface.h" 1
# 53 "./SDL3/SDL_surface.h"
# 1 "./SDL3/SDL_rect.h" 1
# 36 "./SDL3/SDL_rect.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 36 "./SDL3/SDL_rect.h" 2
# 49 "./SDL3/SDL_rect.h"
typedef struct SDL_Point
{
    int x;
    int y;
} SDL_Point;
# 63 "./SDL3/SDL_rect.h"
typedef struct SDL_FPoint
{
    float x;
    float y;
} SDL_FPoint;
# 83 "./SDL3/SDL_rect.h"
typedef struct SDL_Rect
{
    int x, y;
    int w, h;
} SDL_Rect;
# 109 "./SDL3/SDL_rect.h"
typedef struct SDL_FRect
{
    float x;
    float y;
    float w;
    float h;
} SDL_FRect;
# 129 "./SDL3/SDL_rect.h"
static inline void SDL_RectToFRect(const SDL_Rect *rect, SDL_FRect *frect)
{
    frect->x = ((float)(rect->x));
    frect->y = ((float)(rect->y));
    frect->w = ((float)(rect->w));
    frect->h = ((float)(rect->h));
}
# 158 "./SDL3/SDL_rect.h"
static inline bool SDL_PointInRect(const SDL_Point *p, const SDL_Rect *r)
{
    return ( p && r && (p->x >= r->x) && (p->x < (r->x + r->w)) &&
             (p->y >= r->y) && (p->y < (r->y + r->h)) ) ? true : false;
}
# 182 "./SDL3/SDL_rect.h"
static inline bool SDL_RectEmpty(const SDL_Rect *r)
{
    return ((!r) || (r->w <= 0) || (r->h <= 0)) ? true : false;
}
# 206 "./SDL3/SDL_rect.h"
static inline bool SDL_RectsEqual(const SDL_Rect *a, const SDL_Rect *b)
{
    return (a && b && (a->x == b->x) && (a->y == b->y) &&
            (a->w == b->w) && (a->h == b->h)) ? true : false;
}
# 227 "./SDL3/SDL_rect.h"
extern __attribute__((dllimport)) bool  SDL_HasRectIntersection(const SDL_Rect *A, const SDL_Rect *B);
# 246 "./SDL3/SDL_rect.h"
extern __attribute__((dllimport)) bool  SDL_GetRectIntersection(const SDL_Rect *A, const SDL_Rect *B, SDL_Rect *result);
# 262 "./SDL3/SDL_rect.h"
extern __attribute__((dllimport)) bool  SDL_GetRectUnion(const SDL_Rect *A, const SDL_Rect *B, SDL_Rect *result);
# 283 "./SDL3/SDL_rect.h"
extern __attribute__((dllimport)) bool  SDL_GetRectEnclosingPoints(const SDL_Point *points, int count, const SDL_Rect *clip, SDL_Rect *result);
# 305 "./SDL3/SDL_rect.h"
extern __attribute__((dllimport)) bool  SDL_GetRectAndLineIntersection(const SDL_Rect *rect, int *X1, int *Y1, int *X2, int *Y2);
# 331 "./SDL3/SDL_rect.h"
static inline bool SDL_PointInRectFloat(const SDL_FPoint *p, const SDL_FRect *r)
{
    return ( p && r && (p->x >= r->x) && (p->x <= (r->x + r->w)) &&
             (p->y >= r->y) && (p->y <= (r->y + r->h)) ) ? true : false;
}
# 355 "./SDL3/SDL_rect.h"
static inline bool SDL_RectEmptyFloat(const SDL_FRect *r)
{
    return ((!r) || (r->w < 0.0f) || (r->h < 0.0f)) ? true : false;
}
# 385 "./SDL3/SDL_rect.h"
static inline bool SDL_RectsEqualEpsilon(const SDL_FRect *a, const SDL_FRect *b, float epsilon)
{
    return (a && b && ((a == b) ||
            ((SDL_fabsf(a->x - b->x) <= epsilon) &&
            (SDL_fabsf(a->y - b->y) <= epsilon) &&
            (SDL_fabsf(a->w - b->w) <= epsilon) &&
            (SDL_fabsf(a->h - b->h) <= epsilon))))
            ? true : false;
}
# 420 "./SDL3/SDL_rect.h"
static inline bool SDL_RectsEqualFloat(const SDL_FRect *a, const SDL_FRect *b)
{
    return SDL_RectsEqualEpsilon(a, b, 1.1920928955078125e-07F);
}
# 440 "./SDL3/SDL_rect.h"
extern __attribute__((dllimport)) bool  SDL_HasRectIntersectionFloat(const SDL_FRect *A, const SDL_FRect *B);
# 459 "./SDL3/SDL_rect.h"
extern __attribute__((dllimport)) bool  SDL_GetRectIntersectionFloat(const SDL_FRect *A, const SDL_FRect *B, SDL_FRect *result);
# 475 "./SDL3/SDL_rect.h"
extern __attribute__((dllimport)) bool  SDL_GetRectUnionFloat(const SDL_FRect *A, const SDL_FRect *B, SDL_FRect *result);
# 497 "./SDL3/SDL_rect.h"
extern __attribute__((dllimport)) bool  SDL_GetRectEnclosingPointsFloat(const SDL_FPoint *points, int count, const SDL_FRect *clip, SDL_FRect *result);
# 520 "./SDL3/SDL_rect.h"
extern __attribute__((dllimport)) bool  SDL_GetRectAndLineIntersectionFloat(const SDL_FRect *rect, float *X1, float *Y1, float *X2, float *Y2);






# 1 "./SDL3/SDL_close_code.h" 1
# 527 "./SDL3/SDL_rect.h" 2
# 53 "./SDL3/SDL_surface.h" 2



# 1 "./SDL3/SDL_begin_code.h" 1
# 56 "./SDL3/SDL_surface.h" 2
# 68 "./SDL3/SDL_surface.h"
typedef Uint32 SDL_SurfaceFlags;
# 87 "./SDL3/SDL_surface.h"
typedef enum SDL_ScaleMode
{
    SDL_SCALEMODE_INVALID = -1,
    SDL_SCALEMODE_NEAREST,
    SDL_SCALEMODE_LINEAR,
    SDL_SCALEMODE_PIXELART
} SDL_ScaleMode;






typedef enum SDL_FlipMode
{
    SDL_FLIP_NONE,
    SDL_FLIP_HORIZONTAL,
    SDL_FLIP_VERTICAL,
    SDL_FLIP_HORIZONTAL_AND_VERTICAL = (SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL)
} SDL_FlipMode;
# 138 "./SDL3/SDL_surface.h"
struct SDL_Surface
{
    SDL_SurfaceFlags flags;
    SDL_PixelFormat format;
    int w;
    int h;
    int pitch;
    void *pixels;

    int refcount;

    void *reserved;
};


typedef struct SDL_Surface SDL_Surface;
# 173 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) SDL_Surface *  SDL_CreateSurface(int width, int height, SDL_PixelFormat format);
# 203 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) SDL_Surface *  SDL_CreateSurfaceFrom(int width, int height, SDL_PixelFormat format, void *pixels, int pitch);
# 219 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) void  SDL_DestroySurface(SDL_Surface *surface);
# 259 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) SDL_PropertiesID  SDL_GetSurfaceProperties(SDL_Surface *surface);
# 287 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_SetSurfaceColorspace(SDL_Surface *surface, SDL_Colorspace colorspace);
# 307 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) SDL_Colorspace  SDL_GetSurfaceColorspace(SDL_Surface *surface);
# 338 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) SDL_Palette *  SDL_CreateSurfacePalette(SDL_Surface *surface);
# 361 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_SetSurfacePalette(SDL_Surface *surface, SDL_Palette *palette);
# 376 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) SDL_Palette *  SDL_GetSurfacePalette(SDL_Surface *surface);
# 404 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_AddSurfaceAlternateImage(SDL_Surface *surface, SDL_Surface *image);
# 420 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_SurfaceHasAlternateImages(SDL_Surface *surface);
# 448 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) SDL_Surface **  SDL_GetSurfaceImages(SDL_Surface *surface, int *count);
# 467 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) void  SDL_RemoveSurfaceAlternateImages(SDL_Surface *surface);
# 495 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_LockSurface(SDL_Surface *surface);
# 510 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) void  SDL_UnlockSurface(SDL_Surface *surface);
# 531 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) SDL_Surface *  SDL_LoadSurface_IO(SDL_IOStream *src, bool closeio);
# 550 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) SDL_Surface *  SDL_LoadSurface(const char *file);
# 572 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) SDL_Surface *  SDL_LoadBMP_IO(SDL_IOStream *src, bool closeio);
# 592 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) SDL_Surface *  SDL_LoadBMP(const char *file);
# 618 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_SaveBMP_IO(SDL_Surface *surface, SDL_IOStream *dst, bool closeio);
# 642 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_SaveBMP(SDL_Surface *surface, const char *file);
# 668 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) SDL_Surface *  SDL_LoadPNG_IO(SDL_IOStream *src, bool closeio);
# 692 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) SDL_Surface *  SDL_LoadPNG(const char *file);
# 712 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_SavePNG_IO(SDL_Surface *surface, SDL_IOStream *dst, bool closeio);
# 730 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_SavePNG(SDL_Surface *surface, const char *file);
# 755 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) SDL_Surface *  SDL_LoadJPG_IO(SDL_IOStream *src, bool closeio);
# 778 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) SDL_Surface *  SDL_LoadJPG(const char *file);
# 800 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_SetSurfaceRLE(SDL_Surface *surface, bool enabled);
# 816 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_SurfaceHasRLE(SDL_Surface *surface);
# 843 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_SetSurfaceColorKey(SDL_Surface *surface, bool enabled, Uint32 key);
# 860 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_SurfaceHasColorKey(SDL_Surface *surface);
# 882 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_GetSurfaceColorKey(SDL_Surface *surface, Uint32 *key);
# 908 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_SetSurfaceColorMod(SDL_Surface *surface, Uint8 r, Uint8 g, Uint8 b);
# 929 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_GetSurfaceColorMod(SDL_Surface *surface, Uint8 *r, Uint8 *g, Uint8 *b);
# 952 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_SetSurfaceAlphaMod(SDL_Surface *surface, Uint8 alpha);
# 969 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_GetSurfaceAlphaMod(SDL_Surface *surface, Uint8 *alpha);
# 990 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_SetSurfaceBlendMode(SDL_Surface *surface, SDL_BlendMode blendMode);
# 1006 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_GetSurfaceBlendMode(SDL_Surface *surface, SDL_BlendMode *blendMode);
# 1030 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_SetSurfaceClipRect(SDL_Surface *surface, const SDL_Rect *rect);
# 1052 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_GetSurfaceClipRect(SDL_Surface *surface, SDL_Rect *rect);
# 1067 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_FlipSurface(SDL_Surface *surface, SDL_FlipMode flip);
# 1096 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) SDL_Surface *  SDL_RotateSurface(SDL_Surface *surface, float angle);
# 1117 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) SDL_Surface *  SDL_DuplicateSurface(SDL_Surface *surface);
# 1139 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) SDL_Surface *  SDL_ScaleSurface(SDL_Surface *surface, int width, int height, SDL_ScaleMode scaleMode);
# 1168 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) SDL_Surface *  SDL_ConvertSurface(SDL_Surface *surface, SDL_PixelFormat format);
# 1197 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) SDL_Surface *  SDL_ConvertSurfaceAndColorspace(SDL_Surface *surface, SDL_PixelFormat format, SDL_Palette *palette, SDL_Colorspace colorspace, SDL_PropertiesID props);
# 1221 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_ConvertPixels(int width, int height, SDL_PixelFormat src_format, const void *src, int src_pitch, SDL_PixelFormat dst_format, void *dst, int dst_pitch);
# 1254 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_ConvertPixelsAndColorspace(int width, int height, SDL_PixelFormat src_format, SDL_Colorspace src_colorspace, SDL_PropertiesID src_properties, const void *src, int src_pitch, SDL_PixelFormat dst_format, SDL_Colorspace dst_colorspace, SDL_PropertiesID dst_properties, void *dst, int dst_pitch);
# 1280 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_PremultiplyAlpha(int width, int height, SDL_PixelFormat src_format, const void *src, int src_pitch, SDL_PixelFormat dst_format, void *dst, int dst_pitch, bool linear);
# 1298 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_PremultiplySurfaceAlpha(SDL_Surface *surface, bool linear);
# 1321 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_ClearSurface(SDL_Surface *surface, float r, float g, float b, float a);
# 1349 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_FillSurfaceRect(SDL_Surface *dst, const SDL_Rect *rect, Uint32 color);
# 1377 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_FillSurfaceRects(SDL_Surface *dst, const SDL_Rect *rects, int count, Uint32 color);
# 1450 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_BlitSurface(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, const SDL_Rect *dstrect);
# 1474 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_BlitSurfaceUnchecked(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, const SDL_Rect *dstrect);
# 1498 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_BlitSurfaceScaled(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, const SDL_Rect *dstrect, SDL_ScaleMode scaleMode);
# 1523 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_BlitSurfaceUncheckedScaled(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, const SDL_Rect *dstrect, SDL_ScaleMode scaleMode);
# 1546 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_StretchSurface(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, const SDL_Rect *dstrect, SDL_ScaleMode scaleMode);
# 1571 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_BlitSurfaceTiled(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, const SDL_Rect *dstrect);
# 1600 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_BlitSurfaceTiledWithScale(SDL_Surface *src, const SDL_Rect *srcrect, float scale, SDL_ScaleMode scaleMode, SDL_Surface *dst, const SDL_Rect *dstrect);
# 1636 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_BlitSurface9Grid(SDL_Surface *src, const SDL_Rect *srcrect, int left_width, int right_width, int top_height, int bottom_height, float scale, SDL_ScaleMode scaleMode, SDL_Surface *dst, const SDL_Rect *dstrect);
# 1669 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) Uint32  SDL_MapSurfaceRGB(SDL_Surface *surface, Uint8 r, Uint8 g, Uint8 b);
# 1703 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) Uint32  SDL_MapSurfaceRGBA(SDL_Surface *surface, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
# 1733 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_ReadSurfacePixel(SDL_Surface *surface, int x, int y, Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a);
# 1760 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_ReadSurfacePixelFloat(SDL_Surface *surface, int x, int y, float *r, float *g, float *b, float *a);
# 1786 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_WriteSurfacePixel(SDL_Surface *surface, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
# 1809 "./SDL3/SDL_surface.h"
extern __attribute__((dllimport)) bool  SDL_WriteSurfacePixelFloat(SDL_Surface *surface, int x, int y, float r, float g, float b, float a);






# 1 "./SDL3/SDL_close_code.h" 1
# 1816 "./SDL3/SDL_surface.h" 2
# 76 "./SDL3/SDL_camera.h" 2


# 1 "./SDL3/SDL_begin_code.h" 1
# 78 "./SDL3/SDL_camera.h" 2
# 95 "./SDL3/SDL_camera.h"
typedef Uint32 SDL_CameraID;






typedef struct SDL_Camera SDL_Camera;
# 115 "./SDL3/SDL_camera.h"
typedef struct SDL_CameraSpec
{
    SDL_PixelFormat format;
    SDL_Colorspace colorspace;
    int width;
    int height;
    int framerate_numerator;
    int framerate_denominator;
} SDL_CameraSpec;
# 132 "./SDL3/SDL_camera.h"
typedef enum SDL_CameraPosition
{
    SDL_CAMERA_POSITION_UNKNOWN,
    SDL_CAMERA_POSITION_FRONT_FACING,
    SDL_CAMERA_POSITION_BACK_FACING
} SDL_CameraPosition;
# 146 "./SDL3/SDL_camera.h"
typedef enum SDL_CameraPermissionState
{
    SDL_CAMERA_PERMISSION_STATE_DENIED = -1,
    SDL_CAMERA_PERMISSION_STATE_PENDING,
    SDL_CAMERA_PERMISSION_STATE_APPROVED
} SDL_CameraPermissionState;
# 175 "./SDL3/SDL_camera.h"
extern __attribute__((dllimport)) int  SDL_GetNumCameraDrivers(void);
# 199 "./SDL3/SDL_camera.h"
extern __attribute__((dllimport)) const char *  SDL_GetCameraDriver(int index);
# 215 "./SDL3/SDL_camera.h"
extern __attribute__((dllimport)) const char *  SDL_GetCurrentCameraDriver(void);
# 232 "./SDL3/SDL_camera.h"
extern __attribute__((dllimport)) SDL_CameraID *  SDL_GetCameras(int *count);
# 271 "./SDL3/SDL_camera.h"
extern __attribute__((dllimport)) SDL_CameraSpec **  SDL_GetCameraSupportedFormats(SDL_CameraID instance_id, int *count);
# 286 "./SDL3/SDL_camera.h"
extern __attribute__((dllimport)) const char *  SDL_GetCameraName(SDL_CameraID instance_id);
# 305 "./SDL3/SDL_camera.h"
extern __attribute__((dllimport)) SDL_CameraPosition  SDL_GetCameraPosition(SDL_CameraID instance_id);
# 352 "./SDL3/SDL_camera.h"
extern __attribute__((dllimport)) SDL_Camera *  SDL_OpenCamera(SDL_CameraID instance_id, const SDL_CameraSpec *spec);
# 387 "./SDL3/SDL_camera.h"
extern __attribute__((dllimport)) SDL_CameraPermissionState  SDL_GetCameraPermissionState(SDL_Camera *camera);
# 402 "./SDL3/SDL_camera.h"
extern __attribute__((dllimport)) SDL_CameraID  SDL_GetCameraID(SDL_Camera *camera);
# 415 "./SDL3/SDL_camera.h"
extern __attribute__((dllimport)) SDL_PropertiesID  SDL_GetCameraProperties(SDL_Camera *camera);
# 441 "./SDL3/SDL_camera.h"
extern __attribute__((dllimport)) bool  SDL_GetCameraFormat(SDL_Camera *camera, SDL_CameraSpec *spec);
# 484 "./SDL3/SDL_camera.h"
extern __attribute__((dllimport)) SDL_Surface *  SDL_AcquireCameraFrame(SDL_Camera *camera, Uint64 *timestampNS);
# 512 "./SDL3/SDL_camera.h"
extern __attribute__((dllimport)) void  SDL_ReleaseCameraFrame(SDL_Camera *camera, SDL_Surface *frame);
# 527 "./SDL3/SDL_camera.h"
extern __attribute__((dllimport)) void  SDL_CloseCamera(SDL_Camera *camera);






# 1 "./SDL3/SDL_close_code.h" 1
# 534 "./SDL3/SDL_camera.h" 2
# 42 "SDL3/SDL.h" 2

# 1 "./SDL3/SDL_clipboard.h" 1
# 83 "./SDL3/SDL_clipboard.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 83 "./SDL3/SDL_clipboard.h" 2
# 104 "./SDL3/SDL_clipboard.h"
extern __attribute__((dllimport)) bool  SDL_SetClipboardText(const char *text);
# 123 "./SDL3/SDL_clipboard.h"
extern __attribute__((dllimport)) char *  SDL_GetClipboardText(void);
# 137 "./SDL3/SDL_clipboard.h"
extern __attribute__((dllimport)) bool  SDL_HasClipboardText(void);
# 153 "./SDL3/SDL_clipboard.h"
extern __attribute__((dllimport)) bool  SDL_SetPrimarySelectionText(const char *text);
# 172 "./SDL3/SDL_clipboard.h"
extern __attribute__((dllimport)) char *  SDL_GetPrimarySelectionText(void);
# 187 "./SDL3/SDL_clipboard.h"
extern __attribute__((dllimport)) bool  SDL_HasPrimarySelectionText(void);
# 210 "./SDL3/SDL_clipboard.h"
typedef const void *( *SDL_ClipboardDataCallback)(void *userdata, const char *mime_type, uint64 *size);
# 222 "./SDL3/SDL_clipboard.h"
typedef void ( *SDL_ClipboardCleanupCallback)(void *userdata);
# 255 "./SDL3/SDL_clipboard.h"
extern __attribute__((dllimport)) bool  SDL_SetClipboardData(SDL_ClipboardDataCallback callback, SDL_ClipboardCleanupCallback cleanup, void *userdata, const char *const *mime_types, uint64 num_mime_types);
# 269 "./SDL3/SDL_clipboard.h"
extern __attribute__((dllimport)) bool  SDL_ClearClipboardData(void);
# 290 "./SDL3/SDL_clipboard.h"
extern __attribute__((dllimport)) void *  SDL_GetClipboardData(const char *mime_type, uint64 *size);
# 306 "./SDL3/SDL_clipboard.h"
extern __attribute__((dllimport)) bool  SDL_HasClipboardData(const char *mime_type);
# 323 "./SDL3/SDL_clipboard.h"
extern __attribute__((dllimport)) char **  SDL_GetClipboardMimeTypes(uint64 *num_mime_types);






# 1 "./SDL3/SDL_close_code.h" 1
# 330 "./SDL3/SDL_clipboard.h" 2
# 43 "SDL3/SDL.h" 2

# 1 "./SDL3/SDL_cpuinfo.h" 1
# 46 "./SDL3/SDL_cpuinfo.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 46 "./SDL3/SDL_cpuinfo.h" 2
# 73 "./SDL3/SDL_cpuinfo.h"
extern __attribute__((dllimport)) int  SDL_GetNumLogicalCPUCores(void);
# 87 "./SDL3/SDL_cpuinfo.h"
extern __attribute__((dllimport)) int  SDL_GetCPUCacheLineSize(void);
# 101 "./SDL3/SDL_cpuinfo.h"
extern __attribute__((dllimport)) bool  SDL_HasAltiVec(void);
# 114 "./SDL3/SDL_cpuinfo.h"
extern __attribute__((dllimport)) bool  SDL_HasMMX(void);
# 132 "./SDL3/SDL_cpuinfo.h"
extern __attribute__((dllimport)) bool  SDL_HasSSE(void);
# 150 "./SDL3/SDL_cpuinfo.h"
extern __attribute__((dllimport)) bool  SDL_HasSSE2(void);
# 168 "./SDL3/SDL_cpuinfo.h"
extern __attribute__((dllimport)) bool  SDL_HasSSE3(void);
# 186 "./SDL3/SDL_cpuinfo.h"
extern __attribute__((dllimport)) bool  SDL_HasSSE41(void);
# 204 "./SDL3/SDL_cpuinfo.h"
extern __attribute__((dllimport)) bool  SDL_HasSSE42(void);
# 220 "./SDL3/SDL_cpuinfo.h"
extern __attribute__((dllimport)) bool  SDL_HasAVX(void);
# 236 "./SDL3/SDL_cpuinfo.h"
extern __attribute__((dllimport)) bool  SDL_HasAVX2(void);
# 252 "./SDL3/SDL_cpuinfo.h"
extern __attribute__((dllimport)) bool  SDL_HasAVX512F(void);
# 269 "./SDL3/SDL_cpuinfo.h"
extern __attribute__((dllimport)) bool  SDL_HasARMSIMD(void);
# 282 "./SDL3/SDL_cpuinfo.h"
extern __attribute__((dllimport)) bool  SDL_HasNEON(void);
# 294 "./SDL3/SDL_cpuinfo.h"
extern __attribute__((dllimport)) bool  SDL_HasSVE2(void);
# 308 "./SDL3/SDL_cpuinfo.h"
extern __attribute__((dllimport)) bool  SDL_HasLSX(void);
# 322 "./SDL3/SDL_cpuinfo.h"
extern __attribute__((dllimport)) bool  SDL_HasLASX(void);
# 333 "./SDL3/SDL_cpuinfo.h"
extern __attribute__((dllimport)) int  SDL_GetSystemRAM(void);
# 357 "./SDL3/SDL_cpuinfo.h"
extern __attribute__((dllimport)) uint64  SDL_GetSIMDAlignment(void);
# 378 "./SDL3/SDL_cpuinfo.h"
extern __attribute__((dllimport)) int  SDL_GetSystemPageSize(void);






# 1 "./SDL3/SDL_close_code.h" 1
# 385 "./SDL3/SDL_cpuinfo.h" 2
# 44 "SDL3/SDL.h" 2

# 1 "./SDL3/SDL_dialog.h" 1
# 44 "./SDL3/SDL_dialog.h"
# 1 "./SDL3/SDL_video.h" 1
# 60 "./SDL3/SDL_video.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 60 "./SDL3/SDL_video.h" 2
# 75 "./SDL3/SDL_video.h"
typedef Uint32 SDL_DisplayID;
# 84 "./SDL3/SDL_video.h"
typedef Uint32 SDL_WindowID;
# 110 "./SDL3/SDL_video.h"
typedef enum SDL_SystemTheme
{
    SDL_SYSTEM_THEME_UNKNOWN,
    SDL_SYSTEM_THEME_LIGHT,
    SDL_SYSTEM_THEME_DARK
} SDL_SystemTheme;
# 126 "./SDL3/SDL_video.h"
typedef struct SDL_DisplayModeData SDL_DisplayModeData;
# 139 "./SDL3/SDL_video.h"
typedef struct SDL_DisplayMode
{
    SDL_DisplayID displayID;
    SDL_PixelFormat format;
    int w;
    int h;
    float pixel_density;
    float refresh_rate;
    int refresh_rate_numerator;
    int refresh_rate_denominator;

    SDL_DisplayModeData *internal;

} SDL_DisplayMode;






typedef enum SDL_DisplayOrientation
{
    SDL_ORIENTATION_UNKNOWN,
    SDL_ORIENTATION_LANDSCAPE,
    SDL_ORIENTATION_LANDSCAPE_FLIPPED,
    SDL_ORIENTATION_PORTRAIT,
    SDL_ORIENTATION_PORTRAIT_FLIPPED
} SDL_DisplayOrientation;
# 175 "./SDL3/SDL_video.h"
typedef struct SDL_Window SDL_Window;
# 195 "./SDL3/SDL_video.h"
typedef Uint64 SDL_WindowFlags;
# 328 "./SDL3/SDL_video.h"
typedef enum SDL_FlashOperation
{
    SDL_FLASH_CANCEL,
    SDL_FLASH_BRIEFLY,
    SDL_FLASH_UNTIL_FOCUSED
} SDL_FlashOperation;






typedef enum SDL_ProgressState
{
    SDL_PROGRESS_STATE_INVALID = -1,
    SDL_PROGRESS_STATE_NONE,
    SDL_PROGRESS_STATE_INDETERMINATE,
    SDL_PROGRESS_STATE_NORMAL,
    SDL_PROGRESS_STATE_PAUSED,
    SDL_PROGRESS_STATE_ERROR
} SDL_ProgressState;
# 360 "./SDL3/SDL_video.h"
typedef struct SDL_GLContextState *SDL_GLContext;






typedef void *SDL_EGLDisplay;






typedef void *SDL_EGLConfig;






typedef void *SDL_EGLSurface;






typedef int64 SDL_EGLAttrib;






typedef int SDL_EGLint;
# 420 "./SDL3/SDL_video.h"
typedef SDL_EGLAttrib *( *SDL_EGLAttribArrayCallback)(void *userdata);
# 451 "./SDL3/SDL_video.h"
typedef SDL_EGLint *( *SDL_EGLIntArrayCallback)(void *userdata, SDL_EGLDisplay display, SDL_EGLConfig config);
# 470 "./SDL3/SDL_video.h"
typedef enum SDL_GLAttr
{
    SDL_GL_RED_SIZE,
    SDL_GL_GREEN_SIZE,
    SDL_GL_BLUE_SIZE,
    SDL_GL_ALPHA_SIZE,
    SDL_GL_BUFFER_SIZE,
    SDL_GL_DOUBLEBUFFER,
    SDL_GL_DEPTH_SIZE,
    SDL_GL_STENCIL_SIZE,
    SDL_GL_ACCUM_RED_SIZE,
    SDL_GL_ACCUM_GREEN_SIZE,
    SDL_GL_ACCUM_BLUE_SIZE,
    SDL_GL_ACCUM_ALPHA_SIZE,
    SDL_GL_STEREO,
    SDL_GL_MULTISAMPLEBUFFERS,
    SDL_GL_MULTISAMPLESAMPLES,
    SDL_GL_ACCELERATED_VISUAL,
    SDL_GL_RETAINED_BACKING,
    SDL_GL_CONTEXT_MAJOR_VERSION,
    SDL_GL_CONTEXT_MINOR_VERSION,
    SDL_GL_CONTEXT_FLAGS,
    SDL_GL_CONTEXT_PROFILE_MASK,
    SDL_GL_SHARE_WITH_CURRENT_CONTEXT,
    SDL_GL_FRAMEBUFFER_SRGB_CAPABLE,
    SDL_GL_CONTEXT_RELEASE_BEHAVIOR,
    SDL_GL_CONTEXT_RESET_NOTIFICATION,
    SDL_GL_CONTEXT_NO_ERROR,
    SDL_GL_FLOATBUFFERS,
    SDL_GL_EGL_PLATFORM
} SDL_GLAttr;






typedef Uint32 SDL_GLProfile;
# 519 "./SDL3/SDL_video.h"
typedef Uint32 SDL_GLContextFlag;
# 533 "./SDL3/SDL_video.h"
typedef Uint32 SDL_GLContextReleaseFlag;
# 544 "./SDL3/SDL_video.h"
typedef Uint32 SDL_GLContextResetNotification;
# 563 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) int  SDL_GetNumVideoDrivers(void);
# 585 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) const char *  SDL_GetVideoDriver(int index);
# 604 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) const char *  SDL_GetCurrentVideoDriver(void);
# 615 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_SystemTheme  SDL_GetSystemTheme(void);
# 630 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_DisplayID *  SDL_GetDisplays(int *count);
# 644 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_DisplayID  SDL_GetPrimaryDisplay(void);
# 682 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_PropertiesID  SDL_GetDisplayProperties(SDL_DisplayID displayID);
# 702 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) const char *  SDL_GetDisplayName(SDL_DisplayID displayID);
# 722 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_GetDisplayBounds(SDL_DisplayID displayID, SDL_Rect *rect);
# 748 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_GetDisplayUsableBounds(SDL_DisplayID displayID, SDL_Rect *rect);
# 763 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_DisplayOrientation  SDL_GetNaturalDisplayOrientation(SDL_DisplayID displayID);
# 778 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_DisplayOrientation  SDL_GetCurrentDisplayOrientation(SDL_DisplayID displayID);
# 805 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) float  SDL_GetDisplayContentScale(SDL_DisplayID displayID);
# 833 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_DisplayMode **  SDL_GetFullscreenDisplayModes(SDL_DisplayID displayID, int *count);
# 864 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_GetClosestFullscreenDisplayMode(SDL_DisplayID displayID, int w, int h, float refresh_rate, bool include_high_density_modes, SDL_DisplayMode *closest);
# 885 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) const SDL_DisplayMode *  SDL_GetDesktopDisplayMode(SDL_DisplayID displayID);
# 906 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) const SDL_DisplayMode *  SDL_GetCurrentDisplayMode(SDL_DisplayID displayID);
# 922 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_DisplayID  SDL_GetDisplayForPoint(const SDL_Point *point);
# 939 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_DisplayID  SDL_GetDisplayForRect(const SDL_Rect *rect);
# 956 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_DisplayID  SDL_GetDisplayForWindow(SDL_Window *window);
# 975 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) float  SDL_GetWindowPixelDensity(SDL_Window *window);
# 999 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) float  SDL_GetWindowDisplayScale(SDL_Window *window);
# 1034 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_SetWindowFullscreenMode(SDL_Window *window, const SDL_DisplayMode *mode);
# 1050 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) const SDL_DisplayMode *  SDL_GetWindowFullscreenMode(SDL_Window *window);
# 1065 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) void *  SDL_GetWindowICCProfile(SDL_Window *window, uint64 *size);
# 1079 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_PixelFormat  SDL_GetWindowPixelFormat(SDL_Window *window);
# 1095 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_Window **  SDL_GetWindows(int *count);
# 1184 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_Window *  SDL_CreateWindow(const char *title, int w, int h, SDL_WindowFlags flags);
# 1260 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_Window *  SDL_CreatePopupWindow(SDL_Window *parent, int offset_x, int offset_y, int w, int h, SDL_WindowFlags flags);
# 1419 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_Window *  SDL_CreateWindowWithProperties(SDL_PropertiesID props);
# 1477 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_WindowID  SDL_GetWindowID(SDL_Window *window);
# 1495 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_Window *  SDL_GetWindowFromID(SDL_WindowID id);
# 1510 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_Window *  SDL_GetWindowParent(SDL_Window *window);
# 1652 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_PropertiesID  SDL_GetWindowProperties(SDL_Window *window);
# 1714 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_WindowFlags  SDL_GetWindowFlags(SDL_Window *window);
# 1732 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_SetWindowTitle(SDL_Window *window, const char *title);
# 1747 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) const char *  SDL_GetWindowTitle(SDL_Window *window);
# 1774 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_SetWindowIcon(SDL_Window *window, SDL_Surface *icon);
# 1815 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_SetWindowPosition(SDL_Window *window, int x, int y);
# 1840 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_GetWindowPosition(SDL_Window *window, int *x, int *y);
# 1877 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_SetWindowSize(SDL_Window *window, int w, int h);
# 1901 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_GetWindowSize(SDL_Window *window, int *w, int *h);
# 1923 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_GetWindowSafeArea(SDL_Window *window, SDL_Rect *rect);
# 1964 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_SetWindowAspectRatio(SDL_Window *window, float min_aspect, float max_aspect);
# 1983 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_GetWindowAspectRatio(SDL_Window *window, float *min_aspect, float *max_aspect);
# 2020 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_GetWindowBordersSize(SDL_Window *window, int *top, int *left, int *bottom, int *right);
# 2040 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_GetWindowSizeInPixels(SDL_Window *window, int *w, int *h);
# 2058 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_SetWindowMinimumSize(SDL_Window *window, int min_w, int min_h);
# 2078 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_GetWindowMinimumSize(SDL_Window *window, int *w, int *h);
# 2096 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_SetWindowMaximumSize(SDL_Window *window, int max_w, int max_h);
# 2116 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_GetWindowMaximumSize(SDL_Window *window, int *w, int *h);
# 2138 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_SetWindowBordered(SDL_Window *window, bool bordered);
# 2160 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_SetWindowResizable(SDL_Window *window, bool resizable);
# 2179 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_SetWindowAlwaysOnTop(SDL_Window *window, bool on_top);
# 2210 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_SetWindowFillDocument(SDL_Window *window, bool fill);
# 2226 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_ShowWindow(SDL_Window *window);
# 2242 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_HideWindow(SDL_Window *window);
# 2262 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_RaiseWindow(SDL_Window *window);
# 2296 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_MaximizeWindow(SDL_Window *window);
# 2325 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_MinimizeWindow(SDL_Window *window);
# 2355 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_RestoreWindow(SDL_Window *window);
# 2387 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_SetWindowFullscreen(SDL_Window *window, bool fullscreen);
# 2418 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_SyncWindow(SDL_Window *window);
# 2433 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_WindowHasSurface(SDL_Window *window);
# 2462 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_Surface *  SDL_GetWindowSurface(SDL_Window *window);
# 2488 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_SetWindowSurfaceVSync(SDL_Window *window, int vsync);
# 2508 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_GetWindowSurfaceVSync(SDL_Window *window, int *vsync);
# 2529 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_UpdateWindowSurface(SDL_Window *window);
# 2558 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_UpdateWindowSurfaceRects(SDL_Window *window, const SDL_Rect *rects, int numrects);
# 2574 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_DestroyWindowSurface(SDL_Window *window);
# 2607 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_SetWindowKeyboardGrab(SDL_Window *window, bool grabbed);
# 2627 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_SetWindowMouseGrab(SDL_Window *window, bool grabbed);
# 2641 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_GetWindowKeyboardGrab(SDL_Window *window);
# 2658 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_GetWindowMouseGrab(SDL_Window *window);
# 2672 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_Window *  SDL_GetGrabbedWindow(void);
# 2694 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_SetWindowMouseRect(SDL_Window *window, const SDL_Rect *rect);
# 2711 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) const SDL_Rect *  SDL_GetWindowMouseRect(SDL_Window *window);
# 2732 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_SetWindowOpacity(SDL_Window *window, float opacity);
# 2750 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) float  SDL_GetWindowOpacity(SDL_Window *window);
# 2784 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_SetWindowParent(SDL_Window *window, SDL_Window *parent);
# 2804 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_SetWindowModal(SDL_Window *window, bool modal);
# 2818 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_SetWindowFocusable(SDL_Window *window, bool focusable);
# 2844 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_ShowWindowSystemMenu(SDL_Window *window, int x, int y);
# 2855 "./SDL3/SDL_video.h"
typedef enum SDL_HitTestResult
{
    SDL_HITTEST_NORMAL,
    SDL_HITTEST_DRAGGABLE,
    SDL_HITTEST_RESIZE_TOPLEFT,
    SDL_HITTEST_RESIZE_TOP,
    SDL_HITTEST_RESIZE_TOPRIGHT,
    SDL_HITTEST_RESIZE_RIGHT,
    SDL_HITTEST_RESIZE_BOTTOMRIGHT,
    SDL_HITTEST_RESIZE_BOTTOM,
    SDL_HITTEST_RESIZE_BOTTOMLEFT,
    SDL_HITTEST_RESIZE_LEFT
} SDL_HitTestResult;
# 2879 "./SDL3/SDL_video.h"
typedef SDL_HitTestResult ( *SDL_HitTest)(SDL_Window *win,
                                                 const SDL_Point *area,
                                                 void *data);
# 2925 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_SetWindowHitTest(SDL_Window *window, SDL_HitTest callback, void *callback_data);
# 2953 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_SetWindowShape(SDL_Window *window, SDL_Surface *shape);
# 2967 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_FlashWindow(SDL_Window *window, SDL_FlashOperation operation);
# 2982 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_SetWindowProgressState(SDL_Window *window, SDL_ProgressState state);
# 2995 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_ProgressState  SDL_GetWindowProgressState(SDL_Window *window);
# 3010 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_SetWindowProgressValue(SDL_Window *window, float value);
# 3023 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) float  SDL_GetWindowProgressValue(SDL_Window *window);
# 3045 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) void  SDL_DestroyWindow(SDL_Window *window);
# 3064 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_ScreenSaverEnabled(void);
# 3079 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_EnableScreenSaver(void);
# 3100 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_DisableScreenSaver(void);
# 3130 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_GL_LoadLibrary(const char *path);
# 3185 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_FunctionPointer  SDL_GL_GetProcAddress(const char *proc);
# 3204 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_FunctionPointer  SDL_EGL_GetProcAddress(const char *proc);
# 3215 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) void  SDL_GL_UnloadLibrary(void);
# 3238 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_GL_ExtensionSupported(const char *extension);
# 3250 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) void  SDL_GL_ResetAttributes(void);
# 3273 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_GL_SetAttribute(SDL_GLAttr attr, int value);
# 3291 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_GL_GetAttribute(SDL_GLAttr attr, int *value);
# 3321 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_GLContext  SDL_GL_CreateContext(SDL_Window *window);
# 3339 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_GL_MakeCurrent(SDL_Window *window, SDL_GLContext context);
# 3351 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_Window *  SDL_GL_GetCurrentWindow(void);
# 3365 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_GLContext  SDL_GL_GetCurrentContext(void);
# 3377 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_EGLDisplay  SDL_EGL_GetCurrentDisplay(void);
# 3389 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_EGLConfig  SDL_EGL_GetCurrentConfig(void);
# 3402 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) SDL_EGLSurface  SDL_EGL_GetWindowSurface(SDL_Window *window);
# 3424 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) void  SDL_EGL_SetAttributeCallbacks(SDL_EGLAttribArrayCallback platformAttribCallback,
                                                               SDL_EGLIntArrayCallback surfaceAttribCallback,
                                                               SDL_EGLIntArrayCallback contextAttribCallback, void *userdata);
# 3457 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_GL_SetSwapInterval(int interval);
# 3478 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_GL_GetSwapInterval(int *interval);
# 3498 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_GL_SwapWindow(SDL_Window *window);
# 3513 "./SDL3/SDL_video.h"
extern __attribute__((dllimport)) bool  SDL_GL_DestroyContext(SDL_GLContext context);
# 3523 "./SDL3/SDL_video.h"
# 1 "./SDL3/SDL_close_code.h" 1
# 3523 "./SDL3/SDL_video.h" 2
# 44 "./SDL3/SDL_dialog.h" 2


# 1 "./SDL3/SDL_begin_code.h" 1
# 46 "./SDL3/SDL_dialog.h" 2
# 70 "./SDL3/SDL_dialog.h"
typedef struct SDL_DialogFileFilter
{
    const char *name;
    const char *pattern;
} SDL_DialogFileFilter;
# 113 "./SDL3/SDL_dialog.h"
typedef void ( *SDL_DialogFileCallback)(void *userdata, const char * const *filelist, int filter);
# 164 "./SDL3/SDL_dialog.h"
extern __attribute__((dllimport)) void  SDL_ShowOpenFileDialog(SDL_DialogFileCallback callback, void *userdata, SDL_Window *window, const SDL_DialogFileFilter *filters, int nfilters, const char *default_location, bool allow_many);
# 213 "./SDL3/SDL_dialog.h"
extern __attribute__((dllimport)) void  SDL_ShowSaveFileDialog(SDL_DialogFileCallback callback, void *userdata, SDL_Window *window, const SDL_DialogFileFilter *filters, int nfilters, const char *default_location);
# 258 "./SDL3/SDL_dialog.h"
extern __attribute__((dllimport)) void  SDL_ShowOpenFolderDialog(SDL_DialogFileCallback callback, void *userdata, SDL_Window *window, const char *default_location, bool allow_many);
# 270 "./SDL3/SDL_dialog.h"
typedef enum SDL_FileDialogType
{
    SDL_FILEDIALOG_OPENFILE,
    SDL_FILEDIALOG_SAVEFILE,
    SDL_FILEDIALOG_OPENFOLDER
} SDL_FileDialogType;
# 324 "./SDL3/SDL_dialog.h"
extern __attribute__((dllimport)) void  SDL_ShowFileDialogWithProperties(SDL_FileDialogType type, SDL_DialogFileCallback callback, void *userdata, SDL_PropertiesID props);
# 340 "./SDL3/SDL_dialog.h"
# 1 "./SDL3/SDL_close_code.h" 1
# 340 "./SDL3/SDL_dialog.h" 2
# 45 "SDL3/SDL.h" 2

# 1 "./SDL3/SDL_dlopennote.h" 1
# 46 "SDL3/SDL.h" 2



# 1 "./SDL3/SDL_events.h" 1
# 60 "./SDL3/SDL_events.h"
# 1 "./SDL3/SDL_gamepad.h" 1
# 83 "./SDL3/SDL_gamepad.h"
# 1 "./SDL3/SDL_guid.h" 1
# 39 "./SDL3/SDL_guid.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 39 "./SDL3/SDL_guid.h" 2
# 61 "./SDL3/SDL_guid.h"
typedef struct SDL_GUID {
    Uint8 data[16];
} SDL_GUID;
# 80 "./SDL3/SDL_guid.h"
extern __attribute__((dllimport)) void  SDL_GUIDToString(SDL_GUID guid, char *pszGUID, int cbGUID);
# 98 "./SDL3/SDL_guid.h"
extern __attribute__((dllimport)) SDL_GUID  SDL_StringToGUID(const char *pchGUID);






# 1 "./SDL3/SDL_close_code.h" 1
# 105 "./SDL3/SDL_guid.h" 2
# 83 "./SDL3/SDL_gamepad.h" 2


# 1 "./SDL3/SDL_joystick.h" 1
# 69 "./SDL3/SDL_joystick.h"
# 1 "./SDL3/SDL_power.h" 1
# 44 "./SDL3/SDL_power.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 44 "./SDL3/SDL_power.h" 2
# 56 "./SDL3/SDL_power.h"
typedef enum SDL_PowerState
{
    SDL_POWERSTATE_ERROR = -1,
    SDL_POWERSTATE_UNKNOWN,
    SDL_POWERSTATE_ON_BATTERY,
    SDL_POWERSTATE_NO_BATTERY,
    SDL_POWERSTATE_CHARGING,
    SDL_POWERSTATE_CHARGED
} SDL_PowerState;
# 100 "./SDL3/SDL_power.h"
extern __attribute__((dllimport)) SDL_PowerState  SDL_GetPowerInfo(int *seconds, int *percent);






# 1 "./SDL3/SDL_close_code.h" 1
# 107 "./SDL3/SDL_power.h" 2
# 69 "./SDL3/SDL_joystick.h" 2


# 1 "./SDL3/SDL_sensor.h" 1
# 42 "./SDL3/SDL_sensor.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 42 "./SDL3/SDL_sensor.h" 2
# 54 "./SDL3/SDL_sensor.h"
typedef struct SDL_Sensor SDL_Sensor;
# 64 "./SDL3/SDL_sensor.h"
typedef Uint32 SDL_SensorID;
# 132 "./SDL3/SDL_sensor.h"
typedef enum SDL_SensorType
{
    SDL_SENSOR_INVALID = -1,
    SDL_SENSOR_UNKNOWN,
    SDL_SENSOR_ACCEL,
    SDL_SENSOR_GYRO,
    SDL_SENSOR_ACCEL_L,
    SDL_SENSOR_GYRO_L,
    SDL_SENSOR_ACCEL_R,
    SDL_SENSOR_GYRO_R,
    SDL_SENSOR_COUNT
} SDL_SensorType;
# 159 "./SDL3/SDL_sensor.h"
extern __attribute__((dllimport)) SDL_SensorID *  SDL_GetSensors(int *count);
# 171 "./SDL3/SDL_sensor.h"
extern __attribute__((dllimport)) const char *  SDL_GetSensorNameForID(SDL_SensorID instance_id);
# 184 "./SDL3/SDL_sensor.h"
extern __attribute__((dllimport)) SDL_SensorType  SDL_GetSensorTypeForID(SDL_SensorID instance_id);
# 197 "./SDL3/SDL_sensor.h"
extern __attribute__((dllimport)) int  SDL_GetSensorNonPortableTypeForID(SDL_SensorID instance_id);
# 208 "./SDL3/SDL_sensor.h"
extern __attribute__((dllimport)) SDL_Sensor *  SDL_OpenSensor(SDL_SensorID instance_id);
# 219 "./SDL3/SDL_sensor.h"
extern __attribute__((dllimport)) SDL_Sensor *  SDL_GetSensorFromID(SDL_SensorID instance_id);
# 230 "./SDL3/SDL_sensor.h"
extern __attribute__((dllimport)) SDL_PropertiesID  SDL_GetSensorProperties(SDL_Sensor *sensor);
# 241 "./SDL3/SDL_sensor.h"
extern __attribute__((dllimport)) const char *  SDL_GetSensorName(SDL_Sensor *sensor);
# 252 "./SDL3/SDL_sensor.h"
extern __attribute__((dllimport)) SDL_SensorType  SDL_GetSensorType(SDL_Sensor *sensor);
# 262 "./SDL3/SDL_sensor.h"
extern __attribute__((dllimport)) int  SDL_GetSensorNonPortableType(SDL_Sensor *sensor);
# 273 "./SDL3/SDL_sensor.h"
extern __attribute__((dllimport)) SDL_SensorID  SDL_GetSensorID(SDL_Sensor *sensor);
# 288 "./SDL3/SDL_sensor.h"
extern __attribute__((dllimport)) bool  SDL_GetSensorData(SDL_Sensor *sensor, float *data, int num_values);
# 297 "./SDL3/SDL_sensor.h"
extern __attribute__((dllimport)) void  SDL_CloseSensor(SDL_Sensor *sensor);
# 310 "./SDL3/SDL_sensor.h"
extern __attribute__((dllimport)) void  SDL_UpdateSensors(void);
# 320 "./SDL3/SDL_sensor.h"
# 1 "./SDL3/SDL_close_code.h" 1
# 320 "./SDL3/SDL_sensor.h" 2
# 71 "./SDL3/SDL_joystick.h" 2


# 1 "./SDL3/SDL_begin_code.h" 1
# 73 "./SDL3/SDL_joystick.h" 2
# 94 "./SDL3/SDL_joystick.h"
typedef struct SDL_Joystick SDL_Joystick;
# 106 "./SDL3/SDL_joystick.h"
typedef Uint32 SDL_JoystickID;
# 124 "./SDL3/SDL_joystick.h"
typedef enum SDL_JoystickType
{
    SDL_JOYSTICK_TYPE_UNKNOWN,
    SDL_JOYSTICK_TYPE_GAMEPAD,
    SDL_JOYSTICK_TYPE_WHEEL,
    SDL_JOYSTICK_TYPE_ARCADE_STICK,
    SDL_JOYSTICK_TYPE_FLIGHT_STICK,
    SDL_JOYSTICK_TYPE_DANCE_PAD,
    SDL_JOYSTICK_TYPE_GUITAR,
    SDL_JOYSTICK_TYPE_DRUM_KIT,
    SDL_JOYSTICK_TYPE_ARCADE_PAD,
    SDL_JOYSTICK_TYPE_THROTTLE,
    SDL_JOYSTICK_TYPE_COUNT
} SDL_JoystickType;
# 147 "./SDL3/SDL_joystick.h"
typedef enum SDL_JoystickConnectionState
{
    SDL_JOYSTICK_CONNECTION_INVALID = -1,
    SDL_JOYSTICK_CONNECTION_UNKNOWN,
    SDL_JOYSTICK_CONNECTION_WIRED,
    SDL_JOYSTICK_CONNECTION_WIRELESS
} SDL_JoystickConnectionState;
# 189 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) void  SDL_LockJoysticks(void) ;
# 204 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) bool  SDL_TryLockJoysticks(void) ;
# 214 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) void  SDL_UnlockJoysticks(void) ;
# 227 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) bool  SDL_HasJoystick(void);
# 245 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) SDL_JoystickID *  SDL_GetJoysticks(int *count);
# 263 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) const char *  SDL_GetJoystickNameForID(SDL_JoystickID instance_id);
# 281 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) const char *  SDL_GetJoystickPathForID(SDL_JoystickID instance_id);
# 298 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) int  SDL_GetJoystickPlayerIndexForID(SDL_JoystickID instance_id);
# 316 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) SDL_GUID  SDL_GetJoystickGUIDForID(SDL_JoystickID instance_id);
# 335 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) Uint16  SDL_GetJoystickVendorForID(SDL_JoystickID instance_id);
# 354 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) Uint16  SDL_GetJoystickProductForID(SDL_JoystickID instance_id);
# 373 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) Uint16  SDL_GetJoystickProductVersionForID(SDL_JoystickID instance_id);
# 392 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) SDL_JoystickType  SDL_GetJoystickTypeForID(SDL_JoystickID instance_id);
# 410 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) SDL_Joystick *  SDL_OpenJoystick(SDL_JoystickID instance_id);
# 423 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) SDL_Joystick *  SDL_GetJoystickFromID(SDL_JoystickID instance_id);
# 439 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) SDL_Joystick *  SDL_GetJoystickFromPlayerIndex(int player_index);
# 448 "./SDL3/SDL_joystick.h"
typedef struct SDL_VirtualJoystickTouchpadDesc
{
    Uint16 nfingers;
    Uint16 padding[3];
} SDL_VirtualJoystickTouchpadDesc;
# 461 "./SDL3/SDL_joystick.h"
typedef struct SDL_VirtualJoystickSensorDesc
{
    SDL_SensorType type;
    float rate;
} SDL_VirtualJoystickSensorDesc;
# 480 "./SDL3/SDL_joystick.h"
typedef struct SDL_VirtualJoystickDesc
{
    Uint32 version;
    Uint16 type;
    Uint16 padding;
    Uint16 vendor_id;
    Uint16 product_id;
    Uint16 naxes;
    Uint16 nbuttons;
    Uint16 nballs;
    Uint16 nhats;
    Uint16 ntouchpads;
    Uint16 nsensors;
    Uint16 padding2[2];
    Uint32 button_mask;

    Uint32 axis_mask;

    const char *name;
    const SDL_VirtualJoystickTouchpadDesc *touchpads;
    const SDL_VirtualJoystickSensorDesc *sensors;

    void *userdata;
    void ( *Update)(void *userdata);
    void ( *SetPlayerIndex)(void *userdata, int player_index);
    bool ( *Rumble)(void *userdata, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble);
    bool ( *RumbleTriggers)(void *userdata, Uint16 left_rumble, Uint16 right_rumble);
    bool ( *SetLED)(void *userdata, Uint8 red, Uint8 green, Uint8 blue);
    bool ( *SendEffect)(void *userdata, const void *data, int size);
    bool ( *SetSensorsEnabled)(void *userdata, bool enabled);
    void ( *Cleanup)(void *userdata);
} SDL_VirtualJoystickDesc;
# 521 "./SDL3/SDL_joystick.h"
typedef int SDL_compile_time_assert_SDL_VirtualJoystickDesc_SIZE[((sizeof(void *) == 4 && sizeof(SDL_VirtualJoystickDesc) == 84) || (sizeof(void *) == 8 && sizeof(SDL_VirtualJoystickDesc) == 136)) * 2 - 1];
# 554 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) SDL_JoystickID  SDL_AttachVirtualJoystick(const SDL_VirtualJoystickDesc *desc);
# 570 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) bool  SDL_DetachVirtualJoystick(SDL_JoystickID instance_id);
# 582 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) bool  SDL_IsJoystickVirtual(SDL_JoystickID instance_id);
# 613 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) bool  SDL_SetJoystickVirtualAxis(SDL_Joystick *joystick, int axis, Sint16 value);
# 641 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) bool  SDL_SetJoystickVirtualBall(SDL_Joystick *joystick, int ball, Sint16 xrel, Sint16 yrel);
# 668 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) bool  SDL_SetJoystickVirtualButton(SDL_Joystick *joystick, int button, bool down);
# 695 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) bool  SDL_SetJoystickVirtualHat(SDL_Joystick *joystick, int hat, Uint8 value);
# 729 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) bool  SDL_SetJoystickVirtualTouchpad(SDL_Joystick *joystick, int touchpad, int finger, bool down, float x, float y, float pressure);
# 759 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) bool  SDL_SendJoystickVirtualSensorData(SDL_Joystick *joystick, SDL_SensorType type, Uint64 sensor_timestamp, const float *data, int num_values);
# 785 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) SDL_PropertiesID  SDL_GetJoystickProperties(SDL_Joystick *joystick);
# 806 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) const char *  SDL_GetJoystickName(SDL_Joystick *joystick);
# 821 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) const char *  SDL_GetJoystickPath(SDL_Joystick *joystick);
# 838 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) int  SDL_GetJoystickPlayerIndex(SDL_Joystick *joystick);
# 855 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) bool  SDL_SetJoystickPlayerIndex(SDL_Joystick *joystick, int player_index);
# 874 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) SDL_GUID  SDL_GetJoystickGUID(SDL_Joystick *joystick);
# 890 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) Uint16  SDL_GetJoystickVendor(SDL_Joystick *joystick);
# 906 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) Uint16  SDL_GetJoystickProduct(SDL_Joystick *joystick);
# 922 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) Uint16  SDL_GetJoystickProductVersion(SDL_Joystick *joystick);
# 937 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) Uint16  SDL_GetJoystickFirmwareVersion(SDL_Joystick *joystick);
# 952 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) const char *  SDL_GetJoystickSerial(SDL_Joystick *joystick);
# 966 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) SDL_JoystickType  SDL_GetJoystickType(SDL_Joystick *joystick);
# 987 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) void  SDL_GetJoystickGUIDInfo(SDL_GUID guid, Uint16 *vendor, Uint16 *product, Uint16 *version, Uint16 *crc16);
# 1000 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) bool  SDL_JoystickConnected(SDL_Joystick *joystick);
# 1013 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) SDL_JoystickID  SDL_GetJoystickID(SDL_Joystick *joystick);
# 1035 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) int  SDL_GetNumJoystickAxes(SDL_Joystick *joystick);
# 1058 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) int  SDL_GetNumJoystickBalls(SDL_Joystick *joystick);
# 1076 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) int  SDL_GetNumJoystickHats(SDL_Joystick *joystick);
# 1094 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) int  SDL_GetNumJoystickButtons(SDL_Joystick *joystick);
# 1112 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) void  SDL_SetJoystickEventsEnabled(bool enabled);
# 1129 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) bool  SDL_JoystickEventsEnabled(void);
# 1141 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) void  SDL_UpdateJoysticks(void);
# 1167 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) Sint16  SDL_GetJoystickAxis(SDL_Joystick *joystick, int axis);
# 1185 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) bool  SDL_GetJoystickAxisInitialState(SDL_Joystick *joystick, int axis, Sint16 *state);
# 1208 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) bool  SDL_GetJoystickBall(SDL_Joystick *joystick, int ball, int *dx, int *dy);
# 1225 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) Uint8  SDL_GetJoystickHat(SDL_Joystick *joystick, int hat);
# 1251 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) bool  SDL_GetJoystickButton(SDL_Joystick *joystick, int button);
# 1274 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) bool  SDL_RumbleJoystick(SDL_Joystick *joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble, Uint32 duration_ms);
# 1305 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) bool  SDL_RumbleJoystickTriggers(SDL_Joystick *joystick, Uint16 left_rumble, Uint16 right_rumble, Uint32 duration_ms);
# 1327 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) bool  SDL_SetJoystickLED(SDL_Joystick *joystick, Uint8 red, Uint8 green, Uint8 blue);
# 1342 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) bool  SDL_SendJoystickEffect(SDL_Joystick *joystick, const void *data, int size);
# 1355 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) void  SDL_CloseJoystick(SDL_Joystick *joystick);
# 1369 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) SDL_JoystickConnectionState  SDL_GetJoystickConnectionState(SDL_Joystick *joystick);
# 1392 "./SDL3/SDL_joystick.h"
extern __attribute__((dllimport)) SDL_PowerState  SDL_GetJoystickPowerInfo(SDL_Joystick *joystick, int *percent);






# 1 "./SDL3/SDL_close_code.h" 1
# 1399 "./SDL3/SDL_joystick.h" 2
# 85 "./SDL3/SDL_gamepad.h" 2





# 1 "./SDL3/SDL_begin_code.h" 1
# 90 "./SDL3/SDL_gamepad.h" 2
# 100 "./SDL3/SDL_gamepad.h"
typedef struct SDL_Gamepad SDL_Gamepad;
# 111 "./SDL3/SDL_gamepad.h"
typedef enum SDL_GamepadType
{
    SDL_GAMEPAD_TYPE_UNKNOWN = 0,
    SDL_GAMEPAD_TYPE_STANDARD,
    SDL_GAMEPAD_TYPE_XBOX360,
    SDL_GAMEPAD_TYPE_XBOXONE,
    SDL_GAMEPAD_TYPE_PS3,
    SDL_GAMEPAD_TYPE_PS4,
    SDL_GAMEPAD_TYPE_PS5,
    SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO,
    SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_LEFT,
    SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT,
    SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR,
    SDL_GAMEPAD_TYPE_GAMECUBE,
    SDL_GAMEPAD_TYPE_STEAM,
    SDL_GAMEPAD_TYPE_COUNT
} SDL_GamepadType;
# 153 "./SDL3/SDL_gamepad.h"
typedef enum SDL_GamepadButton
{
    SDL_GAMEPAD_BUTTON_INVALID = -1,
    SDL_GAMEPAD_BUTTON_SOUTH,
    SDL_GAMEPAD_BUTTON_EAST,
    SDL_GAMEPAD_BUTTON_WEST,
    SDL_GAMEPAD_BUTTON_NORTH,
    SDL_GAMEPAD_BUTTON_BACK,
    SDL_GAMEPAD_BUTTON_GUIDE,
    SDL_GAMEPAD_BUTTON_START,
    SDL_GAMEPAD_BUTTON_LEFT_STICK,
    SDL_GAMEPAD_BUTTON_RIGHT_STICK,
    SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,
    SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,
    SDL_GAMEPAD_BUTTON_DPAD_UP,
    SDL_GAMEPAD_BUTTON_DPAD_DOWN,
    SDL_GAMEPAD_BUTTON_DPAD_LEFT,
    SDL_GAMEPAD_BUTTON_DPAD_RIGHT,
    SDL_GAMEPAD_BUTTON_MISC1,
    SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1,
    SDL_GAMEPAD_BUTTON_LEFT_PADDLE1,
    SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2,
    SDL_GAMEPAD_BUTTON_LEFT_PADDLE2,
    SDL_GAMEPAD_BUTTON_TOUCHPAD,
    SDL_GAMEPAD_BUTTON_MISC2,
    SDL_GAMEPAD_BUTTON_MISC3,
    SDL_GAMEPAD_BUTTON_MISC4,
    SDL_GAMEPAD_BUTTON_MISC5,
    SDL_GAMEPAD_BUTTON_MISC6,
    SDL_GAMEPAD_BUTTON_COUNT
} SDL_GamepadButton;
# 196 "./SDL3/SDL_gamepad.h"
typedef enum SDL_GamepadButtonLabel
{
    SDL_GAMEPAD_BUTTON_LABEL_UNKNOWN,
    SDL_GAMEPAD_BUTTON_LABEL_A,
    SDL_GAMEPAD_BUTTON_LABEL_B,
    SDL_GAMEPAD_BUTTON_LABEL_X,
    SDL_GAMEPAD_BUTTON_LABEL_Y,
    SDL_GAMEPAD_BUTTON_LABEL_CROSS,
    SDL_GAMEPAD_BUTTON_LABEL_CIRCLE,
    SDL_GAMEPAD_BUTTON_LABEL_SQUARE,
    SDL_GAMEPAD_BUTTON_LABEL_TRIANGLE
} SDL_GamepadButtonLabel;
# 223 "./SDL3/SDL_gamepad.h"
typedef enum SDL_GamepadAxis
{
    SDL_GAMEPAD_AXIS_INVALID = -1,
    SDL_GAMEPAD_AXIS_LEFTX,
    SDL_GAMEPAD_AXIS_LEFTY,
    SDL_GAMEPAD_AXIS_RIGHTX,
    SDL_GAMEPAD_AXIS_RIGHTY,
    SDL_GAMEPAD_AXIS_LEFT_TRIGGER,
    SDL_GAMEPAD_AXIS_RIGHT_TRIGGER,
    SDL_GAMEPAD_AXIS_COUNT
} SDL_GamepadAxis;
# 243 "./SDL3/SDL_gamepad.h"
typedef enum SDL_GamepadCapSenseType
{
    SDL_GAMEPAD_CAPSENSE_INVALID = -1,
    SDL_GAMEPAD_CAPSENSE_LEFT_STICK,
    SDL_GAMEPAD_CAPSENSE_RIGHT_STICK,
    SDL_GAMEPAD_CAPSENSE_LEFT_GRIP,
    SDL_GAMEPAD_CAPSENSE_RIGHT_GRIP,
    SDL_GAMEPAD_CAPSENSE_COUNT
} SDL_GamepadCapSenseType;
# 263 "./SDL3/SDL_gamepad.h"
typedef enum SDL_GamepadBindingType
{
    SDL_GAMEPAD_BINDTYPE_NONE = 0,
    SDL_GAMEPAD_BINDTYPE_BUTTON,
    SDL_GAMEPAD_BINDTYPE_AXIS,
    SDL_GAMEPAD_BINDTYPE_HAT
} SDL_GamepadBindingType;
# 286 "./SDL3/SDL_gamepad.h"
typedef struct SDL_GamepadBinding
{
    SDL_GamepadBindingType input_type;
    union
    {
        int button;

        struct
        {
            int axis;
            int axis_min;
            int axis_max;
        } axis;

        struct
        {
            int hat;
            int hat_mask;
        } hat;

    } input;

    SDL_GamepadBindingType output_type;
    union
    {
        SDL_GamepadButton button;

        struct
        {
            SDL_GamepadAxis axis;
            int axis_min;
            int axis_max;
        } axis;

    } output;
} SDL_GamepadBinding;
# 365 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) int  SDL_AddGamepadMapping(const char *mapping);
# 405 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) int  SDL_AddGamepadMappingsFromIO(SDL_IOStream *src, bool closeio);
# 439 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) int  SDL_AddGamepadMappingsFromFile(const char *file);
# 453 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) bool  SDL_ReloadGamepadMappings(void);
# 469 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) char **  SDL_GetGamepadMappings(int *count);
# 486 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) char *  SDL_GetGamepadMappingForGUID(SDL_GUID guid);
# 507 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) char *  SDL_GetGamepadMapping(SDL_Gamepad *gamepad);
# 527 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) bool  SDL_SetGamepadMapping(SDL_JoystickID instance_id, const char *mapping);
# 540 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) bool  SDL_HasGamepad(void);
# 558 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) SDL_JoystickID *  SDL_GetGamepads(int *count);
# 574 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) bool  SDL_IsGamepad(SDL_JoystickID instance_id);
# 592 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) const char *  SDL_GetGamepadNameForID(SDL_JoystickID instance_id);
# 610 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) const char *  SDL_GetGamepadPathForID(SDL_JoystickID instance_id);
# 627 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) int  SDL_GetGamepadPlayerIndexForID(SDL_JoystickID instance_id);
# 645 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) SDL_GUID  SDL_GetGamepadGUIDForID(SDL_JoystickID instance_id);
# 664 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) Uint16  SDL_GetGamepadVendorForID(SDL_JoystickID instance_id);
# 683 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) Uint16  SDL_GetGamepadProductForID(SDL_JoystickID instance_id);
# 702 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) Uint16  SDL_GetGamepadProductVersionForID(SDL_JoystickID instance_id);
# 720 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) SDL_GamepadType  SDL_GetGamepadTypeForID(SDL_JoystickID instance_id);
# 738 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) SDL_GamepadType  SDL_GetRealGamepadTypeForID(SDL_JoystickID instance_id);
# 756 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) char *  SDL_GetGamepadMappingForID(SDL_JoystickID instance_id);
# 772 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) SDL_Gamepad *  SDL_OpenGamepad(SDL_JoystickID instance_id);
# 786 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) SDL_Gamepad *  SDL_GetGamepadFromID(SDL_JoystickID instance_id);
# 801 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) SDL_Gamepad *  SDL_GetGamepadFromPlayerIndex(int player_index);
# 830 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) SDL_PropertiesID  SDL_GetGamepadProperties(SDL_Gamepad *gamepad);
# 850 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) SDL_JoystickID  SDL_GetGamepadID(SDL_Gamepad *gamepad);
# 866 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) const char *  SDL_GetGamepadName(SDL_Gamepad *gamepad);
# 882 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) const char *  SDL_GetGamepadPath(SDL_Gamepad *gamepad);
# 897 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) SDL_GamepadType  SDL_GetGamepadType(SDL_Gamepad *gamepad);
# 912 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) SDL_GamepadType  SDL_GetRealGamepadType(SDL_Gamepad *gamepad);
# 928 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) int  SDL_GetGamepadPlayerIndex(SDL_Gamepad *gamepad);
# 945 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) bool  SDL_SetGamepadPlayerIndex(SDL_Gamepad *gamepad, int player_index);
# 961 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) Uint16  SDL_GetGamepadVendor(SDL_Gamepad *gamepad);
# 977 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) Uint16  SDL_GetGamepadProduct(SDL_Gamepad *gamepad);
# 993 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) Uint16  SDL_GetGamepadProductVersion(SDL_Gamepad *gamepad);
# 1007 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) Uint16  SDL_GetGamepadFirmwareVersion(SDL_Gamepad *gamepad);
# 1021 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) const char *  SDL_GetGamepadSerial(SDL_Gamepad *gamepad);
# 1036 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) Uint64  SDL_GetGamepadSteamHandle(SDL_Gamepad *gamepad);
# 1050 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) SDL_JoystickConnectionState  SDL_GetGamepadConnectionState(SDL_Gamepad *gamepad);
# 1072 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) SDL_PowerState  SDL_GetGamepadPowerInfo(SDL_Gamepad *gamepad, int *percent);
# 1086 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) bool  SDL_GamepadConnected(SDL_Gamepad *gamepad);
# 1108 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) SDL_Joystick *  SDL_GetGamepadJoystick(SDL_Gamepad *gamepad);
# 1125 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) void  SDL_SetGamepadEventsEnabled(bool enabled);
# 1141 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) bool  SDL_GamepadEventsEnabled(void);
# 1157 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) SDL_GamepadBinding **  SDL_GetGamepadBindings(SDL_Gamepad *gamepad, int *count);
# 1170 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) void  SDL_UpdateGamepads(void);
# 1190 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) SDL_GamepadType  SDL_GetGamepadTypeFromString(const char *str);
# 1206 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) const char *  SDL_GetGamepadStringForType(SDL_GamepadType type);
# 1230 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) SDL_GamepadAxis  SDL_GetGamepadAxisFromString(const char *str);
# 1246 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) const char *  SDL_GetGamepadStringForAxis(SDL_GamepadAxis axis);
# 1265 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) bool  SDL_GamepadHasAxis(SDL_Gamepad *gamepad, SDL_GamepadAxis axis);
# 1293 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) Sint16  SDL_GetGamepadAxis(SDL_Gamepad *gamepad, SDL_GamepadAxis axis);
# 1313 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) SDL_GamepadButton  SDL_GetGamepadButtonFromString(const char *str);
# 1329 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) const char *  SDL_GetGamepadStringForButton(SDL_GamepadButton button);
# 1347 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) bool  SDL_GamepadHasButton(SDL_Gamepad *gamepad, SDL_GamepadButton button);
# 1363 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) bool  SDL_GetGamepadButton(SDL_Gamepad *gamepad, SDL_GamepadButton button);
# 1378 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) SDL_GamepadButtonLabel  SDL_GetGamepadButtonLabelForType(SDL_GamepadType type, SDL_GamepadButton button);
# 1393 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) SDL_GamepadButtonLabel  SDL_GetGamepadButtonLabel(SDL_Gamepad *gamepad, SDL_GamepadButton button);
# 1407 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) int  SDL_GetNumGamepadTouchpads(SDL_Gamepad *gamepad);
# 1424 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) int  SDL_GetNumGamepadTouchpadFingers(SDL_Gamepad *gamepad, int touchpad);
# 1448 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) bool  SDL_GetGamepadTouchpadFinger(SDL_Gamepad *gamepad, int touchpad, int finger, bool *down, float *x, float *y, float *pressure);
# 1465 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) bool  SDL_GamepadHasSensor(SDL_Gamepad *gamepad, SDL_SensorType type);
# 1483 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) bool  SDL_SetGamepadSensorEnabled(SDL_Gamepad *gamepad, SDL_SensorType type, bool enabled);
# 1498 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) bool  SDL_GamepadSensorEnabled(SDL_Gamepad *gamepad, SDL_SensorType type);
# 1511 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) float  SDL_GetGamepadSensorDataRate(SDL_Gamepad *gamepad, SDL_SensorType type);
# 1530 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) bool  SDL_GetGamepadSensorData(SDL_Gamepad *gamepad, SDL_SensorType type, float *data, int num_values);
# 1545 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) bool  SDL_GamepadHasCapSense(SDL_Gamepad *gamepad, SDL_GamepadCapSenseType type);
# 1560 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) bool  SDL_GetGamepadCapSense(SDL_Gamepad *gamepad, SDL_GamepadCapSenseType type);
# 1584 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) bool  SDL_RumbleGamepad(SDL_Gamepad *gamepad, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble, Uint32 duration_ms);
# 1614 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) bool  SDL_RumbleGamepadTriggers(SDL_Gamepad *gamepad, Uint16 left_rumble, Uint16 right_rumble, Uint32 duration_ms);
# 1636 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) bool  SDL_SetGamepadLED(SDL_Gamepad *gamepad, Uint8 red, Uint8 green, Uint8 blue);
# 1651 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) bool  SDL_SendGamepadEffect(SDL_Gamepad *gamepad, const void *data, int size);
# 1665 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) void  SDL_CloseGamepad(SDL_Gamepad *gamepad);
# 1681 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) const char *  SDL_GetGamepadAppleSFSymbolsNameForButton(SDL_Gamepad *gamepad, SDL_GamepadButton button);
# 1696 "./SDL3/SDL_gamepad.h"
extern __attribute__((dllimport)) const char *  SDL_GetGamepadAppleSFSymbolsNameForAxis(SDL_Gamepad *gamepad, SDL_GamepadAxis axis);







# 1 "./SDL3/SDL_close_code.h" 1
# 1704 "./SDL3/SDL_gamepad.h" 2
# 60 "./SDL3/SDL_events.h" 2


# 1 "./SDL3/SDL_keyboard.h" 1
# 39 "./SDL3/SDL_keyboard.h"
# 1 "./SDL3/SDL_keycode.h" 1
# 38 "./SDL3/SDL_keycode.h"
# 1 "./SDL3/SDL_scancode.h" 1
# 52 "./SDL3/SDL_scancode.h"
typedef enum SDL_Scancode
{
    SDL_SCANCODE_UNKNOWN = 0,
# 63 "./SDL3/SDL_scancode.h"
    SDL_SCANCODE_A = 4,
    SDL_SCANCODE_B = 5,
    SDL_SCANCODE_C = 6,
    SDL_SCANCODE_D = 7,
    SDL_SCANCODE_E = 8,
    SDL_SCANCODE_F = 9,
    SDL_SCANCODE_G = 10,
    SDL_SCANCODE_H = 11,
    SDL_SCANCODE_I = 12,
    SDL_SCANCODE_J = 13,
    SDL_SCANCODE_K = 14,
    SDL_SCANCODE_L = 15,
    SDL_SCANCODE_M = 16,
    SDL_SCANCODE_N = 17,
    SDL_SCANCODE_O = 18,
    SDL_SCANCODE_P = 19,
    SDL_SCANCODE_Q = 20,
    SDL_SCANCODE_R = 21,
    SDL_SCANCODE_S = 22,
    SDL_SCANCODE_T = 23,
    SDL_SCANCODE_U = 24,
    SDL_SCANCODE_V = 25,
    SDL_SCANCODE_W = 26,
    SDL_SCANCODE_X = 27,
    SDL_SCANCODE_Y = 28,
    SDL_SCANCODE_Z = 29,

    SDL_SCANCODE_1 = 30,
    SDL_SCANCODE_2 = 31,
    SDL_SCANCODE_3 = 32,
    SDL_SCANCODE_4 = 33,
    SDL_SCANCODE_5 = 34,
    SDL_SCANCODE_6 = 35,
    SDL_SCANCODE_7 = 36,
    SDL_SCANCODE_8 = 37,
    SDL_SCANCODE_9 = 38,
    SDL_SCANCODE_0 = 39,

    SDL_SCANCODE_RETURN = 40,
    SDL_SCANCODE_ESCAPE = 41,
    SDL_SCANCODE_BACKSPACE = 42,
    SDL_SCANCODE_TAB = 43,
    SDL_SCANCODE_SPACE = 44,

    SDL_SCANCODE_MINUS = 45,
    SDL_SCANCODE_EQUALS = 46,
    SDL_SCANCODE_LEFTBRACKET = 47,
    SDL_SCANCODE_RIGHTBRACKET = 48,
    SDL_SCANCODE_BACKSLASH = 49,
# 125 "./SDL3/SDL_scancode.h"
    SDL_SCANCODE_NONUSHASH = 50,
# 137 "./SDL3/SDL_scancode.h"
    SDL_SCANCODE_SEMICOLON = 51,
    SDL_SCANCODE_APOSTROPHE = 52,
    SDL_SCANCODE_GRAVE = 53,
# 156 "./SDL3/SDL_scancode.h"
    SDL_SCANCODE_COMMA = 54,
    SDL_SCANCODE_PERIOD = 55,
    SDL_SCANCODE_SLASH = 56,

    SDL_SCANCODE_CAPSLOCK = 57,

    SDL_SCANCODE_F1 = 58,
    SDL_SCANCODE_F2 = 59,
    SDL_SCANCODE_F3 = 60,
    SDL_SCANCODE_F4 = 61,
    SDL_SCANCODE_F5 = 62,
    SDL_SCANCODE_F6 = 63,
    SDL_SCANCODE_F7 = 64,
    SDL_SCANCODE_F8 = 65,
    SDL_SCANCODE_F9 = 66,
    SDL_SCANCODE_F10 = 67,
    SDL_SCANCODE_F11 = 68,
    SDL_SCANCODE_F12 = 69,

    SDL_SCANCODE_PRINTSCREEN = 70,
    SDL_SCANCODE_SCROLLLOCK = 71,
    SDL_SCANCODE_PAUSE = 72,
    SDL_SCANCODE_INSERT = 73,

    SDL_SCANCODE_HOME = 74,
    SDL_SCANCODE_PAGEUP = 75,
    SDL_SCANCODE_DELETE = 76,
    SDL_SCANCODE_END = 77,
    SDL_SCANCODE_PAGEDOWN = 78,
    SDL_SCANCODE_RIGHT = 79,
    SDL_SCANCODE_LEFT = 80,
    SDL_SCANCODE_DOWN = 81,
    SDL_SCANCODE_UP = 82,

    SDL_SCANCODE_NUMLOCKCLEAR = 83,

    SDL_SCANCODE_KP_DIVIDE = 84,
    SDL_SCANCODE_KP_MULTIPLY = 85,
    SDL_SCANCODE_KP_MINUS = 86,
    SDL_SCANCODE_KP_PLUS = 87,
    SDL_SCANCODE_KP_ENTER = 88,
    SDL_SCANCODE_KP_1 = 89,
    SDL_SCANCODE_KP_2 = 90,
    SDL_SCANCODE_KP_3 = 91,
    SDL_SCANCODE_KP_4 = 92,
    SDL_SCANCODE_KP_5 = 93,
    SDL_SCANCODE_KP_6 = 94,
    SDL_SCANCODE_KP_7 = 95,
    SDL_SCANCODE_KP_8 = 96,
    SDL_SCANCODE_KP_9 = 97,
    SDL_SCANCODE_KP_0 = 98,
    SDL_SCANCODE_KP_PERIOD = 99,

    SDL_SCANCODE_NONUSBACKSLASH = 100,
# 219 "./SDL3/SDL_scancode.h"
    SDL_SCANCODE_APPLICATION = 101,
    SDL_SCANCODE_POWER = 102,


    SDL_SCANCODE_KP_EQUALS = 103,
    SDL_SCANCODE_F13 = 104,
    SDL_SCANCODE_F14 = 105,
    SDL_SCANCODE_F15 = 106,
    SDL_SCANCODE_F16 = 107,
    SDL_SCANCODE_F17 = 108,
    SDL_SCANCODE_F18 = 109,
    SDL_SCANCODE_F19 = 110,
    SDL_SCANCODE_F20 = 111,
    SDL_SCANCODE_F21 = 112,
    SDL_SCANCODE_F22 = 113,
    SDL_SCANCODE_F23 = 114,
    SDL_SCANCODE_F24 = 115,
    SDL_SCANCODE_EXECUTE = 116,
    SDL_SCANCODE_HELP = 117,
    SDL_SCANCODE_MENU = 118,
    SDL_SCANCODE_SELECT = 119,
    SDL_SCANCODE_STOP = 120,
    SDL_SCANCODE_AGAIN = 121,
    SDL_SCANCODE_UNDO = 122,
    SDL_SCANCODE_CUT = 123,
    SDL_SCANCODE_COPY = 124,
    SDL_SCANCODE_PASTE = 125,
    SDL_SCANCODE_FIND = 126,
    SDL_SCANCODE_MUTE = 127,
    SDL_SCANCODE_VOLUMEUP = 128,
    SDL_SCANCODE_VOLUMEDOWN = 129,




    SDL_SCANCODE_KP_COMMA = 133,
    SDL_SCANCODE_KP_EQUALSAS400 = 134,

    SDL_SCANCODE_INTERNATIONAL1 = 135,

    SDL_SCANCODE_INTERNATIONAL2 = 136,
    SDL_SCANCODE_INTERNATIONAL3 = 137,
    SDL_SCANCODE_INTERNATIONAL4 = 138,
    SDL_SCANCODE_INTERNATIONAL5 = 139,
    SDL_SCANCODE_INTERNATIONAL6 = 140,
    SDL_SCANCODE_INTERNATIONAL7 = 141,
    SDL_SCANCODE_INTERNATIONAL8 = 142,
    SDL_SCANCODE_INTERNATIONAL9 = 143,
    SDL_SCANCODE_LANG1 = 144,
    SDL_SCANCODE_LANG2 = 145,
    SDL_SCANCODE_LANG3 = 146,
    SDL_SCANCODE_LANG4 = 147,
    SDL_SCANCODE_LANG5 = 148,
    SDL_SCANCODE_LANG6 = 149,
    SDL_SCANCODE_LANG7 = 150,
    SDL_SCANCODE_LANG8 = 151,
    SDL_SCANCODE_LANG9 = 152,

    SDL_SCANCODE_ALTERASE = 153,
    SDL_SCANCODE_SYSREQ = 154,
    SDL_SCANCODE_CANCEL = 155,
    SDL_SCANCODE_CLEAR = 156,
    SDL_SCANCODE_PRIOR = 157,
    SDL_SCANCODE_RETURN2 = 158,
    SDL_SCANCODE_SEPARATOR = 159,
    SDL_SCANCODE_OUT = 160,
    SDL_SCANCODE_OPER = 161,
    SDL_SCANCODE_CLEARAGAIN = 162,
    SDL_SCANCODE_CRSEL = 163,
    SDL_SCANCODE_EXSEL = 164,

    SDL_SCANCODE_KP_00 = 176,
    SDL_SCANCODE_KP_000 = 177,
    SDL_SCANCODE_THOUSANDSSEPARATOR = 178,
    SDL_SCANCODE_DECIMALSEPARATOR = 179,
    SDL_SCANCODE_CURRENCYUNIT = 180,
    SDL_SCANCODE_CURRENCYSUBUNIT = 181,
    SDL_SCANCODE_KP_LEFTPAREN = 182,
    SDL_SCANCODE_KP_RIGHTPAREN = 183,
    SDL_SCANCODE_KP_LEFTBRACE = 184,
    SDL_SCANCODE_KP_RIGHTBRACE = 185,
    SDL_SCANCODE_KP_TAB = 186,
    SDL_SCANCODE_KP_BACKSPACE = 187,
    SDL_SCANCODE_KP_A = 188,
    SDL_SCANCODE_KP_B = 189,
    SDL_SCANCODE_KP_C = 190,
    SDL_SCANCODE_KP_D = 191,
    SDL_SCANCODE_KP_E = 192,
    SDL_SCANCODE_KP_F = 193,
    SDL_SCANCODE_KP_XOR = 194,
    SDL_SCANCODE_KP_POWER = 195,
    SDL_SCANCODE_KP_PERCENT = 196,
    SDL_SCANCODE_KP_LESS = 197,
    SDL_SCANCODE_KP_GREATER = 198,
    SDL_SCANCODE_KP_AMPERSAND = 199,
    SDL_SCANCODE_KP_DBLAMPERSAND = 200,
    SDL_SCANCODE_KP_VERTICALBAR = 201,
    SDL_SCANCODE_KP_DBLVERTICALBAR = 202,
    SDL_SCANCODE_KP_COLON = 203,
    SDL_SCANCODE_KP_HASH = 204,
    SDL_SCANCODE_KP_SPACE = 205,
    SDL_SCANCODE_KP_AT = 206,
    SDL_SCANCODE_KP_EXCLAM = 207,
    SDL_SCANCODE_KP_MEMSTORE = 208,
    SDL_SCANCODE_KP_MEMRECALL = 209,
    SDL_SCANCODE_KP_MEMCLEAR = 210,
    SDL_SCANCODE_KP_MEMADD = 211,
    SDL_SCANCODE_KP_MEMSUBTRACT = 212,
    SDL_SCANCODE_KP_MEMMULTIPLY = 213,
    SDL_SCANCODE_KP_MEMDIVIDE = 214,
    SDL_SCANCODE_KP_PLUSMINUS = 215,
    SDL_SCANCODE_KP_CLEAR = 216,
    SDL_SCANCODE_KP_CLEARENTRY = 217,
    SDL_SCANCODE_KP_BINARY = 218,
    SDL_SCANCODE_KP_OCTAL = 219,
    SDL_SCANCODE_KP_DECIMAL = 220,
    SDL_SCANCODE_KP_HEXADECIMAL = 221,

    SDL_SCANCODE_LCTRL = 224,
    SDL_SCANCODE_LSHIFT = 225,
    SDL_SCANCODE_LALT = 226,
    SDL_SCANCODE_LGUI = 227,
    SDL_SCANCODE_RCTRL = 228,
    SDL_SCANCODE_RSHIFT = 229,
    SDL_SCANCODE_RALT = 230,
    SDL_SCANCODE_RGUI = 231,

    SDL_SCANCODE_MODE = 257,
# 364 "./SDL3/SDL_scancode.h"
    SDL_SCANCODE_SLEEP = 258,
    SDL_SCANCODE_WAKE = 259,

    SDL_SCANCODE_CHANNEL_INCREMENT = 260,
    SDL_SCANCODE_CHANNEL_DECREMENT = 261,

    SDL_SCANCODE_MEDIA_PLAY = 262,
    SDL_SCANCODE_MEDIA_PAUSE = 263,
    SDL_SCANCODE_MEDIA_RECORD = 264,
    SDL_SCANCODE_MEDIA_FAST_FORWARD = 265,
    SDL_SCANCODE_MEDIA_REWIND = 266,
    SDL_SCANCODE_MEDIA_NEXT_TRACK = 267,
    SDL_SCANCODE_MEDIA_PREVIOUS_TRACK = 268,
    SDL_SCANCODE_MEDIA_STOP = 269,
    SDL_SCANCODE_MEDIA_EJECT = 270,
    SDL_SCANCODE_MEDIA_PLAY_PAUSE = 271,
    SDL_SCANCODE_MEDIA_SELECT = 272,

    SDL_SCANCODE_AC_NEW = 273,
    SDL_SCANCODE_AC_OPEN = 274,
    SDL_SCANCODE_AC_CLOSE = 275,
    SDL_SCANCODE_AC_EXIT = 276,
    SDL_SCANCODE_AC_SAVE = 277,
    SDL_SCANCODE_AC_PRINT = 278,
    SDL_SCANCODE_AC_PROPERTIES = 279,

    SDL_SCANCODE_AC_SEARCH = 280,
    SDL_SCANCODE_AC_HOME = 281,
    SDL_SCANCODE_AC_BACK = 282,
    SDL_SCANCODE_AC_FORWARD = 283,
    SDL_SCANCODE_AC_STOP = 284,
    SDL_SCANCODE_AC_REFRESH = 285,
    SDL_SCANCODE_AC_BOOKMARKS = 286,
# 408 "./SDL3/SDL_scancode.h"
    SDL_SCANCODE_SOFTLEFT = 287,



    SDL_SCANCODE_SOFTRIGHT = 288,



    SDL_SCANCODE_CALL = 289,
    SDL_SCANCODE_ENDCALL = 290,





    SDL_SCANCODE_RESERVED = 400,

    SDL_SCANCODE_COUNT = 512

} SDL_Scancode;
# 38 "./SDL3/SDL_keycode.h" 2
# 59 "./SDL3/SDL_keycode.h"
typedef Uint32 SDL_Keycode;
# 326 "./SDL3/SDL_keycode.h"
typedef Uint16 SDL_Keymod;
# 39 "./SDL3/SDL_keyboard.h" 2






# 1 "./SDL3/SDL_begin_code.h" 1
# 45 "./SDL3/SDL_keyboard.h" 2
# 60 "./SDL3/SDL_keyboard.h"
typedef Uint32 SDL_KeyboardID;
# 75 "./SDL3/SDL_keyboard.h"
extern __attribute__((dllimport)) bool  SDL_HasKeyboard(void);
# 98 "./SDL3/SDL_keyboard.h"
extern __attribute__((dllimport)) SDL_KeyboardID *  SDL_GetKeyboards(int *count);
# 115 "./SDL3/SDL_keyboard.h"
extern __attribute__((dllimport)) const char *  SDL_GetKeyboardNameForID(SDL_KeyboardID instance_id);
# 126 "./SDL3/SDL_keyboard.h"
extern __attribute__((dllimport)) SDL_Window *  SDL_GetKeyboardFocus(void);
# 159 "./SDL3/SDL_keyboard.h"
extern __attribute__((dllimport)) const bool *  SDL_GetKeyboardState(int *numkeys);
# 172 "./SDL3/SDL_keyboard.h"
extern __attribute__((dllimport)) void  SDL_ResetKeyboard(void);
# 186 "./SDL3/SDL_keyboard.h"
extern __attribute__((dllimport)) SDL_Keymod  SDL_GetModState(void);
# 207 "./SDL3/SDL_keyboard.h"
extern __attribute__((dllimport)) void  SDL_SetModState(SDL_Keymod modstate);
# 231 "./SDL3/SDL_keyboard.h"
extern __attribute__((dllimport)) SDL_Keycode  SDL_GetKeyFromScancode(SDL_Scancode scancode, SDL_Keymod modstate, bool key_event);
# 252 "./SDL3/SDL_keyboard.h"
extern __attribute__((dllimport)) SDL_Scancode  SDL_GetScancodeFromKey(SDL_Keycode key, SDL_Keymod *modstate);
# 270 "./SDL3/SDL_keyboard.h"
extern __attribute__((dllimport)) bool  SDL_SetScancodeName(SDL_Scancode scancode, const char *name);
# 296 "./SDL3/SDL_keyboard.h"
extern __attribute__((dllimport)) const char *  SDL_GetScancodeName(SDL_Scancode scancode);
# 313 "./SDL3/SDL_keyboard.h"
extern __attribute__((dllimport)) SDL_Scancode  SDL_GetScancodeFromName(const char *name);
# 333 "./SDL3/SDL_keyboard.h"
extern __attribute__((dllimport)) const char *  SDL_GetKeyName(SDL_Keycode key);
# 350 "./SDL3/SDL_keyboard.h"
extern __attribute__((dllimport)) SDL_Keycode  SDL_GetKeyFromName(const char *name);
# 378 "./SDL3/SDL_keyboard.h"
extern __attribute__((dllimport)) bool  SDL_StartTextInput(SDL_Window *window);
# 391 "./SDL3/SDL_keyboard.h"
typedef enum SDL_TextInputType
{
    SDL_TEXTINPUT_TYPE_TEXT,
    SDL_TEXTINPUT_TYPE_TEXT_NAME,
    SDL_TEXTINPUT_TYPE_TEXT_EMAIL,
    SDL_TEXTINPUT_TYPE_TEXT_USERNAME,
    SDL_TEXTINPUT_TYPE_TEXT_PASSWORD_HIDDEN,
    SDL_TEXTINPUT_TYPE_TEXT_PASSWORD_VISIBLE,
    SDL_TEXTINPUT_TYPE_NUMBER,
    SDL_TEXTINPUT_TYPE_NUMBER_PASSWORD_HIDDEN,
    SDL_TEXTINPUT_TYPE_NUMBER_PASSWORD_VISIBLE
} SDL_TextInputType;
# 415 "./SDL3/SDL_keyboard.h"
typedef enum SDL_Capitalization
{
    SDL_CAPITALIZE_NONE,
    SDL_CAPITALIZE_SENTENCES,
    SDL_CAPITALIZE_WORDS,
    SDL_CAPITALIZE_LETTERS
} SDL_Capitalization;
# 481 "./SDL3/SDL_keyboard.h"
extern __attribute__((dllimport)) bool  SDL_StartTextInputWithProperties(SDL_Window *window, SDL_PropertiesID props);
# 505 "./SDL3/SDL_keyboard.h"
extern __attribute__((dllimport)) bool  SDL_TextInputActive(SDL_Window *window);
# 523 "./SDL3/SDL_keyboard.h"
extern __attribute__((dllimport)) bool  SDL_StopTextInput(SDL_Window *window);
# 539 "./SDL3/SDL_keyboard.h"
extern __attribute__((dllimport)) bool  SDL_ClearComposition(SDL_Window *window);
# 562 "./SDL3/SDL_keyboard.h"
extern __attribute__((dllimport)) bool  SDL_SetTextInputArea(SDL_Window *window, const SDL_Rect *rect, int cursor);
# 583 "./SDL3/SDL_keyboard.h"
extern __attribute__((dllimport)) bool  SDL_GetTextInputArea(SDL_Window *window, SDL_Rect *rect, int *cursor);
# 598 "./SDL3/SDL_keyboard.h"
extern __attribute__((dllimport)) bool  SDL_HasScreenKeyboardSupport(void);
# 612 "./SDL3/SDL_keyboard.h"
extern __attribute__((dllimport)) bool  SDL_ScreenKeyboardShown(SDL_Window *window);






# 1 "./SDL3/SDL_close_code.h" 1
# 619 "./SDL3/SDL_keyboard.h" 2
# 62 "./SDL3/SDL_events.h" 2


# 1 "./SDL3/SDL_mouse.h" 1
# 66 "./SDL3/SDL_mouse.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 66 "./SDL3/SDL_mouse.h" 2
# 81 "./SDL3/SDL_mouse.h"
typedef Uint32 SDL_MouseID;
# 90 "./SDL3/SDL_mouse.h"
typedef struct SDL_Cursor SDL_Cursor;






typedef enum SDL_SystemCursor
{
    SDL_SYSTEM_CURSOR_DEFAULT,
    SDL_SYSTEM_CURSOR_TEXT,
    SDL_SYSTEM_CURSOR_WAIT,
    SDL_SYSTEM_CURSOR_CROSSHAIR,
    SDL_SYSTEM_CURSOR_PROGRESS,
    SDL_SYSTEM_CURSOR_NWSE_RESIZE,
    SDL_SYSTEM_CURSOR_NESW_RESIZE,
    SDL_SYSTEM_CURSOR_EW_RESIZE,
    SDL_SYSTEM_CURSOR_NS_RESIZE,
    SDL_SYSTEM_CURSOR_MOVE,
    SDL_SYSTEM_CURSOR_NOT_ALLOWED,
    SDL_SYSTEM_CURSOR_POINTER,
    SDL_SYSTEM_CURSOR_NW_RESIZE,
    SDL_SYSTEM_CURSOR_N_RESIZE,
    SDL_SYSTEM_CURSOR_NE_RESIZE,
    SDL_SYSTEM_CURSOR_E_RESIZE,
    SDL_SYSTEM_CURSOR_SE_RESIZE,
    SDL_SYSTEM_CURSOR_S_RESIZE,
    SDL_SYSTEM_CURSOR_SW_RESIZE,
    SDL_SYSTEM_CURSOR_W_RESIZE,
    SDL_SYSTEM_CURSOR_CONTEXT_MENU,
    SDL_SYSTEM_CURSOR_HELP,
    SDL_SYSTEM_CURSOR_CELL,
    SDL_SYSTEM_CURSOR_VERTICAL_TEXT,
    SDL_SYSTEM_CURSOR_ALIAS,
    SDL_SYSTEM_CURSOR_COPY,
    SDL_SYSTEM_CURSOR_NO_DROP,
    SDL_SYSTEM_CURSOR_GRAB,
    SDL_SYSTEM_CURSOR_GRABBING,
    SDL_SYSTEM_CURSOR_COL_RESIZE,
    SDL_SYSTEM_CURSOR_ROW_RESIZE,
    SDL_SYSTEM_CURSOR_ALL_SCROLL,
    SDL_SYSTEM_CURSOR_ZOOM_IN,
    SDL_SYSTEM_CURSOR_ZOOM_OUT,
    SDL_SYSTEM_CURSOR_COUNT
} SDL_SystemCursor;






typedef enum SDL_MouseWheelDirection
{
    SDL_MOUSEWHEEL_NORMAL,
    SDL_MOUSEWHEEL_FLIPPED
} SDL_MouseWheelDirection;






typedef struct SDL_CursorFrameInfo
{
    SDL_Surface *surface;
    Uint32 duration;
} SDL_CursorFrameInfo;
# 173 "./SDL3/SDL_mouse.h"
typedef Uint32 SDL_MouseButtonFlags;
# 219 "./SDL3/SDL_mouse.h"
typedef void ( *SDL_MouseMotionTransformCallback)(
    void *userdata,
    Uint64 timestamp,
    SDL_Window *window,
    SDL_MouseID mouseID,
    float *x, float *y
);
# 240 "./SDL3/SDL_mouse.h"
extern __attribute__((dllimport)) bool  SDL_HasMouse(void);
# 263 "./SDL3/SDL_mouse.h"
extern __attribute__((dllimport)) SDL_MouseID *  SDL_GetMice(int *count);
# 280 "./SDL3/SDL_mouse.h"
extern __attribute__((dllimport)) const char *  SDL_GetMouseNameForID(SDL_MouseID instance_id);
# 291 "./SDL3/SDL_mouse.h"
extern __attribute__((dllimport)) SDL_Window *  SDL_GetMouseFocus(void);
# 324 "./SDL3/SDL_mouse.h"
extern __attribute__((dllimport)) SDL_MouseButtonFlags  SDL_GetMouseState(float *x, float *y);
# 361 "./SDL3/SDL_mouse.h"
extern __attribute__((dllimport)) SDL_MouseButtonFlags  SDL_GetGlobalMouseState(float *x, float *y);
# 396 "./SDL3/SDL_mouse.h"
extern __attribute__((dllimport)) SDL_MouseButtonFlags  SDL_GetRelativeMouseState(float *x, float *y);
# 419 "./SDL3/SDL_mouse.h"
extern __attribute__((dllimport)) void  SDL_WarpMouseInWindow(SDL_Window *window,
                                                   float x, float y);
# 444 "./SDL3/SDL_mouse.h"
extern __attribute__((dllimport)) bool  SDL_WarpMouseGlobal(float x, float y);
# 462 "./SDL3/SDL_mouse.h"
extern __attribute__((dllimport)) bool  SDL_SetRelativeMouseTransform(SDL_MouseMotionTransformCallback callback, void *userdata);
# 490 "./SDL3/SDL_mouse.h"
extern __attribute__((dllimport)) bool  SDL_SetWindowRelativeMouseMode(SDL_Window *window, bool enabled);
# 504 "./SDL3/SDL_mouse.h"
extern __attribute__((dllimport)) bool  SDL_GetWindowRelativeMouseMode(SDL_Window *window);
# 552 "./SDL3/SDL_mouse.h"
extern __attribute__((dllimport)) bool  SDL_CaptureMouse(bool enabled);
# 599 "./SDL3/SDL_mouse.h"
extern __attribute__((dllimport)) SDL_Cursor *  SDL_CreateCursor(const Uint8 *data,
                                                     const Uint8 *mask,
                                                     int w, int h, int hot_x,
                                                     int hot_y);
# 636 "./SDL3/SDL_mouse.h"
extern __attribute__((dllimport)) SDL_Cursor *  SDL_CreateColorCursor(SDL_Surface *surface,
                                                          int hot_x,
                                                          int hot_y);
# 686 "./SDL3/SDL_mouse.h"
extern __attribute__((dllimport)) SDL_Cursor * SDL_CreateAnimatedCursor(SDL_CursorFrameInfo *frames,
                                                                 int frame_count,
                                                                 int hot_x,
                                                                 int hot_y);
# 704 "./SDL3/SDL_mouse.h"
extern __attribute__((dllimport)) SDL_Cursor *  SDL_CreateSystemCursor(SDL_SystemCursor id);
# 724 "./SDL3/SDL_mouse.h"
extern __attribute__((dllimport)) bool  SDL_SetCursor(SDL_Cursor *cursor);
# 740 "./SDL3/SDL_mouse.h"
extern __attribute__((dllimport)) SDL_Cursor *  SDL_GetCursor(void);
# 755 "./SDL3/SDL_mouse.h"
extern __attribute__((dllimport)) SDL_Cursor *  SDL_GetDefaultCursor(void);
# 774 "./SDL3/SDL_mouse.h"
extern __attribute__((dllimport)) void  SDL_DestroyCursor(SDL_Cursor *cursor);
# 789 "./SDL3/SDL_mouse.h"
extern __attribute__((dllimport)) bool  SDL_ShowCursor(void);
# 804 "./SDL3/SDL_mouse.h"
extern __attribute__((dllimport)) bool  SDL_HideCursor(void);
# 819 "./SDL3/SDL_mouse.h"
extern __attribute__((dllimport)) bool  SDL_CursorVisible(void);






# 1 "./SDL3/SDL_close_code.h" 1
# 826 "./SDL3/SDL_mouse.h" 2
# 64 "./SDL3/SDL_events.h" 2

# 1 "./SDL3/SDL_pen.h" 1
# 70 "./SDL3/SDL_pen.h"
# 1 "./SDL3/SDL_touch.h" 1
# 47 "./SDL3/SDL_touch.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 47 "./SDL3/SDL_touch.h" 2
# 62 "./SDL3/SDL_touch.h"
typedef Uint64 SDL_TouchID;
# 76 "./SDL3/SDL_touch.h"
typedef Uint64 SDL_FingerID;






typedef enum SDL_TouchDeviceType
{
    SDL_TOUCH_DEVICE_INVALID = -1,
    SDL_TOUCH_DEVICE_DIRECT,
    SDL_TOUCH_DEVICE_INDIRECT_ABSOLUTE,
    SDL_TOUCH_DEVICE_INDIRECT_RELATIVE
} SDL_TouchDeviceType;
# 102 "./SDL3/SDL_touch.h"
typedef struct SDL_Finger
{
    SDL_FingerID id;
    float x;
    float y;
    float pressure;
} SDL_Finger;
# 140 "./SDL3/SDL_touch.h"
extern __attribute__((dllimport)) SDL_TouchID *  SDL_GetTouchDevices(int *count);
# 151 "./SDL3/SDL_touch.h"
extern __attribute__((dllimport)) const char *  SDL_GetTouchDeviceName(SDL_TouchID touchID);
# 161 "./SDL3/SDL_touch.h"
extern __attribute__((dllimport)) SDL_TouchDeviceType  SDL_GetTouchDeviceType(SDL_TouchID touchID);
# 176 "./SDL3/SDL_touch.h"
extern __attribute__((dllimport)) SDL_Finger **  SDL_GetTouchFingers(SDL_TouchID touchID, int *count);






# 1 "./SDL3/SDL_close_code.h" 1
# 183 "./SDL3/SDL_touch.h" 2
# 70 "./SDL3/SDL_pen.h" 2


# 1 "./SDL3/SDL_begin_code.h" 1
# 72 "./SDL3/SDL_pen.h" 2
# 93 "./SDL3/SDL_pen.h"
typedef Uint32 SDL_PenID;
# 114 "./SDL3/SDL_pen.h"
typedef Uint32 SDL_PenInputFlags;
# 140 "./SDL3/SDL_pen.h"
typedef enum SDL_PenAxis
{
    SDL_PEN_AXIS_PRESSURE,
    SDL_PEN_AXIS_XTILT,
    SDL_PEN_AXIS_YTILT,
    SDL_PEN_AXIS_DISTANCE,
    SDL_PEN_AXIS_ROTATION,
    SDL_PEN_AXIS_SLIDER,
    SDL_PEN_AXIS_TANGENTIAL_PRESSURE,
    SDL_PEN_AXIS_COUNT
} SDL_PenAxis;
# 167 "./SDL3/SDL_pen.h"
typedef enum SDL_PenDeviceType
{
    SDL_PEN_DEVICE_TYPE_INVALID = -1,
    SDL_PEN_DEVICE_TYPE_UNKNOWN,
    SDL_PEN_DEVICE_TYPE_DIRECT,
    SDL_PEN_DEVICE_TYPE_INDIRECT
} SDL_PenDeviceType;
# 189 "./SDL3/SDL_pen.h"
extern __attribute__((dllimport)) SDL_PenDeviceType  SDL_GetPenDeviceType(SDL_PenID instance_id);






# 1 "./SDL3/SDL_close_code.h" 1
# 196 "./SDL3/SDL_pen.h" 2
# 65 "./SDL3/SDL_events.h" 2







# 1 "./SDL3/SDL_begin_code.h" 1
# 72 "./SDL3/SDL_events.h" 2
# 84 "./SDL3/SDL_events.h"
typedef enum SDL_EventType
{
    SDL_EVENT_FIRST     = 0,


    SDL_EVENT_QUIT           = 0x100,


    SDL_EVENT_TERMINATING,



    SDL_EVENT_LOW_MEMORY,



    SDL_EVENT_WILL_ENTER_BACKGROUND,



    SDL_EVENT_DID_ENTER_BACKGROUND,



    SDL_EVENT_WILL_ENTER_FOREGROUND,



    SDL_EVENT_DID_ENTER_FOREGROUND,




    SDL_EVENT_LOCALE_CHANGED,

    SDL_EVENT_SYSTEM_THEME_CHANGED,



    SDL_EVENT_DISPLAY_ORIENTATION = 0x151,
    SDL_EVENT_DISPLAY_ADDED,
    SDL_EVENT_DISPLAY_REMOVED,
    SDL_EVENT_DISPLAY_MOVED,
    SDL_EVENT_DISPLAY_DESKTOP_MODE_CHANGED,
    SDL_EVENT_DISPLAY_CURRENT_MODE_CHANGED,
    SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED,
    SDL_EVENT_DISPLAY_USABLE_BOUNDS_CHANGED,
    SDL_EVENT_DISPLAY_FIRST = SDL_EVENT_DISPLAY_ORIENTATION,
    SDL_EVENT_DISPLAY_LAST = SDL_EVENT_DISPLAY_USABLE_BOUNDS_CHANGED,




    SDL_EVENT_WINDOW_SHOWN = 0x202,
    SDL_EVENT_WINDOW_HIDDEN,
    SDL_EVENT_WINDOW_EXPOSED,

    SDL_EVENT_WINDOW_MOVED,
    SDL_EVENT_WINDOW_RESIZED,
    SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED,
    SDL_EVENT_WINDOW_METAL_VIEW_RESIZED,
    SDL_EVENT_WINDOW_MINIMIZED,
    SDL_EVENT_WINDOW_MAXIMIZED,
    SDL_EVENT_WINDOW_RESTORED,
    SDL_EVENT_WINDOW_MOUSE_ENTER,
    SDL_EVENT_WINDOW_MOUSE_LEAVE,
    SDL_EVENT_WINDOW_FOCUS_GAINED,
    SDL_EVENT_WINDOW_FOCUS_LOST,
    SDL_EVENT_WINDOW_CLOSE_REQUESTED,
    SDL_EVENT_WINDOW_HIT_TEST,
    SDL_EVENT_WINDOW_ICCPROF_CHANGED,
    SDL_EVENT_WINDOW_DISPLAY_CHANGED,
    SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED,
    SDL_EVENT_WINDOW_SAFE_AREA_CHANGED,
    SDL_EVENT_WINDOW_OCCLUDED,
    SDL_EVENT_WINDOW_ENTER_FULLSCREEN,
    SDL_EVENT_WINDOW_LEAVE_FULLSCREEN,
    SDL_EVENT_WINDOW_DESTROYED,



    SDL_EVENT_WINDOW_HDR_STATE_CHANGED,
    SDL_EVENT_WINDOW_SETTINGS_CHANGED,
    SDL_EVENT_WINDOW_FIRST = SDL_EVENT_WINDOW_SHOWN,
    SDL_EVENT_WINDOW_LAST = SDL_EVENT_WINDOW_SETTINGS_CHANGED,


    SDL_EVENT_KEY_DOWN        = 0x300,
    SDL_EVENT_KEY_UP,
    SDL_EVENT_TEXT_EDITING,
    SDL_EVENT_TEXT_INPUT,
    SDL_EVENT_KEYMAP_CHANGED,

    SDL_EVENT_KEYBOARD_ADDED,
    SDL_EVENT_KEYBOARD_REMOVED,
    SDL_EVENT_TEXT_EDITING_CANDIDATES,
    SDL_EVENT_SCREEN_KEYBOARD_SHOWN,
    SDL_EVENT_SCREEN_KEYBOARD_HIDDEN,


    SDL_EVENT_MOUSE_MOTION    = 0x400,
    SDL_EVENT_MOUSE_BUTTON_DOWN,
    SDL_EVENT_MOUSE_BUTTON_UP,
    SDL_EVENT_MOUSE_WHEEL,
    SDL_EVENT_MOUSE_ADDED,
    SDL_EVENT_MOUSE_REMOVED,


    SDL_EVENT_JOYSTICK_AXIS_MOTION  = 0x600,
    SDL_EVENT_JOYSTICK_BALL_MOTION,
    SDL_EVENT_JOYSTICK_HAT_MOTION,
    SDL_EVENT_JOYSTICK_BUTTON_DOWN,
    SDL_EVENT_JOYSTICK_BUTTON_UP,
    SDL_EVENT_JOYSTICK_ADDED,
    SDL_EVENT_JOYSTICK_REMOVED,
    SDL_EVENT_JOYSTICK_BATTERY_UPDATED,
    SDL_EVENT_JOYSTICK_UPDATE_COMPLETE,


    SDL_EVENT_GAMEPAD_AXIS_MOTION  = 0x650,
    SDL_EVENT_GAMEPAD_BUTTON_DOWN,
    SDL_EVENT_GAMEPAD_BUTTON_UP,
    SDL_EVENT_GAMEPAD_ADDED,
    SDL_EVENT_GAMEPAD_REMOVED,
    SDL_EVENT_GAMEPAD_REMAPPED,
    SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN,
    SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION,
    SDL_EVENT_GAMEPAD_TOUCHPAD_UP,
    SDL_EVENT_GAMEPAD_SENSOR_UPDATE,
    SDL_EVENT_GAMEPAD_UPDATE_COMPLETE,
    SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED,
    SDL_EVENT_GAMEPAD_CAPSENSE_TOUCH,
    SDL_EVENT_GAMEPAD_CAPSENSE_RELEASE,


    SDL_EVENT_FINGER_DOWN      = 0x700,
    SDL_EVENT_FINGER_UP,
    SDL_EVENT_FINGER_MOTION,
    SDL_EVENT_FINGER_CANCELED,


    SDL_EVENT_PINCH_BEGIN      = 0x710,
    SDL_EVENT_PINCH_UPDATE,
    SDL_EVENT_PINCH_END,




    SDL_EVENT_CLIPBOARD_UPDATE = 0x900,


    SDL_EVENT_DROP_FILE        = 0x1000,
    SDL_EVENT_DROP_TEXT,
    SDL_EVENT_DROP_BEGIN,
    SDL_EVENT_DROP_COMPLETE,
    SDL_EVENT_DROP_POSITION,


    SDL_EVENT_AUDIO_DEVICE_ADDED = 0x1100,
    SDL_EVENT_AUDIO_DEVICE_REMOVED,
    SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED,


    SDL_EVENT_SENSOR_UPDATE = 0x1200,


    SDL_EVENT_PEN_PROXIMITY_IN = 0x1300,
    SDL_EVENT_PEN_PROXIMITY_OUT,
    SDL_EVENT_PEN_DOWN,
    SDL_EVENT_PEN_UP,
    SDL_EVENT_PEN_BUTTON_DOWN,
    SDL_EVENT_PEN_BUTTON_UP,
    SDL_EVENT_PEN_MOTION,
    SDL_EVENT_PEN_AXIS,


    SDL_EVENT_CAMERA_DEVICE_ADDED = 0x1400,
    SDL_EVENT_CAMERA_DEVICE_REMOVED,
    SDL_EVENT_CAMERA_DEVICE_APPROVED,
    SDL_EVENT_CAMERA_DEVICE_DENIED,


    SDL_EVENT_RENDER_TARGETS_RESET = 0x2000,
    SDL_EVENT_RENDER_DEVICE_RESET,
    SDL_EVENT_RENDER_DEVICE_LOST,


    SDL_EVENT_PRIVATE0 = 0x4000,
    SDL_EVENT_PRIVATE1,
    SDL_EVENT_PRIVATE2,
    SDL_EVENT_PRIVATE3,


    SDL_EVENT_POLL_SENTINEL = 0x7F00,




    SDL_EVENT_USER    = 0x8000,




    SDL_EVENT_LAST    = 0xFFFF,


    SDL_EVENT_ENUM_PADDING = 0x7FFFFFFF

} SDL_EventType;






typedef struct SDL_CommonEvent
{
    Uint32 type;
    Uint32 reserved;
    Uint64 timestamp;
} SDL_CommonEvent;






typedef struct SDL_DisplayEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_DisplayID displayID;
    Sint32 data1;
    Sint32 data2;
} SDL_DisplayEvent;






typedef struct SDL_WindowEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_WindowID windowID;
    Sint32 data1;
    Sint32 data2;
} SDL_WindowEvent;






typedef struct SDL_KeyboardDeviceEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_KeyboardID which;
} SDL_KeyboardDeviceEvent;
# 363 "./SDL3/SDL_events.h"
typedef struct SDL_KeyboardEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_WindowID windowID;
    SDL_KeyboardID which;
    SDL_Scancode scancode;
    SDL_Keycode key;
    SDL_Keymod mod;
    Uint16 raw;
    bool down;
    bool repeat;
} SDL_KeyboardEvent;
# 387 "./SDL3/SDL_events.h"
typedef struct SDL_TextEditingEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_WindowID windowID;
    const char *text;
    Sint32 start;
    Sint32 length;
} SDL_TextEditingEvent;






typedef struct SDL_TextEditingCandidatesEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_WindowID windowID;
    const char * const *candidates;
    Sint32 num_candidates;
    Sint32 selected_candidate;
    bool horizontal;
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
} SDL_TextEditingCandidatesEvent;
# 429 "./SDL3/SDL_events.h"
typedef struct SDL_TextInputEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_WindowID windowID;
    const char *text;
} SDL_TextInputEvent;






typedef struct SDL_MouseDeviceEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_MouseID which;
} SDL_MouseDeviceEvent;






typedef struct SDL_MouseMotionEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_WindowID windowID;
    SDL_MouseID which;
    SDL_MouseButtonFlags state;
    float x;
    float y;
    float xrel;
    float yrel;
} SDL_MouseMotionEvent;






typedef struct SDL_MouseButtonEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_WindowID windowID;
    SDL_MouseID which;
    Uint8 button;
    bool down;
    Uint8 clicks;
    Uint8 padding;
    float x;
    float y;
} SDL_MouseButtonEvent;






typedef struct SDL_MouseWheelEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_WindowID windowID;
    SDL_MouseID which;
    float x;
    float y;
    SDL_MouseWheelDirection direction;
    float mouse_x;
    float mouse_y;
    Sint32 integer_x;
    Sint32 integer_y;
} SDL_MouseWheelEvent;






typedef struct SDL_JoyAxisEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_JoystickID which;
    Uint8 axis;
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
    Sint16 value;
    Uint16 padding4;
} SDL_JoyAxisEvent;






typedef struct SDL_JoyBallEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_JoystickID which;
    Uint8 ball;
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
    Sint16 xrel;
    Sint16 yrel;
} SDL_JoyBallEvent;






typedef struct SDL_JoyHatEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_JoystickID which;
    Uint8 hat;
    Uint8 value;






    Uint8 padding1;
    Uint8 padding2;
} SDL_JoyHatEvent;






typedef struct SDL_JoyButtonEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_JoystickID which;
    Uint8 button;
    bool down;
    Uint8 padding1;
    Uint8 padding2;
} SDL_JoyButtonEvent;
# 599 "./SDL3/SDL_events.h"
typedef struct SDL_JoyDeviceEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_JoystickID which;
} SDL_JoyDeviceEvent;






typedef struct SDL_JoyBatteryEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_JoystickID which;
    SDL_PowerState state;
    int percent;
} SDL_JoyBatteryEvent;






typedef struct SDL_GamepadAxisEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_JoystickID which;
    Uint8 axis;
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
    Sint16 value;
    Uint16 padding4;
} SDL_GamepadAxisEvent;







typedef struct SDL_GamepadButtonEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_JoystickID which;
    Uint8 button;
    bool down;
    Uint8 padding1;
    Uint8 padding2;
} SDL_GamepadButtonEvent;
# 674 "./SDL3/SDL_events.h"
typedef struct SDL_GamepadDeviceEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_JoystickID which;
} SDL_GamepadDeviceEvent;






typedef struct SDL_GamepadTouchpadEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_JoystickID which;
    Sint32 touchpad;
    Sint32 finger;
    float x;
    float y;
    float pressure;
} SDL_GamepadTouchpadEvent;






typedef struct SDL_GamepadSensorEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_JoystickID which;
    Sint32 sensor;
    float data[3];
    Uint64 sensor_timestamp;
} SDL_GamepadSensorEvent;






typedef struct SDL_GamepadCapSenseEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_JoystickID which;
    Uint8 capsense;
    bool down;
    Uint8 padding1;
    Uint8 padding2;
} SDL_GamepadCapSenseEvent;
# 742 "./SDL3/SDL_events.h"
typedef struct SDL_AudioDeviceEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_AudioDeviceID which;
    bool recording;
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
} SDL_AudioDeviceEvent;






typedef struct SDL_CameraDeviceEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_CameraID which;
} SDL_CameraDeviceEvent;







typedef struct SDL_RenderEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_WindowID windowID;
} SDL_RenderEvent;
# 801 "./SDL3/SDL_events.h"
typedef struct SDL_TouchFingerEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_TouchID touchID;
    SDL_FingerID fingerID;
    float x;
    float y;
    float dx;
    float dy;
    float pressure;
    SDL_WindowID windowID;
} SDL_TouchFingerEvent;




typedef struct SDL_PinchFingerEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    float scale;
    SDL_WindowID windowID;
} SDL_PinchFingerEvent;
# 846 "./SDL3/SDL_events.h"
typedef struct SDL_PenProximityEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_WindowID windowID;
    SDL_PenID which;
} SDL_PenProximityEvent;
# 866 "./SDL3/SDL_events.h"
typedef struct SDL_PenMotionEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_WindowID windowID;
    SDL_PenID which;
    SDL_PenInputFlags pen_state;
    float x;
    float y;
} SDL_PenMotionEvent;
# 886 "./SDL3/SDL_events.h"
typedef struct SDL_PenTouchEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_WindowID windowID;
    SDL_PenID which;
    SDL_PenInputFlags pen_state;
    float x;
    float y;
    bool eraser;
    bool down;
} SDL_PenTouchEvent;
# 908 "./SDL3/SDL_events.h"
typedef struct SDL_PenButtonEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_WindowID windowID;
    SDL_PenID which;
    SDL_PenInputFlags pen_state;
    float x;
    float y;
    Uint8 button;
    bool down;
} SDL_PenButtonEvent;
# 930 "./SDL3/SDL_events.h"
typedef struct SDL_PenAxisEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_WindowID windowID;
    SDL_PenID which;
    SDL_PenInputFlags pen_state;
    float x;
    float y;
    SDL_PenAxis axis;
    float value;
} SDL_PenAxisEvent;







typedef struct SDL_DropEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_WindowID windowID;
    float x;
    float y;
    const char *source;
    const char *data;
} SDL_DropEvent;







typedef struct SDL_ClipboardEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    bool owner;
    Sint32 num_mime_types;
    const char **mime_types;
} SDL_ClipboardEvent;






typedef struct SDL_SensorEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_SensorID which;
    float data[6];
    Uint64 sensor_timestamp;
} SDL_SensorEvent;






typedef struct SDL_QuitEvent
{
    SDL_EventType type;
    Uint32 reserved;
    Uint64 timestamp;
} SDL_QuitEvent;
# 1016 "./SDL3/SDL_events.h"
typedef struct SDL_UserEvent
{
    Uint32 type;
    Uint32 reserved;
    Uint64 timestamp;
    SDL_WindowID windowID;
    Sint32 code;
    void *data1;
    void *data2;
} SDL_UserEvent;
# 1036 "./SDL3/SDL_events.h"
typedef union SDL_Event
{
    Uint32 type;
    SDL_CommonEvent common;
    SDL_DisplayEvent display;
    SDL_WindowEvent window;
    SDL_KeyboardDeviceEvent kdevice;
    SDL_KeyboardEvent key;
    SDL_TextEditingEvent edit;
    SDL_TextEditingCandidatesEvent edit_candidates;
    SDL_TextInputEvent text;
    SDL_MouseDeviceEvent mdevice;
    SDL_MouseMotionEvent motion;
    SDL_MouseButtonEvent button;
    SDL_MouseWheelEvent wheel;
    SDL_JoyDeviceEvent jdevice;
    SDL_JoyAxisEvent jaxis;
    SDL_JoyBallEvent jball;
    SDL_JoyHatEvent jhat;
    SDL_JoyButtonEvent jbutton;
    SDL_JoyBatteryEvent jbattery;
    SDL_GamepadDeviceEvent gdevice;
    SDL_GamepadAxisEvent gaxis;
    SDL_GamepadButtonEvent gbutton;
    SDL_GamepadTouchpadEvent gtouchpad;
    SDL_GamepadSensorEvent gsensor;
    SDL_GamepadCapSenseEvent gcapsense;
    SDL_AudioDeviceEvent adevice;
    SDL_CameraDeviceEvent cdevice;
    SDL_SensorEvent sensor;
    SDL_QuitEvent quit;
    SDL_UserEvent user;
    SDL_TouchFingerEvent tfinger;
    SDL_PinchFingerEvent pinch;
    SDL_PenProximityEvent pproximity;
    SDL_PenTouchEvent ptouch;
    SDL_PenMotionEvent pmotion;
    SDL_PenButtonEvent pbutton;
    SDL_PenAxisEvent paxis;
    SDL_RenderEvent render;
    SDL_DropEvent drop;
    SDL_ClipboardEvent clipboard;
# 1092 "./SDL3/SDL_events.h"
    Uint8 padding[128];
} SDL_Event;


typedef int SDL_compile_time_assert_SDL_Event[(sizeof(SDL_Event) == sizeof((((SDL_Event *)(((void *)0))))->padding)) * 2 - 1];
# 1121 "./SDL3/SDL_events.h"
extern __attribute__((dllimport)) void  SDL_PumpEvents(void);
# 1130 "./SDL3/SDL_events.h"
typedef enum SDL_EventAction
{
    SDL_ADDEVENT,
    SDL_PEEKEVENT,
    SDL_GETEVENT
} SDL_EventAction;
# 1179 "./SDL3/SDL_events.h"
extern __attribute__((dllimport)) int  SDL_PeepEvents(SDL_Event *events, int numevents, SDL_EventAction action, Uint32 minType, Uint32 maxType);
# 1198 "./SDL3/SDL_events.h"
extern __attribute__((dllimport)) bool  SDL_HasEvent(Uint32 type);
# 1219 "./SDL3/SDL_events.h"
extern __attribute__((dllimport)) bool  SDL_HasEvents(Uint32 minType, Uint32 maxType);
# 1247 "./SDL3/SDL_events.h"
extern __attribute__((dllimport)) void  SDL_FlushEvent(Uint32 type);
# 1274 "./SDL3/SDL_events.h"
extern __attribute__((dllimport)) void  SDL_FlushEvents(Uint32 minType, Uint32 maxType);
# 1325 "./SDL3/SDL_events.h"
extern __attribute__((dllimport)) bool  SDL_PollEvent(SDL_Event *event);
# 1349 "./SDL3/SDL_events.h"
extern __attribute__((dllimport)) bool  SDL_WaitEvent(SDL_Event *event);
# 1379 "./SDL3/SDL_events.h"
extern __attribute__((dllimport)) bool  SDL_WaitEventTimeout(SDL_Event *event, Sint32 timeoutMS);
# 1413 "./SDL3/SDL_events.h"
extern __attribute__((dllimport)) bool  SDL_PushEvent(SDL_Event *event);
# 1434 "./SDL3/SDL_events.h"
typedef bool ( *SDL_EventFilter)(void *userdata, SDL_Event *event);
# 1478 "./SDL3/SDL_events.h"
extern __attribute__((dllimport)) void  SDL_SetEventFilter(SDL_EventFilter filter, void *userdata);
# 1497 "./SDL3/SDL_events.h"
extern __attribute__((dllimport)) bool  SDL_GetEventFilter(SDL_EventFilter *filter, void **userdata);
# 1529 "./SDL3/SDL_events.h"
extern __attribute__((dllimport)) bool  SDL_AddEventWatch(SDL_EventFilter filter, void *userdata);
# 1546 "./SDL3/SDL_events.h"
extern __attribute__((dllimport)) void  SDL_RemoveEventWatch(SDL_EventFilter filter, void *userdata);
# 1566 "./SDL3/SDL_events.h"
extern __attribute__((dllimport)) void  SDL_FilterEvents(SDL_EventFilter filter, void *userdata);
# 1580 "./SDL3/SDL_events.h"
extern __attribute__((dllimport)) void  SDL_SetEventEnabled(Uint32 type, bool enabled);
# 1594 "./SDL3/SDL_events.h"
extern __attribute__((dllimport)) bool  SDL_EventEnabled(Uint32 type);
# 1610 "./SDL3/SDL_events.h"
extern __attribute__((dllimport)) Uint32  SDL_RegisterEvents(int numevents);
# 1626 "./SDL3/SDL_events.h"
extern __attribute__((dllimport)) SDL_Window *  SDL_GetWindowFromEvent(const SDL_Event *event);
# 1658 "./SDL3/SDL_events.h"
extern __attribute__((dllimport)) int  SDL_GetEventDescription(const SDL_Event *event, char *buf, int buflen);






# 1 "./SDL3/SDL_close_code.h" 1
# 1665 "./SDL3/SDL_events.h" 2
# 49 "SDL3/SDL.h" 2

# 1 "./SDL3/SDL_filesystem.h" 1
# 51 "./SDL3/SDL_filesystem.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 51 "./SDL3/SDL_filesystem.h" 2
# 101 "./SDL3/SDL_filesystem.h"
extern __attribute__((dllimport)) const char *  SDL_GetBasePath(void);
# 164 "./SDL3/SDL_filesystem.h"
extern __attribute__((dllimport)) char *  SDL_GetPrefPath(const char *org, const char *app);
# 195 "./SDL3/SDL_filesystem.h"
typedef enum SDL_Folder
{
    SDL_FOLDER_HOME,
    SDL_FOLDER_DESKTOP,
    SDL_FOLDER_DOCUMENTS,
    SDL_FOLDER_DOWNLOADS,
    SDL_FOLDER_MUSIC,
    SDL_FOLDER_PICTURES,
    SDL_FOLDER_PUBLICSHARE,
    SDL_FOLDER_SAVEDGAMES,
    SDL_FOLDER_SCREENSHOTS,
    SDL_FOLDER_TEMPLATES,
    SDL_FOLDER_VIDEOS,
    SDL_FOLDER_COUNT
} SDL_Folder;
# 236 "./SDL3/SDL_filesystem.h"
extern __attribute__((dllimport)) const char *  SDL_GetUserFolder(SDL_Folder folder);
# 251 "./SDL3/SDL_filesystem.h"
typedef enum SDL_PathType
{
    SDL_PATHTYPE_NONE,
    SDL_PATHTYPE_FILE,
    SDL_PATHTYPE_DIRECTORY,
    SDL_PATHTYPE_OTHER
} SDL_PathType;
# 268 "./SDL3/SDL_filesystem.h"
typedef struct SDL_PathInfo
{
    SDL_PathType type;
    Uint64 size;
    SDL_Time create_time;
    SDL_Time modify_time;
    SDL_Time access_time;
} SDL_PathInfo;
# 285 "./SDL3/SDL_filesystem.h"
typedef Uint32 SDL_GlobFlags;
# 305 "./SDL3/SDL_filesystem.h"
extern __attribute__((dllimport)) bool  SDL_CreateDirectory(const char *path);
# 314 "./SDL3/SDL_filesystem.h"
typedef enum SDL_EnumerationResult
{
    SDL_ENUM_CONTINUE,
    SDL_ENUM_SUCCESS,
    SDL_ENUM_FAILURE
} SDL_EnumerationResult;
# 345 "./SDL3/SDL_filesystem.h"
typedef SDL_EnumerationResult ( *SDL_EnumerateDirectoryCallback)(void *userdata, const char *dirname, const char *fname);
# 370 "./SDL3/SDL_filesystem.h"
extern __attribute__((dllimport)) bool  SDL_EnumerateDirectory(const char *path, SDL_EnumerateDirectoryCallback callback, void *userdata);
# 386 "./SDL3/SDL_filesystem.h"
extern __attribute__((dllimport)) bool  SDL_RemovePath(const char *path);
# 411 "./SDL3/SDL_filesystem.h"
extern __attribute__((dllimport)) bool  SDL_RenamePath(const char *oldpath, const char *newpath);
# 455 "./SDL3/SDL_filesystem.h"
extern __attribute__((dllimport)) bool  SDL_CopyFile(const char *oldpath, const char *newpath);
# 474 "./SDL3/SDL_filesystem.h"
extern __attribute__((dllimport)) bool  SDL_GetPathInfo(const char *path, SDL_PathInfo *info);
# 507 "./SDL3/SDL_filesystem.h"
extern __attribute__((dllimport)) char **  SDL_GlobDirectory(const char *path, const char *pattern, SDL_GlobFlags flags, int *count);
# 530 "./SDL3/SDL_filesystem.h"
extern __attribute__((dllimport)) char *  SDL_GetCurrentDirectory(void);






# 1 "./SDL3/SDL_close_code.h" 1
# 537 "./SDL3/SDL_filesystem.h" 2
# 50 "SDL3/SDL.h" 2


# 1 "./SDL3/SDL_gpu.h" 1
# 400 "./SDL3/SDL_gpu.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 400 "./SDL3/SDL_gpu.h" 2
# 411 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUDevice SDL_GPUDevice;
# 435 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUBuffer SDL_GPUBuffer;
# 453 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUTransferBuffer SDL_GPUTransferBuffer;
# 473 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUTexture SDL_GPUTexture;
# 485 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUSampler SDL_GPUSampler;
# 496 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUShader SDL_GPUShader;
# 509 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUComputePipeline SDL_GPUComputePipeline;
# 522 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUGraphicsPipeline SDL_GPUGraphicsPipeline;
# 547 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUCommandBuffer SDL_GPUCommandBuffer;
# 560 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPURenderPass SDL_GPURenderPass;
# 573 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUComputePass SDL_GPUComputePass;
# 586 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUCopyPass SDL_GPUCopyPass;
# 598 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUFence SDL_GPUFence;
# 621 "./SDL3/SDL_gpu.h"
typedef enum SDL_GPUPrimitiveType
{
    SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
    SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP,
    SDL_GPU_PRIMITIVETYPE_LINELIST,
    SDL_GPU_PRIMITIVETYPE_LINESTRIP,
    SDL_GPU_PRIMITIVETYPE_POINTLIST
} SDL_GPUPrimitiveType;
# 638 "./SDL3/SDL_gpu.h"
typedef enum SDL_GPULoadOp
{
    SDL_GPU_LOADOP_LOAD,
    SDL_GPU_LOADOP_CLEAR,
    SDL_GPU_LOADOP_DONT_CARE
} SDL_GPULoadOp;
# 653 "./SDL3/SDL_gpu.h"
typedef enum SDL_GPUStoreOp
{
    SDL_GPU_STOREOP_STORE,
    SDL_GPU_STOREOP_DONT_CARE,
    SDL_GPU_STOREOP_RESOLVE,
    SDL_GPU_STOREOP_RESOLVE_AND_STORE
} SDL_GPUStoreOp;
# 668 "./SDL3/SDL_gpu.h"
typedef enum SDL_GPUIndexElementSize
{
    SDL_GPU_INDEXELEMENTSIZE_16BIT,
    SDL_GPU_INDEXELEMENTSIZE_32BIT
} SDL_GPUIndexElementSize;
# 759 "./SDL3/SDL_gpu.h"
typedef enum SDL_GPUTextureFormat
{
    SDL_GPU_TEXTUREFORMAT_INVALID,


    SDL_GPU_TEXTUREFORMAT_A8_UNORM,
    SDL_GPU_TEXTUREFORMAT_R8_UNORM,
    SDL_GPU_TEXTUREFORMAT_R8G8_UNORM,
    SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
    SDL_GPU_TEXTUREFORMAT_R16_UNORM,
    SDL_GPU_TEXTUREFORMAT_R16G16_UNORM,
    SDL_GPU_TEXTUREFORMAT_R16G16B16A16_UNORM,
    SDL_GPU_TEXTUREFORMAT_R10G10B10A2_UNORM,
    SDL_GPU_TEXTUREFORMAT_B5G6R5_UNORM,
    SDL_GPU_TEXTUREFORMAT_B5G5R5A1_UNORM,
    SDL_GPU_TEXTUREFORMAT_B4G4R4A4_UNORM,
    SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM,

    SDL_GPU_TEXTUREFORMAT_BC1_RGBA_UNORM,
    SDL_GPU_TEXTUREFORMAT_BC2_RGBA_UNORM,
    SDL_GPU_TEXTUREFORMAT_BC3_RGBA_UNORM,
    SDL_GPU_TEXTUREFORMAT_BC4_R_UNORM,
    SDL_GPU_TEXTUREFORMAT_BC5_RG_UNORM,
    SDL_GPU_TEXTUREFORMAT_BC7_RGBA_UNORM,

    SDL_GPU_TEXTUREFORMAT_BC6H_RGB_FLOAT,

    SDL_GPU_TEXTUREFORMAT_BC6H_RGB_UFLOAT,

    SDL_GPU_TEXTUREFORMAT_R8_SNORM,
    SDL_GPU_TEXTUREFORMAT_R8G8_SNORM,
    SDL_GPU_TEXTUREFORMAT_R8G8B8A8_SNORM,
    SDL_GPU_TEXTUREFORMAT_R16_SNORM,
    SDL_GPU_TEXTUREFORMAT_R16G16_SNORM,
    SDL_GPU_TEXTUREFORMAT_R16G16B16A16_SNORM,

    SDL_GPU_TEXTUREFORMAT_R16_FLOAT,
    SDL_GPU_TEXTUREFORMAT_R16G16_FLOAT,
    SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT,
    SDL_GPU_TEXTUREFORMAT_R32_FLOAT,
    SDL_GPU_TEXTUREFORMAT_R32G32_FLOAT,
    SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT,

    SDL_GPU_TEXTUREFORMAT_R11G11B10_UFLOAT,

    SDL_GPU_TEXTUREFORMAT_R8_UINT,
    SDL_GPU_TEXTUREFORMAT_R8G8_UINT,
    SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UINT,
    SDL_GPU_TEXTUREFORMAT_R16_UINT,
    SDL_GPU_TEXTUREFORMAT_R16G16_UINT,
    SDL_GPU_TEXTUREFORMAT_R16G16B16A16_UINT,
    SDL_GPU_TEXTUREFORMAT_R32_UINT,
    SDL_GPU_TEXTUREFORMAT_R32G32_UINT,
    SDL_GPU_TEXTUREFORMAT_R32G32B32A32_UINT,

    SDL_GPU_TEXTUREFORMAT_R8_INT,
    SDL_GPU_TEXTUREFORMAT_R8G8_INT,
    SDL_GPU_TEXTUREFORMAT_R8G8B8A8_INT,
    SDL_GPU_TEXTUREFORMAT_R16_INT,
    SDL_GPU_TEXTUREFORMAT_R16G16_INT,
    SDL_GPU_TEXTUREFORMAT_R16G16B16A16_INT,
    SDL_GPU_TEXTUREFORMAT_R32_INT,
    SDL_GPU_TEXTUREFORMAT_R32G32_INT,
    SDL_GPU_TEXTUREFORMAT_R32G32B32A32_INT,

    SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM_SRGB,
    SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM_SRGB,

    SDL_GPU_TEXTUREFORMAT_BC1_RGBA_UNORM_SRGB,
    SDL_GPU_TEXTUREFORMAT_BC2_RGBA_UNORM_SRGB,
    SDL_GPU_TEXTUREFORMAT_BC3_RGBA_UNORM_SRGB,
    SDL_GPU_TEXTUREFORMAT_BC7_RGBA_UNORM_SRGB,

    SDL_GPU_TEXTUREFORMAT_D16_UNORM,
    SDL_GPU_TEXTUREFORMAT_D24_UNORM,
    SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
    SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT,
    SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT,

    SDL_GPU_TEXTUREFORMAT_ASTC_4x4_UNORM,
    SDL_GPU_TEXTUREFORMAT_ASTC_5x4_UNORM,
    SDL_GPU_TEXTUREFORMAT_ASTC_5x5_UNORM,
    SDL_GPU_TEXTUREFORMAT_ASTC_6x5_UNORM,
    SDL_GPU_TEXTUREFORMAT_ASTC_6x6_UNORM,
    SDL_GPU_TEXTUREFORMAT_ASTC_8x5_UNORM,
    SDL_GPU_TEXTUREFORMAT_ASTC_8x6_UNORM,
    SDL_GPU_TEXTUREFORMAT_ASTC_8x8_UNORM,
    SDL_GPU_TEXTUREFORMAT_ASTC_10x5_UNORM,
    SDL_GPU_TEXTUREFORMAT_ASTC_10x6_UNORM,
    SDL_GPU_TEXTUREFORMAT_ASTC_10x8_UNORM,
    SDL_GPU_TEXTUREFORMAT_ASTC_10x10_UNORM,
    SDL_GPU_TEXTUREFORMAT_ASTC_12x10_UNORM,
    SDL_GPU_TEXTUREFORMAT_ASTC_12x12_UNORM,

    SDL_GPU_TEXTUREFORMAT_ASTC_4x4_UNORM_SRGB,
    SDL_GPU_TEXTUREFORMAT_ASTC_5x4_UNORM_SRGB,
    SDL_GPU_TEXTUREFORMAT_ASTC_5x5_UNORM_SRGB,
    SDL_GPU_TEXTUREFORMAT_ASTC_6x5_UNORM_SRGB,
    SDL_GPU_TEXTUREFORMAT_ASTC_6x6_UNORM_SRGB,
    SDL_GPU_TEXTUREFORMAT_ASTC_8x5_UNORM_SRGB,
    SDL_GPU_TEXTUREFORMAT_ASTC_8x6_UNORM_SRGB,
    SDL_GPU_TEXTUREFORMAT_ASTC_8x8_UNORM_SRGB,
    SDL_GPU_TEXTUREFORMAT_ASTC_10x5_UNORM_SRGB,
    SDL_GPU_TEXTUREFORMAT_ASTC_10x6_UNORM_SRGB,
    SDL_GPU_TEXTUREFORMAT_ASTC_10x8_UNORM_SRGB,
    SDL_GPU_TEXTUREFORMAT_ASTC_10x10_UNORM_SRGB,
    SDL_GPU_TEXTUREFORMAT_ASTC_12x10_UNORM_SRGB,
    SDL_GPU_TEXTUREFORMAT_ASTC_12x12_UNORM_SRGB,

    SDL_GPU_TEXTUREFORMAT_ASTC_4x4_FLOAT,
    SDL_GPU_TEXTUREFORMAT_ASTC_5x4_FLOAT,
    SDL_GPU_TEXTUREFORMAT_ASTC_5x5_FLOAT,
    SDL_GPU_TEXTUREFORMAT_ASTC_6x5_FLOAT,
    SDL_GPU_TEXTUREFORMAT_ASTC_6x6_FLOAT,
    SDL_GPU_TEXTUREFORMAT_ASTC_8x5_FLOAT,
    SDL_GPU_TEXTUREFORMAT_ASTC_8x6_FLOAT,
    SDL_GPU_TEXTUREFORMAT_ASTC_8x8_FLOAT,
    SDL_GPU_TEXTUREFORMAT_ASTC_10x5_FLOAT,
    SDL_GPU_TEXTUREFORMAT_ASTC_10x6_FLOAT,
    SDL_GPU_TEXTUREFORMAT_ASTC_10x8_FLOAT,
    SDL_GPU_TEXTUREFORMAT_ASTC_10x10_FLOAT,
    SDL_GPU_TEXTUREFORMAT_ASTC_12x10_FLOAT,
    SDL_GPU_TEXTUREFORMAT_ASTC_12x12_FLOAT
} SDL_GPUTextureFormat;
# 904 "./SDL3/SDL_gpu.h"
typedef Uint32 SDL_GPUTextureUsageFlags;
# 921 "./SDL3/SDL_gpu.h"
typedef enum SDL_GPUTextureType
{
    SDL_GPU_TEXTURETYPE_2D,
    SDL_GPU_TEXTURETYPE_2D_ARRAY,
    SDL_GPU_TEXTURETYPE_3D,
    SDL_GPU_TEXTURETYPE_CUBE,
    SDL_GPU_TEXTURETYPE_CUBE_ARRAY
} SDL_GPUTextureType;
# 941 "./SDL3/SDL_gpu.h"
typedef enum SDL_GPUSampleCount
{
    SDL_GPU_SAMPLECOUNT_1,
    SDL_GPU_SAMPLECOUNT_2,
    SDL_GPU_SAMPLECOUNT_4,
    SDL_GPU_SAMPLECOUNT_8
} SDL_GPUSampleCount;
# 957 "./SDL3/SDL_gpu.h"
typedef enum SDL_GPUCubeMapFace
{
    SDL_GPU_CUBEMAPFACE_POSITIVEX,
    SDL_GPU_CUBEMAPFACE_NEGATIVEX,
    SDL_GPU_CUBEMAPFACE_POSITIVEY,
    SDL_GPU_CUBEMAPFACE_NEGATIVEY,
    SDL_GPU_CUBEMAPFACE_POSITIVEZ,
    SDL_GPU_CUBEMAPFACE_NEGATIVEZ
} SDL_GPUCubeMapFace;
# 984 "./SDL3/SDL_gpu.h"
typedef Uint32 SDL_GPUBufferUsageFlags;
# 1003 "./SDL3/SDL_gpu.h"
typedef enum SDL_GPUTransferBufferUsage
{
    SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
    SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD
} SDL_GPUTransferBufferUsage;
# 1016 "./SDL3/SDL_gpu.h"
typedef enum SDL_GPUShaderStage
{
    SDL_GPU_SHADERSTAGE_VERTEX,
    SDL_GPU_SHADERSTAGE_FRAGMENT
} SDL_GPUShaderStage;
# 1031 "./SDL3/SDL_gpu.h"
typedef Uint32 SDL_GPUShaderFormat;
# 1048 "./SDL3/SDL_gpu.h"
typedef enum SDL_GPUVertexElementFormat
{
    SDL_GPU_VERTEXELEMENTFORMAT_INVALID,


    SDL_GPU_VERTEXELEMENTFORMAT_INT,
    SDL_GPU_VERTEXELEMENTFORMAT_INT2,
    SDL_GPU_VERTEXELEMENTFORMAT_INT3,
    SDL_GPU_VERTEXELEMENTFORMAT_INT4,


    SDL_GPU_VERTEXELEMENTFORMAT_UINT,
    SDL_GPU_VERTEXELEMENTFORMAT_UINT2,
    SDL_GPU_VERTEXELEMENTFORMAT_UINT3,
    SDL_GPU_VERTEXELEMENTFORMAT_UINT4,


    SDL_GPU_VERTEXELEMENTFORMAT_FLOAT,
    SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
    SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
    SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,


    SDL_GPU_VERTEXELEMENTFORMAT_BYTE2,
    SDL_GPU_VERTEXELEMENTFORMAT_BYTE4,


    SDL_GPU_VERTEXELEMENTFORMAT_UBYTE2,
    SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4,


    SDL_GPU_VERTEXELEMENTFORMAT_BYTE2_NORM,
    SDL_GPU_VERTEXELEMENTFORMAT_BYTE4_NORM,


    SDL_GPU_VERTEXELEMENTFORMAT_UBYTE2_NORM,
    SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM,


    SDL_GPU_VERTEXELEMENTFORMAT_SHORT2,
    SDL_GPU_VERTEXELEMENTFORMAT_SHORT4,


    SDL_GPU_VERTEXELEMENTFORMAT_USHORT2,
    SDL_GPU_VERTEXELEMENTFORMAT_USHORT4,


    SDL_GPU_VERTEXELEMENTFORMAT_SHORT2_NORM,
    SDL_GPU_VERTEXELEMENTFORMAT_SHORT4_NORM,


    SDL_GPU_VERTEXELEMENTFORMAT_USHORT2_NORM,
    SDL_GPU_VERTEXELEMENTFORMAT_USHORT4_NORM,


    SDL_GPU_VERTEXELEMENTFORMAT_HALF2,
    SDL_GPU_VERTEXELEMENTFORMAT_HALF4
} SDL_GPUVertexElementFormat;
# 1114 "./SDL3/SDL_gpu.h"
typedef enum SDL_GPUVertexInputRate
{
    SDL_GPU_VERTEXINPUTRATE_VERTEX,
    SDL_GPU_VERTEXINPUTRATE_INSTANCE
} SDL_GPUVertexInputRate;
# 1127 "./SDL3/SDL_gpu.h"
typedef enum SDL_GPUFillMode
{
    SDL_GPU_FILLMODE_FILL,
    SDL_GPU_FILLMODE_LINE
} SDL_GPUFillMode;
# 1140 "./SDL3/SDL_gpu.h"
typedef enum SDL_GPUCullMode
{
    SDL_GPU_CULLMODE_NONE,
    SDL_GPU_CULLMODE_FRONT,
    SDL_GPU_CULLMODE_BACK
} SDL_GPUCullMode;
# 1155 "./SDL3/SDL_gpu.h"
typedef enum SDL_GPUFrontFace
{
    SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
    SDL_GPU_FRONTFACE_CLOCKWISE
} SDL_GPUFrontFace;
# 1168 "./SDL3/SDL_gpu.h"
typedef enum SDL_GPUCompareOp
{
    SDL_GPU_COMPAREOP_INVALID,
    SDL_GPU_COMPAREOP_NEVER,
    SDL_GPU_COMPAREOP_LESS,
    SDL_GPU_COMPAREOP_EQUAL,
    SDL_GPU_COMPAREOP_LESS_OR_EQUAL,
    SDL_GPU_COMPAREOP_GREATER,
    SDL_GPU_COMPAREOP_NOT_EQUAL,
    SDL_GPU_COMPAREOP_GREATER_OR_EQUAL,
    SDL_GPU_COMPAREOP_ALWAYS
} SDL_GPUCompareOp;
# 1189 "./SDL3/SDL_gpu.h"
typedef enum SDL_GPUStencilOp
{
    SDL_GPU_STENCILOP_INVALID,
    SDL_GPU_STENCILOP_KEEP,
    SDL_GPU_STENCILOP_ZERO,
    SDL_GPU_STENCILOP_REPLACE,
    SDL_GPU_STENCILOP_INCREMENT_AND_CLAMP,
    SDL_GPU_STENCILOP_DECREMENT_AND_CLAMP,
    SDL_GPU_STENCILOP_INVERT,
    SDL_GPU_STENCILOP_INCREMENT_AND_WRAP,
    SDL_GPU_STENCILOP_DECREMENT_AND_WRAP
} SDL_GPUStencilOp;
# 1213 "./SDL3/SDL_gpu.h"
typedef enum SDL_GPUBlendOp
{
    SDL_GPU_BLENDOP_INVALID,
    SDL_GPU_BLENDOP_ADD,
    SDL_GPU_BLENDOP_SUBTRACT,
    SDL_GPU_BLENDOP_REVERSE_SUBTRACT,
    SDL_GPU_BLENDOP_MIN,
    SDL_GPU_BLENDOP_MAX
} SDL_GPUBlendOp;
# 1234 "./SDL3/SDL_gpu.h"
typedef enum SDL_GPUBlendFactor
{
    SDL_GPU_BLENDFACTOR_INVALID,
    SDL_GPU_BLENDFACTOR_ZERO,
    SDL_GPU_BLENDFACTOR_ONE,
    SDL_GPU_BLENDFACTOR_SRC_COLOR,
    SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_COLOR,
    SDL_GPU_BLENDFACTOR_DST_COLOR,
    SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_COLOR,
    SDL_GPU_BLENDFACTOR_SRC_ALPHA,
    SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
    SDL_GPU_BLENDFACTOR_DST_ALPHA,
    SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_ALPHA,
    SDL_GPU_BLENDFACTOR_CONSTANT_COLOR,
    SDL_GPU_BLENDFACTOR_ONE_MINUS_CONSTANT_COLOR,
    SDL_GPU_BLENDFACTOR_SRC_ALPHA_SATURATE
} SDL_GPUBlendFactor;
# 1259 "./SDL3/SDL_gpu.h"
typedef Uint8 SDL_GPUColorComponentFlags;
# 1273 "./SDL3/SDL_gpu.h"
typedef enum SDL_GPUFilter
{
    SDL_GPU_FILTER_NEAREST,
    SDL_GPU_FILTER_LINEAR
} SDL_GPUFilter;
# 1286 "./SDL3/SDL_gpu.h"
typedef enum SDL_GPUSamplerMipmapMode
{
    SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
    SDL_GPU_SAMPLERMIPMAPMODE_LINEAR
} SDL_GPUSamplerMipmapMode;
# 1300 "./SDL3/SDL_gpu.h"
typedef enum SDL_GPUSamplerAddressMode
{
    SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
    SDL_GPU_SAMPLERADDRESSMODE_MIRRORED_REPEAT,
    SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE
} SDL_GPUSamplerAddressMode;
# 1332 "./SDL3/SDL_gpu.h"
typedef enum SDL_GPUPresentMode
{
    SDL_GPU_PRESENTMODE_VSYNC,
    SDL_GPU_PRESENTMODE_IMMEDIATE,
    SDL_GPU_PRESENTMODE_MAILBOX
} SDL_GPUPresentMode;
# 1365 "./SDL3/SDL_gpu.h"
typedef enum SDL_GPUSwapchainComposition
{
    SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
    SDL_GPU_SWAPCHAINCOMPOSITION_SDR_LINEAR,
    SDL_GPU_SWAPCHAINCOMPOSITION_HDR_EXTENDED_LINEAR,
    SDL_GPU_SWAPCHAINCOMPOSITION_HDR10_ST2084
} SDL_GPUSwapchainComposition;
# 1382 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUViewport
{
    float x;
    float y;
    float w;
    float h;
    float min_depth;
    float max_depth;
} SDL_GPUViewport;
# 1413 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUTextureTransferInfo
{
    SDL_GPUTransferBuffer *transfer_buffer;
    Uint32 offset;
    Uint32 pixels_per_row;
    Uint32 rows_per_layer;
} SDL_GPUTextureTransferInfo;
# 1431 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUTransferBufferLocation
{
    SDL_GPUTransferBuffer *transfer_buffer;
    Uint32 offset;
} SDL_GPUTransferBufferLocation;
# 1446 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUTextureLocation
{
    SDL_GPUTexture *texture;
    Uint32 mip_level;
    Uint32 layer;
    Uint32 x;
    Uint32 y;
    Uint32 z;
} SDL_GPUTextureLocation;
# 1467 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUTextureRegion
{
    SDL_GPUTexture *texture;
    Uint32 mip_level;
    Uint32 layer;
    Uint32 x;
    Uint32 y;
    Uint32 z;
    Uint32 w;
    Uint32 h;
    Uint32 d;
} SDL_GPUTextureRegion;
# 1487 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUBlitRegion
{
    SDL_GPUTexture *texture;
    Uint32 mip_level;
    Uint32 layer_or_depth_plane;
    Uint32 x;
    Uint32 y;
    Uint32 w;
    Uint32 h;
} SDL_GPUBlitRegion;
# 1507 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUBufferLocation
{
    SDL_GPUBuffer *buffer;
    Uint32 offset;
} SDL_GPUBufferLocation;
# 1523 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUBufferRegion
{
    SDL_GPUBuffer *buffer;
    Uint32 offset;
    Uint32 size;
} SDL_GPUBufferRegion;
# 1544 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUIndirectDrawCommand
{
    Uint32 num_vertices;
    Uint32 num_instances;
    Uint32 first_vertex;
    Uint32 first_instance;
} SDL_GPUIndirectDrawCommand;
# 1566 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUIndexedIndirectDrawCommand
{
    Uint32 num_indices;
    Uint32 num_instances;
    Uint32 first_index;
    Sint32 vertex_offset;
    Uint32 first_instance;
} SDL_GPUIndexedIndirectDrawCommand;
# 1582 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUIndirectDispatchCommand
{
    Uint32 groupcount_x;
    Uint32 groupcount_y;
    Uint32 groupcount_z;
} SDL_GPUIndirectDispatchCommand;
# 1605 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUSamplerCreateInfo
{
    SDL_GPUFilter min_filter;
    SDL_GPUFilter mag_filter;
    SDL_GPUSamplerMipmapMode mipmap_mode;
    SDL_GPUSamplerAddressMode address_mode_u;
    SDL_GPUSamplerAddressMode address_mode_v;
    SDL_GPUSamplerAddressMode address_mode_w;
    float mip_lod_bias;
    float max_anisotropy;
    SDL_GPUCompareOp compare_op;
    float min_lod;
    float max_lod;
    bool enable_anisotropy;
    bool enable_compare;
    Uint8 padding1;
    Uint8 padding2;

    SDL_PropertiesID props;
} SDL_GPUSamplerCreateInfo;
# 1644 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUVertexBufferDescription
{
    Uint32 slot;
    Uint32 pitch;
    SDL_GPUVertexInputRate input_rate;
    Uint32 instance_step_rate;
} SDL_GPUVertexBufferDescription;
# 1664 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUVertexAttribute
{
    Uint32 location;
    Uint32 buffer_slot;
    SDL_GPUVertexElementFormat format;
    Uint32 offset;
} SDL_GPUVertexAttribute;
# 1682 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUVertexInputState
{
    const SDL_GPUVertexBufferDescription *vertex_buffer_descriptions;
    Uint32 num_vertex_buffers;
    const SDL_GPUVertexAttribute *vertex_attributes;
    Uint32 num_vertex_attributes;
} SDL_GPUVertexInputState;
# 1697 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUStencilOpState
{
    SDL_GPUStencilOp fail_op;
    SDL_GPUStencilOp pass_op;
    SDL_GPUStencilOp depth_fail_op;
    SDL_GPUCompareOp compare_op;
} SDL_GPUStencilOpState;
# 1715 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUColorTargetBlendState
{
    SDL_GPUBlendFactor src_color_blendfactor;
    SDL_GPUBlendFactor dst_color_blendfactor;
    SDL_GPUBlendOp color_blend_op;
    SDL_GPUBlendFactor src_alpha_blendfactor;
    SDL_GPUBlendFactor dst_alpha_blendfactor;
    SDL_GPUBlendOp alpha_blend_op;
    SDL_GPUColorComponentFlags color_write_mask;
    bool enable_blend;
    bool enable_color_write_mask;
    Uint8 padding1;
    Uint8 padding2;
} SDL_GPUColorTargetBlendState;
# 1740 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUShaderCreateInfo
{
    uint64 code_size;
    const Uint8 *code;
    const char *entrypoint;
    SDL_GPUShaderFormat format;
    SDL_GPUShaderStage stage;
    Uint32 num_samplers;
    Uint32 num_storage_textures;
    Uint32 num_storage_buffers;
    Uint32 num_uniform_buffers;

    SDL_PropertiesID props;
} SDL_GPUShaderCreateInfo;
# 1770 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUTextureCreateInfo
{
    SDL_GPUTextureType type;
    SDL_GPUTextureFormat format;
    SDL_GPUTextureUsageFlags usage;
    Uint32 width;
    Uint32 height;
    Uint32 layer_count_or_depth;
    Uint32 num_levels;
    SDL_GPUSampleCount sample_count;

    SDL_PropertiesID props;
} SDL_GPUTextureCreateInfo;
# 1795 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUBufferCreateInfo
{
    SDL_GPUBufferUsageFlags usage;
    Uint32 size;

    SDL_PropertiesID props;
} SDL_GPUBufferCreateInfo;
# 1811 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUTransferBufferCreateInfo
{
    SDL_GPUTransferBufferUsage usage;
    Uint32 size;

    SDL_PropertiesID props;
} SDL_GPUTransferBufferCreateInfo;
# 1837 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPURasterizerState
{
    SDL_GPUFillMode fill_mode;
    SDL_GPUCullMode cull_mode;
    SDL_GPUFrontFace front_face;
    float depth_bias_constant_factor;
    float depth_bias_clamp;
    float depth_bias_slope_factor;
    bool enable_depth_bias;
    bool enable_depth_clip;
    Uint8 padding1;
    Uint8 padding2;
} SDL_GPURasterizerState;
# 1859 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUMultisampleState
{
    SDL_GPUSampleCount sample_count;
    Uint32 sample_mask;
    bool enable_mask;
    bool enable_alpha_to_coverage;
    Uint8 padding2;
    Uint8 padding3;
} SDL_GPUMultisampleState;
# 1878 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUDepthStencilState
{
    SDL_GPUCompareOp compare_op;
    SDL_GPUStencilOpState back_stencil_state;
    SDL_GPUStencilOpState front_stencil_state;
    Uint8 compare_mask;
    Uint8 write_mask;
    bool enable_depth_test;
    bool enable_depth_write;
    bool enable_stencil_test;
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
} SDL_GPUDepthStencilState;
# 1901 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUColorTargetDescription
{
    SDL_GPUTextureFormat format;
    SDL_GPUColorTargetBlendState blend_state;
} SDL_GPUColorTargetDescription;
# 1917 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUGraphicsPipelineTargetInfo
{
    const SDL_GPUColorTargetDescription *color_target_descriptions;
    Uint32 num_color_targets;
    SDL_GPUTextureFormat depth_stencil_format;
    bool has_depth_stencil_target;
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
} SDL_GPUGraphicsPipelineTargetInfo;
# 1942 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUGraphicsPipelineCreateInfo
{
    SDL_GPUShader *vertex_shader;
    SDL_GPUShader *fragment_shader;
    SDL_GPUVertexInputState vertex_input_state;
    SDL_GPUPrimitiveType primitive_type;
    SDL_GPURasterizerState rasterizer_state;
    SDL_GPUMultisampleState multisample_state;
    SDL_GPUDepthStencilState depth_stencil_state;
    SDL_GPUGraphicsPipelineTargetInfo target_info;

    SDL_PropertiesID props;
} SDL_GPUGraphicsPipelineCreateInfo;
# 1964 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUComputePipelineCreateInfo
{
    uint64 code_size;
    const Uint8 *code;
    const char *entrypoint;
    SDL_GPUShaderFormat format;
    Uint32 num_samplers;
    Uint32 num_readonly_storage_textures;
    Uint32 num_readonly_storage_buffers;
    Uint32 num_readwrite_storage_textures;
    Uint32 num_readwrite_storage_buffers;
    Uint32 num_uniform_buffers;
    Uint32 threadcount_x;
    Uint32 threadcount_y;
    Uint32 threadcount_z;

    SDL_PropertiesID props;
} SDL_GPUComputePipelineCreateInfo;
# 2019 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUColorTargetInfo
{
    SDL_GPUTexture *texture;
    Uint32 mip_level;
    Uint32 layer_or_depth_plane;
    SDL_FColor clear_color;
    SDL_GPULoadOp load_op;
    SDL_GPUStoreOp store_op;
    SDL_GPUTexture *resolve_texture;
    Uint32 resolve_mip_level;
    Uint32 resolve_layer;
    bool cycle;
    bool cycle_resolve_texture;
    Uint8 padding1;
    Uint8 padding2;
} SDL_GPUColorTargetInfo;
# 2083 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUDepthStencilTargetInfo
{
    SDL_GPUTexture *texture;
    float clear_depth;
    SDL_GPULoadOp load_op;
    SDL_GPUStoreOp store_op;
    SDL_GPULoadOp stencil_load_op;
    SDL_GPUStoreOp stencil_store_op;
    bool cycle;
    Uint8 clear_stencil;
    Uint8 mip_level;
    Uint8 layer;
} SDL_GPUDepthStencilTargetInfo;
# 2104 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUBlitInfo {
    SDL_GPUBlitRegion source;
    SDL_GPUBlitRegion destination;
    SDL_GPULoadOp load_op;
    SDL_FColor clear_color;
    SDL_FlipMode flip_mode;
    SDL_GPUFilter filter;
    bool cycle;
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
} SDL_GPUBlitInfo;
# 2127 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUBufferBinding
{
    SDL_GPUBuffer *buffer;
    Uint32 offset;
} SDL_GPUBufferBinding;
# 2143 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUTextureSamplerBinding
{
    SDL_GPUTexture *texture;
    SDL_GPUSampler *sampler;
} SDL_GPUTextureSamplerBinding;
# 2157 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUStorageBufferReadWriteBinding
{
    SDL_GPUBuffer *buffer;
    bool cycle;
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
} SDL_GPUStorageBufferReadWriteBinding;
# 2174 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUStorageTextureReadWriteBinding
{
    SDL_GPUTexture *texture;
    Uint32 mip_level;
    Uint32 layer;
    bool cycle;
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
} SDL_GPUStorageTextureReadWriteBinding;
# 2202 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) bool  SDL_GPUSupportsShaderFormats(
    SDL_GPUShaderFormat format_flags,
    const char *name);
# 2216 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) bool  SDL_GPUSupportsProperties(
    SDL_PropertiesID props);
# 2245 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) SDL_GPUDevice *  SDL_CreateGPUDevice(
    SDL_GPUShaderFormat format_flags,
    bool debug_mode,
    const char *name);
# 2362 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) SDL_GPUDevice *  SDL_CreateGPUDeviceWithProperties(
    SDL_PropertiesID props);
# 2418 "./SDL3/SDL_gpu.h"
typedef struct SDL_GPUVulkanOptions
{
    Uint32 vulkan_api_version;
    void *feature_list;
	void *vulkan_10_physical_device_features;
	Uint32 device_extension_count;
	const char **device_extension_names;
	Uint32 instance_extension_count;
	const char **instance_extension_names;
} SDL_GPUVulkanOptions;
# 2438 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_DestroyGPUDevice(SDL_GPUDevice *device);
# 2449 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) int  SDL_GetNumGPUDrivers(void);
# 2468 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) const char *  SDL_GetGPUDriver(int index);
# 2478 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) const char *  SDL_GetGPUDeviceDriver(SDL_GPUDevice *device);
# 2489 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) SDL_GPUShaderFormat  SDL_GetGPUShaderFormats(SDL_GPUDevice *device);
# 2593 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) SDL_PropertiesID  SDL_GetGPUDeviceProperties(SDL_GPUDevice *device);
# 2648 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) SDL_GPUComputePipeline *  SDL_CreateGPUComputePipeline(
    SDL_GPUDevice *device,
    const SDL_GPUComputePipelineCreateInfo *createinfo);
# 2675 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) SDL_GPUGraphicsPipeline *  SDL_CreateGPUGraphicsPipeline(
    SDL_GPUDevice *device,
    const SDL_GPUGraphicsPipelineCreateInfo *createinfo);
# 2702 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) SDL_GPUSampler *  SDL_CreateGPUSampler(
    SDL_GPUDevice *device,
    const SDL_GPUSamplerCreateInfo *createinfo);
# 2781 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) SDL_GPUShader *  SDL_CreateGPUShader(
    SDL_GPUDevice *device,
    const SDL_GPUShaderCreateInfo *createinfo);
# 2845 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) SDL_GPUTexture *  SDL_CreateGPUTexture(
    SDL_GPUDevice *device,
    const SDL_GPUTextureCreateInfo *createinfo);
# 2901 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) SDL_GPUBuffer *  SDL_CreateGPUBuffer(
    SDL_GPUDevice *device,
    const SDL_GPUBufferCreateInfo *createinfo);
# 2934 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) SDL_GPUTransferBuffer *  SDL_CreateGPUTransferBuffer(
    SDL_GPUDevice *device,
    const SDL_GPUTransferBufferCreateInfo *createinfo);
# 2959 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_SetGPUBufferName(
    SDL_GPUDevice *device,
    SDL_GPUBuffer *buffer,
    const char *text);
# 2982 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_SetGPUTextureName(
    SDL_GPUDevice *device,
    SDL_GPUTexture *texture,
    const char *text);
# 3003 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_InsertGPUDebugLabel(
    SDL_GPUCommandBuffer *command_buffer,
    const char *text);
# 3033 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_PushGPUDebugGroup(
    SDL_GPUCommandBuffer *command_buffer,
    const char *name);
# 3051 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_PopGPUDebugGroup(
    SDL_GPUCommandBuffer *command_buffer);
# 3066 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_ReleaseGPUTexture(
    SDL_GPUDevice *device,
    SDL_GPUTexture *texture);
# 3080 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_ReleaseGPUSampler(
    SDL_GPUDevice *device,
    SDL_GPUSampler *sampler);
# 3094 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_ReleaseGPUBuffer(
    SDL_GPUDevice *device,
    SDL_GPUBuffer *buffer);
# 3108 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_ReleaseGPUTransferBuffer(
    SDL_GPUDevice *device,
    SDL_GPUTransferBuffer *transfer_buffer);
# 3122 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_ReleaseGPUComputePipeline(
    SDL_GPUDevice *device,
    SDL_GPUComputePipeline *compute_pipeline);
# 3136 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_ReleaseGPUShader(
    SDL_GPUDevice *device,
    SDL_GPUShader *shader);
# 3150 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_ReleaseGPUGraphicsPipeline(
    SDL_GPUDevice *device,
    SDL_GPUGraphicsPipeline *graphics_pipeline);
# 3178 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) SDL_GPUCommandBuffer *  SDL_AcquireGPUCommandBuffer(
    SDL_GPUDevice *device);
# 3202 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_PushGPUVertexUniformData(
    SDL_GPUCommandBuffer *command_buffer,
    Uint32 slot_index,
    const void *data,
    Uint32 length);
# 3224 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_PushGPUFragmentUniformData(
    SDL_GPUCommandBuffer *command_buffer,
    Uint32 slot_index,
    const void *data,
    Uint32 length);
# 3246 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_PushGPUComputeUniformData(
    SDL_GPUCommandBuffer *command_buffer,
    Uint32 slot_index,
    const void *data,
    Uint32 length);
# 3287 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) SDL_GPURenderPass *  SDL_BeginGPURenderPass(
    SDL_GPUCommandBuffer *command_buffer,
    const SDL_GPUColorTargetInfo *color_target_infos,
    Uint32 num_color_targets,
    const SDL_GPUDepthStencilTargetInfo *depth_stencil_target_info);
# 3303 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_BindGPUGraphicsPipeline(
    SDL_GPURenderPass *render_pass,
    SDL_GPUGraphicsPipeline *graphics_pipeline);
# 3315 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_SetGPUViewport(
    SDL_GPURenderPass *render_pass,
    const SDL_GPUViewport *viewport);
# 3327 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_SetGPUScissor(
    SDL_GPURenderPass *render_pass,
    const SDL_Rect *scissor);
# 3342 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_SetGPUBlendConstants(
    SDL_GPURenderPass *render_pass,
    SDL_FColor blend_constants);
# 3354 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_SetGPUStencilReference(
    SDL_GPURenderPass *render_pass,
    Uint8 reference);
# 3370 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_BindGPUVertexBuffers(
    SDL_GPURenderPass *render_pass,
    Uint32 first_slot,
    const SDL_GPUBufferBinding *bindings,
    Uint32 num_bindings);
# 3387 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_BindGPUIndexBuffer(
    SDL_GPURenderPass *render_pass,
    const SDL_GPUBufferBinding *binding,
    SDL_GPUIndexElementSize index_element_size);
# 3411 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_BindGPUVertexSamplers(
    SDL_GPURenderPass *render_pass,
    Uint32 first_slot,
    const SDL_GPUTextureSamplerBinding *texture_sampler_bindings,
    Uint32 num_bindings);
# 3435 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_BindGPUVertexStorageTextures(
    SDL_GPURenderPass *render_pass,
    Uint32 first_slot,
    SDL_GPUTexture *const *storage_textures,
    Uint32 num_bindings);
# 3459 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_BindGPUVertexStorageBuffers(
    SDL_GPURenderPass *render_pass,
    Uint32 first_slot,
    SDL_GPUBuffer *const *storage_buffers,
    Uint32 num_bindings);
# 3484 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_BindGPUFragmentSamplers(
    SDL_GPURenderPass *render_pass,
    Uint32 first_slot,
    const SDL_GPUTextureSamplerBinding *texture_sampler_bindings,
    Uint32 num_bindings);
# 3508 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_BindGPUFragmentStorageTextures(
    SDL_GPURenderPass *render_pass,
    Uint32 first_slot,
    SDL_GPUTexture *const *storage_textures,
    Uint32 num_bindings);
# 3532 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_BindGPUFragmentStorageBuffers(
    SDL_GPURenderPass *render_pass,
    Uint32 first_slot,
    SDL_GPUBuffer *const *storage_buffers,
    Uint32 num_bindings);
# 3563 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_DrawGPUIndexedPrimitives(
    SDL_GPURenderPass *render_pass,
    Uint32 num_indices,
    Uint32 num_instances,
    Uint32 first_index,
    Sint32 vertex_offset,
    Uint32 first_instance);
# 3591 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_DrawGPUPrimitives(
    SDL_GPURenderPass *render_pass,
    Uint32 num_vertices,
    Uint32 num_instances,
    Uint32 first_vertex,
    Uint32 first_instance);
# 3614 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_DrawGPUPrimitivesIndirect(
    SDL_GPURenderPass *render_pass,
    SDL_GPUBuffer *buffer,
    Uint32 offset,
    Uint32 draw_count);
# 3636 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_DrawGPUIndexedPrimitivesIndirect(
    SDL_GPURenderPass *render_pass,
    SDL_GPUBuffer *buffer,
    Uint32 offset,
    Uint32 draw_count);
# 3652 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_EndGPURenderPass(
    SDL_GPURenderPass *render_pass);
# 3694 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) SDL_GPUComputePass *  SDL_BeginGPUComputePass(
    SDL_GPUCommandBuffer *command_buffer,
    const SDL_GPUStorageTextureReadWriteBinding *storage_texture_bindings,
    Uint32 num_storage_texture_bindings,
    const SDL_GPUStorageBufferReadWriteBinding *storage_buffer_bindings,
    Uint32 num_storage_buffer_bindings);
# 3709 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_BindGPUComputePipeline(
    SDL_GPUComputePass *compute_pass,
    SDL_GPUComputePipeline *compute_pipeline);
# 3732 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_BindGPUComputeSamplers(
    SDL_GPUComputePass *compute_pass,
    Uint32 first_slot,
    const SDL_GPUTextureSamplerBinding *texture_sampler_bindings,
    Uint32 num_bindings);
# 3756 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_BindGPUComputeStorageTextures(
    SDL_GPUComputePass *compute_pass,
    Uint32 first_slot,
    SDL_GPUTexture *const *storage_textures,
    Uint32 num_bindings);
# 3780 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_BindGPUComputeStorageBuffers(
    SDL_GPUComputePass *compute_pass,
    Uint32 first_slot,
    SDL_GPUBuffer *const *storage_buffers,
    Uint32 num_bindings);
# 3806 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_DispatchGPUCompute(
    SDL_GPUComputePass *compute_pass,
    Uint32 groupcount_x,
    Uint32 groupcount_y,
    Uint32 groupcount_z);
# 3830 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_DispatchGPUComputeIndirect(
    SDL_GPUComputePass *compute_pass,
    SDL_GPUBuffer *buffer,
    Uint32 offset);
# 3845 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_EndGPUComputePass(
    SDL_GPUComputePass *compute_pass);
# 3865 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void *  SDL_MapGPUTransferBuffer(
    SDL_GPUDevice *device,
    SDL_GPUTransferBuffer *transfer_buffer,
    bool cycle);
# 3878 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_UnmapGPUTransferBuffer(
    SDL_GPUDevice *device,
    SDL_GPUTransferBuffer *transfer_buffer);
# 3898 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) SDL_GPUCopyPass *  SDL_BeginGPUCopyPass(
    SDL_GPUCommandBuffer *command_buffer);
# 3918 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_UploadToGPUTexture(
    SDL_GPUCopyPass *copy_pass,
    const SDL_GPUTextureTransferInfo *source,
    const SDL_GPUTextureRegion *destination,
    bool cycle);
# 3938 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_UploadToGPUBuffer(
    SDL_GPUCopyPass *copy_pass,
    const SDL_GPUTransferBufferLocation *source,
    const SDL_GPUBufferRegion *destination,
    bool cycle);
# 3965 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_CopyGPUTextureToTexture(
    SDL_GPUCopyPass *copy_pass,
    const SDL_GPUTextureLocation *source,
    const SDL_GPUTextureLocation *destination,
    Uint32 w,
    Uint32 h,
    Uint32 d,
    bool cycle);
# 3989 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_CopyGPUBufferToBuffer(
    SDL_GPUCopyPass *copy_pass,
    const SDL_GPUBufferLocation *source,
    const SDL_GPUBufferLocation *destination,
    Uint32 size,
    bool cycle);
# 4009 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_DownloadFromGPUTexture(
    SDL_GPUCopyPass *copy_pass,
    const SDL_GPUTextureRegion *source,
    const SDL_GPUTextureTransferInfo *destination);
# 4026 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_DownloadFromGPUBuffer(
    SDL_GPUCopyPass *copy_pass,
    const SDL_GPUBufferRegion *source,
    const SDL_GPUTransferBufferLocation *destination);
# 4038 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_EndGPUCopyPass(
    SDL_GPUCopyPass *copy_pass);
# 4051 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_GenerateMipmapsForGPUTexture(
    SDL_GPUCommandBuffer *command_buffer,
    SDL_GPUTexture *texture);
# 4065 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_BlitGPUTexture(
    SDL_GPUCommandBuffer *command_buffer,
    const SDL_GPUBlitInfo *info);
# 4085 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) bool  SDL_WindowSupportsGPUSwapchainComposition(
    SDL_GPUDevice *device,
    SDL_Window *window,
    SDL_GPUSwapchainComposition swapchain_composition);
# 4104 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) bool  SDL_WindowSupportsGPUPresentMode(
    SDL_GPUDevice *device,
    SDL_Window *window,
    SDL_GPUPresentMode present_mode);
# 4136 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) bool  SDL_ClaimWindowForGPUDevice(
    SDL_GPUDevice *device,
    SDL_Window *window);
# 4150 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_ReleaseWindowFromGPUDevice(
    SDL_GPUDevice *device,
    SDL_Window *window);
# 4177 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) bool  SDL_SetGPUSwapchainParameters(
    SDL_GPUDevice *device,
    SDL_Window *window,
    SDL_GPUSwapchainComposition swapchain_composition,
    SDL_GPUPresentMode present_mode);
# 4208 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) bool  SDL_SetGPUAllowedFramesInFlight(
    SDL_GPUDevice *device,
    Uint32 allowed_frames_in_flight);
# 4223 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) SDL_GPUTextureFormat  SDL_GetGPUSwapchainTextureFormat(
    SDL_GPUDevice *device,
    SDL_Window *window);
# 4278 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) bool  SDL_AcquireGPUSwapchainTexture(
    SDL_GPUCommandBuffer *command_buffer,
    SDL_Window *window,
    SDL_GPUTexture **swapchain_texture,
    Uint32 *swapchain_texture_width,
    Uint32 *swapchain_texture_height);
# 4303 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) bool  SDL_WaitForGPUSwapchain(
    SDL_GPUDevice *device,
    SDL_Window *window);
# 4349 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) bool  SDL_WaitAndAcquireGPUSwapchainTexture(
    SDL_GPUCommandBuffer *command_buffer,
    SDL_Window *window,
    SDL_GPUTexture **swapchain_texture,
    Uint32 *swapchain_texture_width,
    Uint32 *swapchain_texture_height);
# 4377 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) bool  SDL_SubmitGPUCommandBuffer(
    SDL_GPUCommandBuffer *command_buffer);
# 4404 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) SDL_GPUFence *  SDL_SubmitGPUCommandBufferAndAcquireFence(
    SDL_GPUCommandBuffer *command_buffer);
# 4429 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) bool  SDL_CancelGPUCommandBuffer(
    SDL_GPUCommandBuffer *command_buffer);
# 4443 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) bool  SDL_WaitForGPUIdle(
    SDL_GPUDevice *device);
# 4462 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) bool  SDL_WaitForGPUFences(
    SDL_GPUDevice *device,
    bool wait_all,
    SDL_GPUFence *const *fences,
    Uint32 num_fences);
# 4479 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) bool  SDL_QueryGPUFence(
    SDL_GPUDevice *device,
    SDL_GPUFence *fence);
# 4495 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) void  SDL_ReleaseGPUFence(
    SDL_GPUDevice *device,
    SDL_GPUFence *fence);
# 4511 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) Uint32  SDL_GPUTextureFormatTexelBlockSize(
    SDL_GPUTextureFormat format);
# 4526 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) bool  SDL_GPUTextureSupportsFormat(
    SDL_GPUDevice *device,
    SDL_GPUTextureFormat format,
    SDL_GPUTextureType type,
    SDL_GPUTextureUsageFlags usage);
# 4542 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) bool  SDL_GPUTextureSupportsSampleCount(
    SDL_GPUDevice *device,
    SDL_GPUTextureFormat format,
    SDL_GPUSampleCount sample_count);
# 4558 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) Uint32  SDL_CalculateGPUTextureFormatSize(
    SDL_GPUTextureFormat format,
    Uint32 width,
    Uint32 height,
    Uint32 depth_or_layer_count);
# 4573 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) SDL_PixelFormat  SDL_GetPixelFormatFromGPUTextureFormat(SDL_GPUTextureFormat format);
# 4585 "./SDL3/SDL_gpu.h"
extern __attribute__((dllimport)) SDL_GPUTextureFormat  SDL_GetGPUTextureFormatFromPixelFormat(SDL_PixelFormat format);
# 4629 "./SDL3/SDL_gpu.h"
# 1 "./SDL3/SDL_close_code.h" 1
# 4629 "./SDL3/SDL_gpu.h" 2
# 52 "SDL3/SDL.h" 2


# 1 "./SDL3/SDL_haptic.h" 1
# 125 "./SDL3/SDL_haptic.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 125 "./SDL3/SDL_haptic.h" 2
# 150 "./SDL3/SDL_haptic.h"
typedef struct SDL_Haptic SDL_Haptic;
# 181 "./SDL3/SDL_haptic.h"
typedef Uint16 SDL_HapticEffectType;
# 407 "./SDL3/SDL_haptic.h"
typedef Uint8 SDL_HapticDirectionType;
# 460 "./SDL3/SDL_haptic.h"
typedef int SDL_HapticEffectID;
# 566 "./SDL3/SDL_haptic.h"
typedef struct SDL_HapticDirection
{
    SDL_HapticDirectionType type;
    Sint32 dir[3];
} SDL_HapticDirection;
# 586 "./SDL3/SDL_haptic.h"
typedef struct SDL_HapticConstant
{

    SDL_HapticEffectType type;
    SDL_HapticDirection direction;


    Uint32 length;
    Uint16 delay;


    Uint16 button;
    Uint16 interval;


    Sint16 level;


    Uint16 attack_length;
    Uint16 attack_level;
    Uint16 fade_length;
    Uint16 fade_level;
} SDL_HapticConstant;
# 672 "./SDL3/SDL_haptic.h"
typedef struct SDL_HapticPeriodic
{

    SDL_HapticEffectType type;


    SDL_HapticDirection direction;


    Uint32 length;
    Uint16 delay;


    Uint16 button;
    Uint16 interval;


    Uint16 period;
    Sint16 magnitude;
    Sint16 offset;
    Uint16 phase;


    Uint16 attack_length;
    Uint16 attack_level;
    Uint16 fade_length;
    Uint16 fade_level;
} SDL_HapticPeriodic;
# 728 "./SDL3/SDL_haptic.h"
typedef struct SDL_HapticCondition
{

    SDL_HapticEffectType type;

    SDL_HapticDirection direction;


    Uint32 length;
    Uint16 delay;


    Uint16 button;
    Uint16 interval;


    Uint16 right_sat[3];
    Uint16 left_sat[3];
    Sint16 right_coeff[3];
    Sint16 left_coeff[3];
    Uint16 deadband[3];
    Sint16 center[3];
} SDL_HapticCondition;
# 767 "./SDL3/SDL_haptic.h"
typedef struct SDL_HapticRamp
{

    SDL_HapticEffectType type;
    SDL_HapticDirection direction;


    Uint32 length;
    Uint16 delay;


    Uint16 button;
    Uint16 interval;


    Sint16 start;
    Sint16 end;


    Uint16 attack_length;
    Uint16 attack_level;
    Uint16 fade_length;
    Uint16 fade_level;
} SDL_HapticRamp;
# 806 "./SDL3/SDL_haptic.h"
typedef struct SDL_HapticLeftRight
{

    SDL_HapticEffectType type;


    Uint32 length;


    Uint16 large_magnitude;
    Uint16 small_magnitude;
} SDL_HapticLeftRight;
# 836 "./SDL3/SDL_haptic.h"
typedef struct SDL_HapticCustom
{

    SDL_HapticEffectType type;
    SDL_HapticDirection direction;


    Uint32 length;
    Uint16 delay;


    Uint16 button;
    Uint16 interval;


    Uint8 channels;
    Uint16 period;
    Uint16 samples;
    Uint16 *data;


    Uint16 attack_length;
    Uint16 attack_level;
    Uint16 fade_length;
    Uint16 fade_level;
} SDL_HapticCustom;
# 935 "./SDL3/SDL_haptic.h"
typedef union SDL_HapticEffect
{

    SDL_HapticEffectType type;
    SDL_HapticConstant constant;
    SDL_HapticPeriodic periodic;
    SDL_HapticCondition condition;
    SDL_HapticRamp ramp;
    SDL_HapticLeftRight leftright;
    SDL_HapticCustom custom;
} SDL_HapticEffect;
# 957 "./SDL3/SDL_haptic.h"
typedef Uint32 SDL_HapticID;
# 975 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) SDL_HapticID *  SDL_GetHaptics(int *count);
# 992 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) const char *  SDL_GetHapticNameForID(SDL_HapticID instance_id);
# 1017 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) SDL_Haptic *  SDL_OpenHaptic(SDL_HapticID instance_id);
# 1029 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) SDL_Haptic *  SDL_GetHapticFromID(SDL_HapticID instance_id);
# 1040 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) SDL_HapticID  SDL_GetHapticID(SDL_Haptic *haptic);
# 1054 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) const char *  SDL_GetHapticName(SDL_Haptic *haptic);
# 1065 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) bool  SDL_IsMouseHaptic(void);
# 1078 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) SDL_Haptic *  SDL_OpenHapticFromMouse(void);
# 1090 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) bool  SDL_IsJoystickHaptic(SDL_Joystick *joystick);
# 1112 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) SDL_Haptic *  SDL_OpenHapticFromJoystick(SDL_Joystick *joystick);
# 1123 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) void  SDL_CloseHaptic(SDL_Haptic *haptic);
# 1141 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) int  SDL_GetMaxHapticEffects(SDL_Haptic *haptic);
# 1157 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) int  SDL_GetMaxHapticEffectsPlaying(SDL_Haptic *haptic);
# 1171 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) Uint32  SDL_GetHapticFeatures(SDL_Haptic *haptic);
# 1185 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) int  SDL_GetNumHapticAxes(SDL_Haptic *haptic);
# 1199 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) bool  SDL_HapticEffectSupported(SDL_Haptic *haptic, const SDL_HapticEffect *effect);
# 1216 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) SDL_HapticEffectID  SDL_CreateHapticEffect(SDL_Haptic *haptic, const SDL_HapticEffect *effect);
# 1238 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) bool  SDL_UpdateHapticEffect(SDL_Haptic *haptic, SDL_HapticEffectID effect, const SDL_HapticEffect *data);
# 1262 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) bool  SDL_RunHapticEffect(SDL_Haptic *haptic, SDL_HapticEffectID effect, Uint32 iterations);
# 1277 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) bool  SDL_StopHapticEffect(SDL_Haptic *haptic, SDL_HapticEffectID effect);
# 1292 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) void  SDL_DestroyHapticEffect(SDL_Haptic *haptic, SDL_HapticEffectID effect);
# 1308 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) bool  SDL_GetHapticEffectStatus(SDL_Haptic *haptic, SDL_HapticEffectID effect);
# 1330 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) bool  SDL_SetHapticGain(SDL_Haptic *haptic, int gain);
# 1349 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) bool  SDL_SetHapticAutocenter(SDL_Haptic *haptic, int autocenter);
# 1368 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) bool  SDL_PauseHaptic(SDL_Haptic *haptic);
# 1383 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) bool  SDL_ResumeHaptic(SDL_Haptic *haptic);
# 1397 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) bool  SDL_StopHapticEffects(SDL_Haptic *haptic);
# 1409 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) bool  SDL_HapticRumbleSupported(SDL_Haptic *haptic);
# 1424 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) bool  SDL_InitHapticRumble(SDL_Haptic *haptic);
# 1440 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) bool  SDL_PlayHapticRumble(SDL_Haptic *haptic, float strength, Uint32 length);
# 1453 "./SDL3/SDL_haptic.h"
extern __attribute__((dllimport)) bool  SDL_StopHapticRumble(SDL_Haptic *haptic);






# 1 "./SDL3/SDL_close_code.h" 1
# 1460 "./SDL3/SDL_haptic.h" 2
# 54 "SDL3/SDL.h" 2

# 1 "./SDL3/SDL_hidapi.h" 1
# 61 "./SDL3/SDL_hidapi.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 61 "./SDL3/SDL_hidapi.h" 2
# 71 "./SDL3/SDL_hidapi.h"
typedef struct SDL_hid_device SDL_hid_device;






typedef enum SDL_hid_bus_type {

    SDL_HID_API_BUS_UNKNOWN = 0x00,




    SDL_HID_API_BUS_USB = 0x01,






    SDL_HID_API_BUS_BLUETOOTH = 0x02,




    SDL_HID_API_BUS_I2C = 0x03,




    SDL_HID_API_BUS_SPI = 0x04

} SDL_hid_bus_type;
# 113 "./SDL3/SDL_hidapi.h"
typedef struct SDL_hid_device_info
{

    char *path;

    unsigned short vendor_id;

    unsigned short product_id;

    int16 *serial_number;


    unsigned short release_number;

    int16 *manufacturer_string;

    int16 *product_string;


    unsigned short usage_page;


    unsigned short usage;






    int interface_number;



    int interface_class;
    int interface_subclass;
    int interface_protocol;


    SDL_hid_bus_type bus_type;


    struct SDL_hid_device_info *next;

} SDL_hid_device_info;
# 177 "./SDL3/SDL_hidapi.h"
extern __attribute__((dllimport)) int  SDL_hid_init(void);
# 192 "./SDL3/SDL_hidapi.h"
extern __attribute__((dllimport)) int  SDL_hid_exit(void);
# 213 "./SDL3/SDL_hidapi.h"
extern __attribute__((dllimport)) Uint32  SDL_hid_device_change_count(void);
# 241 "./SDL3/SDL_hidapi.h"
extern __attribute__((dllimport)) SDL_hid_device_info *  SDL_hid_enumerate(unsigned short vendor_id, unsigned short product_id);
# 253 "./SDL3/SDL_hidapi.h"
extern __attribute__((dllimport)) void  SDL_hid_free_enumeration(SDL_hid_device_info *devs);
# 271 "./SDL3/SDL_hidapi.h"
extern __attribute__((dllimport)) SDL_hid_device *  SDL_hid_open(unsigned short vendor_id, unsigned short product_id, const int16 *serial_number);
# 285 "./SDL3/SDL_hidapi.h"
extern __attribute__((dllimport)) SDL_hid_device *  SDL_hid_open_path(const char *path);
# 301 "./SDL3/SDL_hidapi.h"
extern __attribute__((dllimport)) SDL_PropertiesID  SDL_hid_get_properties(SDL_hid_device *dev);
# 330 "./SDL3/SDL_hidapi.h"
extern __attribute__((dllimport)) int  SDL_hid_write(SDL_hid_device *dev, const unsigned char *data, uint64 length);
# 351 "./SDL3/SDL_hidapi.h"
extern __attribute__((dllimport)) int  SDL_hid_read_timeout(SDL_hid_device *dev, unsigned char *data, uint64 length, int milliseconds);
# 372 "./SDL3/SDL_hidapi.h"
extern __attribute__((dllimport)) int  SDL_hid_read(SDL_hid_device *dev, unsigned char *data, uint64 length);
# 391 "./SDL3/SDL_hidapi.h"
extern __attribute__((dllimport)) int  SDL_hid_set_nonblocking(SDL_hid_device *dev, int nonblock);
# 416 "./SDL3/SDL_hidapi.h"
extern __attribute__((dllimport)) int  SDL_hid_send_feature_report(SDL_hid_device *dev, const unsigned char *data, uint64 length);
# 439 "./SDL3/SDL_hidapi.h"
extern __attribute__((dllimport)) int  SDL_hid_get_feature_report(SDL_hid_device *dev, unsigned char *data, uint64 length);
# 462 "./SDL3/SDL_hidapi.h"
extern __attribute__((dllimport)) int  SDL_hid_get_input_report(SDL_hid_device *dev, unsigned char *data, uint64 length);
# 473 "./SDL3/SDL_hidapi.h"
extern __attribute__((dllimport)) int  SDL_hid_close(SDL_hid_device *dev);
# 486 "./SDL3/SDL_hidapi.h"
extern __attribute__((dllimport)) int  SDL_hid_get_manufacturer_string(SDL_hid_device *dev, int16 *string, uint64 maxlen);
# 499 "./SDL3/SDL_hidapi.h"
extern __attribute__((dllimport)) int  SDL_hid_get_product_string(SDL_hid_device *dev, int16 *string, uint64 maxlen);
# 512 "./SDL3/SDL_hidapi.h"
extern __attribute__((dllimport)) int  SDL_hid_get_serial_number_string(SDL_hid_device *dev, int16 *string, uint64 maxlen);
# 526 "./SDL3/SDL_hidapi.h"
extern __attribute__((dllimport)) int  SDL_hid_get_indexed_string(SDL_hid_device *dev, int string_index, int16 *string, uint64 maxlen);
# 538 "./SDL3/SDL_hidapi.h"
extern __attribute__((dllimport)) SDL_hid_device_info *  SDL_hid_get_device_info(SDL_hid_device *dev);
# 554 "./SDL3/SDL_hidapi.h"
extern __attribute__((dllimport)) int  SDL_hid_get_report_descriptor(SDL_hid_device *dev, unsigned char *buf, uint64 buf_size);
# 563 "./SDL3/SDL_hidapi.h"
extern __attribute__((dllimport)) void  SDL_hid_ble_scan(bool active);






# 1 "./SDL3/SDL_close_code.h" 1
# 570 "./SDL3/SDL_hidapi.h" 2
# 55 "SDL3/SDL.h" 2

# 1 "./SDL3/SDL_hints.h" 1
# 44 "./SDL3/SDL_hints.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 44 "./SDL3/SDL_hints.h" 2
# 4921 "./SDL3/SDL_hints.h"
typedef enum SDL_HintPriority
{
    SDL_HINT_DEFAULT,
    SDL_HINT_NORMAL,
    SDL_HINT_OVERRIDE
} SDL_HintPriority;
# 4949 "./SDL3/SDL_hints.h"
extern __attribute__((dllimport)) bool  SDL_SetHintWithPriority(const char *name, const char *value, SDL_HintPriority priority);
# 4971 "./SDL3/SDL_hints.h"
extern __attribute__((dllimport)) bool  SDL_SetHint(const char *name, const char *value);
# 4991 "./SDL3/SDL_hints.h"
extern __attribute__((dllimport)) bool  SDL_ResetHint(const char *name);
# 5006 "./SDL3/SDL_hints.h"
extern __attribute__((dllimport)) void  SDL_ResetHints(void);
# 5021 "./SDL3/SDL_hints.h"
extern __attribute__((dllimport)) const char * SDL_GetHint(const char *name);
# 5038 "./SDL3/SDL_hints.h"
extern __attribute__((dllimport)) bool  SDL_GetHintBoolean(const char *name, bool default_value);
# 5060 "./SDL3/SDL_hints.h"
typedef void( *SDL_HintCallback)(void *userdata, const char *name, const char *oldValue, const char *newValue);
# 5081 "./SDL3/SDL_hints.h"
extern __attribute__((dllimport)) bool  SDL_AddHintCallback(const char *name, SDL_HintCallback callback, void *userdata);
# 5097 "./SDL3/SDL_hints.h"
extern __attribute__((dllimport)) void  SDL_RemoveHintCallback(const char *name,
                                                        SDL_HintCallback callback,
                                                        void *userdata);






# 1 "./SDL3/SDL_close_code.h" 1
# 5106 "./SDL3/SDL_hints.h" 2
# 56 "SDL3/SDL.h" 2

# 1 "./SDL3/SDL_init.h" 1
# 57 "./SDL3/SDL_init.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 57 "./SDL3/SDL_init.h" 2
# 78 "./SDL3/SDL_init.h"
typedef Uint32 SDL_InitFlags;
# 109 "./SDL3/SDL_init.h"
typedef enum SDL_AppResult
{
    SDL_APP_CONTINUE,
    SDL_APP_SUCCESS,
    SDL_APP_FAILURE
} SDL_AppResult;
# 133 "./SDL3/SDL_init.h"
typedef SDL_AppResult ( *SDL_AppInit_func)(void **appstate, int argc, char *argv[]);
# 148 "./SDL3/SDL_init.h"
typedef SDL_AppResult ( *SDL_AppIterate_func)(void *appstate);
# 164 "./SDL3/SDL_init.h"
typedef SDL_AppResult ( *SDL_AppEvent_func)(void *appstate, SDL_Event *event);
# 178 "./SDL3/SDL_init.h"
typedef void ( *SDL_AppQuit_func)(void *appstate, SDL_AppResult result);
# 238 "./SDL3/SDL_init.h"
extern __attribute__((dllimport)) bool  SDL_Init(SDL_InitFlags flags);
# 257 "./SDL3/SDL_init.h"
extern __attribute__((dllimport)) bool  SDL_InitSubSystem(SDL_InitFlags flags);
# 274 "./SDL3/SDL_init.h"
extern __attribute__((dllimport)) void  SDL_QuitSubSystem(SDL_InitFlags flags);
# 290 "./SDL3/SDL_init.h"
extern __attribute__((dllimport)) SDL_InitFlags  SDL_WasInit(SDL_InitFlags flags);
# 310 "./SDL3/SDL_init.h"
extern __attribute__((dllimport)) void  SDL_Quit(void);
# 330 "./SDL3/SDL_init.h"
extern __attribute__((dllimport)) bool  SDL_IsMainThread(void);
# 341 "./SDL3/SDL_init.h"
typedef void ( *SDL_MainThreadCallback)(void *userdata);
# 367 "./SDL3/SDL_init.h"
extern __attribute__((dllimport)) bool  SDL_RunOnMainThread(SDL_MainThreadCallback callback, void *userdata, bool wait_complete);
# 405 "./SDL3/SDL_init.h"
extern __attribute__((dllimport)) bool  SDL_SetAppMetadata(const char *appname, const char *appversion, const char *appidentifier);
# 468 "./SDL3/SDL_init.h"
extern __attribute__((dllimport)) bool  SDL_SetAppMetadataProperty(const char *name, const char *value);
# 499 "./SDL3/SDL_init.h"
extern __attribute__((dllimport)) const char *  SDL_GetAppMetadataProperty(const char *name);






# 1 "./SDL3/SDL_close_code.h" 1
# 506 "./SDL3/SDL_init.h" 2
# 57 "SDL3/SDL.h" 2





# 1 "./SDL3/SDL_loadso.h" 1
# 63 "./SDL3/SDL_loadso.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 63 "./SDL3/SDL_loadso.h" 2
# 77 "./SDL3/SDL_loadso.h"
typedef struct SDL_SharedObject SDL_SharedObject;
# 93 "./SDL3/SDL_loadso.h"
extern __attribute__((dllimport)) SDL_SharedObject *  SDL_LoadObject(const char *sofile);
# 121 "./SDL3/SDL_loadso.h"
extern __attribute__((dllimport)) SDL_FunctionPointer  SDL_LoadFunction(SDL_SharedObject *handle, const char *name);
# 137 "./SDL3/SDL_loadso.h"
extern __attribute__((dllimport)) void  SDL_UnloadObject(SDL_SharedObject *handle);






# 1 "./SDL3/SDL_close_code.h" 1
# 144 "./SDL3/SDL_loadso.h" 2
# 62 "SDL3/SDL.h" 2

# 1 "./SDL3/SDL_locale.h" 1
# 41 "./SDL3/SDL_locale.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 41 "./SDL3/SDL_locale.h" 2
# 60 "./SDL3/SDL_locale.h"
typedef struct SDL_Locale
{
    const char *language;
    const char *country;
} SDL_Locale;
# 109 "./SDL3/SDL_locale.h"
extern __attribute__((dllimport)) SDL_Locale **  SDL_GetPreferredLocales(int *count);
# 118 "./SDL3/SDL_locale.h"
# 1 "./SDL3/SDL_close_code.h" 1
# 118 "./SDL3/SDL_locale.h" 2
# 63 "SDL3/SDL.h" 2

# 1 "./SDL3/SDL_log.h" 1
# 76 "./SDL3/SDL_log.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 76 "./SDL3/SDL_log.h" 2
# 90 "./SDL3/SDL_log.h"
typedef enum SDL_LogCategory
{
    SDL_LOG_CATEGORY_APPLICATION,
    SDL_LOG_CATEGORY_ERROR,
    SDL_LOG_CATEGORY_ASSERT,
    SDL_LOG_CATEGORY_SYSTEM,
    SDL_LOG_CATEGORY_AUDIO,
    SDL_LOG_CATEGORY_VIDEO,
    SDL_LOG_CATEGORY_RENDER,
    SDL_LOG_CATEGORY_INPUT,
    SDL_LOG_CATEGORY_TEST,
    SDL_LOG_CATEGORY_GPU,


    SDL_LOG_CATEGORY_RESERVED2,
    SDL_LOG_CATEGORY_RESERVED3,
    SDL_LOG_CATEGORY_RESERVED4,
    SDL_LOG_CATEGORY_RESERVED5,
    SDL_LOG_CATEGORY_RESERVED6,
    SDL_LOG_CATEGORY_RESERVED7,
    SDL_LOG_CATEGORY_RESERVED8,
    SDL_LOG_CATEGORY_RESERVED9,
    SDL_LOG_CATEGORY_RESERVED10,
# 122 "./SDL3/SDL_log.h"
    SDL_LOG_CATEGORY_CUSTOM
} SDL_LogCategory;






typedef enum SDL_LogPriority
{
    SDL_LOG_PRIORITY_INVALID,
    SDL_LOG_PRIORITY_TRACE,
    SDL_LOG_PRIORITY_VERBOSE,
    SDL_LOG_PRIORITY_DEBUG,
    SDL_LOG_PRIORITY_INFO,
    SDL_LOG_PRIORITY_WARN,
    SDL_LOG_PRIORITY_ERROR,
    SDL_LOG_PRIORITY_CRITICAL,
    SDL_LOG_PRIORITY_COUNT
} SDL_LogPriority;
# 156 "./SDL3/SDL_log.h"
extern __attribute__((dllimport)) void  SDL_SetLogPriorities(SDL_LogPriority priority);
# 172 "./SDL3/SDL_log.h"
extern __attribute__((dllimport)) void  SDL_SetLogPriority(int category, SDL_LogPriority priority);
# 186 "./SDL3/SDL_log.h"
extern __attribute__((dllimport)) SDL_LogPriority  SDL_GetLogPriority(int category);
# 200 "./SDL3/SDL_log.h"
extern __attribute__((dllimport)) void  SDL_ResetLogPriorities(void);
# 225 "./SDL3/SDL_log.h"
extern __attribute__((dllimport)) bool  SDL_SetLogPriorityPrefix(SDL_LogPriority priority, const char *prefix);
# 248 "./SDL3/SDL_log.h"
extern __attribute__((dllimport)) void  SDL_Log( const char *fmt, ...) ;
# 272 "./SDL3/SDL_log.h"
extern __attribute__((dllimport)) void  SDL_LogTrace(int category,  const char *fmt, ...) ;
# 295 "./SDL3/SDL_log.h"
extern __attribute__((dllimport)) void  SDL_LogVerbose(int category,  const char *fmt, ...) ;
# 319 "./SDL3/SDL_log.h"
extern __attribute__((dllimport)) void  SDL_LogDebug(int category,  const char *fmt, ...) ;
# 343 "./SDL3/SDL_log.h"
extern __attribute__((dllimport)) void  SDL_LogInfo(int category,  const char *fmt, ...) ;
# 367 "./SDL3/SDL_log.h"
extern __attribute__((dllimport)) void  SDL_LogWarn(int category,  const char *fmt, ...) ;
# 391 "./SDL3/SDL_log.h"
extern __attribute__((dllimport)) void  SDL_LogError(int category,  const char *fmt, ...) ;
# 415 "./SDL3/SDL_log.h"
extern __attribute__((dllimport)) void  SDL_LogCritical(int category,  const char *fmt, ...) ;
# 440 "./SDL3/SDL_log.h"
extern __attribute__((dllimport)) void  SDL_LogMessage(int category,
                                            SDL_LogPriority priority,
                                             const char *fmt, ...) ;
# 466 "./SDL3/SDL_log.h"
extern __attribute__((dllimport)) void  SDL_LogMessageV(int category,
                                             SDL_LogPriority priority,
                                              const char *fmt, byte* ap) ;
# 485 "./SDL3/SDL_log.h"
typedef void ( *SDL_LogOutputFunction)(void *userdata, int category, SDL_LogPriority priority, const char *message);
# 500 "./SDL3/SDL_log.h"
extern __attribute__((dllimport)) SDL_LogOutputFunction  SDL_GetDefaultLogOutputFunction(void);
# 517 "./SDL3/SDL_log.h"
extern __attribute__((dllimport)) void  SDL_GetLogOutputFunction(SDL_LogOutputFunction *callback, void **userdata);
# 532 "./SDL3/SDL_log.h"
extern __attribute__((dllimport)) void  SDL_SetLogOutputFunction(SDL_LogOutputFunction callback, void *userdata);







# 1 "./SDL3/SDL_close_code.h" 1
# 540 "./SDL3/SDL_log.h" 2
# 64 "SDL3/SDL.h" 2

# 1 "./SDL3/SDL_messagebox.h" 1
# 47 "./SDL3/SDL_messagebox.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 47 "./SDL3/SDL_messagebox.h" 2
# 59 "./SDL3/SDL_messagebox.h"
typedef Uint32 SDL_MessageBoxFlags;
# 72 "./SDL3/SDL_messagebox.h"
typedef Uint32 SDL_MessageBoxButtonFlags;
# 82 "./SDL3/SDL_messagebox.h"
typedef struct SDL_MessageBoxButtonData
{
    SDL_MessageBoxButtonFlags flags;
    int buttonID;
    const char *text;
} SDL_MessageBoxButtonData;






typedef struct SDL_MessageBoxColor
{
    Uint8 r, g, b;
} SDL_MessageBoxColor;





typedef enum SDL_MessageBoxColorType
{
    SDL_MESSAGEBOX_COLOR_BACKGROUND,
    SDL_MESSAGEBOX_COLOR_TEXT,
    SDL_MESSAGEBOX_COLOR_BUTTON_BORDER,
    SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND,
    SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED,
    SDL_MESSAGEBOX_COLOR_COUNT
} SDL_MessageBoxColorType;






typedef struct SDL_MessageBoxColorScheme
{
    SDL_MessageBoxColor colors[SDL_MESSAGEBOX_COLOR_COUNT];
} SDL_MessageBoxColorScheme;






typedef struct SDL_MessageBoxData
{
    SDL_MessageBoxFlags flags;
    SDL_Window *window;
    const char *title;
    const char *message;

    int numbuttons;
    const SDL_MessageBoxButtonData *buttons;

    const SDL_MessageBoxColorScheme *colorScheme;
} SDL_MessageBoxData;
# 177 "./SDL3/SDL_messagebox.h"
extern __attribute__((dllimport)) bool  SDL_ShowMessageBox(const SDL_MessageBoxData *messageboxdata, int *buttonid);
# 221 "./SDL3/SDL_messagebox.h"
extern __attribute__((dllimport)) bool  SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags flags, const char *title, const char *message, SDL_Window *window);







# 1 "./SDL3/SDL_close_code.h" 1
# 229 "./SDL3/SDL_messagebox.h" 2
# 65 "SDL3/SDL.h" 2

# 1 "./SDL3/SDL_metal.h" 1
# 38 "./SDL3/SDL_metal.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 38 "./SDL3/SDL_metal.h" 2
# 48 "./SDL3/SDL_metal.h"
typedef void *SDL_MetalView;
# 75 "./SDL3/SDL_metal.h"
extern __attribute__((dllimport)) SDL_MetalView  SDL_Metal_CreateView(SDL_Window *window);
# 91 "./SDL3/SDL_metal.h"
extern __attribute__((dllimport)) void  SDL_Metal_DestroyView(SDL_MetalView view);
# 103 "./SDL3/SDL_metal.h"
extern __attribute__((dllimport)) void *  SDL_Metal_GetLayer(SDL_MetalView view);
# 112 "./SDL3/SDL_metal.h"
# 1 "./SDL3/SDL_close_code.h" 1
# 112 "./SDL3/SDL_metal.h" 2
# 66 "SDL3/SDL.h" 2

# 1 "./SDL3/SDL_misc.h" 1
# 35 "./SDL3/SDL_misc.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 35 "./SDL3/SDL_misc.h" 2
# 72 "./SDL3/SDL_misc.h"
extern __attribute__((dllimport)) bool  SDL_OpenURL(const char *url);






# 1 "./SDL3/SDL_close_code.h" 1
# 79 "./SDL3/SDL_misc.h" 2
# 67 "SDL3/SDL.h" 2





# 1 "./SDL3/SDL_platform.h" 1
# 35 "./SDL3/SDL_platform.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 35 "./SDL3/SDL_platform.h" 2
# 58 "./SDL3/SDL_platform.h"
extern __attribute__((dllimport)) const char *  SDL_GetPlatform(void);






# 1 "./SDL3/SDL_close_code.h" 1
# 65 "./SDL3/SDL_platform.h" 2
# 72 "SDL3/SDL.h" 2


# 1 "./SDL3/SDL_process.h" 1
# 52 "./SDL3/SDL_process.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 52 "./SDL3/SDL_process.h" 2
# 64 "./SDL3/SDL_process.h"
typedef struct SDL_Process SDL_Process;
# 106 "./SDL3/SDL_process.h"
extern __attribute__((dllimport)) SDL_Process *  SDL_CreateProcess(const char * const *args, bool pipe_stdio);
# 150 "./SDL3/SDL_process.h"
typedef enum SDL_ProcessIO
{
    SDL_PROCESS_STDIO_INHERITED,
    SDL_PROCESS_STDIO_NULL,
    SDL_PROCESS_STDIO_APP,
    SDL_PROCESS_STDIO_REDIRECT
} SDL_ProcessIO;
# 227 "./SDL3/SDL_process.h"
extern __attribute__((dllimport)) SDL_Process *  SDL_CreateProcessWithProperties(SDL_PropertiesID props);
# 271 "./SDL3/SDL_process.h"
extern __attribute__((dllimport)) SDL_PropertiesID  SDL_GetProcessProperties(SDL_Process *process);
# 308 "./SDL3/SDL_process.h"
extern __attribute__((dllimport)) void *  SDL_ReadProcess(SDL_Process *process, uint64 *datasize, int *exitcode);
# 334 "./SDL3/SDL_process.h"
extern __attribute__((dllimport)) SDL_IOStream *  SDL_GetProcessInput(SDL_Process *process);
# 358 "./SDL3/SDL_process.h"
extern __attribute__((dllimport)) SDL_IOStream *  SDL_GetProcessOutput(SDL_Process *process);
# 381 "./SDL3/SDL_process.h"
extern __attribute__((dllimport)) bool  SDL_KillProcess(SDL_Process *process, bool force);
# 414 "./SDL3/SDL_process.h"
extern __attribute__((dllimport)) bool  SDL_WaitProcess(SDL_Process *process, bool block, int *exitcode);
# 433 "./SDL3/SDL_process.h"
extern __attribute__((dllimport)) void  SDL_DestroyProcess(SDL_Process *process);






# 1 "./SDL3/SDL_close_code.h" 1
# 440 "./SDL3/SDL_process.h" 2
# 74 "SDL3/SDL.h" 2



# 1 "./SDL3/SDL_render.h" 1
# 65 "./SDL3/SDL_render.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 65 "./SDL3/SDL_render.h" 2
# 89 "./SDL3/SDL_render.h"
typedef struct SDL_Vertex
{
    SDL_FPoint position;
    SDL_FColor color;
    SDL_FPoint tex_coord;
} SDL_Vertex;






typedef enum SDL_TextureAccess
{
    SDL_TEXTUREACCESS_STATIC,
    SDL_TEXTUREACCESS_STREAMING,
    SDL_TEXTUREACCESS_TARGET
} SDL_TextureAccess;
# 119 "./SDL3/SDL_render.h"
typedef enum SDL_TextureAddressMode
{
    SDL_TEXTURE_ADDRESS_INVALID = -1,
    SDL_TEXTURE_ADDRESS_AUTO,
    SDL_TEXTURE_ADDRESS_CLAMP,
    SDL_TEXTURE_ADDRESS_WRAP
} SDL_TextureAddressMode;






typedef enum SDL_RendererLogicalPresentation
{
    SDL_LOGICAL_PRESENTATION_DISABLED,
    SDL_LOGICAL_PRESENTATION_STRETCH,
    SDL_LOGICAL_PRESENTATION_LETTERBOX,
    SDL_LOGICAL_PRESENTATION_OVERSCAN,
    SDL_LOGICAL_PRESENTATION_INTEGER_SCALE
} SDL_RendererLogicalPresentation;






typedef struct SDL_Renderer SDL_Renderer;
# 160 "./SDL3/SDL_render.h"
struct SDL_Texture
{
    SDL_PixelFormat format;
    int w;
    int h;

    int refcount;
};


typedef struct SDL_Texture SDL_Texture;
# 193 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) int  SDL_GetNumRenderDrivers(void);
# 217 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) const char *  SDL_GetRenderDriver(int index);
# 239 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_CreateWindowAndRenderer(const char *title, int width, int height, SDL_WindowFlags window_flags, SDL_Window **window, SDL_Renderer **renderer);
# 274 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) SDL_Renderer *  SDL_CreateRenderer(SDL_Window *window, const char *name);
# 338 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) SDL_Renderer *  SDL_CreateRendererWithProperties(SDL_PropertiesID props);
# 388 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) SDL_Renderer *  SDL_CreateGPURenderer(SDL_GPUDevice *device, SDL_Window *window);
# 401 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) SDL_GPUDevice *  SDL_GetGPURendererDevice(SDL_Renderer *renderer);
# 422 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) SDL_Renderer *  SDL_CreateSoftwareRenderer(SDL_Surface *surface);
# 435 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) SDL_Renderer *  SDL_GetRenderer(SDL_Window *window);
# 448 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) SDL_Window *  SDL_GetRenderWindow(SDL_Renderer *renderer);
# 464 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) const char *  SDL_GetRendererName(SDL_Renderer *renderer);
# 552 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) SDL_PropertiesID  SDL_GetRendererProperties(SDL_Renderer *renderer);
# 601 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_GetRenderOutputSize(SDL_Renderer *renderer, int *w, int *h);
# 624 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_GetCurrentRenderOutputSize(SDL_Renderer *renderer, int *w, int *h);
# 649 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) SDL_Texture *  SDL_CreateTexture(SDL_Renderer *renderer, SDL_PixelFormat format, SDL_TextureAccess access, int w, int h);
# 677 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) SDL_Texture *  SDL_CreateTextureFromSurface(SDL_Renderer *renderer, SDL_Surface *surface);
# 805 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) SDL_Texture *  SDL_CreateTextureWithProperties(SDL_Renderer *renderer, SDL_PropertiesID props);
# 934 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) SDL_PropertiesID  SDL_GetTextureProperties(SDL_Texture *texture);
# 978 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) SDL_Renderer *  SDL_GetRendererFromTexture(SDL_Texture *texture);
# 995 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_GetTextureSize(SDL_Texture *texture, float *w, float *h);
# 1017 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_SetTexturePalette(SDL_Texture *texture, SDL_Palette *palette);
# 1032 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) SDL_Palette *  SDL_GetTexturePalette(SDL_Texture *texture);
# 1061 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_SetTextureColorMod(SDL_Texture *texture, Uint8 r, Uint8 g, Uint8 b);
# 1091 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_SetTextureColorModFloat(SDL_Texture *texture, float r, float g, float b);
# 1112 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_GetTextureColorMod(SDL_Texture *texture, Uint8 *r, Uint8 *g, Uint8 *b);
# 1132 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_GetTextureColorModFloat(SDL_Texture *texture, float *r, float *g, float *b);
# 1158 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_SetTextureAlphaMod(SDL_Texture *texture, Uint8 alpha);
# 1184 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_SetTextureAlphaModFloat(SDL_Texture *texture, float alpha);
# 1202 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_GetTextureAlphaMod(SDL_Texture *texture, Uint8 *alpha);
# 1220 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_GetTextureAlphaModFloat(SDL_Texture *texture, float *alpha);
# 1239 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_SetTextureBlendMode(SDL_Texture *texture, SDL_BlendMode blendMode);
# 1255 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_GetTextureBlendMode(SDL_Texture *texture, SDL_BlendMode *blendMode);
# 1275 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_SetTextureScaleMode(SDL_Texture *texture, SDL_ScaleMode scaleMode);
# 1291 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_GetTextureScaleMode(SDL_Texture *texture, SDL_ScaleMode *scaleMode);
# 1325 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_UpdateTexture(SDL_Texture *texture, const SDL_Rect *rect, const void *pixels, int pitch);
# 1357 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_UpdateYUVTexture(SDL_Texture *texture,
                                                 const SDL_Rect *rect,
                                                 const Uint8 *Yplane, int Ypitch,
                                                 const Uint8 *Uplane, int Upitch,
                                                 const Uint8 *Vplane, int Vpitch);
# 1389 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_UpdateNVTexture(SDL_Texture *texture,
                                                 const SDL_Rect *rect,
                                                 const Uint8 *Yplane, int Ypitch,
                                                 const Uint8 *UVplane, int UVpitch);
# 1424 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_LockTexture(SDL_Texture *texture,
                                            const SDL_Rect *rect,
                                            void **pixels, int *pitch);
# 1462 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_LockTextureToSurface(SDL_Texture *texture, const SDL_Rect *rect, SDL_Surface **surface);
# 1483 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) void  SDL_UnlockTexture(SDL_Texture *texture);
# 1510 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_SetRenderTarget(SDL_Renderer *renderer, SDL_Texture *texture);
# 1527 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) SDL_Texture *  SDL_GetRenderTarget(SDL_Renderer *renderer);
# 1574 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_SetRenderLogicalPresentation(SDL_Renderer *renderer, int w, int h, SDL_RendererLogicalPresentation mode);
# 1599 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_GetRenderLogicalPresentation(SDL_Renderer *renderer, int *w, int *h, SDL_RendererLogicalPresentation *mode);
# 1624 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_GetRenderLogicalPresentationRect(SDL_Renderer *renderer, SDL_FRect *rect);
# 1651 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_RenderCoordinatesFromWindow(SDL_Renderer *renderer, float window_x, float window_y, float *x, float *y);
# 1681 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_RenderCoordinatesToWindow(SDL_Renderer *renderer, float x, float y, float *window_x, float *window_y);
# 1717 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_ConvertEventToRenderCoordinates(SDL_Renderer *renderer, SDL_Event *event);
# 1744 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_SetRenderViewport(SDL_Renderer *renderer, const SDL_Rect *rect);
# 1764 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_GetRenderViewport(SDL_Renderer *renderer, SDL_Rect *rect);
# 1786 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_RenderViewportSet(SDL_Renderer *renderer);
# 1808 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_GetRenderSafeArea(SDL_Renderer *renderer, SDL_Rect *rect);
# 1829 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_SetRenderClipRect(SDL_Renderer *renderer, const SDL_Rect *rect);
# 1850 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_GetRenderClipRect(SDL_Renderer *renderer, SDL_Rect *rect);
# 1869 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_RenderClipEnabled(SDL_Renderer *renderer);
# 1897 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_SetRenderScale(SDL_Renderer *renderer, float scaleX, float scaleY);
# 1917 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_GetRenderScale(SDL_Renderer *renderer, float *scaleX, float *scaleY);
# 1942 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_SetRenderDrawColor(SDL_Renderer *renderer, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
# 1967 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_SetRenderDrawColorFloat(SDL_Renderer *renderer, float r, float g, float b, float a);
# 1991 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_GetRenderDrawColor(SDL_Renderer *renderer, Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a);
# 2015 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_GetRenderDrawColorFloat(SDL_Renderer *renderer, float *r, float *g, float *b, float *a);
# 2039 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_SetRenderColorScale(SDL_Renderer *renderer, float scale);
# 2055 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_GetRenderColorScale(SDL_Renderer *renderer, float *scale);
# 2073 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_SetRenderDrawBlendMode(SDL_Renderer *renderer, SDL_BlendMode blendMode);
# 2089 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_GetRenderDrawBlendMode(SDL_Renderer *renderer, SDL_BlendMode *blendMode);
# 2109 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_RenderClear(SDL_Renderer *renderer);
# 2126 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_RenderPoint(SDL_Renderer *renderer, float x, float y);
# 2143 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_RenderPoints(SDL_Renderer *renderer, const SDL_FPoint *points, int count);
# 2162 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_RenderLine(SDL_Renderer *renderer, float x1, float y1, float x2, float y2);
# 2180 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_RenderLines(SDL_Renderer *renderer, const SDL_FPoint *points, int count);
# 2197 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_RenderRect(SDL_Renderer *renderer, const SDL_FRect *rect);
# 2215 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_RenderRects(SDL_Renderer *renderer, const SDL_FRect *rects, int count);
# 2233 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_RenderFillRect(SDL_Renderer *renderer, const SDL_FRect *rect);
# 2251 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_RenderFillRects(SDL_Renderer *renderer, const SDL_FRect *rects, int count);
# 2273 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_RenderTexture(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_FRect *srcrect, const SDL_FRect *dstrect);
# 2301 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_RenderTextureRotated(SDL_Renderer *renderer, SDL_Texture *texture,
                                                     const SDL_FRect *srcrect, const SDL_FRect *dstrect,
                                                     double angle, const SDL_FPoint *center,
                                                     SDL_FlipMode flip);
# 2332 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_RenderTextureAffine(SDL_Renderer *renderer, SDL_Texture *texture,
                                                     const SDL_FRect *srcrect, const SDL_FPoint *origin,
                                                     const SDL_FPoint *right, const SDL_FPoint *down);
# 2361 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_RenderTextureTiled(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_FRect *srcrect, float scale, const SDL_FRect *dstrect);
# 2396 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_RenderTexture9Grid(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_FRect *srcrect, float left_width, float right_width, float top_height, float bottom_height, float scale, const SDL_FRect *dstrect);
# 2434 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_RenderTexture9GridTiled(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_FRect *srcrect, float left_width, float right_width, float top_height, float bottom_height, float scale, const SDL_FRect *dstrect, float tileScale);
# 2461 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_RenderGeometry(SDL_Renderer *renderer,
                                               SDL_Texture *texture,
                                               const SDL_Vertex *vertices, int num_vertices,
                                               const int *indices, int num_indices);
# 2496 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_RenderGeometryRaw(SDL_Renderer *renderer,
                                               SDL_Texture *texture,
                                               const float *xy, int xy_stride,
                                               const SDL_FColor *color, int color_stride,
                                               const float *uv, int uv_stride,
                                               int num_vertices,
                                               const void *indices, int num_indices, int size_indices);
# 2523 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_SetRenderTextureAddressMode(SDL_Renderer *renderer, SDL_TextureAddressMode u_mode, SDL_TextureAddressMode v_mode);
# 2544 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_GetRenderTextureAddressMode(SDL_Renderer *renderer, SDL_TextureAddressMode *u_mode, SDL_TextureAddressMode *v_mode);
# 2571 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) SDL_Surface *  SDL_RenderReadPixels(SDL_Renderer *renderer, const SDL_Rect *rect);
# 2620 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_RenderPresent(SDL_Renderer *renderer);
# 2637 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) void  SDL_DestroyTexture(SDL_Texture *texture);
# 2653 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) void  SDL_DestroyRenderer(SDL_Renderer *renderer);
# 2686 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_FlushRenderer(SDL_Renderer *renderer);
# 2704 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) void *  SDL_GetRenderMetalLayer(SDL_Renderer *renderer);
# 2727 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) void *  SDL_GetRenderMetalCommandEncoder(SDL_Renderer *renderer);
# 2758 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_AddVulkanRenderSemaphores(SDL_Renderer *renderer, Uint32 wait_stage_mask, Sint64 wait_semaphore, Sint64 signal_semaphore);
# 2783 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_SetRenderVSync(SDL_Renderer *renderer, int vsync);
# 2803 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_GetRenderVSync(SDL_Renderer *renderer, int *vsync);
# 2855 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_RenderDebugText(SDL_Renderer *renderer, float x, float y, const char *str);
# 2883 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_RenderDebugTextFormat(SDL_Renderer *renderer, float x, float y,  const char *fmt, ...) ;
# 2901 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_SetDefaultTextureScaleMode(SDL_Renderer *renderer, SDL_ScaleMode scale_mode);
# 2919 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_GetDefaultTextureScaleMode(SDL_Renderer *renderer, SDL_ScaleMode *scale_mode);
# 2928 "./SDL3/SDL_render.h"
typedef struct SDL_GPURenderStateCreateInfo
{
    SDL_GPUShader *fragment_shader;

    Sint32 num_sampler_bindings;
    const SDL_GPUTextureSamplerBinding *sampler_bindings;

    Sint32 num_storage_textures;
    SDL_GPUTexture *const *storage_textures;

    Sint32 num_storage_buffers;
    SDL_GPUBuffer *const *storage_buffers;

    SDL_PropertiesID props;
} SDL_GPURenderStateCreateInfo;
# 2954 "./SDL3/SDL_render.h"
typedef struct SDL_GPURenderState SDL_GPURenderState;
# 2973 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) SDL_GPURenderState *  SDL_CreateGPURenderState(SDL_Renderer *renderer, const SDL_GPURenderStateCreateInfo *createinfo);
# 2993 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_SetGPURenderStateSamplerBindings(SDL_GPURenderState *state, int num_sampler_bindings, const SDL_GPUTextureSamplerBinding *sampler_bindings);
# 3012 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_SetGPURenderStateStorageTextures(SDL_GPURenderState *state, int num_storage_textures, SDL_GPUTexture *const *storage_textures);
# 3031 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_SetGPURenderStateStorageBuffers(SDL_GPURenderState *state, int num_storage_buffers, SDL_GPUBuffer *const *storage_buffers);
# 3051 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_SetGPURenderStateFragmentUniforms(SDL_GPURenderState *state, Uint32 slot_index, const void *data, Uint32 length);
# 3069 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) bool  SDL_SetGPURenderState(SDL_Renderer *renderer, SDL_GPURenderState *state);
# 3083 "./SDL3/SDL_render.h"
extern __attribute__((dllimport)) void  SDL_DestroyGPURenderState(SDL_GPURenderState *state);
# 3128 "./SDL3/SDL_render.h"
# 1 "./SDL3/SDL_close_code.h" 1
# 3128 "./SDL3/SDL_render.h" 2
# 77 "SDL3/SDL.h" 2



# 1 "./SDL3/SDL_storage.h" 1
# 252 "./SDL3/SDL_storage.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 252 "./SDL3/SDL_storage.h" 2
# 274 "./SDL3/SDL_storage.h"
typedef struct SDL_StorageInterface
{

    Uint32 version;


    bool ( *close)(void *userdata);


    bool ( *ready)(void *userdata);


    bool ( *enumerate)(void *userdata, const char *path, SDL_EnumerateDirectoryCallback callback, void *callback_userdata);


    bool ( *info)(void *userdata, const char *path, SDL_PathInfo *info);


    bool ( *read_file)(void *userdata, const char *path, void *destination, Uint64 length);


    bool ( *write_file)(void *userdata, const char *path, const void *source, Uint64 length);


    bool ( *mkdir)(void *userdata, const char *path);


    bool ( *remove)(void *userdata, const char *path);


    bool ( *rename)(void *userdata, const char *oldpath, const char *newpath);


    bool ( *copy)(void *userdata, const char *oldpath, const char *newpath);


    Uint64 ( *space_remaining)(void *userdata);
} SDL_StorageInterface;
# 321 "./SDL3/SDL_storage.h"
typedef int SDL_compile_time_assert_SDL_StorageInterface_SIZE[((sizeof(void *) == 4 && sizeof(SDL_StorageInterface) == 48) || (sizeof(void *) == 8 && sizeof(SDL_StorageInterface) == 96)) * 2 - 1];
# 332 "./SDL3/SDL_storage.h"
typedef struct SDL_Storage SDL_Storage;
# 353 "./SDL3/SDL_storage.h"
extern __attribute__((dllimport)) SDL_Storage *  SDL_OpenTitleStorage(const char *override, SDL_PropertiesID props);
# 379 "./SDL3/SDL_storage.h"
extern __attribute__((dllimport)) SDL_Storage *  SDL_OpenUserStorage(const char *org, const char *app, SDL_PropertiesID props);
# 403 "./SDL3/SDL_storage.h"
extern __attribute__((dllimport)) SDL_Storage *  SDL_OpenFileStorage(const char *path);
# 432 "./SDL3/SDL_storage.h"
extern __attribute__((dllimport)) SDL_Storage *  SDL_OpenStorage(const SDL_StorageInterface *iface, void *userdata);
# 450 "./SDL3/SDL_storage.h"
extern __attribute__((dllimport)) bool  SDL_CloseStorage(SDL_Storage *storage);
# 465 "./SDL3/SDL_storage.h"
extern __attribute__((dllimport)) bool  SDL_StorageReady(SDL_Storage *storage);
# 481 "./SDL3/SDL_storage.h"
extern __attribute__((dllimport)) bool  SDL_GetStorageFileSize(SDL_Storage *storage, const char *path, Uint64 *length);
# 504 "./SDL3/SDL_storage.h"
extern __attribute__((dllimport)) bool  SDL_ReadStorageFile(SDL_Storage *storage, const char *path, void *destination, Uint64 length);
# 522 "./SDL3/SDL_storage.h"
extern __attribute__((dllimport)) bool  SDL_WriteStorageFile(SDL_Storage *storage, const char *path, const void *source, Uint64 length);
# 536 "./SDL3/SDL_storage.h"
extern __attribute__((dllimport)) bool  SDL_CreateStorageDirectory(SDL_Storage *storage, const char *path);
# 565 "./SDL3/SDL_storage.h"
extern __attribute__((dllimport)) bool  SDL_EnumerateStorageDirectory(SDL_Storage *storage, const char *path, SDL_EnumerateDirectoryCallback callback, void *userdata);
# 579 "./SDL3/SDL_storage.h"
extern __attribute__((dllimport)) bool  SDL_RemoveStoragePath(SDL_Storage *storage, const char *path);
# 594 "./SDL3/SDL_storage.h"
extern __attribute__((dllimport)) bool  SDL_RenameStoragePath(SDL_Storage *storage, const char *oldpath, const char *newpath);
# 609 "./SDL3/SDL_storage.h"
extern __attribute__((dllimport)) bool  SDL_CopyStorageFile(SDL_Storage *storage, const char *oldpath, const char *newpath);
# 625 "./SDL3/SDL_storage.h"
extern __attribute__((dllimport)) bool  SDL_GetStoragePathInfo(SDL_Storage *storage, const char *path, SDL_PathInfo *info);
# 638 "./SDL3/SDL_storage.h"
extern __attribute__((dllimport)) Uint64  SDL_GetStorageSpaceRemaining(SDL_Storage *storage);
# 678 "./SDL3/SDL_storage.h"
extern __attribute__((dllimport)) char **  SDL_GlobStorageDirectory(SDL_Storage *storage, const char *path, const char *pattern, SDL_GlobFlags flags, int *count);






# 1 "./SDL3/SDL_close_code.h" 1
# 685 "./SDL3/SDL_storage.h" 2
# 80 "SDL3/SDL.h" 2


# 1 "./SDL3/SDL_system.h" 1
# 43 "./SDL3/SDL_system.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 43 "./SDL3/SDL_system.h" 2
# 54 "./SDL3/SDL_system.h"
typedef struct tagMSG MSG;
# 78 "./SDL3/SDL_system.h"
typedef bool ( *SDL_WindowsMessageHook)(void *userdata, MSG *msg);
# 96 "./SDL3/SDL_system.h"
extern __attribute__((dllimport)) void  SDL_SetWindowsMessageHook(SDL_WindowsMessageHook callback, void *userdata);
# 114 "./SDL3/SDL_system.h"
extern __attribute__((dllimport)) int  SDL_GetDirect3D9AdapterIndex(SDL_DisplayID displayID);
# 131 "./SDL3/SDL_system.h"
extern __attribute__((dllimport)) bool  SDL_GetDXGIOutputInfo(SDL_DisplayID displayID, int *adapterIndex, int *outputIndex);
# 141 "./SDL3/SDL_system.h"
typedef union _XEvent XEvent;
# 163 "./SDL3/SDL_system.h"
typedef bool ( *SDL_X11EventHook)(void *userdata, XEvent *xevent);
# 178 "./SDL3/SDL_system.h"
extern __attribute__((dllimport)) void  SDL_SetX11EventHook(SDL_X11EventHook callback, void *userdata);
# 630 "./SDL3/SDL_system.h"
extern __attribute__((dllimport)) bool  SDL_IsPhone(void);
# 643 "./SDL3/SDL_system.h"
extern __attribute__((dllimport)) bool  SDL_IsTablet(void);
# 656 "./SDL3/SDL_system.h"
extern __attribute__((dllimport)) bool  SDL_IsTV(void);






typedef enum SDL_Sandbox
{
    SDL_SANDBOX_NONE = 0,
    SDL_SANDBOX_UNKNOWN_CONTAINER,
    SDL_SANDBOX_FLATPAK,
    SDL_SANDBOX_SNAP,
    SDL_SANDBOX_MACOS
} SDL_Sandbox;
# 680 "./SDL3/SDL_system.h"
extern __attribute__((dllimport)) SDL_Sandbox  SDL_GetSandbox(void);
# 700 "./SDL3/SDL_system.h"
extern __attribute__((dllimport)) void  SDL_OnApplicationWillTerminate(void);
# 717 "./SDL3/SDL_system.h"
extern __attribute__((dllimport)) void  SDL_OnApplicationDidReceiveMemoryWarning(void);
# 734 "./SDL3/SDL_system.h"
extern __attribute__((dllimport)) void  SDL_OnApplicationWillEnterBackground(void);
# 751 "./SDL3/SDL_system.h"
extern __attribute__((dllimport)) void  SDL_OnApplicationDidEnterBackground(void);
# 768 "./SDL3/SDL_system.h"
extern __attribute__((dllimport)) void  SDL_OnApplicationWillEnterForeground(void);
# 785 "./SDL3/SDL_system.h"
extern __attribute__((dllimport)) void  SDL_OnApplicationDidEnterForeground(void);
# 852 "./SDL3/SDL_system.h"
# 1 "./SDL3/SDL_close_code.h" 1
# 852 "./SDL3/SDL_system.h" 2
# 82 "SDL3/SDL.h" 2


# 1 "./SDL3/SDL_time.h" 1
# 43 "./SDL3/SDL_time.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 43 "./SDL3/SDL_time.h" 2
# 54 "./SDL3/SDL_time.h"
typedef struct SDL_DateTime
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int nanosecond;
    int day_of_week;
    int utc_offset;
} SDL_DateTime;
# 74 "./SDL3/SDL_time.h"
typedef enum SDL_DateFormat
{
    SDL_DATE_FORMAT_YYYYMMDD = 0,
    SDL_DATE_FORMAT_DDMMYYYY = 1,
    SDL_DATE_FORMAT_MMDDYYYY = 2
} SDL_DateFormat;
# 88 "./SDL3/SDL_time.h"
typedef enum SDL_TimeFormat
{
    SDL_TIME_FORMAT_24HR = 0,
    SDL_TIME_FORMAT_12HR = 1
} SDL_TimeFormat;
# 113 "./SDL3/SDL_time.h"
extern __attribute__((dllimport)) bool  SDL_GetDateTimeLocalePreferences(SDL_DateFormat *dateFormat, SDL_TimeFormat *timeFormat);
# 127 "./SDL3/SDL_time.h"
extern __attribute__((dllimport)) bool  SDL_GetCurrentTime(SDL_Time *ticks);
# 145 "./SDL3/SDL_time.h"
extern __attribute__((dllimport)) bool  SDL_TimeToDateTime(SDL_Time ticks, SDL_DateTime *dt, bool localTime);
# 162 "./SDL3/SDL_time.h"
extern __attribute__((dllimport)) bool  SDL_DateTimeToTime(const SDL_DateTime *dt, SDL_Time *ticks);
# 180 "./SDL3/SDL_time.h"
extern __attribute__((dllimport)) void  SDL_TimeToWindows(SDL_Time ticks, Uint32 *dwLowDateTime, Uint32 *dwHighDateTime);
# 197 "./SDL3/SDL_time.h"
extern __attribute__((dllimport)) SDL_Time  SDL_TimeFromWindows(Uint32 dwLowDateTime, Uint32 dwHighDateTime);
# 211 "./SDL3/SDL_time.h"
extern __attribute__((dllimport)) int  SDL_GetDaysInMonth(int year, int month);
# 226 "./SDL3/SDL_time.h"
extern __attribute__((dllimport)) int  SDL_GetDayOfYear(int year, int month, int day);
# 241 "./SDL3/SDL_time.h"
extern __attribute__((dllimport)) int  SDL_GetDayOfWeek(int year, int month, int day);






# 1 "./SDL3/SDL_close_code.h" 1
# 248 "./SDL3/SDL_time.h" 2
# 84 "SDL3/SDL.h" 2

# 1 "./SDL3/SDL_timer.h" 1
# 48 "./SDL3/SDL_timer.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 48 "./SDL3/SDL_timer.h" 2
# 201 "./SDL3/SDL_timer.h"
extern __attribute__((dllimport)) Uint64  SDL_GetTicks(void);
# 213 "./SDL3/SDL_timer.h"
extern __attribute__((dllimport)) Uint64  SDL_GetTicksNS(void);
# 232 "./SDL3/SDL_timer.h"
extern __attribute__((dllimport)) Uint64  SDL_GetPerformanceCounter(void);
# 245 "./SDL3/SDL_timer.h"
extern __attribute__((dllimport)) Uint64  SDL_GetPerformanceFrequency(void);
# 263 "./SDL3/SDL_timer.h"
extern __attribute__((dllimport)) void  SDL_Delay(Uint32 ms);
# 281 "./SDL3/SDL_timer.h"
extern __attribute__((dllimport)) void  SDL_DelayNS(Uint64 ns);
# 299 "./SDL3/SDL_timer.h"
extern __attribute__((dllimport)) void  SDL_DelayPrecise(Uint64 ns);






typedef Uint32 SDL_TimerID;
# 332 "./SDL3/SDL_timer.h"
typedef Uint32 ( *SDL_TimerCallback)(void *userdata, SDL_TimerID timerID, Uint32 interval);
# 368 "./SDL3/SDL_timer.h"
extern __attribute__((dllimport)) SDL_TimerID  SDL_AddTimer(Uint32 interval, SDL_TimerCallback callback, void *userdata);
# 394 "./SDL3/SDL_timer.h"
typedef Uint64 ( *SDL_NSTimerCallback)(void *userdata, SDL_TimerID timerID, Uint64 interval);
# 430 "./SDL3/SDL_timer.h"
extern __attribute__((dllimport)) SDL_TimerID  SDL_AddTimerNS(Uint64 interval, SDL_NSTimerCallback callback, void *userdata);
# 445 "./SDL3/SDL_timer.h"
extern __attribute__((dllimport)) bool  SDL_RemoveTimer(SDL_TimerID id);







# 1 "./SDL3/SDL_close_code.h" 1
# 453 "./SDL3/SDL_timer.h" 2
# 85 "SDL3/SDL.h" 2

# 1 "./SDL3/SDL_tray.h" 1
# 42 "./SDL3/SDL_tray.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 42 "./SDL3/SDL_tray.h" 2
# 52 "./SDL3/SDL_tray.h"
typedef struct SDL_Tray SDL_Tray;






typedef struct SDL_TrayMenu SDL_TrayMenu;






typedef struct SDL_TrayEntry SDL_TrayEntry;
# 79 "./SDL3/SDL_tray.h"
typedef Uint32 SDL_TrayEntryFlags;
# 98 "./SDL3/SDL_tray.h"
typedef void ( *SDL_TrayCallback)(void *userdata, SDL_TrayEntry *entry);
# 114 "./SDL3/SDL_tray.h"
typedef bool ( *SDL_TrayClickCallback)(void *userdata, SDL_Tray *tray);
# 139 "./SDL3/SDL_tray.h"
extern __attribute__((dllimport)) SDL_Tray *  SDL_CreateTray(SDL_Surface *icon, const char *tooltip);
# 183 "./SDL3/SDL_tray.h"
extern __attribute__((dllimport)) SDL_Tray *  SDL_CreateTrayWithProperties(SDL_PropertiesID props);
# 205 "./SDL3/SDL_tray.h"
extern __attribute__((dllimport)) void  SDL_SetTrayIcon(SDL_Tray *tray, SDL_Surface *icon);
# 220 "./SDL3/SDL_tray.h"
extern __attribute__((dllimport)) void  SDL_SetTrayTooltip(SDL_Tray *tray, const char *tooltip);
# 244 "./SDL3/SDL_tray.h"
extern __attribute__((dllimport)) SDL_TrayMenu *  SDL_CreateTrayMenu(SDL_Tray *tray);
# 268 "./SDL3/SDL_tray.h"
extern __attribute__((dllimport)) SDL_TrayMenu *  SDL_CreateTraySubmenu(SDL_TrayEntry *entry);
# 292 "./SDL3/SDL_tray.h"
extern __attribute__((dllimport)) SDL_TrayMenu *  SDL_GetTrayMenu(SDL_Tray *tray);
# 316 "./SDL3/SDL_tray.h"
extern __attribute__((dllimport)) SDL_TrayMenu *  SDL_GetTraySubmenu(SDL_TrayEntry *entry);
# 336 "./SDL3/SDL_tray.h"
extern __attribute__((dllimport)) const SDL_TrayEntry **  SDL_GetTrayEntries(SDL_TrayMenu *menu, int *count);
# 351 "./SDL3/SDL_tray.h"
extern __attribute__((dllimport)) void  SDL_RemoveTrayEntry(SDL_TrayEntry *entry);
# 379 "./SDL3/SDL_tray.h"
extern __attribute__((dllimport)) SDL_TrayEntry *  SDL_InsertTrayEntryAt(SDL_TrayMenu *menu, int pos, const char *label, SDL_TrayEntryFlags flags);
# 401 "./SDL3/SDL_tray.h"
extern __attribute__((dllimport)) void  SDL_SetTrayEntryLabel(SDL_TrayEntry *entry, const char *label);
# 420 "./SDL3/SDL_tray.h"
extern __attribute__((dllimport)) const char *  SDL_GetTrayEntryLabel(SDL_TrayEntry *entry);
# 439 "./SDL3/SDL_tray.h"
extern __attribute__((dllimport)) void  SDL_SetTrayEntryChecked(SDL_TrayEntry *entry, bool checked);
# 458 "./SDL3/SDL_tray.h"
extern __attribute__((dllimport)) bool  SDL_GetTrayEntryChecked(SDL_TrayEntry *entry);
# 475 "./SDL3/SDL_tray.h"
extern __attribute__((dllimport)) void  SDL_SetTrayEntryEnabled(SDL_TrayEntry *entry, bool enabled);
# 492 "./SDL3/SDL_tray.h"
extern __attribute__((dllimport)) bool  SDL_GetTrayEntryEnabled(SDL_TrayEntry *entry);
# 510 "./SDL3/SDL_tray.h"
extern __attribute__((dllimport)) void  SDL_SetTrayEntryCallback(SDL_TrayEntry *entry, SDL_TrayCallback callback, void *userdata);
# 522 "./SDL3/SDL_tray.h"
extern __attribute__((dllimport)) void  SDL_ClickTrayEntry(SDL_TrayEntry *entry);
# 538 "./SDL3/SDL_tray.h"
extern __attribute__((dllimport)) void  SDL_DestroyTray(SDL_Tray *tray);
# 553 "./SDL3/SDL_tray.h"
extern __attribute__((dllimport)) SDL_TrayMenu *  SDL_GetTrayEntryParent(SDL_TrayEntry *entry);
# 573 "./SDL3/SDL_tray.h"
extern __attribute__((dllimport)) SDL_TrayEntry *  SDL_GetTrayMenuParentEntry(SDL_TrayMenu *menu);
# 593 "./SDL3/SDL_tray.h"
extern __attribute__((dllimport)) SDL_Tray *  SDL_GetTrayMenuParentTray(SDL_TrayMenu *menu);
# 605 "./SDL3/SDL_tray.h"
extern __attribute__((dllimport)) void  SDL_UpdateTrays(void);






# 1 "./SDL3/SDL_close_code.h" 1
# 612 "./SDL3/SDL_tray.h" 2
# 86 "SDL3/SDL.h" 2


# 1 "./SDL3/SDL_version.h" 1
# 35 "./SDL3/SDL_version.h"
# 1 "./SDL3/SDL_begin_code.h" 1
# 35 "./SDL3/SDL_version.h" 2
# 160 "./SDL3/SDL_version.h"
extern __attribute__((dllimport)) int  SDL_GetVersion(void);
# 189 "./SDL3/SDL_version.h"
extern __attribute__((dllimport)) const char *  SDL_GetRevision(void);







# 1 "./SDL3/SDL_close_code.h" 1
# 197 "./SDL3/SDL_version.h" 2
# 88 "SDL3/SDL.h" 2


# 1 "./SDL3/SDL_oldnames.h" 1
# 90 "SDL3/SDL.h" 2
# 2 "<string>" 2
