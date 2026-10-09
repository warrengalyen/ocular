/**
 * @file: artistic.h
 * @author Warren Galyen
 * Created: 10-9-2026
 * Last Updated: 10-9-2026
 * Last update: added cutout filter
 *
 * @brief Artistic filter definitions
 */

#ifndef ARTISTIC_H
#define ARTISTIC_H

#include <stdint.h>
#include <stdbool.h>
#include "core.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Colored pencil filter
 *
 * Renders the image as colored pencil strokes on paper: dark areas get dense
 * cross-hatching, light areas get sparse single-direction strokes, and edges
 * are reinforced.
 *
 * @ingroup group_stylize_filters
 * @param Input           Input image buffer (read-only). Supports 1 or 3 channels (R,G,B order).
 * @param Output          Output image buffer (must not overlap Input).
 * @param Width           Image width in pixels.
 * @param Height          Image height in pixels.
 * @param Stride          Row stride in bytes (typically Width * channels).
 * @param PencilWidth     Stroke width. Range [1 - 24]. Suggested default: 4.
 * @param StrokePressure  Stroke pressure. Range [0 - 15]. Suggested default: 8.
 * @param PaperBrightness Paper brightness. Range [0 - 50]. Suggested default: 24.
 * @return OC_STATUS_OK on success, otherwise an error code (see core.h).
 */
OC_STATUS ocularColoredPencilFilter(const unsigned char* Input, unsigned char* Output,
                                    int Width, int Height, int Stride,
                                    int PencilWidth, int StrokePressure, int PaperBrightness);

/**
 * @brief Cutout filter
 *
 * Simplifies the image into a few flat tone levels, like layered cut paper.
 *
 * @ingroup group_stylize_filters
 * @param Input          Input image buffer (read-only). Supports 1 or 3 channels (R,G,B order).
 * @param Output         Output image buffer (must not overlap Input).
 * @param Width          Image width in pixels.
 * @param Height         Image height in pixels.
 * @param Stride         Row stride in bytes (typically Width * channels).
 * @param NumberOfLevels Tone levels. Range [2 - 8]. Suggested default: 6.
 * @param EdgeSimplicity Edge simplification. Range [0 - 10]. Suggested default: 4.
 * @param EdgeFidelity   Edge fidelity. Range [1 - 3]. Suggested default: 2.
 * @return OC_STATUS_OK on success, otherwise an error code (see core.h).
 */
OC_STATUS ocularCutoutFilter(const unsigned char* Input, unsigned char* Output,
                             int Width, int Height, int Stride,
                             int NumberOfLevels, int EdgeSimplicity, int EdgeFidelity);

#ifdef __cplusplus
}
#endif

#endif  /* ARTISTIC_H */
