#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
    typedef struct {
        int Index;
        int Area;
        int Perimeter;
        float CenterX;
        float CenterY;
        float Diameter;
    } ObjectData;

    DLL_EXPORT ObjectData* ProcessImage_Asm2(unsigned char* data, int width, int height, int channels, int* objectCount, unsigned char** processedImage);
    DLL_EXPORT void FreeProcessedImage(unsigned char* image);
    DLL_EXPORT void FreeObjectData(ObjectData* objects);
    DLL_EXPORT unsigned char* BinarizeImage(unsigned char* data, int width, int height, int channels, int threshold);
}
