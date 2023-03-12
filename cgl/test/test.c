#include <stdbool.h>

typedef struct { const char* buffer; long length; } string;


void foo()
{
	string a = { "flskdjfk", 5 };
	a = (string){"flksdjflkj", 4};

	string x = (string) { 0 };
}