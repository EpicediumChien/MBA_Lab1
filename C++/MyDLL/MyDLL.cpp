#include "MyDLL.h"
#include <stdio.h>

// Define functions
int __cdecl Add(int a, int b)
{
    return a + b;
}

int __cdecl Subtract(int a, int b)
{
    return a - b;
}

int __cdecl Multiply(int a, int b)
{
    return a * b;
}

int __cdecl Divide(int a, int b, int* error)
{
    if (b == 0)
    {
        if (error != NULL)
        {
            *error = 1;  // Set error code for division by zero
        }
        return 0;  // Return 0 as an invalid result
    }
    if (error != NULL)
    {
        *error = 0;  // No error
    }
    return a / b;
}