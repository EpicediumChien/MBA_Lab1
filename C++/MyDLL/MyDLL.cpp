// MyDLL.cpp : �w�q DLL ���ε{�����ץX�禡�C
//
// #include "pch.h"
#include "MyDLL.h"


// �o�O�ץX�禡���d�ҡC
MYDLL_API int __cdecl Add(int a, int b)
{
	return a+b;
}

// �o�O�ץX�禡���d�ҡC
MYDLL_API int __cdecl Subtract(int a, int b)
{
	return a - b;
}

// �o�O�ץX�禡���d�ҡC
MYDLL_API int __cdecl Multiply(int a, int b)
{
	return a * b;
}

// �o�O�ץX�禡���d�ҡC
MYDLL_API int __cdecl Divide(int a, int b)
{
	return a / b;
}


