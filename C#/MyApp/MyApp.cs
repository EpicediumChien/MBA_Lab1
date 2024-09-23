using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace MyApp
{
    public class MyDLL
    {
        [DllImport("..\\C++\\x64\\Debug\\MyDLL.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode, EntryPoint = "Add")]
        public extern static int Add(int a, int b);

    }
}
