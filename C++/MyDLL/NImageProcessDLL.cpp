#include "NImageProcessDLL.h"
#include <corecrt_malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define OFFSET 2 // For a 5x5 kernel

int kernel[5][5] = {
    { 0,  0, -1,  0,  0},
    { 0, -1, -2, -1,  0},
    {-1, -2, 16, -2, -1},
    { 0, -1, -2, -1,  0},
    { 0,  0, -1,  0,  0}
};

// Convolution function for 8-bit grayscale image
void applyKernelGrayscale(unsigned char* image, int width, int height, unsigned char* output) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int sum = 0;

            // Apply the kernel
            for (int ky = -OFFSET; ky <= OFFSET; ky++) {
                for (int kx = -OFFSET; kx <= OFFSET; kx++) {
                    int pixelY = y + ky;
                    int pixelX = x + kx;

                    // Check boundaries
                    if (pixelY >= 0 && pixelY < height && pixelX >= 0 && pixelX < width) {
                        int pixelIndex = pixelY * width + pixelX;
                        sum += image[pixelIndex] * kernel[ky + OFFSET][kx + OFFSET];
                    }
                }
            }

            // Clip result to 0-255 and store in output
            int outputIndex = y * width + x;
            output[outputIndex] = (sum < 0) ? 0 : ((sum > 255) ? 255 : sum);
        }
    }
}

// Convolution function for 24-bit RGB image
void applyKernelRGB(unsigned char* image, int width, int height, unsigned char* output) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int r_sum = 0, g_sum = 0, b_sum = 0;

            // Apply the kernel to each color channel (RGB)
            for (int ky = -OFFSET; ky <= OFFSET; ky++) {
                for (int kx = -OFFSET; kx <= OFFSET; kx++) {
                    int pixelY = y + ky;
                    int pixelX = x + kx;

                    // Check boundaries
                    if (pixelY >= 0 && pixelY < height && pixelX >= 0 && pixelX < width) {
                        int pixelIndex = (pixelY * width + pixelX) * 3; // 3 channels (RGB)

                        r_sum += image[pixelIndex] * kernel[ky + OFFSET][kx + OFFSET];         // Red
                        g_sum += image[pixelIndex + 1] * kernel[ky + OFFSET][kx + OFFSET];     // Green
                        b_sum += image[pixelIndex + 2] * kernel[ky + OFFSET][kx + OFFSET];     // Blue
                    }
                }
            }

            // Clip result to 0-255 and store in output for each channel
            int outputIndex = (y * width + x) * 3; // 3 channels (RGB)
            output[outputIndex] = (r_sum < 0) ? 0 : ((r_sum > 255) ? 255 : r_sum);   // Red
            output[outputIndex + 1] = (g_sum < 0) ? 0 : ((g_sum > 255) ? 255 : g_sum); // Green
            output[outputIndex + 2] = (b_sum < 0) ? 0 : ((b_sum > 255) ? 255 : b_sum); // Blue
        }
    }
}

// Function to apply Gaussian Blur using the LoG kernel to a bitmap image
unsigned char* applyLaplacianGaussianBlur(unsigned char* image, int width, int height, int channels) {
    width = width + 4 - width * channels % 4;
    unsigned char* output = (unsigned char*)malloc(width * height * channels * sizeof(unsigned char));

    if (channels == 1) {
        // Process as grayscale
        applyKernelGrayscale(image, width, height, output);
    }
    else {
        // Process as RGB
        applyKernelRGB(image, width, height, output);
    }

    return output;
}

// This function exposes the Gaussian blur
DLL_EXPORT unsigned char* MidtermGaussianBlurImage(unsigned char* data, int width, int height, int channels) {
    // Apply Gaussian blur by calling the applyGaussianBlur method on the NImage instance
    return applyLaplacianGaussianBlur(data, width, height, channels);
}

// Free function (useful in C)
DLL_EXPORT void FreeImage(unsigned char* image) {
    free(image);
}
