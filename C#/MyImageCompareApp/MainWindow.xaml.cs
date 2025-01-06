using Microsoft.Win32;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Media3D;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace MyImageCompareApp
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        #region Properties
        private Queue<IntPtr> loadedImageIntPtrs;
        int sourceWidth, sourceHeight, sourceChannels,
             targetWidth, targetHeight, targetChannels;
        private static string tempFilePath = string.Empty;
        #endregion
        // Load the DLL functions
        private const string DllName = $"../../../../../MyDLL.dll";
        // Import DLL functions
        [StructLayout(LayoutKind.Sequential)]
        public struct ObjectData
        {
            public int Index;
            public int Area;
            public int Perimeter;
            public float CenterX;
            public float CenterY;
            public float Diameter;
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

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void FreeProcessedImage(IntPtr image);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void FreeObjectData(IntPtr objects);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr ProcessImageWithChainCode(IntPtr data, int width, int height, int channels);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern float CompareImagesWithFourierDescriptors(IntPtr sourceImage, int sourceWidth, int sourceHeight, IntPtr targetImage, int targetWidth, int targetHeight);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr TransferBinarizeImage(IntPtr image, int width, int height, int channel = 3);

        public MainWindow()
        {
            InitializeComponent();
            loadedImageIntPtrs = new Queue<nint> { };
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
                bool shiftFlag = false;
                if(loadedImageIntPtrs.Count > 0) shiftFlag = true;
                if (shiftFlag)
                {
                    targetWidth = sourceWidth;
                    targetHeight = sourceHeight;
                    targetChannels = sourceChannels;
                }
                // Only on load image
                sourceChannels = GetChannels(loadedImage);
                sourceWidth = GetWidth(loadedImage);
                sourceHeight = GetHeight(loadedImage);
                if (loadedImageIntPtrs.Count >= 2)
                {
                    IntPtr oldDataPtr = loadedImageIntPtrs.Dequeue();
                    FreeProcessedImage(oldDataPtr);
                }


                loadedImageIntPtrs.Enqueue(GetData(loadedImage));

                ShowIntPtrOnImage(LoadedImage1, loadedImageIntPtrs.Last(), sourceWidth, sourceHeight, sourceChannels);
                if(loadedImageIntPtrs.Count == 2) ShowIntPtrOnImage(LoadedImage2, loadedImageIntPtrs.First(), targetWidth, targetHeight, targetChannels);
            }
            else
            {
                MessageBox.Show("Failed to load image.");
            }
            #endregion
        }

        private void OnClick_RunCompare(object sender, RoutedEventArgs e)
        {
            if (LoadedImage1.Source != null && LoadedImage2.Source != null)
            {
                IntPtr sourceImagePtr = GetIntPtrFromImageSource(LoadedImage1.Source);
                if (sourceChannels != 1) sourceImagePtr = TransferBinarizeImage(sourceImagePtr, sourceWidth, sourceHeight);
                IntPtr targetImagePtr = GetIntPtrFromImageSource(LoadedImage2.Source);
                if (targetChannels != 1) targetImagePtr = TransferBinarizeImage(targetImagePtr, targetWidth, targetHeight);
                float similarity = CompareImagesWithFourierDescriptors(sourceImagePtr, sourceWidth, sourceHeight, targetImagePtr, targetWidth, targetHeight);
                MessageBox.Show($"The similarity is: {similarity}");
            }
            else
            {
                MessageBox.Show("Failed to compare images.");
            }
        }

        private void ShowIntPtrOnImage(Image targetImage, IntPtr imgSource, int width, int height, int channel)
        {
            if (imgSource == IntPtr.Zero)
            {
                MessageBox.Show("Image source is null.");
                return;
            }

            if (width <= 0 || height <= 0)
            {
                MessageBox.Show("Invalid image dimensions.");
                return;
            }

            int channelsToUse = channel;
            PixelFormat pixelFormat = channelsToUse == 1 ? PixelFormats.Gray8 : PixelFormats.Bgr24;

            int adjustedStride = (width * channelsToUse + 3) & ~3;
            byte[] imageData = new byte[adjustedStride * height];

            try
            {
                Marshal.Copy(imgSource, imageData, 0, imageData.Length);

                BitmapSource bitmap = BitmapSource.Create(
                    width,
                    height,
                    96, // DPI X
                    96, // DPI Y
                    pixelFormat,
                    null,
                    imageData,
                    adjustedStride);

                targetImage.Source = bitmap;
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error creating image: {ex.Message}");
            }
            finally {
                Debug.WriteLine($"Width: {width}, Height: {height}, Channels: {channel}, Stride: {adjustedStride}");
            }
        }

        public IntPtr GetIntPtrFromImageSource(ImageSource imageSource)
        {
            if (imageSource == null)
            {
                throw new ArgumentNullException(nameof(imageSource), "ImageSource cannot be null.");
            }

            if (!(imageSource is BitmapSource bitmapSource))
            {
                throw new ArgumentException("ImageSource must be a BitmapSource to retrieve an IntPtr.");
            }

            if (bitmapSource != null)
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

        // Function to copy image data to a new memory block
        private IntPtr CopyImageData(IntPtr sourceData, int dataSize)
        {
            // Create a byte array to hold the data from the source pointer
            byte[] buffer = new byte[dataSize];

            // Copy the unmanaged data from the source pointer to the managed byte array
            Marshal.Copy(sourceData, buffer, 0, dataSize);

            // Allocate unmanaged memory for the copied data
            IntPtr newData = Marshal.AllocHGlobal(dataSize);

            // Copy the data from the managed byte array to the newly allocated unmanaged memory
            Marshal.Copy(buffer, 0, newData, dataSize);

            return newData;
        }
    }
}