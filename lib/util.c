#include "util.h"

float vec2_distance(float vecX, float vecY, float otherX, float otherY) {
    float dx = vecX - otherX;
    float dy = vecY - otherY;
    return sqrtf(dx * dx + dy * dy);
}

unsigned char getMirroredPixel(const unsigned char* input, int x, int y, int width, int height) {
    // Mirror coordinates if they're outside image bounds
    if (x < 0) x = -x;
    if (y < 0) y = -y;
    if (x >= width) x = 2 * (width - 1) - x;
    if (y >= height) y = 2 * (height - 1) - y;
    
    return input[y * width + x];
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

void getLuminance(const unsigned char* input, unsigned char* output, int width, int height, int stride) {
    int channels = stride / width;
    for (int y = 0; y < height; y++) {
        const unsigned char* scanline = input + y * stride;
        unsigned char* outLine = output + y * width;

        for (int x = 0; x < width; x++) {
            outLine[x] = (unsigned char)(0.2126f * scanline[0] + 0.7152f * scanline[1] + 0.0722f * scanline[2] + 0.5f);
            scanline += channels;
        }
    }
}

void calculate_local_mean_deviation(const unsigned char* image, unsigned char* mean, int* deviation, int width, int height, int radius) {

    // Process only valid pixels (with complete neighborhood)
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int sum = 0;
            int count = 0;

            // Calculate mean
            for (int k = -radius; k <= radius; k++) {
                for (int l = -radius; l <= radius; l++) {
                    int row = i + k;
                    int col = j + l;

                    if (row >= 0 && row < height && col >= 0 && col < width) {
                        sum += image[row * width + col];
                        count++;
                    }
                }
            }

            int pixel_mean = (count > 0) ? (sum / count) : 0;
            if (mean != NULL) {
                mean[i * width + j] = (unsigned char)pixel_mean;
            }

            // Calculate deviation
            double variance = 0;
            count = 0;
            for (int k = -radius; k <= radius; k++) {
                for (int l = -radius; l <= radius; l++) {
                    int row = i + k;
                    int col = j + l;

                    if (row >= 0 && row < height && col >= 0 && col < width) {
                        int diff = image[row * width + col] - pixel_mean;
                        variance += diff * diff;
                        count++;
                    }
                }
            }

            deviation[i * width + j] = (count > 0) ? (int)sqrt(variance / count) : 0;
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

unsigned int skinDetection(unsigned char* rgb_src, int width, int height, int channels) {
    int stride = width * channels;
    int lastCol = width * channels - channels;
    int lastRow = height * stride - stride;
    unsigned int sum = 0;
    for (int y = 0; y < height; y++) {
        int cur_row = stride * y;
        int next_row = min(cur_row + stride, lastRow);
        unsigned char* next_scanLine = rgb_src + next_row;
        unsigned char* cur_scanLine = rgb_src + cur_row;
        for (int x = 0; x < width; x++) {
            int cur_col = x * channels;
            int next_col = min(cur_col + channels, lastCol);
            unsigned char* c00 = cur_scanLine + cur_col;
            unsigned char* c10 = cur_scanLine + next_col;
            unsigned char* c01 = next_scanLine + cur_col;
            unsigned char* c11 = next_scanLine + next_col;
            int r_avg = ((c00[0] + c10[0] + c01[0] + c11[0])) >> 2;
            int g_avg = ((c00[1] + c10[1] + c01[1] + c11[1])) >> 2;
            int b_avg = ((c00[2] + c10[2] + c01[2] + c11[2])) >> 2;
            if (r_avg >= 60 && g_avg >= 40 && b_avg >= 20 && r_avg >= b_avg && (r_avg - g_avg) >= 10 &&
                max(max(r_avg, g_avg), b_avg) - min(min(r_avg, g_avg), b_avg) >= 10) {
                sum++;
            }
        }
    }
    return sum;
}

void skinFilter(unsigned char* input, unsigned char* output, int width, int height, int channels) {
    int stride = width * channels;
    int lastCol = width * channels - channels;
    int lastRow = height * stride - stride;
    for (int y = 0; y < height; y++) {
        int cur_row = stride * y;
        int next_row = min(cur_row + stride, lastRow);
        unsigned char* next_scanOutLine = output + next_row;
        unsigned char* cur_scanOutLine = output + cur_row;
        unsigned char* scanOutLine = output + y * stride;
        unsigned char* scanInLine = input + y * stride;
        for (int x = 0; x < width; x++) {
            int cur_col = x * channels;
            int next_col = min(cur_col + channels, lastCol);
            unsigned char* c00 = cur_scanOutLine + cur_col;
            unsigned char* c10 = cur_scanOutLine + next_col;
            unsigned char* c01 = next_scanOutLine + cur_col;
            unsigned char* c11 = next_scanOutLine + next_col;
            int r_avg = ((c00[0] + c10[0] + c01[0] + c11[0])) >> 2;
            int g_avg = ((c00[1] + c10[1] + c01[1] + c11[1])) >> 2;
            int b_avg = ((c00[2] + c10[2] + c01[2] + c11[2])) >> 2;
            int is_skin = !(r_avg >= 60 && g_avg >= 40 && b_avg >= 20 && r_avg >= b_avg && (r_avg - g_avg) >= 10 &&
                            max(max(r_avg, g_avg), b_avg) - min(min(r_avg, g_avg), b_avg) >= 10);
            if (is_skin)
                for (int c = 0; c < channels; ++c)
                    scanOutLine[c] = scanInLine[c];
            scanOutLine += channels;
            scanInLine += channels;
        }
    }
}

void getOffsetPos(int* offsetPos, int length, int left, int right, int step) {
    if (offsetPos == NULL)
        return;
    if ((length < 0) || (left < 0) || (right < 0))
        return;
    for (int x = -left; x < length + right; x++) {
        int pos = x;
        int length2 = length + length;
        if (pos < 0) {
            do {
                pos += length2;
            } while (pos < 0);
        } else if (pos >= length2) {
            do {
                pos -= length2;
            } while (pos >= length2);
        }
        if (pos >= length)
            pos = length2 - 1 - pos;
        offsetPos[x + left] = pos * step;
    }
}

void skinDenoise(unsigned char* input, unsigned char* output, int width, int height, int channels, int radius, int smoothingLevel) {
    if ((input == NULL) || (output == NULL))
        return;
    if ((width <= 0) || (height <= 0))
        return;
    if ((radius <= 0) || (smoothingLevel <= 0))
        return;
    if ((channels != 1) && (channels != 3))
        return;
    int windowSize = (2 * radius + 1) * (2 * radius + 1);
    int* colPower = (int*)malloc(width * channels * sizeof(int));
    int* colValue = (int*)malloc(width * channels * sizeof(int));
    int* rowPos = (int*)malloc((width + radius + radius) * channels * sizeof(int));
    int* colPos = (int*)malloc((height + radius + radius) * channels * sizeof(int));
    if ((colPower == NULL) || (colValue == NULL) || (rowPos == NULL) || (colPos == NULL)) {
        if (colPower)
            free(colPower);
        if (colValue)
            free(colValue);
        if (rowPos)
            free(rowPos);
        if (colPos)
            free(colPos);
        return;
    }
    int stride = width * channels;
    int smoothLut[256] = { 0 };
    float ii = 0.f;
    for (int i = 0; i <= 255; i++, ii -= 1.) {
        smoothLut[i] = (int)((expf(ii * (1.0f / (smoothingLevel * 255.0f))) + (smoothingLevel * (i + 1)) + 1) * 0.5f);
        smoothLut[i] = max(smoothLut[i], 1);
    }
    getOffsetPos(rowPos, width, radius, radius, channels);
    getOffsetPos(colPos, height, radius, radius, stride);
    int* rowOffset = rowPos + radius;
    int* colOffSet = colPos + radius;
    for (int y = 0; y < height; y++) {
        unsigned char* scanInLine = input + y * stride;
        unsigned char* scanOutLine = output + y * stride;
        if (y == 0) {
            for (int x = 0; x < stride; x += channels) {
                int colSum[3] = { 0 };
                int colSumPow[3] = { 0 };
                for (int z = -radius; z <= radius; z++) {
                    unsigned char* sample = input + colOffSet[z] + x;
                    for (int c = 0; c < channels; ++c) {
                        colSum[c] += sample[c];
                        colSumPow[c] += sample[c] * sample[c];
                    }
                }
                for (int c = 0; c < channels; ++c) {
                    colValue[x + c] = colSum[c];
                    colPower[x + c] = colSumPow[c];
                }
            }
        } else {
            unsigned char* lastCol = input + colOffSet[y - radius - 1];
            unsigned char* nextCol = input + colOffSet[y + radius];
            for (int x = 0; x < stride; x += channels) {
                for (int c = 0; c < channels; ++c) {
                    colValue[x + c] -= lastCol[x + c] - nextCol[x + c];
                    colPower[x + c] -= lastCol[x + c] * lastCol[x + c] - nextCol[x + c] * nextCol[x + c];
                }
            }
        }
        int prevSum[3] = { 0 };
        int prevPowerSum[3] = { 0 };
        for (int z = -radius; z <= radius; z++) {
            int index = rowOffset[z];
            for (int c = 0; c < channels; ++c) {
                prevSum[c] += colValue[index + c];
                prevPowerSum[c] += colPower[index + c];
            }
        }
        for (int c = 0; c < channels; ++c) {
            const int mean = prevSum[c] / windowSize;
            const int diff = mean - scanInLine[c];
            const int edge = ClampToByte(diff);
            const int masked_edge = (edge * scanInLine[c] + (256 - edge) * mean) >> 8;
            const int var = Abs(prevPowerSum[c] - mean * prevSum[c]) / windowSize;
            const int out = masked_edge - diff * var / (var + smoothLut[scanInLine[c]]);
            scanOutLine[c] = ClampToByte(out);
        }
        scanInLine += channels;
        scanOutLine += channels;
        for (int x = 1; x < width; x++) {
            int lastRow = rowOffset[x - radius - 1];
            int nextRow = rowOffset[x + radius];
            for (int c = 0; c < channels; ++c) {
                prevSum[c] = prevSum[c] - colValue[lastRow + c] + colValue[nextRow + c];
                prevPowerSum[c] = prevPowerSum[c] - colPower[lastRow + c] + colPower[nextRow + c];
                const int mean = prevSum[c] / windowSize;
                const int diff = mean - scanInLine[c];
                const int edge = ClampToByte(diff);
                const int masked_edge = (edge * scanInLine[c] + (256 - edge) * mean) >> 8;
                const int var = Abs(prevPowerSum[c] - mean * prevSum[c]) / windowSize;
                const int out = masked_edge - diff * var / (var + smoothLut[scanInLine[c]]);
                scanOutLine[c] = ClampToByte(out);
            }
            scanInLine += channels;
            scanOutLine += channels;
        }
    }
    if (colPower)
        free(colPower);
    if (colValue)
        free(colValue);
    if (rowPos)
        free(rowPos);
    if (colPos)
        free(colPos);
}

float calcWeight(const float weight, const float spatialContraDecay, const float diff) {
    return spatialContraDecay * expf(weight * (diff * diff)) * diff;
}