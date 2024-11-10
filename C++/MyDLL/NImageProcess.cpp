#include "NImageProcess.h"
#include <corecrt_malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

// Generate a Gaussian kernel in pure C
float* generateGaussianKernel(int kernelSize, double sigma) {
    float* kernel = (float*)malloc(kernelSize * sizeof(float));
    int halfSize = kernelSize / 2;
    float sum = 0.0f;

    for (int i = -halfSize; i <= halfSize; ++i) {
        float value = exp(-(i * i) / (2 * sigma * sigma)) / (sqrt(2 * M_PI) * sigma);
        kernel[i + halfSize] = value;
        sum += value;
    }

    // Normalize the kernel
    for (int i = 0; i < kernelSize; ++i) {
        kernel[i] /= sum;
    }

    return kernel;
}

// Apply a 1D Gaussian blur along one dimension
void applyGaussianBlur1D(unsigned char* data, int width, int height, int channels, float* kernel, int kernelSize, int horizontal) {
    int halfSize = kernelSize / 2;
    unsigned char* tempData = (unsigned char*)malloc(width * height * channels * sizeof(unsigned char));

    // Process each pixel
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            for (int c = 0; c < channels; ++c) {
                float sum = 0.0f;

                // Apply the kernel
                for (int k = -halfSize; k <= halfSize; ++k) {
                    int xk = horizontal ? x + k : x;
                    int yk = horizontal ? y : y + k;

                    // Reflect edges
                    xk = xk < 0 ? -xk : (xk >= width ? 2 * width - xk - 2 : xk);
                    yk = yk < 0 ? -yk : (yk >= height ? 2 * height - yk - 2 : yk);

                    sum += data[(yk * width + xk) * channels + c] * kernel[k + halfSize];
                }

                tempData[(y * width + x) * channels + c] = (unsigned char)sum;
            }
        }
    }

    // Copy the blurred data back and free tempData
    memcpy(data, tempData, width * height * channels);
    free(tempData);
}

// This function exposes the Gaussian blur
DLL_EXPORT unsigned char* ApplyGaussianBlurImage(unsigned char* data, int width, int height, int channels, int kernelSize, double sigma) {
    // Generate the Gaussian kernel
    float* kernel = generateGaussianKernel(kernelSize, sigma);

    // Apply Gaussian blur in horizontal and then vertical direction
    applyGaussianBlur1D(data, width, height, channels, kernel, kernelSize, 1);  // Horizontal pass
    applyGaussianBlur1D(data, width, height, channels, kernel, kernelSize, 0); // Vertical pass

    // Free the kernel memory
    free(kernel);

    return data;
}