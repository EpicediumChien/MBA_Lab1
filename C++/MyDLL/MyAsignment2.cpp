// MyAssignment2.c
#include "MyAssignment2.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Sobel filter for edge detection
unsigned char* SobelFilterImage(unsigned char* data, int width, int height, int channels) {
    int stride = (width * channels + 3) & ~3; // Align stride to the next multiple of 4
    unsigned char* output = (unsigned char*)calloc(height, stride); // Allocate memory with padding

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
            output[y * stride + x] = (magnitude > 128) ? 255 : 0; // Threshold
        }
    }

    return output;
}


// Static helper function for depth-first search (DFS) to find connected components and calculate area and perimeter
static void dfs(int x, int y, int width, int height, unsigned char* sobelImage, unsigned char* visited, int* area, int* perimeter, int stride) {
    // Stack-based DFS to avoid recursion depth limits
    int* stackX = (int*)malloc(width * height * sizeof(int));
    int* stackY = (int*)malloc(width * height * sizeof(int));
    int stackIndex = 0;

    // Push the initial pixel onto the stack
    stackX[stackIndex] = x;
    stackY[stackIndex] = y;
    stackIndex++;

    // Directions for 8-connectivity (left, right, up, down, and diagonal)
    int directions[8][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1} };

    // Iterative DFS using the stack
    while (stackIndex > 0) {
        // Pop a coordinate from the stack
        stackIndex--;
        int cx = stackX[stackIndex];
        int cy = stackY[stackIndex];

        if (visited[cy * stride + cx] == 1) // Skip already visited pixels
            continue;

        // Mark as visited
        visited[cy * stride + cx] = 1;
        (*area)++;

        // Check if the current pixel is part of the perimeter
        bool isBoundary = false;

        for (int i = 0; i < 8; i++) {
            int nx = cx + directions[i][0];
            int ny = cy + directions[i][1];

            // Check if the neighbor is out of bounds or part of the background (0 in sobelImage)
            if (nx < 0 || nx >= width || ny < 0 || ny >= height || sobelImage[ny * stride + nx] == 0) {
                isBoundary = true;
            }
        }

        if (isBoundary) {
            (*perimeter)++;
        }

        // Traverse all 8 directions and add neighbors to the stack
        for (int i = 0; i < 8; i++) {
            int nx = cx + directions[i][0];
            int ny = cy + directions[i][1];

            if (nx >= 0 && nx < width && ny >= 0 && ny < height &&
                visited[ny * stride + nx] == 0 && sobelImage[ny * stride + nx] == 255) {
                stackX[stackIndex] = nx;
                stackY[stackIndex] = ny;
                stackIndex++;
            }
        }
    }

    free(stackX);
    free(stackY);
}


// Function to process the image, detect objects, and return their data
ObjectData* ProcessImage_Asm2(unsigned char* data, int width, int height, int channels, int* objectCount, unsigned char** processedImage) {
    int stride = (width * channels + 3) & ~3; // Calculate stride with padding
    unsigned char* sobelImage = SobelFilterImage(data, width, height, channels);
    *processedImage = sobelImage;

    unsigned char* visited = (unsigned char*)calloc(height * stride, sizeof(unsigned char)); // Use stride for visited
    ObjectData* objects = (ObjectData*)malloc(sizeof(ObjectData) * width * height);
    int currentIndex = 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (sobelImage[y * stride + x] == 255 && visited[y * stride + x] == 0) {
                int area = 0, perimeter = 0;
                dfs(x, y, width, height, sobelImage, visited, &area, &perimeter, stride);

                objects[currentIndex].Index = currentIndex;
                objects[currentIndex].Perimeter = perimeter;
                objects[currentIndex].Area = area;
                currentIndex++;
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
