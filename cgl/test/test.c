#include <stdbool.h>
#include <stdlib.h>

#include <stdarg.h>


typedef struct { const char* buffer; long length; } string;


void foo()
{
	string a = { "flskdjfk", 5 };
	a = (string){ "flksdjflkj", 4 };

	string x = (string){ 0 };
}



typedef struct { float* ptr; long length; }__af32;
typedef struct { __af32 buffer[4]; }__a4af32;
typedef struct { float buffer[3]; }__a3f32;


static char __G0[] = "hi";
static float __G1[] = { 0.0f,1.0f,2.0f,3.0f,4.0f };


__af32 testArray = { __G1,5 };
__a4af32 trainingData2 = { {{(__a3f32) { {0.0f,0.0f,0.0f} }.buffer,3},{(__a3f32) { {1.0f,0.0f,1.0f} }.buffer,3},{(__a3f32) { {0.0f,1.0f,1.0f} }.buffer,3},{(__a3f32) { {1.0f,1.0f,1.0f} }.buffer,3}} };



int bar()
{
	return 5;
}

void testFunc(int a, ...)
{
	va_list args;
	__va_start(&args, a);
	va_start(args, a);
	char c = va_arg(args, int);
	va_end(args);
}
