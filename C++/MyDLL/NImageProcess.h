#pragma once

#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

extern "C" {
    // New function to apply Gaussian blur
    DLL_EXPORT unsigned char* ApplyGaussianBlurImage(unsigned char* data, int width, int height, int channels, int kernelSize, double sigma);

    DLL_EXPORT unsigned char* InverseImage(unsigned char* data, int width, int height, int channels);

    DLL_EXPORT unsigned char* RgbToGray8bit(unsigned char* data, int width, int height);

    DLL_EXPORT unsigned char* AdaptiveThresholdImage(unsigned char* data, int width, int height);

}