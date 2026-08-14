// snek\file.src
#include <snek.h>


extern void* fopen(i8* filename,i8* mode);
extern i32 fclose(void* file);
extern i32 fseek(void* file,i64 offset,i32 whence);
extern i64 ftell(void* file);
extern i64 fread(void* ptr,i64 size,i64 nmemb,void* file);
i32 readText_2_string_ptr_i8_i32(string path,i8* buffer);


const i32 SEEK_SET=0;
const i32 SEEK_CUR=1;
const i32 SEEK_END=2;
static i8* const _G1="r";


i32 readText_2_string_ptr_i8_i32(string path,i8* buffer){
	i8* const _1=path.ptr;
	void* const _2=fopen(_1,_G1);
	void* file=_2;
	const bool _3=(bool)file;
	if(_3){
		const i32 _4=fseek(file,0ll,2);
		const i64 _5=ftell(file);
		i64 numBytes=_5;
		const i32 _6=fseek(file,0ll,0);
		void* const _7=(void*)buffer;
		const i64 _8=fread(_7,1ll,numBytes,file);
		i64 charactersRead=_8;
		i8* const _9=&buffer[charactersRead];
		(*_9)='\0';
		const i32 _10=fclose(file);
		const i32 _11=(i32)charactersRead;
		return _11;
	}
	return 0;
}
