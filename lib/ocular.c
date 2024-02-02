#include <math.h>
#include <string.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "ocular.h"

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

    void ocularGrayscaleFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride) {

        const int B_WT = (int)(0.114 * 256 + 0.5);
        const int G_WT = (int)(0.587 * 256 + 0.5);
        const int R_WT = 256 - B_WT - G_WT;
        int Channels = Stride / Width;
        if (Channels == 3) {
            for (int Y = 0; Y < Height; Y++) {
                unsigned char* LinePS = Input + Y * Stride;
                unsigned char* LinePD = Output + Y * Width;
                int X = 0;
                for (; X < Width - 4; X += 4, LinePS += Channels * 4) {
                    LinePD[X + 0] = (unsigned char)((B_WT * LinePS[0] + G_WT * LinePS[1] + R_WT * LinePS[2]) >> 8);
                    LinePD[X + 1] = (unsigned char)((B_WT * LinePS[3] + G_WT * LinePS[4] + R_WT * LinePS[5]) >> 8);
                    LinePD[X + 2] = (unsigned char)((B_WT * LinePS[6] + G_WT * LinePS[7] + R_WT * LinePS[8]) >> 8);
                    LinePD[X + 3] = (unsigned char)((B_WT * LinePS[9] + G_WT * LinePS[10] + R_WT * LinePS[11]) >> 8);
                }
                for (; X < Width; X++, LinePS += Channels) {
                    LinePD[X] = (unsigned char)(B_WT * LinePS[0] + G_WT * LinePS[1] + R_WT * LinePS[2]) >> 8;
                }
            }
        } else if (Channels == 4) {
            for (int Y = 0; Y < Height; Y++) {
                unsigned char* LinePS = Input + Y * Stride;
                unsigned char* LinePD = Output + Y * Stride;
                int X = 0;
                for (; X < Width - 4; X += 4, LinePS += Channels * 4) {
                    LinePD[X + 0] = (unsigned char)((B_WT * LinePS[0] + G_WT * LinePS[1] + R_WT * LinePS[2]) >> 8);
                    LinePD[X + 1] = (unsigned char)((B_WT * LinePS[4] + G_WT * LinePS[5] + R_WT * LinePS[6]) >> 8);
                    LinePD[X + 2] = (unsigned char)((B_WT * LinePS[8] + G_WT * LinePS[9] + R_WT * LinePS[10]) >> 8);
                    LinePD[X + 3] = (unsigned char)((B_WT * LinePS[12] + G_WT * LinePS[13] + R_WT * LinePS[14]) >> 8);
                }
                for (; X < Width; X++, LinePS += Channels) {
                    LinePD[X] = (unsigned char)((B_WT * LinePS[0] + G_WT * LinePS[1] + R_WT * LinePS[2]) >> 8);
                }
            }
        } else if (Channels == 1) {
            if (Output != Input) {
                memcpy(Output, Input, Height * Stride);
            }
        }
    }

    void ocularRGBFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float redAdjustment,
                         float greenAdjustment, float blueAdjustment) {

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
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                pOutput[0] = AdjustMapR[pInput[0]];
                pOutput[1] = AdjustMapG[pInput[1]];
                pOutput[2] = AdjustMapB[pInput[2]];
                pInput += Channels;
                pOutput += Channels;
            }
        }
    }

    void ocularAverageLuminanceThresholdFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float thresholdMultiplier) {

        int Channels = Stride / Width;
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
    }

    void ocularAverageColor(unsigned char* Input, int Width, int Height, int Stride, unsigned char* AverageR, unsigned char* AverageG,
                            unsigned char* AverageB, unsigned char* AverageA) {

        int Channels = Stride / Width;
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
    }

    void ocularLuminosity(unsigned char* Input, int Width, int Height, int Stride, unsigned char* Luminance) {

        int Channels = Stride / Width;
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
    }

    void ocularColorMatrixFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float* colorMatrix, float intensity) {

        int Channels = Stride / Width;
        if (Channels == 1)
            return;
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
    }

    void ocularSepiaFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int intensity) {

        int Channels = Stride / Width;
        if (Channels == 1)
            return;
        float fIntensity = intensity / 100.0f;

        float colorMatrix[4 * 4] = { 0.3588f, 0.7044f, 0.1368f, 0.0f, 0.2990f, 0.5870f, 0.1140f, 0.0f,
                                     0.2392f, 0.4696f, 0.0912f, 0.0f, 0.f,     0.f,     0.f,     1.f };
        ocularColorMatrixFilter(Input, Output, Width, Height, Stride, colorMatrix, fIntensity);
    }

    void ocularChromaKeyFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, unsigned char colorToReplaceR,
                               unsigned char colorToReplaceG, unsigned char colorToReplaceB, float thresholdSensitivity, float smoothing) {

        int Channels = Stride / Width;
        if (Channels == 1) {
            return;
        }

        unsigned char maskY = (unsigned char)((19589 * colorToReplaceR + 38443 * colorToReplaceG + 7504 * colorToReplaceB) >> 16);

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
    }

    void ocularLookupFilter(unsigned char* Input, unsigned char* Output, unsigned char* lookupTable, int Width, int Height, int Stride, int intensity) {

        int Channels = Stride / Width;
        if (Channels == 1)
            return;
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
                unsigned char* pLineLookup1 = &lookupTable[(((int)(quad1yMap[B] + green) << 9) + (int)(quad1xMap[B] + red)) * lookupChannels];
                unsigned char* pLineLookup2 = &lookupTable[(((int)(quad2yMap[B] + green) << 9) + (int)(quad2xMap[B] + red)) * lookupChannels];
                unsigned short fractB = fractMap[B];
                pOutput[0] = (unsigned char)((int)(R * c1 + ((*pLineLookup1++ * (256 - fractB) + *pLineLookup2++ * fractB) >> 8) * c2) >> 8);
                pOutput[1] = (unsigned char)((int)(G * c1 + ((*pLineLookup1++ * (256 - fractB) + *pLineLookup2++ * fractB) >> 8) * c2) >> 8);
                pOutput[2] = (unsigned char)((int)(B * c1 + ((*pLineLookup1++ * (256 - fractB) + *pLineLookup2++ * fractB) >> 8) * c2) >> 8);
                pInput += Channels;
                pOutput += Channels;
            }
        }
    }

    void ocularSaturationFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float saturation) {

        int Channels = Stride / Width;
        if (Channels == 1)
            return;
        unsigned char SaturationMap[256 * 256] = { 0 };
        for (int gray = 0; gray < 256; gray++) {
            unsigned char* pSaturationMap = SaturationMap + (gray << 8);
            for (int i = 0; i < 256; i++) {
                pSaturationMap[0] = (unsigned char)((mix_u8((unsigned char)gray, (unsigned char)i, saturation) + i) * 0.5f);
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
                unsigned char* pSaturationMap = SaturationMap + (((13926 * pInput[0] + 46884 * pInput[1] + 4725 * pInput[2]) >> 16) << 8);
                pOutput[0] = pSaturationMap[pInput[0]];
                pOutput[1] = pSaturationMap[pInput[1]];
                pOutput[2] = pSaturationMap[pInput[2]];
                pInput += Channels;
                pOutput += Channels;
            }
        }
    }

    void ocularGammaFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float gamma) {

        int Channels = Stride / Width;
        if (Channels == 1)
            return;
        unsigned char GammasMap[256] = { 0 };
        for (int pixel = 0; pixel < 256; pixel++) {
            GammasMap[pixel] = ClampToByte(pow(pixel, gamma));
        }
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* pOutput = Output + (Y * Stride);
            unsigned char* pInput = Input + (Y * Stride);
            for (int X = 0; X < Width; X++) {
                pOutput[0] = GammasMap[pInput[0]];
                pOutput[1] = GammasMap[pInput[1]];
                pOutput[2] = GammasMap[pInput[2]];
                pInput += Channels;
                pOutput += Channels;
            }
        }
    }

    void ocularContrastFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float contrast) {

        int Channels = Stride / Width;
        if (Channels == 1)
            return;
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
    }

    void ocularExposureFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float exposure) {

        int Channels = Stride / Width;
        if (Channels == 1)
            return;
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
    }

    void ocularBrightnessFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int brightness) {

        int Channels = Stride / Width;
        if (Channels == 1)
            return;

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
    }

    void ocularFalseColorFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, unsigned char firstColorR,
                                unsigned char firstColorG, unsigned char firstColorB, unsigned char secondColorR,
                                unsigned char secondColorG, unsigned char secondColorB, int intensity) {

        int Channels = Stride / Width;
        if (Channels == 1)
            return;
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
                unsigned char luminanceWeighting = (unsigned char)((13926 * pInput[0] + 46884 * pInput[1] + 4725 * pInput[2]) >> 16);

                pOutput[0] = (unsigned char)((pInput[0] * c1 + ColorMapR[luminanceWeighting] * c2) >> 8);
                pOutput[1] = (unsigned char)((pInput[1] * c1 + ColorMapG[luminanceWeighting] * c2) >> 8);
                pOutput[2] = (unsigned char)((pInput[2] * c1 + ColorMapB[luminanceWeighting] * c2) >> 8);
                pInput += Channels;
                pOutput += Channels;
            }
        }
    }

    void ocularHazeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float distance, float slope, int intensity) {

        int Channels = Stride / Width;
        if (Channels == 1)
            return;
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
            return;
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
                        (unsigned char)((int)(pInput[0] * c1 + ((ClampToByte(pInput[0] - distanceColorMap[Y]) * patchDistanceMap[Y]) >> 8) * c2) >> 8);
                pOutput[1] =
                        (unsigned char)((int)(pInput[1] * c1 + ((ClampToByte(pInput[1] - distanceColorMap[Y]) * patchDistanceMap[Y]) >> 8) * c2) >> 8);
                pOutput[2] =
                        (unsigned char)((int)(pInput[2] * c1 + ((ClampToByte(pInput[2] - distanceColorMap[Y]) * patchDistanceMap[Y]) >> 8) * c2) >> 8);
                pInput += Channels;
                pOutput += Channels;
            }
        }
        free(distanceColorMap);
        free(patchDistanceMap);
    }

    void ocularOpacityFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float opacity) {

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
                unsigned char* pInput = Input + (Y * Stride);
                for (int X = 0; X < Width; X++) {
                    pOutput[3] = opacityMap[pInput[3]];
                    pInput += Channels;
                    pOutput += Channels;
                }
            }
        }
    }

    void ocularLevelsFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                            ocularLevelParams* redLevelParams, ocularLevelParams* greenLevelParams, ocularLevelParams* blueLevelParams) {

        int Channels = Stride / Width;
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
    }

    void rgb2yiq(unsigned char* R, unsigned char* G, unsigned char* B, short* Y, short* I, short* Q) {
        *Y = (short)((int)(0.299f * 65536) * *R + (int)(0.587f * 65536) * *G + (int)(0.114f * 65536) * *B) >> 16;
        *I = (short)((int)(0.595f * 65536) * *R - (int)(0.274453f * 65536) * *G - (int)(0.321263f * 65536) * *B) >> 16;
        *Q = (short)((int)(0.211456f * 65536) * *R - (int)(0.522591f * 65536) * *G + (int)(0.311135f * 65536) * *B) >> 16;
    }

    void yiq2rgb(short* Y, short* I, short* Q, unsigned char* R, unsigned char* G, unsigned char* B) {
        *R = ClampToByte((int)(Y + ((((int)(0.9563 * 65536)) * (*I)) + ((int)(0.6210 * 65536)) * (*Q))) >> 16);
        *G = ClampToByte((int)(Y - ((((int)(0.2721 * 65536)) * (*I)) + ((int)(0.6474 * 65536)) * (*Q))) >> 16);
        *B = ClampToByte((int)(Y + ((((int)(1.7046 * 65536)) * (*Q)) - ((int)(1.1070 * 65536)) * (*I))) >> 16);
    }

    void ocularHueFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float hueAdjust) {

        int Channels = Stride / Width;
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
    }

    void ocularHighlightShadowTintFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float shadowTintR,
                                         float shadowTintG, float shadowTintB, float highlightTintR, float highlightTintG,
                                         float highlightTintB, float shadowTintIntensity, float highlightTintIntensity) {

        int Channels = Stride / Width;
        if (Channels == 1) {
            return;
        }

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
    }

    void ocularHighlightShadowFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float shadows, float highlights) {

        int Channels = Stride / Width;
        if (Channels == 1) {
            return;
        }
        short luminanceWeightingMap[256] = { 0 };
        short shadowMap[256] = { 0 };
        short highlightMap[256] = { 0 };

        int divLuminance[256 * 256] = { 0 };
        for (int pixel = 0; pixel < 256; pixel++) {
            luminanceWeightingMap[pixel] = (short)(pixel * 0.3f);
            float luminance = (1.0f / 255.0f) * pixel;
            shadowMap[pixel] = (short)(255.0f *
                                       clamp((powf(luminance, 1.0f / (shadows + 1.0f)) + (-0.76f) * powf(luminance, 2.0f / (shadows + 1.0f))) - luminance,
                                             0.0f, 1.0f));
            highlightMap[pixel] =
                    (short)(255.0f *
                            clamp((1.0f - (powf(1.0f - luminance, 1.0f / (2.0f - highlights)) + (-0.8f) * powf(1.0f - luminance, 2.0f / (2.0f - highlights)))) - luminance,
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
                const short luminance = luminanceWeightingMap[pInput[0]] + luminanceWeightingMap[pInput[1]] + luminanceWeightingMap[pInput[2]];
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
    }

    void ocularMonochromeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, unsigned char filterColorR,
                                unsigned char filterColorG, unsigned char filterColorB, int intensity) {

        int Channels = Stride / Width;
        if (Channels == 1) {
            return;
        }
        unsigned short sIntensity = (unsigned short)max(min(intensity, 100), 0);
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
    }

    void ocularColorInvertFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride) {

        int Channels = Stride / Width;
        if (Channels == 1) {
            return;
        }
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
    }

    void ocularSolidColorGenerator(unsigned char* Output, int Width, int Height, int Stride, unsigned char colorR, unsigned char colorG,
                                   unsigned char colorB, unsigned char colorAlpha) {

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

    void ocularLuminanceThresholdFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, unsigned char threshold) {

        int Channels = Stride / Width;
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
            return;
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
    }

    static void autoLevel(const unsigned int* histgram, unsigned char* remapLut, int numberOfPixels, float cutLimit, float contrast) {
        int minPos = 0, maxPos = 255;
        int minValue = 0, maxValue = 255;
        for (int I = 0; I < 256; I++) {
            if (histgram[I] != 0) {
                minValue = I;
                break;
            }
        }
        for (int I = 255; I >= 0; I--) {
            if (histgram[I] != 0) {
                maxValue = I;
                break;
            }
        }
        int sum = 0;
        for (int I = minValue; I < 256; I++) {
            sum = sum + histgram[I];
            if (sum >= numberOfPixels * cutLimit) {
                minPos = I;
                break;
            }
        }
        sum = 0;
        for (int I = 255; I >= 0; I--) {
            sum = sum + histgram[I];
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

    bool ocularAutoWhiteBalance(unsigned char* input, unsigned char* output, int width, int height, int channels, int stride,
                                int colorCoeff, float cutLimit, float contrast) {

        bool ret = false;
        if (channels == 3 || channels == 4) {
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
            ret = isColorCast(histogramCb, histogramCr, numberOfPixels, colorCoeff);
            if (!ret) {
                memcpy(output, input, numberOfPixels * channels * sizeof(*input));
                return ret;
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
        }
        return ret;
    }

    void ocularWhiteBalanceFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float temperature, float tint) {

        int Channels = Stride / Width;
        if (Channels == 1) {
            return;
        }
        float Temperature = temperature;
        Temperature = Temperature < 5000 ? (float)(0.0004 * (Temperature - 5000.0)) : (float)(0.00006 * (Temperature - 5000.0));

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
    }

    void ocularVibranceFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float vibrance) {

        int Channels = Stride / Width;
        if (Channels == 1) {
            return;
        }
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
    }

    void rgb2hsv(const unsigned char* R, const unsigned char* G, const unsigned char* B, unsigned char* H, unsigned char* S, unsigned char* V) {
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
            // normalize 0-359°
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

        *H = (unsigned char)(h >> 1); // 0-179
        *S = (unsigned char)s;        // 0-255
        *V = (unsigned char)nMax;     // 0-255
    }

    void hsv2rgb(const unsigned char* H, const unsigned char* S, const unsigned char* V, unsigned char* R, unsigned char* G, unsigned char* B) {

        if (S > 0) {
            int r, g, b;
            r = *V;
            g = *V;
            b = *V;
            float h = *H * (6.0f / 180.0f); // 0-180° -> 0.0-1.0
            int i = (int)h;
            int f = (int)(256 * (h - (float)i));
            int VS = (*V * *S) >> 8;
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
            default: break;
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

    void ocularSkinToneFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float skinToneAdjust,
                              float skinHue, float skinHueThreshold, float maxHueShift, float maxSaturationShift, int upperSkinToneColor) {

        int Channels = Stride / Width;
        if (Channels == 1) {
            return;
        }
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
                rgb2hsv(&R, &G, &B, &H, &S, &V);
                // final color
                _S = (unsigned char)(S + satMap[H]);
                hsv2rgb(&hueMap[H], &_S, &V, &pOutput[0], &pOutput[1], &pOutput[2]);

                pInput += Channels;
                pOutput += Channels;
            }
        }
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

    void ocularBilateralFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float sigmaSpatial, float sigmaRange) {

        int Channels = Stride / Width;
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

            return;
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

    void ocularGaussianBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float GaussianSigma) {
        int Channels = Stride / Width;
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
            return;
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
    }

#define float2fixed(x) (((int)((x) * 4096.0f + 0.5f)) << 8)

    void rgb2ycbcr(unsigned char R, unsigned char G, unsigned char B, unsigned char* y, unsigned char* cb, unsigned char* cr) {
        *y = (unsigned char)((19595 * R + 38470 * G + 7471 * B) >> 16);
        *cb = (unsigned char)(((36962 * (B - *y)) >> 16) + 128);
        *cr = (unsigned char)(((46727 * (R - *y)) >> 16) + 128);
    }

    void ycbcr2rgb(unsigned char y, unsigned char Cb, unsigned char Cr, unsigned char* R, unsigned char* G, unsigned char* B) {
        int y_fixed = (y << 20) + (1 << 19); // rounding
        int r, g, b;
        int cr = Cr - 128;
        int cb = Cb - 128;
        r = y_fixed + cr * float2fixed(1.40200f);
        g = y_fixed + (cr * -float2fixed(0.71414f)) + ((cb * -float2fixed(0.34414f)) * 0xffff0000);
        b = y_fixed + cb * float2fixed(1.77200f);
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
        *R = (unsigned char)r;
        *G = (unsigned char)g;
        *B = (unsigned char)b;
    }

    void ocularUnsharpMaskFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float GaussianSigma, int intensity) {

        int Channels = Stride / Width;
        intensity = max(min(intensity, 100), 0);
        int c1 = 256 * (100 - intensity) / 100;
        int c2 = 256 * (100 - (100 - intensity)) / 100;

        unsigned char unsharpMaskMap[256 * 256] = { 0 };
        for (unsigned int PS = 0; PS < 256; PS++) {
            unsigned char* pUnsharpMaskMap = unsharpMaskMap + (PS << 8);
            for (unsigned int PD = 0; PD < 256; PD++) {
                unsigned char retPD = ClampToByte((PS - PD) + 128);
                retPD = (unsigned char)((PS <= 128) ? (retPD * PS / 128) : (255 - (255 - retPD) * (255 - PS) / 128));
                // enhanced edge method
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
                unsigned char* pTemp = Temp + (Y * Width);
                unsigned char* pBlur = Blur + (Y * Width);
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
                unsigned char* pInput = Input + (Y * Stride);
                unsigned char* pOutput = Output + (Y * Stride);
                unsigned char* pTemp = Temp + (Y * Width);
                unsigned char* pBlur = Blur + (Y * Width);
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
                unsigned char* pInput = Input + (Y * Width);
                unsigned char* pBlur = Blur + (Y * Width);
                unsigned char* pOutput = Output + (Y * Width);
                for (int x = 0; x < Width; x++) {
                    // Sharpen: High Contrast Overlay
                    pOutput[0] = (unsigned char)(pInput[0] - pOutput[0] + 128);
                    unsigned char* pUnsharpMaskMap = unsharpMaskMap + (pInput[0] << 8);
                    pOutput[0] = pUnsharpMaskMap[pOutput[0]];

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

    void ocularBoxBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius) {

        int Channels = Stride / Width;
        unsigned char* temp = (unsigned char*)malloc(Width * Height * Channels);
        if (temp == NULL) {
            return;
        }
        boxfilterRow(Input, temp, Width, Height, Channels, Radius);
        boxfilterCol(temp, Output, Width, Height, Channels, Radius);
        free(temp);
    }

    void ocularSharpenExFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float Radius, float sharpness, int intensity) {

        int Channels = Stride / Width;
        intensity = max(min(intensity, 100), 0);
        int c1 = 256 * (100 - intensity) / 100;
        int c2 = 256 * (100 - (100 - intensity)) / 100;
        // Sharpen: High Contrast Overlay
        unsigned char sharpnessMap[256 * 256] = { 0 };
        for (unsigned int PS = 0; PS < 256; PS++) {
            unsigned char* pSharpnessMap = sharpnessMap + (PS << 8);
            for (unsigned int PD = 0; PD < 256; PD++) {
                unsigned char retPD = ClampToByte((sharpness * (PS - PD)) + 128);
                retPD = (unsigned char)((PS <= 128) ? (retPD * PS / 128) : (255 - (255 - retPD) * (255 - PS) / 128));
                // enhanced edge method
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
                unsigned char* pTemp = temp + (Y * Width);
                unsigned char* pBlur = blur + (Y * Width);
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
                unsigned char* pInput = Input + (Y * Stride);
                unsigned char* pOutput = Output + (Y * Stride);
                unsigned char* pTemp = temp + (Y * Width);
                unsigned char* pBlur = blur + (Y * Width);
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
                unsigned char* pInput = Input + (Y * Width);
                unsigned char* pBlur = Blur + (Y * Width);
                unsigned char* pOutput = Output + (Y * Width);
                for (int x = 0; x < Width; x++) {
                    unsigned char* pSharpnessMap = sharpnessMap + (pInput[0] << 8);
                    pOutput[0] = pSharpnessMap[pOutput[0]];

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

    void ocularResamplingFilter(unsigned char* Input, unsigned int Width, unsigned int Height, unsigned int Stride, unsigned char* Output,
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

    void ocularCropFilter(const unsigned char* Input, int Width, int Height, int srcStride, unsigned char* Output, int cropX, int cropY,
                          int dstWidth, int dstHeight, int dstStride) {

        int Channels = srcStride / Width;

        const unsigned char* src = Input + cropY * srcStride + cropX * Channels;
        unsigned char* dst = Output;

        for (int y = 0; y < dstHeight; y++) {
            memcpy(dst, src, dstStride);
            src += srcStride;
            dst += dstStride;
        }
    }

    void ocularAutoLevel(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float fraction) {

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

    void ocularSobelEdgeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height) {

        if ((Input == NULL) || (Output == NULL))
            return;
        if ((Width <= 0) || (Height <= 0))
            return;

        unsigned char* SqrLut = (unsigned char*)malloc(65026 * sizeof(unsigned char));
        unsigned char* RowCopy = (unsigned char*)malloc((Width + 2) * 3 * sizeof(unsigned char));
        if ((SqrLut == NULL) || (RowCopy == NULL)) {
            if (SqrLut != NULL)
                free(SqrLut);
            if (RowCopy != NULL)
                free(RowCopy);
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
            unsigned char* LinePS = Input + Y * Width;
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
    }

    int ocularHoughLines(unsigned char* Input, int Width, int Height, int lineIntensity, int Threshold, float resTheta, int numLine,
                         float* Radius, float* Theta) {

        int halfHoughWidth = (int)(sqrtf((float)(Width * Width + Height * Height)));
        int houghWidth = halfHoughWidth * 2;
        int maxTheta = (int)(180.0f / resTheta + 0.5f);
        int houghMapSize = houghWidth * maxTheta;
        unsigned short* houghMap = (unsigned short*)calloc((size_t)houghMapSize, sizeof(unsigned short));
        float* sinLUT = (float*)calloc((size_t)maxTheta, sizeof(float));
        float* cosLUT = (float*)calloc((size_t)maxTheta, sizeof(float));
        if (sinLUT == NULL || cosLUT == NULL || houghMap == NULL) {
            if (houghMap) {
                free(houghMap);
            }
            if (cosLUT) {
                free(cosLUT);
            }
            if (sinLUT) {
                free(sinLUT);
            }
            return 0;
        }
        float thetaStep = M_PI / maxTheta;
        for (int theta = 0; theta < maxTheta; theta++) {
            sinLUT[theta] = (float)fastSin(theta * thetaStep);
            cosLUT[theta] = (float)fastCos(theta * thetaStep);
        }

        for (int y = 0; y < Height; y++) {
            unsigned char* pIn = Input + (y * Width);
            for (int x = 0; x < Width; x++) {
                if (pIn[x] > Threshold) {
                    for (int theta = 0; theta < maxTheta; theta++) {
                        int r = (int)(x * sinLUT[theta] + y * cosLUT[theta] + halfHoughWidth + 0.5f);
                        houghMap[r * maxTheta + theta]++;
                    }
                }
            }
        }

        int nLine = 0;
        for (int i = 0; i < houghMapSize && nLine < numLine; i++) {
            if (houghMap[i] > lineIntensity) {
                Radius[nLine] = (float)(i / maxTheta);
                Theta[nLine] = (i - Radius[nLine] * maxTheta) * resTheta;
                Radius[nLine] -= halfHoughWidth;
                nLine++;
            }
        }

        if (houghMap) {
            free(houghMap);
        }
        if (cosLUT) {
            free(cosLUT);
        }
        if (sinLUT) {
            free(sinLUT);
        }

        return nLine;
    }

    void ocularDrawLine(unsigned char* canvas, int width, int height, int stride, int x1, int y1, int x2, int y2, unsigned char R,
                        unsigned char G, unsigned char B) {

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
            return;
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
    }

    bool ocularGetImageSize(const char* file_path, int* width, int* height, int* file_size) {

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
                    printf("this is a muti-ico file.");
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

    void ocularConvolution2DFilter(unsigned char* input, unsigned char* output, int width, int height, int channels, float* kernel,
                                   unsigned char filterW, unsigned char cfactor, unsigned char bias) {

        int factor = 256 / cfactor;
        int halfW = filterW / 2;
        if (channels == 3 || channels == 4) {
            for (int y = 0; y < height; y++) {
                int y1 = y - halfW + height;
                for (int x = 0; x < width; x++) {
                    int x1 = x - halfW + width;
                    int r = 0;
                    int g = 0;
                    int b = 0;
                    unsigned int p = (y * width + x) * channels;
                    for (unsigned int fx = 0; fx < filterW; fx++) {
                        int dx = (x1 + fx) % width;
                        int fidx = fx * (filterW);
                        for (unsigned int fy = 0; fy < filterW; fy++) {
                            int pos = (((y1 + fy) % height) * width + dx) * channels;

                            float* pKernel = &kernel[fidx + (fy)];
                            r += input[pos] * (*pKernel);
                            g += input[pos + 1] * (*pKernel);
                            b += input[pos + 2] * (*pKernel);
                        }
                    }
                    output[p] = ClampToByte(((factor * r) >> 8) + bias);
                    output[p + 1] = ClampToByte(((factor * g) >> 8) + bias);
                    output[p + 2] = ClampToByte(((factor * b) >> 8) + bias);
                }
            }
        } else if (channels == 1) {
            for (int y = 0; y < height; y++) {
                int y1 = y - halfW + height;
                for (int x = 0; x < width; x++) {
                    int r = 0;
                    unsigned int p = (y * width + x);
                    int x1 = x - halfW + width;
                    for (unsigned int fx = 0; fx < filterW; fx++) {
                        int dx = (x1 + fx) % width;
                        int fidx = fx * (filterW);
                        for (unsigned int fy = 0; fy < filterW; fy++) {
                            int pos = (((y1 + fy) % height) * width + dx);
                            float szKernel = kernel[fidx + (fy)];
                            r += input[pos] * szKernel;
                        }
                    }
                    output[p] = ClampToByte(((factor * r) >> 8) + bias);
                }
            }
        }
    }

    void ocularMotionBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, int radius, int angle) {

        // enforce odd width kernel which ensures a center pixel is always available
        int kernelSize = ceil(2 * radius + 1);

        // allocate memory for the kernel
        float* kernel = (float*)malloc(kernelSize * kernelSize * sizeof(float));

        // calculate motion blur weights based on the angle
        float angleRadians = angle * M_PI / 180.0; // convert angle to radians
        float dx = fastCos(angleRadians);
        float dy = fastSin(angleRadians);

        // set weights for the kernel
        for (int i = 0; i < kernelSize; i++) {
            for (int j = 0; j < kernelSize; j++) {
                float distance = fabs(round((float)(i - radius) * dx + (float)(j - radius) * dy));
                kernel[i * kernelSize + j] = (int)((radius - distance) / radius); // linear interpolate
                if (kernel[i * kernelSize + j] < 0) {
                    kernel[i * kernelSize + j] = 0;
                }
            }
        }

        // Calculate the sum of all the weights in the kernel
        float sumWeights = 0.0;
        for (int i = 0; i < kernelSize * kernelSize; i++) {
            sumWeights += kernel[i];
        }

        // Normalize the kernel so the sum of all weights is equal to 1
        //        for (int i = 0; i < kernelSize * kernelSize; i++) {
        //            kernel[i] /= sumWeights;
        //        }

        ocularConvolution2DFilter(Input, Output, Width, Height, Channels, kernel, kernelSize, sumWeights, 0);

        free(kernel);
    }
#ifdef __cplusplus
}
#endif