/**
 * @file: ocular.h
 * @author Warren Galyen
 * Created: 1-29-2024
 * Last Updated: 11-1-2024
 * Last update: added version and status string functions
 *
 * @brief Contains exported primary filter functions
 */

#ifndef OCULAR_H
#define OCULAR_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1310)
    #pragma warning(disable : 4996) // VS doesn't like fopen, but fopen_s is not
                                    // standard C so unusable here
#endif /*_MSC_VER */

#include "fastmath.h"
#include "blend.h"
#include "core.h"
#include "interpolate.h"
#include "palette.h"
#include "dither.h"
#include "quanitize.h"
#include "version.h"
// #include "fft.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>


static const char* ocular_version = VERSION_MAJOR "." VERSION_MINOR "." VERSION_PATCH "." VERSION_BUILD ;
static char timestamp[] = __DATE__ " " __TIME__;

    /** @enum OcInterpolationMode
     * @brief Interpolation method to use for resampling filter
     */
    typedef enum {
        OC_INTERPOLATE_NEAREST,
        OC_INTERPOLATE_BILINEAR,
        OC_INTERPOLATE_BICUBIC,
        OC_INTERPOLATE_LANZCOS,
    } OcInterpolationMode;

    /**
     * @struct Parameters for Levels filter
     * @var levelMinimum color level minimum
     * @var levelMiddle color scale median
     * @var levelMaximum maximum value of color scale
     * @var minOutput minimum output value
     * @var maxOutput maximum output value
     * @var Enable Flag used to determine if to apply
     */
    typedef struct {
        int levelMinimum;
        int levelMiddle;
        int levelMaximum;
        int minOutput;
        int maxOutput;
        bool Enable;
    } ocularLevelParams;

    /**
     * @enum OcDirection
     * @brief Parameter for image flip function
     */
    typedef enum {
        OC_DIRECTION_HORIZONTAL,
        OC_DIRECTION_VERTICAL
    } OcDirection;

    /** @enum OcAutoThresholdMethod
     * @brief Thresholding method to use for auto threshold filter
     */
    typedef enum {
        OC_AUTO_THRESHOLD_MEAN,
        OC_AUTO_THRESHOLD_HUANG,
        OC_AUTO_THRESHOLD_MIN,
        OC_AUTO_THRESHOLD_INTERMODES,
        OC_AUTO_THRESHOLD_PTILE,
        OC_AUTO_THRESHOLD_ITERBEST,
        OC_AUTO_THRESHOLD_OTSU,
        OC_AUTO_THRESHOLD_1DMAX,
        OC_AUTO_THRESHOLD_MOMENT,
        OC_AUTO_THRESHOLD_KITTLER,
        OC_AUTO_THRESHOLD_ISODATA,
        OC_AUTO_THRESHOLD_SHANBHAG,
        OC_AUTO_THRESHOLD_YEN,
    } OcAutoThresholdMethod;

    /** @enum OcToneBalanceMode 
     * @brief Tone balance mode to use for color balance filter
    */
    typedef enum {
        SHADOWS,
        MIDTONES,
        HIGHLIGHTS
    } OcToneBalanceMode;

    /** @enum OcRetinexMode
     * @brief Retinex mode to use for multiscale retinex filter
    */
    typedef enum {
        RETINEX_UNIFORM,
        RETINEX_LOW,
        RETINEX_HIGH
    } OcRetinexMode;


    //--------------------------Color adjustments--------------------------

    //    /** @brief Converts an RGB image to single channel grayscale (a slightly faster implementation of the saturation filter,
    //     *  without the ability to vary the color contribution)
    //     *  @ingroup group_color_filters
    //     *  @param Input The image input data buffer.
    //     *  @param Output The image output data buffer.
    //     *  @param Width The width of the image in pixels.
    //     *  @param Height The height of the image in pixels.
    //     *  @param Stride The number of bytes in one row of pixels.
    //     */
    //    OC_STATUS ocularGrayscaleFilter(OcImage* Input, OcImage* Output);

    /** @brief Converts an RGB image to single channel grayscale (a slightly faster implementation of the saturation filter,
     *  without the ability to vary the color contribution)
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularGrayscaleFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

    /** @brief Adjusts the individual RGB channels of an image
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param redAdjustment Normalized values by which each color channel is
     *  multiplied. Range [0.0-1.0], default 1.0
     *  @param greenAdjustment Normalized values by which each color channel is
     *  multiplied. Range [0.0-1.0], default 1.0
     *  @param blueAdjustment Normalized values by which each color channel is
     *  multiplied. Range [0.0-1.0], default 1.0
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularRGBFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float redAdjustment,
                         float greenAdjustment, float blueAdjustment);

    /**
     * @brief Adjusts the Hue, Saturation and Luminance of an image.
     * @ingroup group_color_filters
     * @param Input The image input data buffer.
     * @param Output The image output data buffer.
     * @param Width The width of the image in pixels.
     * @param Height The height of the image in pixels.
     * @param Stride The number of bytes in one row of pixels.
     * @param hueAdjustment The hue modifier to apply. Range [-0.5 - 0.5]
     * @param satAdjustment The saturation modifier to apply.  Range [-2.0 - 2.0]
     * @param lightAdjustment The lightness modifier to apply. Range [-1.0 - 1.0]
     * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularHSLFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float hueAdjustment,
                         float satAdjustment, float lightAdjustment);

    /** @brief Applies a thresholding operation where the threshold is continually adjusted based on the average luminance of the image.
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param thresholdMultiplier Factor that the average luminance will be multiplied by in order to arrive at the final threshold to use.
     *  Default is 1.0.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularAverageLuminanceThresholdFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float thresholdMultiplier);

    /** @brief Determines the average color, by averaging the RGBA components for each each pixel in an image.
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param[out] AverageR Returns the calculated red channel value.
     *  @param[out] AverageG Returns the calculated green channel value.
     *  @param[out] AverageB Returns the calculated blue channel value.
     *  @param[out] AverageA Returns the calculated alpha channel value.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularAverageColor(unsigned char* Input, int Width, int Height, int Stride, unsigned char* AverageR, unsigned char* AverageG,
                            unsigned char* AverageB, unsigned char* AverageA);

    /** @brief Calculates the average luminosity for an image.
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param[out] Luminance Returns average luminosity value.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularLuminosity(unsigned char* Input, int Width, int Height, int Stride, unsigned char* Luminance);

    /** @brief Transforms the colors of an image by apply a matrix to them.
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param colorMatrix  A 4x4 matrix used to transform each color in an image.
     *  @param intensity The degree to which the new transformed color replaces the
     *  original color for each pixel. Range [0.0-1.0]. Default 1.0.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularColorMatrixFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                     float* colorMatrix, float intensity);

    /** @brief Applies a simple sepia tone filter
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param intensity The degree to which the sepia tone replaces the normal image color. Range [0 - 100].
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularSepiaFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                int intensity);

    /** @brief For a given color in the image, sets the alpha channel to 0.
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param colorToReplaceR The red channel to replace.
     *  @param colorToReplaceG The green channel to replace.
     *  @param colorToReplaceB The blue channel to replace.
     *  @param thresholdSensitivity How close a color match needs to exist to the target color to be replaced. Default 0.4.
     *  @param smoothing How smoothly to blend for the color match. Default 0.1.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularChromaKeyFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, unsigned char colorToReplaceR,
                               unsigned char colorToReplaceG, unsigned char colorToReplaceB, float thresholdSensitivity, float smoothing);

    /** @brief Uses an RGB color lookup image to remap the colors in an image.
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param lookupTable A matrix used to lookup each replacement color.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param intensity The intensity of the lookup filter. Range [0 - 100].
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularLookupFilter(unsigned char* Input, unsigned char* Output, unsigned char* lookupTable, int Width, int Height, int Stride, int intensity);

    /** @brief Adjusts the saturation of an image
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param saturation The degree of saturation or desaturation to apply to the
     *  image. Range [0.0 - 2.0]. Default 1.0.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularSaturationFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float saturation);

    /** @brief Adjusts the gamma of an image
     *  @ingroup group_color_filters group_inplace
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param gamma The gamma adjustment to apply to each channel. Range [> 0 to ~4 or 5]. Supports 1 or 3 channels.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularGammaFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float gamma[]);

    /** @brief Adjusts the contrast of the image
     *  @ingroup group_color_filters group_inplace
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param contrast The adjusted contrast. Range [0.0 - 4.0]. Default 1.0.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularContrastFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float contrast);

    /** @brief Adjusts the exposure of the image
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param exposure The adjusted exposure. Range [-10.0 - 10.0]. Default 0.0.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularExposureFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float exposure);

    /** @brief Adjusts the brightness of the image
     *  @ingroup group_color_filters group_inplace
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param brightness The adjusted brightness. Range [-1.0 - 1.0]. Default 0.0.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularBrightnessFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int brightness);

    /** @brief Detects the dark and bright areas of an image, and replaces them with respective colors.
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param firstColorR The red channel to replace in dark areas of the image.
     *  @param firstColorG The green channel to replace in dark areas of the image.
     *  @param firstColorB The blue channel to replace in dark areas of the image.
     *  @param secondColorR The red channel to replace in light areas of the image.
     *  @param secondColorG The green channel to replace in light areas of the image.
     *  @param secondColorB The blue channel to replace in light areas of the image.
     *  @param intensity The luminance intensity to use for dark and light areas. Range [0 - 100].
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularFalseColorFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, unsigned char firstColorR,
                                unsigned char firstColorG, unsigned char firstColorB, unsigned char secondColorR,
                                unsigned char secondColorG, unsigned char secondColorB, int intensity);

    /** @brief Used to add or remove haze (similar to a UV filter)
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param distance Strength of the color applied. Default 0. Values between -0.3 and 0.3 are best.
     *  @param slope Amount of color change. Default 0. Values between -0.3 and 0.3 are best.
     *  @param intensity The luminance intensity to apply. Range [0 - 100]. Default 100.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularHazeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float distance, float slope, int intensity);

    /** @brief Adjusts the alpha channel of the image
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param opacity The value to multiply the incoming alpha channel for each pixel. Range [0.0 - 1.0]. Default 1.0.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularOpacityFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float opacity);

    /** @brief Photoshop-like levels adjustment. The min, max, minOut and maxOut parameters are floats in the range [0 - 1].
     *  If you have parameters from Photoshop in the range[0, 255] you must first convert them to be [0 - 1]. The gamma / mid
     *  parameter is a float >= 0. This matches the value from Photoshop. If you want to apply levels to RGB as well as
     *  individual channels you need to use this filter twice, first for the individual channels and then for all channels.
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param redLevelParams The red channel for parameters for min, mid, max and output values.
     *  @param greenLevelParams The green channel for parameters for min, mid, max and output values.
     *  @param blueLevelParams The blue channel for parameters for min, mid, max and output values.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularLevelsFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                            ocularLevelParams* redLevelParams, ocularLevelParams* greenLevelParams, ocularLevelParams* blueLevelParams);

    /** @brief Adjust the hue of an image
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param hueAdjust The hue angle, in degrees. 90 degrees by default.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularHueFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float hueAdjust);

    /** @brief Allows you to tint the shadows and highlights of an image independently using a color and intensity.
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param shadowTintR Shadow tint red channel adjustment. Range [0 - 1.0]
     *  @param shadowTintG Shadow tint green channel adjustment. Range [0 - 1.0]
     *  @param shadowTintB Shadow tint blue channel adjustment. Range [0 - 1.0]
     *  @param highlightTintR Highlight tint red channel adjustment. Range [0 - 1.0]
     *  @param highlightTintG Highlight tint green channel adjustment. Range [0 - 1.0]
     *  @param highlightTintB Highlight tint blue channel adjustment. Range [0 - 1.0]
     *  @param shadowTintIntensity Shadow tint intensity. Range [0 - 1.0]
     *  @param highlightTintIntensity Highlight tint intensity. Range [0 - 1.0]
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularHighlightShadowTintFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float shadowTintR,
                                         float shadowTintG, float shadowTintB, float highlightTintR, float highlightTintG,
                                         float highlightTintB, float shadowTintIntensity, float highlightTintIntensity);

    /** @brief Adjusts the shadows and highlights of an image
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param shadows Increase to lighten shadows, from 0.0 to 1.0, with 0.0 as the default.
     *  @param highlights Decrease to darken highlights, from 0.0 to 1.0, with 1.0 as the default.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularHighlightShadowFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float shadows, float highlights);

    /** @brief Converts the image to a single color version, based on the luminance of each pixel
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param filterColorR The red channel color to use for the effect.
     *  @param filterColorG The green channel color to use for the effect.
     *  @param filterColorB The blue channel color to use for the effect.
     *  @param intensity The degree to which the specific color replaces the normal
     *  image color. Range [0.0 - 1.0]. Default 100.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularMonochromeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, unsigned char filterColorR,
                                unsigned char filterColorG, unsigned char filterColorB, int intensity);

    /** @brief Inverts the colors of an image
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularColorInvertFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

    /** @brief Outputs a generated image with a solid color.
     *  @ingroup group_ip_general
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param colorR The red channel color to use. Range [0-255]
     *  @param colorG The green channel color to use. Range [0-255]
     *  @param colorB The blue channel color to use. Range [0-255]
     *  @param colorAlpha The alpha channel value to use. Range [0-255]
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularSolidColorGenerator(unsigned char* Output, int Width, int Height, int Stride, unsigned char colorR, unsigned char colorG,
                                   unsigned char colorB, unsigned char colorAlpha);

    /** @brief Converts an image to black and white based on a luminance threshold.
     *  Pixels with a luminance above the threshold will appear white, and those below will be black.
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param threshold The luminance threshold. Range [0 - 255].
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularLuminanceThresholdFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, unsigned char threshold);

    /**
     * @brief Performs an automatic contrast enhancement on an image based on local and image-dependent exponential correction.
     * This method helps correct images that have both overexposed and underexposed regions.
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Channels The numbers of color channels in the image.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularAutoContrast(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels);

    /**
     * @brief Performs automatic gamma correction based on average brightness.
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularAutoGammaCorrection(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

    /** @brief Automatically applies a neutral white balance to an image.
     *  @ingroup group_color_filters
     *  @param input The image input data buffer.
     *  @param output The image output data buffer.
     *  @param width The width of the image in pixels.
     *  @param height The height of the image in pixels.
     *  @param channels The number of color channels in the image.
     *  @param colorCoeff Used to measure the intensity of color casting. Range [0 - 127]
     *  @param cutLimit The histogram clipping upper/lower limit. Range [0 - 1.0]
     *  @param contrast The histogram contrast strength. Range [0 - 1.0]
     *  @param hasColorCast True, if the image has a color cast. Otherwise false.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularAutoWhiteBalance(unsigned char* input, unsigned char* output, int Width, int height, int stride,
                                int colorCoeff, float cutLimit, float contrast, bool* hasColorCast);

    /** @brief Adjust the white balance of of an image
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param temperature The temperature to adjust the image by in Kelvin. A value of 4000 is very cool and 7000 very warm.
     *  The default value is 5000. Note that the scale between 4000 and 5000 is nearly as visually significant as
     *  that between 5000 and 7000.
     *  @param tint Tints the image either towards green or magenta. Useful for removing color casts. A value of -200
     *  is very green and 200 is very pink.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularWhiteBalanceFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float temperature, float tint);

    /** @brief Adjusts the vibrance of an image
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param vibrance The vibrance adjustment to apply. Range [0.0 - 1.0]
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularVibranceFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float vibrance);

    /** @brief A skin-tone adjustment filter that affects a unique range of light  skin-tone colors and adjusts the pink/green
     *  or pink/orange range accordingly. Default values are targeted at fair caucasian skin, but can be adjusted as required.
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param skinToneAdjust Amount to adjust skin tone. Default : 0.0, suggested min/max : -0.3 and 0.3 respectively.
     *  @param skinHue Skin hue to be detected. Default : 0.05 (fair caucasian to reddish skin).
     *  @param skinHueThreshold Amount of variance in skin hue. Default : 40.0.
     *  @param maxHueShift Maximum amount of hue shifting allowed. Default : 0.25.
     *  @param maxSaturationShift Maximum amount of saturation to be shifted (when using orange). Default: 0.4.
     *  @param upperSkinToneColor Adjusts hue or saturation based on what tone we are aiming for. 0 = Pink/Green, 1 = Pink/Orange.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularSkinToneFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float skinToneAdjust,
                              float skinHue, float skinHueThreshold, float maxHueShift, float maxSaturationShift, int upperSkinToneColor);

    /** @brief Automatically calculates levels of an image.
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param fraction Clipping threshold in percentage of pixels allowed to clip to clip to the white and black points
     *  of the histogram. Range [0.001 - 0.1].
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularAutoLevel(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float fraction);

    /**
     * @brief Performs a tone equalization by redistributing the brightness values of the pixels in an image so
     * that they more evenly represent the entire range of brightness levels.
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer. Currently only RGB supported.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularEqualizeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);


    /**
     * @brief Performs a histogram stretch (contrast stretching) on an image.
     * @ingroup group_color_filters
     * @param input The image input data buffer.
     * @param output The image output data buffer.
     * @param width The width of the image in pixels.
     * @param height The height of the image in pixels.
     * @param channels The number of color channels in the image.
     * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularHistogramStretch(uint8_t* input, uint8_t* output, int width, int height, int channels);

    /**
     * @brief Performs global thresholding using various methods.
     * Pixels above calculated threshold value are turned to white, else black.
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer. Requires single channel image.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param method The method to use for automatically calculating threshold value. Good default OC_AUTO_THRESHOLD_OTSU.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularAutoThreshold(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, OcAutoThresholdMethod method);

    /**
     * @brief Attempts to correct images that were captured under extremely low or non-uniform lighting conditions.
     * This is an excellent non-linear color enhancement that combines global curve adjustment and local information.
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer. Requires RGB image.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularBacklightRepair(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

    /**
     * @brief Applies a Photoshop-style layer blending mode to an image using a secondary image to mix with.
     * @ingroup group_color_filters
     * @param baseInput The base image input data buffer. Effect is applied to this data.
     * @param bWidth The width of the base image in pixels.
     * @param bHeight The height of the base image in pixels.
     * @param bStride The number of bytes in one row of pixels for base image.
     * @param mixInput The mix image input data buffer.
     * @param mWidth The width of the base image in pixels.
     * @param mHeight The height of the base image in pixels.
     * @param mStride The number of bytes in one row of pixels for mix image.
     * @param blendMode The blending mode to apply. All 27 Photoshop blend are available.
     * @param alpha The transparency to apply. Range [0 - 100].
     * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularLayerBlend(unsigned char* baseInput, int bWidth, int bHeight, int bStride, unsigned char* mixInput, int mWidth, int mHeight,
                          int mStride, OcBlendMode blendMode, int alpha);


    /**
     * @brief Allows changing the overall mixture of colors in an image to correct color casts.
     * @ingroup group_color_filters
     * @param Input The image input data buffer.
     * @param Output The image output data buffer.
     * @param Width The width of the image in pixels.
     * @param Height The height of the image in pixels.
     * @param Stride The number of bytes in one row of pixels.
     * @param redBalance The amount to adjust the red channel. Range [-100 - 100].
     * @param greenBalance The amount to adjust the green channel. Range [-100 - 100].
     * @param blueBalance The amount to adjust the blue channel. Range [-100 - 100].
     * @param Mode The tonal range to apply the color balance to. [SHADOWS|MIDTONES|HIGHLIGHTS].
     * @param preserveLuminosity Prevents changing the luminosity values in the image while changing the color.
     * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularColorBalance(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int redBalance,
                           int greenBalance, int blueBalance, OcToneBalanceMode Mode, bool preserveLuminosity);

    /**
     * @brief Performs multiscale retinex with color restoration. This combines color constancy with local contrast
     * enhancement so images are rendered similiar to how the human eye perceives.
     * @ingroup group_color_filters     
     * @param input The image input data buffer.
     * @param output The image output data buffer.
     * @param width The width of the image in pixels.
     * @param height The height of the image in pixels.
     * @param channels The number of color channels in the image.
     * @param mode The mode to use for retinex. [RETINEX_UNIFORM|RETINEX_LOW|RETINEX_HIGH]. 
     * RETINEX_UNIFORM treats all image intensities similiarly. RETINEX_LOW enhances dark regions of the image. 
     * RETINEX_HIGH enhances bright regions of the image.
     * @param scale Specifics the depth of the retinex effect. Range [16.0 - 250.0]
     * @param numScales Specifies the number of iterations of the multiscale filter. Values larger than 2 take advantage 
     * of the "multiscale" nature of the algorithm. Range [1.0 - 8.0]
     * @param dynamic Adjusts the color of the result. Large values produce less saturated images. Range [0.05 - 4.0]
     * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularMultiscaleRetinex(unsigned char* input, unsigned char* output, int width, int height, int channels,
                                  OcRetinexMode mode, int scale, float numScales, float dynamic);

    //--------------------------Color adjustments--------------------------

    //--------------------------Blur filters--------------------------

    /** @brief A hardware-optimized, variable radius box blur
     *  @ingroup group_ip_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param Radius A radius in pixels to use for the blur, >= 0.0. This adjusts the sigma variable in the Gaussian distribution function.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularBoxBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

    /** @brief Performs a smoothing of an image using a discrete Gaussian kernel.
     *  Reduces noise by averaging it out, but will also reduce edges.
     *  Note: This is a radius-independent fast gaussian blur implementation.
     *  @ingroup group_ip_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param GaussianSigma A radius in pixels to use for the blur, >= 0.0
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularGaussianBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float GaussianSigma);

    /**
     * @brief Performs an expontential blur on an where the intensity of hte blur gradually decreases
     * as the distance from the center of the center pixel increases, following an exponential decay pattern.
     * Essentially creating a more pronounced blur near the center and a softer fade towards the edges, often 
     * used to simulate realistic light falloff or glow effects
     * @ingroup group_ip_filters
     * @param Input The image input data buffer.
     * @param Output The image output data buffer.
     * @param Width The width of the image in pixels.
     * @param Height The height of the image in pixels.
     * @param Channels The number of color channels in the image.
     * @param Radius The radius of the blur. Range [1.0 - 100.0]
     * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularExponentialBlur(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, float Radius);

    /** @brief Performs a convolution blurring that simulates the effect of shooting a moving object on film.
     *  @ingroup group_ip_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param distance The distance in pixels to use for the blur, >= 0.0
     *  @param angle The angle of the blur. Range [-180 - 180]
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularMotionBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Distance, int Angle);

    /**
     * @brief Performs a rotational blur on an image centered on a point.
     * @ingroup group_ip_filters
     * @param Input The image input data buffer.
     * @param Output The image output data buffer.
     * @param Width The width of the image in pixels.
     * @param Height The height of the image in pixels.
     * @param Stride The number of bytes in one row of pixels.
     * @param centerX The x coordinate to center the blur at. Centered: image width / 2.
     * @param centerY The y coordinate to center the blur at. Centered: image height / 2.
     * @param intensity The strength of the blur to apply. Range [0 - 100]
     * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularRadialBlur(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int centerX, int centerY, int intensity);

    /**
     * @brief Apply zoom blur to the image. This effect mimics the zoom of a camera when capturing the image.
     * @ingroup group_ip_filters
     * @param Input The input image data.
     * @param Output The output image data.
     * @param Width The width of the image.
     * @param Height The height of the image.
     * @param Stride The number of bytes in one row of pixels.
     * @param sampleRadius The radius of the zoom blur. Range [10 - 200]
     * @param blurAmount The amount of blur to apply. Range [0.1 - 1.0]
     * @param centerX The x coordinate of the center of the blur.
     * @param centerY The y coordinate of the center of the blur.
     * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularZoomBlur(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int sampleRadius, float blurAmount,
                        int centerX, int centerY);

    /**
     * @brief Applies a average (mean) blur to an image that that replaces each pixel with the average of of all values in the local area.
     * @ingroup group_ip_filters
     * @param Input The image input data buffer.
     * @param Output The image output data buffer.
     * @param Width The width of the image in pixels.
     * @param Height The height of the image in pixels.
     * @param Stride The number of bytes in one row of pixels.
     * @param Radius A radius in pixels to use for the blur, >= 0.0
     * @param edgeMode Edge handling mode when out of bounds. OC_EDGE_WRAP or OC_EDGE_MIRROR.
     * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularAverageBlur(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

    /** @brief Applies a median blur to an image, which is good for removing salt and pepper noise.
     *  @ingroup group_ip_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param Radius A radius in pixels to use for the blur, >= 0.0
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularMedianBlur(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

    /**
     * @brief Performs a minimum rank filter that replaces the central pixel with the darkest one in the radius.
     *  @ingroup group_ip_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param Radius A radius in pixels to use for calculation, >= 0
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularErodeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

    /**
     * @brief Performs a maximum rank filter that replaces the central pixel with the lightest one in the radius.
     *  @ingroup group_ip_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param Radius A radius in pixels to use for calculation, >= 0
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularDilateFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

   /**
     * @brief Apply a minimum filter. This is the same as Erode except it uses a circular kernel that accounts for edges.
     * @param Input The image input data buffer
     * @param Output The image output data buffer
     * @param Width The width of the image in pixels
     * @param Height The height of the image in pixels
     * @param Stride The number of bytes in one row of pixels
     * @param Radius The radius of the kernel. Range [1 - 256]
     * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularMinFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

    /**
     * @brief Apply a maximum filter. This is the same as Dialte except it uses a circular kernel that accounts for edges.
     * @param Input The image input data buffer
     * @param Output The image output data buffer
     * @param Width The width of the image in pixels
     * @param Height The height of the image in pixels
     * @param Stride The number of bytes in one row of pixels
     * @param Radius The radius of the kernel. Range [1 - 256]
     */
    OC_STATUS ocularMaxFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

    /**
     * @brief Perform a high pass filter that emphasizes high-frequency components (edges and details) while reducing
     * low-frequency components (smooth areas)
     * @ingroup group_ip_filters
     * @param Input The image input data buffer.
     * @param Output The image output data buffer.
     * @param Width The width of the image in pixels.
     * @param Height The height of the image in pixels.
     * @param Stride The number of bytes in one row of pixels.
     * @param Radius Controls the cutoff frequency - larger values will remove more low frequency content, resulting in
     * a more pronounced high pass effect. Range [1 - 512]
     * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularHighPassFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

    /**
     * @brief Performs an optimized blurring of an image, maintaining edges while reducing and smoothing out noise.
     * @ingroup group_ip_filters
     * @param Input The image input data buffer.
     * @param Output The image output data buffer.
     * @param Width The width of the image in pixels.
     * @param Height The height of the image in pixels.
     * @param Stride The number of bytes in one row of pixels.
     * @param Radius The size of the sampling area. Range [1 - 127].
     * @param Threshold Controls how much the tonal values of neighboring pixels must diverge from the center pixel before being
     * included in the blur. Range [2 - 255].
     * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularSurfaceBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius, int Threshold);

    /**
     * @brief Performs a Bi-Exponential Edge-Perserving Smoothing filter that removes irrelevant details while preserving strong edges.
     *        It is faster than a bilateral filter and uses a range akin to the one found in bilateral.
     * @ingroup group_ip_filters
     * @param Input The image input data buffer.
     * @param Output The image output data buffer.
     * @param Width The width of the image in pixels.
     * @param Height The height of the image in pixels.
     * @param Stride The number of bytes in one row of pixels.
     * @param PhotometricStandardDeviation Controls the broadness of the range filter.  
     * The larger the value, the more obvious the blur. Range [1 - 255]
     * @param SpatialDecay Spatial standard deviation. The larger the value, the more obvious the blur.
     *  Range [0.01 - 0.250]
     * @param RangeFilter The type of range filter to use. [0, 1 or 2] [Gaussian|Hyperbolic Secant|Euler Constant]
     * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularBEEPSFilter(const unsigned char* Input, unsigned char* Output, int width, int height, int Stride,
                           float PhotometricStandardDeviation, float SpatialDecay, int RangeFilter);

    //--------------------------Blur filters--------------------------


    //-------------------------- Edge detection -----------------`---------

    // -------------------------- Used for Canny Edge Detection ---------------------------

    typedef enum
    {
        CannyGaus3x3,
        CannyGaus5x5
    } CannyNoiseFilter;

    // Gaussian blur
    // 3 x 3 kernel
    static const int8_t Gaus3x3[] = { 1, 2, 1, 2, 4, 2, // * 1/16
                                        1, 2, 1 };
    static const int Gaus3x3Div = 16;

    static const int8_t Gaus5x5[] = { 2, 4, 5,  4, 2, 4, 9, 12, 9, 4, 5, 12, 15, 12, 5, // * 1/159
                                        4, 9, 12, 9, 4, 2, 4, 5,  4, 2 };
    static const int Gaus5x5Div = 159;

    // -------------------------- Used for Canny Edge Detection ---------------------------

    /**
     * @brief Performs Canny edge detection on an image.
     * This is one of the most reliable methods of edge detection.
     * @ingroup group_ip_filters
     * @param Input The image input data buffer (expects grayscale image).
     * @param Output The image output data buffer.
     * @param Width The width of the image in pixels.
     * @param Height The height of the image in pixels.
     * @param kernel_size The type of gaussian noise to apply. Gaus3x3 or Gaus5x5. Good default Gaus3x3
     * @param weak_threshold Pixel values below this limit are discarded. Range [0 - 255]. Good default 200.
     * @param strong_threshold Pixel values above this limit are to be considered edge pixels. Range [0 - 255]. 
     * Good default 200.
     * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularCannyEdgeDetect(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels,
                                CannyNoiseFilter kernel_size, int weak_threshold, int strong_threshold);

    /** @brief Applies a sobel edge detection filter
     *  @ingroup group_ip_filters
     *  @param Input The image input data buffer (expects grayscale image).
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Channels The numbers of color channels in the image.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularSobelEdgeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels);

    /**
     * @brief Performs edge detection on an image based on the Sobel operator.
     * @ingroup group_ip_filters group_inplace
     * @param Input The image input data buffer (expects grayscale image).
     * @param Output The image output data buffer.
     * @param Width The width of the image in pixels.
     * @param Height The height of the image in pixels.
     * @param Channels The numbers of color channels in the image.
     * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularGradientEdgeDetect(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels);

    //-------------------------- Edge detection --------------------------

    //--------------------------Enhancement filters--------------------------

    /** @brief Performs a non-linear, edge-preserving and noise-reducing smoothing of an image.
     *  This is a fast implementation, like gaussian blur, that performs vertical/horizontal passes independently.
     *  @ingroup group_ip_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param sigmaSpatial The size of the Gaussian bilateral filter window to use. A larger value means that farther
     *  pixels will influence each other as long as their colors are close enough. Range [0 - 1.0]
     *  @param sigmaRange Control how much an adjacent pixel is down-weighted because of the intensity difference. A
     *  larger value means that farther colors within the pixel neighborhood will be mixed together. Range [0 - 1.0]
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularBilateralFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float sigmaSpatial, float sigmaRange);

    /** @brief A better sharpening effect using a gaussian blur as a mask for enhancing edges.
     *  @ingroup group_ip_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param GaussianSigma The blur radius of the underlying Gaussian blur.
     *  @param intensity The strength of the sharpening kernel. Range [0.0-4.0].
     *  @param threshold Pixels with a difference greater than this value will be sharpened. 
     *  Represented as a percentage of the difference. Range [0-100%].
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularUnsharpMaskFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float GaussianSigma, float intensity, float threshold);

    /** @brief Enhances the contrast between pixels to make details and edges more pronounced.
     *  @ingroup group_ip_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param Strength The strength of the sharpening kernel. Range [0.0 - 10.0].
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularSharpenFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float Strength);

    /**
     * @brief Applies denoising smoothing filter on detected skin region while retaining other details.
     * @ingroup group_ip_filters
     * @param Input The image input data buffer.
     * @param Output The image output data buffer.
     * @param Width The width of the image in pixels.
     * @param Height The height of the image in pixels.
     * @param Stride The number of bytes in one row of pixels.
     * @param smoothingLevel The amount of skin denoising to apply.
     * @param applySkinFilter Apply filtering to non-skin areas after initial denoising.
     * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularSkinSmoothingFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int smoothingLevel,
                                       bool applySkinFilter);

    //--------------------------Enhancement filters--------------------------

    //--------------------------Misc--------------------------

    /** @brief Applies a 2D convolution to an image using a kernel.
     *  @ingroup group_ip_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Channels The number of color channels in the image.
     *  @param kernel The kernel matrix to apply. Width and height must be odd. This expects a 1D array.
     *  @param filterW The kernel matrix width.
     *  @param cfactor The sum of all values greater than 0 in the kernel.
     *  @param bias Used to increase/decrease all values greater than 0 in the kernel.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularConvolution2DFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, float* kernel,
                                       unsigned char filterW, unsigned char cfactor, unsigned char bias);

    /** @brief Resizes an image using Nearest-neighbor, Bilinear, Bicubic or Lanczos interpolation.
     *  @ingroup group_ip_general
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param newWidth The new width of the image.
     *  @param newHeight The new height of the image.
     *  @param dstStride The number of bytes in one row of pixels for output. Used to calculate destination offset.
     *  @param InterpolationMode The interpolation mode to use. 
     *  [OC_INTERPOLATION_NEAREST_NEIGHBOR, OC_INTERPOLATION_BILINEAR, OC_INTERPOLATION_BICUBIC, OC_INTERPOLATION_LANCZOS]
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularResamplingFilter(unsigned char* Input, int Width, int Height, unsigned int Stride,
                                    unsigned char* Output, int newWidth, int newHeight, int dstStride, 
                                    OcInterpolationMode InterpolationMode);

    /**
     * @brief Rotates an image using bilinear or bicubic interpolation. Non-image areas are filled with color or transparency.
     * @ingroup group_ip_general
     * @param Input The image input data buffer.
     * @param Width The width of the image in pixels.
     * @param Height The height of the image in pixels.
     * @param Stride The number of bytes in one row of pixels.
     * @param Output The image output data buffer.
     * @param angle The angle to rotate in degrees. Range [0 - 359].
     * @param useTransparency If true, we will use transparency instead of a solid color for non-image areas.
     * Must allocate Output buffer with one extra channel to hold the alpha channel. 2 channels if grayscale, 4 channels if color.
     * @param InterpolationMode The interpolation method to use. [OC_INTERPOLATE_BILINEAR, OC_INTERPOLATE_BICUBIC]
     * @param fillColorR The red channel value to use for filling non-image area. Used if channels = 1.
     * @param fillColorG The green channel value to use for filling non-image area.
     * @param fillColorB The blue channel value to use for filling non-image area.
     * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularRotateImage(unsigned char* Input, int Width, int Height, int Stride, unsigned char* Output, 
                                 int newWidth, int newHeight, float angle, bool useTransparency, 
                                 OcInterpolationMode InterpolationMode, unsigned char fillColorR, unsigned char fillColorG, 
                                 unsigned char fillColorB);

    /** @brief Outputs only a selected portion of an image.
     *  @ingroup group_ip_general
     *  @param Input The image input data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param srcStride The number of bytes in one row of pixels.
     *  @param Output The image output data buffer.
     *  @param cropX Starting X coordinate to start crop.
     *  @param cropY Starting Y coordinate to start crop.
     *  @param dstWidth The crop width of the image.
     *  @param dstHeight The crop height of the image.
     *  @param dstStride The number of bytes in one row of pixels for output. Used to calculate destination offset.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularCropImage(const unsigned char* Input, int Width, int Height, int srcStride, unsigned char* Output, int cropX, int cropY,
                            int dstWidth, int dstHeight, int dstStride);

    /**
     *  @brief Flip image horizontally or vertically.
     *  @ingroup group_ip_general
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Channels The number of color channels in the image.
     *  @param direction The direction of transformation to perform.
     *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularFlipImage(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, OcDirection direction);

    /**
     * @brief Performs an adaptive median filter on an image, useful for removing salt and pepper noise from scanned documents.
     * @ingroup group_ip_general group_ip_ocr
     * @param Input The image input data buffer.
     * @param Output The image output data buffer.
     * @param Width The width of the image in pixels.
     * @param Height The height of the image in pixels.
     * @param Stride The number of bytes in one row of pixels.
     * @param maxWindowSize The filter window size (should be a scalar between 1 and 7). Window size (ws) is defined as
                            W = 2*ws + 1 so that W = 3 is a 3x3 filter window.
        * @param Threshold The adaptive theshold (0 = normal median behavior). Higher values reduce the "aggresiveness" of the filter.
        * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
        */
    OC_STATUS ocularDespeckle(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int maxWindowSize, int Threshold);

    /**
     * @brief Performs deskewing of an image, useful for scanned documents.
     * @ingroup group_ip_general group_ip_ocr
     * @param Input The image input data buffer.
     * @param Output The image output data buffer.
     * @param Width The width of the image in pixels.
     * @param Height The height of the image in pixels.
     * @param Stride The number of bytes in one row of pixels.
     * @param Skewed True, if valid text image found and deskew performed. Otherwise false.
     * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularDocumentDeskew(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, bool* Skewed);

    //------------------------Distort-------------------------

    /**
     * @brief Applies a pixelate effect (sometimes called "mosaic") to an image.
     * @ingroup group_ip_filters
     * @param Input The image input data buffer.
     * @param Output The image output data buffer.
     * @param Width The width of the image in pixels.
     * @param Height The height of the image in pixels.
     * @param Stride The number of bytes in one row of pixels.
     * @param blockSize The width and height of the desired pixelation block (in pixels).
     * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularPixelateFilter(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int blockSize);

    /**
     * @brief Applies an "oil painting" effect to an image.
     * @ingroup group_ip_filters
     * @param Input The image input data buffer.
     * @param Output The image output data buffer.
     * @param Width The width of the image in pixels.
     * @param Height The height of the image in pixels.
     * @param Stride The number of bytes in one row of pixels.
     * @param radius The radius of the effect. Range >= 1.
     * @param intensity The smoothness of the effect. Smaller values indicate less smoothness (less bins are used to calculate luminance).
     * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularOilPaintFilter(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int radius, int intensity);

    /**
     * @brief Simulates the image being observed through a layer of frosted glass by applying random pixel disturbance.
     * @ingroup group_ip_filters
     * @param Input The input image data.
     * @param Output The output image data.
     * @param Height The height of the image.
     * @param Width The width of the image.
     * @param Channels The number of channels in the image.
     * @param Radius The radius of the blur. Recommend keeping this value around 2.
     * @param Range Controls the range of random pixel displacement. Greater values will produce a more pronounced effect,
     * making the image more distorted. Range [1 - 20]
     * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     * */
    OC_STATUS ocularFrostedGlassEffect(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                       int Radius, int Range);

    /**
     * @brief Applies a film grain effect to an image.
     * @ingroup group_ip_filters
     * @param Input The image input data buffer.
     * @param Output The image output data buffer.
     * @param Width The width of the image in pixels.
     * @param Height The height of the image in pixels.
     * @param Channels The number of color channels in the image.
     * @param Strength The intensity of the grain. Range [0.0 - 100.0].
     * @param Softness The softness of the grain. Range [0.0 - 25.0].
     * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularFilmGrainEffect(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, 
                                   float Strength, float Softness);

    /**
     * @brief Remaps the colors in an image using a palette file with optional dithering.
     * @ingroup group_color_filters
     * @param input The image input data buffer.
     * @param output The image output data buffer.
     * @param width The width of the image in pixels.
     * @param height The height of the image in pixels.
     * @param channels The number of color channels in the image.
     * @param filename The path to the palette file.
     * @param method The dithering method to use. See OcDitherMethod in dither.h.
     * @param amount The amount of dithering to apply. Range [0 - 100].
     * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularPalettetizeFromFile(unsigned char* input, unsigned char* output, int width, int height, int channels,
                                        const char* filename, OcDitherMethod method, int amount);

    /**
     * @brief Generates and applies an optimal palette from the image with a maximum number of unique colors using color 
     * quantization and optional dithering.
     * @ingroup group_color_filters
     * @param input The image input data buffer.
     * @param output The image output data buffer.
     * @param width The width of the image in pixels.
     * @param height The height of the image in pixels.
     * @param channels The number of color channels in the image.
     * @param quantizeMethod The quantization method to use. [OC_QUANTIZE_MEDIAN_CUT, OC_QUANTIZE_NEUQUANT]
     * @param maxColors The maximum number of colors to use in the palette.
     * @param ditherMethod The dithering method to use. See OcDitherMethod in dither.h.
     * @param ditherAmount The amount of dithering to apply. Range [0 - 100].
     * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularPalettetizeFromImage(unsigned char* input, unsigned char* output, int width, int height, int channels,
                                         OcQuantizeMethod quantizeMethod, int maxColors, OcDitherMethod ditherMethod, 
                                         int ditherAmount);

    /**
     * @brief Reduces the number of unique colors in an image by preforming segmentation using k-means clustering.
     * @ingroup group_color_filters
     * @param input The image input data buffer.
     * @param output The image output data buffer.
     * @param width The width of the image in pixels.
     * @param height The height of the image in pixels.
     * @param channels The number of color channels in the image.
     * @param levels The number of unique colors to reduce the image to. Range [2 - 255].
     * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularPosterizeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, int Levels);

    //------------------------Distort-------------------------

    //--------------------------Misc--------------------------

    /**
    *  @brief Performs a Hough transform to detect lines in an image.
    *  @ingroup group_ip_general
    *  @param Input The image input data buffer. Must be single channel, preferably processed through edge detection.
    *  @param[out] LineNumber The location where the number of detected lines are to be stored.
    *  @param[out] DetectedLine A location where parameters of detected lines are to be stored.
    *  @param Height The height of the input image.
    *  @param Width The width of the input image.
    *  @param threshold The maximum number of lines to return.
    *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
    */
    OC_STATUS ocularHoughLineDetection(unsigned char* Input, int* LineNumber, struct LineParameter* DetectedLine, int Height,
                                      int Width, int threshold);

    /**
     * @brief Simply draws a straight line. More of a utility function.
     * @ingroup group_ip_general
     * @param canvas The image input data buffer.
     * @param width The width of the image in pixels.
     * @param height The height of the image in pixels.
     * @param stride The number of bytes in one row of pixels.
     * @param x1 Starting x coordinate.
     * @param y1 Starting y coordinate.
     * @param x2 Ending x coordinate.
     * @param y2 Ending y coordinate.
     * @param R The red color to draw line with.
     * @param G The green color to draw line with.
     * @param B The blue color to draw line with.
     * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularDrawLine(unsigned char* canvas, int width, int height, int stride, int x1, int y1, int x2, int y2, unsigned char R,
                        unsigned char G, unsigned char B);

    /**
     * Automatically detect palette format from a file and load palette.
     * @ingroup group_palette
     * @param filename The path to the palette file.
     * @param palette The palette to load the data into.
     * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
     */
    OC_STATUS ocularLoadPalette(const char* filename, OcPalette* palette);

    //--------------------------Misc--------------------------

    //--------------------------preImage processing--------------------------

    /**
     * @brief Quickly retrieve width and height from an image file without processing image. Supports PNG/GIF/JPEG/BMP/ICO files.
     * @ingroup group_ip_utility
     * @param file_path The absolute file path of the image file.
     * @param[out] width The returned image file width.
     * @param[out] height The returned image file height.
     * @param file_size[out] The returned file size in bytes.
     * @return True if able to get image size, otherwise False.
     */
    bool ocularGetImageSize(const char* file_path, int* width, int* height, int* file_size);

    //--------------------------preImage processing--------------------------

    /**
     * @brief Get the version of the library.
     * @ingroup group_ip_utility
     * @return The version of the library as string.
     */
    const char* ocularGetVersion();

    /**
     * @brief Get the status string for a given status code.
     * @ingroup group_ip_utility
     * @param status The status code returned by Ocular functions.
     * @return The status string.
     */
    const char* ocularGetStatusString(OC_STATUS status);

#ifdef __cplusplus
}
#endif
#endif  /* OCULAR_H */
