/**
 * @file: util.h
 * @author Warren Galyen
 * Created: 2-12-2024
 * Last Updated: 2-12-2024
 * Last update: moved clamping functions from ocular.h
 *
 * @brief Ocular utility functions that support primary image processing features.
 */

#ifndef OCULAR_UTIL_H
#define OCULAR_UTIL_H

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "fastmath.h"

#ifndef clamp
    #define clamp(value, min, max) ((value) > (max) ? (max) : (value) < (min) ? (min) : (value))
#endif
#ifndef ClampToByte
    #define ClampToByte(v) (unsigned char)(((unsigned)(int)(v)) < (255) ? (v) : (v < 0) ? (0) : (255))
#endif
#ifndef min
    #define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef min3
    #define min3(a, b, c) min(min((a), (b)), (c))
#endif
#ifndef max
    #define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef max3
    #define max3(a, b, c) max(max((a), (b)), (c))
#endif

#ifndef M_PI
    #define M_PI 3.14159265358979323846f
#endif

#define WIDTHBYTES(bytes) (((bytes * 8) + 31) / 32 * 4)

#define float2fixed(x) (((int)((x) * 4096.0f + 0.5f)) << 8)

struct LineParameter {
    float angle;
    float distance;
};

typedef struct {
    int x;
    int y;
    int Width;
    int Height;
} OcRect;

typedef struct {
    float Theta;
    int Radius;
    int Intensity;
    float RelativeIntensity;
} HoughLine;

#ifndef clamp
    #define clamp(value, min, max) ((value) > (max) ? (max) : (value) < (min) ? (min) : (value))
#endif

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

static inline float mixColor(float a, float b, float t) { return a * (1.0f - t) + b * t; }

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

// Determine if an image is primarily text based.
static bool isTextImage(unsigned char* Input, int Width, int Height) {
    const int blacklimit = 20;
    const int greylimit = 140;
    const int contrast_offset = 80;

    int prev_color[256];
    int cur_color[256];

    for (int i = 0; i < 256; i++) {
        cur_color[i] = 0;
        prev_color[i] = 0;
    }

    for (int i = 0; i <= blacklimit; i++) {
        // black
        cur_color[i] = 100;
        prev_color[i] = 100000;
    }

    for (int i = blacklimit + 1 + contrast_offset; i <= greylimit; i++) {
        // gray
        cur_color[i] = 10;
        prev_color[i] = 10000;
    }

    for (int i = greylimit + 1 + contrast_offset; i <= 255; i++) {
        // white
        cur_color[i] = 1;
        prev_color[i] = 1000;
    }
    int line_count = 0;


    int n = -1;
    for (int y = 0; y < Height; y += 10) {
        n++;
        int white_amt = 0;
        unsigned char* buffer = Input + y * Width;
        int x = 0;
        for (x = 1; x < Width; x++) {
            const unsigned char prev_pixel = buffer[(x - 1)];
            const unsigned char cur_pixel = buffer[x];

            if ((prev_color[prev_pixel]) && (cur_color[cur_pixel])) {
                // whether it is white
                if ((prev_color[prev_pixel] + cur_color[cur_pixel]) == 1001) {
                    white_amt++;
                }
            }
        }
        // white row
        if (((float)white_amt / (float)x) > 0.85f) {
            line_count++;
        }
    }

    float line_count_ratio = (n != 0.f) ? (float)line_count / (float)n : 0.0f;

    if (line_count_ratio < 0.4f || line_count_ratio > 1.0f) {
        return false;
    }

    return true;
}

static float calcSkewAngle(unsigned char* Input, int Width, int Height, OcRect* CheckRectPtr, int maxSkewToDetect, int stepsPerDegree,
                           int localPeakRadius, int nLineCount) {
    OcRect CheckRect = *CheckRectPtr;
    // Make sure the specified area is within the original image range
    CheckRect.x = clamp(CheckRect.x, 0, Width - 1);
    CheckRect.y = clamp(CheckRect.y, 0, Height - 1);
    CheckRect.Width = clamp(CheckRect.Width, 1, Width - 1);
    CheckRect.Height = clamp(CheckRect.Height, 1, Height - 1);

    // processing parameters
    maxSkewToDetect = clamp(maxSkewToDetect, 0, 91);
    localPeakRadius = clamp(localPeakRadius, 1, 10);
    stepsPerDegree = clamp(stepsPerDegree, 1, 10);
    int houghHeight = (2 * maxSkewToDetect * stepsPerDegree);
    float thetaStep = (2 * maxSkewToDetect * M_PI / 180) / houghHeight;
    int halfWidth = Width >> 1;
    int halfHeight = Height >> 1;
    // calculate hough map width
    int halfHoughWidth = (int)sqrtf((float)(halfWidth * halfWidth + halfHeight * halfHeight));
    int houghWidth = (halfHoughWidth * 2);
    float minTheta = 90.0f - maxSkewToDetect;
    unsigned short* houghMap = (unsigned short*)calloc(houghHeight * houghWidth, sizeof(unsigned short));
    float* sinMap = (float*)malloc(houghHeight * sizeof(float));
    float* cosMap = (float*)malloc(houghHeight * sizeof(float));
    HoughLine* HoughLines = (HoughLine*)calloc(houghHeight * houghWidth, sizeof(HoughLine));
    if (houghMap == NULL || sinMap == NULL || cosMap == NULL || HoughLines == NULL) {
        if (houghMap) {
            free(houghMap);
            houghMap = NULL;
        }
        if (sinMap) {
            free(sinMap);
            sinMap = NULL;
        }
        if (cosMap) {
            free(cosMap);
            cosMap = NULL;
        }
        if (HoughLines) {
            free(HoughLines);
            HoughLines = NULL;
        }
        return 0.0f;
    } else {
        // precomputed sin and cos tables
        float mt = (minTheta * M_PI / 180.0f);
        for (int i = 0; i < houghHeight; i++) {
            float cur_weight = mt + (i * thetaStep);
            sinMap[i] = fastSin(cur_weight);
            cosMap[i] = fastCos(cur_weight);
        }
    }
    int startX = -halfWidth + CheckRect.x;
    int startY = -halfHeight + CheckRect.y;
    int stopX = Width - halfWidth - (Width - CheckRect.Width);
    int stopY = Height - halfHeight - (Height - CheckRect.Height) - 1;
    int offset = Width - CheckRect.Width;


    unsigned char* src = Input + CheckRect.y * Width + CheckRect.x;
    unsigned char* srcBelow = src + Width;

    for (int Y = startY; Y < stopY; Y++) {
        for (int X = startX; X < stopX; X++, src++, srcBelow++) {
            if ((*src < 128) && (*srcBelow >= 128)) {
                for (int theta = 0; theta < houghHeight; theta++) {
                    int radius = (int)(cosMap[theta] * X - sinMap[theta] * Y) + halfHoughWidth;

                    if ((radius < 0) || (radius >= houghWidth)) {
                        continue;
                    }

                    houghMap[theta * houghWidth + radius]++;
                }
            }
        }
        src += offset;
        srcBelow += offset;
    }


    // find the maximum value of the hough map
    float maxMapIntensity = 0.0000000001f;
    for (int theta = 0; theta < houghHeight; theta++) {
        unsigned short* houghMapLine = houghMap + theta * houghWidth;
        for (int radius = 0; radius < houghWidth; radius++) {
            maxMapIntensity = max(maxMapIntensity, houghMapLine[radius]);
        }
    }
    int minLineIntensity = Width / 10;

    // collects straight lines greater than or equal to the specified intensity

    int lineIntensity = 0;
    bool foundGreater = false;
    int lineSize = 0;
    for (int theta = 0; theta < houghHeight; theta++) {
        unsigned short* houghMapLine = houghMap + theta * houghWidth;
        for (int radius = 0; radius < houghWidth; radius++) {
            // get current intensity
            lineIntensity = houghMapLine[radius];

            if (lineIntensity < minLineIntensity) {
                continue;
            }

            foundGreater = false;

            // check adjacent edges
            for (int t = theta - localPeakRadius, ttMax = theta + localPeakRadius; t < ttMax; t++) {
                // skip map value
                if (t < 0) {
                    continue;
                }
                if (t >= houghHeight) {
                    break;
                }

                // if it is not a local maximum jump out
                if (foundGreater == true) {
                    break;
                }
                for (int r = radius - localPeakRadius, trMax = radius + localPeakRadius; r < trMax; r++) {
                    // skip map value
                    if (r < 0) {
                        continue;
                    }
                    if (r >= houghWidth) {
                        break;
                    }
                    // compare current value with adjacent edge
                    if (houghMap[t * houghWidth + r] > lineIntensity) {
                        foundGreater = true;
                        break;
                    }
                }
            }
            // it may be a local maximum record it
            if (!foundGreater) {
                HoughLine tempVar;
                tempVar.Theta = 90.0f - maxSkewToDetect + (theta) / stepsPerDegree;
                tempVar.Radius = (radius - halfHoughWidth);
                tempVar.Intensity = lineIntensity;
                tempVar.RelativeIntensity = lineIntensity / maxMapIntensity;
                HoughLines[lineSize] = tempVar;
                lineSize++;
            }
        }
    }

    float skewAngle = 0;
    if (lineSize > 0) {
        // sort from large to small
        HoughLine temp;
        for (int i = 0; i < lineSize; i++) {
            for (int j = 0; j < lineSize - 1; j++) {
                if (HoughLines[j].Intensity < HoughLines[j + 1].Intensity) {
                    temp = HoughLines[j + 1];
                    HoughLines[j + 1] = HoughLines[j];
                    HoughLines[j] = temp;
                }
            }
        }

        int n = min(nLineCount, lineSize);

        float sumIntensity = 0;

        for (int i = 0; i < n; i++) {
            if (HoughLines[i].RelativeIntensity > 0.5f) {
                skewAngle += (HoughLines[i].Theta * HoughLines[i].RelativeIntensity);
                sumIntensity += HoughLines[i].RelativeIntensity;
            }
        }
        skewAngle = skewAngle / sumIntensity;
    }
    if (houghMap) {
        free(houghMap);
        houghMap = NULL;
    }
    if (sinMap) {
        free(sinMap);
        sinMap = NULL;
    }
    if (cosMap) {
        free(cosMap);
        cosMap = NULL;
    }
    if (HoughLines) {
        free(HoughLines);
        HoughLines = NULL;
    }
    if (skewAngle != 0) {
        return skewAngle - 90.0f;
    }
    return skewAngle;
}

// Convert RGB data to single channel
static void SplitRGB(unsigned char* Src, unsigned char* Blue, unsigned char* Green, unsigned char* Red, int Width, int Height, int Stride) {
    for (int Y = 0; Y < Height; Y++) {
        unsigned char* PointerS = Src + Y * Stride;
        unsigned char* PointerB = Blue + Y * Width;
        unsigned char* PointerG = Green + Y * Width;
        unsigned char* PointerR = Red + Y * Width;
        for (int X = 0; X < Width; X++) {
            PointerR[X] = PointerS[0];
            PointerG[X] = PointerS[1];
            PointerB[X] = PointerS[2];
            PointerS += 3;
        }
    }
}

// Convert single-channel data to RGB data
static void CombineRGB(unsigned char* Blue, unsigned char* Green, unsigned char* Red, unsigned char* Dest, int Width, int Height, int Stride) {
    for (int Y = 0; Y < Height; Y++) {
        unsigned char* PointerD = Dest + Y * Stride;
        unsigned char* PointerB = Blue + Y * Width;
        unsigned char* PointerG = Green + Y * Width;
        unsigned char* PointerR = Red + Y * Width;
        for (int X = 0; X < Width; X++) {
            PointerD[0] = PointerR[X];
            PointerD[1] = PointerG[X];
            PointerD[2] = PointerB[X];
            PointerD += 3;
        }
    }
}

static void applyCurve(unsigned char* input, unsigned char* output, int width, int height, int channels, int stride, unsigned char* TableR,
                       unsigned char* TableG, unsigned char* TableB) {

    for (int y = 0; y < height; y++) {
        unsigned char* scanIn = input + y * stride;
        unsigned char* scanOut = output + y * stride;
        for (int x = 0; x < width; x++) {
            scanOut[0] = TableR[scanIn[0]];
            scanOut[1] = TableG[scanIn[1]];
            scanOut[2] = TableB[scanIn[2]];
            scanIn += channels;
            scanOut += channels;
        }
    }
}

static void calculate_local_mean_deviation(const unsigned char* image, unsigned char* mean, int* deviation, int width, int height, int radius) {
    int neighborhood_size = 2 * radius + 1;
    for (int i = radius; i < height - radius; i++) {
        for (int j = radius; j < width - radius; j++) {
            if (i * width + j >= width * height) // Ensure index doesn't exceed array size
                continue;

            int sum = 0;
            for (int k = -radius; k <= radius; k++) {
                for (int l = -radius; l <= radius; l++) {
                    int row = i + k;
                    int col = j + l;
                    if (row >= 0 && row < height && col >= 0 && col < width) {
                        sum += image[row * width + col];
                    }
                }
            }
            mean[i * width + j] = sum / (neighborhood_size * neighborhood_size);

            double variance = 0;
            for (int k = -radius; k <= radius; k++) {
                for (int l = -radius; l <= radius; l++) {
                    int row = i + k;
                    int col = j + l;
                    if (row >= 0 && row < height && col >= 0 && col < width) {
                        variance += (image[row * width + col] - mean[i * width + j]) * (image[row * width + col] - mean[i * width + j]);
                    }
                }
            }
            deviation[i * width + j] = sqrt(variance / (neighborhood_size * neighborhood_size));
        }
    }
}

static void calcMeanVariance(unsigned char* Input, float* mean, float* variance, int Width, int Height, int Stride) {

    int channels = Stride / Width;

    float vsquared = 0.0f;
    *mean = 0.0f;
    for (int y = 0; y < Height; y++) {
        unsigned char* pInput = Input + y * Stride;
        for (int x = 0; x < Width; x++) {
            for (int c = 0; c < channels; c++) {
                *mean += pInput[c];
                vsquared += pInput[c] * pInput[c];
            }
        }
    }

    *mean /= (float)(Height * Stride);
    vsquared /= (float)(Height * Stride);
    *variance = (vsquared - (*mean * *mean));
    *variance = (float)sqrtf(*variance);
}

// find the maximum color value in an RGB image
static int GetMaxValue(unsigned char* Input, int Width, int Height, int Stride) {
    int MaxValue = 0;
    for (int y = 0; y < Height; y++) {
        unsigned char* pInput = Input + y * Stride;
        for (int x = 0; x < Width; x++) {
            if (MaxValue < pInput[0])
                MaxValue = pInput[0];
            if (MaxValue < pInput[1])
                MaxValue = pInput[1];
            if (MaxValue < pInput[2])
                MaxValue = pInput[2];
            pInput += 3;
        }
        if (MaxValue == 255)
            break;
    }
    return MaxValue;
}

// find the minimum and maximum color value in an RGB image
static void GetMinMaxValue(unsigned char* Input, int Width, int Height, int Stride, int* Min, int* Max) {

    int MinValue = 0, MaxValue = 0;

    for (int y = 0; y < Height; y++) {
        unsigned char* pInput = Input + y * Stride;
        for (int x = 0; x < Width; x++) {
            MinValue = pInput[0];
            if (MinValue > pInput[1])
                MinValue = pInput[1];
            if (MinValue > pInput[2])
                MinValue = pInput[2];
            pInput += 3;
        }
        if (MinValue == 0)
            break;
    }
    *Min = MinValue;

    for (int y = 0; y < Height; y++) {
        unsigned char* pInput = Input + y * Stride;
        for (int x = 0; x < Width; x++) {
            if (MaxValue < pInput[0])
                MaxValue = pInput[0];
            if (MaxValue < pInput[1])
                MaxValue = pInput[1];
            if (MaxValue < pInput[2])
                MaxValue = pInput[2];
            pInput += 3;
        }
        if (MaxValue == 255)
            break;
    }
    *Max = MaxValue;
}

static void FillLeftAndRight_Mirror(int* Array, int Length, int Radius) {
    for (int x = 0; x < Radius; x++) {
        Array[x] = Array[Radius + Radius - x];
        Array[Radius + Length + x] = Array[Radius + Length - x - 2];
    }
}

// Add extra rows and columns of zeroes to the edges of an image.
// The zeros are added to the borders of the image so that the size of the image is increased,
// but the original content remains unchanged. This is commonly used in convolution.
static void zeroPadding(const unsigned char* input, unsigned char* output, int row, int col) {
    int inputCols = col - 2; // input number of column

    for (int i = 1; i < row - 1; i++) {
        for (int j = 1; j < col - 1; j++) {
            *(output + i * col + j) = *(input + (i - 1) * inputCols + (j - 1));
        }
    }
}

static int FindArrayMax(const unsigned int* Array, int numElements) {
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

static void calculateNewSize(int Width, int Height, int* newWidth, int* newHeight, bool keepSize, float angle) {
    if (keepSize) {
        *newWidth = Width;
        *newHeight = Height;
        return;
    }

    // angle's sine and cosine
    float angleRad = -angle * M_PI / 180;
    float angleCos = fastCos(angleRad);
    float angleSin = fastSin(angleRad);

    // calculate half size
    float halfWidth = Width / 2;
    float halfHeight = Height / 2;

    // rotate corners
    float cx1 = halfWidth * angleCos;
    float cy1 = halfWidth * angleSin;

    float cx2 = halfWidth * angleCos - halfHeight * angleSin;
    float cy2 = halfWidth * angleSin + halfHeight * angleCos;

    float cx3 = -halfHeight * angleSin;
    float cy3 = halfHeight * angleCos;

    float cx4 = 0;
    float cy4 = 0;

    // recalculate image size
    halfWidth = max(max(cx1, cx2), max(cx3, cx4)) - min(min(cx1, cx2), min(cx3, cx4));
    halfHeight = max(max(cy1, cy2), max(cy3, cy4)) - min(min(cy1, cy2), min(cy3, cy4));

    *newWidth = (int)(halfWidth * 2 + 0.5);
    *newHeight = (int)(halfHeight * 2 + 0.5);
}

static void* AllocMemory(unsigned int Size, bool ZeroMemory) {
    void* ptr = _mm_malloc(Size, 32);
    if (ptr != NULL)
        if (ZeroMemory == true)
            memset(ptr, 0, Size);

    return ptr;
}

static void FreeMemory(void* ptr) {
    if (ptr != NULL)
        _mm_free(ptr);
}

static void HistogramAddShort(unsigned short* X, unsigned short* Y) {

    for (int i = 0; i <= 248; i += 8) {
        *(Y + i) = Y[i] + X[i];
    }
}

static void HistogramSubAddShort(unsigned short* X, unsigned short* Y, unsigned short* Z) {

    for (int i = 0; i <= 248; i += 8) {
        *(Z + i) = (Y[i] + Z[i]) - X[i];
    }
}

static unsigned char HistogramCalc(unsigned short* Hist, unsigned char Value, unsigned short* Intensity) {
    int Weight = 0, Sum = 0, Divisor = 0;
    unsigned short* Offset = Intensity + 255 - Value;
    for (int Y = 0; Y < 256; Y++) {
        Weight = Hist[Y] * Offset[Y];
        Sum += Weight * Y;
        Divisor += Weight;
    }
    if (Divisor > 0)
        return (Sum + (Divisor >> 1)) / Divisor; // rounding
    else
        return Value;
}

//	The speed can be doubled after the cycle unfolds, because the multiplier inside is automatically optimized.
static unsigned char Calc3(unsigned short* Hist, unsigned char Value, unsigned short* Intensity) {
    int Weight = 0, Sum = 0, Divisor = 0;
    unsigned short* Offset = Intensity + 255 - Value;
    Weight = Hist[0] * Offset[0];
    Sum += Weight * 0;
    Divisor += Weight;
    Weight = Hist[1] * Offset[1];
    Sum += Weight * 1;
    Divisor += Weight;
    Weight = Hist[2] * Offset[2];
    Sum += Weight * 2;
    Divisor += Weight;
    Weight = Hist[3] * Offset[3];
    Sum += Weight * 3;
    Divisor += Weight;
    Weight = Hist[4] * Offset[4];
    Sum += Weight * 4;
    Divisor += Weight;
    Weight = Hist[5] * Offset[5];
    Sum += Weight * 5;
    Divisor += Weight;
    Weight = Hist[6] * Offset[6];
    Sum += Weight * 6;
    Divisor += Weight;
    Weight = Hist[7] * Offset[7];
    Sum += Weight * 7;
    Divisor += Weight;
    Weight = Hist[8] * Offset[8];
    Sum += Weight * 8;
    Divisor += Weight;
    Weight = Hist[9] * Offset[9];
    Sum += Weight * 9;
    Divisor += Weight;
    Weight = Hist[10] * Offset[10];
    Sum += Weight * 10;
    Divisor += Weight;
    Weight = Hist[11] * Offset[11];
    Sum += Weight * 11;
    Divisor += Weight;
    Weight = Hist[12] * Offset[12];
    Sum += Weight * 12;
    Divisor += Weight;
    Weight = Hist[13] * Offset[13];
    Sum += Weight * 13;
    Divisor += Weight;
    Weight = Hist[14] * Offset[14];
    Sum += Weight * 14;
    Divisor += Weight;
    Weight = Hist[15] * Offset[15];
    Sum += Weight * 15;
    Divisor += Weight;
    Weight = Hist[16] * Offset[16];
    Sum += Weight * 16;
    Divisor += Weight;
    Weight = Hist[17] * Offset[17];
    Sum += Weight * 17;
    Divisor += Weight;
    Weight = Hist[18] * Offset[18];
    Sum += Weight * 18;
    Divisor += Weight;
    Weight = Hist[19] * Offset[19];
    Sum += Weight * 19;
    Divisor += Weight;
    Weight = Hist[20] * Offset[20];
    Sum += Weight * 20;
    Divisor += Weight;
    Weight = Hist[21] * Offset[21];
    Sum += Weight * 21;
    Divisor += Weight;
    Weight = Hist[22] * Offset[22];
    Sum += Weight * 22;
    Divisor += Weight;
    Weight = Hist[23] * Offset[23];
    Sum += Weight * 23;
    Divisor += Weight;
    Weight = Hist[24] * Offset[24];
    Sum += Weight * 24;
    Divisor += Weight;
    Weight = Hist[25] * Offset[25];
    Sum += Weight * 25;
    Divisor += Weight;
    Weight = Hist[26] * Offset[26];
    Sum += Weight * 26;
    Divisor += Weight;
    Weight = Hist[27] * Offset[27];
    Sum += Weight * 27;
    Divisor += Weight;
    Weight = Hist[28] * Offset[28];
    Sum += Weight * 28;
    Divisor += Weight;
    Weight = Hist[29] * Offset[29];
    Sum += Weight * 29;
    Divisor += Weight;
    Weight = Hist[30] * Offset[30];
    Sum += Weight * 30;
    Divisor += Weight;
    Weight = Hist[31] * Offset[31];
    Sum += Weight * 31;
    Divisor += Weight;
    Weight = Hist[32] * Offset[32];
    Sum += Weight * 32;
    Divisor += Weight;
    Weight = Hist[33] * Offset[33];
    Sum += Weight * 33;
    Divisor += Weight;
    Weight = Hist[34] * Offset[34];
    Sum += Weight * 34;
    Divisor += Weight;
    Weight = Hist[35] * Offset[35];
    Sum += Weight * 35;
    Divisor += Weight;
    Weight = Hist[36] * Offset[36];
    Sum += Weight * 36;
    Divisor += Weight;
    Weight = Hist[37] * Offset[37];
    Sum += Weight * 37;
    Divisor += Weight;
    Weight = Hist[38] * Offset[38];
    Sum += Weight * 38;
    Divisor += Weight;
    Weight = Hist[39] * Offset[39];
    Sum += Weight * 39;
    Divisor += Weight;
    Weight = Hist[40] * Offset[40];
    Sum += Weight * 40;
    Divisor += Weight;
    Weight = Hist[41] * Offset[41];
    Sum += Weight * 41;
    Divisor += Weight;
    Weight = Hist[42] * Offset[42];
    Sum += Weight * 42;
    Divisor += Weight;
    Weight = Hist[43] * Offset[43];
    Sum += Weight * 43;
    Divisor += Weight;
    Weight = Hist[44] * Offset[44];
    Sum += Weight * 44;
    Divisor += Weight;
    Weight = Hist[45] * Offset[45];
    Sum += Weight * 45;
    Divisor += Weight;
    Weight = Hist[46] * Offset[46];
    Sum += Weight * 46;
    Divisor += Weight;
    Weight = Hist[47] * Offset[47];
    Sum += Weight * 47;
    Divisor += Weight;
    Weight = Hist[48] * Offset[48];
    Sum += Weight * 48;
    Divisor += Weight;
    Weight = Hist[49] * Offset[49];
    Sum += Weight * 49;
    Divisor += Weight;
    Weight = Hist[50] * Offset[50];
    Sum += Weight * 50;
    Divisor += Weight;
    Weight = Hist[51] * Offset[51];
    Sum += Weight * 51;
    Divisor += Weight;
    Weight = Hist[52] * Offset[52];
    Sum += Weight * 52;
    Divisor += Weight;
    Weight = Hist[53] * Offset[53];
    Sum += Weight * 53;
    Divisor += Weight;
    Weight = Hist[54] * Offset[54];
    Sum += Weight * 54;
    Divisor += Weight;
    Weight = Hist[55] * Offset[55];
    Sum += Weight * 55;
    Divisor += Weight;
    Weight = Hist[56] * Offset[56];
    Sum += Weight * 56;
    Divisor += Weight;
    Weight = Hist[57] * Offset[57];
    Sum += Weight * 57;
    Divisor += Weight;
    Weight = Hist[58] * Offset[58];
    Sum += Weight * 58;
    Divisor += Weight;
    Weight = Hist[59] * Offset[59];
    Sum += Weight * 59;
    Divisor += Weight;
    Weight = Hist[60] * Offset[60];
    Sum += Weight * 60;
    Divisor += Weight;
    Weight = Hist[61] * Offset[61];
    Sum += Weight * 61;
    Divisor += Weight;
    Weight = Hist[62] * Offset[62];
    Sum += Weight * 62;
    Divisor += Weight;
    Weight = Hist[63] * Offset[63];
    Sum += Weight * 63;
    Divisor += Weight;
    Weight = Hist[64] * Offset[64];
    Sum += Weight * 64;
    Divisor += Weight;
    Weight = Hist[65] * Offset[65];
    Sum += Weight * 65;
    Divisor += Weight;
    Weight = Hist[66] * Offset[66];
    Sum += Weight * 66;
    Divisor += Weight;
    Weight = Hist[67] * Offset[67];
    Sum += Weight * 67;
    Divisor += Weight;
    Weight = Hist[68] * Offset[68];
    Sum += Weight * 68;
    Divisor += Weight;
    Weight = Hist[69] * Offset[69];
    Sum += Weight * 69;
    Divisor += Weight;
    Weight = Hist[70] * Offset[70];
    Sum += Weight * 70;
    Divisor += Weight;
    Weight = Hist[71] * Offset[71];
    Sum += Weight * 71;
    Divisor += Weight;
    Weight = Hist[72] * Offset[72];
    Sum += Weight * 72;
    Divisor += Weight;
    Weight = Hist[73] * Offset[73];
    Sum += Weight * 73;
    Divisor += Weight;
    Weight = Hist[74] * Offset[74];
    Sum += Weight * 74;
    Divisor += Weight;
    Weight = Hist[75] * Offset[75];
    Sum += Weight * 75;
    Divisor += Weight;
    Weight = Hist[76] * Offset[76];
    Sum += Weight * 76;
    Divisor += Weight;
    Weight = Hist[77] * Offset[77];
    Sum += Weight * 77;
    Divisor += Weight;
    Weight = Hist[78] * Offset[78];
    Sum += Weight * 78;
    Divisor += Weight;
    Weight = Hist[79] * Offset[79];
    Sum += Weight * 79;
    Divisor += Weight;
    Weight = Hist[80] * Offset[80];
    Sum += Weight * 80;
    Divisor += Weight;
    Weight = Hist[81] * Offset[81];
    Sum += Weight * 81;
    Divisor += Weight;
    Weight = Hist[82] * Offset[82];
    Sum += Weight * 82;
    Divisor += Weight;
    Weight = Hist[83] * Offset[83];
    Sum += Weight * 83;
    Divisor += Weight;
    Weight = Hist[84] * Offset[84];
    Sum += Weight * 84;
    Divisor += Weight;
    Weight = Hist[85] * Offset[85];
    Sum += Weight * 85;
    Divisor += Weight;
    Weight = Hist[86] * Offset[86];
    Sum += Weight * 86;
    Divisor += Weight;
    Weight = Hist[87] * Offset[87];
    Sum += Weight * 87;
    Divisor += Weight;
    Weight = Hist[88] * Offset[88];
    Sum += Weight * 88;
    Divisor += Weight;
    Weight = Hist[89] * Offset[89];
    Sum += Weight * 89;
    Divisor += Weight;
    Weight = Hist[90] * Offset[90];
    Sum += Weight * 90;
    Divisor += Weight;
    Weight = Hist[91] * Offset[91];
    Sum += Weight * 91;
    Divisor += Weight;
    Weight = Hist[92] * Offset[92];
    Sum += Weight * 92;
    Divisor += Weight;
    Weight = Hist[93] * Offset[93];
    Sum += Weight * 93;
    Divisor += Weight;
    Weight = Hist[94] * Offset[94];
    Sum += Weight * 94;
    Divisor += Weight;
    Weight = Hist[95] * Offset[95];
    Sum += Weight * 95;
    Divisor += Weight;
    Weight = Hist[96] * Offset[96];
    Sum += Weight * 96;
    Divisor += Weight;
    Weight = Hist[97] * Offset[97];
    Sum += Weight * 97;
    Divisor += Weight;
    Weight = Hist[98] * Offset[98];
    Sum += Weight * 98;
    Divisor += Weight;
    Weight = Hist[99] * Offset[99];
    Sum += Weight * 99;
    Divisor += Weight;
    Weight = Hist[100] * Offset[100];
    Sum += Weight * 100;
    Divisor += Weight;
    Weight = Hist[101] * Offset[101];
    Sum += Weight * 101;
    Divisor += Weight;
    Weight = Hist[102] * Offset[102];
    Sum += Weight * 102;
    Divisor += Weight;
    Weight = Hist[103] * Offset[103];
    Sum += Weight * 103;
    Divisor += Weight;
    Weight = Hist[104] * Offset[104];
    Sum += Weight * 104;
    Divisor += Weight;
    Weight = Hist[105] * Offset[105];
    Sum += Weight * 105;
    Divisor += Weight;
    Weight = Hist[106] * Offset[106];
    Sum += Weight * 106;
    Divisor += Weight;
    Weight = Hist[107] * Offset[107];
    Sum += Weight * 107;
    Divisor += Weight;
    Weight = Hist[108] * Offset[108];
    Sum += Weight * 108;
    Divisor += Weight;
    Weight = Hist[109] * Offset[109];
    Sum += Weight * 109;
    Divisor += Weight;
    Weight = Hist[110] * Offset[110];
    Sum += Weight * 110;
    Divisor += Weight;
    Weight = Hist[111] * Offset[111];
    Sum += Weight * 111;
    Divisor += Weight;
    Weight = Hist[112] * Offset[112];
    Sum += Weight * 112;
    Divisor += Weight;
    Weight = Hist[113] * Offset[113];
    Sum += Weight * 113;
    Divisor += Weight;
    Weight = Hist[114] * Offset[114];
    Sum += Weight * 114;
    Divisor += Weight;
    Weight = Hist[115] * Offset[115];
    Sum += Weight * 115;
    Divisor += Weight;
    Weight = Hist[116] * Offset[116];
    Sum += Weight * 116;
    Divisor += Weight;
    Weight = Hist[117] * Offset[117];
    Sum += Weight * 117;
    Divisor += Weight;
    Weight = Hist[118] * Offset[118];
    Sum += Weight * 118;
    Divisor += Weight;
    Weight = Hist[119] * Offset[119];
    Sum += Weight * 119;
    Divisor += Weight;
    Weight = Hist[120] * Offset[120];
    Sum += Weight * 120;
    Divisor += Weight;
    Weight = Hist[121] * Offset[121];
    Sum += Weight * 121;
    Divisor += Weight;
    Weight = Hist[122] * Offset[122];
    Sum += Weight * 122;
    Divisor += Weight;
    Weight = Hist[123] * Offset[123];
    Sum += Weight * 123;
    Divisor += Weight;
    Weight = Hist[124] * Offset[124];
    Sum += Weight * 124;
    Divisor += Weight;
    Weight = Hist[125] * Offset[125];
    Sum += Weight * 125;
    Divisor += Weight;
    Weight = Hist[126] * Offset[126];
    Sum += Weight * 126;
    Divisor += Weight;
    Weight = Hist[127] * Offset[127];
    Sum += Weight * 127;
    Divisor += Weight;
    Weight = Hist[128] * Offset[128];
    Sum += Weight * 128;
    Divisor += Weight;
    Weight = Hist[129] * Offset[129];
    Sum += Weight * 129;
    Divisor += Weight;
    Weight = Hist[130] * Offset[130];
    Sum += Weight * 130;
    Divisor += Weight;
    Weight = Hist[131] * Offset[131];
    Sum += Weight * 131;
    Divisor += Weight;
    Weight = Hist[132] * Offset[132];
    Sum += Weight * 132;
    Divisor += Weight;
    Weight = Hist[133] * Offset[133];
    Sum += Weight * 133;
    Divisor += Weight;
    Weight = Hist[134] * Offset[134];
    Sum += Weight * 134;
    Divisor += Weight;
    Weight = Hist[135] * Offset[135];
    Sum += Weight * 135;
    Divisor += Weight;
    Weight = Hist[136] * Offset[136];
    Sum += Weight * 136;
    Divisor += Weight;
    Weight = Hist[137] * Offset[137];
    Sum += Weight * 137;
    Divisor += Weight;
    Weight = Hist[138] * Offset[138];
    Sum += Weight * 138;
    Divisor += Weight;
    Weight = Hist[139] * Offset[139];
    Sum += Weight * 139;
    Divisor += Weight;
    Weight = Hist[140] * Offset[140];
    Sum += Weight * 140;
    Divisor += Weight;
    Weight = Hist[141] * Offset[141];
    Sum += Weight * 141;
    Divisor += Weight;
    Weight = Hist[142] * Offset[142];
    Sum += Weight * 142;
    Divisor += Weight;
    Weight = Hist[143] * Offset[143];
    Sum += Weight * 143;
    Divisor += Weight;
    Weight = Hist[144] * Offset[144];
    Sum += Weight * 144;
    Divisor += Weight;
    Weight = Hist[145] * Offset[145];
    Sum += Weight * 145;
    Divisor += Weight;
    Weight = Hist[146] * Offset[146];
    Sum += Weight * 146;
    Divisor += Weight;
    Weight = Hist[147] * Offset[147];
    Sum += Weight * 147;
    Divisor += Weight;
    Weight = Hist[148] * Offset[148];
    Sum += Weight * 148;
    Divisor += Weight;
    Weight = Hist[149] * Offset[149];
    Sum += Weight * 149;
    Divisor += Weight;
    Weight = Hist[150] * Offset[150];
    Sum += Weight * 150;
    Divisor += Weight;
    Weight = Hist[151] * Offset[151];
    Sum += Weight * 151;
    Divisor += Weight;
    Weight = Hist[152] * Offset[152];
    Sum += Weight * 152;
    Divisor += Weight;
    Weight = Hist[153] * Offset[153];
    Sum += Weight * 153;
    Divisor += Weight;
    Weight = Hist[154] * Offset[154];
    Sum += Weight * 154;
    Divisor += Weight;
    Weight = Hist[155] * Offset[155];
    Sum += Weight * 155;
    Divisor += Weight;
    Weight = Hist[156] * Offset[156];
    Sum += Weight * 156;
    Divisor += Weight;
    Weight = Hist[157] * Offset[157];
    Sum += Weight * 157;
    Divisor += Weight;
    Weight = Hist[158] * Offset[158];
    Sum += Weight * 158;
    Divisor += Weight;
    Weight = Hist[159] * Offset[159];
    Sum += Weight * 159;
    Divisor += Weight;
    Weight = Hist[160] * Offset[160];
    Sum += Weight * 160;
    Divisor += Weight;
    Weight = Hist[161] * Offset[161];
    Sum += Weight * 161;
    Divisor += Weight;
    Weight = Hist[162] * Offset[162];
    Sum += Weight * 162;
    Divisor += Weight;
    Weight = Hist[163] * Offset[163];
    Sum += Weight * 163;
    Divisor += Weight;
    Weight = Hist[164] * Offset[164];
    Sum += Weight * 164;
    Divisor += Weight;
    Weight = Hist[165] * Offset[165];
    Sum += Weight * 165;
    Divisor += Weight;
    Weight = Hist[166] * Offset[166];
    Sum += Weight * 166;
    Divisor += Weight;
    Weight = Hist[167] * Offset[167];
    Sum += Weight * 167;
    Divisor += Weight;
    Weight = Hist[168] * Offset[168];
    Sum += Weight * 168;
    Divisor += Weight;
    Weight = Hist[169] * Offset[169];
    Sum += Weight * 169;
    Divisor += Weight;
    Weight = Hist[170] * Offset[170];
    Sum += Weight * 170;
    Divisor += Weight;
    Weight = Hist[171] * Offset[171];
    Sum += Weight * 171;
    Divisor += Weight;
    Weight = Hist[172] * Offset[172];
    Sum += Weight * 172;
    Divisor += Weight;
    Weight = Hist[173] * Offset[173];
    Sum += Weight * 173;
    Divisor += Weight;
    Weight = Hist[174] * Offset[174];
    Sum += Weight * 174;
    Divisor += Weight;
    Weight = Hist[175] * Offset[175];
    Sum += Weight * 175;
    Divisor += Weight;
    Weight = Hist[176] * Offset[176];
    Sum += Weight * 176;
    Divisor += Weight;
    Weight = Hist[177] * Offset[177];
    Sum += Weight * 177;
    Divisor += Weight;
    Weight = Hist[178] * Offset[178];
    Sum += Weight * 178;
    Divisor += Weight;
    Weight = Hist[179] * Offset[179];
    Sum += Weight * 179;
    Divisor += Weight;
    Weight = Hist[180] * Offset[180];
    Sum += Weight * 180;
    Divisor += Weight;
    Weight = Hist[181] * Offset[181];
    Sum += Weight * 181;
    Divisor += Weight;
    Weight = Hist[182] * Offset[182];
    Sum += Weight * 182;
    Divisor += Weight;
    Weight = Hist[183] * Offset[183];
    Sum += Weight * 183;
    Divisor += Weight;
    Weight = Hist[184] * Offset[184];
    Sum += Weight * 184;
    Divisor += Weight;
    Weight = Hist[185] * Offset[185];
    Sum += Weight * 185;
    Divisor += Weight;
    Weight = Hist[186] * Offset[186];
    Sum += Weight * 186;
    Divisor += Weight;
    Weight = Hist[187] * Offset[187];
    Sum += Weight * 187;
    Divisor += Weight;
    Weight = Hist[188] * Offset[188];
    Sum += Weight * 188;
    Divisor += Weight;
    Weight = Hist[189] * Offset[189];
    Sum += Weight * 189;
    Divisor += Weight;
    Weight = Hist[190] * Offset[190];
    Sum += Weight * 190;
    Divisor += Weight;
    Weight = Hist[191] * Offset[191];
    Sum += Weight * 191;
    Divisor += Weight;
    Weight = Hist[192] * Offset[192];
    Sum += Weight * 192;
    Divisor += Weight;
    Weight = Hist[193] * Offset[193];
    Sum += Weight * 193;
    Divisor += Weight;
    Weight = Hist[194] * Offset[194];
    Sum += Weight * 194;
    Divisor += Weight;
    Weight = Hist[195] * Offset[195];
    Sum += Weight * 195;
    Divisor += Weight;
    Weight = Hist[196] * Offset[196];
    Sum += Weight * 196;
    Divisor += Weight;
    Weight = Hist[197] * Offset[197];
    Sum += Weight * 197;
    Divisor += Weight;
    Weight = Hist[198] * Offset[198];
    Sum += Weight * 198;
    Divisor += Weight;
    Weight = Hist[199] * Offset[199];
    Sum += Weight * 199;
    Divisor += Weight;
    Weight = Hist[200] * Offset[200];
    Sum += Weight * 200;
    Divisor += Weight;
    Weight = Hist[201] * Offset[201];
    Sum += Weight * 201;
    Divisor += Weight;
    Weight = Hist[202] * Offset[202];
    Sum += Weight * 202;
    Divisor += Weight;
    Weight = Hist[203] * Offset[203];
    Sum += Weight * 203;
    Divisor += Weight;
    Weight = Hist[204] * Offset[204];
    Sum += Weight * 204;
    Divisor += Weight;
    Weight = Hist[205] * Offset[205];
    Sum += Weight * 205;
    Divisor += Weight;
    Weight = Hist[206] * Offset[206];
    Sum += Weight * 206;
    Divisor += Weight;
    Weight = Hist[207] * Offset[207];
    Sum += Weight * 207;
    Divisor += Weight;
    Weight = Hist[208] * Offset[208];
    Sum += Weight * 208;
    Divisor += Weight;
    Weight = Hist[209] * Offset[209];
    Sum += Weight * 209;
    Divisor += Weight;
    Weight = Hist[210] * Offset[210];
    Sum += Weight * 210;
    Divisor += Weight;
    Weight = Hist[211] * Offset[211];
    Sum += Weight * 211;
    Divisor += Weight;
    Weight = Hist[212] * Offset[212];
    Sum += Weight * 212;
    Divisor += Weight;
    Weight = Hist[213] * Offset[213];
    Sum += Weight * 213;
    Divisor += Weight;
    Weight = Hist[214] * Offset[214];
    Sum += Weight * 214;
    Divisor += Weight;
    Weight = Hist[215] * Offset[215];
    Sum += Weight * 215;
    Divisor += Weight;
    Weight = Hist[216] * Offset[216];
    Sum += Weight * 216;
    Divisor += Weight;
    Weight = Hist[217] * Offset[217];
    Sum += Weight * 217;
    Divisor += Weight;
    Weight = Hist[218] * Offset[218];
    Sum += Weight * 218;
    Divisor += Weight;
    Weight = Hist[219] * Offset[219];
    Sum += Weight * 219;
    Divisor += Weight;
    Weight = Hist[220] * Offset[220];
    Sum += Weight * 220;
    Divisor += Weight;
    Weight = Hist[221] * Offset[221];
    Sum += Weight * 221;
    Divisor += Weight;
    Weight = Hist[222] * Offset[222];
    Sum += Weight * 222;
    Divisor += Weight;
    Weight = Hist[223] * Offset[223];
    Sum += Weight * 223;
    Divisor += Weight;
    Weight = Hist[224] * Offset[224];
    Sum += Weight * 224;
    Divisor += Weight;
    Weight = Hist[225] * Offset[225];
    Sum += Weight * 225;
    Divisor += Weight;
    Weight = Hist[226] * Offset[226];
    Sum += Weight * 226;
    Divisor += Weight;
    Weight = Hist[227] * Offset[227];
    Sum += Weight * 227;
    Divisor += Weight;
    Weight = Hist[228] * Offset[228];
    Sum += Weight * 228;
    Divisor += Weight;
    Weight = Hist[229] * Offset[229];
    Sum += Weight * 229;
    Divisor += Weight;
    Weight = Hist[230] * Offset[230];
    Sum += Weight * 230;
    Divisor += Weight;
    Weight = Hist[231] * Offset[231];
    Sum += Weight * 231;
    Divisor += Weight;
    Weight = Hist[232] * Offset[232];
    Sum += Weight * 232;
    Divisor += Weight;
    Weight = Hist[233] * Offset[233];
    Sum += Weight * 233;
    Divisor += Weight;
    Weight = Hist[234] * Offset[234];
    Sum += Weight * 234;
    Divisor += Weight;
    Weight = Hist[235] * Offset[235];
    Sum += Weight * 235;
    Divisor += Weight;
    Weight = Hist[236] * Offset[236];
    Sum += Weight * 236;
    Divisor += Weight;
    Weight = Hist[237] * Offset[237];
    Sum += Weight * 237;
    Divisor += Weight;
    Weight = Hist[238] * Offset[238];
    Sum += Weight * 238;
    Divisor += Weight;
    Weight = Hist[239] * Offset[239];
    Sum += Weight * 239;
    Divisor += Weight;
    Weight = Hist[240] * Offset[240];
    Sum += Weight * 240;
    Divisor += Weight;
    Weight = Hist[241] * Offset[241];
    Sum += Weight * 241;
    Divisor += Weight;
    Weight = Hist[242] * Offset[242];
    Sum += Weight * 242;
    Divisor += Weight;
    Weight = Hist[243] * Offset[243];
    Sum += Weight * 243;
    Divisor += Weight;
    Weight = Hist[244] * Offset[244];
    Sum += Weight * 244;
    Divisor += Weight;
    Weight = Hist[245] * Offset[245];
    Sum += Weight * 245;
    Divisor += Weight;
    Weight = Hist[246] * Offset[246];
    Sum += Weight * 246;
    Divisor += Weight;
    Weight = Hist[247] * Offset[247];
    Sum += Weight * 247;
    Divisor += Weight;
    Weight = Hist[248] * Offset[248];
    Sum += Weight * 248;
    Divisor += Weight;
    Weight = Hist[249] * Offset[249];
    Sum += Weight * 249;
    Divisor += Weight;
    Weight = Hist[250] * Offset[250];
    Sum += Weight * 250;
    Divisor += Weight;
    Weight = Hist[251] * Offset[251];
    Sum += Weight * 251;
    Divisor += Weight;
    Weight = Hist[252] * Offset[252];
    Sum += Weight * 252;
    Divisor += Weight;
    Weight = Hist[253] * Offset[253];
    Sum += Weight * 253;
    Divisor += Weight;
    Weight = Hist[254] * Offset[254];
    Sum += Weight * 254;
    Divisor += Weight;
    Weight = Hist[255] * Offset[255];
    Sum += Weight * 255;
    Divisor += Weight;
    if (Divisor > 0)
        return (Sum + (Divisor >> 1)) / Divisor; //	rounding
    else
        return Value;
}

static int GetMirrorPos(int Length, int Pos) {
    if (Pos < 0) {
        Pos = -Pos;
        while (Pos > Length)
            Pos -= Length;
    } else if (Pos >= Length) {
        Pos = Length - (Pos - Length + 2);
        while (Pos < 0)
            Pos += Length;
    }
    return Pos;
}

static void GetOffsetPos(int* Pos, int Length, int Left, int Right) {
    for (int X = -Left; X < Length + Right; X++) {
        Pos[X + Left] = GetMirrorPos(Length, X);
    }
}

#endif // OCULAR_UTIL_H
