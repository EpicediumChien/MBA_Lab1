#include "NImage.h"
#include "NImageDLL.h"
#include <math.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define BMP_HEADER_SIZE 54


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

NImage::NImage() : width(0), height(0), channels(0), data(NULL), palette(NULL) {}

NImage::~NImage() {
    if (data) {
        free(data);
    }
    if (palette) {
        free(palette);
    }
}

bool NImage::readBMP(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to open BMP file: %s\n", filename);
        return false;
    }

    unsigned char header[BMP_HEADER_SIZE];
    fread(header, sizeof(unsigned char), BMP_HEADER_SIZE, file); // Read BMP header

    // Read image dimensions and bits per pixel
    width = *(int*)&header[18];
    height = *(int*)&header[22];
    short bitsPerPixel = *(short*)&header[28];

    if (bitsPerPixel == 24) {
        channels = 3; // 24-bit color image
        int padding = (4 - (width * channels) % 4) % 4; // Row padding
        int rowSize = width * channels + padding;

        // Allocate memory for image data
        data = (unsigned char*)malloc(rowSize * height);
        if (!data) {
            fclose(file);
            return false;
        }

        // Read pixel data
        for (int y = height - 1; y >= 0; --y) { // Reverse reading order (bottom to top)
            fread(data + y * rowSize, sizeof(unsigned char), rowSize, file);
        }
    }
    else if (bitsPerPixel == 8) {
        channels = 1; // 8-bit grayscale image

        // Read color palette (256 colors for 8-bit BMP)
        palette = (unsigned char*)malloc(256 * 4); // 256 entries with 4 bytes each (B, G, R, 0)
        fread(palette, sizeof(unsigned char), 256 * 4, file);

        int padding = (4 - (width * channels) % 4) % 4; // Row padding
        int rowSize = width * channels + padding;

        // Allocate memory for grayscale data
        data = (unsigned char*)malloc(rowSize * height);
        if (!data) {
            fclose(file);
            return false;
        }

        // Read pixel data (reverse reading order for 8-bit grayscale)
        for (int y = height - 1; y >= 0; --y) { // Reverse reading order (bottom to top)
            fread(data + y * rowSize, sizeof(unsigned char), rowSize, file);
        }
    }
    else {
        printf("Unsupported BMP format: %d bpp\n", bitsPerPixel);
        fclose(file);
        return false;
    }

    fclose(file);
    return true;
}

bool NImage::loadImage(const char* filename) {
    return readBMP(filename);
}

void NImage::displayInfo() const {
    printf("Width: %d, Height: %d, Channels: %d\n", width, height, channels);
    if (palette) {
        printf("Palette exists with 256 entries.\n");
    }
}

unsigned char* NImage::getData() const {
    return data;
}

unsigned char* NImage::getPalette() const {
    return palette;
}

int NImage::getWidth() const {
    return width;
}

int NImage::getHeight() const {
    return height;
}

int NImage::getChannels() const {
    return channels;
}

bool NImage::saveImage(const char* filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Cannot open file for writing.\n";
        return false;
    }

    // Prepare BMP file header
    BMPHeader bmpHeader;
    bmpHeader.fileType = 0x4D42; // 'BM'
    bmpHeader.fileSize = sizeof(BMPHeader) + sizeof(BMPInfoHeader) + (width * height * channels);
    bmpHeader.reserved1 = 0;
    bmpHeader.reserved2 = 0;
    bmpHeader.offsetData = sizeof(BMPHeader) + sizeof(BMPInfoHeader);

    // Prepare BMP info header
    BMPInfoHeader bmpInfoHeader;
    std::memset(&bmpInfoHeader, 0, sizeof(BMPInfoHeader));
    bmpInfoHeader.size = sizeof(BMPInfoHeader);
    bmpInfoHeader.width = width;
    bmpInfoHeader.height = height;
    bmpInfoHeader.planes = 1;
    bmpInfoHeader.bitCount = (channels == 3) ? 24 : 8;
    bmpInfoHeader.compression = 0;
    bmpInfoHeader.sizeImage = width * height * channels;

    // Write headers to file
    file.write(reinterpret_cast<const char*>(&bmpHeader), sizeof(bmpHeader));
    file.write(reinterpret_cast<const char*>(&bmpInfoHeader), sizeof(bmpInfoHeader));

    // Write palette (if grayscale image)
    if (channels == 1 && palette) {
        file.write(reinterpret_cast<const char*>(palette), 256 * 4);
    }

    // Write image data (inverting row order for BMP format)
    for (int i = height - 1; i >= 0; --i) {
        file.write(reinterpret_cast<const char*>(data + (width * channels * i)), width * channels);
    }

    file.close();
    return true;
}
