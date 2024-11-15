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
#include "../lib/palette.h"
#include "../lib/dither.h"
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

    DLIB_EXPORT OC_STATUS ocularGrayscaleFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

    DLIB_EXPORT OC_STATUS ocularRGBFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float redAdjustment,
                                     float greenAdjustment, float blueAdjustment);

    DLIB_EXPORT OC_STATUS ocularHSLFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float hueAdjustment,
                                     float satAdjustment, float lightAdjustment);

    DLIB_EXPORT OC_STATUS ocularAverageLuminanceThresholdFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                                                           float thresholdMultiplier);

    DLIB_EXPORT OC_STATUS ocularAverageColor(unsigned char* Input, int Width, int Height, int Stride, unsigned char* AverageR,
                                        unsigned char* AverageG, unsigned char* AverageB, unsigned char* AverageA);

    DLIB_EXPORT OC_STATUS ocularLuminosity(unsigned char* Input, int Width, int Height, int Stride, unsigned char* Luminance);

    DLIB_EXPORT OC_STATUS ocularColorMatrixFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                                             float* colorMatrix, float intensity);

    DLIB_EXPORT OC_STATUS ocularSepiaFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int intensity);

    DLIB_EXPORT OC_STATUS ocularChromaKeyFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                                           unsigned char colorToReplaceR, unsigned char colorToReplaceG, unsigned char colorToReplaceB,
                                           float thresholdSensitivity, float smoothing);

    DLIB_EXPORT OC_STATUS ocularLookupFilter(unsigned char* Input, unsigned char* Output, unsigned char* lookupTable, int Width, int Height,
                                        int Stride, int intensity);

    DLIB_EXPORT OC_STATUS ocularSaturationFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float saturation);

    DLIB_EXPORT OC_STATUS ocularGammaFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float gamma[]);

    DLIB_EXPORT OC_STATUS ocularContrastFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float contrast);

    DLIB_EXPORT OC_STATUS ocularExposureFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float exposure);

    DLIB_EXPORT OC_STATUS ocularBrightnessFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int brightness);

    DLIB_EXPORT OC_STATUS ocularFalseColorFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                                            unsigned char firstColorR, unsigned char firstColorG, unsigned char firstColorB,
                                            unsigned char secondColorR, unsigned char secondColorG, unsigned char secondColorB, int intensity);

    DLIB_EXPORT OC_STATUS ocularHazeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float distance,
                                      float slope, int intensity);

    DLIB_EXPORT OC_STATUS ocularOpacityFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float opacity);

    DLIB_EXPORT OC_STATUS ocularLevelsFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                                        ocularLevelParams* redLevelParams, ocularLevelParams* greenLevelParams, ocularLevelParams* blueLevelParams);

    DLIB_EXPORT OC_STATUS ocularHueFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float hueAdjust);

    DLIB_EXPORT OC_STATUS ocularHighlightShadowTintFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float shadowTintR,
                                                     float shadowTintG, float shadowTintB, float highlightTintR, float highlightTintG,
                                                     float highlightTintB, float shadowTintIntensity, float highlightTintIntensity);

    DLIB_EXPORT OC_STATUS ocularHighlightShadowFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                                                 float shadows, float highlights);

    DLIB_EXPORT OC_STATUS ocularMonochromeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                                            unsigned char filterColorR, unsigned char filterColorG, unsigned char filterColorB, int intensity);

    DLIB_EXPORT OC_STATUS ocularColorInvertFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

    DLIB_EXPORT OC_STATUS ocularSolidColorGenerator(unsigned char* Output, int Width, int Height, int Stride, unsigned char colorR,
                                               unsigned char colorG, unsigned char colorB, unsigned char colorAlpha);

    DLIB_EXPORT OC_STATUS ocularLuminanceThresholdFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, unsigned char threshold);

    DLIB_EXPORT bool ocularAutoWhiteBalance(unsigned char* input, unsigned char* output, int Width, int height, int channels, int stride,
                                            int colorCoeff, float cutLimit, float contrast);

    DLIB_EXPORT OC_STATUS ocularWhiteBalanceFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float temperature, float tint);

    DLIB_EXPORT OC_STATUS ocularVibranceFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float vibrance);

    DLIB_EXPORT OC_STATUS ocularSkinToneFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float skinToneAdjust,
                                               float skinHue, float skinHueThreshold, float maxHueShift, float maxSaturationShift, int upperSkinToneColor);

    DLIB_EXPORT OC_STATUS ocularAutoLevel(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float fraction);

    DLIB_EXPORT OC_STATUS ocularAutoContrast(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels);

    DLIB_EXPORT OC_STATUS ocularAutoGammaCorrection(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

    DLIB_EXPORT OC_STATUS ocularEqualizeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

    DLIB_EXPORT OC_STATUS ocularAutoThreshold(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

    DLIB_EXPORT OC_STATUS ocularBacklightRepair(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

    DLIB_EXPORT OC_STATUS ocularLayerBlend(unsigned char* baseInput, int bWidth, int bHeight, int bStride, unsigned char* mixInput, int mWidth,
                                           int mHeight, int mStride, OcBlendMode blendMode, int alpha);

    DLIB_EXPORT OC_STATUS ocularColorBalance(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                            int redBalance, int greenBalance, int blueBalance, OcToneBalanceMode Mode, bool preserveLuminosity);

    DLIB_EXPORT OC_STATUS ocularMultiscaleRetinex(unsigned char* input, unsigned char* output, int width, int height, int channels,
                                                  OcRetinexMode mode, int scale, int numScales, float dynamic);   

    //--------------------------Color adjustments--------------------------

    //--------------------------Image processing--------------------------

    DLIB_EXPORT OC_STATUS ocularConvolution2DFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels,
                                               float* kernel, unsigned char filterW, unsigned char cfactor, unsigned char bias);

    DLIB_EXPORT OC_STATUS ocularMotionBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Distance, int Angle);

    DLIB_EXPORT OC_STATUS ocularRadialBlur(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int centerX,
                                           int centerY, int intensity);

    DLIB_EXPORT OC_STATUS ocularZoomBlur(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int sampleRadius, float blurAmount,
                                         int centerX, int centerY);

    DLIB_EXPORT OC_STATUS
    ocularAverageBlur(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius, OcEdgeMode edgeMode);

    DLIB_EXPORT OC_STATUS ocularMedianBlur(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

    DLIB_EXPORT OC_STATUS ocularErodeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

    DLIB_EXPORT OC_STATUS ocularDilateFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

    DLIB_EXPORT OC_STATUS ocularMinFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);   

    DLIB_EXPORT OC_STATUS ocularMaxFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

    DLIB_EXPORT OC_STATUS ocularHighPassFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

    DLIB_EXPORT OC_STATUS ocularBilateralFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                                           float sigmaSpatial, float sigmaRange);

    DLIB_EXPORT OC_STATUS ocularGaussianBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float GaussianSigma);

    DLIB_EXPORT OC_STATUS ocularUnsharpMaskFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                                             float GaussianSigma, float intensity, float threshold);

    DLIB_EXPORT OC_STATUS ocularBoxBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

    DLIB_EXPORT OC_STATUS ocularSurfaceBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius, int Threshold);

    DLIB_EXPORT OC_STATUS ocularBEEPSFilter(const unsigned char* Input, unsigned char* Output, int width, int height, int Stride,
                                      float PhotometricStandardDeviation, float SpatialDecay, int RangeFilter);

    DLIB_EXPORT OC_STATUS ocularSkinSmoothingFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                                               int smoothingLevel, bool applySkinFilter);

    DLIB_EXPORT OC_STATUS ocularSharpenExFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float Radius,
                                           float sharpness, int intensity);

    DLIB_EXPORT OC_STATUS ocularResamplingFilter(unsigned char* Input, int Width, int Height, int Stride, unsigned char* Output,
                                            int newWidth, int newHeight, int dstStride, OcInterpolationMode InterpolationMode);

    DLIB_EXPORT OC_STATUS ocularRotateImage(unsigned char* Input, int Width, int Height, int Stride, unsigned char* Output, 
                                unsigned char newWidth, unsigned char newHeight, float angle, bool useTransparency,
                                OcInterpolationMode InterpolationMode, unsigned char fillColorR, unsigned char fillColorG, 
                                unsigned char fillColorB)                                     

    DLIB_EXPORT OC_STATUS ocularCropImage(const unsigned char* Input, int Width, int Height, int srcStride, unsigned char* Output, int cropX,
                                     int cropY, int dstWidth, int dstHeight, int dstStride);

    DLIB_EXPORT OC_STATUS ocularFlipImage(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, int type);

    DLIB_EXPORT OC_STATUS ocularDespeckle(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int maxWindowSize, int Threshold);

    DLIB_EXPORT bool ocularDocumentDeskew(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

    DLIB_EXPORT OC_STATUS ocularCannyEdgeDetect(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels,
                                                CannyNoiseFilter kernel_size, int weak_threshold, int strong_threshold);

    DLIB_EXPORT OC_STATUS ocularSobelEdgeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels);

    DLIB_EXPORT OC_STATUS ocularGradientEdgeDetect(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels);

    DLIB_EXPORT OC_STATUS ocularPixelateFilter(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int blockSize);

    DLIB_EXPORT OC_STATUS ocularOilPaintFilter(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int radius, int intensity);

    DLIB_EXPORT OC_STATUS ocularFrostedGlassEffect(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius, int Range);

    DLIB_EXPORT OC_STATUS ocularFilmGrainEffect(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, float Strength, float Softness);

    DLIB_EXPORT OC_STATUS ocularPalettetizeFromFile(unsigned char* input, unsigned char* output, int width, int height, int channels,
                                                    const char* filename, OcDitherMethod method, int amount);

    DLIB_EXPORT OC_STATUS ocularPalettetizeFromImage(unsigned char* input, unsigned char* output, int width, int height, int channels,
                                                    OcQuantizeMethod method, int maxColors, int amount);

    DLIB_EXPORT int ocularHoughLineDetection(unsigned char* Input, int Width, int Height, int lineIntensity, int Threshold, float resTheta,
                                             int numLine, float* Radius, float* Theta);

    DLIB_EXPORT OC_STATUS ocularDrawLine(unsigned char* canvas, int width, int height, int stride, int x1, int y1, int x2, int y2,
                                         unsigned char R, unsigned char G, unsigned char B);

    //--------------------------Image processing--------------------------

    //--------------------------preImage processing------------------------

    DLIB_EXPORT bool ocularGetImageSize(const char* file_path, int* width, int* height, int* file_size);

    //--------------------------preImage processing------------------------

    //--------------------------File processing----------------------------
    
    DLIB_EXPORT OC_STATUS ocularLoadPalette(const char* filename, OcPalette* palette);

    DLIB_EXPORT void ocularFreePalette(OcPalette* palette);

    DLIB_EXPORT void read_gimp_palette(const char* filename, OcPalette* palette_data);

    DLIB_EXPORT void save_gimp_palette(const char* filename, const OcPalette* palette);

    DLIB_EXPORT void read_riff_palette(const char* filename, OcPalette* palette);

    DLIB_EXPORT void save_riff_palette(const char* filename, const OcPalette* palette);

    DLIB_EXPORT void read_aco_palette(const char* filename, OcPalette* palette);

    DLIB_EXPORT void save_aco_palette(const char* filename, const OcPalette* palette);

    DLIB_EXPORT void read_paintnet_palette(const char* filename, OcPalette* palette_data);

    DLIB_EXPORT void save_paintnet_palette(const char* filename, const OcPalette* palette);

    DLIB_EXPORT void read_act_palette(const char* filename, OcPalette* palette);

    DLIB_EXPORT void save_act_palette(const char* filename, const OcPalette* palette);

    DLIB_EXPORT void read_ase_palette(const char* filename, OcPalette* palette);

    //---------------------------------------------------------------------

    //---------------------------General functions--------------------------

    DLIB_EXPORT OcImage* ocularCreateImage(int width, int height, int channels);

    DLIB_EXPORT void ocularFreeImage(OcImage** image);

    DLIB_EXPORT void ocularCloneImage(OcImage* Input, OcImage** Output);    

    DLIB_EXPORT void ocularTransposeImage(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

    //---------------------------General functions--------------------------
    

#ifdef __cplusplus
}
#endif
#endif  /* OCULAR_H */
