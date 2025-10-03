/**
 * @file: distort_filters.h
 * @brief Distortion filter implementations for image manipulation
 */

#ifndef DISTORT_FILTERS_H
#define DISTORT_FILTERS_H

#include <stdint.h>
#include <stdbool.h>
#include "core.h"


/**
 * @brief Applies a pinch/bulge distortion effect to an image
 * 
 * This filter warps the image around its center point within a circular region.
 * Positive amounts create a pinch effect (pulling pixels toward the center),
 * while negative amounts create a bulge effect (pushing pixels outward).
 * 
 * The distortion is applied only within a radius equal to half the smaller
 * dimension (width or height). Pixels outside this radius remain untouched.
 * 
 * @param input Input image buffer
 * @param output Output image buffer (can be same as input for in-place operation)
 * @param width Image width in pixels
 * @param height Image height in pixels
 * @param stride Row stride (typically width * channels)
 * @param amount Distortion amount in range [-100, 100]
 *               Negative values create bulge effect
 *               Positive values create pinch effect
 *               0 = no effect
 * @return OC_STATUS_OK on success, error code otherwise
 */
OC_STATUS ocularPinchDistortionFilter(unsigned char* input, unsigned char* output, 
                                      int width, int height, int stride, 
                                      float amount);


#endif /* DISTORT_FILTERS_H */

