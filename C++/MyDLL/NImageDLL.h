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
    DLL_EXPORT void ApplyGaussianBlurImage(void* nimage, int kernelSize, double sigma);
}

#endif