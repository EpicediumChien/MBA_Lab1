#include "NImgProcess.h"
#include "NImageProcess.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_GRAY_LEVELS 256

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

DLL_EXPORT unsigned char* ApplyOtsuBinarization(unsigned char* data, int width, int height, int channels) {
    return applyOtsuBinarization24Bit(data, width, height, channels);
}