#include "NImage.h"
#include <iostream>
#include <fstream>
#include <cstring>

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
