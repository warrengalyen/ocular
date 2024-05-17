/**
 * @file: ocular.h
 * @author Warren Galyen
 * Created: 1-29-2024
 * Last Updated: 2-20-2024
 * Last update: added backlight repair filter
 *
 * @brief Contains exported primary filter function definitions
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
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
     */
    void ocularGrayscaleFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

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
     */
    void ocularRGBFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float redAdjustment,
                         float greenAdjustment, float blueAdjustment);

    /**
     * @brief Adjusts the Hue, Saturation and Luminance of an image.
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     * @param hueAdjustment The hue modifier to apply. Range [-0.5 - 0.5]
     * @param satAdjustment The saturation modifier to apply.  Range [-2.0 - 2.0]
     * @param lightAdjustment The lightness modifier to apply. Range [-1.0 - 1.0]
     */
    void ocularHSLFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float hueAdjustment,
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
     */
    void ocularAverageLuminanceThresholdFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float thresholdMultiplier);

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
     */
    void ocularAverageColor(unsigned char* Input, int Width, int Height, int Stride, unsigned char* AverageR, unsigned char* AverageG,
                            unsigned char* AverageB, unsigned char* AverageA);

    /** @brief Calculates the average luminosity for an image.
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param[out] Luminance Returns average luminosity value.
     */
    void ocularLuminosity(unsigned char* Input, int Width, int Height, int Stride, unsigned char* Luminance);

    /** @brief Transforms the colors of an image by apply a matrix to them.
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param colorMatrix  A 4x4 matrix used to transform each color in an image.
     *  @param intensity The degree to which the new transformed color replaces the
     *  original color for each pixel. Range [0.0-1.0]. Default 1.0.
     */
    void ocularColorMatrixFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float* colorMatrix, float intensity);

    /** @brief Applies a simple sepia tone filter
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param intensity The degree to which the sepia tone replaces the normal image color. Range [0 - 100].
     */
    void ocularSepiaFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int intensity);

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
     */
    void ocularChromaKeyFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, unsigned char colorToReplaceR,
                               unsigned char colorToReplaceG, unsigned char colorToReplaceB, float thresholdSensitivity, float smoothing);

    /** @brief Uses an RGB color lookup image to remap the colors in an image.
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param lookupTable A matrix used to lookup each replacement color.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param intensity int intensity = 100
     */
    void ocularLookupFilter(unsigned char* Input, unsigned char* Output, unsigned char* lookupTable, int Width, int Height, int Stride, int intensity);

    /** @brief Adjusts the saturation of an image
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param saturation The degree of saturation or desaturation to apply to the
     *  image. Range [0.0 - 2.0]. Default 1.0.
     */
    void ocularSaturationFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float saturation);

    /** @brief Adjusts the gamma of an image
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param gamma The gamma adjustment to apply to each channel. Range [> 0 to ~4 or 5]. Supports 1 or 3 channels.
     */
    void ocularGammaFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float gamma[]);

    /** @brief Adjusts the contrast of the image
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param contrast The adjusted contrast. Range [0.0 - 4.0]. Default 1.0.
     */
    void ocularContrastFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float contrast);

    /** @brief Adjusts the exposure of the image
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param exposure The adjusted exposure. Range [-10.0 - 10.0]. Default 0.0.
     */
    void ocularExposureFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float exposure);

    /** @brief Adjusts the brightness of the image
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param brightness The adjusted brightness. Range [-1.0 - 1.0]. Default 0.0.
     */
    void ocularBrightnessFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int brightness);

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
     */
    void ocularFalseColorFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, unsigned char firstColorR,
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
     */
    void ocularHazeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float distance, float slope, int intensity);

    /** @brief Adjusts the alpha channel of the image
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param opacity The value to multiply the incoming alpha channel for each pixel. Range [0.0 - 1.0]. Default 1.0.
     */
    void ocularOpacityFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float opacity);

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
     */
    void ocularLevelsFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                            ocularLevelParams* redLevelParams, ocularLevelParams* greenLevelParams, ocularLevelParams* blueLevelParams);

    /** @brief Adjust the hue of an image
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param hueAdjust The hue angle, in degrees. 90 degrees by default.
     */
    void ocularHueFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float hueAdjust);

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
     */
    void ocularHighlightShadowTintFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float shadowTintR,
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
     */
    void ocularHighlightShadowFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float shadows, float highlights);

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
     *  image color. Range [0.0 - 1.0]. Default 1.0.
     */
    void ocularMonochromeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, unsigned char filterColorR,
                                unsigned char filterColorG, unsigned char filterColorB, int intensity);

    /** @brief Inverts the colors of an image
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     */
    void ocularColorInvertFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

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
     */
    void ocularSolidColorGenerator(unsigned char* Output, int Width, int Height, int Stride, unsigned char colorR, unsigned char colorG,
                                   unsigned char colorB, unsigned char colorAlpha);

    /** @brief Converts an image to black and white based on a luminance threshold.
     *  Pixels with a luminance above the threshold will appear white, and those below will be black.
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param threshold The luminance threshold. Range [0.0 - 1.0]. Default. 0.5.
     */
    void ocularLuminanceThresholdFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, unsigned char threshold);

    /**
     * @brief Performs an automatic contrast enhancement on an image based on local and image-dependent exponential correction.
     * This method helps correct images that have both overexposed and underexposed regions.
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Channels The numbers of color channels in the image.
     */
    void ocularAutoContrast(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels);

    /**
     * @brief Performs automatic gamma correction based on average brightness.
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     */
    void ocularAutoGammaCorrection(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

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
     *  @return bool Whether the image has a color cast.
     */
    bool ocularAutoWhiteBalance(unsigned char* input, unsigned char* output, int Width, int height, int channels, int stride,
                                int colorCoeff, float cutLimit, float contrast);

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
     */
    void ocularWhiteBalanceFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float temperature, float tint);

    /** @brief Adjusts the vibrance of an image
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param vibrance The vibrance adjustment to apply, using 0.0 as the default,
     *  and a suggested min/max of around - 1.2 and 1.2, respectively.
     */
    void ocularVibranceFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float vibrance);

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
     */
    void ocularSkinToneFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float skinToneAdjust,
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
     */
    void ocularAutoLevel(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float fraction);

    /**
     * @brief Performs a tone equalization by redistributing the brightness values of the pixels in an image so
     * that they more evenly represent the entire range of brightness levels.
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer. Currently only RGB supported.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     */
    void ocularEqualizeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

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
     */
    void ocularAutoThreshold(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, OcAutoThresholdMethod method);

    /**
     * @brief Attempts to correct images that were captured under extremely low or non-uniform lighting conditions.
     * This is an excellent non-linear color enhancement that combines global curve adjustment and local information.
     *  @ingroup group_color_filters
     *  @param Input The image input data buffer. Requires RGB image.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     */
    void ocularBacklightRepair(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

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
     */
    void ocularLayerBlend(unsigned char* baseInput, int bWidth, int bHeight, int bStride, unsigned char* mixInput, int mWidth, int mHeight,
                          int mStride, OcBlendMode blendMode, int alpha);

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
     */
    void ocularBoxBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

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
     */
    void ocularGaussianBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float GaussianSigma);

    /** @brief Performs a convolution blurring that simulates the effect of shooting a moving object on film.
     *  @ingroup group_ip_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param distance The distance in pixels to use for the blur, >= 0.0
     *  @param angle The angle of the blur. Range [-180 - 180]
     */
    void ocularMotionBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Distance, int Angle);

    /**
     * @brief Performs a rotational blur on an image centered on a point.
     *  @ingroup group_ip_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     * @param centerX The x coordinate to center the blur at. Centered: image width / 2.
     * @param centerY The y coordinate to center the blur at. Centered: image height / 2.
     * @param intensity The strength of the blur to apply. Range [0 - 100]
     */
    void ocularRadialBlur(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int centerX, int centerY, int intensity);

    /**
     * @brief Applies a average (mean) blur to an image that that replaces each pixel with the average of of all values in the local area.
     *  @ingroup group_ip_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param Radius A radius in pixels to use for the blur, >= 0.0
     * @param edgeMode Edge handling mode when out of bounds. OC_EDGE_WRAP or OC_EDGE_MIRROR.
     */
    void ocularAverageBlur(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius, OcEdgeMode edgeMode);

    /** @brief Applies a median blur to an image, which is good for removing salt and pepper noise.
     *  @ingroup group_ip_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param Radius A radius in pixels to use for the blur, >= 0.0
     */
    void ocularMedianBlur(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

    /**
     * @brief Performs a minimum rank filter that replaces the central pixel with the darkest one in the radius.
     *  @ingroup group_ip_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param Radius A radius in pixels to use for calculation, >= 0
     */
    void ocularErodeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

    /**
     * @brief Performs a maximum rank filter that replaces the central pixel with the lightest one in the radius.
     *  @ingroup group_ip_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param Radius A radius in pixels to use for calculation, >= 0
     */
    void ocularDilateFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

    /**
     * @brief Performs an optimized blurring of an image, maintaining edges while reducing and smoothing out noise.
     *  @ingroup group_ip_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     * @param Radius The size of the sampling area. Range [1 - 127].
     * @param Threshold Controls how much the tonal values of neighboring pixels must diverge from the center pixel before being
     * included in the blur. Range [2 - 255].
     */
    void ocularSurfaceBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius, int Threshold);

    //--------------------------Blur filters--------------------------


    //-------------------------- Edge detection -----------------`---------

    // -------------------------- Used for Canny Edge Detection ---------------------------

    typedef enum {
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
     *  @ingroup group_ip_filters
     *  @param Input The image input data buffer (expects grayscale image).
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     * @param kernel_size The type of gaussian noise to apply. Gaus3x3 or Gaus5x5. Good default Gaus3x3
     * @param weak_threshold Pixel values below this limit are discarded. Range [0 - 255]. Good default 200.
     * @param strong_threshold Pixel values above this limit are to be considered edge pixels. Range [0 - 255]. Good default 200.
     */
    void ocularCannyEdgeDetect(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels,
                               CannyNoiseFilter kernel_size, int weak_threshold, int strong_threshold);

    /** @brief Applies a sobel edge detection filter
     *  @ingroup group_ip_filters
     *  @param Input The image input data buffer (expects grayscale image).
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Channels The numbers of color channels in the image.
     */
    void ocularSobelEdgeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels);

    /**
     * @brief Performs edge detection on an image based on the Sobel operator.
     *  @ingroup group_ip_filters
     *  @param Input The image input data buffer (expects grayscale image).
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Channels The numbers of color channels in the image.
     */
    void ocularGradientEdgeDetect(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels);

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
     */
    void ocularBilateralFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float sigmaSpatial, float sigmaRange);

    /** @brief Applies an unsharp mask
     *  @ingroup group_ip_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param GaussianSigma The blur radius of the underlying Gaussian blur. The
     *  default is 4.0.
     *  @param intensity The strength of the sharpening, >= 0.0, with a default
     *  of 1.0.
     */
    void ocularUnsharpMaskFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float GaussianSigma, int intensity);

    /** @brief Applies a Gaussian sharpening filter to an image.
     *  @ingroup group_ip_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param Radius The radius of the sharpening kernel. The default is 4.0.
     *  @param sharpness The sigma of the gaussian, the smaller sigma is the more
     *  the kernel in concentrated on the center pixel.
     *  @param intensity The strength of the sharpening kernel. Range [0-100]
     */
    void ocularSharpenExFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float Radius, float sharpness, int intensity);

    /**
     * @brief Applies denoising smoothing filter on detected skin region while retaining other details.
     *  @ingroup group_ip_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     * @param smoothingLevel The amount of skin denoising to apply.
     * @param applySkinFilter Apply filtering to non-skin areas after initial denoising.
     */
    void ocularSkinSmoothingFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int smoothingLevel, bool applySkinFilter);

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
     */
    void ocularConvolution2DFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, float* kernel,
                                   unsigned char filterW, unsigned char cfactor, unsigned char bias);

    /** @brief Resizes an image using Lanczos interpolation. This lets you up or down-sample an image using Lanczos
     *  resampling, which results in noticeably better quality than the standard linear or trilinear interpolation.
     *  @ingroup group_ip_general
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param newWidth The new width of the image.
     *  @param newHeight The new height of the image.
     *  @param dstStride The number of bytes in one row of pixels for output. Used to calculate destination offset.
     */
    void ocularResamplingFilter(unsigned char* Input, unsigned int Width, unsigned int Height, unsigned int Stride, unsigned char* Output,
                                int newWidth, int newHeight, int dstStride);

    /**
     * @brief Rotates an image using bilinear interpolation. Non-image areas are filled with color.
     * @ingroup group_ip_general
     * @param Input The image input data buffer.
     * @param Width The width of the image in pixels.
     * @param Height The height of the image in pixels.
     * @param Stride The number of bytes in one row of pixels.
     * @param Output The image output data buffer.
     * @param outWidth The width of the output image in pixels.
     * @param outHeight The height of the output image in pixels.
     * @param angle The angle to rotate in degrees. Range [0 - 359].
     * @param fillColorR The blue channel value to use for filling non-image area.
     * @param fillColorG The blue channel value to use for filling non-image area.
     * @param fillColorB The blue channel value to use for filling non-image area. This is used if the numbers of channels is 1.
     */
    void ocularRotateBilinear(unsigned char* Input, int Width, int Height, int Stride, unsigned char* Output, int outWidth, int outHeight,
                              float angle, bool keepSize, int fillColorR, int fillColorG, int fillColorB);

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
     */
    void ocularCropImage(const unsigned char* Input, int Width, int Height, int srcStride, unsigned char* Output, int cropX, int cropY,
                         int dstWidth, int dstHeight, int dstStride);

    /**
     * @brief Flip image horizontally or vertically.
     *  @ingroup group_ip_general
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Channels The number of color channels in the image.
     * @param direction The direction of transformation to perform.
     */
    void ocularFlipImage(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, OcDirection direction);

    /**
     * @brief Performs deskewing of an image, useful for scanned documents.
     * @ingroup group_ip_general
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     * @param Stride The number of bytes in one row of pixels.
     * @return True, if valid text image found and deskew performed. Otherwise false.
     */
    bool ocularDocumentDeskew(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

    //------------------------Distort-------------------------

    /**
     * @brief Applies a pixelate effect (sometimes called "mosaic") to an image.
     *  @ingroup group_ip_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     * @param blockSize The width and height of the desired pixelation block (in pixels).
     */
    void ocularPixelateFilter(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int blockSize);

    /**
     * @brief Applies an "oil painting" effect to an image.
     *  @ingroup group_ip_filters
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     * @param radius The radius of the effect. Range >= 1.
     * @param intensity The smoothness of the effect. Smaller values indicate less smoothness (less bins are used to calculate luminance).
     */
    void ocularOilPaintFilter(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int radius, int intensity);

    //------------------------Distort-------------------------

    //--------------------------Misc--------------------------

    /** @brief Performs a Hough transform to detect lines in an image.
     *  @ingroup group_ip_general
     *  @param Input The image input data buffer. Must be single channel, preferably processed through edge detection.
     *  @param[out] LineNumber The location where the number of detected lines are to be stored.
     *  @param[out] DetectedLine A location where parameters of detected lines are to be stored.
     *  @param Height The height of the input image.
     *  @param Width The width of the input image.
     *  @param threshold The maximum number of lines to return.
     */
    void ocularHoughLineDetection(unsigned char* Input, int* LineNumber, struct LineParameter* DetectedLine, int Height, int Width, int threshold);

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
     */
    void ocularDrawLine(unsigned char* canvas, int width, int height, int stride, int x1, int y1, int x2, int y2, unsigned char R,
                        unsigned char G, unsigned char B);

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

#ifdef __cplusplus
}
#endif
#endif /* OCULAR_H */
