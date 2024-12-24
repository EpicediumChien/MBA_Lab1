using Microsoft.Win32;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace MyApp2
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        #region Properties
        private IntPtr loadedImageIntPtr;
        int width, height, channels, stride;
        private static string tempFilePath = string.Empty;
        #endregion
        // Load the DLL functions
        private const string DllName = $"../../../../../MyDLL.dll";
        // Import DLL functions
        [StructLayout(LayoutKind.Sequential)]
        public struct ObjectData
        {
            public int Index;
            public int RunCount;
            public int Area;
        }

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr CreateNImage();

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

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "ProcessImage_Asm2")]
        public static extern IntPtr ProcessImage_Asm2(IntPtr data, int width, int height, int channels, out int objectCount, out IntPtr processedImage);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void FreeProcessedImage(IntPtr image);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void FreeObjectData(IntPtr objects);

        public MainWindow()
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

        private void OnClick_RunAsm2(object sender, RoutedEventArgs e)
        {
            // Marshal the ObjectData array
            List<ObjectData> objects = new List<ObjectData>();
            int objectCount;
            IntPtr processedImagePtr;
            IntPtr imagePtr = GetIntPtrFromImageSource(LoadedImage.Source);
            IntPtr result = ProcessImage_Asm2(imagePtr, width, height, channels, out objectCount, out processedImagePtr);
            if (result != IntPtr.Zero)
            {
                try
                {
                    // Display processed image
                    ShowIntPtrOnImage(processedImagePtr);

                    string labelTxt = string.Empty;
                    for (int i = 0; i < objectCount; i++)
                    {
                        // Marshal each object from the pointer
                        ObjectData obj = Marshal.PtrToStructure<ObjectData>(IntPtr.Add(result, i * Marshal.SizeOf<ObjectData>()));
                        labelTxt += string.Format($"idx: {obj.Index}, runcount: {obj.RunCount}, area: {obj.Area}\n");
                    }
                    // LabelResult.Content = labelTxt;
                    // Show the results in a dialog box
                    ResultDialog dialog = new ResultDialog(labelTxt);
                    dialog.ShowDialog(); // Open dialog as a modal window
                }
                finally
                {
                    // Free native memory
                    FreeProcessedImage(processedImagePtr); // Free the processed image
                    FreeObjectData(result);               // Free object data
                }
            }
            else
            {
                MessageBox.Show("Failed to load image.");
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