#ifndef OCULAR_UTIL_H
#define OCULAR_UTIL_H

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

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

#endif // OCULAR_UTIL_H
