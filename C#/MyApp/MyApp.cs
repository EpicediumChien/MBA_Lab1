using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace MyWPFApp
{
    public class MyDLL
    {
        //..\\..\\..\\..\\..\\C++\\x64\\Debug\\MyDLL.dll
        // D:\MBA\Lab1\MBA_Lab1\C++\x64\Debug
        [DllImport("MyDLL.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode, EntryPoint = "Add")]
        public extern static int Add(int a, int b);


        [DllImport("MyDLL.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode, EntryPoint = "Subtract")]
        public extern static int Subtract(int a, int b);


        [DllImport("MyDLL.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode, EntryPoint = "Multiply")]
        public extern static int Multiply(int a, int b);


        [DllImport("MyDLL.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode, EntryPoint = "Divide")]
        public extern static int Divide(int a, int b);

    }
}
