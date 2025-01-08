// MyAssignment2.c
#include "MyAssignment2.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Sobel filter for edge detection
unsigned char* BinarizeImage(unsigned char* data, int width, int height, int channels, int threshold) {
    int stride = (width * channels + 3) & ~3; // Align stride to next multiple of 4
    unsigned char* binaryImage = (unsigned char*)calloc(height, stride);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int pixel = data[y * stride + x * channels];
            binaryImage[y * stride + x * channels] = (pixel > threshold) ? 255 : 0; // Use specified threshold
        }
    }

    return binaryImage;
}

unsigned char* TransferBinarizeImage(unsigned char* data, int width, int height, int channels)
{
    int threshold = 128;
    return BinarizeImage(data, width, height, channels, threshold);
}

// Static helper function for depth-first search (DFS) to find connected components and calculate area and perimeter
static void dfs(int x, int y, int width, int height, unsigned char* binaryImage, unsigned char* visited, int* area, int* perimeter, int stride, unsigned char* rgbImage, float* centerX, float* centerY, float* diameter) {
    int rgbStride = (width * 3 + 3) & ~3; // Stride for the RGB image

    *area = 0;
    *perimeter = 0;

    long long sumX = 0, sumY = 0;
    int minX = width, maxX = 0, minY = height, maxY = 0;

    int* stackX = (int*)malloc(width * height * sizeof(int));
    int* stackY = (int*)malloc(width * height * sizeof(int));
    int stackIndex = 0;

    stackX[stackIndex] = x;
    stackY[stackIndex] = y;
    stackIndex++;

    int directions[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };

    while (stackIndex > 0) {
        stackIndex--;
        int cx = stackX[stackIndex];
        int cy = stackY[stackIndex];

        if (visited[cy * stride + cx] == 1) {
            continue;
        }

        visited[cy * stride + cx] = 1;
        (*area)++;
        sumX += cx;
        sumY += cy;

        if (cx < minX) minX = cx;
        if (cx > maxX) maxX = cx;
        if (cy < minY) minY = cy;
        if (cy > maxY) maxY = cy;

        bool isBoundary = false;

        for (int i = 0; i < 4; i++) {
            int nx = cx + directions[i][0];
            int ny = cy + directions[i][1];

            if (nx < 0 || nx >= width || ny < 0 || ny >= height || binaryImage[ny * stride + nx] == 0) {
                isBoundary = true;
            }
            else if (visited[ny * stride + nx] == 0 && binaryImage[ny * stride + nx] == 255) {
                stackX[stackIndex] = nx;
                stackY[stackIndex] = ny;
                stackIndex++;
            }
        }

        if (isBoundary) {
            rgbImage[cy * rgbStride + cx * 3 + 0] = 0;   // Blue
            rgbImage[cy * rgbStride + cx * 3 + 1] = 0;   // Green
            rgbImage[cy * rgbStride + cx * 3 + 2] = 255; // Red
            (*perimeter)++;
        }
    }

    *centerX = (float)sumX / (*area);
    *centerY = (float)sumY / (*area);
    *diameter = fmax(maxX - minX, maxY - minY);

    free(stackX);
    free(stackY);
}


// Function to process the image, detect objects, and return their data
ObjectData* ProcessImage_Asm2(unsigned char* data, int width, int height, int channels, int* objectCount, unsigned char** processedImage) {
    // Calculate strides
    int binaryStride = (width + 3) & ~3;  // Stride for binary image
    int rgbStride = (width * 3 + 3) & ~3; // Stride for the RGB image

    int threshold = 128;
    unsigned char* binaryImage = BinarizeImage(data, width, height, channels, threshold);
    unsigned char* visited = (unsigned char*)calloc(height, binaryStride);
    unsigned char* visitedBoundary = (unsigned char*)calloc(height, binaryStride);

    // Allocate memory for the RGB image
    unsigned char* rgbImage = (unsigned char*)calloc(height, rgbStride);

    ObjectData* objects = (ObjectData*)malloc(sizeof(ObjectData) * width * height);
    int currentIndex = 0;

    // Initialize the RGB image (copy binary image to grayscale in RGB)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char intensity = binaryImage[y * binaryStride + x];
            rgbImage[y * rgbStride + x * 3 + 0] = intensity; // Blue
            rgbImage[y * rgbStride + x * 3 + 1] = intensity; // Green
            rgbImage[y * rgbStride + x * 3 + 2] = intensity; // Red
        }
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (binaryImage[y * binaryStride + x] == 255 && visited[y * binaryStride + x] == 0) {
                int area = 0, perimeter = 0;
                float centerX = 0, centerY = 0, diameter = 0;
                dfs(x, y, width, height, binaryImage, visited, &area, &perimeter, binaryStride, rgbImage, &centerX, &centerY, &diameter);

                if (area >= 10) { // Only include objects with area >= 10
                    objects[currentIndex].Index = currentIndex;
                    objects[currentIndex].Area = area;
                    objects[currentIndex].Perimeter = perimeter;
                    objects[currentIndex].CenterX = centerX;
                    objects[currentIndex].CenterY = centerY;
                    objects[currentIndex].Diameter = diameter;
                    currentIndex++;
                }
            }
        }
    }

    *processedImage = rgbImage;
    *objectCount = currentIndex;
    objects = (ObjectData*)realloc(objects, sizeof(ObjectData) * currentIndex);

    free(visited);

    return objects;
}

void FreeProcessedImage(unsigned char* image) {
    free(image);
}

void FreeObjectData(ObjectData* objects) {
    free(objects);
}
