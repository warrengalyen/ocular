/**
 * @file: interpolate.h
 * @author Warren Galyen
 * Created: 5-23-2024
 * Last Updated: 5-23-2024
 * Last update: initial implementation
 *
 * @brief Ocular interpolation/resize method definitions
 */

#ifndef OCULAR_INTERPOLATE_H
#define OCULAR_INTERPOLATE_H

#include "util.h"

// Nearest-neighbor interpolation function
static void nearestNeighborResize(unsigned char* Src, unsigned char* Dest, int srcWidth, int srcHeight, int dstWidth, int dstHeight, int Channels) {
    float xRatio = (float)srcWidth / dstWidth;
    float yRatio = (float)srcHeight / dstHeight;

    for (int dstY = 0; dstY < dstHeight; dstY++) {
        for (int dstX = 0; dstX < dstWidth; dstX++) {
            int srcX = (int)(xRatio * dstX);
            int srcY = (int)(yRatio * dstY);

            for (int channel = 0; channel < Channels; channel++) {
                Dest[(dstY * dstWidth + dstX) * Channels + channel] = Src[(srcY * srcWidth + srcX) * Channels + channel];
            }
        }
    }
}

// Cubic interpolation helper function
static float cubicInterpolate(float p0, float p1, float p2, float p3, float fraction) {
    float a = (-0.5 * p0) + (1.5 * p1) - (1.5 * p2) + (0.5 * p3);
    float b = p0 - (2.5 * p1) + (2 * p2) - (0.5 * p3);
    float c = (-0.5 * p0) + (0.5 * p2);
    float d = p1;
    return a * fraction * fraction * fraction + b * fraction * fraction + c * fraction + d;
}

// Bicubic interpolation for a 4x4 patch
static float bicubicInterpolate(float patch[4][4], float xFraction, float yFraction) {
    float colInterpolates[4];
    for (int i = 0; i < 4; i++) {
        colInterpolates[i] = cubicInterpolate(patch[i][0], patch[i][1], patch[i][2], patch[i][3], yFraction);
    }
    return cubicInterpolate(colInterpolates[0], colInterpolates[1], colInterpolates[2], colInterpolates[3], xFraction);
}

static void bicubicResize(unsigned char* Src, unsigned char* Dest, int srcWidth, int srcHeight, int dstWidth, int dstHeight, int Channels) {
    float xRatio = (float)srcWidth / dstWidth;
    float yRatio = (float)srcHeight / dstHeight;
    for (int dstY = 0; dstY < dstHeight; dstY++) {
        for (int dstX = 0; dstX < dstWidth; dstX++) {
            float srcX = xRatio * dstX;
            float srcY = yRatio * dstY;
            int srcXInt = (int)srcX;
            int srcYInt = (int)srcY;
            float xFraction = srcX - srcXInt;
            float yFraction = srcY - srcYInt;

            for (int channel = 0; channel < Channels; channel++) {
                float patch[4][4] = { 0 };
                for (int m = -1; m < 3; m++) {
                    for (int n = -1; n < 3; n++) {
                        int srcXM = srcXInt + m;
                        int srcYN = srcYInt + n;
                        srcXM = srcXM < 0 ? 0 : (srcXM >= srcWidth ? srcWidth - 1 : srcXM);
                        srcYN = srcYN < 0 ? 0 : (srcYN >= srcHeight ? srcHeight - 1 : srcYN);
                        patch[m + 1][n + 1] = Src[(srcYN * srcWidth + srcXM) * Channels + channel];
                    }
                }

                float interpolatedValue = bicubicInterpolate(patch, xFraction, yFraction);
                Dest[(dstY * dstWidth + dstX) * Channels + channel] = (unsigned char)fmaxf(0.0f, fminf(255.0f, interpolatedValue));
            }
        }
    }
}


static void bilinearResize(unsigned char* Src, unsigned char* Dest, int srcWidth, int srcHeight, int dstWidth, int dstHeight, int Channels) {

    float xRatio = (float)(srcWidth - 1) / dstWidth;
    float yRatio = (float)(srcHeight - 1) / dstHeight;
    for (int dstY = 0; dstY < dstHeight; dstY++) {
        for (int dstX = 0; dstX < dstWidth; dstX++) {
            float srcX = xRatio * dstX;
            float srcY = yRatio * dstY;
            int srcXInt = (int)srcX;
            int srcYInt = (int)srcY;
            float xFraction = srcX - srcXInt;
            float yFraction = srcY - srcYInt;

            for (int channel = 0; channel < Channels; channel++) {
                unsigned char topLeft = Src[(srcYInt * srcWidth + srcXInt) * Channels + channel];
                unsigned char topRight = Src[(srcYInt * srcWidth + srcXInt + 1) * Channels + channel];
                unsigned char bottomLeft = Src[((srcYInt + 1) * srcWidth + srcXInt) * Channels + channel];
                unsigned char bottomRight = Src[((srcYInt + 1) * srcWidth + srcXInt + 1) * Channels + channel];

                float interpolatedValue = topLeft * (1 - xFraction) * (1 - yFraction) + topRight * xFraction * (1 - yFraction) +
                        bottomLeft * yFraction * (1 - xFraction) + bottomRight * xFraction * yFraction;

                Dest[(dstY * dstWidth + dstX) * Channels + channel] = (unsigned char)interpolatedValue;
            }
        }
    }
}

static void lanzcosResize(unsigned char* Input, unsigned int Width, unsigned int Height, unsigned int Stride, unsigned char* Output,
                          int newWidth, int newHeight, int dstStride) {

    int Channels = Stride / Width;
    int dstOffset = dstStride - Channels * newWidth;
    float xFactor = (float)Width / newWidth;
    float yFactor = (float)Height / newHeight;

    int ymax = Height - 1;
    int xmax = Width - 1;

    for (int y = 0; y < newHeight; y++) {
        float oy = (float)y * yFactor;
        int oy1 = (int)oy;
        int oy2 = (oy1 == ymax) ? oy1 : oy1 + 1;
        float dy1 = oy - (float)oy1;
        float dy2 = 1.0f - dy1;

        unsigned char* tp1 = Input + oy1 * Stride;
        unsigned char* tp2 = Input + oy2 * Stride;

        for (int x = 0; x < newWidth; x++) {
            float ox = (float)x * xFactor;
            int ox1 = (int)ox;
            int ox2 = (ox1 == xmax) ? ox1 : ox1 + 1;
            float dx1 = ox - (float)ox1;
            float dx2 = 1.0f - dx1;
            unsigned char* p1 = tp1 + ox1 * Channels;
            unsigned char* p2 = tp1 + ox2 * Channels;
            unsigned char* p3 = tp2 + ox1 * Channels;
            unsigned char* p4 = tp2 + ox2 * Channels;

            for (int i = 0; i < Channels; i++, Output++, p1++, p2++, p3++, p4++) {
                *Output = (unsigned char)(dy2 * (dx2 * (*p1) + dx1 * (*p2)) + dy1 * (dx2 * (*p3) + dx1 * (*p4)));
            }
        }
        Output += dstOffset;
    }
}

#endif // OCULAR_INTERPOLATE_H
