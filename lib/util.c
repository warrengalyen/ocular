#include "util.h"

float vec2_distance(float vecX, float vecY, float otherX, float otherY) {
    float dx = vecX - otherX;
    float dy = vecY - otherY;
    return sqrtf(dx * dx + dy * dy);
}

void normalizeKernel(float* kernel, int kernelWidth) {
    int sum = 0;
    int total = kernelWidth * kernelWidth;
    for (int i = 0; i < total; i++) {
        sum += kernel[i];
    }
    for (int i = 0; i < total && sum != 0; i++) {
        kernel[i] /= sum;
    }
}

void autoLevel(const unsigned int* histogram, unsigned char* remapLut, int numberOfPixels, float cutLimit, float contrast) {
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

bool isColorCast(const unsigned int* histogramCb, const unsigned int* histogramCr, int numberOfPixels, int colorCoeff) {
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

int getDiffFactor(const unsigned char* color1, const unsigned char* color2, const int channels) {
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

void CalGaussianCoeff(float sigma, float* a0, float* a1, float* a2, float* a3, float* b1, float* b2, float* cprev, float* cnext) {
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

bool isTextImage(unsigned char* Input, int Width, int Height) {
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

float calcSkewAngle(unsigned char* Input, int Width, int Height, OcRect* CheckRectPtr, int maxSkewToDetect, int stepsPerDegree,
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

void SplitRGB(unsigned char* Src, unsigned char* Blue, unsigned char* Green, unsigned char* Red, int Width, int Height, int Stride) {
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

void CombineRGB(unsigned char* Blue, unsigned char* Green, unsigned char* Red, unsigned char* Dest, int Width, int Height, int Stride) {
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

void applyCurve(unsigned char* input, unsigned char* output, int width, int height, int channels, int stride, unsigned char* TableR,
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

void calculate_local_mean_deviation(const unsigned char* image, unsigned char* mean, int* deviation, int width, int height, int radius) {
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

void calcMeanVariance(unsigned char* Input, float* mean, float* variance, int Width, int Height, int Stride) {

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

int GetMaxValue(unsigned char* Input, int Width, int Height, int Stride) {
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

void GetMinMaxValue(unsigned char* Input, int Width, int Height, int Stride, int* Min, int* Max) {

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

void FillLeftAndRight_Mirror(int* Array, int Length, int Radius) {
    for (int x = 0; x < Radius; x++) {
        Array[x] = Array[Radius + Radius - x];
        Array[Radius + Length + x] = Array[Radius + Length - x - 2];
    }
}

void zeroPadding(const unsigned char* input, unsigned char* output, int row, int col) {
    int inputCols = col - 2; // input number of column

    for (int i = 1; i < row - 1; i++) {
        for (int j = 1; j < col - 1; j++) {
            *(output + i * col + j) = *(input + (i - 1) * inputCols + (j - 1));
        }
    }
}

int FindArrayMax(const unsigned int* Array, int numElements) {
    int N = 0; // N is used to indicate the order of the element int the array which has the max value: Array[N] is the maximum in Array
    for (int i = 1; i < numElements; i++) {
        if (Array[N] >= Array[i])
            N = N;
        else
            N = i;
    }

    return N;
}

void FindMaxVote(unsigned int** VoteTable, int numAngles, int numDistances, int* M, int* N) {
    unsigned int* maxVoteDiffAngles = (unsigned int*)malloc(numAngles * sizeof(unsigned int));

    for (int i = 0; i < numAngles; i++) {
        int j = FindArrayMax(VoteTable[i], numDistances);
        maxVoteDiffAngles[i] = VoteTable[i][j];
    }

    *M = FindArrayMax(maxVoteDiffAngles, numAngles);
    *N = FindArrayMax(VoteTable[*M], numDistances);

    free(maxVoteDiffAngles);
}

void houghTransformLine(unsigned char* input, float minAngle, float angleInterval, int numAngles, float minDistance, float distanceInterval,
                        int NumDistances, unsigned int** VoteTable, int width, int height) {
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

void calculateNewSize(int Width, int Height, int* newWidth, int* newHeight, bool keepSize, float angle) {
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

void HistogramAddShort(unsigned short* X, unsigned short* Y) {

    for (int i = 0; i <= 248; i += 8) {
        *(Y + i) = Y[i] + X[i];
    }
}

void HistogramSubAddShort(unsigned short* X, unsigned short* Y, unsigned short* Z) {

    for (int i = 0; i <= 248; i += 8) {
        *(Z + i) = (Y[i] + Z[i]) - X[i];
    }
}

unsigned char HistogramCalc(unsigned short* Hist, unsigned char Value, unsigned short* Intensity) {
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

void HistogramCalc2(unsigned short* Hist, int Intensity, unsigned char* Pixel, int Threshold) {
    int Low, High, Sum = 0, Weight = 0;
    Low = Intensity - Threshold;
    High = Intensity + Threshold;
    if (Low < 0)
        Low = 0;
    if (High > 255)
        High = 255;
    for (int i = Low; i <= High; i++) {
        Weight += Hist[i];
        Sum += Hist[i] * i;
    }
    if (Weight != 0)
        *Pixel = Sum / Weight;
}

int GetMirrorPos(int Length, int Pos) {
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

void GetOffsetPos(int* Pos, int Length, int Left, int Right) {
    for (int X = -Left; X < Length + Right; X++) {
        Pos[X + Left] = GetMirrorPos(Length, X);
    }
}