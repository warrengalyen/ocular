#ifndef OCULAR_H
#define OCULAR_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1310)
    #pragma warning(disable : 4996) // VS doesn't like fopen, but fopen_s is not
                                    // standard C so unusable here
#endif                              /*_MSC_VER */

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

#include "fastmath.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "dlib_export.h"

    // Parameters for Levels filter
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

    //--------------------------Color conversion--------------------------

    DLIB_EXPORT void rgb2yiq(unsigned char* R, unsigned char* G, unsigned char* B, short* Y, short* I, short* Q);

    DLIB_EXPORT void yiq2rgb(short* Y, short* I, short* Q, unsigned char* R, unsigned char* G, unsigned char* B);

    DLIB_EXPORT void
    rgb2hsv(const unsigned char* R, const unsigned char* G, const unsigned char* B, unsigned char* H, unsigned char* S, unsigned char* V);

    DLIB_EXPORT void
    hsv2rgb(const unsigned char* H, const unsigned char* S, const unsigned char* V, unsigned char* R, unsigned char* G, unsigned char* B);

    /// @brief RGB to YCbCr color space conversion.
    DLIB_EXPORT void rgb2ycbcr(unsigned char R, unsigned char G, unsigned char B, unsigned char* y, unsigned char* cb, unsigned char* cr);

    /// @brief YCbCr to RGB color space conversion.
    DLIB_EXPORT void ycbcr2rgb(unsigned char y, unsigned char Cb, unsigned char Cr, unsigned char* R, unsigned char* G, unsigned char* B);

    //--------------------------Color conversion--------------------------

    //--------------------------Color adjustments--------------------------

    DLIB_EXPORT void ocularGrayscaleFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

    DLIB_EXPORT void ocularRGBFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float redAdjustment,
                                     float greenAdjustment, float blueAdjustment);

    DLIB_EXPORT void ocularAverageLuminanceThresholdFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                                                           float thresholdMultiplier);

    DLIB_EXPORT void ocularAverageColor(unsigned char* Input, int Width, int Height, int Stride, unsigned char* AverageR,
                                        unsigned char* AverageG, unsigned char* AverageB, unsigned char* AverageA);

    DLIB_EXPORT void ocularLuminosity(unsigned char* Input, int Width, int Height, int Stride, unsigned char* Luminance);

    DLIB_EXPORT void ocularColorMatrixFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                                             float* colorMatrix, float intensity);

    DLIB_EXPORT void ocularSepiaFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int intensity);

    DLIB_EXPORT void ocularChromaKeyFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                                           unsigned char colorToReplaceR, unsigned char colorToReplaceG, unsigned char colorToReplaceB,
                                           float thresholdSensitivity, float smoothing);

    DLIB_EXPORT void ocularLookupFilter(unsigned char* Input, unsigned char* Output, unsigned char* lookupTable, int Width, int Height,
                                        int Stride, int intensity);

    DLIB_EXPORT void ocularSaturationFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float saturation);

    DLIB_EXPORT void ocularGammaFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float gamma);

    DLIB_EXPORT void ocularContrastFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float contrast);

    DLIB_EXPORT void ocularExposureFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float exposure);

    DLIB_EXPORT void ocularBrightnessFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int brightness);

    DLIB_EXPORT void ocularFalseColorFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                                            unsigned char firstColorR, unsigned char firstColorG, unsigned char firstColorB,
                                            unsigned char secondColorR, unsigned char secondColorG, unsigned char secondColorB, int intensity);

    DLIB_EXPORT void ocularHazeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float distance,
                                      float slope, int intensity);

    DLIB_EXPORT void ocularOpacityFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float opacity);

    DLIB_EXPORT void ocularLevelsFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                                        ocularLevelParams* redLevelParams, ocularLevelParams* greenLevelParams, ocularLevelParams* blueLevelParams);

    DLIB_EXPORT void ocularHueFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float hueAdjust);

    DLIB_EXPORT void ocularHighlightShadowTintFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float shadowTintR,
                                                     float shadowTintG, float shadowTintB, float highlightTintR, float highlightTintG,
                                                     float highlightTintB, float shadowTintIntensity, float highlightTintIntensity);

    DLIB_EXPORT void ocularHighlightShadowFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                                                 float shadows, float highlights);

    DLIB_EXPORT void ocularMonochromeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                                            unsigned char filterColorR, unsigned char filterColorG, unsigned char filterColorB, int intensity);

    DLIB_EXPORT void ocularColorInvertFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

    DLIB_EXPORT void ocularSolidColorGenerator(unsigned char* Output, int Width, int Height, int Stride, unsigned char colorR,
                                               unsigned char colorG, unsigned char colorB, unsigned char colorAlpha);

    DLIB_EXPORT void ocularLuminanceThresholdFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, unsigned char threshold);

    DLIB_EXPORT bool ocularAutoWhiteBalance(unsigned char* input, unsigned char* output, int Width, int height, int channels, int stride,
                                            int colorCoeff, float cutLimit, float contrast);

    DLIB_EXPORT void
    ocularWhiteBalanceFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float temperature, float tint);

    DLIB_EXPORT void ocularVibranceFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float vibrance);

    DLIB_EXPORTvoid ocularSkinToneFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float skinToneAdjust,
                                         float skinHue, float skinHueThreshold, float maxHueShift, float maxSaturationShift, int upperSkinToneColor);

    DLIB_EXPORT void ocularAutoLevel(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float fraction);

    //--------------------------Color adjustments--------------------------

    //--------------------------Image processing--------------------------

    DLIB_EXPORT void ocularConvolution2DFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels,
                                               int* kernel, unsigned char filterW, unsigned char cfactor, unsigned char bias);

    DLIB_EXPORT void ocularMotionBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, int radius, int angle);

    DLIB_EXPORT void ocularBilateralFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                                           float sigmaSpatial, float sigmaRange);

    DLIB_EXPORT void ocularGaussianBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float GaussianSigma);

    DLIB_EXPORT void ocularUnsharpMaskFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                                             float GaussianSigma, int intensity);

    DLIB_EXPORT void ocularBoxBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

    DLIB_EXPORT void ocularSharpenFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float Radius,
                                         float sharpness, int intensity);

    DLIB_EXPORT void ocularResamplingFilter(unsigned char* Input, unsigned int Width, unsigned int Height, unsigned int Stride,
                                            unsigned char* Output, int newWidth, int newHeight, int dstStride);

    DLIB_EXPORT void ocularCropFilter(const unsigned char* Input, int Width, int Height, int srcStride, unsigned char* Output, int cropX,
                                      int cropY, int dstWidth, int dstHeight, int dstStride);

    DLIB_EXPORT void ocularSobelEdgeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height);

    DLIB_EXPORT int ocularHoughLines(unsigned char* Input, int Width, int Height, int lineIntensity, int Threshold, float resTheta,
                                     int numLine, float* Radius, float* Theta);

    DLIB_EXPORT void ocularDrawLine(unsigned char* canvas, int width, int height, int stride, int x1, int y1, int x2, int y2,
                                    unsigned char R, unsigned char G, unsigned char B);

    //--------------------------Image processing--------------------------

    //--------------------------preImage processing--------------------------

    DLIB_EXPORT bool ocularGetImageSize(const char* file_path, int* width, int* height, int* file_size);

    //--------------------------preImage processing--------------------------

#ifdef __cplusplus
}
#endif
#endif /* OCULAR_H */