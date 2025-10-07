/**
 * @file: denoise_filters.h
 * @author Warren Galyen
 * Created: 10-6-2025
 * Last Updated: 10-6-2025
 * Last update: migrated guided filter from ocular.h
 *
 * @brief Denoise filter definitions
 */

 #ifndef DENOISE_FILTERS_H
 #define DENOISE_FILTERS_H

 #include <stdint.h>
 #include <stdbool.h>
 #include "core.h"

/**
 * @brief Applies a guided filter to the image. This is a edge-preserving smoothing filter that is useful for various
 *        applications like de-noising, detail enhancement and HDR compression.
 * @ingroup group_ip_filters
 * @param Input Input image buffer (can be grayscale or color)
 * @param Guide Guide image buffer (can be NULL to use Input as guide)
 * @param Output Output image buffer
 * @param Width The width of the image in pixels.
 * @param Height The height of the image in pixels.
 * @param Stride The number of bytes in one row of pixels.
 * @param Radius Filter radius. Range [1 - 64]
 * @param Epsilon Prevents division by zero and controls the degree of smoothing near edges. Range [0.001 - 0.4]
 *                Smaller values preserve edges better, larger values increase smoothing.
 * @return OC_STATUS_OK if successful, otherwise an error code
 */
OC_STATUS ocularGuidedFilter(unsigned char* Input, unsigned char* Guide, unsigned char* Output, int Width, int Height, 
                            int Stride, int Radius, float Epsilon);

 #endif /* DENOISE_FILTERS_H */



