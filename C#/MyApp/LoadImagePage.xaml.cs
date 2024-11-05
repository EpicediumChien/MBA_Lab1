using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace MyApp
{
    /// <summary>
    /// LoadImagePage.xaml 的互動邏輯
    /// </summary>
    public partial class LoadImagePage : Window
    {
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
        public static extern void ApplyGaussianBlurImage(IntPtr nimage, int kernelSize, double sigma);

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
            IntPtr nImagePtr = CreateNImage();

            string imagePath = tempFilePath; // Update with your BMP image path
            if (LoadImage(nImagePtr, imagePath))
            {
                int width = GetWidth(nImagePtr);
                int height = GetHeight(nImagePtr);
                int channels = GetChannels(nImagePtr);
                IntPtr dataPtr = GetData(nImagePtr);

                // Copy data to a managed array
                byte[] imageData = new byte[width * height * channels];
                Marshal.Copy(dataPtr, imageData, 0, imageData.Length);

                // Create a BitmapSource from the loaded image data
                BitmapSource bitmap = BitmapSource.Create(
                    width,
                    height,
                    96, // DPI X
                    96, // DPI Y
                    System.Windows.Media.PixelFormats.Bgr24, // Assuming 24-bit color
                    null,
                    imageData,
                    width * channels);

                LoadedImage.Source = bitmap; // Set to your Image control
            }
            else
            {
                MessageBox.Show("Failed to load image.");
            }

            DeleteNImage(nImagePtr); // Clean up
            #endregion
        }

        private void OnClick_Gaussian(object sender, RoutedEventArgs e) { }

        private void OnClick_Sobel(object sender, RoutedEventArgs e) { }
    }
}
