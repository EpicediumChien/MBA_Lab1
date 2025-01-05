// ImageProcess.h
#pragma once

#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

#include <stdint.h>

// BMP file headers
#pragma pack(push, 1)
typedef struct {
    uint16_t fileType;
    uint32_t fileSize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offsetData;
} BMPHeader;

typedef struct {
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bitCount;
    uint32_t compression;
    uint32_t sizeImage;
    int32_t xPelsPerMeter;
    int32_t yPelsPerMeter;
    uint32_t clrUsed;
    uint32_t clrImportant;
} BMPInfoHeader;
#pragma pack(pop)

// NImage class for handling BMP images
typedef struct {
    int width;
    int height;
    int channels;
    unsigned char* data;
    unsigned char* palette;
} NImage;

#ifdef __cplusplus
extern "C" {
#endif

    // Image management
    DLL_EXPORT NImage* CreateNImage();
    DLL_EXPORT void DeleteNImage(NImage* image);
    DLL_EXPORT int LoadImage(NImage* image, const char* filename);
    DLL_EXPORT int SaveImage(NImage* image, const char* filename);

    // Image metadata access
    DLL_EXPORT int GetWidth(const NImage* image);
    DLL_EXPORT int GetHeight(const NImage* image);
    DLL_EXPORT int GetChannels(const NImage* image);
    DLL_EXPORT const unsigned char* GetData(const NImage* image);

    // Image processing functions
    DLL_EXPORT unsigned char* ApplyGaussianBlurImage(unsigned char* data, int width, int height, int channels, int kernelSize, double sigma);
    DLL_EXPORT unsigned char* ApplyOtsuBinarization(unsigned char* data, int width, int height, int channels);
    DLL_EXPORT unsigned char* InverseImage(unsigned char* data, int width, int height, int channels);
    DLL_EXPORT unsigned char* RgbToGray8bit(unsigned char* data, int width, int height);
    DLL_EXPORT unsigned char* AdaptiveThresholdImage(unsigned char* data, int width, int height);
    DLL_EXPORT unsigned char* SobelFilterImage(unsigned char* data, int width, int height, int channels);
    DLL_EXPORT unsigned char* MemCopy(unsigned char* data, int width, int height, int channels);
    DLL_EXPORT unsigned char* MidtermGaussianBlurImage(unsigned char* data, int width, int height, int channels);

    // Image detect
    DLL_EXPORT float CompareImagesWithFourierDescriptors(unsigned char* image1, int width1, int height1, unsigned char* image2, int width2, int height2);

#ifdef __cplusplus
}
#endif
