/**
 * @file: denoise_filters.c
 * @author Warren Galyen
 * Created: 10-6-2025
 * Last Updated: 10-6-2025
 * Last update: migrated guided filter from ocular.c
 *
 * @brief Implementation of denoise filters
 */

#include "denoise_filters.h"
#include "util.h"

// Simple clamp function to avoid macro issues
static inline unsigned char clampToByte(float value) {
    if (value < 0.0f) return 0;
    if (value > 255.0f) return 255;
    return (unsigned char)(value + 0.5f);
}

// High-performance separable box filter for float arrays (based on ocularBoxBlurFilter)
static inline void boxFilterFloatRow(const float* input, float* output, int width, int height, int channels, int radius) {
    float scale = 1.0f / (2 * radius + 1);
    
    switch (channels) {
    case 1: {
        for (int y = 0; y < height; y++) {
            // Initialize sum for first pixel
            float sum = 0.0f;
            int count = 0;
            
            // Build initial window
            for (int x = 0; x < width && x <= radius; x++) {
                sum += input[y * width + x];
                count++;
            }
            output[y * width] = sum / count;
            
            // Process remaining pixels with sliding window
            for (int x = 1; x < width; x++) {
                // Remove leftmost pixel if it was in the window
                if (x - radius - 1 >= 0) {
                    sum -= input[y * width + x - radius - 1];
                    count--;
                }
                
                // Add rightmost pixel if it's within bounds
                if (x + radius < width) {
                    sum += input[y * width + x + radius];
                    count++;
                }
                
                output[y * width + x] = sum / count;
            }
        }
        break;
    }
    case 3: {
        for (int y = 0; y < height; y++) {
            // Initialize sums for first pixel
            float sumR = 0.0f, sumG = 0.0f, sumB = 0.0f;
            int count = 0;
            
            // Build initial window
            for (int x = 0; x < width && x <= radius; x++) {
                int idx = (y * width + x) * 3;
                sumR += input[idx];
                sumG += input[idx + 1];
                sumB += input[idx + 2];
                count++;
            }
            int leftIdx = y * width * 3;
            output[leftIdx] = sumR / count;
            output[leftIdx + 1] = sumG / count;
            output[leftIdx + 2] = sumB / count;
            
            // Process remaining pixels with sliding window
            for (int x = 1; x < width; x++) {
                // Remove leftmost pixel if it was in the window
                if (x - radius - 1 >= 0) {
                    int leftIdx = (y * width + x - radius - 1) * 3;
                    sumR -= input[leftIdx];
                    sumG -= input[leftIdx + 1];
                    sumB -= input[leftIdx + 2];
                    count--;
                }
                
                // Add rightmost pixel if it's within bounds
                if (x + radius < width) {
                    int rightIdx = (y * width + x + radius) * 3;
                    sumR += input[rightIdx];
                    sumG += input[rightIdx + 1];
                    sumB += input[rightIdx + 2];
                    count++;
                }
                
                int outIdx = (y * width + x) * 3;
                output[outIdx] = sumR / count;
                output[outIdx + 1] = sumG / count;
                output[outIdx + 2] = sumB / count;
            }
        }
        break;
    }
    default: {
        // Generic case for any number of channels
        for (int y = 0; y < height; y++) {
            for (int c = 0; c < channels; c++) {
                // Initialize sum for first pixel
                float sum = 0.0f;
                int count = 0;
                
                // Build initial window
                for (int x = 0; x < width && x <= radius; x++) {
                    sum += input[(y * width + x) * channels + c];
                    count++;
                }
                output[(y * width) * channels + c] = sum / count;
                
                // Process remaining pixels with sliding window
                for (int x = 1; x < width; x++) {
                    // Remove leftmost pixel if it was in the window
                    if (x - radius - 1 >= 0) {
                        sum -= input[(y * width + x - radius - 1) * channels + c];
                        count--;
                    }
                    
                    // Add rightmost pixel if it's within bounds
                    if (x + radius < width) {
                        sum += input[(y * width + x + radius) * channels + c];
                        count++;
                    }
                    
                    output[(y * width + x) * channels + c] = sum / count;
                }
            }
        }
        break;
    }
    }
}

static inline void boxFilterFloatCol(const float* input, float* output, int width, int height, int channels, int radius) {
    switch (channels) {
    case 1: {
        for (int x = 0; x < width; x++) {
            // Initialize sum for first pixel
            float sum = 0.0f;
            int count = 0;
            
            // Build initial window
            for (int y = 0; y < height && y <= radius; y++) {
                sum += input[y * width + x];
                count++;
            }
            output[x] = sum / count;
            
            // Process remaining pixels with sliding window
            for (int y = 1; y < height; y++) {
                // Remove topmost pixel if it was in the window
                if (y - radius - 1 >= 0) {
                    sum -= input[(y - radius - 1) * width + x];
                    count--;
                }
                
                // Add bottommost pixel if it's within bounds
                if (y + radius < height) {
                    sum += input[(y + radius) * width + x];
                    count++;
                }
                
                output[y * width + x] = sum / count;
            }
        }
        break;
    }
    case 3: {
        for (int x = 0; x < width; x++) {
            // Initialize sums for first pixel
            float sumR = 0.0f, sumG = 0.0f, sumB = 0.0f;
            int count = 0;
            
            // Build initial window
            for (int y = 0; y < height && y <= radius; y++) {
                int idx = (y * width + x) * 3;
                sumR += input[idx];
                sumG += input[idx + 1];
                sumB += input[idx + 2];
                count++;
            }
            output[x * 3] = sumR / count;
            output[x * 3 + 1] = sumG / count;
            output[x * 3 + 2] = sumB / count;
            
            // Process remaining pixels with sliding window
            for (int y = 1; y < height; y++) {
                // Remove topmost pixel if it was in the window
                if (y - radius - 1 >= 0) {
                    int topIdx = ((y - radius - 1) * width + x) * 3;
                    sumR -= input[topIdx];
                    sumG -= input[topIdx + 1];
                    sumB -= input[topIdx + 2];
                    count--;
                }
                
                // Add bottommost pixel if it's within bounds
                if (y + radius < height) {
                    int bottomIdx = ((y + radius) * width + x) * 3;
                    sumR += input[bottomIdx];
                    sumG += input[bottomIdx + 1];
                    sumB += input[bottomIdx + 2];
                    count++;
                }
                
                int outIdx = (y * width + x) * 3;
                output[outIdx] = sumR / count;
                output[outIdx + 1] = sumG / count;
                output[outIdx + 2] = sumB / count;
            }
        }
        break;
    }
    default: {
        // Generic case for any number of channels
        for (int x = 0; x < width; x++) {
            for (int c = 0; c < channels; c++) {
                // Initialize sum for first pixel
                float sum = 0.0f;
                int count = 0;
                
                // Build initial window
                for (int y = 0; y < height && y <= radius; y++) {
                    sum += input[(y * width + x) * channels + c];
                    count++;
                }
                output[x * channels + c] = sum / count;
                
                // Process remaining pixels with sliding window
                for (int y = 1; y < height; y++) {
                    // Remove topmost pixel if it was in the window
                    if (y - radius - 1 >= 0) {
                        sum -= input[((y - radius - 1) * width + x) * channels + c];
                        count--;
                    }
                    
                    // Add bottommost pixel if it's within bounds
                    if (y + radius < height) {
                        sum += input[((y + radius) * width + x) * channels + c];
                        count++;
                    }
                    
                    output[(y * width + x) * channels + c] = sum / count;
                }
            }
        }
        break;
    }
    }
}

// High-performance separable box filter using float arrays (for guided filter)
static void boxFilterFloat(float* input, float* output, int width, int height, int channels, int radius) {
    int size = width * height * channels;
    float* temp = (float*)malloc(size * sizeof(float));
    if (!temp) {
        memcpy(output, input, size * sizeof(float));
        return;
    }

    // Apply separable box filter: horizontal pass then vertical pass
    boxFilterFloatRow(input, temp, width, height, channels, radius);
    boxFilterFloatCol(temp, output, width, height, channels, radius);
    
    free(temp);
}


// Grayscale guided filter
static OC_STATUS guidedFilterGray(unsigned char* Input, unsigned char* Guide, unsigned char* Output, int Width, int Height, int Stride,
                                  int Radius, float Epsilon) {
    int size = Width * Height;

    float* I = (float*)malloc(size * sizeof(float));
    float* P = (float*)malloc(size * sizeof(float));
    float* meanI = (float*)malloc(size * sizeof(float));
    float* meanP = (float*)malloc(size * sizeof(float));
    float* meanII = (float*)malloc(size * sizeof(float));
    float* meanIP = (float*)malloc(size * sizeof(float));
    float* a = (float*)malloc(size * sizeof(float));
    float* b = (float*)malloc(size * sizeof(float));
    float* meanA = (float*)malloc(size * sizeof(float));
    float* meanB = (float*)malloc(size * sizeof(float));

    if (!I || !P || !meanI || !meanP || !meanII || !meanIP || !a || !b || !meanA || !meanB) {
        OC_STATUS status = OC_STATUS_ERR_OUTOFMEMORY;
        goto cleanup;
    }

    for (int i = 0; i < size; i++) {
        I[i] = Guide[i] / 255.0f;
        P[i] = Input[i] / 255.0f;
        meanI[i] = I[i];
        meanP[i] = P[i];
        meanII[i] = I[i] * I[i];
        meanIP[i] = I[i] * P[i];
    }

    boxFilterFloat(meanI, meanI, Width, Height, 1, Radius);
    boxFilterFloat(meanP, meanP, Width, Height, 1, Radius);
    boxFilterFloat(meanII, meanII, Width, Height, 1, Radius);
    boxFilterFloat(meanIP, meanIP, Width, Height, 1, Radius);

    for (int i = 0; i < size; i++) {
        float varI = meanII[i] - meanI[i] * meanI[i];
        a[i] = (meanIP[i] - meanI[i] * meanP[i]) / (varI + Epsilon);
        b[i] = meanP[i] - a[i] * meanI[i];
    }

    boxFilterFloat(a, meanA, Width, Height, 1, Radius);
    boxFilterFloat(b, meanB, Width, Height, 1, Radius);

    for (int i = 0; i < size; i++) {
        float result = meanA[i] * I[i] + meanB[i];
        Output[i] = clampToByte(result * 255.0f);
    }

    OC_STATUS status = OC_STATUS_OK;

cleanup:
    free(I);
    free(P);
    free(meanI);
    free(meanP);
    free(meanII);
    free(meanIP);
    free(a);
    free(b);
    free(meanA);
    free(meanB);
    return status;
}

// Color-aware guided filter
static OC_STATUS guidedFilterColor(unsigned char* Input, unsigned char* Guide, unsigned char* Output, int Width, int Height, int Stride,
                                   int Radius, float Epsilon) {
    int pixelCount = Width * Height;

    // Process each channel independently for simplicity and robustness
    for (int channel = 0; channel < 3; channel++) {
        // Single channel arrays
        float* I = (float*)malloc(pixelCount * sizeof(float));
        float* P = (float*)malloc(pixelCount * sizeof(float));
        float* meanI = (float*)malloc(pixelCount * sizeof(float));
        float* meanP = (float*)malloc(pixelCount * sizeof(float));
        float* meanII = (float*)malloc(pixelCount * sizeof(float));
        float* meanIP = (float*)malloc(pixelCount * sizeof(float));
        float* a = (float*)malloc(pixelCount * sizeof(float));
        float* b = (float*)malloc(pixelCount * sizeof(float));
        float* meanA = (float*)malloc(pixelCount * sizeof(float));
        float* meanB = (float*)malloc(pixelCount * sizeof(float));

        if (!I || !P || !meanI || !meanP || !meanII || !meanIP || !a || !b || !meanA || !meanB) {
            // Cleanup and continue to next channel
            free(I);
            free(P);
            free(meanI);
            free(meanP);
            free(meanII);
            free(meanIP);
            free(a);
            free(b);
            free(meanA);
            free(meanB);
            continue;
        }

        // Extract channel data
        for (int i = 0; i < pixelCount; i++) {
            int idx = i * 3 + channel;
            I[i] = Guide[idx] / 255.0f;
            P[i] = Input[idx] / 255.0f;
            meanI[i] = I[i];
            meanP[i] = P[i];
            meanII[i] = I[i] * I[i];
            meanIP[i] = I[i] * P[i];
        }

        // Apply box filters
        boxFilterFloat(meanI, meanI, Width, Height, 1, Radius);
        boxFilterFloat(meanP, meanP, Width, Height, 1, Radius);
        boxFilterFloat(meanII, meanII, Width, Height, 1, Radius);
        boxFilterFloat(meanIP, meanIP, Width, Height, 1, Radius);

        // Compute coefficients
        for (int i = 0; i < pixelCount; i++) {
            float varI = meanII[i] - meanI[i] * meanI[i];
            a[i] = (meanIP[i] - meanI[i] * meanP[i]) / (varI + Epsilon);
            b[i] = meanP[i] - a[i] * meanI[i];
        }

        // Average coefficients
        boxFilterFloat(a, meanA, Width, Height, 1, Radius);
        boxFilterFloat(b, meanB, Width, Height, 1, Radius);

        // Apply filter to output
        for (int i = 0; i < pixelCount; i++) {
            int idx = i * 3 + channel;
            float result = meanA[i] * I[i] + meanB[i];
            Output[idx] = clampToByte(result * 255.0f);
        }

        // Cleanup
        free(I);
        free(P);
        free(meanI);
        free(meanP);
        free(meanII);
        free(meanIP);
        free(a);
        free(b);
        free(meanA);
        free(meanB);
    }

    return OC_STATUS_OK;
}

OC_STATUS ocularGuidedFilter(unsigned char* Input, unsigned char* Guide, unsigned char* Output, int Width, int Height, int Stride,
                             int Radius, float Epsilon) {

    if (!Input || !Output || Width <= 0 || Height <= 0 || Radius < 1)
        return OC_STATUS_ERR_INVALIDPARAMETER;

    // If no guide is provided, use input as guide
    if (!Guide)
        Guide = Input;

    int Channels = Stride / Width;
    if (Channels != 1 && Channels != 3)
        return OC_STATUS_ERR_INVALIDPARAMETER;
    
    Epsilon = clamp(Epsilon, 0.001f, 0.4f);

    if (Channels == 3) {
        // Color image - use full color-aware guided filter
        return guidedFilterColor(Input, Guide, Output, Width, Height, Stride, Radius, Epsilon);
    } else {
        // Grayscale - use original per-channel approach
        return guidedFilterGray(Input, Guide, Output, Width, Height, Stride, Radius, Epsilon);
    }
}