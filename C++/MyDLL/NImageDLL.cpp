#include "NImageDLL.h"
#include "NImage.h"
#include <corecrt_malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

DLL_EXPORT unsigned char* RgbToGray8bit(unsigned char* data, int width, int height) {
    if (data == NULL) {
        fprintf(stderr, "Error: Invalid input data.\n");
        return NULL;
    }

    int pixelCount = width * height;

    // Allocate memory for the grayscale image (8-bit, 1 channel)
    unsigned char* grayData = (unsigned char*)malloc(pixelCount * sizeof(unsigned char));
    if (grayData == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for grayscale image.\n");
        return NULL;
    }

    // Convert each 24-bit RGB pixel to 8-bit grayscale
    for (int i = 0; i < pixelCount; i++) {
        // Each RGB pixel consists of 3 bytes (R, G, B)
        unsigned char r = data[i * 3];     // Red channel
        unsigned char g = data[i * 3 + 1]; // Green channel
        unsigned char b = data[i * 3 + 2]; // Blue channel

        // Calculate the grayscale value using the luminance formula
        unsigned char gray = (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);

        // Assign the grayscale value to the gray image data
        grayData[i] = gray;
    }

    return grayData;
}

DLL_EXPORT unsigned char* AdaptiveThresholdImage(unsigned char* data, int width, int height) {
    // channels has to be 1 for grayscale
    const int windowSize = 15; // Default window size
    const int C = 5; // Default constant to subtract from mean

    unsigned char* tempData = (unsigned char*)malloc(width * height * sizeof(unsigned char));
    if (!tempData) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return nullptr;
    }

    int halfWindowSize = windowSize / 2;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int sum = 0;
            int count = 0;

            // Calculate the mean value within the window
            for (int dy = -halfWindowSize; dy <= halfWindowSize; ++dy) {
                for (int dx = -halfWindowSize; dx <= halfWindowSize; ++dx) {
                    int ny = y + dy;
                    int nx = x + dx;

                    // Reflect edges
                    ny = ny < 0 ? -ny : (ny >= height ? 2 * height - ny - 2 : ny);
                    nx = nx < 0 ? -nx : (nx >= width ? 2 * width - nx - 2 : nx);

                    sum += data[ny * width + nx];
                    ++count;
                }
            }

            int mean = sum / count;

            // Apply threshold
            tempData[y * width + x] = (data[y * width + x] > mean - C) ? 255 : 0;
        }
    }

    memcpy(data, tempData, width * height);
    free(tempData);

    return data;
}