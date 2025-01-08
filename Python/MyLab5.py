import ctypes
from PIL import Image
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.image as mpimg

print("Hello Python!\n")

# Define ObjectData structure
class ObjectData(ctypes.Structure):
    _fields_ = [
        ("Index", ctypes.c_int),
        ("Area", ctypes.c_int),
        ("Perimeter", ctypes.c_int),
        ("CenterX", ctypes.c_float),
        ("CenterY", ctypes.c_float),
        ("Diameter", ctypes.c_float),
    ]

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

# Save image to file
nimage_dll.SaveImage.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
nimage_dll.SaveImage.restype = ctypes.c_bool

# ProcessImage_Asm2 function
nimage_dll.ProcessImage_Asm2.argtypes = [
    ctypes.POINTER(ctypes.c_ubyte),  # image data
    ctypes.c_int,                   # width
    ctypes.c_int,                   # height
    ctypes.c_int,                   # channels
    ctypes.POINTER(ctypes.c_int),   # object count
    ctypes.POINTER(ctypes.POINTER(ctypes.c_ubyte))  # processed image pointer
]
nimage_dll.ProcessImage_Asm2.restype = ctypes.POINTER(ObjectData)  # Return ObjectData pointer

# Create an image instance
nimage_instance = nimage_dll.CreateNImage()

# Load an image file (provide a valid path to an image file)
filename = "../Imgs/Asm2_Images/reference.bmp".encode('utf-8')  # Convert filename to bytes
if nimage_dll.LoadImage(nimage_instance, filename):
    print("Image loaded successfully.")

    # Get image dimensions
    width = nimage_dll.GetWidth(nimage_instance)
    height = nimage_dll.GetHeight(nimage_instance)
    channels = nimage_dll.GetChannels(nimage_instance)
    # print(f"Width: {width}, Height: {height}, Channels: {channels}")

    # Calculate row size with padding (must be a multiple of 4)
    row_size_with_padding = width * channels + (4 - (width * channels) % 4)  # padded to 4-byte boundary
    size_with_padding = row_size_with_padding * height

    # Get image data (raw byte array of the image)
    image_data = nimage_dll.GetData(nimage_instance)

    objct_count = ctypes.c_int(0)
    processedImagePtr = ctypes.POINTER(ctypes.c_ubyte)()
    processed_data = nimage_dll.ProcessImage_Asm2(
        image_data, width, height, channels, ctypes.byref(objct_count), ctypes.byref(processedImagePtr)
    )

    # Calculate row size with padding (must be a multiple of 4)
    row_size_with_padding = (width * 3 + 3) & ~3  # Each pixel has 3 bytes (RGB)
    size_with_padding = row_size_with_padding * height

    # Process the image data
    if processedImagePtr:
        # Cast the processed image pointer to the appropriate size
        processed_array = ctypes.cast(
            processedImagePtr, ctypes.POINTER(ctypes.c_ubyte * size_with_padding)
        ).contents
        processed_image_data = np.array(processed_array, dtype=np.uint8)

        # Reshape the array to match the image dimensions
        processed_image = processed_image_data.reshape((height, row_size_with_padding, 3))[:, :width * 3]

        # Convert to a PIL image for saving
        pil_image = Image.fromarray(processed_image, 'RGB')  # Create PIL image in RGB mode
        img_path = "../Imgs/test.bmp"
        pil_image.save(img_path)

        # Display the image using Matplotlib
        img = mpimg.imread(img_path)
        plt.imshow(img)
        plt.axis('off')  # Turn off axis
        plt.show()

        # Free the memory allocated for the processed image
        nimage_dll.FreeProcessedImage(processedImagePtr)
        print("Processed image saved successfully.")

