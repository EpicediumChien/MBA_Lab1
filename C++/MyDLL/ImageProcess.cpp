// ImageProcess.cpp
#include "MyAssignment2.h"
#include "ImageProcess.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <emmintrin.h> // For SSE2 intrinsics
#include <intrin.h>

#define M_PI 3.14159265358979323846 // Define M_PI manually if not available
#define MAX_GRAY_LEVELS 256
#define MAX_POINTS 1000

// Sobel filter kernels for horizontal and vertical edges
const int SobelX[3][3] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}
};

const int SobelY[3][3] = {
    {-1, -2, -1},
    { 0,  0,  0},
    { 1,  2,  1}
};

typedef struct {
    int x, y;
} Point;

NImage* CreateNImage() {
    NImage* image = (NImage*)malloc(sizeof(NImage));
    image->width = 0;
    image->height = 0;
    image->channels = 0;
    image->data = NULL;
    image->palette = NULL;
    return image;
}

void DeleteNImage(NImage* image) {
    if (image->data) free(image->data);
    if (image->palette) free(image->palette);
    free(image);
}

int LoadImage(NImage* image, const char* filename) {
    FILE* file;
    fopen_s(&file, filename, "rb");
    if (!file) return 0;

    unsigned char header[54];
    fread(header, sizeof(unsigned char), 54, file);

    image->width = *(int*)&header[18];
    image->height = *(int*)&header[22];
    short bitsPerPixel = *(short*)&header[28];

    if (bitsPerPixel == 24) {
        image->channels = 3;
        int rowSize = (image->width * 3 + 3) & ~3;
        image->data = (unsigned char*)malloc(rowSize * image->height);
        for (int y = 0; y < image->height; ++y) {
            fread(image->data + (image->height - y - 1) * rowSize, 1, rowSize, file);
        }
    }
    else if (bitsPerPixel == 8) {
        image->channels = 1;
        int rowSize = (image->width + 3) & ~3;
        image->palette = (unsigned char*)malloc(1024);
        if (!image->palette) {
            fprintf(stderr, "Error: Memory allocation failed for palette\n");
            fclose(file);
            return 0;
        }
        fread(image->palette, 1, 1024, file);

        image->data = (unsigned char*)malloc(rowSize * image->height);
        if (!image->data) {
            fprintf(stderr, "Error: Memory allocation failed for image data\n");
            free(image->palette);
            fclose(file);
            return 0;
        }

        // Read image data (bottom-to-top row order)
        for (int y = 0; y < image->height; ++y) {
            fread(image->data + (image->height - y - 1) * rowSize, 1, rowSize, file);
        }
    }
    else {
        fclose(file);
        return 0;
    }

    fclose(file);
    return 1;
}

int SaveImage(NImage* image, const char* filename) {
    FILE* file;
    fopen_s(&file, filename, "rb");
    if (!file) return 0;

    BMPHeader bmpHeader = { 0x4D42, 54 + image->width * image->height * image->channels, 0, 0, 54 };
    BMPInfoHeader bmpInfoHeader = { 40, image->width, image->height, 1, (unsigned short)(image->channels * 8), 0, (unsigned int)(image->width * image->height * image->channels), 0, 0, 0, 0 };

    fwrite(&bmpHeader, sizeof(bmpHeader), 1, file);
    fwrite(&bmpInfoHeader, sizeof(bmpInfoHeader), 1, file);
    fwrite(image->data, image->width * image->height * image->channels, 1, file);

    fclose(file);
    return 1;
}

int GetWidth(const NImage* image) {
    return image->width;
}

int GetHeight(const NImage* image) {
    return image->height;
}

int GetChannels(const NImage* image) {
    return image->channels;
}

const unsigned char* GetData(const NImage* image) {
    return image->data;
}

float* generateGaussianKernel(int kernelSize, double sigma) {
    float* kernel = (float*)malloc(kernelSize * sizeof(float));
    int halfSize = kernelSize / 2;
    float sum = 0.0f;
    for (int i = -halfSize; i <= halfSize; ++i) {
        kernel[i + halfSize] = (float)(exp(-(i * i) / (2 * sigma * sigma)) / (sqrt(2 * M_PI) * sigma));
        sum += kernel[i + halfSize];
    }
    for (int i = 0; i < kernelSize; ++i) kernel[i] /= sum;
    return kernel;
}

unsigned char* ApplyGaussianBlurImage(unsigned char* data, int width, int height, int channels, int kernelSize, double sigma) {
    float* kernel = generateGaussianKernel(kernelSize, sigma);
    unsigned char* temp = (unsigned char*)malloc(width * height * channels);
    memcpy(temp, data, width * height * channels);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            for (int c = 0; c < channels; ++c) {
                float sum = 0.0f;
                for (int k = -kernelSize / 2; k <= kernelSize / 2; ++k) {
                    int posX = x + k;
                    if (posX < 0) posX = -posX;
                    if (posX >= width) posX = 2 * width - posX - 1;
                    sum += temp[(y * width + posX) * channels + c] * kernel[k + kernelSize / 2];
                }
                data[(y * width + x) * channels + c] = (unsigned char)(sum);
            }
        }
    }
    free(kernel);
    free(temp);
    return data;
}

unsigned char* MidtermGaussianBlurImage(unsigned char* data, int width, int height, int channels) {
    const int kernelSize = 5; // Fixed kernel size for midterm blur
    const double sigma = 1.0; // Default sigma for Gaussian blur

    float* kernel = generateGaussianKernel(kernelSize, sigma);
    unsigned char* temp = (unsigned char*)malloc(width * height * channels);
    memcpy(temp, data, width * height * channels);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            for (int c = 0; c < channels; ++c) {
                float sum = 0.0f;
                for (int k = -kernelSize / 2; k <= kernelSize / 2; ++k) {
                    int posX = x + k;
                    if (posX < 0) posX = -posX;
                    if (posX >= width) posX = 2 * width - posX - 1;
                    sum += temp[(y * width + posX) * channels + c] * kernel[k + kernelSize / 2];
                }
                data[(y * width + x) * channels + c] = (unsigned char)(sum);
            }
        }
    }
    free(kernel);
    free(temp);
    return data;
}

unsigned char* InverseImage(unsigned char* data, int width, int height, int channels) {
    for (int i = 0; i < width * height * channels; ++i) {
        data[i] = 255 - data[i];
    }
    return data;
}

unsigned char* RgbToGray8bit(unsigned char* data, int width, int height) {
    unsigned char* grayData = (unsigned char*)malloc(width * height);
    for (int i = 0; i < width * height; ++i) {
        grayData[i] = (unsigned char)(0.299 * data[i * 3] + 0.587 * data[i * 3 + 1] + 0.114 * data[i * 3 + 2]);
    }
    return grayData;
}

unsigned char* AdaptiveThresholdImage(unsigned char* data, int width, int height) {
    const int windowSize = 15;
    const int C = 5;
    unsigned char* tempData = (unsigned char*)malloc(width * height);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int sum = 0, count = 0;
            for (int wy = -windowSize / 2; wy <= windowSize / 2; ++wy) {
                for (int wx = -windowSize / 2; wx <= windowSize / 2; ++wx) {
                    int ny = y + wy;
                    int nx = x + wx;
                    if (ny < 0) ny = -ny;
                    if (ny >= height) ny = 2 * height - ny - 1;
                    if (nx < 0) nx = -nx;
                    if (nx >= width) nx = 2 * width - nx - 1;
                    sum += data[ny * width + nx];
                    ++count;
                }
            }
            int mean = sum / count;
            tempData[y * width + x] = (data[y * width + x] > mean - C) ? 255 : 0;
        }
    }
    memcpy(data, tempData, width * height);
    free(tempData);
    return data;
}

// Function to apply Sobel filter on an 8-bit grayscale image
unsigned char* applySobel8bit(unsigned char* data, int width, int height) {
    int gx, gy, g;
    int sumX, sumY;

    unsigned char* output = (unsigned char*)malloc(width * height * sizeof(unsigned char));
    // Iterate over each pixel in the image (skipping edges)
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            sumX = 0;
            sumY = 0;

            // Apply Sobel kernel for both x and y gradients
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    int pixel = data[(y + ky) * width + (x + kx)];
                    sumX += SobelX[ky + 1][kx + 1] * pixel;
                    sumY += SobelY[ky + 1][kx + 1] * pixel;
                }
            }

            // Calculate the gradient magnitude
            g = (int)sqrt(sumX * sumX + sumY * sumY);
            if (g > 255) g = 255; // Clamp the value to 255 if it exceeds

            output[y * width + x] = (unsigned char)g;
        }
    }
    return output;
}

// Function to apply Sobel filter on a 24-bit color image
unsigned char* applySobel24bit(unsigned char* data, int width, int height) {
    if (data == NULL || width <= 0 || height <= 0) {
        fprintf(stderr, "Error: Invalid input parameters.\n");
        return NULL;
    }

    int rowSize = (width * 3 + 3) & ~3; // Align rows to 4-byte boundaries
    unsigned char* output = (unsigned char*)malloc(rowSize * height);
    if (output == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return NULL;
    }

    // Sobel kernels
    int SobelX[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };
    int SobelY[3][3] = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };

    // Apply Sobel filter
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            int sumXr = 0, sumYr = 0, sumXg = 0, sumYg = 0, sumXb = 0, sumYb = 0;

            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    int ny = y + ky;
                    int nx = x + kx;

                    // Clamp indices to image bounds
                    if (ny < 0 || ny >= height || nx < 0 || nx >= width) {
                        continue;
                    }

                    int idx = ny * rowSize + nx * 3;
                    int r = data[idx];
                    int g = data[idx + 1];
                    int b = data[idx + 2];

                    sumXr += SobelX[ky + 1][kx + 1] * r;
                    sumYr += SobelY[ky + 1][kx + 1] * r;
                    sumXg += SobelX[ky + 1][kx + 1] * g;
                    sumYg += SobelY[ky + 1][kx + 1] * g;
                    sumXb += SobelX[ky + 1][kx + 1] * b;
                    sumYb += SobelY[ky + 1][kx + 1] * b;
                }
            }

            // Gradient magnitudes
            int gR = (int)sqrt(sumXr * sumXr + sumYr * sumYr);
            int gG = (int)sqrt(sumXg * sumXg + sumYg * sumYg);
            int gB = (int)sqrt(sumXb * sumXb + sumYb * sumYb);

            // Clamp values to 0-255
            gR = (gR > 255) ? 255 : gR;
            gG = (gG > 255) ? 255 : gG;
            gB = (gB > 255) ? 255 : gB;

            int idx = y * rowSize + x * 3;
            output[idx] = (unsigned char)gR;
            output[idx + 1] = (unsigned char)gG;
            output[idx + 2] = (unsigned char)gB;
        }
    }

    return output;
}

unsigned char* SobelFilterImage(unsigned char* data, int width, int height, int channels) {
    width = width * channels + 4 - (width % 4);
    if (channels == 1) {
        return applySobel8bit(data, width, height);
    }
    else
    {
        return applySobel24bit(data, width, height);
    }
}

unsigned char* MemCopy(unsigned char* data, int width, int height, int channels) {
    unsigned char* copyData = (unsigned char*)malloc(width * height * channels);
    memcpy(copyData, data, width * height * channels);
    return copyData;
}

// Function to compute the Otsu threshold
unsigned char otsu_threshold(unsigned char* image, int width, int height) {
    int histogram[MAX_GRAY_LEVELS] = { 0 };
    int totalPixels = width * height;

    // Step 1: Compute the histogram of the image
    for (int i = 0; i < totalPixels; i++) {
        histogram[image[i]]++;
    }

    // Step 2: Calculate total sum of pixel intensities
    float totalSum = 0;
    for (int i = 0; i < MAX_GRAY_LEVELS; i++) {
        totalSum += i * histogram[i];
    }

    float sumB = 0; // Sum of the background
    int weightB = 0; // Weight of the background
    int weightF = 0; // Weight of the foreground
    float maxBetweenClassVariance = 0;
    unsigned char bestThreshold = 0;

    // Step 3: Iterate over all possible thresholds (t)
    for (int t = 0; t < MAX_GRAY_LEVELS; t++) {
        weightB += histogram[t];  // Background weight
        if (weightB == 0) continue;

        weightF = totalPixels - weightB;  // Foreground weight
        if (weightF == 0) break;

        sumB += (float)t * histogram[t];  // Sum of background intensities
        float meanB = sumB / weightB;  // Mean of background
        float meanF = (totalSum - sumB) / weightF;  // Mean of foreground

        // Step 4: Calculate the between-class variance
        float betweenClassVariance = (float)weightB * (float)weightF * (meanB - meanF) * (meanB - meanF);
        if (betweenClassVariance > maxBetweenClassVariance) {
            maxBetweenClassVariance = betweenClassVariance;
            bestThreshold = t;
        }
    }

    // Return the best threshold value
    return bestThreshold;
}

// Function to apply Otsu's binarization for 8-bit grayscale images
unsigned char* applyOtsuBinarization8Bit(unsigned char* image, int width, int height) {
    unsigned char* outputImage = (unsigned char*)malloc(width * height * sizeof(unsigned char));
    unsigned char threshold = otsu_threshold(image, width, height);

    // Apply threshold to binarize the image
    for (int i = 0; i < width * height; i++) {
        outputImage[i] = (image[i] > threshold) ? 255 : 0;
    }
    return outputImage;
}

// Function to apply Otsu's binarization for 24-bit RGB images
unsigned char* applyOtsuBinarization24Bit(unsigned char* rgbImage, int width, int height, int channels) {
    unsigned char* grayscaleImage = NULL;

    if (channels == 3) {
        // Convert RGB to grayscale
        grayscaleImage = RgbToGray8bit(rgbImage, width, height);
    }
    else {
        // If it's already grayscale
        grayscaleImage = rgbImage;
    }

    // Apply Otsu's binarization on the grayscale image
    unsigned char* outputImage = applyOtsuBinarization8Bit(grayscaleImage, width, height);

    // Free allocated memory if it's an RGB image
    if (channels == 3) {
        free(grayscaleImage);
    }
    return outputImage;
}

unsigned char* ApplyOtsuBinarization(unsigned char* data, int width, int height, int channels) {
    return applyOtsuBinarization24Bit(data, width, height, channels);
}

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

// Main function to find the reference marker in the image
unsigned char* findReferenceMarker(unsigned char* markData, int markWidth, int markHeight, int markChannels, int markStride,
    unsigned char* currData, int currWidth, int currHeight, int currChannels, int currStride) {

    // Binarize the template and reference images
    unsigned char* biMarkData = TransferBinarizeImage(markData, markWidth, markHeight, markChannels);
    unsigned char* biCurrData = TransferBinarizeImage(currData, currWidth, currHeight, currChannels);

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

    // Extract candidate contours from the reference image
    Point candContour[MAX_POINTS];
    int candContourSize = extractContoursSIMD(biCurrData, currWidth, currHeight, currStride, candContour, MAX_POINTS);
    if (candContourSize == 0) {
        fprintf(stderr, "Error: No contours found in the reference image.\n");
        return refRGB;
    }

    // Match template contour with candidate contours
    double bestMatch = 1e9;
    int bestIndex = -1;
    for (int i = 0; i < candContourSize; i++) {
        double candFDs[MAX_POINTS];
        computeFourierDescriptorsOptimized(&candContour[i], candContourSize, candFDs);

        double matchScore = matchFourierDescriptorsSSE2(markFDs, candFDs, markContourSize);
        if (matchScore < bestMatch) {
            bestMatch = matchScore;
            bestIndex = i;
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
        if (minX < 0 || minY < 0 || maxX >= currWidth || maxY >= currHeight) {
            fprintf(stderr, "Error: Invalid bounding box coordinates.\n");
            return refRGB;
        }

        // Calculate width and height of the bounding box
        int boxWidth = maxX - minX + 1;
        int boxHeight = maxY - minY + 1;

        // Draw the bounding box
        drawRedBoxSSE2(refRGB, currWidth, currHeight, 3, rgbStride, minX, minY, boxWidth, boxHeight);
    }
    else {
        fprintf(stderr, "Error: No matching contour found in the reference image.\n");
    }

    return refRGB;
}