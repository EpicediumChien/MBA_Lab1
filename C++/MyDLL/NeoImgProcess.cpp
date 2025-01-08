// Updated ImageProcess.cpp
#include "ImageProcess.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAX_GRAY_LEVELS 256

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

NImage* CreateNImage() {
    NImage* image = (NImage*)malloc(sizeof(NImage));
    image->width = 0;
    image->height = 0;
    image->channels = 0;
    image->data = NULL;
    image->palette = NULL;
    return image;
}

void DeleteNImage(NImage* image) {
    if (image->data) free(image->data);
    if (image->palette) free(image->palette);
    free(image);
}

int LoadImage(NImage* image, const char* filename) {
    FILE* file = fopen(filename, "rb"); // Updated fopen_s to fopen for GCC compatibility
    if (!file) return 0;

    unsigned char header[54];
    fread(header, sizeof(unsigned char), 54, file);

    image->width = *(int*)&header[18];
    image->height = *(int*)&header[22];
    short bitsPerPixel = *(short*)&header[28];

    if (bitsPerPixel == 24) {
        image->channels = 3;
        int rowSize = (image->width * 3 + 3) & ~3;
        image->data = (unsigned char*)malloc(rowSize * image->height);
        for (int y = 0; y < image->height; ++y) {
            fread(image->data + (image->height - y - 1) * rowSize, 1, rowSize, file);
        }
    }
    else if (bitsPerPixel == 8) {
        image->channels = 1;
        int rowSize = (image->width + 3) & ~3;
        image->palette = (unsigned char*)malloc(1024);
        fread(image->palette, 1, 1024, file);

        image->data = (unsigned char*)malloc(rowSize * image->height);
        for (int y = 0; y < image->height; ++y) {
            fread(image->data + (image->height - y - 1) * rowSize, 1, rowSize, file);
        }
    }
    else {
        fclose(file);
        return 0;
    }

    fclose(file);
    return 1;
}

int SaveImage(NImage* image, const char* filename) {
    FILE* file = fopen(filename, "wb"); // Updated fopen_s to fopen for GCC compatibility
    if (!file) return 0;

    BMPHeader bmpHeader = { 
    0x4D42, 
    static_cast<uint32_t>(54 + static_cast<uint64_t>(image->width) * image->height * image->channels), 
    0, 
    0, 
    54 
    };

    BMPInfoHeader bmpInfoHeader = { 40, image->width, image->height, 1, (unsigned short)(image->channels * 8), 0, (unsigned int)(image->width * image->height * image->channels), 0, 0, 0, 0 };

    fwrite(&bmpHeader, sizeof(bmpHeader), 1, file);
    fwrite(&bmpInfoHeader, sizeof(bmpInfoHeader), 1, file);
    fwrite(image->data, image->width * image->height * image->channels, 1, file);

    fclose(file);
    return 1;
}

int GetWidth(const NImage* image) {
    return image->width;
}

int GetHeight(const NImage* image) {
    return image->height;
}

int GetChannels(const NImage* image) {
    return image->channels;
}

const unsigned char* GetData(const NImage* image) {
    return image->data;
}
