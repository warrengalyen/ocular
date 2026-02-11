/**
 * @file: boilerplate_filter.h
 * @brief Boilerplate filter definitions – template for new filters (header-only)
 */

#ifndef BOILERPLATE_FILTER_H
#define BOILERPLATE_FILTER_H

#include <stdint.h>
#include <stdbool.h>
#include "lib/core.h"

/**
 * @brief Boilerplate filter
 *
 * Use this as a template for new filters. Replace the implementation
 * with your filter logic while keeping the same parameter checks.
 *
 * @param Input  Input image buffer (read-only).
 * @param Output Output image buffer (must not overlap Input).
 * @param Width  Image width in pixels.
 * @param Height Image height in pixels.
 * @param Stride Row stride in bytes (typically Width * channels).
 * @return OC_STATUS_OK on success, otherwise an error code (see core.h).
 */
static inline OC_STATUS ocularImageFilter(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride) {

    if (Input == NULL || Output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    if (Width <= 0 || Height <= 0 || Stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    int channels = Stride / Width;
    if (channels != 1 && channels != 3) {
        return OC_STATUS_ERR_NOTSUPPORTED;
    }

    // Ensure filter specific parameters are within valid ranges
    // (add filter-specific parameter validation here)

    int pPos;
    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {

            pPos = x * channels + y * Stride;
            for (int c = 0; c < channels; c++) {
                Output[pPos + c] = Input[pPos + c];
            }
        }
    }

    return OC_STATUS_OK;
}

#endif /* BOILERPLATE_FILTER_H */
