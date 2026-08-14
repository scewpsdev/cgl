// snek/string.src
#include <snek.h>


extern i32 strncmp(i8* str1,i8* str2,u64 n);
string _substring_3stii(string str,i32 offset,i32 count);
bool _startsWith_2stst(string str,string value);
bool _endsWith_2stst(string str,string value);


string _substring_3stii(string str,i32 offset,i32 count){
i8* const _1=str.ptr;
const i64 _2=(i64)offset;
i8* const _3=_1+_2;
const string _4={_3,count};
	return _4;
}
bool _startsWith_2stst(string str,string value){
const u64 _1=str.length;
const u64 _2=value.length;
const bool _3=_1>=_2;
i8* const _4=str.ptr;
i8* const _5=value.ptr;
const u64 _6=value.length;
const i32 _7=strncmp(_4,_5,_6);
const bool _8=_7==0;
const bool _9=_3&&_8;
	return _9;
}
bool _endsWith_2stst(string str,string value){
const u64 _1=str.length;
const u64 _2=value.length;
const bool _3=_1>=_2;
i8* const _4=str.ptr;
const u64 _5=str.length;
const i64 _6=(i64)_5;
i8* const _7=_4+_6;
const u64 _8=value.length;
const i64 _9=(i64)_8;
i8* const _10=_7-_9;
i8* const _11=value.ptr;
const u64 _12=value.length;
const i32 _13=strncmp(_10,_11,_12);
const bool _14=_13==0;
const bool _15=_3&&_14;
	return _15;
}
