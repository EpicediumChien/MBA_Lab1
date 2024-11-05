#include "NImageDLL.h"
#include "NImage.h"

DLL_EXPORT void* CreateNImage() {
    return new NImage();
}

DLL_EXPORT void DeleteNImage(void* nimage) {
    delete static_cast<NImage*>(nimage);
}

DLL_EXPORT bool LoadImage(void* nimage, const char* filename) {
    return static_cast<NImage*>(nimage)->loadImage(filename);
}

DLL_EXPORT int GetWidth(void* nimage) {
    return static_cast<NImage*>(nimage)->getWidth();
}

DLL_EXPORT int GetHeight(void* nimage) {
    return static_cast<NImage*>(nimage)->getHeight();
}

DLL_EXPORT int GetChannels(void* nimage) {
    return static_cast<NImage*>(nimage)->getChannels();
}

DLL_EXPORT unsigned char* GetData(void* nimage) {
    return static_cast<NImage*>(nimage)->getData();
}

DLL_EXPORT unsigned char* GetPalette(void* nimage) {
    return static_cast<NImage*>(nimage)->getPalette();
}

DLL_EXPORT bool SaveImage(void* nimage, const char* filename) {
    return static_cast<NImage*>(nimage)->saveImage(filename);
}

// This function exposes the Gaussian blur
DLL_EXPORT bool ApplyGaussianBlurToImage(void* nimage, int kernelSize, double sigma) {
    NImage* image = static_cast<NImage*>(nimage);
    if (image == nullptr || image->getData() == nullptr) {
        return false;  // Error: invalid image pointer or no image data
    }

    // Get image parameters
    unsigned char* data = image->getData();
    int width = image->getWidth();
    int height = image->getHeight();
    int channels = image->getChannels();

    // Apply Gaussian blur
    ApplyGaussianBlur(data, width, height, channels, kernelSize, sigma);

    return true;  // Return success
}