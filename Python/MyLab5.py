import ctypes
from PIL import Image
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.image as mpimg

print("Hello Python!\n")

# Load the DLL
nimage_dll = ctypes.CDLL('../MyDLL.dll')

# Function to create a new NImage object
nimage_dll.CreateNImage.restype = ctypes.c_void_p  # Returns a void* pointer

# Function to delete the NImage object
nimage_dll.DeleteNImage.argtypes = [ctypes.c_void_p]
nimage_dll.DeleteNImage.restype = None

# Load an image from file
nimage_dll.LoadImage.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
nimage_dll.LoadImage.restype = ctypes.c_bool

# Get image width, height, and channels
nimage_dll.GetWidth.argtypes = [ctypes.c_void_p]
nimage_dll.GetWidth.restype = ctypes.c_int

nimage_dll.GetHeight.argtypes = [ctypes.c_void_p]
nimage_dll.GetHeight.restype = ctypes.c_int

nimage_dll.GetChannels.argtypes = [ctypes.c_void_p]
nimage_dll.GetChannels.restype = ctypes.c_int

# Get image data and palette pointers
nimage_dll.GetData.argtypes = [ctypes.c_void_p]
nimage_dll.GetData.restype = ctypes.POINTER(ctypes.c_ubyte)

nimage_dll.GetPalette.argtypes = [ctypes.c_void_p]
nimage_dll.GetPalette.restype = ctypes.POINTER(ctypes.c_ubyte)

# Save image to file
nimage_dll.SaveImage.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
nimage_dll.SaveImage.restype = ctypes.c_bool

# Apply Gaussian blur
nimage_dll.MidtermGaussianBlurImage.argtypes = [ctypes.POINTER(ctypes.c_ubyte), ctypes.c_int, ctypes.c_int, ctypes.c_int]
nimage_dll.MidtermGaussianBlurImage.restype = ctypes.POINTER(ctypes.c_ubyte)

# Apply Gaussian blur
nimage_dll.ApplyGaussianBlurImage.argtypes = [ctypes.POINTER(ctypes.c_ubyte), ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_double]
nimage_dll.ApplyGaussianBlurImage.restype = ctypes.POINTER(ctypes.c_ubyte)

# Invert image
nimage_dll.InverseImage.argtypes = [ctypes.POINTER(ctypes.c_ubyte), ctypes.c_int, ctypes.c_int, ctypes.c_int]
nimage_dll.InverseImage.restype = ctypes.POINTER(ctypes.c_ubyte)

# Invert image
nimage_dll.FreeImage.argtypes = [ctypes.POINTER(ctypes.c_ubyte)]
nimage_dll.FreeImage.restype = None

# Invert image
nimage_dll.ApplyOtsuBinarization.argtypes = [ctypes.POINTER(ctypes.c_ubyte), ctypes.c_int, ctypes.c_int, ctypes.c_int]
nimage_dll.ApplyOtsuBinarization.restype = ctypes.POINTER(ctypes.c_ubyte)

# Create an image instance
nimage_instance = nimage_dll.CreateNImage()

# Load an image file (provide a valid path to an image file)
filename = "../Imgs/sudoku.bmp".encode('utf-8')  # Convert filename to bytes
if nimage_dll.LoadImage(nimage_instance, filename):
    print("Image loaded successfully.")

    # Get image dimensions
    width = nimage_dll.GetWidth(nimage_instance)
    height = nimage_dll.GetHeight(nimage_instance)
    channels = nimage_dll.GetChannels(nimage_instance)
    print(f"Width: {width}, Height: {height}, Channels: {channels}")

    # Get image data (raw byte array of the image)
    image_data = nimage_dll.GetData(nimage_instance)

    # Apply the image inversion (or other processing)
    processed_data = nimage_dll.ApplyOtsuBinarization(image_data, width, height, channels)

    # Calculate row size with padding (must be a multiple of 4)
    row_size_with_padding = width * channels + (4 - (width * channels) % 4)  # padded to 4-byte boundary
    size_with_padding = row_size_with_padding * height

    # Convert processed data to a numpy array
    processed_array = ctypes.cast(processed_data, ctypes.POINTER(ctypes.c_ubyte * size_with_padding)).contents
    processed_image_data = np.frombuffer(processed_array, dtype=np.uint8)

    # Ensure correct shape for RGB or grayscale images
    if channels == 3:  # RGB image
        processed_image = processed_image_data.reshape((height, row_size_with_padding, 3))[:, :width * 3]
    else:  # Grayscale image (1 channel)
        processed_image = processed_image_data.reshape((height, row_size_with_padding))[:, :width]

    # Save using PIL (this ensures proper BMP format)
    pil_image = Image.fromarray(processed_image)
    # imgplot = plt.imshow(pil_image)
    # plt.show()
    plt.imshow(pil_image)
    img_path = "../Imgs/sudoku_midtermOtsu.bmp"
    pil_image.save(img_path)
    # Load the image  # Replace with your image path
    img = mpimg.imread(img_path)

    # Display the image
    plt.imshow(img, cmap='gray')
    plt.axis('off')  # Optional: Turn off axis
    plt.show()
    # Free the memory after processing
    nimage_dll.FreeImage(processed_data)

    print("Processed image saved successfully.")
    

# # Assuming the previous setup and function definitions are already present

# def show_image(data_pointer, width, height, channels):
#     """
#     Convert raw image data from the DLL into a format that Pillow can display.
#     Args:
#         data_pointer (ctypes.POINTER(ctypes.c_ubyte)): Pointer to the image data
#         width (int): Width of the image
#         height (int): Height of the image
#         channels (int): Number of color channels (3 for RGB, 4 for RGBA, etc.)
#     """
#     # Calculate the size and retrieve the data as a byte array
#     size = width * height * channels
#     image_data = ctypes.cast(data_pointer, ctypes.POINTER(ctypes.c_ubyte * size)).contents

#     # Convert the byte data to a format Pillow understands
#     mode = "RGB" if channels == 3 else "RGBA" if channels == 4 else "L"
#     image = Image.frombytes(mode, (width, height), bytes(image_data))

#     # Show the image
#     image.show()

# # Example of usage after applying Gaussian blur
# if nimage_dll.LoadImage(nimage_instance, filename):
#     print("Image loaded successfully.")

#     width = nimage_dll.GetWidth(nimage_instance)
#     height = nimage_dll.GetHeight(nimage_instance)
#     channels = nimage_dll.GetChannels(nimage_instance)

#     # Apply Gaussian blur
#     blurred_data_pointer = nimage_dll.ApplyGaussianBlurImage(nimage_instance, 5, 1.0)
#     if blurred_data_pointer:
#         print("Gaussian blur applied.")
        
#         # Show the blurred image
#         show_image(blurred_data_pointer, width, height, channels)
#     else:
#         print("Failed to apply Gaussian blur.")

# # Clean up by deleting the NImage instance
# nimage_dll.DeleteNImage(nimage_instance)