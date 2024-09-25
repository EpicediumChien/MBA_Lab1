// MyDLL.cpp : 定義 DLL 應用程式的匯出函式。
//
// #include "pch.h"
#include "MyDLL.h"


// 這是匯出函式的範例。
MYDLL_API int __cdecl Add(int a, int b)
{
	return a+b;
}

// 這是匯出函式的範例。
MYDLL_API int __cdecl Subtract(int a, int b)
{
	return a - b;
}

// 這是匯出函式的範例。
MYDLL_API int __cdecl Multiply(int a, int b)
{
	return a * b;
}

// 這是匯出函式的範例。
MYDLL_API int __cdecl Divide(int a, int b)
{
	return a / b;
}


