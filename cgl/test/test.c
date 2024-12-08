// day1
#include <cgl.h>


typedef struct { i32 buffer[1000]; }__a1000i32;
typedef struct { i32* ptr; long length; }__ai32;
typedef struct { any* ptr; long length; }__ay;


static const string __G0 = { "input1.txt",10 };
static const string __G1 = { "Result: %",9 };


i32 parseNumber_3si32i32(string str, i32 idx, i32 len);

void day1();

extern string snek_file__readText_1s(string path);
extern void snek_sort__sort_1ai32(__ai32 list);
extern void snek_console__println_1s_vy(string format, __ay args);


i32 parseNumber_3si32i32(string str, i32 idx, i32 len) {
    i32 result = 0;
    {
        i32 i = 0;
        while (i < len) {
            assert(idx + i >= 0 && idx + i < str.length, "src\\day1.src", 10, 30);
            result = result * 10 + (i32)((str.ptr[idx + i] - (i8)(48)));
            i32 __0 = i;
            i += 1;
        }
    }
    return result;
}

void day1() {
    string __0 = snek_file__readText_1s(__G0);
    string str = __0;
    __a1000i32 list1 = { 0 };
    __a1000i32 list2 = { 0 };
    i32 cursor = 0;
    {
        i32 i = 0;
        while (i < 1000) {
            assert(i >= 0 && i < 1000, "src\\day1.src", 24, 8);
            i32 __1 = parseNumber_3si32i32(str, i * 14 + 0, 5);
            list1.buffer[i] = __1;
            assert(i >= 0 && i < 1000, "src\\day1.src", 25, 8);
            i32 __2 = parseNumber_3si32i32(str, i * 14 + 5 + 3, 5);
            list2.buffer[i] = __2;
            i32 __3 = i;
            i += 1;
        }
    }
    snek_sort__sort_1ai32({ list1.buffer,1000 });
    snek_sort__sort_1ai32({ list2.buffer,1000 });
    i32 result = 0;
    {
        i32 i = 0;
        while (i < 1000) {
            assert(i >= 0 && i < 1000, "src\\day1.src", 34, 18);
            assert(i >= 0 && i < 1000, "src\\day1.src", 34, 29);
            result = result + list2.buffer[i] - list1.buffer[i];
            i32 __4 = i;
            i += 1;
        }
    }
    any __5[1] = { (any) { (void*)2,* (void**)&result } };
    snek_console__println_1s_vy(__G1, (__ay) { __5, 1 });
}






#include <stdbool.h>
#include <stdlib.h>

#include <stdarg.h>


typedef struct { const char* buffer; long length; } string;



// main
//#include <cgl.h>


static const string __G0 = { "Hello World",11 };
static const string __G1 = { "lfdsjkfj",8 };


//static const string a = __G1;


extern void snek_console__writeln_1s(string str);


int main() {
	snek_console__writeln_1s(__G0);
	return 0;
}
