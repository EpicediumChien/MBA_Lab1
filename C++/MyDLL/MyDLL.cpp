#include "MyDLL.h"


MYDLL_API int __cdecl Add(int a, int b)
{
	return a+b;
}

MYDLL_API int __cdecl Subtract(int a, int b)
{
	return a - b;
}

MYDLL_API int __cdecl Multiply(int a, int b)
{
	return a * b;
}

MYDLL_API int __cdecl Divide(int a, int b)
{
	return a / b;
}


