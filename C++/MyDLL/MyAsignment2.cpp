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
    int threshold = 150;
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
