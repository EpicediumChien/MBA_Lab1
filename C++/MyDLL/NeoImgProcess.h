#pragma once

#ifndef DLL_EXPORT
#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __attribute__((visibility("default")))
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

DLL_EXPORT unsigned char* FindReferenceMarker(unsigned char* refData, int refWidth, int refHeight, int refChannels, int refStride,
                                              unsigned char* currData, int currWidth, int currHeight, int currChannels, int currStride);

#ifdef __cplusplus
}
#endif
