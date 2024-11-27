#pragma once

#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

extern "C" {
    // New function to apply Gaussian blur
    DLL_EXPORT unsigned char* MidtermGaussianBlurImage(unsigned char* data, int width, int height, int channels);
    DLL_EXPORT void FreeImage(unsigned char* image);
}