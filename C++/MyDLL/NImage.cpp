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

    unsigned char header[54];
    fread(header, sizeof(unsigned char), 54, file); // 讀取 BMP 標頭

    width = *(int*)&header[18];
    height = *(int*)&header[22];
    short bitsPerPixel = *(short*)&header[28];

    if (bitsPerPixel == 24) {
        channels = 3; // 24 位元彩色影像
        int padding = (4 - (width * channels) % 4) % 4;
        int rowSize = width * channels + padding;

        data = (unsigned char*)malloc(rowSize * height);
        if (!data) {
            fclose(file);
            return false;
        }

        fread(data, sizeof(unsigned char), rowSize * height, file);
    }
    else if (bitsPerPixel == 8) {
        channels = 1; // 8 位元灰階影像

        // 讀取調色盤
        palette = (unsigned char*)malloc(256 * 4); // 每個調色盤條目有 4 個位元組 (B, G, R, 0)
        fread(palette, sizeof(unsigned char), 256 * 4, file);

        int padding = (4 - (width * channels) % 4) % 4;
        int rowSize = width * channels + padding;

        data = (unsigned char*)malloc(rowSize * height);
        if (!data) {
            fclose(file);
            return false;
        }

        fread(data, sizeof(unsigned char), rowSize * height, file);
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

unsigned char* NImage::adaptiveThresholding() const {
    if (channels != 1) {
        std::cerr << "Error: Adaptive thresholding works only with grayscale images." << std::endl;
        return nullptr;
    }

    const int windowSize = 15; // Default window size
    const int C = 5; // Default constant to subtract from mean

    unsigned char* tempData = (unsigned char*)malloc(width * height * sizeof(unsigned char));
    if (!tempData) {
        std::cerr << "Error: Memory allocation failed." << std::endl;
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

unsigned char* NImage::rgbToGray8bit(int width, int height) const {
    if (data == NULL) {
        fprintf(stderr, "Error: Invalid input data.\n");
        return nullptr;
    }
    int pixelCount = width * height;
    int isGray = 1;  // Flag to check if the image is grayscale

    // Check if the input data is already grayscale (1 byte per pixel)
    for (int i = 0; i < pixelCount; i++) {
        if (data[i * 3] != data[i * 3 + 1] || data[i * 3] != data[i * 3 + 2]) {
            // If R, G, and B values are not the same, it's not a grayscale image
            isGray = 0;
            break;
        }
    }

    // If it's already a grayscale image, do nothing
    if (isGray) {
        return nullptr;
    }
    // Allocate memory for the grayscale image (8-bit, 1 channel)
    unsigned char* grayData = (unsigned char*)malloc(pixelCount * sizeof(unsigned char));

    // Otherwise, process the 24-bit RGB image into an 8-bit grayscale image.
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