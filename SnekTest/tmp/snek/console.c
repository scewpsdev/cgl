// snek\console.src
#include <snek.h>


typedef struct{i8 data[50];}A50c;
typedef struct{any* data;u64 length;}A0a;


extern i32 putchar(i32 c);
extern i32 system(i8* command);
void _write_F1c(i8 c);
void _write_F1st(string str);
void _writeln_F1st(string str);
i32 _write_F1pc_i(i8* str);
i32 _writeln_F1pc_i(i8* str);
void _write_F1f(float f);
void _writeln_F1f(float f);
void _write_F1l(i64 i);
void _writeln_F1l(i64 i);
void _write_F1ul(u64 u);
void _writeln_F1ul(u64 u);
void _writeInt_F1l(i64 value);
void _writeUInt_F1ul(u64 value);
void _writeFloat_F1d(double value);
void _writeBoolean_F1b(bool b);
void _writePointer_F1pv(void* ptr);
void _writeAny_F1a(any value);
void _print_F2stA0a(string format,A0a args);
void _println_F2stA0a(string format,A0a args);


static i8* const _G1="true";
static i8* const _G2="false";


void _write_F1c(i8 c){
	const i32 _1=(i32)c;
	const i32 _2=putchar(_1);
}
void _write_F1st(string str){
	const u64 _1=str.length;
	const i32 _2=_1>=0?1:-1;
	for(int i=0;i*_2<_1*_2;i+=_2){
		i8* const _3=&str.data[i];
		const i32 _4=(i32)(*_3);
		const i32 _5=putchar(_4);
	}
}
void _writeln_F1st(string str){
	const u64 _1=str.length;
	const i32 _2=_1>=0?1:-1;
	for(int i=0;i*_2<_1*_2;i+=_2){
		i8* const _3=&str.data[i];
		const i32 _4=(i32)(*_3);
		const i32 _5=putchar(_4);
	}
	const i32 _6=(i32)'\n';
	const i32 _7=putchar(_6);
}
i32 _write_F1pc_i(i8* str){
	i32 i=0;
	while(1){
		i8* const _1=&str[i];
		const bool _2=(bool)(*_1);
		if(!_2)break;
		const i32 _3=i;
		i=i+1;
		i8* const _4=&str[_3];
		const i32 _5=(i32)(*_4);
		const i32 _6=putchar(_5);
	}
	return i;
}
i32 _writeln_F1pc_i(i8* str){
	i32 i=0;
	while(1){
		i8* const _1=&str[i];
		const bool _2=(bool)(*_1);
		if(!_2)break;
		const i32 _3=i;
		i=i+1;
		i8* const _4=&str[_3];
		const i32 _5=(i32)(*_4);
		const i32 _6=putchar(_5);
	}
	const i32 _7=(i32)'\n';
	const i32 _8=putchar(_7);
	const i32 _9=i;
	i=i+1;
	return i;
}
void _write_F1f(float f){
	const double _1=(double)f;
	_writeFloat_F1d(_1);
}
void _writeln_F1f(float f){
	const double _1=(double)f;
	_writeFloat_F1d(_1);
	const i32 _2=(i32)'\n';
	const i32 _3=putchar(_2);
}
void _write_F1l(i64 i){
	_writeInt_F1l(i);
}
void _writeln_F1l(i64 i){
	_writeInt_F1l(i);
	const i32 _1=(i32)'\n';
	const i32 _2=putchar(_1);
}
void _write_F1ul(u64 u){
	const i64 _1=(i64)u;
	_writeInt_F1l(_1);
}
void _writeln_F1ul(u64 u){
	const i64 _1=(i64)u;
	_writeInt_F1l(_1);
	const i32 _2=(i32)'\n';
	const i32 _3=putchar(_2);
}
void _writeInt_F1l(i64 value){
	const i64 _1=(i64)0;
	const bool _2=value==_1;
	if(_2){
		const i32 _3=(i32)'0';
		const i32 _4=putchar(_3);
		return;
	}
	A50c buffer={0};
	i32 len=0;
	const i64 _5=(i64)0;
	const bool _6=value<_5;
	bool isNegative=_6;
	if(isNegative){
		const i64 _7=-value;
		value=_7;
	}
	while(1){
		const bool _8=(bool)value;
		if(!_8)break;
		const i32 _9=len;
		len=len+1;
		i8* const _10=&buffer.data[_9];
		const i64 _11=(i64)10;
		const i64 _12=value%_11;
		const i64 _13=(i64)'0';
		const i64 _14=_12+_13;
		const i8 _15=(i8)_14;
		(*_10)=_15;
		const i64 _16=(i64)10;
		const i64 _17=value/_16;
		value=_17;
	}
	if(isNegative){
		const i32 _18=len;
		len=len+1;
		i8* const _19=&buffer.data[_18];
		(*_19)='-';
	}
	const i32 _20=len-1;
	const i32 _21=0>=_20?1:-1;
	for(int i=_20;i*_21<=0*_21;i+=_21){
		i8* const _22=&buffer.data[i];
		const i32 _23=(i32)(*_22);
		const i32 _24=putchar(_23);
	}
}
void _writeUInt_F1ul(u64 value){
	const u64 _1=(u64)0;
	const bool _2=value==_1;
	if(_2){
		const i32 _3=(i32)'0';
		const i32 _4=putchar(_3);
		return;
	}
	A50c buffer={0};
	i32 len=0;
	while(1){
		const bool _5=(bool)value;
		if(!_5)break;
		const i32 _6=len;
		len=len+1;
		i8* const _7=&buffer.data[_6];
		const u64 _8=(u64)10;
		const u64 _9=value%_8;
		const u64 _10=(u64)'0';
		const u64 _11=_9+_10;
		const i8 _12=(i8)_11;
		(*_7)=_12;
		const u64 _13=(u64)10u;
		const u64 _14=value/_13;
		value=_14;
	}
	const i32 _15=len-1;
	const i32 _16=0>=_15?1:-1;
	for(int i=_15;i*_16<=0*_16;i+=_16){
		i8* const _17=&buffer.data[i];
		const i32 _18=(i32)(*_17);
		const i32 _19=putchar(_18);
	}
}
void _writeFloat_F1d(double value){
	const i64 _1=(i64)value;
	i64 ipart=_1;
	const double _2=(double)ipart;
	const double _3=value-_2;
	double fpart=_3;
	_writeInt_F1l(ipart);
	const i32 _4=(i32)'.';
	const i32 _5=putchar(_4);
	i32 precision=6;
	const i32 _6=precision>=0?1:-1;
	for(int __it=0;__it*_6<precision*_6;__it+=_6){
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
void _writeBoolean_F1b(bool b){
	if(b){
		const string _1={_G1,4};
		_write_F1st(_1);
	}else{
		const string _2={_G2,5};
		_write_F1st(_2);
	}
}
void _writePointer_F1pv(void* ptr){
	return;
}
void _writeAny_F1a(any value){
	const i32 _1=value.type;
	const i32 _2=(i32)2;
	const bool _3=_1==_2;
	if(_3){
		const i32 _4=(i32)value.int_;
		const i64 _5=(i64)_4;
		_writeInt_F1l(_5);
	}else{
		const i32 _6=value.type;
		const i32 _7=(i32)12;
		const bool _8=_6==_7;
		if(_8){
			const float _9=(float)value.float_;
			const double _10=(double)_9;
			_writeFloat_F1d(_10);
		}else{
			const i32 _11=value.type;
			const i32 _12=(i32)16;
			const bool _13=_11==_12;
			if(_13){
				const bool _14=value.bool_;
				_writeBoolean_F1b(_14);
			}else{
				const i32 _15=value.type;
				const i32 _16=(i32)23;
				const bool _17=_15==_16;
				if(_17){
					void** const _18=(void**)value.ptr;
					_writePointer_F1pv((*_18));
				}else{
					const i32 _19=value.type;
					const i32 _20=(i32)18;
					const bool _21=_19==_20;
					if(_21){
						string* const _22=(string*)value.ptr;
						_write_F1st((*_22));
					}else{
						const i32 _23=(i32)'?';
						const i32 _24=putchar(_23);
					}
				}
			}
		}
	}
}
void _print_F2stA0a(string format,A0a args){
	i32 varArgIdx=0;
	const u64 _1=format.length;
	const i32 _2=_1>=0?1:-1;
	for(int i=0;i*_2<_1*_2;i+=_2){
		i8* const _3=&format.data[i];
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
			_writeAny_F1a((*_10));
		}else{
			const i32 _11=(i32)c;
			const i32 _12=putchar(_11);
		}
	}
}
void _println_F2stA0a(string format,A0a args){
	const any _1={.ptr=&args,.type=26};
	any _2[1]={_1};
	const A0a _3={_2,1};
	_print_F2stA0a(format,_3);
	const i32 _4=(i32)'\n';
	const i32 _5=putchar(_4);
}
