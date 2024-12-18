// MyAssignment2.c
#include "MyAssignment2.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Sobel filter for edge detection
unsigned char* SobelFilterImage(unsigned char* data, int width, int height, int channels) {
    int stride = width * channels;
    unsigned char* output = (unsigned char*)malloc(width * height);

    // Sobel filter kernels for edge detection
    int Gx[3][3] = { { -1, 0, 1 }, { -2, 0, 2 }, { -1, 0, 1 } };
    int Gy[3][3] = { { -1, -2, -1 }, { 0, 0, 0 }, { 1, 2, 1 } };

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int sumX = 0, sumY = 0;
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int pixel = data[(y + ky) * stride + (x + kx) * channels];
                    sumX += Gx[ky + 1][kx + 1] * pixel;
                    sumY += Gy[ky + 1][kx + 1] * pixel;
                }
            }
            int magnitude = (int)sqrt(sumX * sumX + sumY * sumY);
            output[y * width + x] = (magnitude > 128) ? 255 : 0; // Threshold
        }
    }
    return output;
}

// Static helper function for depth-first search (DFS) to find connected components
static void dfs(int x, int y, int width, int height, unsigned char* sobelImage, int* visited, int* area, int* runCount) {
    visited[y * width + x] = 1; // Mark as visited
    (*area)++;
    (*runCount)++;

    // Directions for 8-connectivity (left, right, up, down, and diagonal)
    int directions[8][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1} };

    // Traverse all 8 directions
    for (int i = 0; i < 8; i++) {
        int nx = x + directions[i][0];
        int ny = y + directions[i][1];
        if (nx >= 0 && nx < width && ny >= 0 && ny < height && !visited[ny * width + nx] && sobelImage[ny * width + nx] == 255) {
            dfs(nx, ny, width, height, sobelImage, visited, area, runCount);
        }
    }
}

// Function to process the image, detect objects, and return their data
ObjectData* ProcessImage_Asm2(unsigned char* data, int width, int height, int channels, int* objectCount) {
    unsigned char* sobelImage = SobelFilterImage(data, width, height, channels);
    int stride = width;
    int* visited = (int*)calloc(width * height, sizeof(int)); // To mark visited pixels
    ObjectData* objects = (ObjectData*)malloc(sizeof(ObjectData) * width * height); // Maximum possible objects (not realistic, but just to be safe)
    int currentIndex = 0;

    // Detect objects by checking connected components
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (sobelImage[y * width + x] == 255 && visited[y * width + x] == 0) {
                // New object detected
                int area = 0, runCount = 0;
                dfs(x, y, width, height, sobelImage, visited, &area, &runCount);

                // Store object data
                objects[currentIndex].Index = currentIndex;
                objects[currentIndex].RunCount = runCount;
                objects[currentIndex].Area = area;
                currentIndex++;
            }
        }
    }

    // Set the object count and return the array of objects
    *objectCount = currentIndex;
    objects = (ObjectData*)realloc(objects, sizeof(ObjectData) * currentIndex); // Resize the array to fit the actual number of objects

    // Free allocated memory
    free(sobelImage);
    free(visited);

    return objects;
}
