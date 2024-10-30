using System;
using System.Runtime.InteropServices;
using System.Drawing;
using System.Drawing.Imaging;
using System.Windows.Media;
using PixelFormat = System.Drawing.Imaging.PixelFormat;
using Color = System.Drawing.Color;

namespace MyApp.Model
{
    public class NImage
    {
        [DllImport("NImageDLL.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr CreateNImage();

        [DllImport("NImageDLL.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void DeleteNImage(IntPtr nimage);

        [DllImport("NImageDLL.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern bool LoadImage(IntPtr nimage, string filename);

        [DllImport("NImageDLL.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern bool SaveImage(IntPtr nimage, string filename);

        [DllImport("NImageDLL.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int GetWidth(IntPtr nimage);

        [DllImport("NImageDLL.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int GetHeight(IntPtr nimage);

        [DllImport("NImageDLL.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int GetChannels(IntPtr nImage);

        [DllImport("NImageDLL.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr GetData(IntPtr nImage);

        [DllImport("NImageDLL.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr GetPalette(IntPtr nImage);

        private IntPtr nimage;

        public NImage()
        {
            nimage = CreateNImage();
        }

        ~NImage()
        {
            DeleteNImage(nimage);
        }

        public bool Load(string filename)
        {
            return LoadImage(nimage, filename);
        }

        public bool Save(string filename)
        {
            return SaveImage(nimage, filename);
        }

        public int Width => GetWidth(nimage);
        public int Height => GetHeight(nimage);
        public int Channels => GetChannels(nimage);

        public Bitmap ToBitmap()
        {
            int width = Width;
            int height = Height;
            int channels = Channels;

            IntPtr dataPtr = GetData(nimage);
            if (dataPtr == IntPtr.Zero) return null;

            Bitmap bmp = new Bitmap(width, height, channels == 3 ? PixelFormat.Format24bppRgb : PixelFormat.Format8bppIndexed);

            BitmapData bmpData = bmp.LockBits(new Rectangle(0, 0, width, height), ImageLockMode.WriteOnly, bmp.PixelFormat);

            int stride = bmpData.Stride;
            byte[] imageData = new byte[stride * height];
            Marshal.Copy(dataPtr, imageData, 0, imageData.Length);
            Marshal.Copy(imageData, 0, bmpData.Scan0, imageData.Length);

            bmp.UnlockBits(bmpData);

            if (channels == 1)
            {
                ColorPalette palette = bmp.Palette;
                IntPtr palettePtr = GetPalette(nimage);
                if (palettePtr != IntPtr.Zero)
                {
                    byte[] paletteData = new byte[256 * 4];
                    Marshal.Copy(palettePtr, paletteData, 0, 256 * 4);
                    for (int i = 0; i < 256; ++i)
                    {
                        palette.Entries[i] = Color.FromArgb(paletteData[i * 4 + 3], paletteData[i * 4 + 2], paletteData[i * 4 + 1], paletteData[i * 4]);
                    }
                    bmp.Palette = palette;
                }
            }

            return bmp;
        }
    }
}
