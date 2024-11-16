#include <math.h>
#include <string.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "threshold.h"
#include "color.h"
#include "ocr.h"
#include "retinex.h"    
#include "ocular.h"
#include "util.h"


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

    OC_STATUS ocularGrayscaleFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;
        
        int Channels = Stride / Width;
        if (Channels != 1 && Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        const int B_WT = (int)(0.114 * 256 + 0.5);
        const int G_WT = (int)(0.587 * 256 + 0.5);
        const int R_WT = 256 - B_WT - G_WT; //     int(0.299 * 256 + 0.5);
        int Channel = Stride / Width;
        if (Channel == 3) {
            for (int Y = 0; Y < Height; Y++) {
                unsigned char* LinePS = Input + Y * Stride;
                unsigned char* LinePD = Output + Y * Width;
                int X = 0;
                for (; X < Width - 4; X += 4, LinePS += Channel * 4) {
                    LinePD[X + 0] = (B_WT * LinePS[0] + G_WT * LinePS[1] + R_WT * LinePS[2]) >> 8;
                    LinePD[X + 1] = (B_WT * LinePS[3] + G_WT * LinePS[4] + R_WT * LinePS[5]) >> 8;
                    LinePD[X + 2] = (B_WT * LinePS[6] + G_WT * LinePS[7] + R_WT * LinePS[8]) >> 8;
                    LinePD[X + 3] = (B_WT * LinePS[9] + G_WT * LinePS[10] + R_WT * LinePS[11]) >> 8;
                }
                for (; X < Width; X++, LinePS += Channel) {
                    LinePD[X] = (B_WT * LinePS[0] + G_WT * LinePS[1] + R_WT * LinePS[2]) >> 8;
                }
            }
        } else if (Channel == 4) {
            for (int Y = 0; Y < Height; Y++) {
                unsigned char* LinePS = Input + Y * Stride;
                unsigned char* LinePD = Output + Y * Width;
                int X = 0;
                for (; X < Width - 4; X += 4, LinePS += Channel * 4) {
                    LinePD[X + 0] = (B_WT * LinePS[0] + G_WT * LinePS[1] + R_WT * LinePS[2]) >> 8;
                    LinePD[X + 1] = (B_WT * LinePS[4] + G_WT * LinePS[5] + R_WT * LinePS[6]) >> 8;
                    LinePD[X + 2] = (B_WT * LinePS[8] + G_WT * LinePS[9] + R_WT * LinePS[10]) >> 8;
                    LinePD[X + 3] = (B_WT * LinePS[12] + G_WT * LinePS[13] + R_WT * LinePS[14]) >> 8;
                }
                for (; X < Width; X++, LinePS += Channel) {
                    LinePD[X] = (B_WT * LinePS[0] + G_WT * LinePS[1] + R_WT * LinePS[2]) >> 8;
                }
            }
        } else if (Channel == 1) {
            if (Output != Input) {
                memcpy(Output, Input, Height * Stride);
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularRGBFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float redAdjustment,
                         float greenAdjustment, float blueAdjustment) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels == 1 || Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        redAdjustment = clamp(redAdjustment, 0.0, 1.0);
        greenAdjustment = clamp(greenAdjustment, 0.0, 1.0);
        blueAdjustment = clamp(blueAdjustment, 0.0, 1.0);

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
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                pOutput[0] = AdjustMapR[pInput[0]];
                pOutput[1] = AdjustMapG[pInput[1]];
                pOutput[2] = AdjustMapB[pInput[2]];
                pInput += Channels;
                pOutput += Channels;
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularHSLFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                             float hueAdjustment, float satAdjustment, float lightAdjustment) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels == 1 && Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        hueAdjustment = clamp(hueAdjustment, 0.0, 360.0);
        satAdjustment = clamp(satAdjustment, 0.0, 1.0);
        lightAdjustment = clamp(lightAdjustment, 0.0, 1.0);

        float r, g, b;
        float h, s, l;
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * Stride);
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                r = pInput[0];
                g = pInput[1];
                b = pInput[2];

                // get the hue and saturation
                rgb2hsl(r, g, b, &h, &s, &l);

                // Apply modifiers
                h = h + hueAdjustment;
                if (h > 1)
                    h -= 1;
                if (h < 0.0)
                    h += 1;

                s = s * satAdjustment;
                if (s < 0)
                    s = 0;
                if (s > 1)
                    s = 1;

                l = l + lightAdjustment;
                if (l < 0)
                    l = 0;
                if (l > 1)
                    l = 1;

                hsl2rgb(h, s, l, &r, &g, &b);

                pOutput[0] = ClampToByte(r);
                pOutput[1] = ClampToByte(g);
                pOutput[2] = ClampToByte(b);

                pInput += Channels;
                pOutput += Channels;
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularAverageLuminanceThresholdFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, 
                                                    int Stride, float thresholdMultiplier) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels != 1 && Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        thresholdMultiplier = clamp(thresholdMultiplier, 0.0, 1.0);

        unsigned char Luminance = 0;
        if (Channels == 1) {
            int numberOfPixels = Width * Height;
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
                unsigned char* pInput = Input + (Y * Stride);
                for (int X = 0; X < Width; X++) {
                    pOutput[0] = step(Luminance, pInput[0]);
                    pInput++;
                    pOutput++;
                }
            }
        } else if (Channels == 3 || Channels == 4) {

            int numberOfPixels = Width * Height;
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
                unsigned char* pInput = Input + (Y * Stride);
                for (int X = 0; X < Width; X++) {
                    unsigned char luminance = (unsigned char)((13926 * pInput[0] + 46884 * pInput[1] + 4725 * pInput[2]) >> 16);
                    pOutput[2] = pOutput[1] = pOutput[0] = step(Luminance, luminance);
                    pInput += Channels;
                    pOutput += Channels;
                }
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularAverageColor(unsigned char* Input, int Width, int Height, int Stride, unsigned char* AverageR, 
                                 unsigned char* AverageG, unsigned char* AverageB, unsigned char* AverageA) {

        if (Input == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels != 1 && Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        if (Channels == 1) {
            int numberOfPixels = Width * Height;
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
            *AverageR = (unsigned char)(Sum / numberOfPixels);
            *AverageG = *AverageR;
            *AverageB = *AverageR;
            *AverageA = *AverageR;
        } else if (Channels == 3) {

            int numberOfPixels = Width * Height;
            unsigned int histogramRGB[768] = { 0 };
            unsigned int* histogramR = &histogramRGB[0];
            unsigned int* histogramG = &histogramRGB[256];
            unsigned int* histogramB = &histogramRGB[512];
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
            *AverageR = (unsigned char)(SumPixR / numberOfPixels);
            *AverageG = (unsigned char)(SumPixG / numberOfPixels);
            *AverageB = (unsigned char)(SumPixB / numberOfPixels);
            *AverageA = 255;
        } else if (Channels == 4) {

            int numberOfPixels = Width * Height;
            unsigned int histogramRGB[768 + 256] = { 0 };
            unsigned int* histogramR = &histogramRGB[0];
            unsigned int* histogramG = &histogramRGB[256];
            unsigned int* histogramB = &histogramRGB[512];
            unsigned int* histogramA = &histogramRGB[768];
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
            *AverageR = (unsigned char)(SumPixR / numberOfPixels);
            *AverageG = (unsigned char)(SumPixG / numberOfPixels);
            *AverageB = (unsigned char)(SumPixB / numberOfPixels);
            *AverageA = (unsigned char)(SumPixA / numberOfPixels);
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularLuminosity(unsigned char* Input, int Width, int Height, int Stride, unsigned char* Luminance) {

        if (Input == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels != 1 && Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        if (Channels == 1) {
            int numberOfPixels = Width * Height;
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

            int numberOfPixels = Width * Height;
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

        return OC_STATUS_OK;
    }

    OC_STATUS ocularColorMatrixFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                     float* colorMatrix, float intensity) {

        if (Input == NULL || Output == NULL || colorMatrix == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels == 1 && Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        intensity = clamp(intensity, 0.0, 1.0);

        unsigned char degreeMap[256 * 256] = { 0 };
        for (int pixel = 0; pixel < 256; pixel++) {
            unsigned char* pDegreeMap = degreeMap + pixel * 256;
            for (int out = 0; out < 256; out++) {
                pDegreeMap[0] = degree((unsigned char)pixel, (unsigned char)out, intensity);
                pDegreeMap++;
            }
        }
        unsigned char colorMatrixMap[256 * 16] = { 0 };
        unsigned char* colorMatrix0 = &colorMatrixMap[0];
        unsigned char* colorMatrix1 = &colorMatrixMap[256];
        unsigned char* colorMatrix2 = &colorMatrixMap[256 * 2];
        unsigned char* colorMatrix3 = &colorMatrixMap[256 * 3];
        unsigned char* colorMatrix4 = &colorMatrixMap[256 * 4];
        unsigned char* colorMatrix5 = &colorMatrixMap[256 * 5];
        unsigned char* colorMatrix6 = &colorMatrixMap[256 * 6];
        unsigned char* colorMatrix7 = &colorMatrixMap[256 * 7];
        unsigned char* colorMatrix8 = &colorMatrixMap[256 * 8];
        unsigned char* colorMatrix9 = &colorMatrixMap[256 * 9];
        unsigned char* colorMatrix10 = &colorMatrixMap[256 * 10];
        unsigned char* colorMatrix11 = &colorMatrixMap[256 * 11];
        unsigned char* colorMatrix12 = &colorMatrixMap[256 * 12];
        unsigned char* colorMatrix13 = &colorMatrixMap[256 * 13];
        unsigned char* colorMatrix14 = &colorMatrixMap[256 * 14];
        unsigned char* colorMatrix15 = &colorMatrixMap[256 * 15];
        for (int pixel = 0; pixel < 256; pixel++) {
            colorMatrix0[pixel] = ClampToByte(pixel * colorMatrix[0]);
            colorMatrix1[pixel] = ClampToByte(pixel * colorMatrix[1]);
            colorMatrix2[pixel] = ClampToByte(pixel * colorMatrix[2]);
            colorMatrix3[pixel] = ClampToByte(pixel * colorMatrix[3]);
            colorMatrix4[pixel] = ClampToByte(pixel * colorMatrix[4]);
            colorMatrix5[pixel] = ClampToByte(pixel * colorMatrix[5]);
            colorMatrix6[pixel] = ClampToByte(pixel * colorMatrix[6]);
            colorMatrix7[pixel] = ClampToByte(pixel * colorMatrix[7]);
            colorMatrix8[pixel] = ClampToByte(pixel * colorMatrix[8]);
            colorMatrix9[pixel] = ClampToByte(pixel * colorMatrix[9]);
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
                unsigned char* pInput = Input + (Y * WidthStep);
                for (int X = 0; X < Width; X++) {
                    const unsigned char r = pInput[0];
                    const unsigned char g = pInput[1];
                    const unsigned char b = pInput[2];
                    const unsigned char a = pInput[3];
                    outR = ClampToByte(colorMatrix0[r] + colorMatrix1[g] + colorMatrix2[b] + colorMatrix3[a]);
                    unsigned char* pDegreeMapR = degreeMap + (r << 8);
                    pOutput[0] = pDegreeMapR[outR];
                    outG = ClampToByte(colorMatrix4[r] + colorMatrix5[g] + colorMatrix6[b] + colorMatrix7[a]);
                    unsigned char* pDegreeMapG = degreeMap + (g << 8);
                    pOutput[1] = pDegreeMapG[outG];
                    outB = ClampToByte(colorMatrix8[r] + colorMatrix9[g] + colorMatrix10[b] + colorMatrix11[a]);
                    unsigned char* pDegreeMapB = degreeMap + (b << 8);
                    pOutput[2] = pDegreeMapB[outB];
                    outA = ClampToByte(colorMatrix12[r] + colorMatrix13[g] + colorMatrix14[b] + colorMatrix15[a]);
                    unsigned char* pDegreeMapA = degreeMap + (a << 8);
                    pOutput[3] = pDegreeMapA[outA];
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
                unsigned char* pInput = Input + (Y * WidthStep);
                for (int X = 0; X < Width; X++) {
                    const unsigned char r = pInput[0];
                    const unsigned char g = pInput[1];
                    const unsigned char b = pInput[2];
                    outR = ClampToByte(colorMatrix0[r] + colorMatrix1[g] + colorMatrix2[b]);
                    unsigned char* pDegreeMapR = degreeMap + (r << 8);
                    pOutput[0] = pDegreeMapR[outR];
                    outG = ClampToByte(colorMatrix4[r] + colorMatrix5[g] + colorMatrix6[b]);
                    unsigned char* pDegreeMapG = degreeMap + (g << 8);
                    pOutput[1] = pDegreeMapG[outG];
                    outB = ClampToByte(colorMatrix8[r] + colorMatrix9[g] + colorMatrix10[b]);
                    unsigned char* pDegreeMapB = degreeMap + (b << 8);
                    pOutput[2] = pDegreeMapB[outB];
                    pInput += Channels;
                    pOutput += Channels;
                }
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularSepiaFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                int intensity) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels == 1 && Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        intensity = clamp(intensity, 0, 100);

        float fIntensity = intensity / 100.0f;

        float colorMatrix[4 * 4] = { 0.3588f, 0.7044f, 0.1368f, 0.0f, 0.2990f, 0.5870f, 0.1140f, 0.0f,
                                     0.2392f, 0.4696f, 0.0912f, 0.0f, 0.f,     0.f,     0.f,     1.f };
        OC_STATUS status = ocularColorMatrixFilter(Input, Output, Width, Height, Stride, colorMatrix, fIntensity);
        if (status != OC_STATUS_OK)
            return status;

        return OC_STATUS_OK;
    }

    OC_STATUS ocularChromaKeyFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                    unsigned char colorToReplaceR, unsigned char colorToReplaceG, 
                                    unsigned char colorToReplaceB, float thresholdSensitivity, 
                                    float smoothing) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels == 1 && Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        colorToReplaceR = ClampToByte(colorToReplaceR);
        colorToReplaceG = ClampToByte(colorToReplaceG);
        colorToReplaceB = ClampToByte(colorToReplaceB);
        thresholdSensitivity = clamp(thresholdSensitivity, 0.0, 1.0);
        smoothing = clamp(smoothing, 0.0, 1.0);

        unsigned char maskY = (unsigned char)((19589 * colorToReplaceR + 38443 * colorToReplaceG + 7504 * 
                                                colorToReplaceB) >> 16);

        unsigned char maskCr = (unsigned char)((46740 * (colorToReplaceR - maskY) >> 16) + 128);

        unsigned char maskCb = (unsigned char)((37008 * (colorToReplaceB - maskY) >> 16) + 128);
        int iThresholdSensitivity = (int)(thresholdSensitivity * 255.0f);
        int iSmoothing = (int)(smoothing * 256);
        if (Channels == 3) {
            short blendMap[256 * 256] = { 0 };
            for (int Cr = 0; Cr < 256; Cr++) {
                short* pBlendMap = blendMap + (Cr << 8);
                for (int Cb = 0; Cb < 256; Cb++) {
                    pBlendMap[Cb] = (short)(255.0f *
                                    smoothstep((float)iThresholdSensitivity, (float)iThresholdSensitivity + iSmoothing,
                                                vec2_distance((float)Cr, (float)Cb, (float)maskCr, (float)maskCb)));
                }
            }
            for (int Y = 0; Y < Height; Y++) {
                unsigned char* pOutput = Output + (Y * Stride);
                unsigned char* pInput = Input + (Y * Stride);
                for (int X = 0; X < Width; X++) {
                    const unsigned char R = pInput[0];
                    const unsigned char G = pInput[1];
                    const unsigned char B = pInput[2];
                    unsigned char y = (unsigned char)((19589 * R + 38443 * G + 7504 * B) >> 16);
                    unsigned char Cr = (unsigned char)((46740 * (R - y) >> 16) + 128);
                    unsigned char Cb = (unsigned char)((37008 * (B - y) >> 16) + 128);
                    // Multiply by 255 to obtain the mask. If not multiplied by 255, the
                    // mask will be eliminated.
                    short* pBlendMap = blendMap + (Cr << 8);
                    const short blendValue = pBlendMap[Cb];
                    pOutput[0] = (unsigned char)(255 - (R * blendValue));
                    pOutput[1] = (unsigned char)(255 - (G * blendValue));
                    pOutput[2] = (unsigned char)(255 - (B * blendValue));
                    pInput += Channels;
                    pOutput += Channels;
                }
            }
        } else if (Channels == 4) {
            unsigned char blendMap[256 * 256] = { 0 };
            for (int Cr = 0; Cr < 256; Cr++) {
                unsigned char* pBlendMap = blendMap + (Cr << 8);
                for (int Cb = 0; Cb < 256; Cb++) {
                    pBlendMap[Cb] = (unsigned char)(255.0f *
                                    smoothstep((float)iThresholdSensitivity, (float)iThresholdSensitivity + iSmoothing,
                                                vec2_distance((float)Cr, (float)Cb, (float)maskCr, (float)maskCb)));
                }
            }
            for (int Y = 0; Y < Height; Y++) {
                unsigned char* pOutput = Output + (Y * Stride);
                unsigned char* pInput = Input + (Y * Stride);
                for (int X = 0; X < Width; X++) {
                    const unsigned char R = pInput[0];
                    const unsigned char G = pInput[1];
                    const unsigned char B = pInput[2];
                    const unsigned char A = pInput[3];
                    unsigned char y = (unsigned char)((19589 * R + 38443 * G + 7504 * B) >> 16);
                    unsigned char Cr = (unsigned char)((46740 * (R - y) >> 16) + 128);
                    unsigned char Cb = (unsigned char)((37008 * (B - y) >> 16) + 128);
                    // Handle transparency channels directly
                    unsigned char* pBlendMap = blendMap + (Cr << 8);
                    const unsigned char blendValue = pBlendMap[Cb];
                    pOutput[3] = ClampToByte(A * blendValue);
                    pInput += Channels;
                    pOutput += Channels;
                }
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularLookupFilter(unsigned char* Input, unsigned char* Output, unsigned char* lookupTable, int Width, 
                                 int Height, int Stride, int intensity) {

        if (Input == NULL || Output == NULL || lookupTable == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels == 1 && Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        intensity = clamp(intensity, 0, 100);

        float preMap[256 * 5] = { 0 };
        float* pixelColorMap = &preMap[0];
        float* quad1yMap = &preMap[256];
        float* quad1xMap = &preMap[256 + 256];
        float* quad2yMap = &preMap[256 + 256 + 256];
        float* quad2xMap = &preMap[256 + 256 + 256 + 256];

        unsigned short fractMap[256] = { 0 };
        unsigned short sIntensity = (unsigned short)max(min(intensity, 100), 0);
        int c1 = 256 * (100 - sIntensity) / 100;
        int c2 = 256 * (100 - (100 - sIntensity)) / 100;

        for (int b = 0; b < 256; b++) {
            pixelColorMap[b] = b * (63.0f / 255.0f);
            fractMap[b] = (unsigned short)(256 * (pixelColorMap[b] - truncf(pixelColorMap[b])));
            quad1yMap[b] = floorf(floorf(pixelColorMap[b]) * (1.0f / 8.0f));
            quad1xMap[b] = floorf(pixelColorMap[b]) - (quad1yMap[b] * 8.0f);
            quad2yMap[b] = floorf(ceilf(pixelColorMap[b]) * (1.0f / 8.0f));
            quad2xMap[b] = ceilf(pixelColorMap[b]) - (quad2yMap[b] * 8.0f);
            quad1yMap[b] = quad1yMap[b] * 64.0f + 0.5f;
            quad2yMap[b] = quad2yMap[b] * 64.0f + 0.5f;
            quad1xMap[b] = quad1xMap[b] * 64.0f + 0.5f;
            quad2xMap[b] = quad2xMap[b] * 64.0f + 0.5f;
        }

        int lookupChannels = 3;
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * Stride);
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                unsigned char R = pInput[0];
                unsigned char G = pInput[1];
                unsigned char B = pInput[2];
                float green = pixelColorMap[G];
                float red = pixelColorMap[R];
                unsigned char* pLineLookup1 = &lookupTable[(((int)(quad1yMap[B] + green) << 9) + 
                                                (int)(quad1xMap[B] + red)) * lookupChannels];
                unsigned char* pLineLookup2 = &lookupTable[(((int)(quad2yMap[B] + green) << 9) + 
                                                (int)(quad2xMap[B] + red)) * lookupChannels];
                unsigned short fractB = fractMap[B];
                pOutput[0] = (unsigned char)((int)(R * c1 + ((*pLineLookup1++ * (256 - fractB) + 
                                                *pLineLookup2++ * fractB) >> 8) * c2) >> 8);
                pOutput[1] = (unsigned char)((int)(G * c1 + ((*pLineLookup1++ * (256 - fractB) + 
                                                *pLineLookup2++ * fractB) >> 8) * c2) >> 8);
                pOutput[2] = (unsigned char)((int)(B * c1 + ((*pLineLookup1++ * (256 - fractB) + 
                                                *pLineLookup2++ * fractB) >> 8) * c2) >> 8);
                pInput += Channels;
                pOutput += Channels;
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularSaturationFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                     float saturation) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels == 1 && Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        saturation = clamp(saturation, 0.0, 1.0);

        unsigned char SaturationMap[256 * 256] = { 0 };
        for (int gray = 0; gray < 256; gray++) {
            unsigned char* pSaturationMap = SaturationMap + (gray << 8);
            for (int i = 0; i < 256; i++) {
                pSaturationMap[0] = (unsigned char)((mix_u8((unsigned char)gray, (unsigned char)i, saturation) + i) 
                                                    * 0.5f);
                pSaturationMap++;
            }
        }
        // 0.2125*256.0*256.0=13926.4
        // 0.7154*256.0*256.0=46884.4544
        // 0.0721*256.0*256.0=4725.1456
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * Stride);
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                unsigned char* pSaturationMap = SaturationMap + (((13926 * pInput[0] + 46884 * pInput[1] + 4725 * 
                                                pInput[2]) >> 16) << 8);
                pOutput[0] = pSaturationMap[pInput[0]];
                pOutput[1] = pSaturationMap[pInput[1]];
                pOutput[2] = pSaturationMap[pInput[2]];
                pInput += Channels;
                pOutput += Channels;
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularGammaFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                 float gamma[]) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels != 1 && Channels != 3)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        for (int i = 0; i < Channels; i++) {
            if (gamma[i] <= 0)
                return OC_STATUS_ERR_INVALIDPARAMETER;
        }

        // Create lookup tables to speed up calculation
        unsigned char GammaMap[256][3];
        for (int i = 0; i < 256; i++) {
            for (int c = 0; c < Channels; c++) {
                // calculate gamma
                GammaMap[i][c] = ClampToByte(pow((float)i / 255.0, gamma[c]) * 255.0f);
                GammaMap[i][c] = ClampToByte(pow((float)i / 255.0, gamma[c]) * 255.0f);
                GammaMap[i][c] = ClampToByte(pow((float)i / 255.0, gamma[c]) * 255.0f);
            }
        }
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * Stride);
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                for (int c = 0; c < Channels; c++) {
                    pOutput[c] = GammaMap[pInput[c]][c];
                }
                pInput += Channels;
                pOutput += Channels;
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularContrastFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                   float contrast) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels == 1 && Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        contrast = clamp(contrast, 0.0, 4.0);

        unsigned char contrastMap[256] = { 0 };
        for (int pixel = 0; pixel < 256; pixel++) {
            contrastMap[pixel] = ClampToByte((pixel - 127) * contrast + 127);
        }
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * Stride);
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                pOutput[0] = contrastMap[pInput[0]];
                pOutput[1] = contrastMap[pInput[1]];
                pOutput[2] = contrastMap[pInput[2]];
                pInput += Channels;
                pOutput += Channels;
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularExposureFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                   float exposure) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels == 1 && Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        exposure = clamp(exposure, -10.0, 10.0);

        unsigned char exposureMap[256] = { 0 };
        for (int pixel = 0; pixel < 256; pixel++) {
            exposureMap[pixel] = ClampToByte(pixel * pow(2.0, exposure));
        }
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * Stride);
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                pOutput[0] = exposureMap[pInput[0]];
                pOutput[1] = exposureMap[pInput[1]];
                pOutput[2] = exposureMap[pInput[2]];
                pInput += Channels;
                pOutput += Channels;
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularBrightnessFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                     int brightness) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels == 1 && Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        brightness = clamp(brightness, -1.0, 1.0);

        unsigned char BrightnessMap[256] = { 0 };
        for (int pixel = 0; pixel < 256; pixel++) {
            BrightnessMap[pixel] = ClampToByte(pixel + brightness);
        }
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * Stride);
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                pOutput[0] = BrightnessMap[pInput[0]];
                pOutput[1] = BrightnessMap[pInput[1]];
                pOutput[2] = BrightnessMap[pInput[2]];
                pInput += Channels;
                pOutput += Channels;
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularFalseColorFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                     unsigned char firstColorR, unsigned char firstColorG, unsigned char firstColorB, 
                                     unsigned char secondColorR, unsigned char secondColorG, unsigned char secondColorB, 
                                     int intensity) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels == 1 && Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        firstColorR = ClampToByte(firstColorR);
        firstColorG = ClampToByte(firstColorG);
        firstColorB = ClampToByte(firstColorB);
        secondColorR = ClampToByte(secondColorR);
        secondColorG = ClampToByte(secondColorG);
        secondColorB = ClampToByte(secondColorB);
        intensity = clamp(intensity, 0, 100);

        unsigned short sIntensity = (unsigned short)max(min(intensity, 100), 0);
        int c1 = 256 * (100 - sIntensity) / 100;
        int c2 = 256 * (100 - (100 - sIntensity)) / 100;

        unsigned char ColorMapR[256] = { 0 };
        unsigned char ColorMapG[256] = { 0 };
        unsigned char ColorMapB[256] = { 0 };
        for (int pixel = 0; pixel < 256; pixel++) {
            float fPixel = pixel * (1.0f / 255.0f);
            ColorMapR[pixel] = mix_u8(firstColorR, secondColorR, fPixel);
            ColorMapG[pixel] = mix_u8(firstColorG, secondColorG, fPixel);
            ColorMapB[pixel] = mix_u8(firstColorB, secondColorB, fPixel);
        }
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * Stride);
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                unsigned char luminanceWeighting = (unsigned char)((13926 * pInput[0] + 46884 * pInput[1] + 4725 * 
                                                                  pInput[2]) >> 16);

                pOutput[0] = (unsigned char)((pInput[0] * c1 + ColorMapR[luminanceWeighting] * c2) >> 8);
                pOutput[1] = (unsigned char)((pInput[1] * c1 + ColorMapG[luminanceWeighting] * c2) >> 8);
                pOutput[2] = (unsigned char)((pInput[2] * c1 + ColorMapB[luminanceWeighting] * c2) >> 8);
                pInput += Channels;
                pOutput += Channels;
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularHazeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                               float distance, float slope, int intensity) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels == 1 && Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        distance = clamp(distance, -0.3, 0.3);
        slope = clamp(slope, -0.3, 0.3);
        intensity = clamp(intensity, 0, 100);

        unsigned short sIntensity = (unsigned short)max(min(intensity, 100), 0);
        int c1 = 256 * (100 - sIntensity) / 100;
        int c2 = 256 * (100 - (100 - sIntensity)) / 100;
        short* distanceColorMap = (short*)malloc(Height * sizeof(short));
        short* patchDistanceMap = (short*)malloc(Height * sizeof(short));
        if (distanceColorMap == NULL || patchDistanceMap == NULL) {
            if (distanceColorMap) {
                free(distanceColorMap);
            }
            if (patchDistanceMap) {
                free(patchDistanceMap);
            }
            return OC_STATUS_ERR_OUTOFMEMORY;
        }
        float color = 1.0f;
        for (int i = 0; i < Height; i++) {
            float d = i * (1.0f / Height) * slope + distance;
            distanceColorMap[i] = ClampToByte(255.0 * d * color);
            patchDistanceMap[i] = (short)(256 * clamp(1.0f / (1.0f - d), 0.0f, 1.0f));
        }
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * Stride);
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                pOutput[0] =
                        (unsigned char)((int)(pInput[0] * c1 + ((ClampToByte(pInput[0] - distanceColorMap[Y]) * 
                                                                 patchDistanceMap[Y]) >> 8) * c2) >> 8);
                pOutput[1] =
                        (unsigned char)((int)(pInput[1] * c1 + ((ClampToByte(pInput[1] - distanceColorMap[Y]) * 
                                                                 patchDistanceMap[Y]) >> 8) * c2) >> 8);
                pOutput[2] =
                        (unsigned char)((int)(pInput[2] * c1 + ((ClampToByte(pInput[2] - distanceColorMap[Y]) * 
                                                                 patchDistanceMap[Y]) >> 8) * c2) >> 8);
                pInput += Channels;
                pOutput += Channels;
            }
        }
        free(distanceColorMap);
        free(patchDistanceMap);

        return OC_STATUS_OK;
    }

    OC_STATUS ocularOpacityFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                   float opacity) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        opacity = clamp(opacity, 0.0, 1.0);

        unsigned char opacityMap[256] = { 0 };
        for (unsigned int pixel = 0; pixel < 256; pixel++) {
            opacityMap[pixel] = (unsigned char)(pixel * opacity);
        }
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * Stride);
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                pOutput[3] = opacityMap[pInput[3]];
                pInput += Channels;
                pOutput += Channels;
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularLevelsFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                            ocularLevelParams* redLevelParams, ocularLevelParams* greenLevelParams, 
                            ocularLevelParams* blueLevelParams) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels != 1 && Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        if (redLevelParams != NULL) {
            if (redLevelParams->Enable) {
                redLevelParams->levelMinimum = clamp(redLevelParams->levelMinimum, 0, 255);
                redLevelParams->levelMaximum = clamp(redLevelParams->levelMaximum, 0, 255);
                redLevelParams->levelMiddle = clamp(redLevelParams->levelMiddle, 0, 255);
                redLevelParams->minOutput = clamp(redLevelParams->minOutput, 0, 255);
                redLevelParams->maxOutput = clamp(redLevelParams->maxOutput, 0, 255);
            } 
        }
        if (greenLevelParams != NULL) {
            if (greenLevelParams->Enable) {
                greenLevelParams->levelMinimum = clamp(greenLevelParams->levelMinimum, 0, 255);
                greenLevelParams->levelMaximum = clamp(greenLevelParams->levelMaximum, 0, 255);
                greenLevelParams->levelMiddle = clamp(greenLevelParams->levelMiddle, 0, 255);
                greenLevelParams->minOutput = clamp(greenLevelParams->minOutput, 0, 255);
                greenLevelParams->maxOutput = clamp(greenLevelParams->maxOutput, 0, 255);
            }
        }
        if (blueLevelParams != NULL) {
            if (blueLevelParams->Enable) {
                blueLevelParams->levelMinimum = clamp(blueLevelParams->levelMinimum, 0, 255);
                blueLevelParams->levelMaximum = clamp(blueLevelParams->levelMaximum, 0, 255);
                blueLevelParams->levelMiddle = clamp(blueLevelParams->levelMiddle, 0, 255);
                blueLevelParams->minOutput = clamp(blueLevelParams->minOutput, 0, 255);
                blueLevelParams->maxOutput = clamp(blueLevelParams->maxOutput, 0, 255);
            }
        }

        unsigned char LevelMapR[256] = { 0 };
        unsigned char LevelMapG[256] = { 0 };
        unsigned char LevelMapB[256] = { 0 };
        for (int pixel = 0; pixel < 256; pixel++) {
            if (redLevelParams->Enable) {
                LevelMapR[pixel] =
                        (mix_u8((unsigned char)redLevelParams->minOutput, (unsigned char)redLevelParams->maxOutput,
                                (powf(min(max(pixel - redLevelParams->levelMinimum, (0.0f)) / (redLevelParams->levelMaximum - redLevelParams->levelMinimum), (255)),
                                      1.0f / (redLevelParams->levelMiddle * (1.0f / 255.0f))))));
            } else {
                LevelMapR[pixel] = (unsigned char)pixel;
            }
            if (greenLevelParams->Enable) {
                LevelMapG[pixel] = (mix_u8((unsigned char)greenLevelParams->minOutput, (unsigned char)greenLevelParams->maxOutput,
                                           (powf(min(max(pixel - greenLevelParams->levelMinimum, (0.0f)) /
                                                             (greenLevelParams->levelMaximum - greenLevelParams->levelMinimum),
                                                     (255)),
                                                 1.0f / (greenLevelParams->levelMiddle * (1.0f / 255.0f))))));
            } else {
                LevelMapG[pixel] = (unsigned char)pixel;
            }
            if (blueLevelParams->Enable) {
                LevelMapB[pixel] = (mix_u8((unsigned char)blueLevelParams->minOutput, (unsigned char)blueLevelParams->maxOutput,
                                           (powf(min(max(pixel - blueLevelParams->levelMinimum, (0.0f)) /
                                                             (blueLevelParams->levelMaximum - blueLevelParams->levelMinimum),
                                                     (255)),
                                                 1.0f / (blueLevelParams->levelMiddle * (1.0f / 255.0f))))));
            } else {
                LevelMapB[pixel] = (unsigned char)pixel;
            }
        }

        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * Stride);
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                pOutput[0] = LevelMapR[pInput[0]];
                pOutput[1] = LevelMapG[pInput[1]];
                pOutput[2] = LevelMapB[pInput[2]];
                pInput += Channels;
                pOutput += Channels;
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularHueFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float hueAdjust) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels != 1 && Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        hueAdjust = clamp(hueAdjust, 0.0f, 360.0f);

        hueAdjust = fmodf(hueAdjust, 360.0f) * 3.14159265358979323846f / 180.0f;
        float hueMap[256 * 256] = { 0 };
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
        float hue = 0;
        short YPrime = 0;
        short I = 0;
        short Q = 0;
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * Stride);
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                // Convert to YIQ
                rgb2yiq(&pInput[0], &pInput[1], &pInput[2], &YPrime, &I, &Q);
                // Calculate the hue and chroma
                float* pHueMap = hueMap + ((Q + 128) << 8);
                hue = pHueMap[I + 128];
                float* pChromaMap = ChromaMap + ((Q + 128) << 8);
                float chroma = pChromaMap[I + 128];
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

        return OC_STATUS_OK;
    }

    OC_STATUS ocularHighlightShadowTintFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                              float shadowTintR, float shadowTintG, float shadowTintB, float highlightTintR,    
                                              float highlightTintG, float highlightTintB, float shadowTintIntensity, 
                                              float highlightTintIntensity) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        shadowTintR = clamp(shadowTintR, 0.0f, 1.0f);
        shadowTintG = clamp(shadowTintG, 0.0f, 1.0f);
        shadowTintB = clamp(shadowTintB, 0.0f, 1.0f);
        highlightTintR = clamp(highlightTintR, 0.0f, 1.0f);
        highlightTintG = clamp(highlightTintG, 0.0f, 1.0f);
        highlightTintB = clamp(highlightTintB, 0.0f, 1.0f);
        shadowTintIntensity = clamp(shadowTintIntensity, 0.0f, 1.0f);
        highlightTintIntensity = clamp(highlightTintIntensity, 0.0f, 1.0f);

        unsigned char HighlightShadowMapR[256 * 256] = { 0 };
        unsigned char HighlightShadowMapG[256 * 256] = { 0 };
        unsigned char HighlightShadowMapB[256 * 256] = { 0 };
        for (int lum = 0; lum < 256; lum++) {
            float luminance = (1.0f / 255.0f) * lum;
            unsigned char* pHighlightShadowMapR = HighlightShadowMapR + (lum << 8);
            unsigned char* pHighlightShadowMapG = HighlightShadowMapG + (lum << 8);
            unsigned char* pHighlightShadowMapB = HighlightShadowMapB + (lum << 8);
            for (int pixel = 0; pixel < 256; pixel++) {
                float fpixel = (1.0f / 255.0f) * pixel;
                float shadowResultR = mix(fpixel, max(fpixel, mix(shadowTintR, fpixel, luminance)), shadowTintIntensity);
                float shadowResultG = mix(fpixel, max(fpixel, mix(shadowTintG, fpixel, luminance)), shadowTintIntensity);
                float shadowResultB = mix(fpixel, max(fpixel, mix(shadowTintB, fpixel, luminance)), shadowTintIntensity);
                float highlightResultR = mix(fpixel, min(shadowResultR, mix(shadowResultR, highlightTintR, luminance)), highlightTintIntensity);
                float highlightResultG = mix(fpixel, min(shadowResultG, mix(shadowResultG, highlightTintG, luminance)), highlightTintIntensity);
                float highlightResultB = mix(fpixel, min(shadowResultB, mix(shadowResultB, highlightTintB, luminance)), highlightTintIntensity);
                pHighlightShadowMapR[pixel] = ClampToByte(255.0f * mix(shadowResultR, highlightResultR, luminance));
                pHighlightShadowMapG[pixel] = ClampToByte(255.0f * mix(shadowResultG, highlightResultG, luminance));
                pHighlightShadowMapB[pixel] = ClampToByte(255.0f * mix(shadowResultB, highlightResultB, luminance));
            }
        }
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * Stride);
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                const unsigned char R = pInput[0];
                const unsigned char G = pInput[1];
                const unsigned char B = pInput[2];
                unsigned short lum = (unsigned short)((13926 * R + 46884 * G + 4725 * B) >> 16) << 8;
                unsigned char* pHighlightShadowMapR = HighlightShadowMapR + (lum);
                unsigned char* pHighlightShadowMapG = HighlightShadowMapG + (lum);
                unsigned char* pHighlightShadowMapB = HighlightShadowMapB + (lum);
                pOutput[0] = pHighlightShadowMapR[R];
                pOutput[1] = pHighlightShadowMapG[G];
                pOutput[2] = pHighlightShadowMapB[B];
                pInput += Channels;
                pOutput += Channels;
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularHighlightShadowFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                          float shadows, float highlights) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        shadows = clamp(shadows, 0.0f, 1.0f);
        highlights = clamp(highlights, 0.0f, 1.0f);

        short luminanceWeightingMap[256] = { 0 };
        short shadowMap[256] = { 0 };
        short highlightMap[256] = { 0 };

        int divLuminance[256 * 256] = { 0 };
        for (int pixel = 0; pixel < 256; pixel++) {
            luminanceWeightingMap[pixel] = (short)(pixel * 0.3f);
            float luminance = (1.0f / 255.0f) * pixel;
            shadowMap[pixel] = (short)(255.0f *
                                       clamp((powf(luminance, 1.0f / (shadows + 1.0f)) + (-0.76f) * 
                                             powf(luminance, 2.0f / (shadows + 1.0f))) - luminance,
                                             0.0f, 1.0f));
            highlightMap[pixel] =
                    (short)(255.0f *
                            clamp((1.0f - (powf(1.0f - luminance, 1.0f / (2.0f - highlights)) + (-0.8f) * 
                                  powf(1.0f - luminance, 2.0f / (2.0f - highlights)))) - luminance,
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
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                const short luminance = luminanceWeightingMap[pInput[0]] + luminanceWeightingMap[pInput[1]] + 
                                        luminanceWeightingMap[pInput[2]];
                const short shadow = shadowMap[luminance];
                const short highlight = highlightMap[luminance];
                short lshpixel = (luminance + shadow + highlight);
                int* pDivLuminance = divLuminance + (luminance << 8);
                pOutput[0] = (unsigned char)((lshpixel * pDivLuminance[pInput[0]]) >> 8);
                pOutput[1] = (unsigned char)((lshpixel * pDivLuminance[pInput[1]]) >> 8);
                pOutput[2] = (unsigned char)((lshpixel * pDivLuminance[pInput[2]]) >> 8);
                pInput += Channels;
                pOutput += Channels;
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularMonochromeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                     unsigned char filterColorR, unsigned char filterColorG, unsigned char filterColorB, 
                                     int intensity) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;  

        int Channels = Stride / Width;
        if (Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        filterColorR = ClampToByte(filterColorR);
        filterColorG = ClampToByte(filterColorG);
        filterColorB = ClampToByte(filterColorB);
        intensity = max(min(intensity, 100), 0);

        unsigned short sIntensity = (unsigned short)intensity;
        int c1 = 256 * (100 - sIntensity) / 100;
        int c2 = 256 * (100 - (100 - sIntensity)) / 100;
        float fColorR = (float)filterColorR * (1.0f / 255.0f);
        float fColorG = (float)filterColorG * (1.0f / 255.0f);
        float fColorB = (float)filterColorB * (1.0f / 255.0f);
        unsigned char filterColorRMap[256] = { 0 };
        unsigned char filterColorGMap[256] = { 0 };
        unsigned char filterColorBMap[256] = { 0 };
        for (int luminance = 0; luminance < 256; luminance++) {
            float lum = (1.0f / 255.0f) * luminance;
            filterColorRMap[luminance] =
                    (unsigned char)(255.0f * (lum < 0.5f ? (2.0f * lum * fColorR) : (1.0f - 2.0f * (1.0f - lum) * (1.0f - fColorR))));
            filterColorGMap[luminance] =
                    (unsigned char)(255.0f * (lum < 0.5f ? (2.0f * lum * fColorG) : (1.0f - 2.0f * (1.0f - lum) * (1.0f - fColorG))));
            filterColorBMap[luminance] =
                    (unsigned char)(255.0f * (lum < 0.5f ? (2.0f * lum * fColorB) : (1.0f - 2.0f * (1.0f - lum) * (1.0f - fColorB))));
        }
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * Stride);
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                unsigned char lum = (unsigned char)((13926 * pInput[0] + 46884 * pInput[1] + 4725 * pInput[2]) >> 16);
                pOutput[0] = (unsigned char)((int)(pInput[0] * c1 + filterColorRMap[lum] * c2) >> 8);
                pOutput[1] = (unsigned char)((int)(pInput[1] * c1 + filterColorGMap[lum] * c2) >> 8);
                pOutput[2] = (unsigned char)((int)(pInput[2] * c1 + filterColorBMap[lum] * c2) >> 8);
                pInput += Channels;
                pOutput += Channels;
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularColorInvertFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        unsigned char invertMap[256] = { 0 };
        for (int pixel = 0; pixel < 256; pixel++) {
            invertMap[pixel] = (unsigned char)(255 - pixel);
        }
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * Stride);
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                pOutput[0] = invertMap[pInput[0]];
                pOutput[1] = invertMap[pInput[1]];
                pOutput[2] = invertMap[pInput[2]];
                pInput += Channels;
                pOutput += Channels;
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularSolidColorGenerator(unsigned char* Output, int Width, int Height, int Stride, unsigned char colorR, 
                                        unsigned char colorG, unsigned char colorB, unsigned char colorAlpha) {

        if (Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        colorR = ClampToByte(colorR);
        colorG = ClampToByte(colorG);
        colorB = ClampToByte(colorB);
        colorAlpha = ClampToByte(colorAlpha);

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

        return OC_STATUS_OK;
    }

    OC_STATUS ocularLuminanceThresholdFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, 
                                             int Stride, unsigned char threshold) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels != 1 && Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        threshold = ClampToByte(threshold);

        if (Channels == 1) {

            for (int Y = 0; Y < Height; Y++) {
                unsigned char* pOutput = Output + (Y * Stride);
                unsigned char* pInput = Input + (Y * Stride);
                for (int X = 0; X < Width; X++) {
                    pOutput[0] = step(threshold, pInput[0]);
                    pInput++;
                    pOutput++;
                }
            }
            return OC_STATUS_OK;
        }

        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * Stride);
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                unsigned char luminance = (unsigned char)((13926 * pInput[0] + 46884 * pInput[1] + 4725 * pInput[2]) >> 16);
                pOutput[2] = pOutput[1] = pOutput[0] = step(threshold, luminance);
                pInput += Channels;
                pOutput += Channels;
            }
        }
        return OC_STATUS_OK;
    }

    OC_STATUS ocularAutoContrast(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels) {

        // implementation of Local Color Correction using Non-Linear Masking published by Nathan Moroney Hewlett-Packard
        // Laboratories, Palo Alto, California.

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Channels <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        if (Channels != 1 && Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        unsigned char* Luminance = (unsigned char*)malloc(Width * Height * 2 * sizeof(unsigned char));
        unsigned char* Mask = Luminance + (Width * Height);
        if (Luminance == NULL)
            return OC_STATUS_ERR_OUTOFMEMORY;
        unsigned char LocalLut[256 * 256];
        for (int mask = 0; mask < 256; mask++) {
            unsigned char* pLocalLut = LocalLut + (mask << 8);
            for (int pix = 0; pix < 256; pix++) {
                pLocalLut[pix] = ClampToByte(255.0f * powf(pix / 255.0f, powf(2.0f, (128.0f - (255.0f - mask)) / 128.0f)));
            }
        }
        ocularGrayscaleFilter(Input, Luminance, Width, Height, Width * Channels);
        int Radius = (max(Width, Height) / 512) + 1;
        ocularBoxBlurFilter(Luminance, Mask, Width, Height, Width, Radius);
        for (int y = 0; y < Height; y++) {
            unsigned char* pOutput = Output + (y * Width * Channels);
            unsigned char* pInput = Input + (y * Width * Channels);
            unsigned char* pMask = Mask + (y * Width);
            unsigned char* pLuminance = Luminance + (y * Width);
            for (int x = 0; x < Width; x++) {
                unsigned char* pLocalLut = LocalLut + (pMask[x] << 8);
                for (int c = 0; c < Channels; c++) {
                    pOutput[c] = ClampToByte(
                            (pLocalLut[pLuminance[x]] * (pInput[c] + pLuminance[x]) / (pLuminance[x] + 1) + pInput[c] - pLuminance[x]) >> 1);
                }
                pOutput += Channels;
                pInput += Channels;
            }
        }
        free(Luminance);

        return OC_STATUS_OK;
    }

    OC_STATUS ocularAutoGammaCorrection(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride) {

        if ((Input == NULL) || (Output == NULL))
            return OC_STATUS_ERR_NULLREFERENCE;
        if ((Width <= 0) || (Height <= 0) || (Stride <= 0))
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if ((Channels != 1) && (Channels != 3) && (Channels != 4))
            return OC_STATUS_ERR_NOTSUPPORTED;

        unsigned char AvgR, AvgG, AvgB, AvgA;
        ocularAverageColor(Input, Width, Height, Stride, &AvgR, &AvgG, &AvgB, &AvgA);
        if (Channels == 1) {
            float Gamma = -0.3 / (log10(AvgB));
            unsigned char Table[256] = { 0 };
            for (int y = 0; y < 256; y++) {
                Table[y] = ClampToByte((int)(pow(y / 255.0f, Gamma) * 255.0f));
            }
            applyCurve(Input, Output, Width, Height, Stride, Channels, Table, Table, Table);
        } else {
            float GammaR = -0.3 / (log10(AvgR / 256.0f));
            float GammaG = -0.3 / (log10(AvgG / 256.0f));
            float GammaB = -0.3 / (log10(AvgB / 256.0f));

            unsigned char TableR[256], TableG[256], TableB[256] = { 0 };
            for (int y = 0; y < 256; y++) {
                TableR[y] = ClampToByte((int)(pow(y / 255.0f, GammaR) * 255.0f));
                TableG[y] = ClampToByte((int)(pow(y / 255.0f, GammaG) * 255.0f));
                TableB[y] = ClampToByte((int)(pow(y / 255.0f, GammaB) * 255.0f));
            }
            applyCurve(Input, Output, Width, Height, Channels, Stride, TableR, TableG, TableB);
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularAutoWhiteBalance(unsigned char* input, unsigned char* output, int width, int height, int stride,
                                int colorCoeff, float cutLimit, float contrast, bool* hasColorCast) {

        if (input == NULL || output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (width <= 0 || height <= 0 || stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int channels = stride / width;
        if (channels != 3 && channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        colorCoeff = clamp(colorCoeff, 0, 127);
        cutLimit = clamp(cutLimit, 0.0f, 1.0f);
        contrast = clamp(contrast, 0.0f, 1.0f);

        int numberOfPixels = height * width;
        unsigned int histogramYcbcr[768] = { 0 };
        unsigned int* histogramY = &histogramYcbcr[0];
        unsigned int* histogramCb = &histogramYcbcr[256];
        unsigned int* histogramCr = &histogramYcbcr[512];
        unsigned int histogramRGB[768] = { 0 };
        unsigned int* histogramR = &histogramRGB[0];
        unsigned int* histogramG = &histogramRGB[256];
        unsigned int* histogramB = &histogramRGB[512];
        unsigned char Y = 0;
        unsigned char Cb = 0;
        unsigned char Cr = 0;
        for (int y = 0; y < height; y++) {
            const unsigned char* scanIn = input + y * stride;
            for (int x = 0; x < width; x++) {
                const unsigned char R = scanIn[0];
                const unsigned char G = scanIn[1];
                const unsigned char B = scanIn[2];
                histogramR[R]++;
                histogramG[G]++;
                histogramB[B]++;
                rgb2ycbcr(R, G, B, &Y, &Cb, &Cr);
                histogramY[Y]++;
                histogramCb[Cb]++;
                histogramCr[Cr]++;
                scanIn += channels;
            }
        }
        bool colorCast = isColorCast(histogramCb, histogramCr, numberOfPixels, colorCoeff);
        if (!colorCast) {
            memcpy(output, input, numberOfPixels * channels * sizeof(*input));
            *hasColorCast = true;
        }
        unsigned char mapRGB[256 * 3] = { 0 };
        unsigned char* mapR = &mapRGB[0];
        unsigned char* mapG = &mapRGB[256];
        unsigned char* mapB = &mapRGB[256 + 256];
        autoLevel(histogramR, mapR, numberOfPixels, cutLimit, contrast);
        autoLevel(histogramG, mapG, numberOfPixels, cutLimit, contrast);
        autoLevel(histogramB, mapB, numberOfPixels, cutLimit, contrast);
        for (int y = 0; y < height; y++) {
            unsigned char* scanIn = input + y * stride;
            unsigned char* scanOut = output + y * stride;
            for (int x = 0; x < width; x++) {
                scanOut[0] = mapR[scanIn[0]];
                scanOut[1] = mapG[scanIn[1]];
                scanOut[2] = mapB[scanIn[2]];
                scanIn += channels;
                scanOut += channels;
            }
        }
        return OC_STATUS_OK;
    }

    OC_STATUS ocularWhiteBalanceFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                       float temperature, float tint) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        temperature = clamp(temperature, 2000.0f, 8000.0f); // These should be good working ranges for photography
        tint = clamp(tint, -200.0f, 200.0f);

        float Temperature = temperature;
        Temperature = Temperature < 5000 ? (float)(0.0004 * (Temperature - 5000.0)) : 
                                           (float)(0.00006 * (Temperature - 5000.0));

        float Tint = tint;
        Tint = (float)(Tint / 100.0f);

        short YPrime = 0;
        short I = 0;
        short Q = 0;

        float warmFilterR = 0.93f;
        float warmFilterG = 0.54f;
        float warmFilterB = 0;
        int plusTint = (int)(Tint * 255.0f * 0.5226f * 0.1f);

        short QTint[256] = { 0 };
        unsigned char processedMap[256 * 3] = { 0 };
        unsigned char* processedRMap = &processedMap[0];
        unsigned char* processedGMap = &processedMap[256];
        unsigned char* processedBMap = &processedMap[512];
        for (int pixel = 0; pixel < 256; pixel++) {
            float fpixel = pixel * (1.0f / 255.0f);
            QTint[pixel] = (short)clamp((float)(pixel - 127 + plusTint), -127.0f, 127.0f);
            float processedR = (fpixel < 0.5f ? (2.0f * fpixel * warmFilterR) : (1.0f - 2.0f * (1.0f - fpixel) * (1.0f - warmFilterR)));
            float processedG = (fpixel < 0.5f ? (2.0f * fpixel * warmFilterG) : (1.0f - 2.0f * (1.0f - fpixel) * (1.0f - warmFilterG)));
            float processedB = (fpixel < 0.5f ? (2.0f * fpixel * warmFilterB) : (1.0f - 2.0f * (1.0f - fpixel) * (1.0f - warmFilterB)));
            processedRMap[pixel] = ClampToByte(255.0f * mix(fpixel, processedR, Temperature));
            processedGMap[pixel] = ClampToByte(255.0f * mix(fpixel, processedG, Temperature));
            processedBMap[pixel] = ClampToByte(255.0f * mix(fpixel, processedB, Temperature));
        }
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * Stride);
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                const unsigned char R = pInput[0];
                const unsigned char G = pInput[1];
                const unsigned char B = pInput[2];

                YPrime = (short)(((int)(0.299f * 65536) * R + (int)(0.587f * 65536) * G + (int)(0.114f * 65536) * B) >> 16);
                I = (short)(((int)(0.595f * 65536) * R - (int)(0.274453f * 65536) * G - (int)(0.321263f * 65536) * B) >> 16);
                Q = (short)(((int)(0.211456f * 65536) * R - (int)(0.522591f * 65536) * G + (int)(0.311135f * 65536) * B) >> 16);
                // adjusting tint
                Q = QTint[Q + 127];
                // adjusting temperature
                pOutput[0] = processedRMap[ClampToByte(YPrime + (((int)(0.9563 * 65536) * I + (int)(0.6210 * 65536) * Q) >> 16))];
                pOutput[1] = processedGMap[ClampToByte(YPrime - (((int)(0.2721 * 65536) * I + (int)(0.6474 * 65536) * Q) >> 16))];
                pOutput[2] = processedBMap[ClampToByte(YPrime + (((int)(1.7046 * 65536) * Q - (int)(1.1070 * 65536) * I) >> 16))];

                pInput += Channels;
                pOutput += Channels;
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularVibranceFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float vibrance) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        vibrance = clamp(vibrance, -1.0f, 1.0f);

        int iVibrance = (int)(-(vibrance * 256));
        unsigned char mx = 0;
        int amt = 0;
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * Stride);
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                const unsigned char r = pInput[0];
                const unsigned char g = pInput[1];
                const unsigned char b = pInput[2];
                mx = max3(r, g, b);
                amt = (3 * mx - (r + g + b)) * iVibrance;
                pOutput[0] = ClampToByte((r * (255 * 256 - amt) + mx * amt) >> 16);
                pOutput[1] = ClampToByte((g * (255 * 256 - amt) + mx * amt) >> 16);
                pOutput[2] = ClampToByte((b * (255 * 256 - amt) + mx * amt) >> 16);

                pInput += Channels;
                pOutput += Channels;
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularSkinToneFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float skinToneAdjust,
                              float skinHue, float skinHueThreshold, float maxHueShift, float maxSaturationShift, int upperSkinToneColor) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        skinToneAdjust = clamp(skinToneAdjust, -1.0f, 1.0f);
        skinHue = clamp(skinHue, 0.0f, 1.0f);
        skinHueThreshold = clamp(skinHueThreshold, 0.0f, 1.0f);
        maxHueShift = clamp(maxHueShift, 0.0f, 1.0f);
        maxSaturationShift = clamp(maxSaturationShift, 0.0f, 1.0f);
        // 0 = pink/green, 1 = pink/orange
        upperSkinToneColor = clamp(upperSkinToneColor, 0, 1);

        int maxSatShiftAdjust = (int)(maxSaturationShift * 255.0f * skinToneAdjust);
        float maxHueShiftAdjust = maxHueShift * skinToneAdjust;
        unsigned char hueMap[256] = { 0 };
        int satMap[256] = { 0 };
        for (unsigned int H = 0; H < 256; H++) {
            satMap[H] = 0;
            float hue = H * (1.0f / 255.0f);
            // check how far from skin hue
            float dist = hue - skinHue;
            if (dist > 0.5f)
                dist -= 1.0f;
            if (dist < -0.5f)
                dist += 1.0f;
            dist = (float)(fabsf(dist) * (1.0f / 0.5f)); // normalized to [0,1]
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
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                unsigned char R = pInput[0];
                unsigned char G = pInput[1];
                unsigned char B = pInput[2];
                // Convert color to HSV, extract hue
                rgb2hsv(R, G, B, &H, &S, &V);
                // final color
                _S = (unsigned char)(S + satMap[H]);
                hsv2rgb(hueMap[H], _S, V, &pOutput[0], &pOutput[1], &pOutput[2]);

                pInput += Channels;
                pOutput += Channels;
            }
        }

        return OC_STATUS_OK;
    }

    static void bilateralHorizontal(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, float* range_table_f,
                                    float inv_alpha_f, float* left_Color_Buffer, float* left_Factor_Buffer, float* right_Color_Buffer,
                                    float* right_Factor_Buffer) {

        // Left and right pass
        int Stride = Width * Channels;
        const unsigned char* src_left_color = Input;
        float* left_Color = left_Color_Buffer;
        float* left_Factor = left_Factor_Buffer;

        int last_index = Stride * Height - 1;
        const unsigned char* src_right_color = Input + last_index;
        float* right_Color = right_Color_Buffer + last_index;
        float* right_Factor = right_Factor_Buffer + Width * Height - 1;

        for (int y = 0; y < Height; y++) {
            const unsigned char* src_left_prev = Input;
            const float* left_prev_factor = left_Factor;
            const float* left_prev_color = left_Color;

            const unsigned char* src_right_prev = src_right_color;
            const float* right_prev_factor = right_Factor;
            const float* right_prev_color = right_Color;

            // process 1st pixel separately since it has no previous
            {
                // if x = 0
                *left_Factor++ = 1.f;
                *right_Factor-- = 1.f;
                for (int c = 0; c < Channels; c++) {
                    *left_Color++ = *src_left_color++;
                    *right_Color-- = *src_right_color--;
                }
            }
            // handle other pixels
            for (int x = 1; x < Width; x++) {
                // determine difference in pixel color between current and previous
                // calculation is different depending on number of channels
                int left_diff = getDiffFactor(src_left_color, src_left_prev, Channels);
                src_left_prev = src_left_color;

                int right_diff = getDiffFactor(src_right_color, src_right_color - Channels, Channels);
                src_right_prev = src_right_color;

                float left_alpha_f = range_table_f[left_diff];
                float right_alpha_f = range_table_f[right_diff];
                *left_Factor++ = inv_alpha_f + left_alpha_f * (*left_prev_factor++);
                *right_Factor-- = inv_alpha_f + right_alpha_f * (*right_prev_factor--);

                for (int c = 0; c < Channels; c++) {
                    *left_Color++ = (inv_alpha_f * (*src_left_color++) + left_alpha_f * (*left_prev_color++));
                    *right_Color-- = (inv_alpha_f * (*src_right_color--) + right_alpha_f * (*right_prev_color--));
                }
            }
        }
        // vertical pass will be applied on top on horizontal pass, while using pixel differences from original image
        // result color stored in 'leftcolor' and vertical pass will use it as source color
        {
            unsigned char* dst_color = Output; // use as temporary buffer
            const float* leftcolor = left_Color_Buffer;
            const float* leftfactor = left_Factor_Buffer;
            const float* rightcolor = right_Color_Buffer;
            const float* rightfactor = right_Factor_Buffer;

            int width_height = Width * Height;
            for (int i = 0; i < width_height; i++) {
                // average color divided by average factor
                float factor = 1.f / ((*leftfactor++) + (*rightfactor++));
                for (int c = 0; c < Channels; c++) {

                    *dst_color++ = (factor * ((*leftcolor++) + (*rightcolor++)));
                }
            }
        }
    }

    static void bilateralVertical(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, float* range_table_f, float inv_alpha_f,
                                  float* down_Color_Buffer, float* down_Factor_Buffer, float* up_Color_Buffer, float* up_Factor_Buffer) {

        // Down pass and Up pass
        int Stride = Width * Channels;
        const unsigned char* src_color_first_hor = Output; // result of horizontal pass filter
        const unsigned char* src_down_color = Input;
        float* down_color = down_Color_Buffer;
        float* down_factor = down_Factor_Buffer;

        const unsigned char* src_down_prev = src_down_color;
        const float* down_prev_color = down_color;
        const float* down_prev_factor = down_factor;

        int last_index = Stride * Height - 1;
        const unsigned char* src_up_color = Input + last_index;
        const unsigned char* src_color_last_hor = Output + last_index; // result of horizontal pass filter
        float* up_color = up_Color_Buffer + last_index;
        float* up_factor = up_Factor_Buffer + (Width * Height - 1);

        const float* up_prev_color = up_color;
        const float* up_prev_factor = up_factor;

        // 1st line done separately because no previous line
        {
            // if y=0
            for (int x = 0; x < Width; x++) {
                *down_factor++ = 1.f;
                *up_factor-- = 1.f;
                for (int c = 0; c < Channels; c++) {
                    *down_color++ = *src_color_first_hor++;
                    *up_color-- = *src_color_last_hor--;
                }
                src_down_color += Channels;
                src_up_color -= Channels;
            }
        }
        // handle other lines
        for (int y = 1; y < Height; y++) {
            for (int x = 0; x < Width; x++) {
                // determine difference in pixel color between current and previous
                // calculation is different depending on number of channels
                int down_diff = getDiffFactor(src_down_color, src_down_prev, Channels);
                src_down_prev += Channels;
                src_down_color += Channels;
                src_up_color -= Channels;
                int up_diff = getDiffFactor(src_up_color, src_up_color + Stride, Channels);
                float down_alpha_f = range_table_f[down_diff];
                float up_alpha_f = range_table_f[up_diff];

                *down_factor++ = inv_alpha_f + down_alpha_f * (*down_prev_factor++);
                *up_factor-- = inv_alpha_f + up_alpha_f * (*up_prev_factor--);

                for (int c = 0; c < Channels; c++) {
                    *down_color++ = inv_alpha_f * (*src_color_first_hor++) + down_alpha_f * (*down_prev_color++);
                    *up_color-- = inv_alpha_f * (*src_color_last_hor--) + up_alpha_f * (*up_prev_color--);
                }
            }
        }

        // average result of vertical pass is written to output buffer
        {
            unsigned char* dst_color = Output;
            const float* downcolor = down_Color_Buffer;
            const float* downfactor = down_Factor_Buffer;
            const float* upcolor = up_Color_Buffer;
            const float* upfactor = up_Factor_Buffer;

            int width_height = Width * Height;
            for (int i = 0; i < width_height; i++) {
                // average color divided by average factor
                float factor = 1.f / ((*upfactor++) + (*downfactor++));
                for (int c = 0; c < Channels; c++) {
                    *dst_color++ = (factor * ((*upcolor++) + (*downcolor++)));
                }
            }
        }
    }

    OC_STATUS ocularBilateralFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                    float sigmaSpatial, float sigmaRange) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels != 1 && Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        sigmaSpatial = clamp(sigmaSpatial, 0.0f, 1.0f);
        sigmaRange = clamp(sigmaRange, 0.0f, 1.0f);

        int reserveWidth = Width;
        int reserveHeight = Height;

        int reservePixels = reserveWidth * reserveHeight;
        int numberOfPixels = reservePixels * Channels;

        float* leftColorBuffer = (float*)calloc(sizeof(float) * numberOfPixels, 1);
        float* leftFactorBuffer = (float*)calloc(sizeof(float) * reservePixels, 1);
        float* rightColorBuffer = (float*)calloc(sizeof(float) * numberOfPixels, 1);
        float* rightFactorBuffer = (float*)calloc(sizeof(float) * reservePixels, 1);

        if (leftColorBuffer == NULL || leftFactorBuffer == NULL || rightColorBuffer == NULL || rightFactorBuffer == NULL) {
            if (leftColorBuffer)
                free(leftColorBuffer);
            if (leftFactorBuffer)
                free(leftFactorBuffer);
            if (rightColorBuffer)
                free(rightColorBuffer);
            if (rightFactorBuffer)
                free(rightFactorBuffer);

            return OC_STATUS_ERR_OUTOFMEMORY;
        }
        float* downColorBuffer = leftColorBuffer;
        float* downFactorBuffer = leftFactorBuffer;
        float* upColorBuffer = rightColorBuffer;
        float* upFactorBuffer = rightFactorBuffer;
        // compute a lookup table
        float alpha_f = (float)(exp(-sqrt(2.0) / (sigmaSpatial * 255)));
        float inv_alpha_f = 1.f - alpha_f;

        float range_table_f[255 + 1];
        float inv_sigma_range = 1.0f / (sigmaRange * 255);

        float ii = 0.f;
        for (int i = 0; i <= 255; i++, ii -= 1.f) {
            range_table_f[i] = alpha_f * exp(ii * inv_sigma_range);
        }
        bilateralHorizontal(Input, Output, Width, Height, Channels, range_table_f, inv_alpha_f, leftColorBuffer, leftFactorBuffer,
                            rightColorBuffer, rightFactorBuffer);
        bilateralVertical(Input, Output, Width, Height, Channels, range_table_f, inv_alpha_f, downColorBuffer, downFactorBuffer,
                          upColorBuffer, upFactorBuffer);

        if (leftColorBuffer) {
            free(leftColorBuffer);
            leftColorBuffer = NULL;
        }

        if (leftFactorBuffer) {
            free(leftFactorBuffer);
            leftFactorBuffer = NULL;
        }

        if (rightColorBuffer) {
            free(rightColorBuffer);
            rightColorBuffer = NULL;
        }

        if (rightFactorBuffer) {
            free(rightFactorBuffer);
            rightFactorBuffer = NULL;
        }

        return OC_STATUS_OK;
    }

    static void gaussianHorizontal(unsigned char* bufferPerLine, const unsigned char* lpRowInitial, unsigned char* lpColumn, int width,
                                   int height, int Channels, int Nwidth, float a0a1, float a2a3, float b1b2, float cprev, float cnext) {
        int HeightStep = Channels * height;
        int WidthSubOne = width - 1;
        if (Channels == 3) {
            float prevOut[3];
            prevOut[0] = (lpRowInitial[0] * cprev);
            prevOut[1] = (lpRowInitial[1] * cprev);
            prevOut[2] = (lpRowInitial[2] * cprev);
            for (int x = 0; x < width; ++x) {
                prevOut[0] = ((lpRowInitial[0] * (a0a1)) - (prevOut[0] * (b1b2)));
                prevOut[1] = ((lpRowInitial[1] * (a0a1)) - (prevOut[1] * (b1b2)));
                prevOut[2] = ((lpRowInitial[2] * (a0a1)) - (prevOut[2] * (b1b2)));
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
                prevOut = ((lpRowInitial[0] * (a0a1)) - (prevOut * (b1b2)));
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

    static void gaussianVertical(unsigned char* bufferPerLine, const unsigned char* lpRowInitial, unsigned char* lpColInitial, int height,
                                 int width, int Channels, float a0a1, float a2a3, float b1b2, float cprev, float cnext) {

        int WidthStep = Channels * width;
        int HeightSubOne = height - 1;
        if (Channels == 3) {
            float prevOut[3];
            prevOut[0] = (lpRowInitial[0] * cprev);
            prevOut[1] = (lpRowInitial[1] * cprev);
            prevOut[2] = (lpRowInitial[2] * cprev);

            for (int y = 0; y < height; y++) {
                prevOut[0] = ((lpRowInitial[0] * a0a1) - (prevOut[0] * b1b2));
                prevOut[1] = ((lpRowInitial[1] * a0a1) - (prevOut[1] * b1b2));
                prevOut[2] = ((lpRowInitial[2] * a0a1) - (prevOut[2] * b1b2));
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
                prevOut[0] = ((lpRowInitial[0] * a0a1) - (prevOut[0] * b1b2));
                prevOut[1] = ((lpRowInitial[1] * a0a1) - (prevOut[1] * b1b2));
                prevOut[2] = ((lpRowInitial[2] * a0a1) - (prevOut[2] * b1b2));
                prevOut[3] = ((lpRowInitial[3] * a0a1) - (prevOut[3] * b1b2));
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
            prevOut = (lpRowInitial[0] * cprev);
            for (int y = 0; y < height; y++) {
                prevOut = ((lpRowInitial[0] * a0a1) - (prevOut * b1b2));
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

    OC_STATUS ocularGaussianBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                       float GaussianSigma) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels != 1 && Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        GaussianSigma = max(GaussianSigma, 0.0f);

        float a0, a1, a2, a3, b1, b2, cprev, cnext;

        CalGaussianCoeff(GaussianSigma, &a0, &a1, &a2, &a3, &b1, &b2, &cprev, &cnext);

        float a0a1 = (a0 + a1);
        float a2a3 = (a2 + a3);
        float b1b2 = (b1 + b2);

        int bufferSizePerThread = (Width > Height ? Width : Height) * Channels;
        unsigned char* bufferPerLine = (unsigned char*)malloc((size_t)bufferSizePerThread);
        unsigned char* tempData = (unsigned char*)malloc((size_t)Height * Stride);
        if (bufferPerLine == NULL || tempData == NULL) {
            if (tempData) {
                free(tempData);
            }
            if (bufferPerLine) {
                free(bufferPerLine);
            }
            return OC_STATUS_ERR_OUTOFMEMORY;
        }
        for (int y = 0; y < Height; ++y) {
            unsigned char* lpRowInitial = Input + Stride * y;
            unsigned char* lpColInitial = tempData + y * Channels;
            gaussianHorizontal(bufferPerLine, lpRowInitial, lpColInitial, Width, Height, Channels, Width, a0a1, a2a3, b1b2, cprev, cnext);
        }
        int HeightStep = Height * Channels;
        for (int x = 0; x < Width; ++x) {
            unsigned char* lpColInitial = Output + x * Channels;
            unsigned char* lpRowInitial = tempData + HeightStep * x;
            gaussianVertical(bufferPerLine, lpRowInitial, lpColInitial, Height, Width, Channels, a0a1, a2a3, b1b2, cprev, cnext);
        }

        free(bufferPerLine);
        free(tempData);

        return OC_STATUS_OK;
    }

    OC_STATUS ocularUnsharpMaskFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float GaussianSigma,
                                      float intensity, float threshold) {
        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels != 1 && Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter parameters are within valid ranges
        GaussianSigma = clamp(GaussianSigma, 0.1f, 200.0f);
        intensity = clamp(intensity, 0.0f, 4.0f);
        threshold = clamp(threshold, 0.0f, 100.0f);

        // Convert threshold percentage to pixel difference value (0-255)
        float boostedIntensity = intensity;
        float thresholdValue = (threshold / 100.0f) * 255.0f;

        // Allocate blur buffer for the entire image
        unsigned char* Blur = (unsigned char*)malloc(Width * Height * Channels);
        if (Blur == NULL) {
            return OC_STATUS_ERR_OUTOFMEMORY;
        }

        // Copy input to blur buffer
        memcpy(Blur, Input, Width * Height * Channels);

        // Apply Gaussian blur
        ocularGaussianBlurFilter(Blur, Blur, Width, Height, Stride, GaussianSigma);

        // Apply unsharp mask with threshold and smooth blending
        for (int Y = 0; Y < Height; Y++) {
            for (int X = 0; X < Width; X++) {
                // Calculate luminance delta for edge detection
                float lumDelta = 0.0f;
                if (Channels >= 3) {
                    // Calculate luminance for original and blurred using BT.709 coefficients
                    int idx = Y * Stride + X * Channels;
                    float lumOrig = 0.2126f * Input[idx] + 0.7152f * Input[idx + 1] + 0.0722f * Input[idx + 2];
                    float lumBlur = 0.2126f * Blur[idx] + 0.7152f * Blur[idx + 1] + 0.0722f * Blur[idx + 2];
                    lumDelta = fabsf(lumOrig - lumBlur);
                }

                for (int c = 0; c < Channels; c++) {
                    int idx = Y * Stride + X * Channels + c;

                    // Skip alpha channel if present
                    if (Channels == 4 && c == 3) {
                        Output[idx] = Input[idx];
                        continue;
                    }

                    // Calculate difference between original and blurred
                    float diff = (float)(Input[idx] - Blur[idx]);

                    // For grayscale images, use direct difference as luminance delta
                    if (Channels == 1) {
                        lumDelta = fabsf(diff);
                    }

                    // Apply threshold with smooth transition based on luminance delta
                    if (lumDelta < thresholdValue) {
                        float thresholdFactor = lumDelta / thresholdValue;
                        diff *= thresholdFactor * thresholdFactor; // Smooth quadratic falloff
                    }

                    // Calculate sharpened value
                    int sharpened = Input[idx] + (int)(diff * boostedIntensity);
                    sharpened = clamp(sharpened, 0, 255);

                    // Calculate blend factor based on luminance delta
                    float blendFactor = lumDelta / 255.0f; // Normalize difference to 0-1 range
                    blendFactor = clamp(blendFactor * (intensity), 0.0f, 1.0f);

                    // Smooth blend between original and sharpened
                    Output[idx] = (unsigned char)(Input[idx] * (1.0f - blendFactor) + sharpened * blendFactor);
                }
            }
        }

        free(Blur);
        return OC_STATUS_OK;
    }

    static inline void boxfilterRow(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, int Radius) {
        int iRadius = Radius + 1;
        int iScale = (int)((256.0f * 256.0f) / (2 * Radius + 1));
        int iWidthStep = Width * Channels;
        int iWidthStepDec = (Width - 1) * Channels;
        int iRadChannels = Radius * Channels;
        int iRadChannelsPlus = (iRadChannels + Channels);
        switch (Channels) {
        case 1: {
            for (int y = 0; y < Height; y++) {
                //  Process left edge
                int iY = y * iWidthStep;
                int sum = Input[iY] * Radius;
                for (int x = 0; x < iRadius; x++) {
                    int p = (y * Width + x) * Channels;
                    sum += Input[p];
                }
                Output[iY] = (unsigned char)((sum * iScale) >> 16);
                for (int x = 1; x < iRadius; x++) {
                    int pLeft = iY + x * Channels;
                    int p0 = pLeft + iRadChannels;

                    sum += Input[p0];
                    sum -= Input[iY];
                    Output[pLeft] = (unsigned char)((sum * iScale) >> 16);
                }

                // core zone
                for (int x = iRadius; x < Width - Radius; x++) {
                    int pKernal = iY + x * Channels;

                    int i0 = pKernal + iRadChannels;
                    int i1 = pKernal - iRadChannelsPlus;

                    sum += Input[i0];
                    sum -= Input[i1];

                    Output[pKernal] = (unsigned char)((sum * iScale) >> 16);
                }

                // Process right edge
                for (int x = Width - Radius; x < Width; x++) {
                    int iRight = iY + x * Channels;
                    int i0 = iY + iWidthStepDec;
                    int i1 = iRight - iRadChannelsPlus;

                    sum += Input[i0];
                    sum -= Input[i1];
                    Output[iRight] = (unsigned char)((sum * iScale) >> 16);
                }
            }
            break;
        }
        case 3: {
            for (int y = 0; y < Height; y++) {
                //  Process left edge

                int iY = y * iWidthStep;
                int sumR = Input[iY] * Radius;
                int sumG = Input[iY + 1] * Radius;
                int sumB = Input[iY + 2] * Radius;
                for (int x = 0; x < iRadius; x++) {
                    int i = iY + x * Channels;
                    sumR += Input[i];
                    sumG += Input[i + 1];
                    sumB += Input[i + 2];
                }
                Output[iY] = (unsigned char)((sumR * iScale) >> 16);
                Output[iY + 1] = (unsigned char)((sumG * iScale) >> 16);
                Output[iY + 2] = (unsigned char)((sumB * iScale) >> 16);
                for (int x = 1; x < iRadius; x++) {
                    int iLeft = iY + x * Channels;
                    int i0 = iLeft + iRadChannels;

                    sumR += Input[i0];
                    sumR -= Input[iY];
                    sumG += Input[i0 + 1];
                    sumG -= Input[iY + 1];
                    sumB += Input[i0 + 2];
                    sumB -= Input[iY + 2];
                    Output[iLeft] = (unsigned char)((sumR * iScale) >> 16);
                    Output[iLeft + 1] = (unsigned char)((sumG * iScale) >> 16);
                    Output[iLeft + 2] = (unsigned char)((sumB * iScale) >> 16);
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
                    Output[iKernal] = (unsigned char)((sumR * iScale) >> 16);
                    Output[iKernal + 1] = (unsigned char)((sumG * iScale) >> 16);
                    Output[iKernal + 2] = (unsigned char)((sumB * iScale) >> 16);
                }

                // Process right edge
                for (int x = Width - Radius; x < Width; x++) {
                    int iRight = iY + x * Channels;
                    int i0 = iY + iWidthStepDec;
                    int i1 = iRight - iRadChannelsPlus;

                    sumR += Input[i0];
                    sumR -= Input[i1];

                    sumG += Input[i0 + 1];
                    sumG -= Input[i1 + 1];

                    sumB += Input[i0 + 2];
                    sumB -= Input[i1 + 2];
                    Output[iRight] = (unsigned char)((sumR * iScale) >> 16);
                    Output[iRight + 1] = (unsigned char)((sumG * iScale) >> 16);
                    Output[iRight + 2] = (unsigned char)((sumB * iScale) >> 16);
                }
            }
            break;
        }
        case 4: {
            for (int y = 0; y < Height; y++) {
                //  Process left edge
                int iY = y * iWidthStep;
                int sumR = Input[iY] * Radius;
                int sumG = Input[iY + 1] * Radius;
                int sumB = Input[iY + 2] * Radius;
                for (int x = 0; x < iRadius; x++) {
                    int i = iY + x * Channels;
                    sumR += Input[i];
                    sumG += Input[i + 1];
                    sumB += Input[i + 2];
                }
                Output[iY] = (unsigned char)((sumR * iScale) >> 16);
                Output[iY + 1] = (unsigned char)((sumG * iScale) >> 16);
                Output[iY + 2] = (unsigned char)((sumB * iScale) >> 16);
                Output[iY + 3] = Input[iY + 3];
                for (int x = 1; x < iRadius; x++) {
                    int iLeft = iY + x * Channels;
                    int i0 = iLeft + iRadChannels;
                    sumR += Input[i0];
                    sumR -= Input[iLeft];
                    sumG += Input[i0 + 1];
                    sumG -= Input[iLeft + 1];
                    sumB += Input[i0 + 2];
                    sumB -= Input[iLeft + 2];
                    Output[iLeft] = (unsigned char)((sumR * iScale) >> 16);
                    Output[iLeft + 1] = (unsigned char)((sumG * iScale) >> 16);
                    Output[iLeft + 2] = (unsigned char)((sumB * iScale) >> 16);
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
                    Output[iKernal] = (unsigned char)((sumR * iScale) >> 16);
                    Output[iKernal + 1] = (unsigned char)((sumG * iScale) >> 16);
                    Output[iKernal + 2] = (unsigned char)((sumB * iScale) >> 16);
                    Output[iKernal + 3] = Input[iKernal + 3];
                }

                // Process right edge
                for (int x = Width - Radius; x < Width; x++) {
                    int iRight = iY + x * Channels;
                    int i0 = iY + iWidthStepDec;
                    int i1 = iRight - iRadChannelsPlus;

                    sumR += Input[i0];
                    sumR -= Input[i1];

                    sumG += Input[i0 + 1];
                    sumG -= Input[i1 + 1];

                    sumB += Input[i0 + 2];
                    sumB -= Input[i1 + 2];
                    Output[iRight] = (unsigned char)((sumR * iScale) >> 16);
                    Output[iRight + 1] = (unsigned char)((sumG * iScale) >> 16);
                    Output[iRight + 2] = (unsigned char)((sumB * iScale) >> 16);
                    Output[iRight + 3] = Input[iRight + 3];
                }
            }
            break;
        }
        default: break;
        }
    }

    static inline void boxfilterCol(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, int Radius) {
        int iScale = (int)((256.0f * 256.0f) / (2 * Radius + 1));
        int iWidthStep = Width * Channels;
        int iWidthStepDec = (Height - 1) * iWidthStep;
        int iRadWidthStep = Radius * iWidthStep;
        int iRadWidthStepDec = (iRadWidthStep + iWidthStep);
        int iHeightRadius = Height - Radius;
        int iRadius = Radius + 1;
        switch (Channels) {
        case 1: {
            for (int x = 0; x < Width; x++) {
                //  Process left edge
                int iX = x * Channels;
                int sum = Input[iX] * Radius;
                for (int y = 0; y < iRadius; y++) {
                    int i = (y * Width + x) * Channels;
                    sum += Input[i];
                }
                Output[x] = (unsigned char)((sum * iScale) >> 16);

                for (int y = 1; y < iRadius; y++) {
                    int i = iX + y * iWidthStep;

                    int i0 = i + iRadWidthStep;
                    int i1 = x * Channels;

                    sum += Input[i0];
                    sum -= Input[i1];
                    Output[i] = (unsigned char)((sum * iScale) >> 16);
                }

                // core zone
                for (int y = iRadius; y < iHeightRadius; y++) {

                    int iKernal = iX + y * iWidthStep;
                    int i0 = iKernal + iRadWidthStep;
                    int i1 = iKernal - iRadWidthStepDec;

                    sum += Input[i0];
                    sum -= Input[i1];
                    Output[iKernal] = (unsigned char)((sum * iScale) >> 16);
                }

                // Process right edge
                for (int y = iHeightRadius; y < Height; y++) {
                    int iRight = iX + y * iWidthStep;

                    int i0 = iWidthStepDec + x * Channels;
                    int i1 = iRight - iRadWidthStepDec;

                    sum += Input[i0];
                    sum -= Input[i1];
                    Output[iRight] = (unsigned char)((sum * iScale) >> 16);
                }
            }

            break;
        }
        case 3: {
            for (int x = 0; x < Width; x++) {
                //  Process left edge
                int iX = x * Channels;
                int sumR = Input[iX] * Radius;
                int sumG = Input[iX + 1] * Radius;
                int sumB = Input[iX + 2] * Radius;
                for (int y = 0; y < iRadius; y++) {
                    int i = iX + y * iWidthStep;
                    sumR += Input[i];
                    sumG += Input[i + 1];
                    sumB += Input[i + 2];
                }
                Output[iX] = (unsigned char)((sumR * iScale) >> 16);
                Output[iX + 1] = (unsigned char)((sumG * iScale) >> 16);
                Output[iX + 2] = (unsigned char)((sumB * iScale) >> 16);

                for (int y = 1; y < iRadius; y++) {
                    int i = iX + y * iWidthStep;
                    int i0 = i + iRadWidthStep;

                    sumR += Input[i0];
                    sumR -= Input[iX];
                    sumG += Input[i0 + 1];
                    sumG -= Input[iX + 1];
                    sumB += Input[i0 + 2];
                    sumB -= Input[iX + 2];
                    Output[i] = (unsigned char)((sumR * iScale) >> 16);
                    Output[i + 1] = (unsigned char)((sumG * iScale) >> 16);
                    Output[i + 2] = (unsigned char)((sumB * iScale) >> 16);
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
                    Output[iKernal] = (unsigned char)((sumR * iScale) >> 16);
                    Output[iKernal + 1] = (unsigned char)((sumG * iScale) >> 16);
                    Output[iKernal + 2] = (unsigned char)((sumB * iScale) >> 16);
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
                    Output[iRight] = (unsigned char)((sumR * iScale) >> 16);
                    Output[iRight + 1] = (unsigned char)((sumG * iScale) >> 16);
                    Output[iRight + 2] = (unsigned char)((sumB * iScale) >> 16);
                }
            }

            break;
        }
        case 4: {
            for (int x = 0; x < Width; x++) {
                //  Process left edge
                int iX = x * Channels;
                int sumR = Input[iX] * Radius;
                int sumG = Input[iX + 1] * Radius;
                int sumB = Input[iX + 2] * Radius;
                for (int y = 0; y < iRadius; y++) {
                    int i = iX + y * iWidthStep;
                    sumR += Input[i];
                    sumG += Input[i + 1];
                    sumB += Input[i + 2];
                }
                Output[iX] = (unsigned char)((sumR * iScale) >> 16);
                Output[iX + 1] = (unsigned char)((sumG * iScale) >> 16);
                Output[iX + 2] = (unsigned char)((sumB * iScale) >> 16);
                Output[iX + 3] = Input[iX + 3];
                for (int y = 1; y < iRadius; y++) {
                    int i = iX + y * iWidthStep;
                    int i0 = i + iRadWidthStep;
                    sumR += Input[i0];
                    sumR -= Input[iX];
                    sumG += Input[i0 + 1];
                    sumG -= Input[iX + 1];
                    sumB += Input[i0 + 2];
                    sumB -= Input[iX + 2];
                    Output[i] = (unsigned char)((sumR * iScale) >> 16);
                    Output[i + 1] = (unsigned char)((sumG * iScale) >> 16);
                    Output[i + 2] = (unsigned char)((sumB * iScale) >> 16);
                    Output[i + 3] = Input[i + 3];
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
                    Output[iKernal] = (unsigned char)((sumR * iScale) >> 16);
                    Output[iKernal + 1] = (unsigned char)((sumG * iScale) >> 16);
                    Output[iKernal + 2] = (unsigned char)((sumB * iScale) >> 16);
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
                    Output[iRight] = (unsigned char)((sumR * iScale) >> 16);
                    Output[iRight + 1] = (unsigned char)((sumG * iScale) >> 16);
                    Output[iRight + 2] = (unsigned char)((sumB * iScale) >> 16);
                    Output[iRight + 3] = Input[iRight + 3];
                }
            }
            break;
        }
        default: break;
        }
    }

    OC_STATUS ocularBoxBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                  int Radius) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels != 1 && Channels != 3 && Channels != 4)
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure Radius is within valid range
        Radius = clamp(Radius, 1, 127);

        unsigned char *temp = (unsigned char*)malloc(Width * Height * Channels);
        if (temp == NULL) {
            return OC_STATUS_ERR_OUTOFMEMORY;
        }
        boxfilterRow(Input, temp, Width, Height, Channels, Radius);
        boxfilterCol(temp, Output, Width, Height, Channels, Radius);
        free(temp);

        return OC_STATUS_OK;
    }

    OC_STATUS ocularSurfaceBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                     int Radius, int Threshold) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channel = Stride / Width;
        if ((Channel != 1) && (Channel != 3))
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure Radius and Threshold are within valid ranges
        Radius = clamp(Radius, 1, 127);
        Threshold = clamp(Threshold, 2, 255);

        if (Channel == 1) {
            unsigned short* ColHist = (unsigned short*)_aligned_malloc(256 * (Width + Radius + Radius) * sizeof(unsigned short), 32);
            if (ColHist == NULL)
                return OC_STATUS_ERR_OUTOFMEMORY;
            unsigned short* Hist = (unsigned short*)_aligned_malloc(256 * sizeof(unsigned short), 32);
            if (Hist == NULL)
                return OC_STATUS_ERR_OUTOFMEMORY;

            unsigned short* Intensity = (unsigned short*)_aligned_malloc(511 * sizeof(unsigned short), 32); // Avoid abs when a negative value is used
            if (Intensity == NULL)
                return OC_STATUS_ERR_OUTOFMEMORY;
            unsigned short* Level = (unsigned short*)_aligned_malloc(256 * sizeof(unsigned short), 32);
            if (Level == NULL)
                return OC_STATUS_ERR_OUTOFMEMORY;

            int* RowOffset = (int*)malloc((Width + Radius + Radius) * sizeof(int));
            if (RowOffset == NULL)
                return OC_STATUS_ERR_OUTOFMEMORY;
            int* ColOffset = (int*)malloc((Height + Radius + Radius) * sizeof(int));
            if (ColOffset == NULL)
                return OC_STATUS_ERR_OUTOFMEMORY;

            GetOffsetPos(RowOffset, Width, Radius, Radius);
            GetOffsetPos(ColOffset, Height, Radius, Radius);

            memset(ColHist, 0, 256 * (Width + Radius + Radius) * sizeof(unsigned short)); //	Make sure to clear

            for (int Y = 0; Y < 256; Y++)
                Level[Y] = Y;

            for (int Y = -255; Y <= 255; Y++) {
                int Factor = (255 - abs(Y) * 100 / Threshold);
                if (Factor < 0)
                    Factor = 0;
                Intensity[Y + 255] = Factor / 2;
            }

            for (int Y = 0; Y < Height; Y++) {
                if (Y == 0) //	The first row of column histograms
                {
                    for (int K = -Radius; K <= Radius; K++) {
                        unsigned char* LinePS = Input + ColOffset[K + Radius] * Stride;
                        for (int X = -Radius; X < Width + Radius; X++) {
                            ColHist[(X + Radius) * 256 + LinePS[RowOffset[X + Radius]]]++;
                        }
                    }
                } else //	Column histogram for other rows, update it
                {
                    unsigned char* LinePS = Input + ColOffset[Y - 1] * Stride;
                    for (int X = -Radius; X < Width + Radius; X++) // Delete the histogram data for the row that is out of range
                    {
                        ColHist[(X + Radius) * 256 + LinePS[RowOffset[X + Radius]]]--;
                    }

                    LinePS = Input + ColOffset[Y + Radius + Radius] * Stride;
                    for (int X = -Radius; X < Width + Radius; X++) // Increase the histogram data for the line in the incoming range
                    {
                        ColHist[(X + Radius) * 256 + LinePS[RowOffset[X + Radius]]]++;
                    }
                }

                memset(Hist, 0, 256 * sizeof(unsigned short)); //	Each row of histogram data is cleared first

                unsigned char* LinePS = Input + Y * Stride;
                unsigned char* LinePD = Output + Y * Stride;

                for (int X = 0; X < Width; X++) {
                    if (X == 0) {
                        for (int K = -Radius; K <= Radius; K++) //	First pixel, needs to be recalculated
                            HistogramAddShort(ColHist + (K + Radius) * 256, Hist);
                    } else {
                        HistogramSubAddShort(ColHist + (RowOffset[X - 1] + Radius) * 256, ColHist + (RowOffset[X + Radius + Radius] + Radius) * 256,
                                             Hist); //	The other pixels in the line can be deleted and added in turn.
                    }

                    LinePD[X] = HistogramCalc(Hist, LinePS[X], Intensity);
                }
            }
            _aligned_free(ColHist);
            _aligned_free(Hist);
            _aligned_free(Intensity);
            _aligned_free(Level);
            free(RowOffset);
            free(ColOffset);
        } else {
            unsigned char* SrcB = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
            unsigned char* SrcG = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
            unsigned char* SrcR = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
            unsigned char* DstB = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
            unsigned char* DstG = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
            unsigned char* DstR = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));

            SplitRGB(Input, SrcB, SrcG, SrcR, Width, Height, Stride);
            {
                ocularSurfaceBlurFilter(SrcB, DstB, Width, Height, Width, Radius, Threshold);
                ocularSurfaceBlurFilter(SrcG, DstG, Width, Height, Width, Radius, Threshold);
                ocularSurfaceBlurFilter(SrcR, DstR, Width, Height, Width, Radius, Threshold);
            }
            CombineRGB(DstB, DstG, DstR, Output, Width, Height, Stride);

            free(SrcB);
            free(SrcG);
            free(SrcR);
            free(DstB);
            free(DstG);
            free(DstR);
        }
    }

    OC_STATUS ocularBEEPSFilter(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                           float PhotometricStandardDeviation, float SpatialDecay, int RangeFilter) {

        // Implementation of the paper "Bi-Exponential Edge-Preserving Smoother".
        // Reference: https://bigwww.epfl.ch/publications/thevenaz1202.html

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if ((Channels != 1) && (Channels != 3) && (Channels != 4))
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        PhotometricStandardDeviation = clamp(PhotometricStandardDeviation, 1.0f, 255.0f);
        SpatialDecay = clamp(SpatialDecay, 0.01f, 0.250f);
        if (RangeFilter != 0 && RangeFilter != 1 && RangeFilter != 2) {
            RangeFilter = 0; // Default to Gaussian
        }

        float spatialContraDecay = 1.0f - SpatialDecay;
        float lambda = expf(-SpatialDecay);
        float rho = lambda + SpatialDecay / (2.0f - (spatialContraDecay + SpatialDecay));
        float inv_rho = 1.0f / rho;
        float weight = 0;
        int bpp = Channels;
        if (bpp == 4)
            bpp--;

        // Gaussian
        if (RangeFilter == 0) {
            weight = -0.5f / (PhotometricStandardDeviation * PhotometricStandardDeviation);
        }
        // Hyperbolic Secant
        if (RangeFilter == 1) {
            weight = -M_PI / (2.0f * PhotometricStandardDeviation);
        }
        // Euler
        if (RangeFilter == 2) {
            float euler = 2.718281828459f;
            weight = -powf((0.75f * euler) / (PhotometricStandardDeviation * 
                                              PhotometricStandardDeviation), 1.0f / 3.0f);
            weight *= (PhotometricStandardDeviation < 0.0f) 
                        ? (-1.0f) 
                        : ((0.0 == PhotometricStandardDeviation) ? (0.0) : (1.0f));
        }
        float *cache = (float *)calloc(Stride * Height, sizeof(float));
        if (cache == NULL)
            return OC_STATUS_ERR_OUTOFMEMORY;

        for (int y = 0; y < Height; y++) {
            float* lineCache = cache + y * Stride;
            const unsigned char* lineIn = Input + y * Stride;
            for (int x = 0; x < Width; x++) {
                for (int c = 0; c < Channels; ++c) {
                    lineCache[c] = lineIn[c];
                }
                lineCache += Channels;
                lineIn += Channels;
            }
        }

        float *horizontal = cache;
        for (int y = 0; y < Height; y++) {
            // forwards
            float* fNextHori = horizontal + y * Stride + Channels;
            float* fPrevHori = horizontal + y * Stride;
            for (int x = 1; x < Width; x++) {
                for (int c = 0; c < bpp; ++c) {
                    fNextHori[c] -= calcWeight(weight, spatialContraDecay, fNextHori[c] - rho * fPrevHori[c]);
                    fNextHori[c] *= inv_rho;
                }
                fNextHori += Channels;
                fPrevHori += Channels;
            }
            // backwards
            float* bPrevHori = horizontal + y * Stride + (Width - 2) * Channels;
            float* bNextHori = horizontal + y * Stride + (Width - 1) * Channels;
            for (int x = 1; x < Width; x++) {
                for (int c = 0; c < bpp; ++c) {
                    bPrevHori[c] -= calcWeight(weight, spatialContraDecay, bPrevHori[c] - rho * bNextHori[c]);
                    bPrevHori[c] *= inv_rho;
                }
                bPrevHori -= Channels;
                bNextHori -= Channels;
            }
        }

        float *vertical = cache;
        for (int x = 0; x < Width; x++) {
            // forwards
            float* fNextVert = vertical + x * Channels + Stride;
            float* fPrevVert = vertical + x * Channels;
            for (int y = 1; y < Height; y++) {
                for (int c = 0; c < bpp; ++c) {
                    fNextVert[c] -= calcWeight(weight, spatialContraDecay, fNextVert[c] - rho * fPrevVert[c]);
                    fNextVert[c] *= inv_rho;
                }
                fNextVert += Stride;
                fPrevVert += Stride;
            }
            // backwards
            float* bPrevVert = vertical + x * Channels + (Height - 2) * Stride;
            float* bNextVert = vertical + x * Channels + (Height - 1) * Stride;
            for (int y = 1; y < Height; y++) {
                for (int c = 0; c < bpp; ++c) {
                    bPrevVert[c] -= calcWeight(weight, spatialContraDecay, bPrevVert[c] - rho * bNextVert[c]);
                    bPrevVert[c] *= inv_rho;
                }
                bPrevVert -= Stride;
                bNextVert -= Stride;
            }
        }

        for (int y = 0; y < Height; y++) {
            const float* lineCache = cache + y * Stride;
            unsigned char* lineOut = Output + y * Stride;
            for (int x = 0; x < Width; x++) {
                for (int c = 0; c < Channels; ++c) {
                    lineOut[c] = (unsigned char)clamp(lineCache[c], 0, 255);
                }
                lineOut += Channels;
                lineCache += Channels;
            }
        }
        free(cache);

        return OC_STATUS_OK;
    }

    OC_STATUS ocularSharpenFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float Strength) {

        if (Input == NULL || Output == NULL) {
            return OC_STATUS_ERR_NULLREFERENCE;
        }
        if (Width <= 0 || Height <= 0 || Stride <= 0) {
            return OC_STATUS_ERR_INVALIDPARAMETER;
        }

        // Clamp filter specific parameters to valid range
        Strength = clamp(Strength, 0.0f, 10.0f);

        // Calculate center weight based on strength
        float center = 1.0f + (4.0f * Strength);
        float adjacent = -Strength;

        int Channels = Stride / Width;

        // Process each pixel except borders
        for (int y = 1; y < Height - 1; y++) {
            for (int x = 1; x < Width - 1; x++) {
                for (int c = 0; c < Channels; c++) {
                    float sum = 0.0f;
                    int idx = (y * Width + x) * Channels + c;

                    // Apply kernel weights
                    sum += Input[idx] * center;              // Center pixel
                    sum += Input[idx - Channels] * adjacent; // Top
                    sum += Input[idx + Channels] * adjacent; // Bottom
                    sum += Input[idx - Stride] * adjacent;   // Left
                    sum += Input[idx + Stride] * adjacent;   // Right

                    Output[idx] = ClampToByte(sum);
                }
            }
        }

        // Copy unchanged border pixels
        for (int y = 0; y < Height; y++) {
            for (int x = 0; x < Width; x++) {
                if (y == 0 || y == Height - 1 || x == 0 || x == Width - 1) {
                    for (int c = 0; c < Channels; c++) {
                        int idx = (y * Width + x) * Channels + c;
                        Output[idx] = Input[idx];
                    }
                }
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularSkinSmoothingFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                        int smoothingLevel, bool applySkinFilter) {

        int Channels = Stride / Width;
        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Channels == 1)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        // Ensure filter specific parameters are within valid ranges
        smoothingLevel = clamp(smoothingLevel, 0, 100);

        // 1. Detect skin color, adapt radius according to skin color ratio
        unsigned int skinSum = skinDetection(Input, Width, Height, Channels);
        float skin_rate = skinSum / (float)(Width * Height) * 100;
        int radius = min(Width, Height) / skin_rate + 1;
        // 2. Perform edge detection to obtain an edge map and apply skin denoise using smoothing level
        skinDenoise(Input, Output, Width, Height, Channels, radius, smoothingLevel);
        // 3. Re-detect skin color based on the denoise results, filtering non-skin areas
        if (applySkinFilter)
            skinFilter(Input, Output, Width, Height, Channels);

        return OC_STATUS_OK;
    }

    OC_STATUS ocularResamplingFilter(unsigned char* Input, int Width, int Height, unsigned int Stride, unsigned char* Output,
                                int newWidth, int newHeight, int dstStride, OcInterpolationMode InterpolationMode) {

        int Channels = Stride / Width;
        if ((Input == NULL) || (Output == NULL))
            return OC_STATUS_ERR_NULLREFERENCE;
        if ((Width <= 0) || (Height <= 0) || (newWidth <= 0) || (newHeight <= 0))
            return OC_STATUS_ERR_INVALIDPARAMETER;
        if ((Channels != 1) && (Channels != 3) && (Channels != 4))
            return OC_STATUS_ERR_NOTSUPPORTED;

        if ((Width == newWidth) && (Height == newHeight)) {
            memcpy(Output, Input, Width * Height * Channels * sizeof(unsigned char));
            return OC_STATUS_OK;
        }

        switch (InterpolationMode) {
            case OC_INTERPOLATE_NEAREST: nearestNeighborResize(Input, Output, Width, Height, newWidth, newHeight, Channels); break;
            case OC_INTERPOLATE_BILINEAR: bilinearResize(Input, Output, Width, Height, newWidth, newHeight, Channels); break;
            case OC_INTERPOLATE_BICUBIC: bicubicResize(Input, Output, Width, Height, newWidth, newHeight, Channels); break;
            case OC_INTERPOLATE_LANZCOS: lanzcosResize(Input, Width, Height, Stride, Output, newWidth, newHeight, dstStride); break;
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularRotateImage(unsigned char* Input, int Width, int Height, int Stride, unsigned char* Output, 
                                int newWidth, int newHeight, float angle, bool useTransparency,
                                OcInterpolationMode InterpolationMode, unsigned char fillColorR, unsigned char fillColorG, 
                                unsigned char fillColorB) {

        // print parameters for testing
        printf("Width: %d, Height: %d, Stride: %d, newWidth: %d, newHeight: %d, angle: %f, useTransparency: %d, fillColorR: %d, fillColorG: %d, fillColorB: %d\n", 
               Width, Height, Stride, newWidth, newHeight, angle, useTransparency, fillColorR, fillColorG, fillColorB);

        int Channels = Stride / Width;
        if ((Input == NULL) || (Output == NULL))
            return OC_STATUS_ERR_NULLREFERENCE;
        if ((Width <= 0) || (Height <= 0) || (newWidth <= 0) || (newHeight <= 0))
            return OC_STATUS_ERR_INVALIDPARAMETER;
        if ((Channels != 1) && (Channels != 3) && (Channels != 4))
            return OC_STATUS_ERR_NOTSUPPORTED;

        // Ensure filter specific parameters are within valid ranges
        angle = clamp(angle, -360.0f, 360.0f);
        fillColorR = ClampToByte(fillColorR);
        fillColorG = ClampToByte(fillColorG);
        fillColorB = ClampToByte(fillColorB);

        switch (InterpolationMode) {
            case OC_INTERPOLATE_BILINEAR:
                bilinearRotate(Input, Width, Height, Stride, Output, newWidth, newHeight, angle, useTransparency, 
                                fillColorR, fillColorG, fillColorB);
                break;
            case OC_INTERPOLATE_BICUBIC:
                bicubicRotate(Input, Width, Height, Stride, Output, newWidth, newHeight, angle, useTransparency, 
                                fillColorR, fillColorG, fillColorB);
                break;
            default: return OC_STATUS_ERR_INVALIDPARAMETER;
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularCropImage(const unsigned char* Input, int Width, int Height, int srcStride, unsigned char* Output, 
                                int cropX, int cropY, int dstWidth, int dstHeight, int dstStride) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || dstWidth <= 0 || dstHeight <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        // Ensure filter specific parameters are within valid ranges
        cropX = clamp(cropX, 0, Width - 1);
        cropY = clamp(cropY, 0, Height - 1);

        int Channels = srcStride / Width;

        const unsigned char* src = Input + cropY * srcStride + cropX * Channels;
        unsigned char* dst = Output;

        for (int y = 0; y < dstHeight; y++) {
            memcpy(dst, src, dstStride);
            src += srcStride;
            dst += dstStride;
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularFlipImage(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                OcDirection direction) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        if (direction == OC_DIRECTION_HORIZONTAL) {
            int channels = Stride / Width;
            for (int y = 0; y < Height; y++) {
                for (int x = 0; x < Width / 2; x++) {
                    for (int c = 0; c < channels; c++) {
                        unsigned char temp = Input[y * Stride + x * channels + c];
                        Output[y * Stride + x * channels + c] = Input[y * Stride + (Width - x - 1) * channels + c];
                        Output[y * Stride + (Width - x - 1) * channels + c] = temp;
                    }
                }
            }
        } else if (direction == OC_DIRECTION_VERTICAL) {
            int channels = Stride / Width;
            for (int y = 0; y < Height / 2; y++) {
                for (int x = 0; x < Width; x++) {
                    for (int c = 0; c < channels; c++) {
                        unsigned char temp = Input[y * Stride + x * channels + c];
                        Output[y * Stride + x * channels + c] = Input[(Height - y - 1) * Stride + x * channels + c];
                        Output[(Height - y - 1) * Stride + x * channels + c] = temp;
                    }
                }
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularDespeckle(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                int maxWindowSize, int Threshold) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || maxWindowSize <= 0 || Threshold <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        // Ensure filter specific parameters are within valid ranges
        maxWindowSize = clamp(maxWindowSize, 1, 127);
        Threshold = clamp(Threshold, 2, 255);

        int windowSize, x, y, c;
        unsigned char* window = (unsigned char*)malloc(maxWindowSize * maxWindowSize * sizeof(unsigned char));
        if (window == NULL)
            return OC_STATUS_ERR_OUTOFMEMORY;

        int Channels = Stride / Width;

        for (y = 0; y < Height; y++) {
            for (x = 0; x < Width; x++) {
                for (c = 0; c < Channels; c++) {
                    windowSize = 3; // Start with a 3x3 window
                    while (windowSize <= maxWindowSize) {
                        int windowOffset = windowSize / 2;
                        int windowIndex = 0;

                        // Fill the window
                        for (int wy = -windowOffset; wy <= windowOffset; wy++) {
                            for (int wx = -windowOffset; wx <= windowOffset; wx++) {
                                int sx = x + wx;
                                int sy = y + wy;
                                if (sx >= 0 && sx < Width && sy >= 0 && sy < Height) {
                                    window[windowIndex++] = Input[(sy * Width + sx) * Channels + c];
                                } else {
                                    window[windowIndex++] = Input[(y * Width + x) * Channels + c];
                                }
                            }
                        }

                        unsigned char median = getMedian(window, windowSize * windowSize);
                        unsigned char minValue = window[0];
                        unsigned char maxValue = window[0];
                        for (int i = 1; i < windowSize * windowSize; i++) {
                            if (window[i] < minValue)
                                minValue = window[i];
                            if (window[i] > maxValue)
                                maxValue = window[i];
                        }

                        unsigned char currentPixel = Input[(y * Width + x) * Channels + c];
                        int difference = abs((int)currentPixel - (int)median);

                        if (difference <= Threshold) {
                            Output[(y * Width + x) * Channels + c] = currentPixel;
                            break;
                        } else if (median > minValue && median < maxValue) {
                            Output[(y * Width + x) * Channels + c] = median;
                            break;
                        } else {
                            windowSize += 2;
                            if (windowSize > maxWindowSize) {
                                Output[(y * Width + x) * Channels + c] = median;
                            }
                        }
                    }
                }
            }
        }

        free(window);

        return OC_STATUS_OK;
    }

    OC_STATUS ocularDocumentDeskew(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                    bool* Skewed) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;

        int Channels = Stride / Width;
        // maximum tilt angle
        int maxSkewToDetect = 89;

        OcRect rect = { 0 };
        rect.Width = Width;
        rect.Height = Height;
        // calculate the tilt angle based on the two straight lines with the largest weight
        int nLineCount = 2;
        // number of angle steps
        int stepsPerDegree = 1;
        // local critical radius
        int localPeakRadius = 10;
        ocularGrayscaleFilter(Input, Output, Width, Height, Stride);
        if (!isTextImage(Output, Width, Height)) {
            ocularColorInvertFilter(Output, Output, Width, Height, Width);
        }
        float skewAngle = calcSkewAngle(Output, Width, Height, &rect, maxSkewToDetect, stepsPerDegree, localPeakRadius, nLineCount);
        if ((skewAngle == 0) || (skewAngle < -maxSkewToDetect || skewAngle > maxSkewToDetect)) {
            memcpy(Output, Input, Height * Stride * sizeof(unsigned char));
            Skewed = false;
            return OC_STATUS_OK;
        } else {
            bilinearRotate(Input, Width, Height, Stride, Output, Width, Height, -skewAngle, false, 255, 255, 255);
            *Skewed = true;
        }
        return OC_STATUS_OK;
    }

    OC_STATUS ocularAutoLevel(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                float fraction) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || fraction <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        // Ensure filter specific parameters are within valid ranges
        fraction = clamp(fraction, 0.001f, 0.1f);

        int Channels = Stride / Width;
        switch (Channels) {
        case 4:
        case 3: {
            unsigned int histoRGB[256 * 3] = { 0 };
            unsigned int* histoR = &histoRGB[0];
            unsigned int* histoG = &histoRGB[256];
            unsigned int* histoB = &histoRGB[512];
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
            int gap = (int)(fraction * Width * Height);
            int sumR = 0;
            int sumG = 0;
            int sumB = 0;

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
            unsigned char* MapB = &MapRGB[0];
            unsigned char* MapG = &MapRGB[256];
            unsigned char* MapR = &MapRGB[512];
            for (int i = 0; i < 256; i++) {
                if (i < thresholdRMin)
                    MapR[i] = (unsigned char)((i + 0) >> 1);
                else if (i > thresholdRMax)
                    MapR[i] = (255);
                else
                    MapR[i] = ClampToByte((int)((i - thresholdRMin) * 255.0) / (thresholdRMax - thresholdRMin));
                if (i < thresholdGMin)
                    MapG[i] = (unsigned char)((i + 0) >> 1);
                else if (i > thresholdGMax)
                    MapG[i] = (255);
                else
                    MapG[i] = ClampToByte((int)((i - thresholdGMin) * 255.0) / (thresholdGMax - thresholdGMin));
                if (i < thresholdBMin)
                    MapB[i] = (unsigned char)((i + 0) >> 1);
                else if (i > thresholdBMax)
                    MapB[i] = (255);
                else
                    MapB[i] = ClampToByte((int)((i - thresholdBMin) * 255.0) / (thresholdBMax - thresholdBMin));
            }

            for (int Y = 0; Y < Height; Y++) {
                const unsigned char* pInput = Input + (Y * Stride);
                unsigned char* pOutput = Output + (Y * Stride);
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
            int gap = (int)(fraction * Width * Height);
            int sumGray = 0;
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
                return OC_STATUS_ERR_UNKNOWN;
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

        return OC_STATUS_OK;
    }

    OC_STATUS ocularEqualizeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels != 3)
            return OC_STATUS_ERR_NOTSUPPORTED;

        int histogram[256] = { 0 };
        unsigned char Lut[256] = { 0 };

        for (int y = 0; y < Height; y++) {
            unsigned char* pInput = Input + (y * Stride);
            for (int x = 0; x < Width; x++) {
                histogram[pInput[0]]++;
                histogram[pInput[1]]++;
                histogram[pInput[2]]++;
                pInput += Channels;
            }
        }

        long total = Width * Height * Channels;
        int curr = 0;
        for (int i = 0; i < 256; i++) {
            // cumulative frequency
            curr += histogram[i];
            Lut[i] = round((((float)curr) * 255) / total);
        }
        for (int y = 0; y < Height; y++) {
            unsigned char* pInput = Input + (y * Stride);
            unsigned char* pOutput = Output + (y * Stride);
            for (int x = 0; x < Width; x++) {
                pOutput[0] = ClampToByte(Lut[pInput[0]]);
                pOutput[1] = ClampToByte(Lut[pInput[1]]);
                pOutput[2] = ClampToByte(Lut[pInput[2]]);
                pInput += Channels;
                pOutput += Channels;
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularAutoThreshold(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                    OcAutoThresholdMethod method) {

        if (Input == NULL || Output == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (Width <= 0 || Height <= 0 || Stride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int Channels = Stride / Width;
        if (Channels != 1)
            return OC_STATUS_ERR_NOTSUPPORTED;

        int histogram[256] = { 0 };
        int* histogramSmooth[256] = { 0 }; // for future use
        int threshold = 0;

        // get histogram
        for (int y = 0; y < Height; y++) {
            unsigned char* pInput = Input + (y * Stride);
            for (int x = 0; x < Width; x++) {
                histogram[pInput[0]]++;
                pInput++;
            }
        }

        switch (method) {
            case OC_AUTO_THRESHOLD_MEAN: threshold = GetMeanThreshold(histogram); break;
            case OC_AUTO_THRESHOLD_HUANG: threshold = GetHuangFuzzyThreshold(histogram); break;
            case OC_AUTO_THRESHOLD_MIN: threshold = GetMinimumThreshold(histogram, histogramSmooth); break;
            case OC_AUTO_THRESHOLD_INTERMODES: threshold = GetIntermodesThreshold(histogram, histogramSmooth); break;
            case OC_AUTO_THRESHOLD_PTILE: threshold = GetPTileThreshold(histogram, 50); break;
            case OC_AUTO_THRESHOLD_ITERBEST: threshold = GetIterativeBestThreshold(histogram); break;
            case OC_AUTO_THRESHOLD_OTSU: threshold = GetOSTUThreshold(histogram); break;
            case OC_AUTO_THRESHOLD_1DMAX: threshold = Get1DMaxEntropyThreshold(histogram); break;
            case OC_AUTO_THRESHOLD_MOMENT: threshold = GetMomentPreservingThreshold(histogram); break;
            case OC_AUTO_THRESHOLD_KITTLER: threshold = GetKittlerMinError(histogram); break;
            case OC_AUTO_THRESHOLD_ISODATA: threshold = GetIsoDataThreshold(histogram); break;
            case OC_AUTO_THRESHOLD_SHANBHAG: threshold = GetShanbhagThreshold(histogram); break;
            case OC_AUTO_THRESHOLD_YEN: threshold = GetYenThreshold(histogram); break;
            default: break;
        }

        if (threshold <= 0)
            return OC_STATUS_ERR_UNKNOWN; // selected method failed to calculate a threshold for whatever reason

        for (int y = 0; y < Height; y++) {
            unsigned char* pInput = Input + (y * Stride);
            unsigned char* pOutput = Output + (y * Stride);
            for (int x = 0; x < Width; x++) {
                pOutput[0] = pInput[0] > threshold ? 255 : 0;
                pInput += Channels;
                pOutput += Channels;
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularBacklightRepair(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride) {

        // Implementation of the paper "Adaptive and integrated neighborhood-dependent approach for nonlinear 
        // enhancement of color images", but it has been deeply improved.
        // Reference: https://ui.adsabs.harvard.edu/abs/2005JEI....14d3006T/abstract

        int channels = Stride / Width;

        if (channels != 3)
            return OC_STATUS_ERR_NOTSUPPORTED;
        if ((Input == NULL) || (Output == NULL))
            return OC_STATUS_ERR_NULLREFERENCE;
        if ((Width <= 0) || (Height <= 0))
            return OC_STATUS_ERR_INVALIDPARAMETER;

        int RadiusS = 5, RadiusM = 20, RadiusL = 120;
        const int LowLevel = 50, HighLevel = 150;
        const float MinCDF = 0.1f;
        const float Inv255 = 1.0f / 255.0f;

        int* Histogram = (int*)calloc(256, sizeof(int));
        unsigned char* Table = (unsigned char*)malloc(256 * 256 * sizeof(unsigned char));
        unsigned char* BlurS = (unsigned char*)malloc(Width * Height * sizeof(unsigned char)); //     Blur of each scale
        unsigned char* BlurM = (unsigned char*)malloc(Width * Height * sizeof(unsigned char)); //     Blur of each scale
        unsigned char* BlurL = (unsigned char*)malloc(Width * Height * sizeof(unsigned char)); //     Blur of each scale
        unsigned char* Luminance = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));

        float Z = 0, P = 0;
        ocularGrayscaleFilter(Input, Luminance, Width, Height, Stride); // get the brightness component

        for (int y = 0; y < Height * Width; y++)
            Histogram[Luminance[y]]++; // histogram of statistical brightness components

        float Sum = 0, Mean = 0, StdDev = 0;
        for (int y = 0; y < 256; y++)
            Sum += Histogram[y] * y; // sum of pixels
        Mean = Sum / (Width * Height);
        for (int y = 0; y < 256; y++)
            StdDev += Histogram[y] * (y - Mean) * (y - Mean);
        StdDev = sqrtf(StdDev / (Width * Height)); // mean square error of the global image

        int CDF = 0, L = 0;
        for (L = 0; L < 256; L++) {
            CDF += Histogram[L];
            if (CDF >= Width * Height * MinCDF)
                break; // where L is the intensity level corresponding to a cumulative distribution function CDF of 0.1.
        }
        // Calculate Z value
        if (L <= LowLevel)
            Z = 0;
        else if (L <= HighLevel)
            Z = (L - LowLevel) * 1.0f / (HighLevel - LowLevel);
        else
            Z = 1;

        // Calculate P value. Also, P is determined by the global standard deviation of the input intensity image I x, y as
        if (StdDev <= 3)
            P = 3;
        else if (StdDev <= 10)
            P = (27 - 2 * StdDev) / 7.0f;
        else
            P = 1;

        // Y represents the convolution value of I
        for (int y = 0; y < 256; y++) {
            // X represents I (original brightness value)
            for (int x = 0; x < 256; x++) {
                float I = x * Inv255;
                I = (powf(I, 0.75f * Z + 0.25f) + (1 - I) * 0.4f * (1 - Z) + powf(I, 2 - Z)) * 0.5f;
                Table[y * 256 + x] = ClampToByte(255 * powf(I, powf((y + 1.0f) / (x + 1.0f), P)) + 0.5f);
            }
        }

        ocularGaussianBlurFilter(Luminance, BlurS, Width, Height, Width, RadiusS);
        ocularGaussianBlurFilter(Luminance, BlurM, Width, Height, Width, RadiusM);
        ocularGaussianBlurFilter(Luminance, BlurL, Width, Height, Width, RadiusL);

        for (int y = 0; y < Height; y++) {
            unsigned char* scanIn = Input + y * Stride;
            unsigned char* scanOut = Output + y * Stride;
            int index = y * Width;
            for (int x = 0; x < Width; x++, index++, scanIn += 3, scanOut += 3) {
                int lum = Luminance[index];
                if (lum == 0) {
                    scanOut[0] = 0;
                    scanOut[1] = 0;
                    scanOut[2] = 0;
                } else {
                    int value = ((Table[lum + (BlurS[index] << 8)] + Table[lum + (BlurM[index] << 8)] + Table[lum + (BlurL[index] << 8)]) / 3);
                    scanOut[0] = ClampToByte(scanIn[0] * value / lum);
                    scanOut[1] = ClampToByte(scanIn[1] * value / lum);
                    scanOut[2] = ClampToByte(scanIn[2] * value / lum);
                }
            }
        }
        free(Histogram);
        free(Table);
        free(BlurS);
        free(BlurM);
        free(BlurL);
        free(Luminance);

        return OC_STATUS_OK;
    }

    OC_STATUS ocularLayerBlend(unsigned char* baseInput, int bWidth, int bHeight, int bStride, unsigned char* mixInput, 
                               int mWidth, int mHeight, int mStride, OcBlendMode blendMode, int alpha) {

        if (baseInput == NULL || mixInput == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if (bWidth <= 0 || bHeight <= 0 || mWidth <= 0 || mHeight <= 0 || bStride <= 0 || mStride <= 0)
            return OC_STATUS_ERR_INVALIDPARAMETER;

        // Ensure filter specific parameters are within valid ranges
        alpha = clamp(alpha, 0, 100);

        for (int y = 0; y < bHeight; y++) {
            unsigned char* pBaseInput = baseInput + (y * bStride);
            unsigned char* pMixInput = mixInput + (y * bStride);
            for (int x = 0; x < bWidth; x++) {
                int baseR = pBaseInput[0];
                int baseG = pBaseInput[1];
                int baseB = pBaseInput[2];
                int mixR = pMixInput[0];
                int mixG = pMixInput[1];
                int mixB = pMixInput[2];
                int resR = 0;
                int resG = 0;
                int resB = 0;
                layerBlend(baseR, baseG, baseB, mixR, mixG, mixB, &resR, &resG, &resB, blendMode, alpha);
                pBaseInput[0] = resR;
                pBaseInput[1] = resG;
                pBaseInput[2] = resB;
                pBaseInput += 3;
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularColorBalance(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int redBalance,
                                 int greenBalance, int blueBalance, OcToneBalanceMode mode, bool preserveLuminosity) {

        if (Input == NULL || Output == NULL) {
            return OC_STATUS_ERR_NULLREFERENCE;
        }
        if (Width <= 0 || Height <= 0 || Stride <= 0) {
            return OC_STATUS_ERR_INVALIDPARAMETER;
        }

        int Channels = Stride / Width;
        if (Channels == 1) {
            return OC_STATUS_ERR_NOTSUPPORTED;
        }

        // Ensure filter specific parameters are within valid ranges
        redBalance = clamp(redBalance, -100, 100);
        greenBalance = clamp(greenBalance, -100, 100);
        blueBalance = clamp(blueBalance, -100, 100);

        unsigned char lookupR[256], lookupG[256], lookupB[256];

        for (int i = 0; i < 256; i++) {
            float normalizedValue = i / 255.0f;
            float factor;

            switch (mode) {
                case SHADOWS: factor = 0.5f * (1.0f - normalizedValue); break;
                case MIDTONES: factor = 0.5f - fabs(normalizedValue - 0.5f); break;
                case HIGHLIGHTS: factor = 0.5f * normalizedValue; break;
            }

            // Convert integer balance (-100 to 100) to float (-1.0 to 1.0)
            float r = i + (redBalance / 100.0f) * factor * 255.0f;
            float g = i + (greenBalance / 100.0f) * factor * 255.0f;
            float b = i + (blueBalance / 100.0f) * factor * 255.0f;

            lookupR[i] = (unsigned char)fmax(0, fmin(255, r));
            lookupG[i] = (unsigned char)fmax(0, fmin(255, g));
            lookupB[i] = (unsigned char)fmax(0, fmin(255, b));
        }

        for (int i = 0; i < Width * Height * Channels; i += Channels) {
            unsigned char r = lookupR[Input[i]];
            unsigned char g = lookupG[Input[i + 1]];
            unsigned char b = lookupB[Input[i + 2]];

            if (preserveLuminosity) {
                float originalLuminosity = 0.299f * Input[i] + 0.587f * Input[i + 1] + 0.114f * Input[i + 2];
                float newLuminosity = 0.299f * r + 0.587f * g + 0.114f * b;

                if (newLuminosity > 0) {
                    float luminosityRatio = originalLuminosity / newLuminosity;
                    r = (unsigned char)fmin(255, r * luminosityRatio);
                    g = (unsigned char)fmin(255, g * luminosityRatio);
                    b = (unsigned char)fmin(255, b * luminosityRatio);
                }
            }

            Output[i] = r;
            Output[i + 1] = g;
            Output[i + 2] = b;

            if (Channels == 4) {
                Output[i + 3] = Input[i + 3]; // Preserve alpha channel if present
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularMultiscaleRetinex(unsigned char* input, unsigned char* output, int width, int height, int channels, 
                                 OcRetinexMode mode, int scale, float numScales, float dynamic) {

        if (input == NULL || output == NULL) {
            return OC_STATUS_ERR_NULLREFERENCE;
        }
        if (width <= 0 || height <= 0) {
            return OC_STATUS_ERR_INVALIDPARAMETER;
        }

        // Ensure filter specific parameters are within valid ranges
        scale = clamp(scale, 16, 250);
        numScales = clamp(numScales, 1.0f, 8.0f);
        dynamic = clamp(dynamic, 0.05f, 4.0f);

        RetinexParams params = {
            .scales_mode = mode,
            .scale = scale,
            .nscales = numScales,
            .cvar = dynamic
        };

        int scaleIdx, row, col;
        int i, j;
        int size;
        int pos;
        int channel;
        unsigned char* psrc = NULL; // backup pointer for src buffer
        float* dst = NULL;          // float buffer for algorithm
        float* pdst = NULL;         // backup pointer for float buffer
        float *in, *out;
        int channelsize; /* Float memory cache for one channel */
        float weight;
        gauss3_coefs coef;
        float mean, var;
        float mini, range, maxi;
        float alpha;
        float gain;
        float offset;

        // Allocate all the memory needed for algorithm
        size = width * height * channels;
        dst = (float*)malloc(size * sizeof(float));
        if (dst == NULL) {
            return OC_STATUS_ERR_OUTOFMEMORY;
        }
        memset(dst, 0, size * sizeof(float));

        channelsize = (width * height);
        in = (float*)malloc(channelsize * sizeof(float));
        if (in == NULL) {
            free(dst);
            return OC_STATUS_ERR_OUTOFMEMORY;
        }

        out = (float*)malloc(channelsize * sizeof(float));
        if (out == NULL) {
            free(in);
            free(dst);
            return OC_STATUS_ERR_OUTOFMEMORY;
        }

        // Calculate the scales of filtering according to the number of filter and their distribution.
        retinex_scales_distribution(RetinexScales, params.nscales, params.scales_mode, params.scale);
        
        /*
        Filtering according to the various scales.
        Summerize the results of the various filters according to a specific weight (here equivalent for all).
        */
        weight = 1.0f / (float)params.nscales;

        /*
        The recursive filtering algorithm needs different coefficients according to the selected scale 
        (~ = standard deviation of Gaussian).
        */
        pos = 0;
        for (channel = 0; channel < 3; channel++) {
            for (i = 0, pos = channel; i < channelsize; i++, pos += channels) {
                in[i] = (float)(input[pos] + 1.0);
            }
            for (scaleIdx = 0; scaleIdx < params.nscales; scaleIdx++) {
                compute_coefs3(&coef, RetinexScales[scaleIdx]);

                /*
                 *  Filtering (smoothing) Gaussian recursive.
                 *
                 *  Filter rows first
                 */
                for (row = 0; row < height; row++) {
                    pos = row * width;
                    gausssmooth(in + pos, out + pos, width, 1, &coef);
                }

                memcpy(in, out, channelsize * sizeof(float));
                memset(out, 0, channelsize * sizeof(float));

                /*
                 *  Filtering (smoothing) Gaussian recursive.
                 *
                 *  Second columns
                 */
                for (col = 0; col < width; col++) {
                    pos = col;
                    gausssmooth(in + pos, out + pos, height, width, &coef);
                }


                /*
                Summarize the filtered values.
                In fact one calculates a ratio between the original values and the filtered values.
                */
                for (i = 0, pos = channel; i < channelsize; i++, pos += channels) {
                    dst[pos] += weight * (float)(log(input[pos] + 1.0f) - log(out[i]));
                }
            }
        }
        free(in);
        free(out);

        /*
        Final calculation with original value and cumulated filter values.
        The parameters gain, alpha and offset are constants.
        */
        /* Ci(x,y)=log[a Ii(x,y)]-log[ Ei=1-s Ii(x,y)] */

        alpha = 128.0f;
        gain = 1.0f;
        offset = 0.0f;

        unsigned char* pOutput = NULL;
        for (i = 0; i < size; i += channels) {
            float logl;

            psrc = input + i;
            //pOutput = output + i;
            pdst = dst + i;

            logl = (float)log((float)psrc[0] + (float)psrc[1] + (float)psrc[2] + 3.0f);

            pdst[0] = gain * ((float)(log(alpha * (psrc[0] + 1.0f)) - logl) * pdst[0]) + offset;
            pdst[1] = gain * ((float)(log(alpha * (psrc[1] + 1.0f)) - logl) * pdst[1]) + offset;
            pdst[2] = gain * ((float)(log(alpha * (psrc[2] + 1.0f)) - logl) * pdst[2]) + offset;
        }

        /*
        Adapt the dynamics of the colors according to the statistics of the first and second order.
        The use of the variance makes it possible to control the degree of saturation of the colors.
        */
        pdst = dst;

        compute_mean_var(pdst, &mean, &var, size, channels);
        mini = mean - params.cvar * var;
        maxi = mean + params.cvar * var;
        range = maxi - mini;

        if (!range)
            range = 1.0;

        for (i = 0; i < size; i += channels) {
            psrc = input + i;
            pdst = dst + i;
            pOutput = output + i;

            for (j = 0; j < 3; j++) {
                float c = 255 * (pdst[j] - mini) / range;
                pOutput[j] = (unsigned char)clamp(c, 0, 255);
            }
        }

        free(dst);

        return OC_STATUS_OK;
    }

    OC_STATUS ocularCannyEdgeDetect(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels,
                               CannyNoiseFilter kernel_size, int weak_threshold, int strong_threshold) {

        if (Input == NULL || Output == NULL) {
            return OC_STATUS_ERR_NULLREFERENCE;
        }
        if (Width <= 0 || Height <= 0) {
            return OC_STATUS_ERR_INVALIDPARAMETER;
        }
        if (Channels != 1) {    
            return OC_STATUS_ERR_NOTSUPPORTED;
        }
        
        // Ensure filter specific parameters are within valid ranges
        if (kernel_size != CannyGaus3x3 && kernel_size != CannyGaus5x5) {
            kernel_size = CannyGaus3x3;
        }
        weak_threshold = clamp(weak_threshold, 0, 255);
        strong_threshold = clamp(strong_threshold, 0, 255);

        // Sobel, as for why this can be used instead of Gaussian derivative, see
        // https://medium.com/@haidarlina4/sobel-vs-canny-edge-detection-techniques-step-by-step-implementation-11ae6103a56a
        const int8_t Gx[] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };

        // Upward minus downward, to achieve the difference in the y direction, dy
        const int8_t Gy[] = { 1, 2, 1, 0, 0, 0, -1, -2, -1 };

        int offset_xy = 1; // for kernel = 3
        int8_t* kernel = (int8_t*)Gaus3x3;
        int kernel_div = Gaus3x3Div;

        if (kernel_size == CannyGaus5x5) {
            offset_xy = 2;
            kernel = (int8_t*)Gaus5x5;
            kernel_div = Gaus5x5Div;
        }

        float* G_ = (float*)calloc(Width * Height, sizeof(double));
        float* M_ = (float*)calloc(Width * Height, sizeof(double));
        unsigned char* s_ = (unsigned char*)calloc(Width * Height, sizeof(unsigned char));

        // gaussian filter
        for (int x = 0; x < Width; x++) {
            for (int y = 0; y < Height; y++) {
                int pos = x + (y * Width);
                if (x < offset_xy || x >= (Width - offset_xy) || y < offset_xy || y >= (Height - offset_xy)) {
                    // The first and last rows, the first and last columns, get the source value
                    Output[pos] = Input[pos];
                    continue;
                }
                int convolve = 0;
                int k = 0;
                // Calculate convolution using relevant methods
                for (int kx = -offset_xy; kx <= offset_xy; kx++) {
                    for (int ky = -offset_xy; ky <= offset_xy; ky++) {
                        convolve += (Input[pos + (kx + (ky * Width))] * kernel[k]);
                        k++;
                    }
                }

                Output[pos] = (unsigned char)((double)convolve / (double)kernel_div);
            }
        }

        // apply sobel kernels
        offset_xy = 1; // 3x3
        for (int x = offset_xy; x < Width - offset_xy; x++) {
            for (int y = offset_xy; y < Height - offset_xy; y++) {
                // The first and last rows, the first and last columns are skipped and not processed.
                double convolve_X = 0.0;
                double convolve_Y = 0.0;
                int k = 0;
                int src_pos = x + (y * Width);

                for (int ky = -offset_xy; ky <= offset_xy; ky++) {
                    for (int kx = -offset_xy; kx <= offset_xy; kx++) {
                        convolve_X += Output[src_pos + (kx + (ky * Width))] * Gx[k];
                        convolve_Y += Output[src_pos + (kx + (ky * Width))] * Gy[k];

                        k++;
                    }
                }

                // gradient hypot & direction
                int segment = 0;

                if (convolve_X == 0.0 || convolve_Y == 0.0) {
                    G_[src_pos] = 0;
                } else {
                    // calculate the intensity
                    G_[src_pos] = ((sqrt((convolve_X * convolve_X) + (convolve_Y * convolve_Y))));
                    // direction
                    double theta = atan2(convolve_Y, convolve_X); // radians. atan2 range: -PI,+PI,
                                                                  // theta : 0 - 2PI
                    // convert to degrees
                    theta = theta * (360.0 / (2.0 * M_PI)); // degrees

                    if ((theta <= 22.5 && theta >= -22.5) || (theta <= -157.5) || (theta >= 157.5)) {
                        // close the horizontal line
                        segment = 1; // "-"
                    } else if ((theta > 22.5 && theta <= 67.5) || (theta > -157.5 && theta <= -112.5)) {
                        // close to the 45 degree diagonal
                        segment = 2; // "/"
                    } else if ((theta > 67.5 && theta <= 112.5) || (theta >= -112.5 && theta < -67.5)) {
                        // close vertical line
                        segment = 3; // "|"
                    } else if ((theta >= -67.5 && theta < -22.5) || (theta > 112.5 && theta < 157.5)) {
                        // another diagonal line close to 135 degrees
                        segment = 4; // "\"
                    } else {
                        // std::cout << "error " << theta << std::endl;
                    }
                }

                s_[src_pos] = (unsigned char)segment;
            }
        }

        // The 9-square grid range is set to 0 if it is not the largest in the direction.
        // local maxima: non maxima suppression
        memcpy(M_, G_, Width * Height * sizeof(double));

        for (int x = 1; x < Width - 1; x++) {
            for (int y = 1; y < Height - 1; y++) {
                // The first and last rows, the first and last columns are skipped and not processed.
                int pos = x + (y * Width);

                switch (s_[pos]) {
                case 1:
                    // close to the horizontal line
                    if (G_[pos - 1] >= G_[pos] || G_[pos + 1] > G_[pos]) {
                        // If there is one on the left and right that is larger than itself, set it to 0
                        M_[pos] = 0;
                    }
                    break;
                case 2:
                    // close to the 45 degree diagonal
                    if (G_[pos - (Width - 1)] >= G_[pos] || G_[pos + (Width - 1)] > G_[pos]) {
                        M_[pos] = 0;
                    }
                    break;
                case 3:
                    // close to vertical line
                    if (G_[pos - (Width)] >= G_[pos] || G_[pos + (Width)] > G_[pos]) {
                        // If there is one above or below that is larger than itself, set it to 0
                        M_[pos] = 0;
                    }
                    break;
                case 4:
                    // another diagonal line close to 135 degrees
                    if (G_[pos - (Width + 1)] >= G_[pos] || G_[pos + (Width + 1)] > G_[pos]) {
                        M_[pos] = 0;
                    }
                    break;
                default: M_[pos] = 0; break;
                }
            }
        }


        // double threshold
        for (int x = 0; x < Width; x++) {
            for (int y = 0; y < Height; y++) {
                int src_pos = x + (y * Width);
                if (M_[src_pos] > strong_threshold) {
                    Output[src_pos] = 255;
                } else if (M_[src_pos] > weak_threshold) {
                    Output[src_pos] = 100;
                } else {
                    Output[src_pos] = 0;
                }
            }
        }

        // Connect the border
        // edges with hysteresis
        for (int x = 1; x < Width - 1; x++) {
            for (int y = 1; y < Height - 1; y++) {
                // The first and last rows, the first and last columns are skipped and not processed.
                int src_pos = x + (y * Width);
                if (Output[src_pos] == 255) {
                    Output[src_pos] = 255;
                } else if (Output[src_pos] == 100) {
                    if (Output[src_pos - 1] == 255 || Output[src_pos + 1] == 255 || Output[src_pos - 1 - Width] == 255 ||
                        Output[src_pos + 1 - Width] == 255 || Output[src_pos + Width] == 255 || Output[src_pos + Width - 1] == 255 ||
                        Output[src_pos + Width + 1] == 255) {
                        Output[src_pos] = 255;
                        // In the 9-square grid range of 100, if there is 255, adjust it to 255
                    } else {
                        // Otherwise return to 0
                        Output[src_pos] = 0;
                    }
                } else {
                    Output[src_pos] = 0;
                }
            }
        }

        free(G_);
        free(M_);
        free(s_);

        return OC_STATUS_OK;
    }

    OC_STATUS ocularSobelEdgeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels) {

        if ((Input == NULL) || (Output == NULL)) {
            return OC_STATUS_ERR_NULLREFERENCE;
        }
        if ((Width <= 0) || (Height <= 0)) {
            return OC_STATUS_ERR_INVALIDPARAMETER;
        }

        if (Channels != 1) {
            return OC_STATUS_ERR_NOTSUPPORTED;
        }

        unsigned char* SqrLut = (unsigned char*)malloc(65026 * sizeof(unsigned char));
        if (SqrLut == NULL) {
            return OC_STATUS_ERR_OUTOFMEMORY;
        }
        unsigned char* RowCopy = (unsigned char*)malloc((Width + 2) * 3 * sizeof(unsigned char));
        if (RowCopy == NULL) {
            return OC_STATUS_ERR_OUTOFMEMORY;
        }

        unsigned char *First = RowCopy, *Second = RowCopy + (Width + 2), *Third = RowCopy + (Width + 2) * 2;

        for (int Y = 0; Y < 65026; Y++)
            SqrLut[Y] = (unsigned char)ClampToByte((sqrtf(Y + 0.0f) + 0.5f));

        memcpy(Second, Input, 1);
        memcpy(Second + 1, Input, Width);
        memcpy(Second + Width + 1, Input + Width - 1, 1);

        memcpy(First, Second, Width + 2);

        memcpy(Third, Input + Width, 1);
        memcpy(Third + 1, Input + Width, Width);
        memcpy(Third + Width + 1, Input + Width + Width - 1, 1);

        for (int Y = 0; Y < Height; Y++) {
            unsigned char* LinePD = Output + Y * Width;
            if (Y != 0) {
                unsigned char* Temp = First;
                First = Second;
                Second = Third;
                Third = Temp;
            }
            if (Y == Height - 1) {
                memcpy(Third, Second, Width + 2);
            } else {
                memcpy(Third, Input + (Y + 1) * Width, 1);
                memcpy(Third + 1, Input + (Y + 1) * Width, Width);
                memcpy(Third + Width + 1, Input + (Y + 1) * Width + Width - 1, 1);
            }
            for (int X = 0; X < Width; X++) {
                int Hori = First[X] + 2 * First[X + 1] + First[X + 3] - (Third[X] + 2 * Third[X + 1] + Third[X + 2]);
                int Vert = First[X] + 2 * Second[X] + Third[X] - (First[X + 2] + 2 * Second[X + 2] + Third[X + 2]);
                int Value = Hori * Hori + Vert * Vert;
                LinePD[X] = SqrLut[min(Value, 65025)];
            }
        }
        if (RowCopy) {
            free(RowCopy);
        }
        if (SqrLut) {
            free(SqrLut);
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularGradientEdgeDetect(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels) {

        if ((Input == NULL) || (Output == NULL)) {
            return OC_STATUS_ERR_NULLREFERENCE;
        }
        if ((Width <= 0) || (Height <= 0)) {
            return OC_STATUS_ERR_INVALIDPARAMETER;
        }

        if (Channels != 1) {
            return OC_STATUS_ERR_NOTSUPPORTED;
        }

        unsigned char* RowCopy = (unsigned char*)malloc((Width + 2) * 3 * Channels);
        if (RowCopy == NULL) {
            return OC_STATUS_ERR_OUTOFMEMORY;
        }
        unsigned char* SqrValue = (unsigned char*)malloc(65026 * sizeof(unsigned char));
        if (SqrValue == NULL) {
            return OC_STATUS_ERR_OUTOFMEMORY;
        }
        unsigned char* First = RowCopy;
        unsigned char* Second = RowCopy + (Width + 2) * Channels;
        unsigned char* Third = RowCopy + (Width + 2) * 2 * Channels;

        for (int y = 0; y < 65026; y++)
            SqrValue[y] = (int)(sqrtf(y * 1.0) + 0.49999f);

        memcpy(Second, Input, Channels);
        memcpy(Second + Channels, Input, Width * Channels); //     Copy data to the middle position
        memcpy(Second + (Width + 1) * Channels, Input + (Width - 1) * Channels, Channels);

        memcpy(First, Second, (Width + 2) * Channels); //     The first row is the same as the second row

        memcpy(Third, Input + Width * Channels, Channels); //     Copy the second row of data
        memcpy(Third + Channels, Input + Width * Channels, Width * Channels);
        memcpy(Third + (Width + 1) * Channels, Input + Width * Channels + (Width - 1) * Channels, Channels);

        for (int y = 0; y < Height; y++) {
            unsigned char* LinePD = Output + y * Width;
            if (y != 0) {
                unsigned char* Temp = First;
                First = Second;
                Second = Third;
                Third = Temp;
            }
            if (y == Height - 1) {
                memcpy(Third, Second, (Width + 2) * Channels);
            } else {
                memcpy(Third, Input + (y + 1) * Width * Channels, Channels);
                memcpy(Third + Channels, Input + (y + 1) * Width * Channels,
                       Width * Channels); //     Since the data of the previous row is backed up, there is no problem even if Src and Dest are the same
                memcpy(Third + (Width + 1) * Channels, Input + (y + 1) * Width * Channels + (Width - 1) * Channels, Channels);
            }
            for (int x = 0; x < Width; x++) {
                int GradientH = 0, GradientV = 0;
                if (x == 0) {
                    GradientH = First[x + 0] + First[x + 1] + First[x + 2] - (Third[x + 0] + Third[x + 1] + Third[x + 2]);
                } else {
                    GradientH = GradientH - First[x - 1] + First[x + 2] + Third[x - 1] - Third[x + 2];
                }
                GradientV = First[x + 0] + Second[x + 0] * 2 + Third[x + 0] - (First[x + 2] + Second[x + 2] * 2 + Third[x + 2]);
                int Value = (GradientH * GradientH + GradientV * GradientV) >> 1;
                if (Value > 65025)
                    LinePD[x] = 255;
                else
                    LinePD[x] = SqrValue[Value];
            }
        }
        free(RowCopy);
        free(SqrValue);

        return OC_STATUS_OK;
    }

    OC_STATUS ocularHoughLineDetection(unsigned char* Input, int* LineNumber, struct LineParameter* DetectedLine, 
                                       int Height, int Width, int threshold) {

        if ((Input == NULL) || (LineNumber == NULL)) {
            return OC_STATUS_ERR_NULLREFERENCE;
        }

        // Ensure filter specific parameters are within valid ranges
        threshold = min(threshold, 1); // max number of lines to return

        float diagonalLength = sqrt((float)(Height * Height + Width * Width));
        float minAngle = 0;
        float maxAngle = 90;
        float angleIntervalCoarse = 2;
        float angleIntervalFine = 0.1;
        float distanceIntervalCoarse = 2;
        float distanceIntervalFine = 1;

        int numAnglesCoarse = (int)(maxAngle / angleIntervalCoarse) + 1;
        int numDistancesCoarse = (int)diagonalLength / distanceIntervalCoarse + 1;

        // build and initialize the vote tables for coarse angle and distance
        unsigned int** voteTableCoarse = (unsigned int**)malloc(numAnglesCoarse * sizeof(unsigned int*));
        if (voteTableCoarse == NULL) {
            return OC_STATUS_ERR_OUTOFMEMORY;
        }
        for (int i = 0; i < numAnglesCoarse; i++) {
            voteTableCoarse[i] = (unsigned int*)malloc(numDistancesCoarse * sizeof(unsigned int));
        }
        for (int i = 0; i < numAnglesCoarse; i++) {
            for (int j = 0; j < numDistancesCoarse; j++) {
                voteTableCoarse[i][j] = 0;
            }
        }

        houghTransformLine(Input, minAngle, angleIntervalCoarse, numAnglesCoarse, 0, distanceIntervalCoarse, numDistancesCoarse,
                           voteTableCoarse, Height, Width);

        int maxVote = 110;
        int m = 0;
        int n = 0; // voteTableCoarse[m][n] has the maximum votes.

        while (maxVote > threshold) {
            FindMaxVote(voteTableCoarse, numAnglesCoarse, numDistancesCoarse, &m, &n);
            maxVote = voteTableCoarse[m][n];
            float angleWithMaxVoteCoarse = minAngle + m * angleIntervalCoarse;
            float distanceWithMaxVoteCoarse = n * distanceIntervalCoarse;
            (*LineNumber)++;
            DetectedLine[(*LineNumber) - 1].angle = angleWithMaxVoteCoarse;
            DetectedLine[(*LineNumber) - 1].distance = distanceWithMaxVoteCoarse;

            if (n > 0) {
                voteTableCoarse[m][n - 1] = 0;
                if (m > 0) {
                    voteTableCoarse[m - 1][n - 1] = 0;
                }
                if (m < numAnglesCoarse - 1) {
                    voteTableCoarse[n + 1][n - 1] = 0;
                }
            }

            if (n < numDistancesCoarse - 1) {
                voteTableCoarse[m][n + 1] = 0;
                if (m > 0) {
                    voteTableCoarse[m - 1][n + 1] = 0;
                }
                if (m < numAnglesCoarse - 1) {
                    voteTableCoarse[m + 1][n + 1] = 0;
                }
            }

            if (m > 0) {
                voteTableCoarse[m - 1][n] = 0;
            }

            if (m < numAnglesCoarse - 1) {
                voteTableCoarse[m + 1][n] = 0;
            }
        }

        for (int i = 0; i < numAnglesCoarse; i++) {
            free(voteTableCoarse[i]);
        }

        free(voteTableCoarse);

        return OC_STATUS_OK;
    }

    OC_STATUS ocularDrawLine(unsigned char* canvas, int width, int height, int stride, int x1, int y1, int x2, int y2, 
                             unsigned char R, unsigned char G, unsigned char B) {

        if (canvas == NULL) {
            return OC_STATUS_ERR_NULLREFERENCE;
        }
        if (width <= 0 || height <= 0) {
            return OC_STATUS_ERR_INVALIDPARAMETER;
        }

        // Ensure parmeters are within valid ranges
        x1 = clamp(x1, 0, width - 1);
        x2 = clamp(x2, 0, width - 1);
        y1 = clamp(y1, 0, height - 1);
        y2 = clamp(y2, 0, height - 1);
        R = ClampToByte(R);
        G = ClampToByte(G);
        B = ClampToByte(B);

        int channels = stride / width;

        int xs, ys, xe, ye;
        if (x1 == x2) {
            if (y1 < y2) {
                ys = y1;
                ye = y2;
            } else {
                ys = y2;
                ye = y1;
            }
            unsigned char* Line = canvas + x1 * channels;
            for (int r = ys; r <= ye; r++) {
                unsigned char* curLine = Line + r * stride;

                curLine[0] = R;
                curLine[1] = G;
                curLine[2] = B;
            }
            return OC_STATUS_ERR_PARAMISMATCH;
        }

        float a = (float)(y2 - y1) / (x2 - x1);
        int nHeight = height;

        if ((a > -1) && (a < 1)) {
            if (x1 < x2) {
                xs = x1;
                xe = x2;
                ys = y1;
                ye = y2;
            } else {
                xs = x2;
                xe = x1;
                ys = y2;
                ye = y1;
            }
            for (int c = xs; c <= xe; c++) {
                unsigned char* Line = canvas + c * channels;
                int r = (int)(a * (c - xs) + ys + 0.5f);
                if (r < 0 || r >= nHeight)
                    continue;
                unsigned char* curLine = Line + r * stride;
                curLine[0] = R;
                curLine[1] = G;
                curLine[2] = B;
            }
        } else {
            float invA = 1.0f / a;
            if (y1 < y2) {
                ys = y1;
                ye = y2;
                xs = x1;
                xe = x2;
            } else {
                ys = y2;
                ye = y1;
                xs = x2;
                xe = x1;
            }
            for (int r = ys; r <= ye; r++) {
                int c = (int)(invA * (r - ys) + xs + 0.5f);
                unsigned char* Line = canvas + c * channels;
                if (r < 0 || r >= nHeight)
                    continue;
                unsigned char* curLine = Line + r * stride;
                curLine[0] = R;
                curLine[1] = G;
                curLine[2] = B;
            }
        }

        return OC_STATUS_OK;
    }

    bool ocularGetImageSize(const char* file_path, int* width, int* height, int* file_size) {

        // Based on work by Paulo Scardine
        // https://github.com/scardine/image_size

        bool has_image_size = false;
        *height = -1;
        *width = -1;
        *file_size = -1;
        FILE* fp = fopen(file_path, "rb");
        if (fp == NULL)
            return has_image_size;
        struct stat st;
        char sigBuf[26];
        if (fstat(fileno(fp), &st) < 0) {
            fclose(fp);
            return has_image_size;
        } else {
            *file_size = (int)st.st_size;
        }
        if (fread(&sigBuf, 26, 1, fp) < 1) {
            fclose(fp);
            return has_image_size;
        }
        const char* png_signature = "\211PNG\r\n\032\n";
        const char* ihdr_signature = "IHDR";
        const char* gif87_signature = "GIF87a";
        const char* gif89_signature = "GIF89a";
        const char* jpeg_signature = "\377\330";
        const char* bmp_signature = "BM";
        if ((*file_size >= 10) &&
            (memcmp(sigBuf, gif87_signature, strlen(gif87_signature)) == 0 || memcmp(sigBuf, gif89_signature, strlen(gif89_signature)) == 0)) {
            // image type: gif
            unsigned short* size_info = (unsigned short*)(sigBuf + 6);
            *width = size_info[0];
            *height = size_info[1];
            has_image_size = true;
        } else if ((*file_size >= 24) &&
                   (memcmp(sigBuf, png_signature, strlen(png_signature)) == 0 && memcmp(sigBuf + 12, ihdr_signature, strlen(ihdr_signature)) == 0)) {
            // image type:   png
            unsigned long* size_info = (unsigned long*)(sigBuf + 16);
            *width = (int)byteswap_ulong(size_info[0]);
            *height = (int)byteswap_ulong(size_info[1]);
            has_image_size = true;
        } else if ((*file_size >= 16) && (memcmp(sigBuf, png_signature, strlen(png_signature)) == 0)) {
            // image type: old png
            unsigned long* size_info = (unsigned long*)(sigBuf + 8);
            *width = (int)byteswap_ulong(size_info[0]);
            *height = (int)byteswap_ulong(size_info[1]);
            has_image_size = true;
        } else if ((*file_size >= 2) && (memcmp(sigBuf, jpeg_signature, strlen(jpeg_signature)) == 0)) {
            // image type: jpeg
            fseek(fp, 0, SEEK_SET);
            char b = 0;
            fread(&sigBuf, 2, 1, fp);
            fread(&b, 1, 1, fp);
            int w = -1;
            int h = -1;
            while (b && ((unsigned char)b & 0xff) != 0xDA) {
                while (((unsigned char)b & 0xff) != 0xFF) {
                    fread(&b, 1, 1, fp);
                }
                while (((unsigned char)b & 0xff) == 0xFF) {
                    fread(&b, 1, 1, fp);
                }
                if (((unsigned char)b & 0xff) >= 0xC0 && ((unsigned char)b & 0xff) <= 0xC3) {
                    fread(&sigBuf, 3, 1, fp);
                    fread(&sigBuf, 4, 1, fp);
                    unsigned short* size_info = (unsigned short*)(sigBuf);
                    h = byteswap_ushort(size_info[0]);
                    w = byteswap_ushort(size_info[1]);
                    break;
                } else {
                    unsigned short chunk_size = 0;
                    fread(&chunk_size, 2, 1, fp);
                    if (fseek(fp, byteswap_ushort(chunk_size) - 2, SEEK_CUR) != 0)
                        break;
                }
                fread(&b, 1, 1, fp);
            }
            if (w != -1 && h != -1) {
                *width = w;
                *height = h;
            }
            has_image_size = true;
        } else if ((*file_size >= 26) && (memcmp(sigBuf, bmp_signature, strlen(bmp_signature)) == 0)) {
            // image type: bmp
            unsigned int header_size = (unsigned int)(*(sigBuf + 14));
            if (header_size == 12) {
                unsigned short* size_info = (unsigned short*)(sigBuf + 18);
                *width = size_info[0];
                *height = size_info[1];
            } else if (header_size >= 40) {
                unsigned int* size_info = (unsigned int*)(sigBuf + 18);
                *width = size_info[0];
                *height = Abs((size_info[1]));
            }
            has_image_size = true;
        } else if (*file_size >= 2) {
            // image type: ico
            fseek(fp, 0, SEEK_SET);
            unsigned short format = 0;
            unsigned short reserved = 0;
            fread(&reserved, 2, 1, fp);
            fread(&format, 2, 1, fp);
            if (reserved == 0 && format == 1) {
                unsigned short num = 0;
                fread(&num, 2, 1, fp);
                if (num > 1) {
                    printf("this is a multi-ico file.");
                } else {
                    char w = 0, h = 0;
                    fread(&w, 1, 1, fp);
                    fread(&h, 1, 1, fp);
                    *width = (int)((unsigned char)w & 0xff);
                    *height = (int)((unsigned char)h & 0xff);
                }
            }
            has_image_size = true;
        }
        if (fp != NULL)
            fclose(fp);
        return has_image_size;
    }

    OC_STATUS ocularConvolution2DFilter(unsigned char* input, unsigned char* output, int width, int height, int channels, float* kernel,
                                        unsigned char filterW, unsigned char cfactor, unsigned char bias) {

        if (input == NULL || output == NULL || kernel == NULL) {
            return OC_STATUS_ERR_NULLREFERENCE;
        }
        if (width <= 0 || height <= 0) {
            return OC_STATUS_ERR_INVALIDPARAMETER;
        }

        // Ensure kernel size is odd
        if (filterW % 2 == 0) {
            return OC_STATUS_ERR_INVALIDPARAMETER;
        }

        // Ensure filter specific parameters are within valid ranges
        filterW = max(filterW, 3); // we should be using at least 3x3
        cfactor = clamp(cfactor, 1, 255);
        bias = clamp(bias, 0, 255);

        const int factor = 256 / cfactor;
        const int halfW = (filterW - 1) / 2;
        const int CHUNK_SIZE = 32; // Process image in chunks for better cache utilization

        // Pre-calculate kernel lookup table
        float* kernelLUT = (float*)malloc(filterW * filterW * sizeof(float));
        if (!kernelLUT)
            return OC_STATUS_ERR_OUTOFMEMORY;

        // Flatten and pre-multiply kernel values
        for (int i = 0; i < filterW * filterW; i++) {
            kernelLUT[i] = kernel[i] * factor;
        }

        if (channels == 3 || channels == 4) {
            // Process image in chunks
            for (int blockY = 0; blockY < height; blockY += CHUNK_SIZE) {
                for (int blockX = 0; blockX < width; blockX += CHUNK_SIZE) {
                    int endY = min(blockY + CHUNK_SIZE, height);
                    int endX = min(blockX + CHUNK_SIZE, width);

                    for (int y = blockY; y < endY; y++) {
                        unsigned char* outRow = output + (y * width + blockX) * channels;

                        for (int x = blockX; x < endX; x++) {
                            float sumR = 0, sumG = 0, sumB = 0;

                            // Unroll the kernel loops for common 3x3 case
                            if (filterW == 3) {
                                for (int ky = -1; ky <= 1; ky++) {
                                    int sy = (y + ky + height) % height;
                                    const unsigned char* inRow = input + (sy * width) * channels;

                                    for (int kx = -1; kx <= 1; kx++) {
                                        int sx = (x + kx + width) % width;
                                        const unsigned char* pixel = inRow + sx * channels;
                                        float k = kernelLUT[(ky + 1) * 3 + (kx + 1)];

                                        sumR += pixel[0] * k;
                                        sumG += pixel[1] * k;
                                        sumB += pixel[2] * k;
                                    }
                                }
                            } else {
                                // other kernel sizes
                                for (int ky = -halfW; ky <= halfW; ky++) {
                                    int sy = (y + ky + height) % height;
                                    const unsigned char* inRow = input + (sy * width) * channels;

                                    for (int kx = -halfW; kx <= halfW; kx++) {
                                        int sx = (x + kx + width) % width;
                                        const unsigned char* pixel = inRow + sx * channels;
                                        float k = kernelLUT[(ky + halfW) * filterW + (kx + halfW)];

                                        sumR += pixel[0] * k;
                                        sumG += pixel[1] * k;
                                        sumB += pixel[2] * k;
                                    }
                                }
                            }

                            // Write results with bias
                            outRow[0] = ClampToByte((int)(sumR / 256) + bias);
                            outRow[1] = ClampToByte((int)(sumG / 256) + bias);
                            outRow[2] = ClampToByte((int)(sumB / 256) + bias);
                            if (channels == 4) {
                                outRow[3] = input[(y * width + x) * channels + 3]; // Preserve alpha
                            }
                            outRow += channels;
                        }
                    }
                }
            }
        } else if (channels == 1) {
            for (int blockY = 0; blockY < height; blockY += CHUNK_SIZE) {
                for (int blockX = 0; blockX < width; blockX += CHUNK_SIZE) {
                    int endY = min(blockY + CHUNK_SIZE, height);
                    int endX = min(blockX + CHUNK_SIZE, width);

                    for (int y = blockY; y < endY; y++) {
                        unsigned char* outRow = output + y * width + blockX;

                        for (int x = blockX; x < endX; x++) {
                            float sum = 0;

                            if (filterW == 3) {
                                for (int ky = -1; ky <= 1; ky++) {
                                    int sy = (y + ky + height) % height;
                                    const unsigned char* inRow = input + sy * width;

                                    for (int kx = -1; kx <= 1; kx++) {
                                        int sx = (x + kx + width) % width;
                                        sum += inRow[sx] * kernelLUT[(ky + 1) * 3 + (kx + 1)];
                                    }
                                }
                            } else {
                                for (int ky = -halfW; ky <= halfW; ky++) {
                                    int sy = (y + ky + height) % height;
                                    const unsigned char* inRow = input + sy * width;

                                    for (int kx = -halfW; kx <= halfW; kx++) {
                                        int sx = (x + kx + width) % width;
                                        sum += inRow[sx] * kernelLUT[(ky + halfW) * filterW + (kx + halfW)];
                                    }
                                }
                            }

                            *outRow++ = ClampToByte((int)(sum / 256) + bias);
                        }
                    }
                }
            }
        }

        free(kernelLUT);
        return OC_STATUS_OK;
    }

    OC_STATUS ocularMotionBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                     int Distance, int Angle) {

        if (Input == NULL || Output == NULL) {
            return OC_STATUS_ERR_NULLREFERENCE;
        }
        if (Width <= 0 || Height <= 0 || Stride <= 0) {
            return OC_STATUS_ERR_INVALIDPARAMETER;
        }

        // Ensure filter specific parameters are within valid ranges
        Distance = max(Distance, 1); // Changed from min to max since we want at least 1 pixel distance
        Angle = clamp(Angle, -180, 180);

        int channels = Stride / Width;

        // Pre-calculate angle and direction
        float radian = ((float)(Angle % 360) + 180.0f) / 180.0f * M_PI;
        int dx = (int)((float)Distance * fastCos(radian) + 0.5f);
        int dy = (int)((float)Distance * fastSin(radian) + 0.5f);

        // Pre-calculate sign to avoid branching in loop
        int sign = (dx == 0) ? 0 : (dx < 0 ? -1 : 1);
        int absDistance = abs(dx);

        // Process image in chunks for better cache utilization
        const int CHUNK_SIZE = 32;
        for (int blockY = 0; blockY < Height; blockY += CHUNK_SIZE) {
            for (int blockX = 0; blockX < Width; blockX += CHUNK_SIZE) {
                int endY = min(blockY + CHUNK_SIZE, Height);
                int endX = min(blockX + CHUNK_SIZE, Width);

                // Pre-allocate arrays for accumulation
                int sums[4] = { 0 }; // Support up to 4 channels
                float invCount;

                for (int y = blockY; y < endY; y++) {
                    unsigned char* pInput = Input + (y * Stride);
                    unsigned char* pOutput = Output + (y * Stride);

                    for (int x = blockX; x < endX; x++) {
                        // Reset sums for new pixel
                        memset(sums, 0, sizeof(sums));
                        int count = 0;

                        // Vectorizable inner loop
                        for (int p = 0; p < absDistance; p++) {
                            int yOffset = y + p * sign;
                            int xOffset = x + p * sign;

                            if (yOffset >= 0 && yOffset < Height && xOffset >= 0 && xOffset < Width) {
                                unsigned char* pOffset = Input + (yOffset * Stride + xOffset * channels);
                                count++;

                                // Unrolled channel accumulation
                                switch (channels) {
                                case 4:
                                    sums[3] += pOffset[3];
                                    // fallthrough
                                case 3:
                                    sums[2] += pOffset[2];
                                    sums[1] += pOffset[1];
                                    sums[0] += pOffset[0];
                                    break;
                                case 1: sums[0] += pOffset[0]; break;
                                }
                            }
                        }

                        // Write output
                        if (count == 0) {
                            // Direct copy if no valid samples
                            memcpy(pOutput + x * channels, pInput + x * channels, channels);
                        } else {
                            // Pre-calculate inverse count for multiplication instead of division
                            invCount = 1.0f / count;
                            switch (channels) {
                            case 4:
                                pOutput[x * channels + 3] = ClampToByte(sums[3] * invCount + 0.5f);
                                // fallthrough
                            case 3:
                                pOutput[x * channels + 2] = ClampToByte(sums[2] * invCount + 0.5f);
                                pOutput[x * channels + 1] = ClampToByte(sums[1] * invCount + 0.5f);
                                pOutput[x * channels + 0] = ClampToByte(sums[0] * invCount + 0.5f);
                                break;
                            case 1: pOutput[x * channels] = ClampToByte(sums[0] * invCount + 0.5f); break;
                            }
                        }
                    }
                }
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularRadialBlur(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int centerX, int centerY, int intensity) {

        if (Input == NULL || Output == NULL) {
            return OC_STATUS_ERR_NULLREFERENCE;
        }
        if (Width <= 0 || Height <= 0 || Stride <= 0) {
            return OC_STATUS_ERR_INVALIDPARAMETER;
        }

        int channels = Stride / Width;
        if (channels != 1 && channels != 3) {
            return OC_STATUS_ERR_NOTSUPPORTED;
        }

        // Ensure filter specific parameters are within valid ranges
        centerX = clamp(centerX, 0, Width - 1);
        centerY = clamp(centerY, 0, Height - 1);
        intensity = clamp(intensity, 1, 100);

        // Pre-calculate angle increment and inverse intensity
        const float angleIncrement = 0.005f;
        const float invIntensity = 1.0f / intensity;

        // Process image in chunks for better cache utilization
        const int CHUNK_SIZE = 32;

        if (channels == 1) {
            for (int blockY = 0; blockY < Height; blockY += CHUNK_SIZE) {
                for (int blockX = 0; blockX < Width; blockX += CHUNK_SIZE) {
                    int endY = min(blockY + CHUNK_SIZE, Height);
                    int endX = min(blockX + CHUNK_SIZE, Width);

                    for (int y = blockY; y < endY; y++) {
                        unsigned char* pOut = Output + y * Stride;

                        for (int x = blockX; x < endX; x++) {
                            float dx = (float)(x - centerX);
                            float dy = (float)(y - centerY);
                            float distance = sqrtf(dx * dx + dy * dy);
                            float angle = atan2f(dy, dx);

                            float sum = 0.0f;

                            // Unroll the intensity loop for better performance
                            for (int n = 0; n < intensity; n += 4) {
                                // Process 4 samples at once
                                for (int k = 0; k < 4 && (n + k) < intensity; k++) {
                                    float curAngle = angle + (n + k) * angleIncrement;
                                    float cosAngle = fastCos(curAngle);
                                    float sinAngle = fastSin(curAngle);

                                    int newX = (int)(distance * cosAngle + centerX);
                                    int newY = (int)(distance * sinAngle + centerY);

                                    // Clamp coordinates
                                    newX = min(Width - 1, max(0, newX));
                                    newY = min(Height - 1, max(0, newY));

                                    sum += Input[newY * Stride + newX];
                                }
                            }

                            pOut[x] = ClampToByte(sum * invIntensity);
                        }
                    }
                }
            }
        } 
        if (channels == 3) {
            for (int blockY = 0; blockY < Height; blockY += CHUNK_SIZE) {
                for (int blockX = 0; blockX < Width; blockX += CHUNK_SIZE) {
                    int endY = min(blockY + CHUNK_SIZE, Height);
                    int endX = min(blockX + CHUNK_SIZE, Width);

                    float sums[3];

                    for (int y = blockY; y < endY; y++) {
                        unsigned char* pOut = Output + y * Stride;

                        for (int x = blockX; x < endX; x++) {
                            float dx = (float)(x - centerX);
                            float dy = (float)(y - centerY);
                            float distance = sqrtf(dx * dx + dy * dy);
                            float angle = atan2f(dy, dx);

                            sums[0] = sums[1] = sums[2] = 0.0f;

                            // Unroll the intensity loop
                            for (int n = 0; n < intensity; n += 4) {
                                // Process 4 samples at once
                                for (int k = 0; k < 4 && (n + k) < intensity; k++) {
                                    float curAngle = angle + (n + k) * angleIncrement;
                                    float cosAngle = fastCos(curAngle);
                                    float sinAngle = fastSin(curAngle);

                                    int newX = (int)(distance * cosAngle + centerX);
                                    int newY = (int)(distance * sinAngle + centerY);

                                    // Clamp coordinates
                                    newX = min(Width - 1, max(0, newX));
                                    newY = min(Height - 1, max(0, newY));

                                    const unsigned char* pIn = Input + (newY * Stride + newX * 3);
                                    sums[0] += pIn[0];
                                    sums[1] += pIn[1];
                                    sums[2] += pIn[2];
                                }
                            }

                            pOut[x * 3] = ClampToByte(sums[0] * invIntensity);
                            pOut[x * 3 + 1] = ClampToByte(sums[1] * invIntensity);
                            pOut[x * 3 + 2] = ClampToByte(sums[2] * invIntensity);
                        }
                    }
                }
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularZoomBlur(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                              int sampleRadius, float blurAmount, int centerX, int centerY) {

        if (Input == NULL || Output == NULL) {
            return OC_STATUS_ERR_NULLREFERENCE;
        }
        if (Width <= 0 || Height <= 0 || Stride <= 0) {
            return OC_STATUS_ERR_INVALIDPARAMETER;
        }

        // Ensure filter specific parameters are within valid ranges
        sampleRadius = clamp(sampleRadius, 10, 200);
        blurAmount = clamp(blurAmount, 0.1f, 1.0f);
        centerX = clamp(centerX, 0, Width - 1);
        centerY = clamp(centerY, 0, Height - 1);

        int Channels = Stride / Width;
        float maxDistance = sqrtf(Width * Width + Height * Height) / 2.0f;

        for (int y = 0; y < Height; y++) {
            for (int x = 0; x < Width; x++) {
                float dx = (float)(x - centerX);
                float dy = (float)(y - centerY);
                float distance = sqrtf(dx * dx + dy * dy);
                float strength = distance / maxDistance;

                float r = 0, g = 0, b = 0;
                float totalWeight = 0;

                int actualSamples = (int)(strength * sampleRadius);
                for (int sample = 0; sample <= actualSamples; sample++) {
                    float t = (float)sample / (float)actualSamples;
                    float weight = 1.0f - t;
                    float sampleX = x + dx * t * blurAmount;
                    float sampleY = y + dy * t * blurAmount;

                    // Clamp sample coordinates to image boundaries
                    int ix = (int)fmaxf(0, fminf(sampleX, Width - 1));
                    int iy = (int)fmaxf(0, fminf(sampleY, Height - 1));
                    int index = (iy * Stride) + (ix * Channels);

                    r += Input[index] * weight;
                    g += Input[index + 1] * weight;
                    b += Input[index + 2] * weight;
                    totalWeight += weight;
                }

                int outIndex = (y * Stride) + (x * Channels);
                if (totalWeight > 0) {
                    Output[outIndex] = (unsigned char)(r / totalWeight);
                    Output[outIndex + 1] = (unsigned char)(g / totalWeight);
                    Output[outIndex + 2] = (unsigned char)(b / totalWeight);
                } else {
                    Output[outIndex] = Input[outIndex];
                    Output[outIndex + 1] = Input[outIndex + 1];
                    Output[outIndex + 2] = Input[outIndex + 2];
                }

                if (Channels == 4) {
                    Output[outIndex + 3] = Input[outIndex + 3];
                }
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularAverageBlur(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius, OcEdgeMode edgeMode) {

        if (Input == NULL || Output == NULL) {
            return OC_STATUS_ERR_NULLREFERENCE;
        }
        if (Width <= 0 || Height <= 0 || Stride <= 0) {
            return OC_STATUS_ERR_INVALIDPARAMETER;
        }

        int channels = Stride / Width;

        if (channels != 3) {
            return OC_STATUS_ERR_NOTSUPPORTED;
        }

        // Ensure filter specific parameters are within valid ranges
        Radius = max(Radius, 1);
        if (edgeMode != OC_EDGE_WRAP && edgeMode != OC_EDGE_MIRROR) {
            edgeMode = OC_EDGE_WRAP;
        }

        int kernelSize = 2 * Radius + 1;
        int count = kernelSize * kernelSize;

        for (int y = 0; y < Height; y++) {
            for (int x = 0; x < Width; x++) {
                int sumR = 0, sumG = 0, sumB = 0;

                // Center the kernel around current pixel
                for (int dy = -Radius; dy <= Radius; dy++) {
                    for (int dx = -Radius; dx <= Radius; dx++) {
                        int xOffset = x + dx;
                        int yOffset = y + dy;

                        // Apply edge handling mode
                        if (edgeMode == OC_EDGE_WRAP) {
                            // Wrap around edges
                            xOffset = (xOffset + Width) % Width;
                            yOffset = (yOffset + Height) % Height;
                        } else if (edgeMode == OC_EDGE_MIRROR) {
                            // Mirror at edges
                            if (xOffset < 0) {
                                xOffset = -xOffset;
                            } else if (xOffset >= Width) {
                                xOffset = 2 * Width - xOffset - 2;
                            }

                            if (yOffset < 0) {
                                yOffset = -yOffset;
                            } else if (yOffset >= Height) {
                                yOffset = 2 * Height - yOffset - 2;
                            }
                        }

                        int pPos = (yOffset * Width + xOffset) * channels;
                        sumR += Input[pPos + 0];
                        sumG += Input[pPos + 1];
                        sumB += Input[pPos + 2];
                    }
                }

                int outPos = (y * Width + x) * channels;
                Output[outPos + 0] = ClampToByte(sumR / count);
                Output[outPos + 1] = ClampToByte(sumG / count);
                Output[outPos + 2] = ClampToByte(sumB / count);
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularMedianBlur(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius) {
        
        if (Input == NULL || Output == NULL) {
            return OC_STATUS_ERR_NULLREFERENCE;
        }
        if (Width <= 0 || Height <= 0 || Stride <= 0) {
            return OC_STATUS_ERR_INVALIDPARAMETER;
        }

        int Channel = Stride / Width;
        if ((Channel != 1) && (Channel != 3)) {
            return OC_STATUS_ERR_NOTSUPPORTED;
        }

        // Ensure filter specific parameters are within valid ranges
        Radius = min(Radius, 0);

        const int Level = 256;

        if (Channel == 1) {

            int* histogram = (int*)malloc(Level * sizeof(int));
            if (histogram == NULL) {
                return OC_STATUS_ERR_OUTOFMEMORY;
            }
            for (int Y = 0; Y < Height; Y++) {
                unsigned char* LinePS = Input + Y * Stride;
                unsigned char* LinePD = Output + Y * Stride;
                memset(histogram, 0, Level * sizeof(int)); // all assigned values are 0
                int CutPoint = -1;
                int Balance = 0;

                for (int J = max(Y - Radius, 0); J <= min(Y + Radius, Height - 1); J++) {
                    for (int I = max(0 - Radius, 0); I <= min(0 + Radius, Width - 1); I++) {
                        int Value = Input[J * Stride + I];
                        // Calculate the two-dimensional histogram of the first point in each row.
                        // The horizontal direction of the histogram is the Feature coordinate and
                        // the vertical direction is the Value coordinate.
                        histogram[Value]++;
                        Balance--;
                    }
                }
                for (int X = 0; X < Width; X++) {

                    if (Balance < 0) // the balance of the first point must be less than 0
                    {
                        for (; Balance < 0 && CutPoint != Level - 1; CutPoint++) {
                            Balance += 2 * histogram[CutPoint + 1];
                        }
                    } else if (Balance > 0) // If the balance value is greater than 0, move the middle value to the left
                    {
                        for (; Balance > 0 && CutPoint != 0; CutPoint--) {
                            Balance -= 2 * histogram[CutPoint];
                        }
                    }
                    LinePD[X] = CutPoint;
                    if ((X - Radius) >= 0) {
                        for (int J = max(Y - Radius, 0); J <= min(Y + Radius, Height - 1); J++) // the column of data to be moved out
                        {
                            int Value = Input[J * Stride + X - Radius];
                            histogram[Value]--;
                            if (Value <= CutPoint) // If the value moved out is less than the current median value
                                Balance--;
                            else
                                Balance++;
                        }
                    }
                    if ((X + Radius + 1) <= Width - 1) {
                        for (int J = max(Y - Radius, 0); J <= min(Y + Radius, Height - 1); J++) {
                            int Value = Input[J * Stride + X + Radius + 1];
                            histogram[Value]++;
                            if (Value <= CutPoint) // If the value moved out is less than the current median value
                                Balance++;
                            else
                                Balance--;
                        }
                    }
                }
            }
            free(histogram);
        } else {
            unsigned char* SrcR = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
            unsigned char* SrcG = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
            unsigned char* SrcB = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));

            unsigned char* DstR = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
            unsigned char* DstG = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
            unsigned char* DstB = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));

            SplitRGB(Input, SrcB, SrcG, SrcR, Width, Height, Stride);

            // TODO: Investigate using parallel processing here
            ocularMedianBlur(SrcR, DstR, Width, Height, Width, Radius);
            ocularMedianBlur(SrcG, DstG, Width, Height, Width, Radius);
            ocularMedianBlur(SrcB, DstB, Width, Height, Width, Radius);

            CombineRGB(DstB, DstG, DstR, Output, Width, Height, Stride);

            free(SrcR);
            free(SrcG);
            free(SrcB);
            free(DstR);
            free(DstG);
            free(DstB);
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularErodeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius) {

        if (Input == NULL || Output == NULL) {
            return OC_STATUS_ERR_NULLREFERENCE;
        }
        if (Width <= 0 || Height <= 0 || Stride <= 0) {
            return OC_STATUS_ERR_INVALIDPARAMETER;
        }

        // Ensure filter specific parameters are within valid ranges
        Radius = min(Radius, 0);

        int Channels = Stride / Width;

        if (Channels == 1) {
            for (int y = 0; y < Height; y++) {
                for (int x = 0; x < Width; x++) {
                    int minPixelValue = 255;
                    for (int ky = -Radius; ky <= Radius; ky++) {
                        for (int kx = -Radius; kx <= Radius; kx++) {
                            int offsetX = x + kx;
                            int offsetY = y + ky;
                            if (offsetX >= 0 && offsetX < Width && offsetY >= 0 && offsetY < Height) {
                                int pixelValue = Input[offsetY * Width + offsetX];
                                if (pixelValue < minPixelValue) {
                                    minPixelValue = pixelValue;
                                }
                            }
                        }
                    }
                    for (int c = 0; c < Channels; c++) {
                        Output[y * Width + x + c] = minPixelValue;
                    }
                }
            }
        }
        if (Channels == 3) {

            unsigned char* SrcR = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
            unsigned char* SrcG = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
            unsigned char* SrcB = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));

            unsigned char* DstR = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
            unsigned char* DstG = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
            unsigned char* DstB = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));

            SplitRGB(Input, SrcB, SrcG, SrcR, Width, Height, Stride);

            ocularErodeFilter(SrcR, DstR, Width, Height, Width, Radius);
            ocularErodeFilter(SrcG, DstG, Width, Height, Width, Radius);
            ocularErodeFilter(SrcB, DstB, Width, Height, Width, Radius);

            CombineRGB(DstB, DstG, DstR, Output, Width, Height, Stride);

            free(SrcR);
            free(SrcG);
            free(SrcB);
            free(DstR);
            free(DstG);
            free(DstB);
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularDilateFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius) {

        if (Input == NULL || Output == NULL) {
            return OC_STATUS_ERR_NULLREFERENCE;
        }
        if (Width <= 0 || Height <= 0 || Stride <= 0) {
            return OC_STATUS_ERR_INVALIDPARAMETER;
        }

        // Ensure filter specific parameters are within valid ranges
        Radius = min(Radius, 0);

        int Channels = Stride / Width;

        if (Channels == 1) {
            for (int y = 0; y < Height; y++) {
                for (int x = 0; x < Width; x++) {
                    int maxPixelValue = 0;
                    for (int ky = -Radius; ky <= Radius; ky++) {
                        for (int kx = -Radius; kx <= Radius; kx++) {
                            int offsetX = x + kx;
                            int offsetY = y + ky;
                            if (offsetX >= 0 && offsetX < Width && offsetY >= 0 && offsetY < Height) {
                                int pixelValue = Input[offsetY * Width + offsetX];
                                if (pixelValue > maxPixelValue) {
                                    maxPixelValue = pixelValue;
                                }
                            }
                        }
                    }
                    for (int c = 0; c < Channels; c++) {
                        Output[y * Width + x + c] = maxPixelValue;
                    }
                }
            }
        }
        if (Channels == 3) {

            unsigned char* SrcR = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
            unsigned char* SrcG = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
            unsigned char* SrcB = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));

            unsigned char* DstR = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
            unsigned char* DstG = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
            unsigned char* DstB = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));

            SplitRGB(Input, SrcB, SrcG, SrcR, Width, Height, Stride);

            ocularDilateFilter(SrcR, DstR, Width, Height, Width, Radius);
            ocularDilateFilter(SrcG, DstG, Width, Height, Width, Radius);
            ocularDilateFilter(SrcB, DstB, Width, Height, Width, Radius);

            CombineRGB(DstB, DstG, DstR, Output, Width, Height, Stride);

            free(SrcR);
            free(SrcG);
            free(SrcB);
            free(DstR);
            free(DstG);
            free(DstB);
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularMinFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius) {
        
        if (Input == NULL || Output == NULL) {
            return OC_STATUS_ERR_NULLREFERENCE;
        }
        if (Width <= 0 || Height <= 0 || Stride <= 0) {
            return OC_STATUS_ERR_INVALIDPARAMETER;
        }
        
        int Channels = Stride / Width;
        if (Channels != 1 && Channels != 3 && Channels != 4) {
            return OC_STATUS_ERR_NOTSUPPORTED;
        }

        // Ensure filter specific parameters are within valid ranges
        Radius = min(Radius, 0);

        // Pre-calculate circle mask
        const int maskSize = (Radius * 2 + 1);
        char* circleMask = (char*)malloc(maskSize * maskSize);
        int radiusSquared = Radius * Radius;

        for (int y = 0; y < maskSize; y++) {
            for (int x = 0; x < maskSize; x++) {
                int dx = x - Radius;
                int dy = y - Radius;
                circleMask[y * maskSize + x] = (dx * dx + dy * dy <= radiusSquared);
            }
        }

        // Process image in chunks for better cache utilization
        const int CHUNK_SIZE = 32;
        for (int blockY = 0; blockY < Height; blockY += CHUNK_SIZE) {
            for (int blockX = 0; blockX < Width; blockX += CHUNK_SIZE) {
                int endY = min(blockY + CHUNK_SIZE, Height);
                int endX = min(blockX + CHUNK_SIZE, Width);

                for (int y = blockY; y < endY; y++) {
                    unsigned char* pOutput = Output + (y * Stride) + (blockX * Channels);

                    for (int x = blockX; x < endX; x++) {
                        unsigned char minValues[4] = { 255, 255, 255, 255 };

                        // Scan only the pixels within the circle mask
                        for (int ky = 0; ky < maskSize; ky++) {
                            int ny = y + ky - Radius;
                            if (ny < 0 || ny >= Height)
                                continue;

                            unsigned char* pInput = Input + (ny * Stride);

                            for (int kx = 0; kx < maskSize; kx++) {
                                if (!circleMask[ky * maskSize + kx])
                                    continue;

                                int nx = x + kx - Radius;
                                if (nx < 0 || nx >= Width)
                                    continue;

                                unsigned char* pixel = pInput + (nx * Channels);

                                // Unrolled channel comparison
                                if (Channels >= 1 && pixel[0] < minValues[0])
                                    minValues[0] = pixel[0];
                                if (Channels >= 2 && pixel[1] < minValues[1])
                                    minValues[1] = pixel[1];
                                if (Channels >= 3 && pixel[2] < minValues[2])
                                    minValues[2] = pixel[2];
                                if (Channels == 4 && pixel[3] < minValues[3])
                                    minValues[3] = pixel[3];
                            }
                        }

                        for (int c = 0; c < Channels; c++) {
                            pOutput[c] = minValues[c];
                        }
                        pOutput += Channels;
                    }
                }
            }
        }

        free(circleMask);

        return OC_STATUS_OK;
    }

    OC_STATUS ocularMaxFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius) {

        if (Input == NULL || Output == NULL) {
            return OC_STATUS_ERR_NULLREFERENCE;
        }
        if (Width <= 0 || Height <= 0 || Stride <= 0) {
            return OC_STATUS_ERR_INVALIDPARAMETER;
        }

        int Channels = Stride / Width;
        if (Channels != 1 && Channels != 3 && Channels != 4) {
            return OC_STATUS_ERR_NOTSUPPORTED;
        };

        // Ensure filter specific parameters are within valid ranges
        Radius = min(Radius, 0);

        // Pre-calculate circle mask
        const int maskSize = (Radius * 2 + 1);
        char* circleMask = (char*)malloc(maskSize * maskSize);
        int radiusSquared = Radius * Radius;

        for (int y = 0; y < maskSize; y++) {
            for (int x = 0; x < maskSize; x++) {
                int dx = x - Radius;
                int dy = y - Radius;
                circleMask[y * maskSize + x] = (dx * dx + dy * dy <= radiusSquared);
            }
        }

        // Process image in chunks for better cache utilization
        const int CHUNK_SIZE = 32;
        for (int blockY = 0; blockY < Height; blockY += CHUNK_SIZE) {
            for (int blockX = 0; blockX < Width; blockX += CHUNK_SIZE) {
                int endY = min(blockY + CHUNK_SIZE, Height);
                int endX = min(blockX + CHUNK_SIZE, Width);

                for (int y = blockY; y < endY; y++) {
                    unsigned char* pOutput = Output + (y * Stride) + (blockX * Channels);

                    for (int x = blockX; x < endX; x++) {
                        unsigned char maxValues[4] = { 0, 0, 0, 0 };

                        // Scan only the pixels within the circle mask
                        for (int ky = 0; ky < maskSize; ky++) {
                            int ny = y + ky - Radius;
                            if (ny < 0 || ny >= Height)
                                continue;

                            unsigned char* pInput = Input + (ny * Stride);

                            for (int kx = 0; kx < maskSize; kx++) {
                                if (!circleMask[ky * maskSize + kx])
                                    continue;

                                int nx = x + kx - Radius;
                                if (nx < 0 || nx >= Width)
                                    continue;

                                // Use pointer arithmetic for faster access
                                unsigned char* pixel = pInput + (nx * Channels);

                                // Unrolled channel comparison
                                if (Channels >= 1 && pixel[0] > maxValues[0])
                                    maxValues[0] = pixel[0];
                                if (Channels >= 2 && pixel[1] > maxValues[1])
                                    maxValues[1] = pixel[1];
                                if (Channels >= 3 && pixel[2] > maxValues[2])
                                    maxValues[2] = pixel[2];
                                if (Channels == 4 && pixel[3] > maxValues[3])
                                    maxValues[3] = pixel[3];
                            }
                        }

                        // Write output values
                        for (int c = 0; c < Channels; c++) {
                            pOutput[c] = maxValues[c];
                        }
                        pOutput += Channels;
                    }
                }
            }
        }

        free(circleMask);

        return OC_STATUS_OK;
    }

    OC_STATUS ocularHighPassFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius) {

        if (Input == NULL || Output == NULL) {
            return OC_STATUS_ERR_NULLREFERENCE;
        }
        if (Width <= 0 || Height <= 0 || Stride <= 0) {
            return OC_STATUS_ERR_INVALIDPARAMETER;
        }

        int Channels = Stride / Width;
        if (Channels != 1 && Channels != 3 && Channels != 4) {
            return OC_STATUS_ERR_NOTSUPPORTED;
        }

        // Ensure filter specific parameters are within valid ranges
        Radius = min(Radius, 0);

        // Create temporary buffer for blur result
        unsigned char* blurBuffer = (unsigned char*)malloc(Width * Height * Stride);
        if (blurBuffer == NULL) {
            return OC_STATUS_ERR_OUTOFMEMORY;
        }

        // First apply Gaussian blur to get low frequency components
        ocularGaussianBlurFilter(Input, blurBuffer, Width, Height, Stride, Radius);

        // Subtract blurred image from original to get high frequency components
        for (int y = 0; y < Height; y++) {
            unsigned char* pInput = Input + (y * Stride);
            unsigned char* pBlur = blurBuffer + (y * Stride);
            unsigned char* pOutput = Output + (y * Stride);

            for (int x = 0; x < Width; x++) {
                for (int c = 0; c < (Channels == 4 ? 3 : Channels); c++) {
                    // High pass = Original - Low pass (blur)
                    // Add 128 to center the result around middle gray
                    int highPass = 128 + (pInput[c] - pBlur[c]);
                    pOutput[c] = ClampToByte(highPass);
                }

                // Preserve alpha channel if it exists
                if (Channels == 4) {
                    pOutput[3] = pInput[3];
                }

                pInput += Channels;
                pBlur += Channels;
                pOutput += Channels;
            }
        }

        free(blurBuffer);

        return OC_STATUS_OK;
    }

    OC_STATUS ocularPixelateFilter(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                                  int blockSize) {

        if (Input == NULL || Output == NULL) {
            return OC_STATUS_ERR_NULLREFERENCE;
        }
        if (Width <= 0 || Height <= 0 || Stride <= 0) {
            return OC_STATUS_ERR_INVALIDPARAMETER;
        }

        int channels = Stride / Width;
        if (channels != 1 && channels != 3) {
            return OC_STATUS_ERR_NOTSUPPORTED;
        }

        // Ensure filter specific parameters are within valid ranges
        blockSize = min(blockSize, 1);

        int pPos;
        for (int y = 0; y < Height; y += blockSize) {
            for (int x = 0; x < Width; x += blockSize) {

                // Find the average color value of the pixels in a block
                int numPix = 0;
                int avg[3];
                int blockAvg[3];
                memset(avg, 0, sizeof avg);
                for (int dy = 0; dy < blockSize; dy++) {
                    for (int dx = 0; dx < blockSize; dx++) {
                        int xOffset = x + dx;
                        int yOffset = y + dy;

                        // If possible, add the pixel value to the average
                        if (yOffset < Height && xOffset < Width) {
                            pPos = xOffset * channels + yOffset * Stride;
                            for (int c = 0; c < channels; c++) {
                                avg[c] += Input[pPos + c];
                            }
                            numPix++;
                        }
                    }
                }
                for (int c = 0; c < channels; c++) {
                    blockAvg[c] = avg[c] / numPix;
                }

                for (int dy = 0; dy < blockSize; dy++) {
                    for (int dx = 0; dx < blockSize; dx++) {
                        int xOffset = x + dx;
                        int yOffset = y + dy;

                        // The pixel is the value of the upper left pixel in the block
                        if (yOffset < Height - 1 && xOffset < Width - 1) {
                            pPos = xOffset * channels + yOffset * Stride;
                            for (int c = 0; c < channels; c++) {
                                Output[pPos + c] = blockAvg[c];
                            }
                        }
                    }
                }
            }
        }

        return OC_STATUS_OK;
    }

    OC_STATUS ocularOilPaintFilter(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int radius, int intensity) {

        if (Input == NULL || Output == NULL) {
            return OC_STATUS_ERR_NULLREFERENCE;
        }
        if (Width <= 0 || Height <= 0 || Stride <= 0) {
            return OC_STATUS_ERR_INVALIDPARAMETER;
        }

        int Channels = Stride / Width;
        if (Channels != 3) {
            return OC_STATUS_ERR_NOTSUPPORTED;
        }

        // Ensure filter specific parameters are within valid ranges
        radius = clamp(radius, 1, 200);
        intensity = clamp(intensity, 1, 100);

        // Allocate arrays on heap to prevent stack overflow
        int* intensityCount = (int*)calloc(intensity, sizeof(int));
        int* sumR = (int*)calloc(intensity, sizeof(int));
        int* sumG = (int*)calloc(intensity, sizeof(int));
        int* sumB = (int*)calloc(intensity, sizeof(int));

        if (!intensityCount || !sumR || !sumG || !sumB) {
            free(intensityCount);
            free(sumR);
            free(sumG);
            free(sumB);
            return OC_STATUS_ERR_OUTOFMEMORY;
        }

        // Process each pixel
        for (int y = 0; y < Height; y++) {
            for (int x = 0; x < Width; x++) {
                // Reset arrays for new pixel
                memset(intensityCount, 0, intensity * sizeof(int));
                memset(sumR, 0, intensity * sizeof(int));
                memset(sumG, 0, intensity * sizeof(int));
                memset(sumB, 0, intensity * sizeof(int));

                // Sample the neighborhood
                for (int dy = -radius; dy <= radius; dy++) {
                    for (int dx = -radius; dx <= radius; dx++) {
                        int sampleX = x + dx;
                        int sampleY = y + dy;

                        // Mirror pixels at boundaries
                        if (sampleX < 0) {
                            sampleX = -sampleX;
                        } else if (sampleX >= Width) {
                            sampleX = 2 * Width - sampleX - 2;
                        }

                        if (sampleY < 0) {
                            sampleY = -sampleY;
                        } else if (sampleY >= Height) {
                            sampleY = 2 * Height - sampleY - 2;
                        }

                        // Get pixel color
                        int idx = (sampleY * Width + sampleX) * Channels;
                        int r = Input[idx];
                        int g = Input[idx + 1];
                        int b = Input[idx + 2];

                        // Calculate intensity level
                        int intensityLevel = ((r + g + b) / 3 * intensity) / 255;
                        intensityLevel = clamp(intensityLevel, 0, intensity - 1);

                        // Accumulate values
                        intensityCount[intensityLevel]++;
                        sumR[intensityLevel] += r;
                        sumG[intensityLevel] += g;
                        sumB[intensityLevel] += b;
                    }
                }

                // Find dominant intensity level
                int maxCount = intensityCount[0];
                int maxIndex = 0;
                for (int i = 1; i < intensity; i++) {
                    if (intensityCount[i] > maxCount) {
                        maxCount = intensityCount[i];
                        maxIndex = i;
                    }
                }

                // Write output pixel
                int outIdx = (y * Width + x) * Channels;
                if (maxCount > 0) {
                    Output[outIdx] = ClampToByte(sumR[maxIndex] / maxCount);
                    Output[outIdx + 1] = ClampToByte(sumG[maxIndex] / maxCount);
                    Output[outIdx + 2] = ClampToByte(sumB[maxIndex] / maxCount);
                } else {
                    // If no samples found, copy input pixel
                    Output[outIdx] = Input[outIdx];
                    Output[outIdx + 1] = Input[outIdx + 1];
                    Output[outIdx + 2] = Input[outIdx + 2];
                }
            }
        }

        // Clean up
        free(intensityCount);
        free(sumR);
        free(sumG);
        free(sumB);

        return OC_STATUS_OK;
    }

    OC_STATUS ocularFrostedGlassEffect(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                       int Radius, int Range) {

        if (Input == NULL || Output == NULL) {
            return OC_STATUS_ERR_NULLREFERENCE;
        }
        if (Width <= 0 || Height <= 0 || Stride <= 0) {
            return OC_STATUS_ERR_INVALIDPARAMETER;
        }

        // Ensure filter specific parameters are within valid ranges
        Radius = min(Radius, 0);
        Range = clamp(Range, 1, 20);

        int Channels = Stride / Width;

        // First the image is blurred by Gaussian filtering, then the blurred image is randomly sampled in the neighborhood,
        // giving the image a certain degree of random disturbance and blur.
        ocularGaussianBlurFilter(Input, Input, Width, Height, Stride, Radius);

        for (int y = 0; y < Height; y++) {
            for (int x = 0; x < Width; x++) {
                float randomOffsetX = (rand() / (float)RAND_MAX) - 0.5f;
                float randomOffsetY = (rand() / (float)RAND_MAX) - 0.5f;
                int offsetX = (int)(randomOffsetX * (Range * 2 - 1));
                int offsetY = (int)(randomOffsetY * (Range * 2 - 1));

                // Reflect pixels that are out of bounds
                int newY = y + offsetY;
                int newX = x + offsetX;

                if (newY < 0)
                    newY = -newY;
                if (newY >= Height)
                    newY = 2 * Height - newY - 2;
                if (newX < 0)
                    newX = -newX;
                if (newX >= Width)
                    newX = 2 * Width - newX - 2;

                int src_idx = (newY * Width + newX) * Channels;
                int dst_idx = (y * Width + x) * Channels;

                for (int c = 0; c < Channels; c++) {
                    Output[dst_idx + c] = Input[src_idx + c];
                }
            }
        }
    }

    OC_STATUS ocularFilmGrainEffect(unsigned char* input, unsigned char* output, int width, int height, int channels, 
                            float strength, float softness) {

        if (input == NULL || output == NULL) {
            return OC_STATUS_ERR_NULLREFERENCE;
        }
        if (width <= 0 || height <= 0 || channels <= 0) {
            return OC_STATUS_ERR_INVALIDPARAMETER;
        }

        // Clamp filter specific parameters to valid ranges
        strength = clamp(strength, 0.0f, 100.0f) * 0.5f; // Reduced strength impact
        softness = clamp(softness, 0.0f, 25.0f);

        // Pre-calculate noise values for better performance
        float* noiseValues = (float*)malloc(width * height * sizeof(float));
        for (int i = 0; i < width * height; i++) {
            noiseValues[i] = (float)(rand() % 256) / 255.0f;
        }

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                float noise = noiseValues[y * width + x];

                // Apply softness by averaging with neighboring noise (only if needed)
                if (softness > 0) {
                    float blur = 0;
                    int samples = 0;
                    int radius = (int)(softness / 4); // Reduced radius for better performance

                    for (int dy = -radius; dy <= radius; dy += 2) { // Skip pixels for speed
                        for (int dx = -radius; dx <= radius; dx += 2) {
                            int nx = x + dx;
                            int ny = y + dy;

                            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                                blur += noiseValues[ny * width + nx];
                                samples++;
                            }
                        }
                    }
                    noise = (noise + (blur / samples)) / 2.0f;
                }

                // Center the noise around 0 for neutral brightness
                float grainOffset = (noise - 0.5f) * (strength / 100.0f);

                // Apply to all channels
                int baseIdx = (y * width + x) * channels;
                for (int c = 0; c < channels; c++) {
                    float pixel = input[baseIdx + c];
                    // Add noise while preserving original brightness
                    pixel += grainOffset * 255.0f;
                    output[baseIdx + c] = (unsigned char)fmax(0, fmin(255, pixel));
                }
            }
        }

        free(noiseValues);
    }

    OC_STATUS ocularPalettetizeFromFile(unsigned char* input, unsigned char* output, int width, int height, int channels,
                                        const char* filename, OcDitherMethod method, int amount) {

        if (input == NULL || output == NULL) {
            return OC_STATUS_ERR_NULLREFERENCE;
        }
        if (access(filename, 0) == -1) {
            return OC_STATUS_ERR_FILENOTFOUND;
        }
        if (method < OC_DITHER_NONE || method > OC_DITHER_BAYER_8X8) {
            return OC_STATUS_ERR_INVALIDPARAMETER;
        }
        if (channels != 3 && channels != 4) {
            return OC_STATUS_ERR_NOTSUPPORTED;
        }
        amount = clamp(amount, 0, 100);

        OcPalette palette;
        OC_STATUS status = ocularLoadPalette(filename, &palette);
        if (status != OC_STATUS_OK) {
            ocularFreePalette(&palette);
            return status;
        }

        if (!applyDithering(input, output, width, height, channels, &palette, method, amount)) {
            ocularFreePalette(&palette);
            return OC_STATUS_ERR_UNKNOWN;
        }

        ocularFreePalette(&palette);

        return OC_STATUS_OK;
    };

    OC_STATUS ocularPalettetizeFromImage(unsigned char* input, unsigned char* output, int width, int height, int channels,
                                         OcQuantizeMethod quantizeMethod, int maxColors, OcDitherMethod ditherMethod, 
                                         int ditherAmount) {

        if (input == NULL || output == NULL) {
            return OC_STATUS_ERR_NULLREFERENCE;
        }
        if (width <= 0 || height <= 0) {
            return OC_STATUS_ERR_INVALIDPARAMETER;
        }
        if (channels != 3 && channels != 4) {
            return OC_STATUS_ERR_NOTSUPPORTED;
        }
        ditherAmount = clamp(ditherAmount, 0, 100);
        maxColors = clamp(maxColors, 1, 256);
        
        OcPalette palette;
        if (quantizeMethod == OC_QUANTIZE_MEDIAN_CUT) {
            if (!generateOptimalPaletteMedianCut(input, width, height, channels, maxColors, &palette)) {
                ocularFreePalette(&palette);
                return OC_STATUS_ERR_UNKNOWN;
            }
        } else if (quantizeMethod == OC_QUANTIZE_OCTREE) {
            if (!generateOptimalPaletteOctree(input, width, height, channels, maxColors, &palette)) {
                ocularFreePalette(&palette);
                return OC_STATUS_ERR_UNKNOWN;
            }
        } else {
            return OC_STATUS_ERR_INVALIDPARAMETER; // must be one of the above
        }

        if (!applyDithering(input, output, width, height, channels, &palette, ditherMethod, ditherAmount)) {
            ocularFreePalette(&palette);
            return OC_STATUS_ERR_UNKNOWN;
        }

        ocularFreePalette(&palette);

        return OC_STATUS_OK;
    }

    OC_STATUS ocularLoadPalette(const char* filename, OcPalette* palette) {

        PaletteFormat format = detect_palette_format(filename);
        switch (format) {
            case FORMAT_GIMP: 
                if (read_gimp_palette(filename, palette) != OC_STATUS_OK) {
                    return OC_STATUS_ERR_NOTSUPPORTED;
                }
                return OC_STATUS_OK;
            case FORMAT_RIFF: 
                if (read_riff_palette(filename, palette) != OC_STATUS_OK) {
                    return OC_STATUS_ERR_NOTSUPPORTED;
                }
                return OC_STATUS_OK;
            case FORMAT_ACO: 
                if (read_aco_palette(filename, palette) != OC_STATUS_OK) {
                    return OC_STATUS_ERR_NOTSUPPORTED;
                }
                return OC_STATUS_OK;
            case FORMAT_PAINTNET: 
                if (read_paintnet_palette(filename, palette) != OC_STATUS_OK) {
                    return OC_STATUS_ERR_NOTSUPPORTED;
                }
                return OC_STATUS_OK;
            case FORMAT_ACT: 
                if (read_act_palette(filename, palette) != OC_STATUS_OK) {
                    return OC_STATUS_ERR_NOTSUPPORTED;
                }
                return OC_STATUS_OK;
            case FORMAT_ASE: 
                if (read_ase_palette(filename, palette) != OC_STATUS_OK) {
                    return OC_STATUS_ERR_NOTSUPPORTED;
                }
                return OC_STATUS_OK;
            default: return OC_STATUS_ERR_NOTSUPPORTED;
        }
    }

    const char* ocularGetVersion() { return ocular_version; }

    const char* ocularGetStatusString(OC_STATUS status) {
        switch (status) {
            case OC_STATUS_ERR_OUTOFMEMORY: return "Memory allocation error";
            case OC_STATUS_ERR_STACKOVERFLOW: return "Stack overflow";
            case OC_STATUS_ERR_NULLREFERENCE: return "Empty reference provided";
            case OC_STATUS_ERR_INVALIDPARAMETER: return "Invalid parameter provided, empty or not within valid range";
            case OC_STATUS_ERR_PARAMISMATCH: return "Input and output sizes do not match";
            case OC_STATUS_ERR_INDEXOUTOFRANGE: return "Index out of range";
            case OC_STATUS_ERR_NOTSUPPORTED: return "Unsupported image format";
            case OC_STATUS_ERR_OVERFLOW: return "Overflow";
            case OC_STATUS_ERR_FILENOTFOUND: return "File not found";
            case OC_STATUS_ERR_UNKNOWN: return "Unknown error occurred";
            default: return "Unknown error";
        }
    }
#ifdef __cplusplus
}
#endif