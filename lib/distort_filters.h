/**
 * @file: distort_filters.h
 * @author Warren Galyen
 * Created: 10-2-2025
 * Last Updated: 10-3-2025
 * Last update: added polar coordinates filter
 *
 * @brief Distortion filter definitions

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

/**
 * @brief Applies a twirl distortion effect to an image
 * 
 * This filter rotates pixels around the center point within a circular region.
 * The rotation strength decreases smoothly from the center to the edge.
 * Positive amounts create clockwise rotation, negative amounts create
 * counter-clockwise rotation.
 * 
 * @param input Input image buffer
 * @param output Output image buffer (can be same as input for in-place operation)
 * @param width Image width in pixels
 * @param height Image height in pixels
 * @param stride Row stride (typically width * channels)
 * @param angle Rotation angle in degrees, range typically [-360, 360]
 *              Negative values = counter-clockwise rotation
 *              Positive values = clockwise rotation
 *              0 = no effect
 * @return OC_STATUS_OK on success, error code otherwise
 */
OC_STATUS ocularTwirlDistortionFilter(unsigned char* input, unsigned char* output,
                                      int width, int height, int stride,
                                      float angle);

/**
 * @enum OcRippleSize
 * @brief Size parameter for ripple distortion filter
 */
typedef enum {
    OC_RIPPLE_SMALL = 0,   // Small ripples (high frequency)
    OC_RIPPLE_MEDIUM = 1,  // Medium ripples (default)
    OC_RIPPLE_LARGE = 2    // Large ripples (low frequency)
} OcRippleSize;

/**
 * @brief Applies a ripple distortion effect to an image
 * 
 * This filter creates concentric wave-like distortions radiating from the center
 * point. Pixels are displaced radially (toward/away from center) based on a
 * sinusoidal wave pattern, creating a water ripple effect.
 * 
 * @param input Input image buffer
 * @param output Output image buffer (can be same as input for in-place operation)
 * @param width Image width in pixels
 * @param height Image height in pixels
 * @param stride Row stride (typically width * channels)
 * @param amount Ripple amount in range [-999, 999] (percent)
 *               Negative values = ripples compress inward
 *               Positive values = ripples expand outward
 *               0 = no effect
 * @param size Ripple size: OC_RIPPLE_SMALL, OC_RIPPLE_MEDIUM, or OC_RIPPLE_LARGE
 *             Controls the wavelength/frequency of ripples
 * @return OC_STATUS_OK on success, error code otherwise
 */
OC_STATUS ocularRippleDistortionFilter(unsigned char* input, unsigned char* output,
                                       int width, int height, int stride,
                                       int amount, OcRippleSize size);

/**
 * @enum OcSpherizeMode
 * @brief Mode parameter for spherize distortion filter
 */
typedef enum {
    OC_SPHERIZE_NORMAL = 0,      // Full spherical distortion (both axes)
    OC_SPHERIZE_HORIZONTAL = 1,  // Horizontal only (cylindrical)
    OC_SPHERIZE_VERTICAL = 2     // Vertical only (cylindrical)
} OcSpherizeMode;

/**
 * @brief Applies a spherize distortion effect to an image
 * 
 * This filter maps the image onto a spherical or cylindrical surface, creating
 * a 3D bulge or pinch effect. Simulates viewing the image wrapped around a
 * sphere or cylinder.
 * 
 * @param input Input image buffer
 * @param output Output image buffer (can be same as input for in-place operation)
 * @param width Image width in pixels
 * @param height Image height in pixels
 * @param stride Row stride (typically width * channels)
 * @param amount Spherize amount in range [-100, 100] (percent)
 *               Negative values = concave (pinch inward)
 *               Positive values = convex (bulge outward)
 *               0 = no effect
 * @param mode Spherize mode: OC_SPHERIZE_NORMAL, OC_SPHERIZE_HORIZONTAL, or OC_SPHERIZE_VERTICAL
 *             Normal = full sphere, Horizontal/Vertical = cylinder along that axis
 * @return OC_STATUS_OK on success, error code otherwise
 */
OC_STATUS ocularSpherizeDistortionFilter(unsigned char* input, unsigned char* output,
                                         int width, int height, int stride,
                                         int amount, OcSpherizeMode mode);

/**
 * @enum OcPolarMode
 * @brief Mode parameter for polar coordinates filter
 */
typedef enum {
    OC_POLAR_TO_RECT = 0,     // Convert from polar to rectangular coordinates
    OC_RECT_TO_POLAR = 1     // Convert from rectangular to polar coordinates
} OcPolarMode;

/**
 * @brief Converts an image between rectangular and polar coordinate systems
 * 
 * This filter transforms images between Cartesian (rectangular) and polar
 * coordinate systems, similar to Photoshop's Polar Coordinates filter.
 * 
 * @param input Input image buffer
 * @param output Output image buffer (can be same as input for in-place operation)
 * @param width Image width in pixels
 * @param height Image height in pixels
 * @param stride Row stride (typically width * channels)
 * @param mode Conversion mode: OC_POLAR_TO_RECT or OC_RECT_TO_POLAR
 * @return OC_STATUS_OK on success, error code otherwise
 */
OC_STATUS ocularPolarCoordinatesFilter(unsigned char* input, unsigned char* output,
                                       int width, int height, int stride,
                                       OcPolarMode mode);


#endif /* DISTORT_FILTERS_H */

