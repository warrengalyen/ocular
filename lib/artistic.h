/**
 * @file: artistic.h
 * @author Warren Galyen
 * Created: 10-9-2026
 * Last Updated: 10-9-2026
 * Last update: added paint daubs filter
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

/** @brief Brush types for the paint daubs filter */
typedef enum {
    OC_PAINTDAUBS_SIMPLE = 0,   /**< Plain daubs */
    OC_PAINTDAUBS_LIGHT_ROUGH,  /**< Rough, lightened texture */
    OC_PAINTDAUBS_DARK_ROUGH,   /**< Rough, darkened texture */
    OC_PAINTDAUBS_WIDE_SHARP,   /**< Larger daubs, sharp */
    OC_PAINTDAUBS_WIDE_BLURRY,  /**< Larger daubs, blurred */
    OC_PAINTDAUBS_SPARKLE       /**< Plain daubs with bright speckles along edges */
} OcPaintDaubsBrush;

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
 * @param ForegroundR/G/B Foreground (pencil) color, 0-255 each. The pencil strokes are the image
 *                        colors screened with this color, so black (default 0,0,0) leaves them
 *                        unchanged and lighter colors tint them lighter.
 * @param BackgroundR/G/B Background (paper) color, 0-255 each. Scaled by PaperBrightness.
 *                        Suggested default: white (255,255,255).
 * @return OC_STATUS_OK on success, otherwise an error code (see core.h).
 */
OC_STATUS ocularColoredPencilFilter(const unsigned char* Input, unsigned char* Output,
                                    int Width, int Height, int Stride,
                                    int PencilWidth, int StrokePressure, int PaperBrightness,
                                    unsigned char ForegroundR, unsigned char ForegroundG, unsigned char ForegroundB,
                                    unsigned char BackgroundR, unsigned char BackgroundG, unsigned char BackgroundB);

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

/**
 * @brief Dry brush filter
 *
 * Paints the image with a dry brush: regions are flattened into brush-like
 * patches, edges are re-sharpened, tones are partly posterized and a light
 * grain is added.
 *
 * @ingroup group_stylize_filters
 * @param Input       Input image buffer (read-only). Supports 1 or 3 channels (R,G,B order).
 * @param Output      Output image buffer (must not overlap Input).
 * @param Width       Image width in pixels.
 * @param Height      Image height in pixels.
 * @param Stride      Row stride in bytes (typically Width * channels).
 * @param BrushSize   Brush size. Range [0 - 10]. Suggested default: 2.
 * @param BrushDetail Brush detail. Range [0 - 10]. Suggested default: 8.
 * @param Texture     Texture amount. Range [1 - 3]. Suggested default: 1.
 * @return OC_STATUS_OK on success, otherwise an error code (see core.h).
 */
OC_STATUS ocularDryBrushFilter(const unsigned char* Input, unsigned char* Output,
                               int Width, int Height, int Stride,
                               int BrushSize, int BrushDetail, int Texture);

/**
 * @brief Fresco filter
 *
 * Paints the image in rough, high-contrast patches with dark edges, like a
 * fresco: Kuwahara smoothing, strong sharpening, partial posterization with
 * grain, then a contrast curve and edge darkening.
 *
 * @ingroup group_stylize_filters
 * @param Input       Input image buffer (read-only). Supports 1 or 3 channels (R,G,B order).
 * @param Output      Output image buffer (must not overlap Input).
 * @param Width       Image width in pixels.
 * @param Height      Image height in pixels.
 * @param Stride      Row stride in bytes (typically Width * channels).
 * @param BrushSize   Brush size. Range [0 - 10]. Suggested default: 2.
 * @param BrushDetail Brush detail. Range [0 - 10]. Suggested default: 8.
 * @param Texture     Texture amount. Range [1 - 3]. Suggested default: 1.
 * @return OC_STATUS_OK on success, otherwise an error code (see core.h).
 */
OC_STATUS ocularFrescoFilter(const unsigned char* Input, unsigned char* Output,
                             int Width, int Height, int Stride,
                             int BrushSize, int BrushDetail, int Texture);

/**
 * @brief Paint daubs filter
 *
 * Repaints the image with short, blobby brush daubs. The brush type selects
 * between simple, rough (light/dark), wide (sharp/blurry) and sparkle looks.
 *
 * @ingroup group_stylize_filters
 * @param Input     Input image buffer (read-only). Supports 1 or 3 channels (R,G,B order).
 * @param Output    Output image buffer (must not overlap Input).
 * @param Width     Image width in pixels.
 * @param Height    Image height in pixels.
 * @param Stride    Row stride in bytes (typically Width * channels).
 * @param BrushSize Brush size. Range [1 - 50]. Suggested default: 7.
 * @param Sharpness Sharpness. Range [0 - 40]. Suggested default: 5.
 * @param BrushType One of OcPaintDaubsBrush. Suggested default: OC_PAINTDAUBS_SIMPLE.
 * @return OC_STATUS_OK on success, otherwise an error code (see core.h).
 */
OC_STATUS ocularPaintDaubsFilter(const unsigned char* Input, unsigned char* Output,
                                 int Width, int Height, int Stride,
                                 int BrushSize, int Sharpness, int BrushType);

#ifdef __cplusplus
}
#endif

#endif  /* ARTISTIC_H */
