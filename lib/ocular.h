#ifndef OCULAR_H
#define OCULAR_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1310)
    #pragma warning(disable : 4996) // VS doesn't like fopen, but fopen_s is not
// standard C so unusable here
#endif /*_MSC_VER */

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
#include "util.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

    // TODO: Implement these within the filters
    // Return status codes for filter exported functions
    typedef enum {
        OC_STATUS_OK = 0,                   //	Normal, no errors
        OC_STATUS_ERR_OUTOFMEMORY = 1,      //	Out of memory
        OC_STATUS_ERR_STACKOVERFLOW = 2,    //	Stack overflow
        OC_STATUS_ERR_NULLREFERENCE = 3,    //	Empty reference
        OC_STATUS_ERR_INVALIDPARAMETER = 4, //	The parameters are not within the normal range
        OC_STATUS_ERR_PARAMISMATCH = 5,     //	Parameter mismatch
        OC_STATUS_ERR_INDEXOUTOFRANGE = 6,
        OC_STATUS_ERR_NOTSUPPORTED = 7,
        OC_STATUS_ERR_OVERFLOW = 8,
        OC_STATUS_ERR_FILENOTFOUND = 9,
        OC_STATUS_ERR_UNKNOWN
    } OC_STATUS;

    typedef enum {
        OC_DEPTH_8U = 0,  //	unsigned char
        OC_DEPTH_8S = 1,  //	char
        OC_DEPTH_16S = 2, //	short
        OC_DEPTH_32S = 3, //    int
        OC_DEPTH_32F = 4, //	float
        OC_DEPTH_64F = 5, //	double
    } OC_BITDEPTH;

    typedef struct {
        int Width;           // The width of the image
        int Height;          // The height of the image
        int Stride;          // The number of bytes occupied by a line element
        int Channels;        // Number of color channels
        int Depth;           // The data type used
        unsigned char* Data; // Data of the image
        int Reserved;        // Reserved use (not yet used)
    } OcImage;

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

    // RGB to YIQ color space conversion.
    void rgb2yiq(unsigned char* R, unsigned char* G, unsigned char* B, short* Y, short* I, short* Q);

    // YIQ to RGB color space conversion.
    void yiq2rgb(short* Y, short* I, short* Q, unsigned char* R, unsigned char* G, unsigned char* B);

    // RGB to HSV color space conversion.
    void rgb2hsv(const unsigned char* R, const unsigned char* G, const unsigned char* B, unsigned char* H, unsigned char* S, unsigned char* V);

    // HSV to RGB color space conversion.
    void hsv2rgb(const unsigned char* H, const unsigned char* S, const unsigned char* V, unsigned char* R, unsigned char* G, unsigned char* B);

    // RGB to YCbCr color space conversion.
    void rgb2ycbcr(unsigned char R, unsigned char G, unsigned char B, unsigned char* y, unsigned char* cb, unsigned char* cr);

    // YCbCr to RGB color space conversion.
    void ycbcr2rgb(unsigned char y, unsigned char Cb, unsigned char Cr, unsigned char* R, unsigned char* G, unsigned char* B);

    //--------------------------Color adjustments--------------------------

    /** @brief Converts an RGB image to single channel grayscale (a slightly faster implementation of the saturation filter,
     *  without the ability to vary the color contribution)
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     */
    void ocularGrayscaleFilter(OcImage* Input, OcImage* Output);

    /** @brief Adjusts the individual RGB channels of an image
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

    /** @brief Applies a thresholding operation where the threshold is continually adjusted based on the average luminance of the image.
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

    /** @brief Reduces an image to its average luminosity.
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
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param intensity The degree to which the sepia tone replaces the normal image color. Range [0 - 100].
     */
    void ocularSepiaFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int intensity);

    /** @brief For a given color in the image, sets the alpha channel to 0.
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
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param gamma The gamma adjustment to apply. Range [0.0-3.0]. Default 1.0.
     */
    void ocularGammaFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float gamma);

    /** @brief Adjusts the contrast of the image
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param contrast The adjusted contrast. Range [0.0 - 4.0]. Default 1.0.
     */
    void ocularContrastFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float contrast);

    /** @brief Adjusts the exposure of the image
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param exposure The adjusted exposure. Range [-10.0 - 10.0]. Default 0.0.
     */
    void ocularExposureFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float exposure);

    /** @brief Adjusts the brightness of the image
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param brightness The adjusted brightness. Range [-1.0 - 1.0]. Default 0.0.
     */
    void ocularBrightnessFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int brightness);

    /** @brief Uses the luminance of the image to mix between two specified colors
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
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param hueAdjust The hue angle, in degrees. 90 degrees by default.
     */
    void ocularHueFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float hueAdjust);

    /** @brief Allows you to tint the shadows and highlights of an image independently using a color and intensity.
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
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     */
    void ocularColorInvertFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

    /** @brief Outputs a generated image with a solid color.
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
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param threshold The luminance threshold. Range [0.0 - 1.0]. Default. 0.5.
     */
    void ocularLuminanceThresholdFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, unsigned char threshold);

    /** @brief Automatically applies a neutral white balance to an image.
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
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param fraction Clipping threshold in percentage of pixels allowed to clip to clip to the white and black points
     *  of the histogram. Range [0.001 - 0.1].
     */
    void ocularAutoLevel(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float fraction);

    //--------------------------Color adjustments--------------------------

    //--------------------------Blur filters--------------------------

    /** @brief A hardware-optimized, variable radius box blur
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
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Stride The number of bytes in one row of pixels.
     *  @param GaussianSigma A radius in pixels to use for the blur, >= 0.0
     */
    void ocularGaussianBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float GaussianSigma);

    /** @brief Performs a convolution blurring that simulates the effect of shooting a moving object on film.
     *  @param Input The image input data buffer.
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     *  @param Channels The numbers of color channels in the image.
     *  @param radius A radius in pixels to use for the blur, >= 0.0
     *  @param angle The angle of the blur. Range [0 - 360]
     */
    void ocularMotionBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, int radius, int angle);

    //--------------------------Blur filters--------------------------


    //-------------------------- Edge detection --------------------------

    /** @brief Applies a sobel edge detection filter
     *  @param Input The image input data buffer (assumes grayscale image).
     *  @param Output The image output data buffer.
     *  @param Width The width of the image in pixels.
     *  @param Height The height of the image in pixels.
     */
    void ocularSobelEdgeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height);

    //-------------------------- Edge detection --------------------------

    //--------------------------Enhancement filters--------------------------

    /** @brief Performs a non-linear, edge-preserving and noise-reducing smoothing of an image.
     *  This is a fast implementation, like gaussian blur, that performs vertical/horizontal passes independently.
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

    //--------------------------Enhancement filters--------------------------

    //--------------------------Misc--------------------------

    /** @brief Applies a 2D convolution to an image using a kernel.
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

    /** @brief Outputs only a selected portion of an image.
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
    void ocularCropFilter(const unsigned char* Input, int Width, int Height, int srcStride, unsigned char* Output, int cropX, int cropY,
                          int dstWidth, int dstHeight, int dstStride);

    //--------------------------Misc--------------------------

    /** @brief Performs a Hough transform to detect lines in an image.
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

    /**
     * @brief Allocates a new image data structure in memory.
     * @param Width The width of the image.
     * @param Height The height of the image.
     * @param Depth The color depth of the image.
     * @param Channel The number of color channels of the image.
     * @param[out] image The returned image data structure.
     * @return 0 if success, otherwise fail.
     */
    OC_STATUS ocularCreateImage(int Width, int Height, int Depth, int Channel, OcImage** image);

    /**
     * @brief Releases a created image data structure from memory.
     * @param image The image data structure that needs to be released.
     * @return 0 if success, otherwise fail.
     */
    OC_STATUS ocularFreeImage(OcImage** image);

    /**
     * @brief Clone an existing image.
     * @param Input The image data structure to copy.
     * @param[out] Output The returned image data structure. This should be empty and not previously allocated.
     * @return 0 if success, otherwise fail.
     */
    OC_STATUS ocularCloneImage(OcImage* Input, OcImage** Output);

    //--------------------------Misc--------------------------

    //--------------------------preImage processing--------------------------

    /**
     * @brief Quickly retrieve width and height from an image file without processing image. Supports PNG/GIF/JPEG/BMP/ICO files.
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
