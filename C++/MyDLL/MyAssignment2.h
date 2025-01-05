#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

extern "C" {
    // Structure to hold object data (Area and Perimeter)
    typedef struct {
        int Index;      // Object index
        int Area;       // Area of the object (count of pixels)
        int Perimeter;  // Perimeter of the object (count of boundary pixels)
        float CenterX;  // X-coordinate of the object's center
        float CenterY;  // Y-coordinate of the object's center
        float Diameter; // Estimated diameter of the object
    } ObjectData;

    // Function to process the image, detect objects, and return their data
    DLL_EXPORT ObjectData* ProcessImage_Asm2(unsigned char* data, int width, int height, int channels, int* objectCount, unsigned char** processedImage);
    //DLL_EXPORT unsigned char* ProcessImageWithChainCode(unsigned char* image, int width, int height, int channels);
    DLL_EXPORT void FreeProcessedImage(unsigned char* image);
    DLL_EXPORT void FreeObjectData(ObjectData* objects);
    DLL_EXPORT unsigned char* BinarizeImage(unsigned char* data, int width, int height, int channels)
    {
        int threshold = 128;
        return BinarizeImage(data, width, height, channels, threshold);
    }
}

