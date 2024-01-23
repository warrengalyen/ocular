#include "ocular.h"
#include <cstring>
#include <math.h>


/*
LevelParams redLevelParams = {
    // color level minimum
    0,
    // color scale median
    127,
    // maximum value of color scale
    255,
    // minimum output value
    0,
    // maximum output value
    255,
    // whether to apply
    true,
};
LevelParams greenLevelParams = {
    // color level minimum
    0,
    // color scale median
    127,
    // maximum value of color scale
    255,
    // minimum output value
    0,
    // maximum output value
    255,
    // whether to apply
    true,
};
LevelParams blueLevelParams = {
    // color level minimum
    0,
    // color scale median
    127,
    // maximum value of color scale
    255,
    // minimum output value
    0,
    // maximum output value
    255,
    // whether to apply
    true,
}; */

inline unsigned char step(unsigned char edge, unsigned char x) { return x < edge ? 0 : 255; }

inline int Abs(int x) { return (x ^ (x >> 31)) - (x >> 31); }

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

float clamp(float value, float min, float max) {
    if (value > max)
        value = max;
    if (value < min)
        value = min;
    return value;
}
unsigned char mix_u8(unsigned char a, unsigned char b, float alpha) {
    return (unsigned char)ClampToByte(a * (1.0f - alpha) + b * alpha);
}

float dot(unsigned char R, unsigned char G, unsigned char B, float fR, float fG, float fB) {
    return (float)(R * fR + G * fG + B * fB);
}

inline float mix(float a, float b, float alpha) { return (a * (1.0f - alpha) + b * alpha); }

unsigned char degree(unsigned char InputColor, unsigned char OutputColor, float intensity) {
    return (unsigned char)ClampToByte(((intensity * OutputColor) + (1.0f - intensity) * InputColor));
}

float smoothstep(float edgeMin, float edgeMax, float x) {
    return clamp((x - edgeMin) / (edgeMax - edgeMin), 0.0f, 1.0f);
}

float vec2_distance(float vecX, float vecY, float otherX, float otherY) {
    float dx = vecX - otherX;
    float dy = vecY - otherY;
    return sqrtf(dx * dx + dy * dy);
}

/// @brief Converts an image to grayscale(a slightly faster implementation of the saturation filter, without
///        the ability to vary the color contribution)
void ocularGrayscaleFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride) {

    int Channels = Stride / Width;
    for (int Y = 0; Y < Height; Y++) {
        unsigned char* pOutput = Output + (Y * Width);
        unsigned char* pInput  = Input + (Y * Stride);
        for (int X = 0; X < Width; X++) {
            pOutput[0] = ((13926 * pInput[0] + 46884 * pInput[1] + 4725 * pInput[2]) >> 16);
            pInput += Channels;
            pOutput++;
        }
    }
}

/// @brief Adjusts the individual RGB channels of an image
/// @param redAdjustment Normalized values by which each color channel is multiplied. Range [0.0-1.0], default 1.0
/// @param greenAdjustment Normalized values by which each color channel is multiplied. Range [0.0-1.0], default 1.0
/// @param blueAdjustment Normalized values by which each color channel is multiplied. Range [0.0-1.0], default 1.0
void ocularRGBFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                     float redAdjustment, float greenAdjustment, float blueAdjustment) {

    int Channels = Stride / Width;
    if (Channels == 1)
        return;
    unsigned char AdjustMapR[256] = { 0 };
    unsigned char AdjustMapG[256] = { 0 };
    unsigned char AdjustMapB[256] = { 0 };
    for (int pixel = 0; pixel < 256; pixel++) {
        AdjustMapR[pixel] = (unsigned char)(pixel * redAdjustment);
        AdjustMapG[pixel] = (unsigned char)(pixel * greenAdjustment);
        AdjustMapB[pixel] = (unsigned char)(pixel * blueAdjustment);
    }
    for (int Y = 0; Y < Height; Y++) {
        unsigned char* pOutput = Output + (Y * Stride);
        unsigned char* pInput  = Input + (Y * Stride);
        for (int X = 0; X < Width; X++) {
            pOutput[0] = AdjustMapR[pInput[0]];
            pOutput[1] = AdjustMapG[pInput[1]];
            pOutput[2] = AdjustMapB[pInput[2]];
            pInput += Channels;
            pOutput += Channels;
        }
    }
}

/// @brief Applies a thresholding operation where the threshold is continually adjusted based on the average luminance
///        of the image.
/// @param thresholdMultiplier Factor that the average luminance will be multiplied by in order to arrive at the final
///                            threshold to use. Default is 1.0.
void ocularAverageLuminanceThresholdFilter(unsigned char* Input, unsigned char* Output, int Width,
                                           int Height, int Stride, float thresholdMultiplier) {

    int Channels            = Stride / Width;
    unsigned char Luminance = 0;
    if (Channels == 1) {
        unsigned int numberOfPixels     = Width * Height;
        unsigned int histogramGray[256] = { 0 };
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                histogramGray[pInput[0]]++;
                pInput += Channels;
            }
        }
        unsigned int SumPix = 0;
        for (unsigned int i = 0; i < 256; i++) {
            SumPix += histogramGray[i] * i;
        }
        Luminance = (unsigned char)((SumPix / numberOfPixels) * thresholdMultiplier);

        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * Stride);
            unsigned char* pInput  = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                pOutput[0] = step(Luminance, pInput[0]);
                pInput++;
                pOutput++;
            }
        }
    } else if (Channels == 3 || Channels == 4) {

        unsigned int numberOfPixels    = Width * Height;
        unsigned int histogramLum[256] = { 0 };
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                const unsigned char R = pInput[0];
                const unsigned char G = pInput[1];
                const unsigned char B = pInput[2];
                histogramLum[((13926 * R + 46884 * G + 4725 * B) >> 16)]++;
                pInput += Channels;
            }
        }
        unsigned int Sum = 0;
        for (unsigned int i = 0; i < 256; i++) {
            Sum += histogramLum[i] * i;
        }
        Luminance = (unsigned char)((Sum / numberOfPixels) * thresholdMultiplier);
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * Stride);
            unsigned char* pInput  = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                unsigned char luminance = ((13926 * pInput[0] + 46884 * pInput[1] + 4725 * pInput[2]) >> 16);
                pOutput[2] = pOutput[1] = pOutput[0] = step(Luminance, luminance);
                pInput += Channels;
                pOutput += Channels;
            }
        }
    }
}

/// @brief Determines the average color, by averaging the RGBA componenets for each each pixel in an image.
void ocularAverageColor(unsigned char* Input, int Width, int Height, int Stride, unsigned char* AverageR,
                        unsigned char* AverageG, unsigned char* AverageB, unsigned char* AverageA) {

    int Channels = Stride / Width;
    if (Channels == 1) {
        unsigned int numberOfPixels     = Width * Height;
        unsigned int histogramGray[256] = { 0 };
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                histogramGray[pInput[0]]++;
                pInput += Channels;
            }
        }
        unsigned int Sum = 0;
        for (unsigned int i = 0; i < 256; i++) {
            Sum += histogramGray[i] * i;
        }
        *AverageR = Sum / numberOfPixels;
        *AverageG = *AverageR;
        *AverageB = *AverageR;
        *AverageA = *AverageR;
    } else if (Channels == 3) {

        unsigned int numberOfPixels    = Width * Height;
        unsigned int histogramRGB[768] = { 0 };
        unsigned int* histogramR       = &histogramRGB[0];
        unsigned int* histogramG       = &histogramRGB[256];
        unsigned int* histogramB       = &histogramRGB[512];
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                const unsigned char R = pInput[0];
                const unsigned char G = pInput[1];
                const unsigned char B = pInput[2];
                histogramR[R]++;
                histogramG[G]++;
                histogramB[B]++;
                pInput += Channels;
            }
        }
        unsigned int SumPixR = 0, SumPixG = 0, SumPixB = 0;
        for (unsigned int i = 0; i < 256; i++) {
            SumPixR += histogramR[i] * i;
            SumPixG += histogramG[i] * i;
            SumPixB += histogramB[i] * i;
        }
        *AverageR = SumPixR / numberOfPixels;
        *AverageG = SumPixG / numberOfPixels;
        *AverageB = SumPixB / numberOfPixels;
        *AverageA = 255;
    } else if (Channels == 4) {

        unsigned int numberOfPixels          = Width * Height;
        unsigned int histogramRGB[768 + 256] = { 0 };
        unsigned int* histogramR             = &histogramRGB[0];
        unsigned int* histogramG             = &histogramRGB[256];
        unsigned int* histogramB             = &histogramRGB[512];
        unsigned int* histogramA             = &histogramRGB[768];
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                const unsigned char R = pInput[0];
                const unsigned char G = pInput[1];
                const unsigned char B = pInput[2];
                const unsigned char A = pInput[3];
                histogramR[R]++;
                histogramG[G]++;
                histogramB[B]++;
                histogramA[A]++;
                pInput += Channels;
            }
        }
        unsigned int SumPixR = 0, SumPixG = 0, SumPixB = 0, SumPixA = 0;
        for (unsigned int i = 0; i < 256; i++) {
            SumPixR += histogramR[i] * i;
            SumPixG += histogramG[i] * i;
            SumPixB += histogramB[i] * i;
            SumPixA += histogramA[i] * i;
        }
        *AverageR = SumPixR / numberOfPixels;
        *AverageG = SumPixG / numberOfPixels;
        *AverageB = SumPixB / numberOfPixels;
        *AverageA = SumPixA / numberOfPixels;
    }
}

/// @brief This reduces an image to its average luminosity.
/// @param Input
/// @param Width
/// @param Height
/// @param Stride
/// @param Luminance
void ocularLuminosity(unsigned char* Input, int Width, int Height, int Stride, unsigned char* Luminance) {

    int Channels = Stride / Width;
    if (Channels == 1) {
        unsigned int numberOfPixels     = Width * Height;
        unsigned int histogramGray[256] = { 0 };
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                histogramGray[pInput[0]]++;
                pInput++;
            }
        }
        unsigned int Sum = 0;
        for (int i = 0; i < 256; i++) {
            Sum += histogramGray[i] * i;
        }
        *Luminance = (unsigned char)(Sum / numberOfPixels);
    } else if (Channels == 3 || Channels == 4) {

        unsigned int numberOfPixels    = Width * Height;
        unsigned int histogramLum[256] = { 0 };
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                const unsigned char R = pInput[0];
                const unsigned char G = pInput[1];
                const unsigned char B = pInput[2];
                histogramLum[((13926 * R + 46884 * G + 4725 * B) >> 16)]++;
                pInput += Channels;
            }
        }
        unsigned int Sum = 0;
        for (unsigned int i = 0; i < 256; i++) {
            Sum += histogramLum[i] * i;
        }
        *Luminance = (unsigned char)(Sum / numberOfPixels);
    }
}

/// @brief Transforms the colors of an image by apply a matrix to them.
/// @param colorMatrix A 4x4 matrix used to transform each color in an image.
/// @param intensity The degree to which the new transformed color replaces the original color for each pixel.
void ocularColorMatrixFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                             int Stride, float* colorMatrix, float intensity) {

    int Channels = Stride / Width;
    if (Channels == 1)
        return;
    unsigned char degreeMap[256 * 256] = { 0 };
    for (int pixel = 0; pixel < 256; pixel++) {
        unsigned char* pDegreeMap = degreeMap + pixel * 256;
        for (int out = 0; out < 256; out++) {
            pDegreeMap[0] = degree(pixel, out, intensity);
            pDegreeMap++;
        }
    }
    unsigned char colorMatrixMap[256 * 16] = { 0 };
    unsigned char* colorMatrix0            = &colorMatrixMap[0];
    unsigned char* colorMatrix1            = &colorMatrixMap[256];
    unsigned char* colorMatrix2            = &colorMatrixMap[256 * 2];
    unsigned char* colorMatrix3            = &colorMatrixMap[256 * 3];
    unsigned char* colorMatrix4            = &colorMatrixMap[256 * 4];
    unsigned char* colorMatrix5            = &colorMatrixMap[256 * 5];
    unsigned char* colorMatrix6            = &colorMatrixMap[256 * 6];
    unsigned char* colorMatrix7            = &colorMatrixMap[256 * 7];
    unsigned char* colorMatrix8            = &colorMatrixMap[256 * 8];
    unsigned char* colorMatrix9            = &colorMatrixMap[256 * 9];
    unsigned char* colorMatrix10           = &colorMatrixMap[256 * 10];
    unsigned char* colorMatrix11           = &colorMatrixMap[256 * 11];
    unsigned char* colorMatrix12           = &colorMatrixMap[256 * 12];
    unsigned char* colorMatrix13           = &colorMatrixMap[256 * 13];
    unsigned char* colorMatrix14           = &colorMatrixMap[256 * 14];
    unsigned char* colorMatrix15           = &colorMatrixMap[256 * 15];
    for (int pixel = 0; pixel < 256; pixel++) {
        colorMatrix0[pixel]  = ClampToByte(pixel * colorMatrix[0]);
        colorMatrix1[pixel]  = ClampToByte(pixel * colorMatrix[1]);
        colorMatrix2[pixel]  = ClampToByte(pixel * colorMatrix[2]);
        colorMatrix3[pixel]  = ClampToByte(pixel * colorMatrix[3]);
        colorMatrix4[pixel]  = ClampToByte(pixel * colorMatrix[4]);
        colorMatrix5[pixel]  = ClampToByte(pixel * colorMatrix[5]);
        colorMatrix6[pixel]  = ClampToByte(pixel * colorMatrix[6]);
        colorMatrix7[pixel]  = ClampToByte(pixel * colorMatrix[7]);
        colorMatrix8[pixel]  = ClampToByte(pixel * colorMatrix[8]);
        colorMatrix9[pixel]  = ClampToByte(pixel * colorMatrix[9]);
        colorMatrix10[pixel] = ClampToByte(pixel * colorMatrix[10]);
        colorMatrix11[pixel] = ClampToByte(pixel * colorMatrix[11]);
        colorMatrix12[pixel] = ClampToByte(pixel * colorMatrix[12]);
        colorMatrix13[pixel] = ClampToByte(pixel * colorMatrix[13]);
        colorMatrix14[pixel] = ClampToByte(pixel * colorMatrix[14]);
        colorMatrix15[pixel] = ClampToByte(pixel * colorMatrix[15]);
    }
    if (Channels == 4) {
        unsigned char outR, outG, outB, outA;
        int WidthStep = Width * Channels;
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * WidthStep);
            unsigned char* pInput  = Input + (Y * WidthStep);
            for (int X = 0; X < Width; X++) {
                const unsigned char r = pInput[0];
                const unsigned char g = pInput[1];
                const unsigned char b = pInput[2];
                const unsigned char a = pInput[3];
                outR = ClampToByte(colorMatrix0[r] + colorMatrix1[g] + colorMatrix2[b] + colorMatrix3[a]);
                unsigned char* pDegreeMapR = degreeMap + (r << 8);
                pOutput[0]                 = pDegreeMapR[outR];
                outG = ClampToByte(colorMatrix4[r] + colorMatrix5[g] + colorMatrix6[b] + colorMatrix7[a]);
                unsigned char* pDegreeMapG = degreeMap + (g << 8);
                pOutput[1]                 = pDegreeMapG[outG];
                outB = ClampToByte(colorMatrix8[r] + colorMatrix9[g] + colorMatrix10[b] + colorMatrix11[a]);
                unsigned char* pDegreeMapB = degreeMap + (b << 8);
                pOutput[2]                 = pDegreeMapB[outB];
                outA =
                ClampToByte(colorMatrix12[r] + colorMatrix13[g] + colorMatrix14[b] + colorMatrix15[a]);
                unsigned char* pDegreeMapA = degreeMap + (a << 8);
                pOutput[3]                 = pDegreeMapA[outA];
                pInput += Channels;
                pOutput += Channels;
            }
        }
    } else if (Channels == 3) {
        // For three channels, a is 255 opaque.
        unsigned char outR, outG, outB;
        int WidthStep = Width * Channels;
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * WidthStep);
            unsigned char* pInput  = Input + (Y * WidthStep);
            for (int X = 0; X < Width; X++) {
                const unsigned char r = pInput[0];
                const unsigned char g = pInput[1];
                const unsigned char b = pInput[2];
                outR = ClampToByte(colorMatrix0[r] + colorMatrix1[g] + colorMatrix2[b]);
                unsigned char* pDegreeMapR = degreeMap + (r << 8);
                pOutput[0]                 = pDegreeMapR[outR];
                outG = ClampToByte(colorMatrix4[r] + colorMatrix5[g] + colorMatrix6[b]);
                unsigned char* pDegreeMapG = degreeMap + (g << 8);
                pOutput[1]                 = pDegreeMapG[outG];
                outB = ClampToByte(colorMatrix8[r] + colorMatrix9[g] + colorMatrix10[b]);
                unsigned char* pDegreeMapB = degreeMap + (b << 8);
                pOutput[2]                 = pDegreeMapB[outB];
                pInput += Channels;
                pOutput += Channels;
            }
        }
    }
}


void ocularSepiaFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                       int Stride, int intensity) {

    int Channels = Stride / Width;
    if (Channels == 1)
        return;
    float fIntensity = intensity / 100.0f;

    float colorMatrix[4 * 4] = { 0.3588f, 0.7044f, 0.1368f, 0.0f, 0.2990f, 0.5870f, 0.1140f, 0.0f,
                                 0.2392f, 0.4696f, 0.0912f, 0.0f, 0.f,     0.f,     0.f,     1.f };

    ocularColorMatrixFilter(Input, Output, Width, Height, Stride, colorMatrix, fIntensity);
}

// For a given color in the image, sets the alpha channel to 0. This is similar to the
// ocularChromaKeyBlendFilter, only instead of blending in a second image for a matching color
// this doesn't take in a second image and just turns a given color transparent.
// thresholdSensitivity : How close a color match needs to exist to the target color to be
// replaced(default of 0.4)
// smoothing : How smoothly to blend for the color match(default of 0.1)
void ocularChromaKeyFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                           int Stride, unsigned char colorToReplaceR, unsigned char colorToReplaceG,
                           unsigned char colorToReplaceB, float thresholdSensitivity, float smoothing) {

    int Channels = Stride / Width;
    if (Channels == 1) {
        return;
    }

    unsigned char maskY = (19589 * colorToReplaceR + 38443 * colorToReplaceG + 7504 * colorToReplaceB) >> 16;

    unsigned char maskCr = (46740 * (colorToReplaceR - maskY) >> 16) + 128;

    unsigned char maskCb      = (37008 * (colorToReplaceB - maskY) >> 16) + 128;
    int iThresholdSensitivity = (int)(thresholdSensitivity * 255.0f);
    int iSmoothing            = (int)(smoothing * 256);
    if (Channels == 3) {
        short blendMap[256 * 256] = { 0 };
        for (int Cr = 0; Cr < 256; Cr++) {
            short* pBlendMap = blendMap + (Cr << 8);
            for (int Cb = 0; Cb < 256; Cb++) {
                pBlendMap[Cb] =
                (short)(255.0f *
                        smoothstep((float)iThresholdSensitivity, (float)iThresholdSensitivity + iSmoothing,
                                   vec2_distance((float)Cr, (float)Cb, (float)maskCr, (float)maskCb)));
            }
        }
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * Stride);
            unsigned char* pInput  = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                const unsigned char R = pInput[0];
                const unsigned char G = pInput[1];
                const unsigned char B = pInput[2];
                unsigned char Y       = (19589 * R + 38443 * G + 7504 * B) >> 16;
                unsigned char Cr      = (46740 * (R - Y) >> 16) + 128;
                unsigned char Cb      = (37008 * (B - Y) >> 16) + 128;
                // Multiply by 255 to obtain the mask. If not multiplied by 255, the mask will be eliminated.
                short* pBlendMap       = blendMap + (Cr << 8);
                const short blendValue = pBlendMap[Cb];
                pOutput[0]             = 255 - (R * blendValue);
                pOutput[1]             = 255 - (G * blendValue);
                pOutput[2]             = 255 - (B * blendValue);
                pInput += Channels;
                pOutput += Channels;
            }
        }
    } else if (Channels == 4) {
        unsigned char blendMap[256 * 256] = { 0 };
        for (int Cr = 0; Cr < 256; Cr++) {
            unsigned char* pBlendMap = blendMap + (Cr << 8);
            for (int Cb = 0; Cb < 256; Cb++) {
                pBlendMap[Cb] =
                (unsigned char)(255.0f *
                                smoothstep((float)iThresholdSensitivity, (float)iThresholdSensitivity + iSmoothing,
                                           vec2_distance((float)Cr, (float)Cb, (float)maskCr, (float)maskCb)));
            }
        }
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * Stride);
            unsigned char* pInput  = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                const unsigned char R = pInput[0];
                const unsigned char G = pInput[1];
                const unsigned char B = pInput[2];
                const unsigned char A = pInput[3];
                unsigned char Y       = (19589 * R + 38443 * G + 7504 * B) >> 16;
                unsigned char Cr      = (46740 * (R - Y) >> 16) + 128;
                unsigned char Cb      = (37008 * (B - Y) >> 16) + 128;
                // Handle transparency channels directly
                unsigned char* pBlendMap       = blendMap + (Cr << 8);
                const unsigned char blendValue = pBlendMap[Cb];
                pOutput[3]                     = ClampToByte(A * blendValue);
                pInput += Channels;
                pOutput += Channels;
            }
        }
    }
}

// Uses an RGB color lookup image to remap the colors in an image.
void ocularLookupFilter(unsigned char* Input, unsigned char* Output, unsigned char* lookupTable,
                        int Width, int Height, int Stride, int intensity) {

    int Channels = Stride / Width;
    if (Channels == 1)
        return;
    float preMap[256 * 5] = { 0 };
    float* pixelColorMap  = &preMap[0];
    float* quad1yMap      = &preMap[256];
    float* quad1xMap      = &preMap[256 + 256];
    float* quad2yMap      = &preMap[256 + 256 + 256];
    float* quad2xMap      = &preMap[256 + 256 + 256 + 256];

    unsigned short fractMap[256] = { 0 };
    unsigned short sIntensity    = max(min(intensity, 100), 0);
    int c1                       = 256 * (100 - sIntensity) / 100;
    int c2                       = 256 * (100 - (100 - sIntensity)) / 100;

    for (int b = 0; b < 256; b++) {
        pixelColorMap[b] = b * (63.0f / 255.0f);
        fractMap[b]      = (unsigned short)(256 * (pixelColorMap[b] - truncf(pixelColorMap[b])));
        quad1yMap[b]     = floorf(floorf(pixelColorMap[b]) * (1.0f / 8.0f));
        quad1xMap[b]     = floorf(pixelColorMap[b]) - (quad1yMap[b] * 8.0f);
        quad2yMap[b]     = floorf(ceilf(pixelColorMap[b]) * (1.0f / 8.0f));
        quad2xMap[b]     = ceilf(pixelColorMap[b]) - (quad2yMap[b] * 8.0f);
        quad1yMap[b]     = quad1yMap[b] * 64.0f + 0.5f;
        quad2yMap[b]     = quad2yMap[b] * 64.0f + 0.5f;
        quad1xMap[b]     = quad1xMap[b] * 64.0f + 0.5f;
        quad2xMap[b]     = quad2xMap[b] * 64.0f + 0.5f;
    }

    int lookupChannels = 3;
    for (int Y = 0; Y < Height; Y++) {
        unsigned char* pOutput = Output + (Y * Stride);
        unsigned char* pInput  = Input + (Y * Stride);
        for (int X = 0; X < Width; X++) {
            unsigned char R = pInput[0];
            unsigned char G = pInput[1];
            unsigned char B = pInput[2];
            float green     = pixelColorMap[G];
            float red       = pixelColorMap[R];
            unsigned char* pLineLookup1 =
            &lookupTable[(((int)(quad1yMap[B] + green) << 9) + (int)(quad1xMap[B] + red)) * lookupChannels];
            unsigned char* pLineLookup2 =
            &lookupTable[(((int)(quad2yMap[B] + green) << 9) + (int)(quad2xMap[B] + red)) * lookupChannels];
            unsigned short fractB = fractMap[B];
            pOutput[0] =
            (int)(R * c1 + ((*pLineLookup1++ * (256 - fractB) + *pLineLookup2++ * fractB) >> 8) * c2) >> 8;
            pOutput[1] =
            (int)(G * c1 + ((*pLineLookup1++ * (256 - fractB) + *pLineLookup2++ * fractB) >> 8) * c2) >> 8;
            pOutput[2] =
            (int)(B * c1 + ((*pLineLookup1++ * (256 - fractB) + *pLineLookup2++ * fractB) >> 8) * c2) >> 8;
            pInput += Channels;
            pOutput += Channels;
        }
    }
}

// Adjusts the saturation of an image
// saturation : The degree of saturation or desaturation to apply to the image(0.0 - 2.0, with 1.0 as the default)
void ocularSaturationFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                            int Stride, float saturation) {

    int Channels = Stride / Width;
    if (Channels == 1)
        return;
    unsigned char SaturationMap[256 * 256] = { 0 };
    for (int grey = 0; grey < 256; grey++) {
        unsigned char* pSaturationMap = SaturationMap + (grey << 8);
        for (int Input = 0; Input < 256; Input++) {
            pSaturationMap[0] = (unsigned char)((mix_u8(grey, Input, saturation) + Input) * 0.5f);
            pSaturationMap++;
        }
    }
    // 0.2125*256.0*256.0=13926.4
    // 0.7154*256.0*256.0=46884.4544
    // 0.0721*256.0*256.0=4725.1456
    for (int Y = 0; Y < Height; Y++) {
        unsigned char* pOutput = Output + (Y * Stride);
        unsigned char* pInput  = Input + (Y * Stride);
        for (int X = 0; X < Width; X++) {
            unsigned char* pSaturationMap =
            SaturationMap + (((13926 * pInput[0] + 46884 * pInput[1] + 4725 * pInput[2]) >> 16) << 8);
            pOutput[0] = pSaturationMap[pInput[0]];
            pOutput[1] = pSaturationMap[pInput[1]];
            pOutput[2] = pSaturationMap[pInput[2]];
            pInput += Channels;
            pOutput += Channels;
        }
    }
}

// Adjusts the gamma of an image
// gamma : The gamma adjustment to apply(0.0 - 3.0, with 1.0 as the default)
void ocularGammaFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                       int Stride, float gamma) {

    int Channels = Stride / Width;
    if (Channels == 1)
        return;
    unsigned char GammasMap[256] = { 0 };
    for (int pixel = 0; pixel < 256; pixel++) {
        GammasMap[pixel] = ClampToByte(pow(pixel, gamma));
    }
    for (int Y = 0; Y < Height; Y++) {
        unsigned char* pOutput = Output + (Y * Stride);
        unsigned char* pInput  = Input + (Y * Stride);
        for (int X = 0; X < Width; X++) {
            pOutput[0] = GammasMap[pInput[0]];
            pOutput[1] = GammasMap[pInput[1]];
            pOutput[2] = GammasMap[pInput[2]];
            pInput += Channels;
            pOutput += Channels;
        }
    }
}

// Adjusts the contrast of the image
// contrast : The adjusted contrast(0.0 - 4.0, with 1.0 as the default)
void ocularConstrastFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                           int Stride, float contrast) {

    int Channels = Stride / Width;
    if (Channels == 1)
        return;
    unsigned char contrastMap[256] = { 0 };
    for (int pixel = 0; pixel < 256; pixel++) {
        contrastMap[pixel] = ClampToByte((pixel - 127) * contrast + 127);
    }
    for (int Y = 0; Y < Height; Y++) {
        unsigned char* pOutput = Output + (Y * Stride);
        unsigned char* pInput  = Input + (Y * Stride);
        for (int X = 0; X < Width; X++) {
            pOutput[0] = contrastMap[pInput[0]];
            pOutput[1] = contrastMap[pInput[1]];
            pOutput[2] = contrastMap[pInput[2]];
            pInput += Channels;
            pOutput += Channels;
        }
    }
}

// Adjusts the exposure of the image
// exposure : The adjusted exposure(-10.0 - 10.0, with 0.0 as the default)
void ocularExposureFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                          int Stride, float exposure) {

    int Channels = Stride / Width;
    if (Channels == 1)
        return;
    unsigned char exposureMap[256] = { 0 };
    for (int pixel = 0; pixel < 256; pixel++) {
        exposureMap[pixel] = ClampToByte(pixel * pow(2.0, exposure));
    }
    for (int Y = 0; Y < Height; Y++) {
        unsigned char* pOutput = Output + (Y * Stride);
        unsigned char* pInput  = Input + (Y * Stride);
        for (int X = 0; X < Width; X++) {
            pOutput[0] = exposureMap[pInput[0]];
            pOutput[1] = exposureMap[pInput[1]];
            pOutput[2] = exposureMap[pInput[2]];
            pInput += Channels;
            pOutput += Channels;
        }
    }
}

// Adjusts the brightness of the image
// brightness : The adjusted brightness(-1.0 - 1.0, with 0.0 as the default)
void ocularBrightnessFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                            int Stride, int brightness) {

    int Channels = Stride / Width;
    if (Channels == 1)
        return;

    unsigned char BrightnessMap[256] = { 0 };
    for (int pixel = 0; pixel < 256; pixel++) {
        BrightnessMap[pixel] = ClampToByte(pixel + brightness);
    }
    for (int Y = 0; Y < Height; Y++) {
        unsigned char* pOutput = Output + (Y * Stride);
        unsigned char* pInput  = Input + (Y * Stride);
        for (int X = 0; X < Width; X++) {
            pOutput[0] = BrightnessMap[pInput[0]];
            pOutput[1] = BrightnessMap[pInput[1]];
            pOutput[2] = BrightnessMap[pInput[2]];
            pInput += Channels;
            pOutput += Channels;
        }
    }
}

// Uses the luminance of the image to mix between two user - specified colors
// firstColor : The first and second colors specify what colors replace the dark and light areas of
//              the image, respectively. The defaults are(0.0, 0.0, 0.5) amd(1.0, 0.0, 0.0).
// secondColor :
void ocularFalseColorFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                            int Stride, unsigned char firstColorR, unsigned char firstColorG,
                            unsigned char firstColorB, unsigned char secondColorR,
                            unsigned char secondColorG, unsigned char secondColorB, int intensity) {

    int Channels = Stride / Width;
    if (Channels == 1)
        return;
    unsigned short sIntensity = max(min(intensity, 100), 0);
    int c1                    = 256 * (100 - sIntensity) / 100;
    int c2                    = 256 * (100 - (100 - sIntensity)) / 100;

    unsigned char ColorMapR[256] = { 0 };
    unsigned char ColorMapG[256] = { 0 };
    unsigned char ColorMapB[256] = { 0 };
    for (int pixel = 0; pixel < 256; pixel++) {
        float fPixel     = pixel * (1.0f / 255.0f);
        ColorMapR[pixel] = mix_u8(firstColorR, secondColorR, fPixel);
        ColorMapG[pixel] = mix_u8(firstColorG, secondColorG, fPixel);
        ColorMapB[pixel] = mix_u8(firstColorB, secondColorB, fPixel);
    }
    for (int Y = 0; Y < Height; Y++) {
        unsigned char* pOutput = Output + (Y * Stride);
        unsigned char* pInput  = Input + (Y * Stride);
        for (int X = 0; X < Width; X++) {
            unsigned char luminanceWeighting =
            ((13926 * pInput[0] + 46884 * pInput[1] + 4725 * pInput[2]) >> 16);

            pOutput[0] = (pInput[0] * c1 + ColorMapR[luminanceWeighting] * c2) >> 8;
            pOutput[1] = (pInput[1] * c1 + ColorMapG[luminanceWeighting] * c2) >> 8;
            pOutput[2] = (pInput[2] * c1 + ColorMapB[luminanceWeighting] * c2) >> 8;
            pInput += Channels;
            pOutput += Channels;
        }
    }
}

// Used to add or remove haze (similar to a UV filter)
// distance : Strength of the color applied. Default 0. Values between -0.3 and 0.3 are best.
// slope : Amount of color change. Default 0. Values between -0.3 and 0.3 are best.
void ocularHazeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                      int Stride, float distance, float slope, int intensity) {

    int Channels = Stride / Width;
    if (Channels == 1)
        return;
    unsigned short sIntensity = max(min(intensity, 100), 0);
    int c1                    = 256 * (100 - sIntensity) / 100;
    int c2                    = 256 * (100 - (100 - sIntensity)) / 100;
    short* distanceColorMap   = (short*)malloc(Height * sizeof(short));
    short* patchDistanceMap   = (short*)malloc(Height * sizeof(short));
    if (distanceColorMap == NULL || patchDistanceMap == NULL) {
        if (distanceColorMap) {
            free(distanceColorMap);
        }
        if (patchDistanceMap) {
            free(patchDistanceMap);
        }
        return;
    }
    float color = 1.0f;
    for (int i = 0; i < Height; i++) {
        float d             = i * (1.0f / Height) * slope + distance;
        distanceColorMap[i] = ClampToByte(255.0 * d * color);
        patchDistanceMap[i] = (short)(256 * clamp(1.0f / (1.0f - d), 0.0f, 1.0f));
    }
    for (int Y = 0; Y < Height; Y++) {
        unsigned char* pOutput = Output + (Y * Stride);
        unsigned char* pInput  = Input + (Y * Stride);
        for (int X = 0; X < Width; X++) {
            pOutput[0] =
            (int)(pInput[0] * c1 +
                  ((ClampToByte(pInput[0] - distanceColorMap[Y]) * patchDistanceMap[Y]) >> 8) * c2) >>
            8;
            pOutput[1] =
            (int)(pInput[1] * c1 +
                  ((ClampToByte(pInput[1] - distanceColorMap[Y]) * patchDistanceMap[Y]) >> 8) * c2) >>
            8;
            pOutput[2] =
            (int)(pInput[2] * c1 +
                  ((ClampToByte(pInput[2] - distanceColorMap[Y]) * patchDistanceMap[Y]) >> 8) * c2) >>
            8;
            pInput += Channels;
            pOutput += Channels;
        }
    }
    free(distanceColorMap);
    free(patchDistanceMap);
}

// Adjusts the alpha channel of the incoming image
// opacity : The value to multiply the incoming alpha channel for each pixel by (0.0 - 1.0, with 1.0 as the default)
void ocularOpacityFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                         int Stride, float opacity) {

    int Channels = Stride / Width;
    if (Channels == 1)
        return;
    if (Channels == 4) {
        unsigned char opacityMap[256] = { 0 };
        for (unsigned int pixel = 0; pixel < 256; pixel++) {
            opacityMap[pixel] = (unsigned char)(pixel * opacity);
        }
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * Stride);
            unsigned char* pInput  = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                pOutput[3] = opacityMap[pInput[3]];
                pInput += Channels;
                pOutput += Channels;
            }
        }
    }
}

// Photoshop-like levels adjustment. The min, max, minOut and maxOut parameters are floats in the
// range[0, 1] .If you have parameters from Photoshop in the range[0, 255] you must first convert
// them to be[0, 1]. The gamma / mid parameter is a float >= 0. This matches the value
// from Photoshop. If you want to apply levels to RGB as well as individual channels you need to use
// this filter twice - first for the individual channels and then for all channels.
void ocularLevelsFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                        int Stride, ocularLevelParams* redLevelParams,
                        ocularLevelParams* greenLevelParams, ocularLevelParams* blueLevelParams) {

    int Channels                 = Stride / Width;
    unsigned char LevelMapR[256] = { 0 };
    unsigned char LevelMapG[256] = { 0 };
    unsigned char LevelMapB[256] = { 0 };
    for (int pixel = 0; pixel < 256; pixel++) {
        if (redLevelParams->Enable) {
            LevelMapR[pixel] =
            (mix_u8(redLevelParams->minOutput, redLevelParams->maxOutput,
                    (powf(min(max(pixel - redLevelParams->levelMinimum, (0.0f)) /
                              (redLevelParams->levelMaximum - redLevelParams->levelMinimum),
                              (255)),
                          1.0f / (redLevelParams->levelMiddle * (1.0f / 255.0f))))));
        } else {
            LevelMapR[pixel] = pixel;
        }
        if (greenLevelParams->Enable) {
            LevelMapG[pixel] =
            (mix_u8(greenLevelParams->minOutput, greenLevelParams->maxOutput,
                    (powf(min(max(pixel - greenLevelParams->levelMinimum, (0.0f)) /
                              (greenLevelParams->levelMaximum - greenLevelParams->levelMinimum),
                              (255)),
                          1.0f / (greenLevelParams->levelMiddle * (1.0f / 255.0f))))));
        } else {
            LevelMapG[pixel] = pixel;
        }
        if (blueLevelParams->Enable) {
            LevelMapB[pixel] =
            (mix_u8(blueLevelParams->minOutput, blueLevelParams->maxOutput,
                    (powf(min(max(pixel - blueLevelParams->levelMinimum, (0.0f)) /
                              (blueLevelParams->levelMaximum - blueLevelParams->levelMinimum),
                              (255)),
                          1.0f / (blueLevelParams->levelMiddle * (1.0f / 255.0f))))));
        } else {
            LevelMapB[pixel] = pixel;
        }
    }

    for (int Y = 0; Y < Height; Y++) {
        unsigned char* pOutput = Output + (Y * Stride);
        unsigned char* pInput  = Input + (Y * Stride);
        for (int X = 0; X < Width; X++) {
            pOutput[0] = LevelMapR[pInput[0]];
            pOutput[1] = LevelMapG[pInput[1]];
            pOutput[2] = LevelMapB[pInput[2]];
            pInput += Channels;
            pOutput += Channels;
        }
    }
}

/* RGB to YIQ */
void rgb2yiq(unsigned char* R, unsigned char* G, unsigned char* B, short* Y, short* I, short* Q) {
    *Y = ((int)(0.299f * 65536) * *R + (int)(0.587f * 65536) * *G + (int)(0.114f * 65536) * *B) >> 16;
    *I = ((int)(0.595f * 65536) * *R - (int)(0.274453f * 65536) * *G - (int)(0.321263f * 65536) * *B) >> 16;
    *Q = ((int)(0.211456f * 65536) * *R - (int)(0.522591f * 65536) * *G + (int)(0.311135f * 65536) * *B) >> 16;
}


/* YIQ to RGB */
void yiq2rgb(short* Y, short* I, short* Q, unsigned char* R, unsigned char* G, unsigned char* B) {
    *R = ClampToByte((int)(Y + ((((int)(0.9563 * 65536)) * (*I)) + ((int)(0.6210 * 65536)) * (*Q))) >> 16);
    *G = ClampToByte((int)(Y - ((((int)(0.2721 * 65536)) * (*I)) + ((int)(0.6474 * 65536)) * (*Q))) >> 16);
    *B = ClampToByte((int)(Y + ((((int)(1.7046 * 65536)) * (*Q)) - ((int)(1.1070 * 65536)) * (*I))) >> 16);
}


// Adjusts the hue of an image
// hue : The hue angle, in degrees. 90 degrees by default
void ocularHueFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                     float hueAdjust) {
    int Channels               = Stride / Width;
    hueAdjust                  = fmodf(hueAdjust, 360.0f) * 3.14159265358979323846f / 180.0f;
    float hueMap[256 * 256]    = { 0 };
    float ChromaMap[256 * 256] = { 0 };

    for (int Q = 0; Q < 256; Q++) {
        float* pChromaMap = ChromaMap + Q * 256;
        for (int I = 0; I < 256; I++) {
            pChromaMap[0] = sqrtf((float)((I - 128) * (I - 128) + (Q - 128) * (Q - 128)));
            pChromaMap++;
        }
    }
    for (int Q = 0; Q < 256; Q++) {
        float* pHueMap = hueMap + Q * 256;
        for (int I = 0; I < 256; I++) {
            pHueMap[0] = atan2f((float)Q - 128, (float)I - 128);
            pHueMap++;
        }
    }
    float hue    = 0;
    short YPrime = 0;
    short I      = 0;
    short Q      = 0;
    for (int Y = 0; Y < Height; Y++) {
        unsigned char* pOutput = Output + (Y * Stride);
        unsigned char* pInput  = Input + (Y * Stride);
        for (int X = 0; X < Width; X++) {
            // Convert to YIQ
            rgb2yiq(&pInput[0], &pInput[1], &pInput[2], &YPrime, &I, &Q);
            // Calculate the hue and chroma
            float* pHueMap    = hueMap + ((Q + 128) << 8);
            hue               = pHueMap[I + 128];
            float* pChromaMap = ChromaMap + ((Q + 128) << 8);
            float chroma      = pChromaMap[I + 128];
            // Make the user's adjustments
            hue += (-hueAdjust); // why negative rotation?
            // Convert back to YIQ
            Q = (short)(chroma * sinf(hue));
            I = (short)(chroma * cosf(hue));
            yiq2rgb(&YPrime, &I, &Q, &pOutput[0], &pOutput[1], &pOutput[2]);
            pInput += Channels;
            pOutput += Channels;
        }
    }
}

// Allows you to tint the shadows and highlights of an image independently using a color and
// intensity.
// shadowTintColor : Shadow tint RGB color(GPUVector4).Default : {1.0f, 0.0f, 0.0f, 1.0f}
// (red).
// highlightTintColor : Highlight tint RGB color(GPUVector4).Default : {0.0f, 0.0f, 1.0f, 1.0f} (blue).
// shadowTintIntensity : Shadow tint intensity, from 0.0 to 1.0.Default :
// 0.0 highlightTintIntensity : Highlight tint intensity, from 0.0 to 1.0, with 0.0 as the default.
void ocularHighlightShadowTintFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                                     int Stride, float shadowTintR, float shadowTintG, float shadowTintB,
                                     float highlightTintR, float highlightTintG, float highlightTintB,
                                     float shadowTintIntensity, float highlightTintIntensity) {

    int Channels = Stride / Width;
    if (Channels == 1) {
        return;
    }

    unsigned char HighlightShadowMapR[256 * 256] = { 0 };
    unsigned char HighlightShadowMapG[256 * 256] = { 0 };
    unsigned char HighlightShadowMapB[256 * 256] = { 0 };
    for (int lum = 0; lum < 256; lum++) {
        float luminance                     = (1.0f / 255.0f) * lum;
        unsigned char* pHighlightShadowMapR = HighlightShadowMapR + (lum << 8);
        unsigned char* pHighlightShadowMapG = HighlightShadowMapG + (lum << 8);
        unsigned char* pHighlightShadowMapB = HighlightShadowMapB + (lum << 8);
        for (int pixel = 0; pixel < 256; pixel++) {
            float fpixel = (1.0f / 255.0f) * pixel;
            float shadowResultR =
            mix(fpixel, max(fpixel, mix(shadowTintR, fpixel, luminance)), shadowTintIntensity);
            float shadowResultG =
            mix(fpixel, max(fpixel, mix(shadowTintG, fpixel, luminance)), shadowTintIntensity);
            float shadowResultB =
            mix(fpixel, max(fpixel, mix(shadowTintB, fpixel, luminance)), shadowTintIntensity);
            float highlightResultR =
            mix(fpixel, min(shadowResultR, mix(shadowResultR, highlightTintR, luminance)), highlightTintIntensity);
            float highlightResultG =
            mix(fpixel, min(shadowResultG, mix(shadowResultG, highlightTintG, luminance)), highlightTintIntensity);
            float highlightResultB =
            mix(fpixel, min(shadowResultB, mix(shadowResultB, highlightTintB, luminance)), highlightTintIntensity);
            pHighlightShadowMapR[pixel] =
            ClampToByte(255.0f * mix(shadowResultR, highlightResultR, luminance));
            pHighlightShadowMapG[pixel] =
            ClampToByte(255.0f * mix(shadowResultG, highlightResultG, luminance));
            pHighlightShadowMapB[pixel] =
            ClampToByte(255.0f * mix(shadowResultB, highlightResultB, luminance));
        }
    }
    for (int Y = 0; Y < Height; Y++) {
        unsigned char* pOutput = Output + (Y * Stride);
        unsigned char* pInput  = Input + (Y * Stride);
        for (int X = 0; X < Width; X++) {
            const unsigned char R               = pInput[0];
            const unsigned char G               = pInput[1];
            const unsigned char B               = pInput[2];
            unsigned short lum                  = ((13926 * R + 46884 * G + 4725 * B) >> 16) << 8;
            unsigned char* pHighlightShadowMapR = HighlightShadowMapR + (lum);
            unsigned char* pHighlightShadowMapG = HighlightShadowMapG + (lum);
            unsigned char* pHighlightShadowMapB = HighlightShadowMapB + (lum);
            pOutput[0]                          = pHighlightShadowMapR[R];
            pOutput[1]                          = pHighlightShadowMapG[G];
            pOutput[2]                          = pHighlightShadowMapB[B];
            pInput += Channels;
            pOutput += Channels;
        }
    }
}

// Adjusts the shadows and highlights of an image
// shadows : Increase to lighten shadows, from 0.0 to 1.0, with 0.0 as the default.
// highlights : Decrease to darken highlights, from 0.0 to 1.0, with 1.0 as the default.
void ocularHighlightShadowFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                                 int Stride, float shadows, float highlights) {

    int Channels = Stride / Width;
    if (Channels == 1) {
        return;
    }
    short luminanceWeightingMap[256] = { 0 };
    short shadowMap[256]             = { 0 };
    short highlightMap[256]          = { 0 };

    int divLuminance[256 * 256] = { 0 };
    for (int pixel = 0; pixel < 256; pixel++) {
        luminanceWeightingMap[pixel] = (short)(pixel * 0.3f);
        float luminance              = (1.0f / 255.0f) * pixel;
        shadowMap[pixel]             = (short)(255.0f *
                                   clamp((powf(luminance, 1.0f / (shadows + 1.0f)) +
                                          (-0.76f) * powf(luminance, 2.0f / (shadows + 1.0f))) -
                                         luminance,
                                         0.0f, 1.0f));
        highlightMap[pixel] =
        (short)(255.0f *
                clamp((1.0f -
                       (powf(1.0f - luminance, 1.0f / (2.0f - highlights)) +
                        (-0.8f) * powf(1.0f - luminance, 2.0f / (2.0f - highlights)))) -
                      luminance,
                      -1.0f, 0.0f));
    }
    for (int luminance = 0; luminance < 256; luminance++) {
        int* pDivLuminance = divLuminance + luminance * 256;
        for (int pixel = 0; pixel < 256; pixel++) {
            pDivLuminance[0] = (int)(255.0f * pixel * (1.0f / luminance));
            pDivLuminance++;
        }
    }
    for (int Y = 0; Y < Height; Y++) {
        unsigned char* pOutput = Output + (Y * Stride);
        unsigned char* pInput  = Input + (Y * Stride);
        for (int X = 0; X < Width; X++) {
            const short luminance = luminanceWeightingMap[pInput[0]] +
            luminanceWeightingMap[pInput[1]] + luminanceWeightingMap[pInput[2]];
            const short shadow    = shadowMap[luminance];
            const short highlight = highlightMap[luminance];
            short lshpixel        = (luminance + shadow + highlight);
            int* pDivLuminance    = divLuminance + (luminance << 8);
            pOutput[0]            = (lshpixel * pDivLuminance[pInput[0]]) >> 8;
            pOutput[1]            = (lshpixel * pDivLuminance[pInput[1]]) >> 8;
            pOutput[2]            = (lshpixel * pDivLuminance[pInput[2]]) >> 8;
            pInput += Channels;
            pOutput += Channels;
        }
    }
}

// Converts the image to a single - color version, based on the luminance of each pixel
// intensity : The degree to which the specific color replaces the normal image color(0.0 - 1.0, with 1.0 as the default)
// color : The color to use as the basis for the effect, with(0.6, 0.45, 0.3, 1.0) as the default.
void ocularMonochromeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                            int Stride, unsigned char filterColorR, unsigned char filterColorG,
                            unsigned char filterColorB, int intensity) {

    int Channels = Stride / Width;
    if (Channels == 1) {
        return;
    }
    unsigned short sIntensity          = max(min(intensity, 100), 0);
    int c1                             = 256 * (100 - sIntensity) / 100;
    int c2                             = 256 * (100 - (100 - sIntensity)) / 100;
    float fColorR                      = (float)filterColorR * (1.0f / 255.0f);
    float fColorG                      = (float)filterColorG * (1.0f / 255.0f);
    float fColorB                      = (float)filterColorB * (1.0f / 255.0f);
    unsigned char filterColorRMap[256] = { 0 };
    unsigned char filterColorGMap[256] = { 0 };
    unsigned char filterColorBMap[256] = { 0 };
    for (int luminance = 0; luminance < 256; luminance++) {
        float lum = (1.0f / 255.0f) * luminance;
        filterColorRMap[luminance] =
        (unsigned char)(255.0f *
                        (lum < 0.5f ? (2.0f * lum * fColorR) : (1.0f - 2.0f * (1.0f - lum) * (1.0f - fColorR))));
        filterColorGMap[luminance] =
        (unsigned char)(255.0f *
                        (lum < 0.5f ? (2.0f * lum * fColorG) : (1.0f - 2.0f * (1.0f - lum) * (1.0f - fColorG))));
        filterColorBMap[luminance] =
        (unsigned char)(255.0f *
                        (lum < 0.5f ? (2.0f * lum * fColorB) : (1.0f - 2.0f * (1.0f - lum) * (1.0f - fColorB))));
    }
    for (int Y = 0; Y < Height; Y++) {
        unsigned char* pOutput = Output + (Y * Stride);
        unsigned char* pInput  = Input + (Y * Stride);
        for (int X = 0; X < Width; X++) {
            unsigned char lum = ((13926 * pInput[0] + 46884 * pInput[1] + 4725 * pInput[2]) >> 16);
            pOutput[0]        = (int)(pInput[0] * c1 + filterColorRMap[lum] * c2) >> 8;
            pOutput[1]        = (int)(pInput[1] * c1 + filterColorGMap[lum] * c2) >> 8;
            pOutput[2]        = (int)(pInput[2] * c1 + filterColorBMap[lum] * c2) >> 8;
            pInput += Channels;
            pOutput += Channels;
        }
    }
}

// Inverts the colors of an image
void ocularColorInvertFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride) {

    int Channels = Stride / Width;
    if (Channels == 1) {
        return;
    }
    unsigned char invertMap[256] = { 0 };
    for (int pixel = 0; pixel < 256; pixel++) {
        invertMap[pixel] = (255 - pixel);
    }
    for (int Y = 0; Y < Height; Y++) {
        unsigned char* pOutput = Output + (Y * Stride);
        unsigned char* pInput  = Input + (Y * Stride);
        for (int X = 0; X < Width; X++) {
            pOutput[0] = invertMap[pInput[0]];
            pOutput[1] = invertMap[pInput[1]];
            pOutput[2] = invertMap[pInput[2]];
            pInput += Channels;
            pOutput += Channels;
        }
    }
}

// Outputs a generated image with a solid color.
// color : The color, in a four component format, that is used to fill the image.
void ocularSolidColorGenerator(unsigned char* Output, int Width, int Height, int Stride, unsigned char colorR,
                               unsigned char colorG, unsigned char colorB, unsigned char colorAlpha) {

    int Channels = Stride / Width;
    if (Channels == 4) {
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                pOutput[0] = colorR;
                pOutput[1] = colorG;
                pOutput[2] = colorB;
                pOutput[3] = colorAlpha;
                pOutput += Channels;
            }
        }
    } else if (Channels == 3) {
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                pOutput[0] = colorR;
                pOutput[1] = colorG;
                pOutput[2] = colorB;
                pOutput += Channels;
            }
        }
    }
}

// Pixels with a luminance above the threshold will appear white, and those below will be black
// threshold : The luminance threshold, from 0.0 to 1.0, with a default of 0.5
void ocularLuminanceThresholdFilter(unsigned char* Input, unsigned char* Output, int Width,
                                    int Height, int Stride, unsigned char threshold) {

    int Channels = Stride / Width;
    if (Channels == 1) {

        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * Stride);
            unsigned char* pInput  = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                pOutput[0] = step(threshold, pInput[0]);
                pInput++;
                pOutput++;
            }
        }
        return;
    }

    for (int Y = 0; Y < Height; Y++) {
        unsigned char* pOutput = Output + (Y * Stride);
        unsigned char* pInput  = Input + (Y * Stride);
        for (int X = 0; X < Width; X++) {
            unsigned char luminance = ((13926 * pInput[0] + 46884 * pInput[1] + 4725 * pInput[2]) >> 16);
            pOutput[2] = pOutput[1] = pOutput[0] = step(threshold, luminance);
            pInput += Channels;
            pOutput += Channels;
        }
    }
}

// Adjusts the white balance of an image.
// temperature : The temperature to adjust the image by, in ºK.A value of 4000 is very cool and 7000
// very warm. The default value is 5000. Note that the scale between 4000 and 5000 is nearly as
// visually significant as that between 5000 and 7000.
// tint : The tint to adjust the image by. A value of - 200 is very green and 200 is very pink.
// The default value is 0.
void ocularWhiteBalanceFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                              int Stride, float temperature, float tint) {

    int Channels = Stride / Width;
    if (Channels == 1) {
        return;
    }
    float Temperature = temperature;
    Temperature       = Temperature < 5000 ? (float)(0.0004 * (Temperature - 5000.0)) :
                                             (float)(0.00006 * (Temperature - 5000.0));

    float Tint = tint;
    Tint       = (float)(Tint / 100.0f);

    short YPrime = 0;
    short I      = 0;
    short Q      = 0;

    float warmFilterR = 0.93f;
    float warmFilterG = 0.54f;
    float warmFilterB = 0;
    int plusTint      = (int)(Tint * 255.0f * 0.5226f * 0.1f);

    short QTint[256]                    = { 0 };
    unsigned char processedMap[256 * 3] = { 0 };
    unsigned char* processedRMap        = &processedMap[0];
    unsigned char* processedGMap        = &processedMap[256];
    unsigned char* processedBMap        = &processedMap[512];
    for (int pixel = 0; pixel < 256; pixel++) {
        float fpixel         = pixel * (1.0f / 255.0f);
        QTint[pixel]         = (short)clamp((float)(pixel - 127 + plusTint), -127.0f, 127.0f);
        float processedR     = (fpixel < 0.5f ? (2.0f * fpixel * warmFilterR) :
                                                (1.0f - 2.0f * (1.0f - fpixel) * (1.0f - warmFilterR)));
        float processedG     = (fpixel < 0.5f ? (2.0f * fpixel * warmFilterG) :
                                                (1.0f - 2.0f * (1.0f - fpixel) * (1.0f - warmFilterG)));
        float processedB     = (fpixel < 0.5f ? (2.0f * fpixel * warmFilterB) :
                                                (1.0f - 2.0f * (1.0f - fpixel) * (1.0f - warmFilterB)));
        processedRMap[pixel] = ClampToByte(255.0f * mix(fpixel, processedR, Temperature));
        processedGMap[pixel] = ClampToByte(255.0f * mix(fpixel, processedG, Temperature));
        processedBMap[pixel] = ClampToByte(255.0f * mix(fpixel, processedB, Temperature));
    }
    for (int Y = 0; Y < Height; Y++) {
        unsigned char* pOutput = Output + (Y * Stride);
        unsigned char* pInput  = Input + (Y * Stride);
        for (int X = 0; X < Width; X++) {
            const unsigned char R = pInput[0];
            const unsigned char G = pInput[1];
            const unsigned char B = pInput[2];

            YPrime =
            ((int)(0.299f * 65536) * R + (int)(0.587f * 65536) * G + (int)(0.114f * 65536) * B) >> 16;
            I = ((int)(0.595f * 65536) * R - (int)(0.274453f * 65536) * G - (int)(0.321263f * 65536) * B) >> 16;
            Q = ((int)(0.211456f * 65536) * R - (int)(0.522591f * 65536) * G + (int)(0.311135f * 65536) * B) >> 16;
            // adjusting tint
            Q = QTint[Q + 127];
            // adjusting temperature
            pOutput[0] =
            processedRMap[ClampToByte(YPrime + (((int)(0.9563 * 65536) * I + (int)(0.6210 * 65536) * Q) >> 16))];
            pOutput[1] =
            processedGMap[ClampToByte(YPrime - (((int)(0.2721 * 65536) * I + (int)(0.6474 * 65536) * Q) >> 16))];
            pOutput[2] =
            processedBMap[ClampToByte(YPrime + (((int)(1.7046 * 65536) * Q - (int)(1.1070 * 65536) * I) >> 16))];

            pInput += Channels;
            pOutput += Channels;
        }
    }
}

// Adjusts the vibrance of an image
// vibrance : The vibrance adjustment to apply, using 0.0 as the default, and a suggested min / max
// of around - 1.2 and 1.2, respectively.
void ocularVibranceFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                          int Stride, float vibrance) {

    int Channels = Stride / Width;
    if (Channels == 1) {
        return;
    }
    int iVibrance    = (int)(-(vibrance * 256));
    unsigned char mx = 0;
    int amt          = 0;
    for (int Y = 0; Y < Height; Y++) {
        unsigned char* pOutput = Output + (Y * Stride);
        unsigned char* pInput  = Input + (Y * Stride);
        for (int X = 0; X < Width; X++) {
            const unsigned char r = pInput[0];
            const unsigned char g = pInput[1];
            const unsigned char b = pInput[2];
            mx                    = max3(r, g, b);
            amt                   = (3 * mx - (r + g + b)) * iVibrance;
            pOutput[0]            = ClampToByte((r * (255 * 256 - amt) + mx * amt) >> 16);
            pOutput[1]            = ClampToByte((g * (255 * 256 - amt) + mx * amt) >> 16);
            pOutput[2]            = ClampToByte((b * (255 * 256 - amt) + mx * amt) >> 16);

            pInput += Channels;
            pOutput += Channels;
        }
    }
}

void rgb2hsv(const unsigned char* R, const unsigned char* G, const unsigned char* B,
             unsigned char* H, unsigned char* S, unsigned char* V) {
    int r = *R;
    int g = *G;
    int b = *B;

    int h, s;
    int nMax = max3(r, g, b);
    int nMin = min3(r, g, b);
    int diff = nMax - nMin;

    if (diff == 0) {
        h = 0;
        s = 0;
    } else {
        if (nMin == b) {
            h = 60 * (g - r) / diff + 60;
        } else if (nMin == r) {
            h = 60 * (b - g) / diff + 180;
        } else {
            h = 60 * (r - b) / diff + 300;
        }
        // normalize 0`359
        // if (h < 0)    h += 360; if (h >= 360) h -= 360;
        if (!((unsigned)(int)(h) < (360))) {
            if (h < 0)
                h += 360;
            else
                h -= 360;
        }
        if (nMax == 0) {
            s = 0;
        } else {
            s = 255 * diff / nMax;
        }
    }

    *H = (unsigned char)(h >> 1); // 0`179
    *S = (unsigned char)s;        // 0`255
    *V = nMax;                    // 0`255
}

void hsv2rgb(const unsigned char* H, const unsigned char* S, const unsigned char* V,
             unsigned char* R, unsigned char* G, unsigned char* B) {

    if (S > (const unsigned char*)0) {
        int r, g, b;
        r       = *V;
        g       = *V;
        b       = *V;
        float h = *H * (6.0f / 180.0f); // 0`180 -> 0.0`1.0
        int i   = (int)h;
        int f   = (int)(256 * (h - (float)i));
        int VS  = (*V * *S) >> 8;
        int VSF = VS * f;
        switch (i) {
        case 0:
            b -= VS;
            g = b + (VSF >> 8);
            break;
        case 1:
            r = *V - (VSF >> 8);
            b -= VS;
            break;
        case 2:
            r -= VS;
            b = r + (VSF >> 8);
            break;
        case 3:
            r -= VS;
            g -= (VSF >> 8);
            break;
        case 4:
            g -= VS;
            r = g + (VSF >> 8);
            break;
        case 5:
            g -= VS;
            b -= (VSF >> 8);
            break;
        }
        *R = (unsigned char)(r);
        *G = (unsigned char)(g);
        *B = (unsigned char)(b);
    } else {
        *R = *V;
        *G = *V;
        *B = *V;
    }
}

// A skin-tone adjustment filter that affects a unique range of light skin-tone colors and adjusts
// the pink / green or pink / orange range accordingly. Default values are targeted at fair
// caucasian skin, but can be adjusted as required.
// skinToneAdjust : Amount to adjust skin tone.Default : 0.0, suggested min / max : -0.3 and 0.3
// respectively.
// skinHue : Skin hue to be detected.Default : 0.05 (fair caucasian to reddish skin).
// skinHueThreshold : Amount of variance in skin hue. Default : 40.0.
// maxHueShift : Maximum amount of hue shifting allowed. Default : 0.25.
// maxSaturationShift = Maximum amount of saturation to be shifted(when using orange). Default: 0.4.
// upperSkinToneColor = GPUImageSkinToneUpperColorGreen or GPUImageSkinToneUpperColorOrange
void ocularSkinToneFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                          int Stride, float skinToneAdjust, float skinHue, float skinHueThreshold,
                          float maxHueShift, float maxSaturationShift, int upperSkinToneColor) {

    int Channels = Stride / Width;
    if (Channels == 1) {
        return;
    }
    int maxSatShiftAdjust     = (int)(maxSaturationShift * 255.0f * skinToneAdjust);
    float maxHueShiftAdjust   = maxHueShift * skinToneAdjust;
    unsigned char hueMap[256] = { 0 };
    int satMap[256]           = { 0 };
    for (unsigned int H = 0; H < 256; H++) {
        satMap[H] = 0;
        float hue = H * (1.0f / 255.0f);
        // check how far from skin hue
        float dist = hue - skinHue;
        if (dist > 0.5f)
            dist -= 1.0f;
        if (dist < -0.5f)
            dist += 1.0f;
        dist = (float)(fabs(dist) * (1.0f / 0.5f)); // normalized to [0,1]
        // Apply Gaussian like filter
        float weightMap = clamp(expf(-dist * dist * skinHueThreshold), 0.0f, 1.0f);

        // Using pink/green, so only adjust hue
        if (upperSkinToneColor == 0) {
            hue += maxHueShiftAdjust * weightMap;
            // Using pink/orange, so adjust hue < 0 and saturation > 0
        } else if (upperSkinToneColor == 1) {
            // We want more orange, so increase saturation
            if (skinToneAdjust > 0.0f)
                satMap[H] = (int)(maxSatShiftAdjust * weightMap);
            // we want more pinks, so decrease hue
            else
                hue += maxHueShiftAdjust * weightMap;
        }
        hueMap[H] = ClampToByte(hue * 255.0f);
    }
    unsigned char H, S, V, _S;
    for (int Y = 0; Y < Height; Y++) {
        unsigned char* pOutput = Output + (Y * Stride);
        unsigned char* pInput  = Input + (Y * Stride);
        for (int X = 0; X < Width; X++) {
            unsigned char R = pInput[0];
            unsigned char G = pInput[1];
            unsigned char B = pInput[2];
            // Convert color to HSV, extract hue
            rgb2hsv(&R, &G, &B, &H, &S, &V);
            // final color
            _S = (S + satMap[H]);
            hsv2rgb(&hueMap[H], &_S, &V, &pOutput[0], &pOutput[1], &pOutput[2]);

            pInput += Channels;
            pOutput += Channels;
        }
    }
}

void CalGaussianCoeff(float sigma, float* a0, float* a1, float* a2, float* a3, float* b1, float* b2,
                      float* cprev, float* cnext) {
    float alpha, lamma, k;

    if (sigma < 0.5f)
        sigma = 0.5f;
    alpha  = (float)exp((0.726) * (0.726)) / sigma;
    lamma  = (float)exp(-alpha);
    *b2    = (float)exp(-2 * alpha);
    k      = (1 - lamma) * (1 - lamma) / (1 + 2 * alpha * lamma - (*b2));
    *a0    = k;
    *a1    = k * (alpha - 1) * lamma;
    *a2    = k * (alpha + 1) * lamma;
    *a3    = -k * (*b2);
    *b1    = -2 * lamma;
    *cprev = (*a0 + *a1) / (1 + *b1 + *b2);
    *cnext = (*a2 + *a3) / (1 + *b1 + *b2);
}

void gaussianHorizontal(unsigned char* bufferPerLine, unsigned char* lpRowInitial,
                        unsigned char* lpColumn, int width, int height, int Channels, int Nwidth,
                        float a0a1, float a2a3, float b1b2, float cprev, float cnext) {
    int HeightStep  = Channels * height;
    int WidthSubOne = width - 1;
    if (Channels == 3) {
        float prevOut[3];
        prevOut[0] = (lpRowInitial[0] * cprev);
        prevOut[1] = (lpRowInitial[1] * cprev);
        prevOut[2] = (lpRowInitial[2] * cprev);
        for (int x = 0; x < width; ++x) {
            prevOut[0]       = ((lpRowInitial[0] * (a0a1)) - (prevOut[0] * (b1b2)));
            prevOut[1]       = ((lpRowInitial[1] * (a0a1)) - (prevOut[1] * (b1b2)));
            prevOut[2]       = ((lpRowInitial[2] * (a0a1)) - (prevOut[2] * (b1b2)));
            bufferPerLine[0] = (unsigned char)prevOut[0];
            bufferPerLine[1] = (unsigned char)prevOut[1];
            bufferPerLine[2] = (unsigned char)prevOut[2];
            bufferPerLine += Channels;
            lpRowInitial += Channels;
        }
        lpRowInitial -= Channels;
        lpColumn += HeightStep * WidthSubOne;
        bufferPerLine -= Channels;
        prevOut[0] = (lpRowInitial[0] * cnext);
        prevOut[1] = (lpRowInitial[1] * cnext);
        prevOut[2] = (lpRowInitial[2] * cnext);

        for (int x = WidthSubOne; x >= 0; --x) {
            prevOut[0] = ((lpRowInitial[0] * (a2a3)) - (prevOut[0] * (b1b2)));
            prevOut[1] = ((lpRowInitial[1] * (a2a3)) - (prevOut[1] * (b1b2)));
            prevOut[2] = ((lpRowInitial[2] * (a2a3)) - (prevOut[2] * (b1b2)));
            bufferPerLine[0] += (unsigned char)prevOut[0];
            bufferPerLine[1] += (unsigned char)prevOut[1];
            bufferPerLine[2] += (unsigned char)prevOut[2];
            lpColumn[0] = bufferPerLine[0];
            lpColumn[1] = bufferPerLine[1];
            lpColumn[2] = bufferPerLine[2];
            lpRowInitial -= Channels;
            lpColumn -= HeightStep;
            bufferPerLine -= Channels;
        }
    } else if (Channels == 4) {
        float prevOut[4];

        prevOut[0] = (lpRowInitial[0] * cprev);
        prevOut[1] = (lpRowInitial[1] * cprev);
        prevOut[2] = (lpRowInitial[2] * cprev);
        prevOut[3] = (lpRowInitial[3] * cprev);
        for (int x = 0; x < width; ++x) {
            prevOut[0] = ((lpRowInitial[0] * (a0a1)) - (prevOut[0] * (b1b2)));
            prevOut[1] = ((lpRowInitial[1] * (a0a1)) - (prevOut[1] * (b1b2)));
            prevOut[2] = ((lpRowInitial[2] * (a0a1)) - (prevOut[2] * (b1b2)));
            prevOut[3] = ((lpRowInitial[3] * (a0a1)) - (prevOut[3] * (b1b2)));

            bufferPerLine[0] = (unsigned char)prevOut[0];
            bufferPerLine[1] = (unsigned char)prevOut[1];
            bufferPerLine[2] = (unsigned char)prevOut[2];
            bufferPerLine[3] = (unsigned char)prevOut[3];
            bufferPerLine += Channels;
            lpRowInitial += Channels;
        }
        lpRowInitial -= Channels;
        lpColumn += HeightStep * WidthSubOne;
        bufferPerLine -= Channels;

        prevOut[0] = (lpRowInitial[0] * cnext);
        prevOut[1] = (lpRowInitial[1] * cnext);
        prevOut[2] = (lpRowInitial[2] * cnext);
        prevOut[3] = (lpRowInitial[3] * cnext);

        for (int x = WidthSubOne; x >= 0; --x) {
            prevOut[0] = ((lpRowInitial[0] * a2a3) - (prevOut[0] * b1b2));
            prevOut[1] = ((lpRowInitial[1] * a2a3) - (prevOut[1] * b1b2));
            prevOut[2] = ((lpRowInitial[2] * a2a3) - (prevOut[2] * b1b2));
            prevOut[3] = ((lpRowInitial[3] * a2a3) - (prevOut[3] * b1b2));
            bufferPerLine[0] += (unsigned char)prevOut[0];
            bufferPerLine[1] += (unsigned char)prevOut[1];
            bufferPerLine[2] += (unsigned char)prevOut[2];
            bufferPerLine[3] += (unsigned char)prevOut[3];
            lpColumn[0] = bufferPerLine[0];
            lpColumn[1] = bufferPerLine[1];
            lpColumn[2] = bufferPerLine[2];
            lpColumn[3] = bufferPerLine[3];
            lpRowInitial -= Channels;
            lpColumn -= HeightStep;
            bufferPerLine -= Channels;
        }
    } else if (Channels == 1) {
        float prevOut = (lpRowInitial[0] * cprev);

        for (int x = 0; x < width; ++x) {
            prevOut          = ((lpRowInitial[0] * (a0a1)) - (prevOut * (b1b2)));
            bufferPerLine[0] = (unsigned char)prevOut;
            bufferPerLine += Channels;
            lpRowInitial += Channels;
        }
        lpRowInitial -= Channels;
        lpColumn += HeightStep * WidthSubOne;
        bufferPerLine -= Channels;

        prevOut = (lpRowInitial[0] * cnext);

        for (int x = WidthSubOne; x >= 0; --x) {
            prevOut = ((lpRowInitial[0] * a2a3) - (prevOut * b1b2));
            bufferPerLine[0] += (unsigned char)prevOut;
            lpColumn[0] = bufferPerLine[0];
            lpRowInitial -= Channels;
            lpColumn -= HeightStep;
            bufferPerLine -= Channels;
        }
    }
}

void gaussianVertical(unsigned char* bufferPerLine, unsigned char* lpRowInitial,
                      unsigned char* lpColInitial, int height, int width, int Channels, float a0a1,
                      float a2a3, float b1b2, float cprev, float cnext) {

    int WidthStep    = Channels * width;
    int HeightSubOne = height - 1;
    if (Channels == 3) {
        float prevOut[3];
        prevOut[0] = (lpRowInitial[0] * cprev);
        prevOut[1] = (lpRowInitial[1] * cprev);
        prevOut[2] = (lpRowInitial[2] * cprev);

        for (int y = 0; y < height; y++) {
            prevOut[0]       = ((lpRowInitial[0] * a0a1) - (prevOut[0] * b1b2));
            prevOut[1]       = ((lpRowInitial[1] * a0a1) - (prevOut[1] * b1b2));
            prevOut[2]       = ((lpRowInitial[2] * a0a1) - (prevOut[2] * b1b2));
            bufferPerLine[0] = (unsigned char)prevOut[0];
            bufferPerLine[1] = (unsigned char)prevOut[1];
            bufferPerLine[2] = (unsigned char)prevOut[2];
            bufferPerLine += Channels;
            lpRowInitial += Channels;
        }
        lpRowInitial -= Channels;
        bufferPerLine -= Channels;
        lpColInitial += WidthStep * HeightSubOne;
        prevOut[0] = (lpRowInitial[0] * cnext);
        prevOut[1] = (lpRowInitial[1] * cnext);
        prevOut[2] = (lpRowInitial[2] * cnext);
        for (int y = HeightSubOne; y >= 0; y--) {
            prevOut[0] = ((lpRowInitial[0] * a2a3) - (prevOut[0] * b1b2));
            prevOut[1] = ((lpRowInitial[1] * a2a3) - (prevOut[1] * b1b2));
            prevOut[2] = ((lpRowInitial[2] * a2a3) - (prevOut[2] * b1b2));
            bufferPerLine[0] += (unsigned char)prevOut[0];
            bufferPerLine[1] += (unsigned char)prevOut[1];
            bufferPerLine[2] += (unsigned char)prevOut[2];
            lpColInitial[0] = bufferPerLine[0];
            lpColInitial[1] = bufferPerLine[1];
            lpColInitial[2] = bufferPerLine[2];
            lpRowInitial -= Channels;
            lpColInitial -= WidthStep;
            bufferPerLine -= Channels;
        }
    } else if (Channels == 4) {
        float prevOut[4];

        prevOut[0] = (lpRowInitial[0] * cprev);
        prevOut[1] = (lpRowInitial[1] * cprev);
        prevOut[2] = (lpRowInitial[2] * cprev);
        prevOut[3] = (lpRowInitial[3] * cprev);

        for (int y = 0; y < height; y++) {
            prevOut[0]       = ((lpRowInitial[0] * a0a1) - (prevOut[0] * b1b2));
            prevOut[1]       = ((lpRowInitial[1] * a0a1) - (prevOut[1] * b1b2));
            prevOut[2]       = ((lpRowInitial[2] * a0a1) - (prevOut[2] * b1b2));
            prevOut[3]       = ((lpRowInitial[3] * a0a1) - (prevOut[3] * b1b2));
            bufferPerLine[0] = (unsigned char)prevOut[0];
            bufferPerLine[1] = (unsigned char)prevOut[1];
            bufferPerLine[2] = (unsigned char)prevOut[2];
            bufferPerLine[3] = (unsigned char)prevOut[3];
            bufferPerLine += Channels;
            lpRowInitial += Channels;
        }
        lpRowInitial -= Channels;
        bufferPerLine -= Channels;
        lpColInitial += WidthStep * HeightSubOne;
        prevOut[0] = (lpRowInitial[0] * cnext);
        prevOut[1] = (lpRowInitial[1] * cnext);
        prevOut[2] = (lpRowInitial[2] * cnext);
        prevOut[3] = (lpRowInitial[3] * cnext);
        for (int y = HeightSubOne; y >= 0; y--) {
            prevOut[0] = ((lpRowInitial[0] * a2a3) - (prevOut[0] * b1b2));
            prevOut[1] = ((lpRowInitial[1] * a2a3) - (prevOut[1] * b1b2));
            prevOut[2] = ((lpRowInitial[2] * a2a3) - (prevOut[2] * b1b2));
            prevOut[3] = ((lpRowInitial[3] * a2a3) - (prevOut[3] * b1b2));
            bufferPerLine[0] += (unsigned char)prevOut[0];
            bufferPerLine[1] += (unsigned char)prevOut[1];
            bufferPerLine[2] += (unsigned char)prevOut[2];
            bufferPerLine[3] += (unsigned char)prevOut[3];
            lpColInitial[0] = bufferPerLine[0];
            lpColInitial[1] = bufferPerLine[1];
            lpColInitial[2] = bufferPerLine[2];
            lpColInitial[3] = bufferPerLine[3];
            lpRowInitial -= Channels;
            lpColInitial -= WidthStep;
            bufferPerLine -= Channels;
        }
    } else if (Channels == 1) {
        float prevOut = 0;
        prevOut       = (lpRowInitial[0] * cprev);
        for (int y = 0; y < height; y++) {
            prevOut          = ((lpRowInitial[0] * a0a1) - (prevOut * b1b2));
            bufferPerLine[0] = (unsigned char)prevOut;
            bufferPerLine += Channels;
            lpRowInitial += Channels;
        }
        lpRowInitial -= Channels;
        bufferPerLine -= Channels;
        lpColInitial += WidthStep * HeightSubOne;
        prevOut = (lpRowInitial[0] * cnext);
        for (int y = HeightSubOne; y >= 0; y--) {
            prevOut = ((lpRowInitial[0] * a2a3) - (prevOut * b1b2));
            bufferPerLine[0] += (unsigned char)prevOut;
            lpColInitial[0] = bufferPerLine[0];
            lpRowInitial -= Channels;
            lpColInitial -= WidthStep;
            bufferPerLine -= Channels;
        }
    }
}

// A hardware-optimized, variable radius Gaussian blur
void ocularGaussianBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                              int Stride, float GaussianSigma) {

    int Channels = Stride / Width;
    float a0, a1, a2, a3, b1, b2, cprev, cnext;

    CalGaussianCoeff(GaussianSigma, &a0, &a1, &a2, &a3, &b1, &b2, &cprev, &cnext);

    float a0a1 = (a0 + a1);
    float a2a3 = (a2 + a3);
    float b1b2 = (b1 + b2);

    int bufferSizePerThread      = (Width > Height ? Width : Height) * Channels;
    unsigned char* bufferPerLine = (unsigned char*)malloc(bufferSizePerThread);
    unsigned char* tempData      = (unsigned char*)malloc(Height * Stride);
    if (bufferPerLine == NULL || tempData == NULL) {
        if (tempData) {
            free(tempData);
        }
        if (bufferPerLine) {
            free(bufferPerLine);
        }
        return;
    }
    for (int y = 0; y < Height; ++y) {
        unsigned char* lpRowInitial = Input + Stride * y;
        unsigned char* lpColInitial = tempData + y * Channels;
        gaussianHorizontal(bufferPerLine, lpRowInitial, lpColInitial, Width, Height, Channels,
                           Width, a0a1, a2a3, b1b2, cprev, cnext);
    }
    int HeightStep = Height * Channels;
    for (int x = 0; x < Width; ++x) {
        unsigned char* lpColInitial = Output + x * Channels;
        unsigned char* lpRowInitial = tempData + HeightStep * x;
        gaussianVertical(bufferPerLine, lpRowInitial, lpColInitial, Height, Width, Channels, a0a1,
                         a2a3, b1b2, cprev, cnext);
    }

    free(bufferPerLine);
    free(tempData);
}


#define float2fixed(x) (((int)((x)*4096.0f + 0.5f)) << 8)


void rgb2ycbcr(unsigned char R, unsigned char G, unsigned char B, unsigned char* y,
               unsigned char* cb, unsigned char* cr) {
    *y  = (unsigned char)((19595 * R + 38470 * G + 7471 * B) >> 16);
    *cb = (unsigned char)(((36962 * (B - *y)) >> 16) + 128);
    *cr = (unsigned char)(((46727 * (R - *y)) >> 16) + 128);
}


void ycbcr2rgb(unsigned char y, unsigned char Cb, unsigned char Cr, unsigned char* R,
               unsigned char* G, unsigned char* B) {
    int y_fixed = (y << 20) + (1 << 19); // rounding
    int r, g, b;
    int cr = Cr - 128;
    int cb = Cb - 128;
    r      = y_fixed + cr * float2fixed(1.40200f);
    g      = y_fixed + (cr * -float2fixed(0.71414f)) + ((cb * -float2fixed(0.34414f)) * 0xffff0000);
    b      = y_fixed + cb * float2fixed(1.77200f);
    r >>= 20;
    g >>= 20;
    b >>= 20;
    if ((unsigned)r > 255) {
        if (r < 0)
            r = 0;
        else
            r = 255;
    }
    if ((unsigned)g > 255) {
        if (g < 0)
            g = 0;
        else
            g = 255;
    }
    if ((unsigned)b > 255) {
        if (b < 0)
            b = 0;
        else
            b = 255;
    }
    *R = r;
    *G = g;
    *B = b;
}

// Applies an unsharp mask
// GaussianSigma : The blur radius of the underlying Gaussian blur. The default is 4.0.
// intensity : The strength of the sharpening, from 0.0 on up, with a default of 1.0
void ocularUnsharpMaskFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                             int Stride, float GaussianSigma, int intensity) {

    int Channels = Stride / Width;
    intensity    = max(min(intensity, 100), 0);
    int c1       = 256 * (100 - intensity) / 100;
    int c2       = 256 * (100 - (100 - intensity)) / 100;

    unsigned char unsharpMaskMap[256 * 256] = { 0 };
    for (unsigned int PS = 0; PS < 256; PS++) {
        unsigned char* pUnsharpMaskMap = unsharpMaskMap + (PS << 8);
        for (unsigned int PD = 0; PD < 256; PD++) {
            unsigned char retPD = ClampToByte((PS - PD) + 128);
            retPD = ((PS <= 128) ? (retPD * PS / 128) : (255 - (255 - retPD) * (255 - PS) / 128));
            // enhanced edge method
            //   unsigned char retPD = ClampToByte((PS - PD) + PS);
            pUnsharpMaskMap[0] = ClampToByte((PS * c1 + retPD * c2) >> 8);
            pUnsharpMaskMap++;
        }
    }

    switch (Channels) {
    case 4:
    case 3: {
        unsigned char* Temp = (unsigned char*)malloc(Width * Height * (sizeof(unsigned char)));
        unsigned char* Blur = (unsigned char*)malloc(Width * Height * (sizeof(unsigned char)));
        if (Blur == NULL || Temp == NULL) {
            if (Blur) {
                free(Blur);
            }
            if (Temp) {
                free(Temp);
            }
            return;
        }
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pInput = Input + (Y * Stride);
            unsigned char* pTemp  = Temp + (Y * Width);
            unsigned char* pBlur  = Blur + (Y * Width);
            for (int X = 0; X < Width; X++) {
                pTemp[0] = (unsigned char)((19595 * pInput[0] + 38470 * pInput[1] + 7471 * pInput[2]) >> 16);

                pBlur[0] = pTemp[0];

                pInput += Channels;
                pTemp++;
                pBlur++;
            }
        }
        ocularGaussianBlurFilter(Temp, Blur, Width, Height, Width, GaussianSigma);
        unsigned char cb, cr;
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pInput  = Input + (Y * Stride);
            unsigned char* pOutput = Output + (Y * Stride);
            unsigned char* pTemp   = Temp + (Y * Width);
            unsigned char* pBlur   = Blur + (Y * Width);
            for (int x = 0; x < Width; x++) {
                cb = (unsigned char)((36962 * (pInput[2] - (int)(pTemp[0])) >> 16) + 128);
                cr = (unsigned char)((46727 * (pInput[0] - (int)(pTemp[0])) >> 16) + 128);
                // Sharpen: High Contrast Overlay

                unsigned char* pUnsharpMaskMap = unsharpMaskMap + (pTemp[0] << 8);

                ycbcr2rgb(pUnsharpMaskMap[pBlur[0]], cb, cr, &pOutput[0], &pOutput[1], &pOutput[2]);

                pTemp++;
                pBlur++;
                pOutput += Channels;
                pInput += Channels;
            }
        }
        free(Temp);
        free(Blur);
        break;
    }


    case 1: {
        unsigned char* Blur = (unsigned char*)malloc(Width * Height * (sizeof(unsigned char)));
        if (Blur == NULL) {
            return;
        }

        ocularGaussianBlurFilter(Input, Blur, Width, Height, Width, GaussianSigma);

        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pInput  = Input + (Y * Width);
            unsigned char* pBlur   = Blur + (Y * Width);
            unsigned char* pOutput = Output + (Y * Width);
            for (int x = 0; x < Width; x++) {
                // Sharpen: High Contrast Overlay
                pOutput[0]                     = pInput[0] - pOutput[0] + 128;
                unsigned char* pUnsharpMaskMap = unsharpMaskMap + (pInput[0] << 8);
                pOutput[0]                     = pUnsharpMaskMap[pOutput[0]];

                pBlur++;
                pOutput++;
                pInput++;
            }
        }
        free(Blur);
    }

    break;

    default: break;
    }
}

inline void boxfilterRow(unsigned char* Input, unsigned char* Output, int Width, int Height,
                         int Channels, int Radius) {
    int iRadius          = Radius + 1;
    int iScale           = (int)((256.0f * 256.0f) / (2 * Radius + 1));
    int iWidthStep       = Width * Channels;
    int iWidthStepDec    = (Width - 1) * Channels;
    int iRadChannels     = Radius * Channels;
    int iRadChannelsPlus = (iRadChannels + Channels);
    switch (Channels) {
    case 1: {
        for (int y = 0; y < Height; y++) {
            //  Process left edge
            int iY  = y * iWidthStep;
            int sum = Input[iY] * Radius;
            for (int x = 0; x < iRadius; x++) {
                int p = (y * Width + x) * Channels;
                sum += Input[p];
            }
            Output[iY] = (sum * iScale) >> 16;
            for (int x = 1; x < iRadius; x++) {
                int pLeft = iY + x * Channels;
                int p0    = pLeft + iRadChannels;

                sum += Input[p0];
                sum -= Input[iY];
                Output[pLeft] = (sum * iScale) >> 16;
            }

            // core zone
            for (int x = iRadius; x < Width - Radius; x++) {
                int pKernal = iY + x * Channels;

                int i0 = pKernal + iRadChannels;
                int i1 = pKernal - iRadChannelsPlus;

                sum += Input[i0];
                sum -= Input[i1];

                Output[pKernal] = (sum * iScale) >> 16;
            }

            // Process right edge
            for (int x = Width - Radius; x < Width; x++) {
                int iRight = iY + x * Channels;
                int i0     = iY + iWidthStepDec;
                int i1     = iRight - iRadChannelsPlus;

                sum += Input[i0];
                sum -= Input[i1];
                Output[iRight] = (sum * iScale) >> 16;
            }
        }
        break;
    }
    case 3: {
        for (int y = 0; y < Height; y++) {
            //  Process left edge

            int iY   = y * iWidthStep;
            int sumR = Input[iY] * Radius;
            int sumG = Input[iY + 1] * Radius;
            int sumB = Input[iY + 2] * Radius;
            for (int x = 0; x < iRadius; x++) {
                int i = iY + x * Channels;
                sumR += Input[i];
                sumG += Input[i + 1];
                sumB += Input[i + 2];
            }
            Output[iY]     = (sumR * iScale) >> 16;
            Output[iY + 1] = (sumG * iScale) >> 16;
            Output[iY + 2] = (sumB * iScale) >> 16;
            for (int x = 1; x < iRadius; x++) {
                int iLeft = iY + x * Channels;
                int i0    = iLeft + iRadChannels;

                sumR += Input[i0];
                sumR -= Input[iY];
                sumG += Input[i0 + 1];
                sumG -= Input[iY + 1];
                sumB += Input[i0 + 2];
                sumB -= Input[iY + 2];
                Output[iLeft]     = (sumR * iScale) >> 16;
                Output[iLeft + 1] = (sumG * iScale) >> 16;
                Output[iLeft + 2] = (sumB * iScale) >> 16;
            }

            // core zone
            for (int x = iRadius; x < Width - Radius; x++) {
                int iKernal = iY + x * Channels;

                int i0 = iKernal + iRadChannels;
                int i1 = iKernal - iRadChannelsPlus;

                sumR += Input[i0];
                sumR -= Input[i1];

                sumG += Input[i0 + 1];
                sumG -= Input[i1 + 1];

                sumB += Input[i0 + 2];
                sumB -= Input[i1 + 2];
                Output[iKernal]     = (sumR * iScale) >> 16;
                Output[iKernal + 1] = (sumG * iScale) >> 16;
                Output[iKernal + 2] = (sumB * iScale) >> 16;
            }

            // Process right edge
            for (int x = Width - Radius; x < Width; x++) {
                int iRight = iY + x * Channels;
                int i0     = iY + iWidthStepDec;
                int i1     = iRight - iRadChannelsPlus;

                sumR += Input[i0];
                sumR -= Input[i1];

                sumG += Input[i0 + 1];
                sumG -= Input[i1 + 1];

                sumB += Input[i0 + 2];
                sumB -= Input[i1 + 2];
                Output[iRight]     = (sumR * iScale) >> 16;
                Output[iRight + 1] = (sumG * iScale) >> 16;
                Output[iRight + 2] = (sumB * iScale) >> 16;
            }
        }
        break;
    }
    case 4: {
        for (int y = 0; y < Height; y++) {
            //  Process left edge
            int iY   = y * iWidthStep;
            int sumR = Input[iY] * Radius;
            int sumG = Input[iY + 1] * Radius;
            int sumB = Input[iY + 2] * Radius;
            for (int x = 0; x < iRadius; x++) {
                int i = iY + x * Channels;
                sumR += Input[i];
                sumG += Input[i + 1];
                sumB += Input[i + 2];
            }
            Output[iY]     = (sumR * iScale) >> 16;
            Output[iY + 1] = (sumG * iScale) >> 16;
            Output[iY + 2] = (sumB * iScale) >> 16;
            Output[iY + 3] = Input[iY + 3];
            for (int x = 1; x < iRadius; x++) {
                int iLeft = iY + x * Channels;
                int i0    = iLeft + iRadChannels;
                sumR += Input[i0];
                sumR -= Input[iLeft];
                sumG += Input[i0 + 1];
                sumG -= Input[iLeft + 1];
                sumB += Input[i0 + 2];
                sumB -= Input[iLeft + 2];
                Output[iLeft]     = (sumR * iScale) >> 16;
                Output[iLeft + 1] = (sumG * iScale) >> 16;
                Output[iLeft + 2] = (sumB * iScale) >> 16;
                Output[iLeft + 3] = Input[iLeft + 3];
            }

            // core zone
            for (int x = iRadius; x < Width - Radius; x++) {
                int iKernal = iY + x * Channels;

                int i0 = iKernal + iRadChannels;
                int i1 = iKernal - iRadChannelsPlus;

                sumR += Input[i0];
                sumR -= Input[i1];

                sumG += Input[i0 + 1];
                sumG -= Input[i1 + 1];

                sumB += Input[i0 + 2];
                sumB -= Input[i1 + 2];
                Output[iKernal]     = (sumR * iScale) >> 16;
                Output[iKernal + 1] = (sumG * iScale) >> 16;
                Output[iKernal + 2] = (sumB * iScale) >> 16;
                Output[iKernal + 3] = Input[iKernal + 3];
            }

            // Process right edge
            for (int x = Width - Radius; x < Width; x++) {
                int iRight = iY + x * Channels;
                int i0     = iY + iWidthStepDec;
                int i1     = iRight - iRadChannelsPlus;

                sumR += Input[i0];
                sumR -= Input[i1];

                sumG += Input[i0 + 1];
                sumG -= Input[i1 + 1];

                sumB += Input[i0 + 2];
                sumB -= Input[i1 + 2];
                Output[iRight]     = (sumR * iScale) >> 16;
                Output[iRight + 1] = (sumG * iScale) >> 16;
                Output[iRight + 2] = (sumB * iScale) >> 16;
                Output[iRight + 3] = Input[iRight + 3];
            }
        }
        break;
    }
    default: break;
    }
}

inline void boxfilterCol(unsigned char* Input, unsigned char* Output, int Width, int Height,
                         int Channels, int Radius) {
    int iScale           = (int)((256.0f * 256.0f) / (2 * Radius + 1));
    int iWidthStep       = Width * Channels;
    int iWidthStepDec    = (Height - 1) * iWidthStep;
    int iRadWidthStep    = Radius * iWidthStep;
    int iRadWidthStepDec = (iRadWidthStep + iWidthStep);
    int iHeightRadius    = Height - Radius;
    int iRadius          = Radius + 1;
    switch (Channels) {
    case 1: {
        for (int x = 0; x < Width; x++) {
            //  Process left edge
            int iX  = x * Channels;
            int sum = Input[iX] * Radius;
            for (int y = 0; y < iRadius; y++) {
                int i = (y * Width + x) * Channels;
                sum += Input[i];
            }
            Output[x] = (sum * iScale) >> 16;

            for (int y = 1; y < iRadius; y++) {
                int i = iX + y * iWidthStep;

                int i0 = i + iRadWidthStep;
                int i1 = x * Channels;


                sum += Input[i0];
                sum -= Input[i1];
                Output[i] = (sum * iScale) >> 16;
            }

            // core zone
            for (int y = iRadius; y < iHeightRadius; y++) {

                int iKernal = iX + y * iWidthStep;
                int i0      = iKernal + iRadWidthStep;
                int i1      = iKernal - iRadWidthStepDec;

                sum += Input[i0];
                sum -= Input[i1];
                Output[iKernal] = (sum * iScale) >> 16;
            }

            // Process right edge
            for (int y = iHeightRadius; y < Height; y++) {
                int iRight = iX + y * iWidthStep;

                int i0 = iWidthStepDec + x * Channels;
                int i1 = iRight - iRadWidthStepDec;


                sum += Input[i0];
                sum -= Input[i1];
                Output[iRight] = (sum * iScale) >> 16;
            }
        }


        break;
    }
    case 3: {
        for (int x = 0; x < Width; x++) {
            //  Process left edge
            int iX   = x * Channels;
            int sumR = Input[iX] * Radius;
            int sumG = Input[iX + 1] * Radius;
            int sumB = Input[iX + 2] * Radius;
            for (int y = 0; y < iRadius; y++) {
                int i = iX + y * iWidthStep;
                sumR += Input[i];
                sumG += Input[i + 1];
                sumB += Input[i + 2];
            }
            Output[iX]     = (sumR * iScale) >> 16;
            Output[iX + 1] = (sumG * iScale) >> 16;
            Output[iX + 2] = (sumB * iScale) >> 16;

            for (int y = 1; y < iRadius; y++) {
                int i  = iX + y * iWidthStep;
                int i0 = i + iRadWidthStep;

                sumR += Input[i0];
                sumR -= Input[iX];
                sumG += Input[i0 + 1];
                sumG -= Input[iX + 1];
                sumB += Input[i0 + 2];
                sumB -= Input[iX + 2];
                Output[i]     = (sumR * iScale) >> 16;
                Output[i + 1] = (sumG * iScale) >> 16;
                Output[i + 2] = (sumB * iScale) >> 16;
            }

            // core zone
            for (int y = iRadius; y < iHeightRadius; y++) {

                int iKernal = iX + y * iWidthStep;

                int i0 = iKernal + iRadWidthStep;
                int i1 = iKernal - iRadWidthStepDec;


                sumR += Input[i0];
                sumR -= Input[i1];
                sumG += Input[i0 + 1];
                sumG -= Input[i1 + 1];
                sumB += Input[i0 + 2];
                sumB -= Input[i1 + 2];
                Output[iKernal]     = (sumR * iScale) >> 16;
                Output[iKernal + 1] = (sumG * iScale) >> 16;
                Output[iKernal + 2] = (sumB * iScale) >> 16;
            }

            // Process right edge
            for (int y = iHeightRadius; y < Height; y++) {
                int iRight = iX + y * iWidthStep;
                int i0     = iWidthStepDec + iX;
                int i1     = iRight - iRadWidthStepDec;

                sumR += Input[i0];
                sumR -= Input[i1];
                sumG += Input[i0 + 1];
                sumG -= Input[i1 + 1];
                sumB += Input[i0 + 2];
                sumB -= Input[i1 + 2];
                Output[iRight]     = (sumR * iScale) >> 16;
                Output[iRight + 1] = (sumG * iScale) >> 16;
                Output[iRight + 2] = (sumB * iScale) >> 16;
            }
        }


        break;
    }
    case 4: {
        for (int x = 0; x < Width; x++) {
            //  Process left edge
            int iX   = x * Channels;
            int sumR = Input[iX] * Radius;
            int sumG = Input[iX + 1] * Radius;
            int sumB = Input[iX + 2] * Radius;
            for (int y = 0; y < iRadius; y++) {
                int i = iX + y * iWidthStep;
                sumR += Input[i];
                sumG += Input[i + 1];
                sumB += Input[i + 2];
            }
            Output[iX]     = (sumR * iScale) >> 16;
            Output[iX + 1] = (sumG * iScale) >> 16;
            Output[iX + 2] = (sumB * iScale) >> 16;
            Output[iX + 3] = Input[iX + 3];
            for (int y = 1; y < iRadius; y++) {
                int i  = iX + y * iWidthStep;
                int i0 = i + iRadWidthStep;
                sumR += Input[i0];
                sumR -= Input[iX];
                sumG += Input[i0 + 1];
                sumG -= Input[iX + 1];
                sumB += Input[i0 + 2];
                sumB -= Input[iX + 2];
                Output[i]     = (sumR * iScale) >> 16;
                Output[i + 1] = (sumG * iScale) >> 16;
                Output[i + 2] = (sumB * iScale) >> 16;
                Output[i + 3] = Input[i + 3];
            }

            // core zone
            for (int y = iRadius; y < iHeightRadius; y++) {

                int iKernal = iX + y * iWidthStep;
                int i0      = iKernal + iRadWidthStep;
                int i1      = iKernal - iRadWidthStepDec;
                sumR += Input[i0];
                sumR -= Input[i1];
                sumG += Input[i0 + 1];
                sumG -= Input[i1 + 1];
                sumB += Input[i0 + 2];
                sumB -= Input[i1 + 2];
                Output[iKernal]     = (sumR * iScale) >> 16;
                Output[iKernal + 1] = (sumG * iScale) >> 16;
                Output[iKernal + 2] = (sumB * iScale) >> 16;
                Output[iKernal + 3] = Input[iKernal + 3];
            }

            // Process right edge
            for (int y = iHeightRadius; y < Height; y++) {
                int iRight = iX + y * iWidthStep;

                int i0 = iWidthStepDec + iX;
                int i1 = iRight - iRadWidthStepDec;
                sumR += Input[i0];
                sumR -= Input[i1];
                sumG += Input[i0 + 1];
                sumG -= Input[i1 + 1];
                sumB += Input[i0 + 2];
                sumB -= Input[i1 + 2];
                Output[iRight]     = (sumR * iScale) >> 16;
                Output[iRight + 1] = (sumG * iScale) >> 16;
                Output[iRight + 2] = (sumB * iScale) >> 16;
                Output[iRight + 3] = Input[iRight + 3];
            }
        }
        break;
    }
    default: break;
    }
}

// A hardware-optimized, variable radius box blur
// Radius : A radius in pixels to use for the blur, with a default of 2.0. This adjusts the sigma
// variable in the Gaussian distribution function.
void ocularBoxBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                         int Stride, int Radius) {

    int Channels        = Stride / Width;
    unsigned char* temp = (unsigned char*)malloc(Width * Height * Channels);
    if (temp == NULL) {
        return;
    }
    boxfilterRow(Input, temp, Width, Height, Channels, Radius);
    boxfilterCol(temp, Output, Width, Height, Channels, Radius);
    free(temp);
}

void ocularSharpenFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                         int Stride, float Radius, int sharpness, int intensity) {

    int Channels = Stride / Width;
    intensity    = max(min(intensity, 100), 0);
    int c1       = 256 * (100 - intensity) / 100;
    int c2       = 256 * (100 - (100 - intensity)) / 100;
    // Sharpen: High Contrast Overlay
    unsigned char sharpnessMap[256 * 256] = { 0 };
    for (unsigned int PS = 0; PS < 256; PS++) {
        unsigned char* pSharpnessMap = sharpnessMap + (PS << 8);
        for (unsigned int PD = 0; PD < 256; PD++) {
            unsigned char retPD = ClampToByte((sharpness * (PS - PD)) + 128);
            retPD = ((PS <= 128) ? (retPD * PS / 128) : (255 - (255 - retPD) * (255 - PS) / 128));
            // enhanced edge method
            //   unsigned char retPD = ClampToByte(sharpness*(PS - PD) + PS);

            pSharpnessMap[0] = ClampToByte((PS * c1 + retPD * c2) >> 8);
            pSharpnessMap++;
        }
    }
    switch (Channels) {
    case 4:
    case 3: {
        unsigned char* temp = (unsigned char*)malloc(Width * Height * (sizeof(unsigned char)));
        unsigned char* blur = (unsigned char*)malloc(Width * Height * (sizeof(unsigned char)));
        if (blur == NULL || temp == NULL) {
            if (temp) {
                free(temp);
            }
            if (blur) {
                free(blur);
            }
            return;
        }
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pInput = Input + (Y * Stride);
            unsigned char* pTemp  = temp + (Y * Width);
            unsigned char* pBlur  = blur + (Y * Width);
            for (int X = 0; X < Width; X++) {
                pTemp[0] = (unsigned char)((19595 * pInput[0] + 38470 * pInput[1] + 7471 * pInput[2]) >> 16);
                pBlur[0] = pTemp[0];
                pInput += Channels;
                pTemp++;
                pBlur++;
            }
        }
        ocularBoxBlurFilter(temp, blur, Width, Height, Width, (int)Radius);
        unsigned char cb, cr;
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pInput  = Input + (Y * Stride);
            unsigned char* pOutput = Output + (Y * Stride);
            unsigned char* pTemp   = temp + (Y * Width);
            unsigned char* pBlur   = blur + (Y * Width);
            for (int x = 0; x < Width; x++) {
                cb = (unsigned char)((36962 * (pInput[2] - (int)(pTemp[0])) >> 16) + 128);
                cr = (unsigned char)((46727 * (pInput[0] - (int)(pTemp[0])) >> 16) + 128);
                // Sharpen: High Contrast Overlay
                unsigned char* pSharpnessMap = sharpnessMap + (pTemp[0] << 8);

                ycbcr2rgb(pSharpnessMap[pBlur[0]], cb, cr, &pOutput[0], &pOutput[1], &pOutput[2]);

                pTemp++;
                pBlur++;
                pOutput += Channels;
                pInput += Channels;
            }
        }
        free(temp);
        free(blur);
        break;
    }


    case 1: {

        unsigned char* Blur = (unsigned char*)malloc(Width * Height * (sizeof(unsigned char)));
        if (Blur == NULL) {
            return;
        }

        ocularBoxBlurFilter(Input, Blur, Width, Height, Width, (int)Radius);

        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pInput  = Input + (Y * Width);
            unsigned char* pBlur   = Blur + (Y * Width);
            unsigned char* pOutput = Output + (Y * Width);
            for (int x = 0; x < Width; x++) {
                unsigned char* pSharpnessMap = sharpnessMap + (pInput[0] << 8);
                pOutput[0]                   = pSharpnessMap[pOutput[0]];


                pBlur++;
                pOutput++;
                pInput++;
            }
        }
        free(Blur);
    }

    break;

    default: break;
    }
}

// Bilinear interpolation. This lets you up - or downsample an image using Lanczos resampling,
// which results in noticeably better quality than the standard linear or trilinear interpolation.
void ocularResamplingFilter(unsigned char* Input, unsigned int Width, unsigned int Height, unsigned int Stride,
                            unsigned char* Output, int newWidth, int newHeight, int dstStride) {

    int Channels  = Stride / Width;
    int dstOffset = dstStride - Channels * newWidth;
    float xFactor = (float)Width / newWidth;
    float yFactor = (float)Height / newHeight;

    int ymax = Height - 1;
    int xmax = Width - 1;

    for (int y = 0; y < newHeight; y++) {
        float oy  = (float)y * yFactor;
        int oy1   = (int)oy;
        int oy2   = (oy1 == ymax) ? oy1 : oy1 + 1;
        float dy1 = oy - (float)oy1;
        float dy2 = 1.0f - dy1;

        unsigned char* tp1 = Input + oy1 * Stride;
        unsigned char* tp2 = Input + oy2 * Stride;

        for (int x = 0; x < newWidth; x++) {
            float ox          = (float)x * xFactor;
            int ox1           = (int)ox;
            int ox2           = (ox1 == xmax) ? ox1 : ox1 + 1;
            float dx1         = ox - (float)ox1;
            float dx2         = 1.0f - dx1;
            unsigned char* p1 = tp1 + ox1 * Channels;
            unsigned char* p2 = tp1 + ox2 * Channels;
            unsigned char* p3 = tp2 + ox1 * Channels;
            unsigned char* p4 = tp2 + ox2 * Channels;

            for (int i = 0; i < Channels; i++, Output++, p1++, p2++, p3++, p4++) {
                *Output =
                (unsigned char)(dy2 * (dx2 * (*p1) + dx1 * (*p2)) + dy1 * (dx2 * (*p3) + dx1 * (*p4)));
            }
        }
        Output += dstOffset;
    }
}

void ocularCropFilter(const unsigned char* Input, int Width, int Height, int srcStride, unsigned char* Output,
                      int cropX, int cropY, int dstWidth, int dstHeight, int dstStride) {

    int Channels = srcStride / Width;

    const unsigned char* src = Input + cropY * srcStride + cropX * Channels;
    unsigned char* dst       = Output;

    for (int y = 0; y < dstHeight; y++) {
        memcpy(dst, src, dstStride);
        src += srcStride;
        dst += dstStride;
    }
}

void ocularAutoLevel(const unsigned char* Input, unsigned char* Output, int Width, int Height,
                     int Stride, float fraction) {

    int Channels = Stride / Width;
    switch (Channels) {
    case 4:
    case 3: {
        unsigned int histoRGB[256 * 3] = { 0 };
        unsigned int* histoR           = &histoRGB[0];
        unsigned int* histoG           = &histoRGB[256];
        unsigned int* histoB           = &histoRGB[512];
        for (int Y = 0; Y < Height; Y++) {
            const unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                histoR[pInput[0]]++;
                histoG[pInput[1]]++;
                histoB[pInput[2]]++;
                pInput += Channels;
            }
        }
        int thresholdRMin = 0;
        int thresholdRMax = 0;
        int thresholdGMin = 0;
        int thresholdGMax = 0;
        int thresholdBMin = 0;
        int thresholdBMax = 0;
        int gap           = (int)(fraction * Width * Height);
        int sumR          = 0;
        int sumG          = 0;
        int sumB          = 0;

        for (int i = 0; sumR < gap; ++i) {
            sumR += histoR[i];
            thresholdRMin = i;
        }
        sumR = 0;
        for (int i = 255; sumR < gap; --i) {
            sumR += histoR[i];
            thresholdRMax = i;
        }
        for (int i = 0; sumG < gap; ++i) {
            sumG += histoG[i];
            thresholdGMin = i;
        }
        sumG = 0;
        for (int i = 255; sumG < gap; --i) {
            sumG += histoG[i];
            thresholdGMax = i;
        }
        for (int i = 0; sumB < gap; ++i) {
            sumB += histoB[i];
            thresholdBMin = i;
        }
        sumB = 0;
        for (int i = 255; sumB < gap; --i) {
            sumB += histoB[i];
            thresholdBMax = i;
        }
        unsigned char MapRGB[256 * 3] = { 0 };
        unsigned char* MapB           = &MapRGB[0];
        unsigned char* MapG           = &MapRGB[256];
        unsigned char* MapR           = &MapRGB[512];
        for (int i = 0; i < 256; i++) {
            if (i < thresholdRMin)
                MapR[i] = (i + 0) >> 1;
            else if (i > thresholdRMax)
                MapR[i] = (255);
            else
                MapR[i] = ClampToByte((int)((i - thresholdRMin) * 255.0) / (thresholdRMax - thresholdRMin));
            if (i < thresholdGMin)
                MapG[i] = (i + 0) >> 1;
            else if (i > thresholdGMax)
                MapG[i] = (255);
            else
                MapG[i] = ClampToByte((int)((i - thresholdGMin) * 255.0) / (thresholdGMax - thresholdGMin));
            if (i < thresholdBMin)
                MapB[i] = (0);
            else if (i > thresholdBMax)
                MapB[i] = (255);
            else
                MapB[i] = ClampToByte((int)((i - thresholdBMin) * 255.0) / (thresholdBMax - thresholdBMin));
        }

        for (int Y = 0; Y < Height; Y++) {
            const unsigned char* pInput = Input + (Y * Stride);
            unsigned char* pOutput      = Output + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                pOutput[0] = MapR[pInput[0]];
                pOutput[1] = MapG[pInput[1]];
                pOutput[2] = MapB[pInput[2]];
                pInput += Channels;
                pOutput += Channels;
            }
        }
        break;
    }
    case 1: {
        unsigned int histoGray[256] = { 0 };
        for (int Y = 0; Y < Height; Y++) {
            const unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                histoGray[pInput[0]]++;
                pInput++;
            }
        }
        int thresholdMin = 0;
        int thresholdMax = 0;
        int gap          = (int)(fraction * Width * Height);
        int sumGray      = 0;
        for (int i = 0; sumGray < gap; ++i) {
            sumGray += histoGray[i];
            thresholdMin = i;
        }
        sumGray = 0;
        for (int i = 255; sumGray < gap; --i) {
            sumGray += histoGray[i];
            thresholdMax = i;
        }
        unsigned char MapGray[256] = { 0 };
        if ((thresholdMax - thresholdMin) <= 0)
            return;
        for (int i = 0; i < 256; i++) {
            if (i < thresholdMin)
                MapGray[i] = (0);
            else if (i > thresholdMax)
                MapGray[i] = (255);
            else
                MapGray[i] = ClampToByte((int)((i - thresholdMin) * 255.0f) / (thresholdMax - thresholdMin));
        }

        for (int Y = 0; Y < Height; Y++) {
            const unsigned char* pInput = Input + (Y * Stride);

            unsigned char* pOutput = Output + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                pOutput[0] = MapGray[pInput[0]];
                pInput++;
                pOutput++;
            }
        }
    } break;

    default: break;
    }
}