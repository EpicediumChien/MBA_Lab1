using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Channels;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Media3D;
using System.Windows.Shapes;

namespace MyApp
{
    /// <summary>
    /// LoadImagePage.xaml 的互動邏輯
    /// </summary>
    public partial class LoadImagePage : Window
    {
        #region Properties
        private IntPtr loadedImageIntPtr;
        int width, height, channels, stride;
        #endregion
        // Load the DLL functions
        private const string DllName = $"MyDLL.dll";
        // Import DLL functions
        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr CreateNImage();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void DeleteNImage(IntPtr nimage);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool LoadImage(IntPtr nimage, string filename);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetWidth(IntPtr nimage);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetHeight(IntPtr nimage);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetChannels(IntPtr nimage);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr GetData(IntPtr nimage);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr ApplyGaussianBlurImage(IntPtr nimage, int kernelSize, double sigma);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr InverseImage(IntPtr nimage, int width, int height, int channel);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr RgbToGray8bit(IntPtr nimage, int width, int height);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr AdaptiveThresholdImage(IntPtr nimage);

        private static string tempFilePath = string.Empty;

        public LoadImagePage()
        {
            InitializeComponent();
        }

        private void OnClick_LoadImage(object sender, RoutedEventArgs e)
        {
            #region browse
            // Create an OpenFileDialog instance
            OpenFileDialog openFileDialog = new OpenFileDialog
            {
                Filter = "Image Files|*.bmp;|All Files|*.*", // Filter for image files
                Title = "Select an Image File"
            };

            // Show the dialog and get result
            if (openFileDialog.ShowDialog() == true)
            {
                tempFilePath = openFileDialog.FileName;
            }
            #endregion

            #region LoadImage
            nint loadedImage = CreateNImage();

            string imagePath = tempFilePath; // Update with your BMP image path
            if (LoadImage(loadedImage, imagePath))
            {
                // Only on load image
                width = GetWidth(loadedImage);
                height = GetHeight(loadedImage);
                channels = GetChannels(loadedImage);
                stride = (width * channels + 3) & ~3;
                loadedImageIntPtr = GetData(loadedImage);
                ShowIntPtrOnImage(loadedImageIntPtr);
            }
            else
            {
                MessageBox.Show("Failed to load image.");
            }
            #endregion
        }

        private void OnClick_Gaussian(object sender, RoutedEventArgs e) { }

        private void OnClick_Sobel(object sender, RoutedEventArgs e) { }

        private void OnClick_Inverse(object sender, RoutedEventArgs e)
        {

            IntPtr dataPtr = InverseImage(loadedImageIntPtr, width, height, channels);
            if (dataPtr != IntPtr.Zero)
            {
                ShowIntPtrOnImage(dataPtr);
            }
            else
            {
                MessageBox.Show("Failed to load image.");
            }
        }

        private void OnClick_AdaptiveThreshold(object sender, RoutedEventArgs e)
        {
            // Turn gray
            IntPtr dataPtr = RgbToGray8bit(loadedImageIntPtr, width, height);
            if (dataPtr != IntPtr.Zero) {
                IntPtr thresholdPtr = dataPtr; // AdaptiveThresholdImage(dataPtr);
                // Copy data to a managed array
                byte[] imageData = new byte[width * height];
                Marshal.Copy(thresholdPtr, imageData, 0, imageData.Length);

                PixelFormat pixelFormat = PixelFormats.Bgr24;
                if (channels == 1)
                {
                    pixelFormat = PixelFormats.Gray8;
                }
                // Create a BitmapSource from the loaded image data
                BitmapSource bitmap = BitmapSource.Create(
                    width,
                    height,
                    96, // DPI X
                    96, // DPI Y
                    pixelFormat, // Assuming 24-bit color
                    null,
                    imageData,
                    width * channels);

                LoadedImage.Source = bitmap; // Set to your Image control
            }
            else
            {
                MessageBox.Show("Failed to load image.");
            }
        }

        private void OnClick_Ini(object sender, RoutedEventArgs e)
        {
            if (loadedImageIntPtr != IntPtr.Zero)
            {
                ShowIntPtrOnImage(loadedImageIntPtr);
            }
            else
            {
                MessageBox.Show("Failed to read cache.");
            }
        }

        private void ShowIntPtrOnImage(IntPtr imgSource)
        {
            // Copy data to a managed array
            byte[] imageData = new byte[stride * height];

            if (imgSource == IntPtr.Zero)
            {
                MessageBox.Show("Failed to retrieve image data.");
                return;
            }

            // Ensure width, height, and channels are correctly assigned
            if (width <= 0 || height <= 0 || (channels != 1 && channels != 3))
            {
                MessageBox.Show("Invalid image dimensions or channels.");
                return;
            }
            Marshal.Copy(imgSource, imageData, 0, imageData.Length);

            // Handle the case where the image is upside down (flip vertically)
            if (height > 1)
            {
                int rowSize = stride;  // Number of bytes per row
                byte[] flippedData = new byte[imageData.Length];
                for (int y = 0; y < height; y++)
                {
                    Array.Copy(imageData, y * rowSize, flippedData, (height - 1 - y) * rowSize, rowSize);
                }
                imageData = flippedData;  // Use the flipped data
            }


            // Set the pixel format based on channels
            PixelFormat pixelFormat = PixelFormats.Bgr24;
            if (channels == 1)
            {
                pixelFormat = PixelFormats.Gray8;
            }

            // Create a BitmapSource from the loaded image data
            BitmapSource bitmap = BitmapSource.Create(
                width,
                height,
                96, // DPI X
                96, // DPI Y
                pixelFormat,
                null,
                imageData,
                stride);

            // Set the image to the Image control
            LoadedImage.Source = bitmap;
        }

        //private IntPtr GetImagePointer(Image wpfImage)
        //{
        //    if (wpfImage.Source is BitmapSource bitmapSource)
        //    {
        //        // Define stride (bytes per row)
        //        int stride = (bitmapSource.PixelWidth * bitmapSource.Format.BitsPerPixel + 7) / 8;
        //        int size = stride * bitmapSource.PixelHeight;

        //        // Allocate unmanaged memory
        //        IntPtr ptr = Marshal.AllocHGlobal(size);

        //        try
        //        {
        //            // Copy pixels to unmanaged memory
        //            bitmapSource.CopyPixels(Int32Rect.Empty, ptr, size, stride);
        //            return ptr; // Returns pointer to the unmanaged memory containing pixel data
        //        }
        //        catch
        //        {
        //            // Free memory in case of failure
        //            Marshal.FreeHGlobal(ptr);
        //            throw;
        //        }
        //    }

        //    return IntPtr.Zero; // Return zero if image source is not a BitmapSource
        //}
    }
}
