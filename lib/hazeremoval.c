#include "hazeremoval.h"
#include "core.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

// Applies minimum filter horizontally across image rows with specified radius
void minFilterHorizontal(float* input, float* output, int width, int height, int radius) {
    if (!input || !output || width <= 0 || height <= 0)
        return;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int startX = (x - radius < 0) ? 0 : x - radius;
            int endX = (x + radius >= width) ? width - 1 : x + radius;

            if (startX <= endX && startX >= 0 && endX < width) {
                float minVal = input[y * width + startX]; // Initialize with first valid value
                for (int kx = startX + 1; kx <= endX; kx++) {
                    minVal = fminf(minVal, input[y * width + kx]);
                }
                output[y * width + x] = minVal;
            } else {
                output[y * width + x] = (startX >= 0 && startX < width) ? input[y * width + startX] : 0.0f;
            }
        }
    }
}

// Applies minimum filter vertically across image columns with specified radius
void minFilterVertical(float* input, float* output, int width, int height, int radius) {
    if (!input || !output || width <= 0 || height <= 0)
        return;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int startY = (y - radius < 0) ? 0 : y - radius;
            int endY = (y + radius >= height) ? height - 1 : y + radius;

            if (startY <= endY && startY >= 0 && endY < height) {
                float minVal = input[startY * width + x]; // Initialize with first valid value
                for (int ky = startY + 1; ky <= endY; ky++) {
                    minVal = fminf(minVal, input[ky * width + x]);
                }
                output[y * width + x] = minVal;
            } else {
                output[y * width + x] = (startY >= 0 && startY < height) ? input[startY * width + x] : 0.0f;
            }
        }
    }
}

// Computes dark channel prior by finding minimum RGB values in local neighborhoods using separable filters
void calculateDarkChannelFast(unsigned char* input, float* darkChannel, int width, int height, int stride, int radius) {
    int channels = stride / width;
    int size = width * height;

    // First, find minimum across RGB channels for each pixel
    for (int i = 0; i < size; i++) {
        int imgIdx = (i / width) * stride + (i % width) * channels;
        float pixelMin = (float)input[imgIdx] / 255.0f; // R
        if (channels >= 3) {
            pixelMin = fminf(pixelMin, (float)input[imgIdx + 1] / 255.0f); // G
            pixelMin = fminf(pixelMin, (float)input[imgIdx + 2] / 255.0f); // B
        }
        darkChannel[i] = pixelMin;
    }

    // Apply separable minimum filter
    float* temp = (float*)malloc(size * sizeof(float));
    if (temp) {
        minFilterHorizontal(darkChannel, temp, width, height, radius);
        minFilterVertical(temp, darkChannel, width, height, radius);
        free(temp);
    }
}

// Estimates atmospheric light value by finding brightest pixels in top 0.1% of dark channel
float estimateAtmosphericLight(unsigned char* input, float* darkChannel, int width, int height, int stride, float maxAtm) {
    int channels = stride / width;
    int totalPixels = width * height;

    // Find the brightest pixels in the dark channel (top 0.1%)
    int topPixelCount = (int)(totalPixels * 0.001f);
    if (topPixelCount < 1)
        topPixelCount = 1;

    // Create array of dark channel values with positions
    typedef struct {
        float value;
        int index;
    } PixelValue;

    PixelValue* pixels = (PixelValue*)malloc(totalPixels * sizeof(PixelValue));
    if (!pixels)
        return 0.8f; // fallback value

    for (int i = 0; i < totalPixels; i++) {
        pixels[i].value = darkChannel[i];
        pixels[i].index = i;
    }

    // Simple selection sort for top pixels (efficient for small counts)
    int sortCount = (topPixelCount < totalPixels) ? topPixelCount : totalPixels;
    for (int i = 0; i < sortCount - 1; i++) {
        int maxIdx = i;
        for (int j = i + 1; j < totalPixels; j++) {
            if (pixels[j].value > pixels[maxIdx].value) {
                maxIdx = j;
            }
        }
        if (maxIdx != i) {
            PixelValue temp = pixels[i];
            pixels[i] = pixels[maxIdx];
            pixels[maxIdx] = temp;
        }
    }

    // Find the pixel with highest intensity among the top dark channel pixels
    float maxIntensity = 0.0f;
    int actualCount = (topPixelCount < totalPixels) ? topPixelCount : totalPixels;
    for (int i = 0; i < actualCount; i++) {
        int pixelIdx = pixels[i].index;
        int y = pixelIdx / width;
        int x = pixelIdx % width;
        int imgIdx = y * stride + x * channels;

        float intensity = (float)input[imgIdx]; // R
        if (channels >= 3) {
            intensity = fmaxf(intensity, (float)input[imgIdx + 1]); // G
            intensity = fmaxf(intensity, (float)input[imgIdx + 2]); // B
        }

        maxIntensity = fmaxf(maxIntensity, intensity);
    }

    free(pixels);

    float atmLight = maxIntensity / 255.0f;
    return fminf(atmLight, maxAtm);
}

// Estimates transmission map by normalizing image by atmospheric light and applying minimum filter
void estimateTransmissionFast(unsigned char* input, float* transmission, int width, int height, int stride, float atmLight,
                              int radius, float omega) {
    int channels = stride / width;
    int size = width * height;

    // First, normalize by atmospheric light and find minimum across RGB channels
    for (int i = 0; i < size; i++) {
        int imgIdx = (i / width) * stride + (i % width) * channels;
        float pixelMin = (float)input[imgIdx] / (255.0f * atmLight); // R
        if (channels >= 3) {
            pixelMin = fminf(pixelMin, (float)input[imgIdx + 1] / (255.0f * atmLight)); // G
            pixelMin = fminf(pixelMin, (float)input[imgIdx + 2] / (255.0f * atmLight)); // B
        }
        transmission[i] = pixelMin;
    }

    // Apply separable minimum filter
    float* temp = (float*)malloc(size * sizeof(float));
    if (temp) {
        minFilterHorizontal(transmission, temp, width, height, radius);
        minFilterVertical(temp, transmission, width, height, radius);
        free(temp);
    }

    // Apply omega factor and compute final transmission
    for (int i = 0; i < size; i++) {
        transmission[i] = 1.0f - omega * transmission[i];
    }
}

// Applies separable box filter for fast mean filtering with specified radius
void boxFilterFast(float* input, float* output, int width, int height, int radius) {
    int size = width * height;
    float* temp = (float*)malloc(size * sizeof(float));
    if (!temp) {
        memcpy(output, input, size * sizeof(float));
        return;
    }

    // Horizontal pass
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float sum = 0.0f;
            int count = 0;
            int startX = (x - radius < 0) ? 0 : x - radius;
            int endX = (x + radius >= width) ? width - 1 : x + radius;

            for (int kx = startX; kx <= endX; kx++) {
                sum += input[y * width + kx];
                count++;
            }
            temp[y * width + x] = (count > 0) ? sum / count : 0.0f;
        }
    }

    // Vertical pass
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float sum = 0.0f;
            int count = 0;
            int startY = (y - radius < 0) ? 0 : y - radius;
            int endY = (y + radius >= height) ? height - 1 : y + radius;

            for (int ky = startY; ky <= endY; ky++) {
                sum += temp[ky * width + x];
                count++;
            }
            output[y * width + x] = (count > 0) ? sum / count : 0.0f;
        }
    }

    free(temp);
}

// Performs edge-preserving smoothing using guided filter with guide image and regularization parameter
void guidedFilterFast(float* input, float* guide, float* output, int width, int height, int radius, float epsilon) {
    int size = width * height;
    float* meanI = (float*)malloc(size * sizeof(float));
    float* meanP = (float*)malloc(size * sizeof(float));
    float* meanIp = (float*)malloc(size * sizeof(float));
    float* meanII = (float*)malloc(size * sizeof(float));
    float* corrIp = (float*)malloc(size * sizeof(float));
    float* corrII = (float*)malloc(size * sizeof(float));

    if (!meanI || !meanP || !meanIp || !meanII || !corrIp || !corrII) {
        // Fallback: copy input to output
        memcpy(output, input, size * sizeof(float));
        free(meanI);
        free(meanP);
        free(meanIp);
        free(meanII);
        free(corrIp);
        free(corrII);
        return;
    }

    // Compute correlation and cross-correlation
    for (int i = 0; i < size; i++) {
        corrIp[i] = guide[i] * input[i];
        corrII[i] = guide[i] * guide[i];
    }

    // Apply box filter to all terms
    boxFilterFast(guide, meanI, width, height, radius);
    boxFilterFast(input, meanP, width, height, radius);
    boxFilterFast(corrIp, meanIp, width, height, radius);
    boxFilterFast(corrII, meanII, width, height, radius);

    // Calculate linear coefficients
    for (int i = 0; i < size; i++) {
        float varI = meanII[i] - meanI[i] * meanI[i];
        float covIp = meanIp[i] - meanI[i] * meanP[i];

        float a = covIp / (varI + epsilon);
        float b = meanP[i] - a * meanI[i];

        corrIp[i] = a; // Reuse buffer for a
        corrII[i] = b; // Reuse buffer for b
    }

    // Apply box filter to coefficients
    boxFilterFast(corrIp, meanI, width, height, radius); // mean_a
    boxFilterFast(corrII, meanP, width, height, radius); // mean_b

    // Compute final result
    for (int i = 0; i < size; i++) {
        output[i] = meanI[i] * guide[i] + meanP[i];
    }

    free(meanI);
    free(meanP);
    free(meanIp);
    free(meanII);
    free(corrIp);
    free(corrII);
}

// Recovers haze-free scene radiance using transmission map and atmospheric light estimation
void recoverScene(unsigned char* input, unsigned char* output, int width, int height, int stride, float* transmission, float atmLight, float t0) {
    int channels = stride / width;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * stride + x * channels;
            int tIdx = y * width + x;

            float t = fmaxf(transmission[tIdx], t0);

            for (int c = 0; c < channels; c++) {
                float I = (float)input[idx + c] / 255.0f;
                float J = (I - atmLight) / t + atmLight;

                // Clamp result to valid range
                J = fmaxf(0.0f, fminf(1.0f, J));
                output[idx + c] = (unsigned char)(J * 255.0f + 0.5f);
            }
        }
    }
}

