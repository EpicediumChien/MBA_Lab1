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
        public static extern void DeleteNImage(IntPtr nImage);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool LoadImage(IntPtr nImage, string filename);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetWidth(IntPtr nImage);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetHeight(IntPtr nImage);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetChannels(IntPtr nImage);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr GetData(IntPtr nImage);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr ApplyGaussianBlurImage(IntPtr nImage, int width, int height, int channels, int kernelSize, double sigma);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr InverseImage(IntPtr nImage, int width, int height, int channel);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr RgbToGray8bit(IntPtr nImage, int width, int height);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr AdaptiveThresholdImage(IntPtr nImage, int width, int height);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr SobelFilterImage(IntPtr nImage, int width, int height, int channel);

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
                width = width + (4 - (width * channels) % 4) % 4;
                height = GetHeight(loadedImage);
                channels = GetChannels(loadedImage);
                // Real width
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

        private void OnClick_Gaussian(object sender, RoutedEventArgs e) 
        {
            IntPtr imagePtr = GetIntPtrFromImageSource(LoadedImage.Source);
            IntPtr dataPtr = ApplyGaussianBlurImage(imagePtr, width, height, channels, 7, 1.0);
            if (dataPtr != IntPtr.Zero)
            {
                ShowIntPtrOnImage(dataPtr);
            }
            else
            {
                MessageBox.Show("Failed to load image.");
            }
        }

        private void OnClick_Sobel(object sender, RoutedEventArgs e) 
        { 
            IntPtr imagePtr = GetIntPtrFromImageSource(LoadedImage.Source);
            IntPtr dataPtr = SobelFilterImage(imagePtr, width, height, channels);
            if (dataPtr != IntPtr.Zero)
            {
                ShowIntPtrOnImage(dataPtr);
            }
            else
            {
                MessageBox.Show("Failed to load image.");
            }
        }

        private void OnClick_Inverse(object sender, RoutedEventArgs e)
        {
            // Copy the image from loaded ptr
            IntPtr imagePtr = GetIntPtrFromImageSource(LoadedImage.Source);//CopyImageToNewIntPtr(loadedImageIntPtr, stride * height);
            IntPtr dataPtr = InverseImage(imagePtr, width, height, channels);
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
            // Copy the image from loaded ptr
            IntPtr imagePtr = GetIntPtrFromImageSource(LoadedImage.Source);//CopyImageToNewIntPtr(loadedImageIntPtr, stride * height);
            IntPtr dataPtr = RgbToGray8bit(imagePtr, width, height);
            channels = 1;
            stride = (width * channels + 3) & ~3;
            width = stride;
            if (dataPtr != IntPtr.Zero)
            {
                IntPtr thresholdPtr = AdaptiveThresholdImage(dataPtr, width, height);
                // Copy data to a managed array
                ShowIntPtrOnImage(thresholdPtr);
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

        public IntPtr CopyImageToNewIntPtr(IntPtr sourcePtr, int byteSize)
        {
            // Allocate a new block of memory with the specified byte size
            IntPtr destinationPtr = Marshal.AllocHGlobal(byteSize);

            try
            {
                // Copy data from sourcePtr to destinationPtr
                byte[] buffer = new byte[byteSize];
                Marshal.Copy(sourcePtr, buffer, 0, byteSize); // Copy data from sourcePtr to managed buffer
                Marshal.Copy(buffer, 0, destinationPtr, byteSize); // Copy data from buffer to destinationPtr
            }
            catch
            {
                // Free the allocated memory in case of any errors
                Marshal.FreeHGlobal(destinationPtr);
                throw;
            }

            return destinationPtr; // Return the new pointer to the copied image data
        }

        public IntPtr GetIntPtrFromImageSource(ImageSource imageSource)
        {
            if (imageSource is BitmapSource bitmapSource)
            {
                // Convert to a WritableBitmap to access the BackBuffer
                var writableBitmap = new WriteableBitmap(bitmapSource);

                // Lock the WritableBitmap to access the BackBuffer
                writableBitmap.Lock();

                IntPtr ptr = writableBitmap.BackBuffer;

                // Unlock after getting the pointer (keeps the image data in memory)
                writableBitmap.Unlock();

                return ptr;
            }
            else
            {
                throw new ArgumentException("ImageSource must be a BitmapSource to retrieve an IntPtr.");
            }
        }
    }
}
