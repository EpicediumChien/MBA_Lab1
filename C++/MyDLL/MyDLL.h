#ifndef MYDLL_H
#define MYDLL_H

#ifdef BUILD_MYDLL
#define MYDLL_API __declspec(dllexport)  // Export when building the DLL
#else
#define MYDLL_API __declspec(dllimport)  // Import when using the DLL
#endif

// Declare functions
MYDLL_API int __cdecl Add(int a, int b);
MYDLL_API int __cdecl Subtract(int a, int b);
MYDLL_API int __cdecl Multiply(int a, int b);
MYDLL_API int __cdecl Divide(int a, int b, int* error);

#endif // MYDLL_H