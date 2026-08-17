// main.src
#include <snek.h>


typedef struct{string* data;u64 length;}arr_0_string;


void _main_1Ast(arr_0_string args);
extern void _writeln_1st(string str);


static i8* const _G1="hello world";


void _main_1Ast(arr_0_string args){
	const string _1={_G1,11};
	_writeln_1st(_1);
}
int main(int argc, char* argv[]){
	string argsData[argc];
	for(int i=0;i<argc;i++)argsData[i]=(string){argv[i],__cstrl(argv[i])};
	arr_0_string args={argsData,argc};
	_main_1Ast(args);
	return 0;
}