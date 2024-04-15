#include "cgl.h"


struct _iobuf {
	char* _ptr;
	int _cnt;
	char* _base;
	int _flag;
	int _file;
	int _charbuf;
	int _bufsiz;
	char* _tmpfname;
};
typedef struct _iobuf FILE;
extern FILE(*_imp___iob)[];
#define __iob_func()    (*_imp___iob)
#define stderr (&__iob_func()[2])

#define	SIGTRAP	5

extern void* malloc(u64 size);
extern int fputs(const char*, FILE*);
int raise(int _SigNum);
void exit(int status);


void __debugbreak()
{
	raise(SIGTRAP);
}

void __assertmsg(int x, const char* msg)
{
	if (!x)
	{
		fputs(msg, stderr);
		__debugbreak();
		exit(-1);
	}
}


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
