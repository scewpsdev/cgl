typedef char i8;
typedef short i16;
typedef int i32;
typedef long long i64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef float f32;
typedef double f64;
typedef _Bool bool;
typedef struct { char* ptr; long length; } string;

#define __INT8_MIN -0x80
#define __INT8_MAX 0x7f
#define __INT16_MIN -0x8000
#define __INT16_MAX 0x7fff
#define __INT32_MIN -0x80000000
#define __INT32_MAX 0x7fffffff
#define __INT64_MIN -0x8000000000000000
#define __INT64_MAX 0x7fffffffffffffff
#define __UINT8_MIN 0
#define __UINT8_MAX 0xff
#define __UINT16_MIN 0
#define __UINT16_MAX 0xffff
#define __UINT32_MIN 0
#define __UINT32_MAX 0xffffffff
#define __UINT64_MIN 0
#define __UINT64_MAX 0xffffffffffffffff


extern void* malloc(u64 size);

i64 __stoi64(string s)
{
	i64 value = 0;
	bool isNegative = 0;
	i32 base = 10;
	for (int i = 0; i < s.length; i++)
	{
		if (s.ptr[i] == '-')
			isNegative = 1;
		else if (s.ptr[i] == '_')
			;
		else if (s.ptr[i] >= '0' && s.ptr[i] <= '9')
			value = value * base + (s.ptr[i] - '0');
		else if ((s.ptr[i] >= 'A' && s.ptr[i] <= 'F' || s.ptr[i] >= 'a' && s.ptr[i] <= 'f') && base == 16)
			value = value * base + (s.ptr[i] - 'a' + 10);
		else if (base == 10 && s.ptr[i] == 'b')
			base = 2;
		else if (base == 10 && s.ptr[i] == 'o')
			base = 8;
		else if (base == 10 && s.ptr[i] == 'x')
			base = 16;
	}

	if (isNegative)
		value *= -1;

	return value;
}

i32 __stoi32(string s) { return (i32)__stoi64(s); }
i16 __stoi16(string s) { return (i16)__stoi64(s); }
i8 __stoi8(string s) { return (i8)__stoi64(s); }

u64 __stou64(string s)
{
	u64 value = 0;
	i32 base = 10;
	for (int i = 0; i < s.length; i++)
	{
		if (s.ptr[i] == '_')
			;
		else if (s.ptr[i] >= '0' && s.ptr[i] <= '9')
			value = value * base + (s.ptr[i] - '0');
		else if ((s.ptr[i] >= 'A' && s.ptr[i] <= 'F' || s.ptr[i] >= 'a' && s.ptr[i] <= 'f') && base == 16)
			value = value * base + (s.ptr[i] - 'a' + 10);
		else if (base == 10 && s.ptr[i] == 'b')
			base = 2;
		else if (base == 10 && s.ptr[i] == 'o')
			base = 8;
		else if (base == 10 && s.ptr[i] == 'x')
			base = 16;
	}

	return value;
}

u32 __stou32(string s) { return (u32)__stou64(s); }
u16 __stou16(string s) { return (u16)__stou64(s); }
u8 __stou8(string s) { return (u8)__stou64(s); }

string __itos(i64 n)
{
	char* buffer = malloc(21);
	int i = 0;

	bool isNeg = n < 0;

	i64 n1 = isNeg ? -n : n;

	while (n1 != 0)
	{
		buffer[i++] = n1 % 10 + '0';
		n1 = n1 / 10;
	}

	if (isNeg)
		buffer[i++] = '-';

	buffer[i] = '\0';

	for (int t = 0; t < i / 2; t++)
	{
		buffer[t] ^= buffer[i - t - 1];
		buffer[i - t - 1] ^= buffer[t];
		buffer[t] ^= buffer[i - t - 1];
	}

	if (n == 0)
	{
		buffer[0] = '0';
		buffer[1] = '\0';
		i = 1;
	}

	return (string) { buffer, i };
}

string __utos(u64 n)
{
	char* buffer = malloc(20);
	int i = 0;

	while (n != 0)
	{
		buffer[i++] = n % 10 + '0';
		n = n / 10;
	}

	buffer[i] = '\0';

	for (int t = 0; t < i / 2; t++)
	{
		buffer[t] ^= buffer[i - t - 1];
		buffer[i - t - 1] ^= buffer[t];
		buffer[t] ^= buffer[i - t - 1];
	}

	if (n == 0)
	{
		buffer[0] = '0';
		buffer[1] = '\0';
		i = 1;
	}

	return (string) { buffer, i };
}
