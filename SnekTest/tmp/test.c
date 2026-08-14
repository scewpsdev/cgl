// test.src
#include <snek.h>


void _runTests_0();
extern string _substring_3stii(string str,i32 offset,i32 count);
extern void _writeln_1st(string str);


static i8* const _G1="Test String";


void _runTests_0(){
	const string _1={_G1,11};
	string str=_1;
	const string _2=_substring_3stii(str,0,4);
	string a=_2;
	const string _3=_substring_3stii(str,5,6);
	string b=_3;
	_writeln_1st(a);
	_writeln_1st(b);
}
