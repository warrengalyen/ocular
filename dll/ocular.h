#ifndef OCULAR_H
#define OCULAR_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1310)
    #pragma warning(disable : 4996) // VS doesn't like fopen, but fopen_s is not
                                    // standard C so unusable here
#endif                              /*_MSC_VER */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../lib/color.h"
#include "../lib/blend.h"
#include "../lib/ocr.h"
#include "../lib/interpolate.h"
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

    typedef enum {
        OC_EDGE_WRAP = 0,  // repeat edge pixel
        OC_EDGE_MIRROR = 1 // mirror edge pixel
    } OcEdgeMode;

    typedef enum {
        CannyGaus3x3,
        CannyGaus5x5
    } CannyNoiseFilter;


    //--------------------------Color adjustments--------------------------

    DLIB_EXPORT void ocularGrayscaleFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

    DLIB_EXPORT void ocularRGBFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float redAdjustment,
                                     float greenAdjustment, float blueAdjustment);

    DLIB_EXPORT void ocularHSLFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float hueAdjustment,
                                     float satAdjustment, float lightAdjustment);

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

    DLIB_EXPORT void ocularGammaFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float gamma[]);

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

    DLIB_EXPORT void ocularSkinToneFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float skinToneAdjust,
                                          float skinHue, float skinHueThreshold, float maxHueShift, float maxSaturationShift, int upperSkinToneColor);

    DLIB_EXPORT void ocularAutoLevel(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float fraction);

    DLIB_EXPORT void ocularAutoContrast(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels);

    DLIB_EXPORT void ocularAutoGammaCorrection(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

    DLIB_EXPORT void ocularEqualizeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

    DLIB_EXPORT void ocularAutoThreshold(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

    DLIB_EXPORT void ocularBacklightRepair(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

    DLIB_EXPORT void ocularLayerBlend(unsigned char* baseInput, int bWidth, int bHeight, int bStride, unsigned char* mixInput, int mWidth,
                                      int mHeight, int mStride, OcBlendMode blendMode, int alpha);

    //--------------------------Color adjustments--------------------------

    //--------------------------Image processing--------------------------

    DLIB_EXPORT void ocularConvolution2DFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels,
                                               float* kernel, unsigned char filterW, unsigned char cfactor, unsigned char bias);

    DLIB_EXPORT void ocularMotionBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Distance, int Angle);

    DLIB_EXPORT void ocularRadialBlur(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int centerX,
                                      int centerY, int intensity);

    DLIB_EXPORT void
    ocularAverageBlur(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius, OcEdgeMode edgeMode);

    DLIB_EXPORT void ocularMedianBlur(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

    DLIB_EXPORT void ocularErodeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

    DLIB_EXPORT void ocularDilateFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

    DLIB_EXPORT void ocularBilateralFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                                           float sigmaSpatial, float sigmaRange);

    DLIB_EXPORT void ocularGaussianBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float GaussianSigma);

    DLIB_EXPORT void ocularUnsharpMaskFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                                             float GaussianSigma, int intensity);

    DLIB_EXPORT void ocularBoxBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

    DLIB_EXPORT void ocularSurfaceBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius, int Threshold);

    DLIB_EXPORT void ocularSkinSmoothingFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                                               int smoothingLevel, bool applySkinFilter);

    DLIB_EXPORT void ocularSharpenFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float Radius,
                                         float sharpness, int intensity);

    DLIB_EXPORT void ocularResamplingFilter(unsigned char* Input, unsigned int Width, unsigned int Height, unsigned int Stride, unsigned char* Output,
                                            int newWidth, int newHeight, int dstStride, OcInterpolationMode InterpolationMode);

    DLIB_EXPORT void ocularCropImage(const unsigned char* Input, int Width, int Height, int srcStride, unsigned char* Output, int cropX,
                                     int cropY, int dstWidth, int dstHeight, int dstStride);

    DLIB_EXPORT void ocularFlipImage(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, int type);

    DLIB_EXPORT void ocularDespeckle(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int maxWindowSize, int Threshold);

    DLIB_EXPORT bool ocularDocumentDeskew(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

    DLIB_EXPORT void ocularCannyEdgeDetect(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels,
                                           CannyNoiseFilter kernel_size, int weak_threshold, int strong_threshold);

    DLIB_EXPORT void ocularSobelEdgeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels);

    DLIB_EXPORT void ocularGradientEdgeDetect(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels);

    DLIB_EXPORT void ocularPixelateFilter(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int blockSize);

    DLIB_EXPORT void ocularOilPaintFilter(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int radius, int intensity);

    DLIB_EXPORT int ocularHoughLineDetection(unsigned char* Input, int Width, int Height, int lineIntensity, int Threshold, float resTheta,
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
#endif  /* OCULAR_H */
