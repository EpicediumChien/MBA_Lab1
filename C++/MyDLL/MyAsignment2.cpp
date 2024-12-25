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

// Static helper function for depth-first search (DFS) to find connected components and calculate area and perimeter
static void dfs(int x, int y, int width, int height, unsigned char* binaryImage, unsigned char* visited, int* area, int* perimeter, int stride) {
    // Stack for DFS
    int* stackX = (int*)malloc(width * height * sizeof(int));
    int* stackY = (int*)malloc(width * height * sizeof(int));
    int stackIndex = 0;

    // Push initial pixel onto the stack
    stackX[stackIndex] = x;
    stackY[stackIndex] = y;
    stackIndex++;

    // Directions for 8-connectivity
    int directions[8][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1} };

    while (stackIndex > 0) {
        // Pop pixel from stack
        stackIndex--;
        int cx = stackX[stackIndex];
        int cy = stackY[stackIndex];

        if (visited[cy * stride + cx] == 1) {
            continue;
        }

        // Mark as visited
        visited[cy * stride + cx] = 1;
        (*area)++;

        bool isBoundary = false;

        // Check all neighbors
        for (int i = 0; i < 8; i++) {
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
            (*perimeter)++;
        }
    }

    free(stackX);
    free(stackY);
}


// Function to process the image, detect objects, and return their data
ObjectData* ProcessImage_Asm2(unsigned char* data, int width, int height, int channels, int* objectCount, unsigned char** processedImage) {
    int stride = (width * channels + 3) & ~3;
    int threshold = 128;
    unsigned char* binaryImage = BinarizeImage(data, width, height, channels, threshold);
    *processedImage = binaryImage;
    unsigned char* visited = (unsigned char*)calloc(height, stride);

    ObjectData* objects = (ObjectData*)malloc(sizeof(ObjectData) * width * height);
    int currentIndex = 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (binaryImage[y * stride + x] == 255 && visited[y * stride + x] == 0) {
                int area = 0, perimeter = 0;
                dfs(x, y, width, height, binaryImage, visited, &area, &perimeter, stride);

                if (area >= 10) { // Only include objects with area >= 10
                    objects[currentIndex].Index = currentIndex;
                    objects[currentIndex].Area = area;
                    objects[currentIndex].Perimeter = perimeter;
                    currentIndex++;
                }
            }
        }
    }

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

// Function to mark edges in red
unsigned char* MarkEdgesWithChainCode(unsigned char* binaryImage, int width, int height) {
    // Calculate strides
    int binaryStride = (width + 3) & ~3;  // Stride for binary image
    int rgbStride = (width * 3 + 3) & ~3; // Stride for the RGB image

    // Allocate memory for the RGB image
    unsigned char* rgbImage = (unsigned char*)calloc(height, rgbStride);

    // Initialize the RGB image (copy binary image to grayscale in RGB)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char intensity = binaryImage[y * binaryStride + x];
            rgbImage[y * rgbStride + x * 3 + 0] = intensity; // Blue
            rgbImage[y * rgbStride + x * 3 + 1] = intensity; // Green
            rgbImage[y * rgbStride + x * 3 + 2] = intensity; // Red
        }
    }

    // Chain code directions (clockwise starting from right)
    int directions[8][2] = { {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}, {1, 0}, {1, 1} };

    // Mark edges in red using chain code
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            if (binaryImage[y * binaryStride + x] == 255) {
                for (int d = 0; d < 8; d++) {
                    int nx = x + directions[d][1];
                    int ny = y + directions[d][0];

                    // If the neighbor is background (0), mark the current pixel as red
                    if (nx < 0 || nx >= width || ny < 0 || ny >= height || binaryImage[ny * binaryStride + nx] == 0) {
                        rgbImage[y * rgbStride + x * 3 + 0] = 0;   // Blue
                        rgbImage[y * rgbStride + x * 3 + 1] = 0;   // Green
                        rgbImage[y * rgbStride + x * 3 + 2] = 255; // Red
                        break;
                    }
                }
            }
        }
    }

    return rgbImage;
}


// Main processing function
unsigned char* ProcessImageWithChainCode(unsigned char* data, int width, int height, int channels) {
    unsigned char* binaryImage = BinarizeImage(data, width, height, channels, 128);
    unsigned char* outputImage = MarkEdgesWithChainCode(binaryImage, width, height);

    free(binaryImage); // Free binary image memory
    return outputImage;
}