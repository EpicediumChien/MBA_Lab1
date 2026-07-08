#include "MyAssignment2.h"
#include "FinalImageProcess.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <emmintrin.h> // For SSE2 intrinsics
#include <intrin.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846 // Define M_PI manually if not available
#endif // !M_PI

#define MAX_POINTS 1000

typedef struct {
    int x, y;
} Point;

// Approximation for Cosine
static inline __m128d simd_cos(__m128d x) {
    // Use scalar math functions for now
    double values[2];
    _mm_storeu_pd(values, x);
    values[0] = cos(values[0]);
    values[1] = cos(values[1]);
    return _mm_loadu_pd(values);
}

static inline __m128d simd_sin(__m128d x) {
    // Use scalar math functions for now
    double values[2];
    _mm_storeu_pd(values, x); // Extract values from the SIMD register
    values[0] = sin(values[0]); // Compute sine for the first value
    values[1] = sin(values[1]); // Compute sine for the second value
    return _mm_loadu_pd(values); // Pack the results back into an SIMD register
}

int countTrailingZeros(unsigned int mask) {
    unsigned long index; // MSVC requires `unsigned long` for the index
    if (_BitScanForward(&index, mask)) {
        return (int)index; // Return the position of the first set bit
    }
    return 32; // If `mask` is 0, return 32 (no set bit found)
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

// Function to extract contours from a binary image
int extractContoursSIMD(unsigned char* data, int width, int height, int stride, Point* contours, int maxContours) {
    int numContours = 0;

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x += 16) {
            __m128i pixels = _mm_loadu_si128((__m128i*) & data[y * stride + x]);
            __m128i whiteMask = _mm_cmpeq_epi8(pixels, _mm_set1_epi8(255));

            int mask = _mm_movemask_epi8(whiteMask);
            while (mask && numContours < maxContours) {
                int offset = countTrailingZeros(mask); // Use MSVC-compatible function
                int px = x + offset;
                if (px >= 0 && px < width && y >= 0 && y < height) {
                    // Check neighbors (simplified boundary check)
                    int isBoundary = 0;
                    for (int dy = -1; dy <= 1 && !isBoundary; dy++) {
                        for (int dx = -1; dx <= 1; dx++) {
                            if (dx == 0 && dy == 0) continue;
                            int nx = px + dx;
                            int ny = y + dy;
                            if (data[ny * stride + nx] == 0) {
                                isBoundary = 1;
                                break;
                            }
                        }
                    }

                    if (isBoundary) {
                        Point point;      // Create a temporary Point
                        point.x = px;
                        point.y = y;
                        contours[numContours++] = point; // Assign to contours
                    }
                    mask &= mask - 1; // Clear the first set bit
                }
            }
        }
    }

    return numContours;
}

// Function to compute Fourier Descriptors (FDs)
void computeFourierDescriptorsOptimized(Point* points, int numPoints, double* FDs) {
    __m128d sumCx = _mm_setzero_pd();
    __m128d sumCy = _mm_setzero_pd();

    // Compute centroid using SIMD
    for (int i = 0; i < numPoints; i += 2) {
        __m128d px = _mm_set_pd(points[i + 1].x, points[i].x);
        __m128d py = _mm_set_pd(points[i + 1].y, points[i].y);
        sumCx = _mm_add_pd(sumCx, px);
        sumCy = _mm_add_pd(sumCy, py);
    }

    double cx[2], cy[2];
    _mm_storeu_pd(cx, sumCx);
    _mm_storeu_pd(cy, sumCy);
    cx[0] = (cx[0] + cx[1]) / numPoints;
    cy[0] = (cy[0] + cy[1]) / numPoints;

    for (int u = 0; u < numPoints; u++) {
        __m128d realPart = _mm_setzero_pd();
        __m128d imagPart = _mm_setzero_pd();

        for (int n = 0; n < numPoints; n += 2) {
            __m128d theta = _mm_set_pd(-2.0 * M_PI * u * (n + 1) / numPoints, -2.0 * M_PI * u * n / numPoints);
            __m128d dx = _mm_set_pd(points[n + 1].x - cx[0], points[n].x - cx[0]);
            __m128d dy = _mm_set_pd(points[n + 1].y - cy[0], points[n].y - cy[0]);
            __m128d r = _mm_sqrt_pd(_mm_add_pd(_mm_mul_pd(dx, dx), _mm_mul_pd(dy, dy)));

            realPart = _mm_add_pd(realPart, _mm_mul_pd(r, simd_cos(theta)));
            imagPart = _mm_add_pd(imagPart, _mm_mul_pd(r, simd_sin(theta)));
        }

        double real[2], imag[2];
        _mm_storeu_pd(real, realPart);
        _mm_storeu_pd(imag, imagPart);

        FDs[u] = sqrt((real[0] + real[1]) * (real[0] + real[1]) +
            (imag[0] + imag[1]) * (imag[0] + imag[1]));
    }
}

// Function to match FDs between reference and candidate contours
double matchFourierDescriptorsSSE2(double* refFDs, double* candFDs, int numFDs) {
    __m128d sum = _mm_setzero_pd();

    for (int i = 0; i < numFDs; i += 2) {
        __m128d ref = _mm_loadu_pd(&refFDs[i]);
        __m128d cand = _mm_loadu_pd(&candFDs[i]);

        __m128d diff = _mm_sub_pd(ref, cand);
        __m128d squared = _mm_mul_pd(diff, diff);
        sum = _mm_add_pd(sum, squared);
    }

    double result[2];
    _mm_storeu_pd(result, sum);
    return sqrt(result[0] + result[1]);
}

// Function to draw a red box around a region
void drawRedBoxSSE2(unsigned char* rgbImage, int width, int height, int channels, int stride, int x, int y, int boxWidth, int boxHeight) {
    __m128i red = _mm_set_epi8(
        255, 0, 0, 255, 0, 0, 255, 0, // Second pixel
        0, 0, 255, 0, 0, 255, 0, 0  // First pixel
    );;

    for (int i = 0; i < boxWidth; i++) {
        if (x + i >= 0 && x + i < width) {
            int top = y * stride + (x + i) * channels;
            int bottom = (y + boxHeight - 1) * stride + (x + i) * channels;

            if (y >= 0 && y < height) {
                _mm_storeu_si128((__m128i*) & rgbImage[top], red);
            }

            if (y + boxHeight - 1 >= 0 && y + boxHeight - 1 < height) {
                _mm_storeu_si128((__m128i*) & rgbImage[bottom], red);
            }
        }
    }

    for (int i = 0; i < boxHeight; i++) {
        if (y + i >= 0 && y + i < height) {
            int left = (y + i) * stride + x * channels;
            int right = (y + i) * stride + (x + boxWidth - 1) * channels;

            if (x >= 0 && x < width) {
                _mm_storeu_si128((__m128i*) & rgbImage[left], red);
            }

            if (x + boxWidth - 1 >= 0 && x + boxWidth - 1 < width) {
                _mm_storeu_si128((__m128i*) & rgbImage[right], red);
            }
        }
    }
}


unsigned char* initializeRGBImage(unsigned char* binaryData, int width, int height, int stride) {
    int rgbStride = (width * 3 + 3) & ~3;
    unsigned char* rgbImage = (unsigned char*)calloc(height, rgbStride);

    if (!rgbImage) {
        fprintf(stderr, "Error: Memory allocation failed for RGB image.\n");
        return NULL;
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char intensity = binaryData[y * stride + x];
            int offset = y * rgbStride + x * 3;
            rgbImage[offset + 0] = intensity; // Blue
            rgbImage[offset + 1] = intensity; // Green
            rgbImage[offset + 2] = intensity; // Red
        }
    }

    return rgbImage;
}

int isValidContour(Point* contour, int contourSize, int imgWidth, int imgHeight) {
    if (contourSize <= 0) return 0; // Invalid if no points

    for (int i = 0; i < contourSize; i++) {
        if (contour[i].x < 0 || contour[i].x >= imgWidth ||
            contour[i].y < 0 || contour[i].y >= imgHeight) {
            return 0; // Invalid if any point is out of bounds
        }
    }

    // Add more checks, like minimum contour area, if needed
    return 1; // Valid contour
}

double calculatePerimeter(Point* contour, int contourSize) {
    double perimeter = 0.0;
    for (int i = 0; i < contourSize; i++) {
        int nextIndex = (i + 1) % contourSize;
        double dx = contour[nextIndex].x - contour[i].x;
        double dy = contour[nextIndex].y - contour[i].y;
        perimeter += sqrt(dx * dx + dy * dy);
    }
    return perimeter;
}

double calculateArea(Point* contour, int contourSize) {
    double area = 0.0;
    for (int i = 0; i < contourSize; i++) {
        int nextIndex = (i + 1) % contourSize;
        area += (contour[i].x * contour[nextIndex].y) - (contour[nextIndex].x * contour[i].y);
    }
    return fabs(area) / 2.0;
}

// Main function to find the reference marker in the image
unsigned char* findReferenceMarker(unsigned char* markData, int markWidth, int markHeight, int markChannels, int markStride,
    unsigned char* currData, int currWidth, int currHeight, int currChannels, int currStride, unsigned char** processedBiMarkData) {

    // Binarize the template and reference images
    unsigned char* biMarkData = BinarizeImage(markData, markWidth, markHeight, markChannels, 128);
    *processedBiMarkData = biMarkData;
    unsigned char* biCurrData = BinarizeImage(currData, currWidth, currHeight, currChannels, 128);

    // Prepare the reference image for drawing (convert to RGB if grayscale)
    int rgbStride = (currWidth * 3 + 3) & ~3; // Align stride for RGB
    unsigned char* refRGB = (currChannels == 1) ? initializeRGBImage(biCurrData, currWidth, currHeight, currStride) : biCurrData;

    // Extract template contour
    Point markContour[MAX_POINTS];
    int markContourSize = extractContoursSIMD(biMarkData, markWidth, markHeight, markStride, markContour, MAX_POINTS);
    if (markContourSize == 0) {
        fprintf(stderr, "Error: No contours found in the template.\n");
        return refRGB;
    }

    // Compute Fourier descriptors for the template contour
    double markFDs[MAX_POINTS];
    computeFourierDescriptorsOptimized(markContour, markContourSize, markFDs);

    double markPerimeter = calculatePerimeter(markContour, markContourSize);
    double markArea = calculateArea(markContour, markContourSize);

    // Extract candidate contours from the reference image
    Point candContour[MAX_POINTS];
    int candContourSize = extractContoursSIMD(biCurrData, currWidth, currHeight, currStride, candContour, MAX_POINTS);
    if (candContourSize == 0) {
        fprintf(stderr, "Error: No contours found in the reference image.\n");
        return refRGB;
    }

    // Match template contour with candidate contours
    double bestMatchScore = 1e9;
    int bestIndex = -1;
    for (int i = 0; i < candContourSize; i++) {
        if (isValidContour(&candContour[i], candContourSize, currWidth, currHeight)) {
            double candPerimeter = calculatePerimeter(&candContour[i], candContourSize);
            double candArea = calculateArea(&candContour[i], candContourSize);

            // Compare perimeter and area differences
            double perimeterDiff = fabs(candPerimeter - markPerimeter);
            double areaDiff = fabs(candArea - markArea);

            // Calculate Fourier descriptor match score
            double candFDs[MAX_POINTS];
            computeFourierDescriptorsOptimized(&candContour[i], candContourSize, candFDs);
            double matchScore = matchFourierDescriptorsSSE2(markFDs, candFDs, markContourSize);

            // Combine metrics into a single score
            double totalScore = perimeterDiff + areaDiff + matchScore;
            if (totalScore < bestMatchScore) {
                bestMatchScore = totalScore;
                bestIndex = i;
            }
        }
    }

    // Check if a match was found
    if (bestIndex != -1) {
        // Draw a bounding box around the best-matched contour
        Point* bestContour = &candContour[bestIndex];

        // Initialize min and max coordinates
        int minX = currWidth, maxX = 0;
        int minY = currHeight, maxY = 0;

        // Iterate through the points in the contour
        for (int i = 0; i < candContourSize; i++) {
            if (bestContour[i].x < minX) minX = bestContour[i].x;
            if (bestContour[i].x > maxX) maxX = bestContour[i].x;
            if (bestContour[i].y < minY) minY = bestContour[i].y;
            if (bestContour[i].y > maxY) maxY = bestContour[i].y;
        }

        // Check if min/max values are valid
        if (minX >= 0 && minY >= 0 && maxX < currWidth && maxY < currHeight) {
            int boxWidth = maxX - minX + 1;
            int boxHeight = maxY - minY + 1;
            drawRedBoxSSE2(refRGB, currWidth, currHeight, 3, rgbStride, minX, minY, boxWidth, boxHeight);
        }
    }
    else {
        fprintf(stderr, "Error: No matching contour found in the reference image.\n");
    }

    return refRGB;
}

