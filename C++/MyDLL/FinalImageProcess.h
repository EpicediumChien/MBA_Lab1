// ImageProcess.h
#pragma once

#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

#include <stdint.h>

unsigned char* findReferenceMarker(unsigned char* refData, int refWidth, int refHeight, int refChannels, int refStride,
    unsigned char* currData, int currWidth, int currHeight, int currChannels, int currStride, unsigned char** processedBiMarkData);

#ifdef __cplusplus
extern "C" {
#endif
    typedef struct {
        int Index;
        int Area;
        int Perimeter;
    } ObjectData;

    // Image detect
    DLL_EXPORT float CompareImagesWithFourierDescriptors(unsigned char* image1, int width1, int height1, unsigned char* image2, int width2, int height2);
    DLL_EXPORT unsigned char* FindReferenceMarker(unsigned char* refData, int refWidth, int refHeight, int refChannels, int refStride,
        unsigned char* currData, int currWidth, int currHeight, int currChannels, int currStride, unsigned char** processedBiMarkData) {
        return findReferenceMarker(refData, refWidth, refHeight, refChannels, refStride, currData, currWidth, currHeight, currChannels, currStride, processedBiMarkData);
    }
#ifdef __cplusplus
    }
#endif