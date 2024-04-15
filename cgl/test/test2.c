// main
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
struct _iobuf {
	char* _ptr;
	int _cnt;
	char* _base;
	int _flag;
	int _file;
	int _charbuf;
	int _bufsiz;
	char* _tmpfname;
};
typedef struct _iobuf FILE;
extern FILE(*_imp___iob)[];
#define __iob_func()    (*_imp___iob)
int fputs(const char*, FILE*);
int raise(int _SigNum);
#define	SIGTRAP	5
#define debugbreak() raise(SIGTRAP);
void exit(int status);
#define assert(x, file, line, col) if(!(x)) { fputs("Assertion failed at " file ":" #line ":" #col ": " #x "\n", &__iob_func()[2]); debugbreak(); exit(-1); }


typedef struct { any* ptr; long length; }__ay;


static char __G0[] = "hello world";
static char __G1[] = "%";


extern void snek_console__writeAny_1y(any value);
extern void putchar(i32 c);
extern void snek_console__print_1s_vy(string format, __ay args);


void test_1s_vy(string format, __ay args) {
	i32 varArgIdx = 0;
	i32 i = 0;
	while (i < format.length) {
		assert(i >= 0 && i < format.length, "src\\main.src", 10, 18);
		i8 c = format.ptr[i];
		if ((c == 37 && varArgIdx < args.length)) {
			assert(varArgIdx >= 0 && varArgIdx < args.length, "src\\main.src", 13, 17);
			snek_console__writeAny_1y(args.ptr[varArgIdx]);
			i32 __0 = varArgIdx;
			varArgIdx += 1;
		}
		else {
			putchar((i32)(c));
		}
		i32 __1 = i;
		i += 1;
	}
}

int main() {
	i64 __0 = 42;
	any __1[1] = { (any) { (void*)2,* (void**)&__0 } };
	__ay arr = { __1,1 };
	assert(0 >= 0 && 0 < arr.length, "src\\main.src", 24, 5);
	string __2 = (string){ __G0,11 };
	arr.ptr[0] = (any){ (void*)15,*(void**)&__2 };
	assert(0 >= 0 && 0 < arr.length, "src\\main.src", 25, 16);
	any __3[1] = { arr.ptr[0] };
	snek_console__print_1s_vy((string) { __G1, 1 }, (__ay) { __3, 1 });
	return 0;
}
