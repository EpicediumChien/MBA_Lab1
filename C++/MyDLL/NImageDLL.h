#ifndef NIMAGEDLL_H
#define NIMAGEDLL_H

#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

extern "C" {
    DLL_EXPORT void* CreateNImage();

    DLL_EXPORT void DeleteNImage(void* nimage);

    DLL_EXPORT bool LoadImage(void* nimage, const char* filename);

    DLL_EXPORT int GetWidth(void* nimage);

    DLL_EXPORT int GetHeight(void* nimage);

    DLL_EXPORT int GetChannels(void* nimage);

    DLL_EXPORT unsigned char* GetData(void* nimage);

    DLL_EXPORT unsigned char* GetPalette(void* nimage);

    DLL_EXPORT bool SaveImage(void* nimage, const char* filename);

    // New function to apply Gaussian blur
    DLL_EXPORT unsigned char* ApplyGaussianBlurImage(unsigned char* data, int width, int height, int channels, int kernelSize, double sigma);

    DLL_EXPORT unsigned char* InverseImage(unsigned char* data, int width, int height, int channels);

    DLL_EXPORT unsigned char* RgbToGray8bit(unsigned char* data, int width, int height);

    DLL_EXPORT unsigned char* AdaptiveThresholdImage(unsigned char* data, int width, int height);

    DLL_EXPORT unsigned char* SobelFilterImage(unsigned char* data, int width, int height, int channels);
}

#endif