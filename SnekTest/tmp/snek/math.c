// snek\math.src
#include <snek.h>


extern float expf(float x);
extern double exp(double x);
float exp_1_float_float(float x);
extern float log10f(float x);
extern double log10(double x);
float log10_1_float_float(float x);
extern float powf(float x,float y);
extern double pow(double x,double y);
float pow_2_float_float_float(float x,float y);
extern float sinf(float x);
extern double sin(double x);
float sin_1_float_float(float x);
extern float cosf(float x);
extern double cos(double x);
float cos_1_float_float(float x);
extern float tanf(float x);
extern double tan(double x);
float tan_1_float_float(float x);
float sigmoid_1_float_float(float x);
i32 abs_1_i32_i32(i32 i);
i32 sign_1_i32_i32(i32 i);
float toRadians_1_float_float(float f);


const float PI=3.14159265358979323846;


float exp_1_float_float(float x){
const float _1=expf(x);
	return _1;
}
float log10_1_float_float(float x){
const float _1=log10f(x);
	return _1;
}
float pow_2_float_float_float(float x,float y){
const float _1=powf(x,y);
	return _1;
}
float sin_1_float_float(float x){
const float _1=sinf(x);
	return _1;
}
float cos_1_float_float(float x){
const float _1=cosf(x);
	return _1;
}
float tan_1_float_float(float x){
const float _1=tanf(x);
	return _1;
}
float sigmoid_1_float_float(float x){
1.0f1.0fconst float _1=-x;
const float _2=expf(_1);
const float _3=+_2;
const float _4=/_3;
	return _4;
}
i32 abs_1_i32_i32(i32 i){
const bool _1=i<0;
const i32 _2=-i;
const i32 _3=_1?_2:i;
	return _3;
}
i32 sign_1_i32_i32(i32 i){
const bool _1=i>0;
const bool _2=i<0;
const i32 _3=_2?-1:0;
const i32 _4=_1?1:_3;
	return _4;
}
float toRadians_1_float_float(float f){
180.0fconst float _1=f/;
3.14159265358979323846const float _2=_1*;
	return _2;
}
