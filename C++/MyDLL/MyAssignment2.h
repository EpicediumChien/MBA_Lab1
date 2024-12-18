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
        int RunCount;  // Perimeter of the object (count of boundary pixels)
    } ObjectData;

    // Function to process the image, detect objects, and return their data
    DLL_EXPORT ObjectData* ProcessImage_Asm2(unsigned char* data, int width, int height, int channels, int* objectCount);
}

