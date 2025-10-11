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
 * @ingroup group_distort_filters
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
 * @ingroup group_distort_filters
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
 * @brief Applies a ripple distortion effect to an image
 *
 * This filter creates concentric wave-like distortions radiating from a specified
 * center point. Pixels are displaced radially (toward/away from center) based on a
 * sinusoidal wave pattern, creating a water ripple effect.
 *
 * @ingroup group_distort_filters
 * @param input Input image buffer
 * @param output Output image buffer (can be same as input for in-place operation)
 * @param width Image width in pixels
 * @param height Image height in pixels
 * @param stride Row stride (typically width * channels)
 * @param wavelength Distance between wave peaks in pixels (1-200)
 *                   Smaller values = more ripples, higher frequency
 *                   Larger values = fewer ripples, lower frequency
 * @param amplitude Maximum displacement in pixels (0-100)
 *                  Controls the strength/intensity of the ripple effect
 *                  0 = no effect
 * @param centerX X coordinate of ripple center (0.0-1.0)
 *                0.0 = left edge, 0.5 = center, 1.0 = right edge
 * @param centerY Y coordinate of ripple center (0.0-1.0)
 *                0.0 = top edge, 0.5 = center, 1.0 = bottom edge
 * @param radiusPercentage Effect radius as percentage of max radius (1-100)
 *                         Controls how far the ripple effect extends
 *                         100 = full radius, 50 = half radius
 * @param phase Wave phase offset in degrees (0-360)
 *              Controls the starting position of the wave pattern
 *              0 = wave starts at peak, 90 = wave starts at zero crossing
 * @return OC_STATUS_OK on success, error code otherwise
 */
OC_STATUS ocularRippleDistortionFilter(unsigned char* input, unsigned char* output,
                                       int width, int height, int stride,
                                       float wavelength, float amplitude,
                                       float centerX, float centerY,
                                       float radiusPercentage, float phase);

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
 * @ingroup group_distort_filters
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
 * @ingroup group_distort_filters
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


// TODO: Continue testing and tweaking this filter
/**
 * @enum OcWaveType
 * @brief Wave shape parameter for wave distortion filter
 */
typedef enum {
    OC_WAVE_SINE = 0,      // Smooth sinusoidal waves
    OC_WAVE_TRIANGLE = 1,  // Linear triangle waves
    OC_WAVE_SQUARE = 2     // Sharp square waves
} OcWaveType;

/**
 * @brief Applies a wave distortion effect to an image
 * 
 * This filter creates wave-like distortions similar to Photoshop's Wave filter.
 * Multiple wave generators with random wavelengths and amplitudes are combined
 * to create complex wave patterns. Waves can affect horizontal and/or vertical
 * displacement of pixels.
 * 
 * Each generator creates both horizontal and vertical wave components using the
 * same wavelength and amplitude:
 *   - Horizontal displacement (dx) based on Y coordinate creates horizontal wave lines
 *   - Vertical displacement (dy) based on X coordinate creates vertical wave lines
 * The filter assigns each generator a random wavelength (between minWavelength and 
 * maxWavelength) and random amplitude (between minAmplitude and maxAmplitude).
 * 
 * @ingroup group_ip_filters
 * @param input Input image buffer
 * @param output Output image buffer (can be same as input for in-place operation)
 * @param width Image width in pixels
 * @param height Image height in pixels
 * @param stride Row stride (typically width * channels)
 * @param numGenerators Number of wave generators to combine (1-100)
 *                      More generators create more complex patterns
 * @param minWavelength Minimum wavelength in pixels (1-999)
 *                      Shorter wavelengths create tighter waves (more cycles per image)
 * @param maxWavelength Maximum wavelength in pixels (1-999)
 *                      Must be >= minWavelength
 * @param minAmplitude Minimum wave amplitude in pixels (1-999)
 *                     Controls minimum displacement distance
 * @param maxAmplitude Maximum wave amplitude in pixels (1-999)
 *                     Must be >= minAmplitude
 * @param scaleX Horizontal scale percentage (1-100)
 *               100 = full horizontal displacement
 * @param scaleY Vertical scale percentage (1-100)
 *               100 = full vertical displacement
 * @param waveType Wave shape: OC_WAVE_SINE, OC_WAVE_TRIANGLE, or OC_WAVE_SQUARE
 * @param seed Random seed for reproducible results (0 = use random seed)
 * @return OC_STATUS_OK on success, error code otherwise
 */
OC_STATUS ocularWaveDistortionFilter(unsigned char* input, unsigned char* output,
                                     int width, int height, int stride,
                                     int numGenerators,
                                     int minWavelength, int maxWavelength,
                                     int minAmplitude, int maxAmplitude,
                                     int scaleX, int scaleY,
                                     OcWaveType waveType,
                                     unsigned int seed);

/**
 * @brief Applies a kaleidoscope effect to an image
 *
 * This filter creates a kaleidoscope-like mirror effect by dividing the image
 * into wedge-shaped sections and mirroring them around the center point. Based 
 * on Jerry Huxtable's (JH Labs) transformation with some modifications.
 * 
 * @ingroup group_distort_filters
 * @param input Input image buffer
 * @param output Output image buffer (can be same as input for in-place operation)
 * @param width Image width in pixels
 * @param height Image height in pixels
 * @param stride The
 * @param mirrors Number of mirror segments (2-20)
 *                Creates radial symmetry with the specified number of segments
 *                2 mirrors = 2 segments (bilateral symmetry)
 *                3 mirrors = 3 segments (triangular pattern with empty areas)
 *                6 mirrors = 6 segments (hexagonal pattern)
 *                8 mirrors = 8 segments (octagonal pattern)
 *                12 mirrors = 12 segments (dodecagonal pattern)
 * @param angle Primary rotation angle in degrees (0-360)
 *              Controls the orientation of the mirror segments
 * @param angle2 Secondary rotation angle in degrees (0-360)
 *               Additional rotation applied after mirroring
 * @param centerX Center X position as proportion of image width (0.0-1.0)
 *                0.0 = left edge, 0.5 = center, 1.0 = right edge
 * @param centerY Center Y position as proportion of image height (0.0-1.0)
 *                0.0 = top edge, 0.5 = center, 1.0 = bottom edge
 * @param radius Effect radius as percentage (0-100)
 *               0 = no effect, 100 = full image
 *               Pixels outside radius show original image
 * @return OC_STATUS_OK on success, error code otherwise
 */
OC_STATUS ocularKaleidoscopeFilter(unsigned char* input, unsigned char* output,
                                   int width, int height, int stride,
                                   int mirrors, float angle, float angle2,
                                   float centerX, float centerY, float radius);


#endif /* DISTORT_FILTERS_H */

