#include "NImageProcess.h"
#include <corecrt_malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

void applyGaussianBlur1D(unsigned char* data, int width, int height, int channels, float* kernel, int kernelSize, int horizontal) {
    if (!data || !kernel || kernelSize <= 0 || width <= 0 || height <= 0 || channels <= 0) {
        printf("Error: Invalid input parameters.\n");
        return;
    }

    int halfSize = kernelSize / 2;
    unsigned char* tempData = (unsigned char*)malloc(width * height * channels * sizeof(unsigned char));
    if (!tempData) {
        printf("Error: Memory allocation failed.\n");
        return;
    }

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

                    // Check bounds
                    if (xk >= 0 && xk < width && yk >= 0 && yk < height) {
                        sum += data[(yk * width + xk) * channels + c] * kernel[k + halfSize];
                    }
                }

                // Clamp sum to valid byte range and store in tempData
                int index = (y * width + x) * channels + c;
                tempData[index] = (unsigned char)(sum < 0 ? 0 : (sum > 255 ? 255 : sum));
            }
        }
    }

    // Copy the blurred data back to original and free tempData
    memcpy(data, tempData, width * height * channels);
    free(tempData);
}

// Main function to apply Gaussian blur
unsigned char* applyGaussianBlur(unsigned char* data, int width, int height, int channels, int kernelSize, double sigma) {
    // Generate the Gaussian kernel
    float* kernel = generateGaussianKernel(kernelSize, sigma);

    // Apply Gaussian blur in horizontal and then vertical direction
    applyGaussianBlur1D(data, width, height, channels, kernel, kernelSize, 1);  // Horizontal pass
    applyGaussianBlur1D(data, width, height, channels, kernel, kernelSize, 0); // Vertical pass

    // Free the kernel memory
    free(kernel);
    return data;
}

// Sobel filter kernels for horizontal and vertical edges
const int SobelX[3][3] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}
};

const int SobelY[3][3] = {
    {-1, -2, -1},
    { 0,  0,  0},
    { 1,  2,  1}
};

// Function to apply Sobel filter on an 8-bit grayscale image
unsigned char* applySobel8bit(unsigned char* data, int width, int height) {
    int gx, gy, g;
    int sumX, sumY;

    unsigned char* output = (unsigned char*)malloc(width * height * sizeof(unsigned char));
    // Iterate over each pixel in the image (skipping edges)
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            sumX = 0;
            sumY = 0;

            // Apply Sobel kernel for both x and y gradients
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    int pixel = data[(y + ky) * width + (x + kx)];
                    sumX += SobelX[ky + 1][kx + 1] * pixel;
                    sumY += SobelY[ky + 1][kx + 1] * pixel;
                }
            }

            // Calculate the gradient magnitude
            g = (int)sqrt(sumX * sumX + sumY * sumY);
            if (g > 255) g = 255; // Clamp the value to 255 if it exceeds

            output[y * width + x] = (unsigned char)g;
        }
    }
    return output;
}

// Function to apply Sobel filter on a 24-bit color image
unsigned char* applySobel24bit(unsigned char* data, int width, int height) {
    int gx, gy, gR, gG, gB;
    int sumXr, sumYr, sumXg, sumYg, sumXb, sumYb;

    unsigned char* output = (unsigned char*)malloc(width * height * 3 * sizeof(unsigned char));
    // Iterate over each pixel in the image (skipping edges)
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            sumXr = sumYr = sumXg = sumYg = sumXb = sumYb = 0;

            // Apply Sobel kernel for both x and y gradients
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    int idx = ((y + ky) * width + (x + kx)) * 3; // 3 channels for RGB
                    int r = data[idx];     // Red channel
                    int g = data[idx + 1]; // Green channel
                    int b = data[idx + 2]; // Blue channel

                    sumXr += SobelX[ky + 1][kx + 1] * r;
                    sumYr += SobelY[ky + 1][kx + 1] * r;
                    sumXg += SobelX[ky + 1][kx + 1] * g;
                    sumYg += SobelY[ky + 1][kx + 1] * g;
                    sumXb += SobelX[ky + 1][kx + 1] * b;
                    sumYb += SobelY[ky + 1][kx + 1] * b;
                }
            }

            // Calculate the gradient magnitudes for each channel
            gR = (int)sqrt(sumXr * sumXr + sumYr * sumYr);
            gG = (int)sqrt(sumXg * sumXg + sumYg * sumYg);
            gB = (int)sqrt(sumXb * sumXb + sumYb * sumYb);

            // Clamp the values to be within the 0-255 range
            if (gR > 255) gR = 255;
            if (gG > 255) gG = 255;
            if (gB > 255) gB = 255;

            // Set the output pixel values
            int idx = (y * width + x) * 3;
            output[idx] = (unsigned char)gR;
            output[idx + 1] = (unsigned char)gG;
            output[idx + 2] = (unsigned char)gB;

        }
    }
    return output;
}


// This function exposes the Gaussian blur
DLL_EXPORT unsigned char* ApplyGaussianBlurImage(unsigned char* data, int width, int height, int channels, int kernelSize, double sigma) {
    // Apply Gaussian blur by calling the applyGaussianBlur method on the NImage instance
    return applyGaussianBlur(data, width, height, channels, kernelSize, sigma);
}

DLL_EXPORT unsigned char* InverseImage(unsigned char* data, int width, int height, int channels) {
    // Calculate the total image size (width * height * channels)
    int imageSize = width * height * channels;

    // Allocate memory for the inverted data
    unsigned char* invertedData = (unsigned char*)malloc(imageSize);
    if (!invertedData) {
        printf("Error: Memory allocation failed.\n");
        return NULL;
    }

    // Invert each byte (255 - current value)
    for (int i = 0; i < imageSize; ++i) {
        invertedData[i] = 255 - data[i];
    }

    // Return the pointer to the modified data
    return invertedData;
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

DLL_EXPORT unsigned char* SobelFilterImage(unsigned char* data, int width, int height, int channels)
{
    if (channels == 1) {
        return applySobel8bit(data, width, height);
    }
    else
    {
        return applySobel24bit(data, width, height);
    }
}
