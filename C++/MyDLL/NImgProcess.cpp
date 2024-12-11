#include "NImgProcess.h"
#include <corecrt_malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAX_GRAY_LEVELS 256

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
    if (data == NULL || width <= 0 || height <= 0) {
        fprintf(stderr, "Error: Invalid input parameters.\n");
        return NULL;
    }

    int rowSize = (width * 3 + 3) & ~3; // Align rows to 4-byte boundaries
    unsigned char* output = (unsigned char*)malloc(rowSize * height);
    if (output == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return NULL;
    }

    // Sobel kernels
    int SobelX[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };
    int SobelY[3][3] = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };

    // Apply Sobel filter
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            int sumXr = 0, sumYr = 0, sumXg = 0, sumYg = 0, sumXb = 0, sumYb = 0;

            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    int ny = y + ky;
                    int nx = x + kx;

                    // Clamp indices to image bounds
                    if (ny < 0 || ny >= height || nx < 0 || nx >= width) {
                        continue;
                    }

                    int idx = ny * rowSize + nx * 3;
                    int r = data[idx];
                    int g = data[idx + 1];
                    int b = data[idx + 2];

                    sumXr += SobelX[ky + 1][kx + 1] * r;
                    sumYr += SobelY[ky + 1][kx + 1] * r;
                    sumXg += SobelX[ky + 1][kx + 1] * g;
                    sumYg += SobelY[ky + 1][kx + 1] * g;
                    sumXb += SobelX[ky + 1][kx + 1] * b;
                    sumYb += SobelY[ky + 1][kx + 1] * b;
                }
            }

            // Gradient magnitudes
            int gR = (int)sqrt(sumXr * sumXr + sumYr * sumYr);
            int gG = (int)sqrt(sumXg * sumXg + sumYg * sumYg);
            int gB = (int)sqrt(sumXb * sumXb + sumYb * sumYb);

            // Clamp values to 0-255
            gR = (gR > 255) ? 255 : gR;
            gG = (gG > 255) ? 255 : gG;
            gB = (gB > 255) ? 255 : gB;

            int idx = y * rowSize + x * 3;
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
    width = width * channels + 4 - (width % 4);
    if (channels == 1) {
        return applySobel8bit(data, width, height);
    }
    else
    {
        return applySobel24bit(data, width, height);
    }
}

unsigned char* memCopy(unsigned char* data, int width, int height, int channels) {
    int size = width * height * channels;
    unsigned char* copyData = (unsigned char*)malloc(size * sizeof(unsigned char));
    if (!copyData) {
        // Handle memory allocation failure
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return NULL;
    }
    for (int i = size - 1; i >= 0; i--) {
        copyData[i] = data[i];
    }
    return copyData;
}

DLL_EXPORT unsigned char* MemCopy(unsigned char* data, int width, int height, int channels) {
    return memCopy(data, width, height, channels);
}

// Function to compute the Otsu threshold
unsigned char otsu_threshold(unsigned char* image, int width, int height) {
    int histogram[MAX_GRAY_LEVELS] = { 0 };
    int totalPixels = width * height;

    // Step 1: Compute the histogram of the image
    for (int i = 0; i < totalPixels; i++) {
        histogram[image[i]]++;
    }

    // Step 2: Calculate total sum of pixel intensities
    float totalSum = 0;
    for (int i = 0; i < MAX_GRAY_LEVELS; i++) {
        totalSum += i * histogram[i];
    }

    float sumB = 0; // Sum of the background
    int weightB = 0; // Weight of the background
    int weightF = 0; // Weight of the foreground
    float maxBetweenClassVariance = 0;
    unsigned char bestThreshold = 0;

    // Step 3: Iterate over all possible thresholds (t)
    for (int t = 0; t < MAX_GRAY_LEVELS; t++) {
        weightB += histogram[t];  // Background weight
        if (weightB == 0) continue;

        weightF = totalPixels - weightB;  // Foreground weight
        if (weightF == 0) break;

        sumB += (float)t * histogram[t];  // Sum of background intensities
        float meanB = sumB / weightB;  // Mean of background
        float meanF = (totalSum - sumB) / weightF;  // Mean of foreground

        // Step 4: Calculate the between-class variance
        float betweenClassVariance = (float)weightB * (float)weightF * (meanB - meanF) * (meanB - meanF);
        if (betweenClassVariance > maxBetweenClassVariance) {
            maxBetweenClassVariance = betweenClassVariance;
            bestThreshold = t;
        }
    }

    // Return the best threshold value
    return bestThreshold;
}

// Function to apply Otsu's binarization for 8-bit grayscale images
unsigned char* applyOtsuBinarization8Bit(unsigned char* image, int width, int height) {
    unsigned char* outputImage = (unsigned char*)malloc(width * height * sizeof(unsigned char));
    unsigned char threshold = otsu_threshold(image, width, height);

    // Apply threshold to binarize the image
    for (int i = 0; i < width * height; i++) {
        outputImage[i] = (image[i] > threshold) ? 255 : 0;
    }
    return outputImage;
}

// Function to apply Otsu's binarization for 24-bit RGB images
unsigned char* applyOtsuBinarization24Bit(unsigned char* rgbImage, int width, int height, int channels) {
    unsigned char* grayscaleImage = NULL;

    if (channels == 3) {
        // Convert RGB to grayscale
        grayscaleImage = RgbToGray8bit(rgbImage, width, height);
    }
    else {
        // If it's already grayscale
        grayscaleImage = rgbImage;
    }

    // Apply Otsu's binarization on the grayscale image
    unsigned char* outputImage = applyOtsuBinarization8Bit(grayscaleImage, width, height);

    // Free allocated memory if it's an RGB image
    if (channels == 3) {
        free(grayscaleImage);
    }
    return outputImage;
}

DLL_EXPORT unsigned char* ApplyOtsuBinarization(unsigned char* data, int width, int height, int channels) {
    return applyOtsuBinarization24Bit(data, width, height, channels);
}