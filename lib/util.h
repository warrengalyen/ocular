/**
 * @file: util.h
 * @author Warren Galyen
 * Created: 2-12-2024
 * Last Updated: 10-15-2024
 * Last update: moved deskew utility functions to ocr.h
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
#ifndef ClampToInt
    #define ClampToInt(v) (int)(((int)(v)) < (255) ? (v) : (v < 0) ? (0) : (255))
#endif
#ifndef ClampToFloat
    #define ClampToFloat(v) (float)(((float)(v)) < (1.0f) ? (v) : (v < 0) ? (0) : (1.0f))
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

float vec2_distance(float vecX, float vecY, float otherX, float otherY);

// Normalize a kernel so that it produces correct colors.
void normalizeKernel(float* kernel, int kernelWidth);

void autoLevel(const unsigned int* histogram, unsigned char* remapLut, int numberOfPixels, float cutLimit, float contrast);

bool isColorCast(const unsigned int* histogramCb, const unsigned int* histogramCr, int numberOfPixels, int colorCoeff);

int getDiffFactor(const unsigned char* color1, const unsigned char* color2, const int channels);

void CalGaussianCoeff(float sigma, float* a0, float* a1, float* a2, float* a3, float* b1, float* b2, float* cprev, float* cnext);

// Convert RGB data to single channel
void SplitRGB(unsigned char* Src, unsigned char* Blue, unsigned char* Green, unsigned char* Red, int Width, int Height, int Stride);

// Convert single-channel data to RGB data
void CombineRGB(unsigned char* Blue, unsigned char* Green, unsigned char* Red, unsigned char* Dest, int Width, int Height, int Stride);

void applyCurve(unsigned char* input, unsigned char* output, int width, int height, int channels, int stride, unsigned char* TableR,
                unsigned char* TableG, unsigned char* TableB);

void calculate_local_mean_deviation(const unsigned char* image, unsigned char* mean, int* deviation, int width, int height, int radius);

void calcMeanVariance(unsigned char* Input, float* mean, float* variance, int Width, int Height, int Stride);

// find the maximum color value in an RGB image
int GetMaxValue(unsigned char* Input, int Width, int Height, int Stride);

// find the minimum and maximum color value in an RGB image
void GetMinMaxValue(unsigned char* Input, int Width, int Height, int Stride, int* Min, int* Max);

void FillLeftAndRight_Mirror(int* Array, int Length, int Radius);

// Add extra rows and columns of zeroes to the edges of an image.
// The zeros are added to the borders of the image so that the size of the image is increased,
// but the original content remains unchanged. This is commonly used in convolution.
void zeroPadding(const unsigned char* input, unsigned char* output, int row, int col);

int FindArrayMax(const unsigned int* Array, int numElements);

void FindMaxVote(unsigned int** VoteTable, int numAngles, int numDistances, int* M, int* N);

void houghTransformLine(unsigned char* input, float minAngle, float angleInterval, int numAngles, float minDistance, float distanceInterval,
                        int NumDistances, unsigned int** VoteTable, int width, int height);

// calculates new image dimensions based on rotation angle
void calculateNewSize(int Width, int Height, int* newWidth, int* newHeight, bool keepSize, float angle);

void HistogramAddShort(unsigned short* X, unsigned short* Y);
void HistogramSubAddShort(unsigned short* X, unsigned short* Y, unsigned short* Z);
unsigned char HistogramCalc(unsigned short* Hist, unsigned char Value, unsigned short* Intensity);
void HistogramCalc2(unsigned short* Hist, int Intensity, unsigned char* Pixel, int Threshold);
int GetMirrorPos(int Length, int Pos);
void GetOffsetPos(int* Pos, int Length, int Left, int Right);

// skin smoothing filter
void getOffsetPos(int* offsetPos, int length, int left, int right, int step);
unsigned int skinDetection(unsigned char* rgb_src, int width, int height, int channels);
void skinFilter(unsigned char* input, unsigned char* output, int width, int height, int channels);
void skinDenoise(unsigned char* input, unsigned char* output, int width, int height, int channels, int radius, int smoothingLevel);

// BEEP filter
float calcWeight(const float weight, const float spatialContraDecay, const float diff);

#endif  /* OCULAR_UTIL_H */
