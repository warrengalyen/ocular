#ifndef OCULAR_UTIL_H
#define OCULAR_UTIL_H

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#define float2fixed(x) (((int)((x) * 4096.0f + 0.5f)) << 8)

struct LineParameter {
    float angle;
    float distance;
};

static inline unsigned long byteswap_ulong(unsigned long i) {
    unsigned long j;
    j = (i << 24);
    j += (i << 8) & 0x00FF0000;
    j += (i >> 8) & 0x0000FF00;
    j += (i >> 24);
    return j;
}

static inline unsigned short byteswap_ushort(unsigned short i) {
    unsigned short j;
    j = (i << 8);
    j += (i >> 8);
    return j;
}

static inline unsigned char step(unsigned char edge, unsigned char x) { return (unsigned char)(x < edge ? 0 : 255); }

static inline int Abs(int x) { return (x ^ (x >> 31)) - (x >> 31); }

static unsigned char mix_u8(unsigned char a, unsigned char b, float alpha) {
    return (unsigned char)ClampToByte(a * (1.0f - alpha) + b * alpha);
}

static float dot(unsigned char R, unsigned char G, unsigned char B, float fR, float fG, float fB) {
    return (float)(R * fR + G * fG + B * fB);
}

static inline float mix(float a, float b, float alpha) { return (a * (1.0f - alpha) + b * alpha); }

static unsigned char degree(unsigned char InputColor, unsigned char OutputColor, float intensity) {
    return (unsigned char)ClampToByte(((intensity * OutputColor) + (1.0f - intensity) * InputColor));
}

static float smoothstep(float edgeMin, float edgeMax, float x) { return clamp((x - edgeMin) / (edgeMax - edgeMin), 0.0f, 1.0f); }

static float vec2_distance(float vecX, float vecY, float otherX, float otherY) {
    float dx = vecX - otherX;
    float dy = vecY - otherY;
    return sqrtf(dx * dx + dy * dy);
}

// Normalize a kernel so that it produces correct colors.
static void normalizeKernel(float* kernel, int kernelWidth) {
    int sum = 0;
    int total = kernelWidth * kernelWidth;
    for (int i = 0; i < total; i++) {
        sum += kernel[i];
    }
    for (int i = 0; i < total && sum != 0; i++) {
        kernel[i] /= sum;
    }
}

static void autoLevel(const unsigned int* histogram, unsigned char* remapLut, int numberOfPixels, float cutLimit, float contrast) {
    int minPos = 0, maxPos = 255;
    int minValue = 0, maxValue = 255;
    for (int I = 0; I < 256; I++) {
        if (histogram[I] != 0) {
            minValue = I;
            break;
        }
    }
    for (int I = 255; I >= 0; I--) {
        if (histogram[I] != 0) {
            maxValue = I;
            break;
        }
    }
    int sum = 0;
    for (int I = minValue; I < 256; I++) {
        sum = sum + histogram[I];
        if (sum >= numberOfPixels * cutLimit) {
            minPos = I;
            break;
        }
    }
    sum = 0;
    for (int I = 255; I >= 0; I--) {
        sum = sum + histogram[I];
        if (sum >= numberOfPixels * cutLimit) {
            maxPos = I;
            break;
        }
    }

    int delta = (int)((maxValue - minValue) * contrast * 0.5f);
    minValue = ClampToByte(minValue - delta);
    maxValue = ClampToByte(maxValue + delta);
    if (maxPos != minPos) {
        for (int I = 0; I < 256; I++) {
            if (I < minPos)
                remapLut[I] = (unsigned char)minValue;
            else if (I > maxPos)
                remapLut[I] = (unsigned char)maxValue;
            else
                remapLut[I] = (unsigned char)ClampToByte((maxValue - minValue) * (I - minPos) / (maxPos - minPos) + minValue);
        }
    } else {
        for (int I = 0; I < 256; I++) {
            remapLut[I] = (unsigned char)maxPos;
        }
    }
}

static bool isColorCast(const unsigned int* histogramCb, const unsigned int* histogramCr, int numberOfPixels, int colorCoeff) {
    unsigned int sumCb = 0;
    unsigned int sumCr = 0;
    float meanCb = 0, meanCr = 0;
    for (unsigned int i = 0; i < 256; i++) {
        sumCb += histogramCb[i] * i;
        sumCr += histogramCr[i] * i;
    }
    meanCb = sumCb * (1.0f / numberOfPixels);
    meanCr = sumCr * (1.0f / numberOfPixels);
    int avgColorCoeff = (abs(meanCb - 127) + abs(meanCr - 127));
    if (avgColorCoeff < colorCoeff) {
        return false;
    }
    return true;
}

static int getDiffFactor(const unsigned char* color1, const unsigned char* color2, const int channels) {
    int final_diff;
    int component_diff[4];

    // find absolute difference between each component
    for (int i = 0; i < channels; i++) {
        component_diff[i] = abs(color1[i] - color2[i]);
    }

    // based on number of components, produce a single difference value in the 0-255 range
    switch (channels) {
    case 1: final_diff = component_diff[0]; break;

    case 2: final_diff = ((component_diff[0] + component_diff[1]) >> 1); break;

    case 3: final_diff = ((component_diff[0] + component_diff[2]) >> 2) + (component_diff[1] >> 1); break;

    case 4: final_diff = ((component_diff[0] + component_diff[1] + component_diff[2] + component_diff[3]) >> 2); break;

    default: final_diff = 0;
    }

    return final_diff;
}

static void CalGaussianCoeff(float sigma, float* a0, float* a1, float* a2, float* a3, float* b1, float* b2, float* cprev, float* cnext) {
    float alpha, lamma, k;

    if (sigma < 0.5f)
        sigma = 0.5f;
    alpha = (float)expf((0.726) * (0.726)) / sigma;
    lamma = (float)expf(-alpha);
    *b2 = (float)expf(-2 * alpha);
    k = (1 - lamma) * (1 - lamma) / (1 + 2 * alpha * lamma - (*b2));
    *a0 = k;
    *a1 = k * (alpha - 1) * lamma;
    *a2 = k * (alpha + 1) * lamma;
    *a3 = -k * (*b2);
    *b1 = -2 * lamma;
    *cprev = (*a0 + *a1) / (1 + *b1 + *b2);
    *cnext = (*a2 + *a3) / (1 + *b1 + *b2);
}

// Add extra rows and columns of zeroes to the edges of an image.
// The zeros are added to the borders of the image so that the size of the image is increased,
// but the original content remains unchanged. This is commonly used in convolution.
static void zeroPadding(unsigned char* input, unsigned char* output, int row, int col) {
    int inputCols = col - 2; // input number of column

    for (int i = 1; i < row - 1; i++) {
        for (int j = 1; j < col - 1; j++) {
            *(output + i * col + j) = *(input + (i - 1) * inputCols + (j - 1));
        }
    }
}

static int FindArrayMax(unsigned int* Array, int numElements) {
    int N = 0; // N is used to indicate the order of the element int the array which has the max value: Array[N] is the maximum in Array
    for (int i = 1; i < numElements; i++) {
        if (Array[N] >= Array[i])
            N = N;
        else
            N = i;
    }

    return N;
}

static void FindMaxVote(unsigned int** VoteTable, int numAngles, int numDistances, int* M, int* N) {
    unsigned int* maxVoteDiffAngles = (unsigned int*)malloc(numAngles * sizeof(unsigned int));

    for (int i = 0; i < numAngles; i++) {
        int j = FindArrayMax(VoteTable[i], numDistances);
        maxVoteDiffAngles[i] = VoteTable[i][j];
    }

    *M = FindArrayMax(maxVoteDiffAngles, numAngles);
    *N = FindArrayMax(VoteTable[*M], numDistances);

    free(maxVoteDiffAngles);
}

static void houghTransformLine(unsigned char* input, float minAngle, float angleInterval, int numAngles, float minDistance,
                               float distanceInterval, int NumDistances, unsigned int** VoteTable, int width, int height) {
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if (*(input + y * width + x) == 255) {
                for (int i = 0; i < numAngles; i++) {
                    float angle = minAngle + angleInterval * ((float)i);
                    float distance = ((float)x) * cos(angle / 180 * M_PI) + ((float)y) * sin(angle / 180 * M_PI);
                    int j = (int)((distance - minDistance) / distanceInterval);
                    if (j < NumDistances) {
                        VoteTable[i][j] = ((VoteTable[i][j]) + 1);
                    }
                }
            }
        }
    }
}


#endif // OCULAR_UTIL_H
