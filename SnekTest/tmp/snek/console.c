// snek\console.src
#include <snek.h>


typedef struct{i8 data[50];}arr_50_i8;
typedef struct{any* data;u64 length;}arr_0_any;


extern i32 putchar(i32 c);
extern i32 system(i8* command);
void _write_1c(i8 c);
void _write_1st(string str);
void _writeln_1st(string str);
i32 _write_1pc(i8* str);
i32 _writeln_1pc(i8* str);
void _write_1f(float f);
void _writeln_1f(float f);
void _write_1l(i64 i);
void _writeln_1l(i64 i);
void _write_1ul(u64 u);
void _writeln_1ul(u64 u);
void _writeInt_1l(i64 value);
void _writeUInt_1ul(u64 value);
void _writeFloat_1d(double value);
void _writeBoolean_1b(bool b);
void _writePointer_1pv(void* ptr);
void _writeAny_1a(any value);
void _print_2stAa(string format,arr_0_any args);
void _println_2stAa(string format,arr_0_any args);


static i8* const _G1="true";
static i8* const _G2="false";


void _write_1c(i8 c){
	const i32 _1=(i32)c;
	const i32 _2=putchar(_1);
}
void _write_1st(string str){
	const u64 _1=str.length;
	const i32 _2=_1>=0?1:-1;
	for(int i=0;i*_2<_1*_2;i+=_2){
		i8* const _3=&str.ptr[i];
		const i32 _4=(i32)(*_3);
		const i32 _5=putchar(_4);
	}
}
void _writeln_1st(string str){
	const u64 _1=str.length;
	const i32 _2=_1>=0?1:-1;
	for(int i=0;i*_2<_1*_2;i+=_2){
		i8* const _3=&str.ptr[i];
		const i32 _4=(i32)(*_3);
		const i32 _5=putchar(_4);
	}
	const i32 _6=(i32)'\n';
	const i32 _7=putchar(_6);
}
i32 _write_1pc(i8* str){
	i32 i=0;
	while(1){
		i8* const _1=&str[i];
		const i32 _2=(i32)(*_1);
		const bool _3=_2!=0;
		if(!_3)break;
		const i32 _4=i;
		i=i+1;
		i8* const _5=&str[_4];
		const i32 _6=(i32)(*_5);
		const i32 _7=putchar(_6);
	}
	return i;
}
i32 _writeln_1pc(i8* str){
	i32 i=0;
	while(1){
		i8* const _1=&str[i];
		const i32 _2=(i32)(*_1);
		const bool _3=_2!=0;
		if(!_3)break;
		const i32 _4=i;
		i=i+1;
		i8* const _5=&str[_4];
		const i32 _6=(i32)(*_5);
		const i32 _7=putchar(_6);
	}
	const i32 _8=(i32)'\n';
	const i32 _9=putchar(_8);
	const i32 _10=i;
	i=i+1;
	return i;
}
void _write_1f(float f){
	const double _1=(double)f;
	_writeFloat_1d(_1);
}
void _writeln_1f(float f){
	const double _1=(double)f;
	_writeFloat_1d(_1);
	const i32 _2=(i32)'\n';
	const i32 _3=putchar(_2);
}
void _write_1l(i64 i){
	_writeInt_1l(i);
}
void _writeln_1l(i64 i){
	_writeInt_1l(i);
	const i32 _1=(i32)'\n';
	const i32 _2=putchar(_1);
}
void _write_1ul(u64 u){
	const i64 _1=(i64)u;
	_writeInt_1l(_1);
}
void _writeln_1ul(u64 u){
	const i64 _1=(i64)u;
	_writeInt_1l(_1);
	const i32 _2=(i32)'\n';
	const i32 _3=putchar(_2);
}
void _writeInt_1l(i64 value){
	const i64 _1=(i64)0;
	const bool _2=value==_1;
	if(_2){
		const i32 _3=(i32)'0';
		const i32 _4=putchar(_3);
		return;
	}
	arr_50_i8 buffer={0};
	i32 len=0;
	const i64 _5=(i64)0;
	const bool _6=value<_5;
	bool isNegative=_6;
	if(isNegative){
		const i64 _7=-value;
		value=_7;
	}
	while(1){
		const i64 _8=(i64)0;
		const bool _9=value!=_8;
		if(!_9)break;
		const i32 _10=len;
		len=len+1;
		i8* const _11=&buffer.data[_10];
		const i64 _12=(i64)10;
		const i64 _13=value%_12;
		const i64 _14=(i64)'0';
		const i64 _15=_13+_14;
		const i8 _16=(i8)_15;
		(*_11)=_16;
		const i64 _17=(i64)10;
		const i64 _18=value/_17;
		value=_18;
	}
	if(isNegative){
		const i32 _19=len;
		len=len+1;
		i8* const _20=&buffer.data[_19];
		(*_20)='-';
	}
	const i32 _21=len-1;
	const i32 _22=0>=_21?1:-1;
	for(int i=_21;i*_22<=0*_22;i+=_22){
		i8* const _23=&buffer.data[i];
		const i32 _24=(i32)(*_23);
		const i32 _25=putchar(_24);
	}
}
void _writeUInt_1ul(u64 value){
	const u64 _1=(u64)0;
	const bool _2=value==_1;
	if(_2){
		const i32 _3=(i32)'0';
		const i32 _4=putchar(_3);
		return;
	}
	arr_50_i8 buffer={0};
	i32 len=0;
	while(1){
		const u64 _5=(u64)0;
		const bool _6=value!=_5;
		if(!_6)break;
		const i32 _7=len;
		len=len+1;
		i8* const _8=&buffer.data[_7];
		const u64 _9=(u64)10;
		const u64 _10=value%_9;
		const u64 _11=(u64)'0';
		const u64 _12=_10+_11;
		const i8 _13=(i8)_12;
		(*_8)=_13;
		const u64 _14=(u64)10u;
		const u64 _15=value/_14;
		value=_15;
	}
	const i32 _16=len-1;
	const i32 _17=0>=_16?1:-1;
	for(int i=_16;i*_17<=0*_17;i+=_17){
		i8* const _18=&buffer.data[i];
		const i32 _19=(i32)(*_18);
		const i32 _20=putchar(_19);
	}
}
void _writeFloat_1d(double value){
	const i64 _1=(i64)value;
	i64 ipart=_1;
	const double _2=(double)ipart;
	const double _3=value-_2;
	double fpart=_3;
	_writeInt_1l(ipart);
	const i32 _4=(i32)'.';
	const i32 _5=putchar(_4);
	i32 precision=6;
	const i32 _6=precision>=0?1:-1;
	for(int i=0;i*_6<precision*_6;i+=_6){
		const double _7=(double)10;
		fpart*=_7;
		const i32 _8=(i32)fpart;
		i32 digit=_8;
		const i32 _9=(i32)'0';
		const i32 _10=_9+digit;
		const i32 _11=putchar(_10);
		const double _12=(double)digit;
		fpart-=_12;
	}
}
void _writeBoolean_1b(bool b){
	if(b){
		const string _1={_G1,4};
		_write_1st(_1);
	}else{
		const string _2={_G2,5};
		_write_1st(_2);
	}
}
void _writePointer_1pv(void* ptr){
	return;
}
void _writeAny_1a(any value){
	const i32 _1=value.type;
	const i32 _2=(i32)2;
	const bool _3=_1==_2;
	if(_3){
		const i32 _4=(i32)value.int_;
		const i64 _5=(i64)_4;
		_writeInt_1l(_5);
	}else{
		const i32 _6=value.type;
		const i32 _7=(i32)12;
		const bool _8=_6==_7;
		if(_8){
			const float _9=(float)value.float_;
			const double _10=(double)_9;
			_writeFloat_1d(_10);
		}else{
			const i32 _11=value.type;
			const i32 _12=(i32)16;
			const bool _13=_11==_12;
			if(_13){
				const bool _14=value.bool_;
				_writeBoolean_1b(_14);
			}else{
				const i32 _15=value.type;
				const i32 _16=(i32)23;
				const bool _17=_15==_16;
				if(_17){
					void** const _18=(void**)value.ptr;
					_writePointer_1pv((*_18));
				}else{
					const i32 _19=value.type;
					const i32 _20=(i32)18;
					const bool _21=_19==_20;
					if(_21){
						string* const _22=(string*)value.ptr;
						_write_1st((*_22));
					}else{
						const i32 _23=(i32)'?';
						const i32 _24=putchar(_23);
					}
				}
			}
		}
	}
}
void _print_2stAa(string format,arr_0_any args){
	i32 varArgIdx=0;
	const u64 _1=format.length;
	const i32 _2=_1>=0?1:-1;
	for(int i=0;i*_2<_1*_2;i+=_2){
		i8* const _3=&format.ptr[i];
		i8 c=(*_3);
		const bool _4=c=='%';
		const u64 _5=(u64)varArgIdx;
		const u64 _6=args.length;
		const bool _7=_5<_6;
		const bool _8=_4&&_7;
		if(_8){
			const i32 _9=varArgIdx;
			varArgIdx=varArgIdx+1;
			any* const _10=&args.data[_9];
			_writeAny_1a((*_10));
		}else{
			const i32 _11=(i32)c;
			const i32 _12=putchar(_11);
		}
	}
}
void _println_2stAa(string format,arr_0_any args){
	const any _1={.ptr=&args,.type=26};
	any _2[1]={_1};
	const arr_0_any _3={_2,1};
	_print_2stAa(format,_3);
	const i32 _4=(i32)'\n';
	const i32 _5=putchar(_4);
}
