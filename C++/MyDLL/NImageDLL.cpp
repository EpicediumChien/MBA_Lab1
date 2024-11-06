#include "NImageDLL.h"
#include "NImage.h"
#include <stdio.h>

DLL_EXPORT void* CreateNImage() {
    return new NImage();
}

DLL_EXPORT void DeleteNImage(void* nimage) {
    delete static_cast<NImage*>(nimage);
}

DLL_EXPORT bool LoadImage(void* nimage, const char* filename) {
    return static_cast<NImage*>(nimage)->loadImage(filename);
}

DLL_EXPORT int GetWidth(void* nimage) {
    return static_cast<NImage*>(nimage)->getWidth();
}

DLL_EXPORT int GetHeight(void* nimage) {
    return static_cast<NImage*>(nimage)->getHeight();
}

DLL_EXPORT int GetChannels(void* nimage) {
    return static_cast<NImage*>(nimage)->getChannels();
}

DLL_EXPORT unsigned char* GetData(void* nimage) {
    return static_cast<NImage*>(nimage)->getData();
}

DLL_EXPORT unsigned char* GetPalette(void* nimage) {
    return static_cast<NImage*>(nimage)->getPalette();
}

DLL_EXPORT bool SaveImage(void* nimage, const char* filename) {
    return static_cast<NImage*>(nimage)->saveImage(filename);
}

// This function exposes the Gaussian blur
DLL_EXPORT unsigned char* ApplyGaussianBlurImage(void* nimage, int kernelSize, double sigma) {
    // Apply Gaussian blur by calling the applyGaussianBlur method on the NImage instance
    return static_cast<NImage*>(nimage)->applyGaussianBlur(kernelSize, sigma);
}

DLL_EXPORT unsigned char* InverseImage(unsigned char* data, int width, int height, int channels) {
    if (!data) {
        // Handle the error if data is null
        printf("Error: No image data to invert.\n");
        return NULL;
    }

    // Calculate the total image size (width * height * channels)
    int imageSize = width * height * channels;

    // Invert each byte (255 - current value)
    for (int i = 0; i < imageSize; ++i) {
        data[i] = 255 - data[i];
    }

    // Return the pointer to the modified data
    return data;
}

DLL_EXPORT unsigned char* AdaptiveThresholdImage(void* nimage) {
    return static_cast<NImage*>(nimage)->adaptiveThresholding();
}

DLL_EXPORT unsigned char* RgbToGray8bit(void* nimage, int width, int height) {
    // Cast the void pointer to NImage* and call the rgbToGray8bit function
    return static_cast<NImage*>(nimage)->rgbToGray8bit(width, height);
}