#ifndef OCULAR_BLUR_FILTERS_H
#define OCULAR_BLUR_FILTERS_H

#include "core.h"
#include "util.h"


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
 * @brief Performs an expontential blur where the intensity of the blur gradually decreases
 * as the distance from the center pixel increases, following an exponential decay pattern.
 * Essentially creating a more pronounced blur near the center and a softer fade towards the edges, often
 * used to simulate realistic light falloff or glow effects.
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

#endif // OCULAR_BLUR_FILTERS_H
