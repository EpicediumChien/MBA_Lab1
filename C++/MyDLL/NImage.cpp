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

// Main function to apply Gaussian blur
void InternalApplyGaussianBlur(unsigned char* data, int width, int height, int channels, int kernelSize, double sigma) {
    // Generate the Gaussian kernel
    float* kernel = generateGaussianKernel(kernelSize, sigma);

    // Apply Gaussian blur in horizontal and then vertical direction
    applyGaussianBlur1D(data, width, height, channels, kernel, kernelSize, 1);  // Horizontal pass
    applyGaussianBlur1D(data, width, height, channels, kernel, kernelSize, 0); // Vertical pass

    // Free the kernel memory
    free(kernel);
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

unsigned char* NImage::applyGaussianBlur(int kernelSize, double sigma) {
    InternalApplyGaussianBlur(data, width, height, channels, kernelSize, sigma);
    return data;
}

