#ifndef OCULAR_H
#define OCULAR_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "fastmath.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif


    typedef struct {
        // color level minimum
        int levelMinimum;
        // color scale median
        int levelMiddle;
        // maximum value of color scale
        int levelMaximum;
        // minimum output value
        int minOutput;
        // maximum output value
        int maxOutput;
        // whether to apply
        bool Enable;
    } ocularLevelParams;

    void rgb2yiq(unsigned char* R, unsigned char* G, unsigned char* B, short* Y, short* I, short* Q);
    void yiq2rgb(short* Y, short* I, short* Q, unsigned char* R, unsigned char* G, unsigned char* B);
    void rgb2hsv(const unsigned char* R, const unsigned char* G, const unsigned char* B,
                 unsigned char* H, unsigned char* S, unsigned char* V);
    void hsv2rgb(const unsigned char* H, const unsigned char* S, const unsigned char* V,
                 unsigned char* R, unsigned char* G, unsigned char* B);
    void rgb2ycbcr(unsigned char R, unsigned char G, unsigned char B, unsigned char* y,
                   unsigned char* cb, unsigned char* cr);
    void ycbcr2rgb(unsigned char y, unsigned char Cb, unsigned char Cr, unsigned char* R,
                   unsigned char* G, unsigned char* B);

    //--------------------------Color adjustments--------------------------
    void ocularGrayscaleFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);
    // float redAdjustment = 1.0f, float greenAdjustment = 1.0f, float blueAdjustment = 1.0f
    void ocularRGBFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                         int Stride, float redAdjustment, float greenAdjustment, float blueAdjustment);
    // float thresholdMultiplier = 1.0f
    void ocularAverageLuminanceThresholdFilter(unsigned char* Input, unsigned char* Output, int Width,
                                               int Height, int Stride, float thresholdMultiplier);
    void ocularAverageColor(unsigned char* Input, int Width, int Height, int Stride, unsigned char* AverageR,
                            unsigned char* AverageG, unsigned char* AverageB, unsigned char* AverageA);
    void ocularLuminosity(unsigned char* Input, int Width, int Height, int Stride, unsigned char* Luminance);
    // float intensity = 1.0f
    void ocularColorMatrixFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                                 int Stride, float* colorMatrix, float intensity);
    // int intensity = 100
    void ocularSepiaFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                           int Stride, int intensity);
    // unsigned char colorToReplaceR = 0, unsigned char colorToReplaceG = 160, unsigned char
    // colorToReplaceB = 0, float thresholdSensitivity = 0.2f, float smoothing = 0.1f
    void ocularChromaKeyFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                               int Stride, unsigned char colorToReplaceR, unsigned char colorToReplaceG,
                               unsigned char colorToReplaceB, float thresholdSensitivity, float smoothing);
    // int intensity = 100
    void ocularLookupFilter(unsigned char* Input, unsigned char* Output, unsigned char* lookupTable,
                            int Width, int Height, int Stride, int intensity);

    // float saturation = 1.0
    void ocularSaturationFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                                int Stride, float saturation);
    // float gamma = 1.0f
    void ocularGammaFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                           int Stride, float gamma);
    //  float contrast = 1.0f
    void ocularContrastFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                              int Stride, float contrast);
    // float exposure = 0.0f
    void ocularExposureFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                              int Stride, float exposure);
    // int brightness = 0.0f
    void ocularBrightnessFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                                int Stride, int brightness);
    // unsigned char firstColorR = 0, unsigned char firstColorG = 0, unsigned char firstColorB = 0.5 * 255, unsigned char
    // secondColorR = 1.0f * 255, unsigned char secondColorG = 0, unsigned char secondColorB = 0, int intensity = 100
    void ocularFalseColorFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                                int Stride, unsigned char firstColorR, unsigned char firstColorG,
                                unsigned char firstColorB, unsigned char secondColorR,
                                unsigned char secondColorG, unsigned char secondColorB, int intensity);
    // float distance = 0.3, float slope = 0, int intensity = 100
    void ocularHazeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                          int Stride, float distance, float slope, int intensity);
    // float opacity = 1.0f
    void ocularOpacityFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                             int Stride, float opacity);
    void ocularLevelsFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                            int Stride, ocularLevelParams* redLevelParams,
                            ocularLevelParams* greenLevelParams, ocularLevelParams* blueLevelParams);
    // float hueAdjust = 90.0f
    void ocularHueFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                         int Stride, float hueAdjust);
    // float shadowTintR = 1.0f, float shadowTintG = 0.0f, float shadowTintB = 0.0f, float
    // highlightTintR = 0.0f, float highlightTintG = 0.0f, float highlightTintB = 1.0f, float
    // shadowTintIntensity = 0.0f, float highlightTintIntensity = 0.0f
    void ocularHighlightShadowTintFilter(unsigned char* Input, unsigned char* Output, int Width,
                                         int Height, int Stride, float shadowTintR,
                                         float shadowTintG, float shadowTintB, float highlightTintR,
                                         float highlightTintG, float highlightTintB,
                                         float shadowTintIntensity, float highlightTintIntensity);
    //  float shadows = 0.0f, float highlights = 1.0f
    void ocularHighlightShadowFilter(unsigned char* Input, unsigned char* Output, int Width,
                                     int Height, int Stride, float shadows, float highlights);
    //  unsigned char filterColorR = 0.6 * 255, unsigned char filterColorG = 0.45 * 255, unsigned
    //  char filterColorB = 0.3 * 255, int intensity = 100
    void ocularMonochromeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                                int Stride, unsigned char filterColorR, unsigned char filterColorG,
                                unsigned char filterColorB, int intensity);

    void ocularColorInvertFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);
    // unsigned char colorAlpha = 255
    void ocularSolidColorGenerator(unsigned char* Output, int Width, int Height, int Stride,
                                   unsigned char colorR, unsigned char colorG, unsigned char colorB,
                                   unsigned char colorAlpha);
    // unsigned char threshold = 127
    void ocularLuminanceThresholdFilter(unsigned char* Input, unsigned char* Output, int Width,
                                        int Height, int Stride, unsigned char threshold);
    // float temperature = 5000, float tint = 0
    void ocularWhiteBalanceFilter(unsigned char* Input, unsigned char* Output, int Width,
                                  int Height, int Stride, float temperature, float tint);
    // float vibrance = 1.2
    void ocularVibranceFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                              int Stride, float vibrance);
    // float skinToneAdjust = 0.3f, float skinHue = 0.05f, float skinHueThreshold = 80.0f, float
    // maxHueShift = 0.25f, float maxSaturationShift = 0.4f, int upperSkinToneColor = 0
    void ocularSkinToneFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                              int Stride, float skinToneAdjust, float skinHue, float skinHueThreshold,
                              float maxHueShift, float maxSaturationShift, int upperSkinToneColor);
    // float fraction = 0.05f
    void ocularAutoLevel(const unsigned char* Input, unsigned char* Output, int Width, int Height,
                         int Stride, float fraction);
    //--------------------------Color adjustments--------------------------

    //--------------------------Image processing--------------------------
    void ocularGaussianBlurFilter(unsigned char* Input, unsigned char* Output, int Width,
                                  int Height, int Stride, float GaussianSigma);
    // float GaussianSigma = 4, int intensity = 100
    void ocularUnsharpMaskFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                                 int Stride, float GaussianSigma, int intensity);
    // int Radius = 3
    void ocularBoxBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                             int Stride, int Radius);
    // float Radius = 4, int sharpness = 1, int intensity = 100
    void ocularSharpenFilter(unsigned char* Input, unsigned char* Output, int Width, int Height,
                             int Stride, float Radius, int sharpness, int intensity);
    void ocularResamplingFilter(unsigned char* Input, unsigned int Width, unsigned int Height, unsigned int Stride,
                                unsigned char* Output, int newWidth, int newHeight, int dstStride);
    void ocularCropFilter(const unsigned char* Input, int Width, int Height, int srcStride, unsigned char* Output,
                          int cropX, int cropY, int dstWidth, int dstHeight, int dstStride);
    void ocularSobelEdge(unsigned char* Input, unsigned char* Output, int Width, int Height);
    int ocularHoughLines(unsigned char* Input, int Width, int Height, int lineIntensity,
                         int Threshold, float resTheta, int numLine, float* Radius, float* Theta);
    void ocularDrawLine(unsigned char* canvas, int width, int height, int stride, int x1, int y1,
                        int x2, int y2, unsigned char R, unsigned char G, unsigned char B);
    //--------------------------Image processing--------------------------

#ifdef __cplusplus
}
#endif
#endif /* OCULAR_H */
