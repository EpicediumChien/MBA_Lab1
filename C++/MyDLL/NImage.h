#define _CRT_SECURE_NO_WARNINGS
#ifndef NIMAGE_H
#define NIMAGE_H

#include <cstdint>

// Define the BMP file header structure
#pragma pack(push, 1)  // Ensure there is no padding
typedef struct {
    uint16_t fileType;     // File type, should be 'BM'
    uint32_t fileSize;     // Size of the file in bytes
    uint16_t reserved1;    // Reserved, set to 0
    uint16_t reserved2;    // Reserved, set to 0
    uint32_t offsetData;   // Offset to start of pixel data
} BMPHeader;

// Define the BMP info header structure
typedef struct {
    uint32_t size;         // Size of this header
    int32_t width;         // Width of the image
    int32_t height;        // Height of the image
    uint16_t planes;       // Number of color planes (must be 1)
    uint16_t bitCount;     // Bits per pixel (e.g., 24 for RGB, 8 for grayscale)
    uint32_t compression;  // Compression type (0 for no compression)
    uint32_t sizeImage;    // Size of the image data in bytes
    int32_t xPelsPerMeter; // Horizontal resolution (pixels per meter)
    int32_t yPelsPerMeter; // Vertical resolution (pixels per meter)
    uint32_t clrUsed;      // Number of colors in the palette
    uint32_t clrImportant; // Important colors (0 means all colors are important)
} BMPInfoHeader;
#pragma pack(pop)

class NImage {
private:
    int width;
    int height;
    int channels; // 3 for color, 1 for grayscale
    unsigned char* data;
    unsigned char* palette; // 用於存儲灰階影像的調色盤

    bool readBMP(const char* filename);

public:
    NImage();
    ~NImage();

    bool loadImage(const char* filename); // 自動辨識 BMP 格式 (彩色或灰階)
    bool saveImage(const char* filename); // Save the image as BMP

    void displayInfo() const;

    unsigned char* getData() const;
    unsigned char* getPalette() const;
    int getWidth() const;
    int getHeight() const;
    int getChannels() const; 

    void applyGaussianBlur(int kernelSize, double sigma); // New function
};

#endif