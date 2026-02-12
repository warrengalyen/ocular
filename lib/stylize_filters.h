/**
 * @file: stylize_filters.h
 * @author Warren Galyen
 * Created: 10-4-2025
 * Last Updated: 2-12-2026
 * Last update: added marble filter
 *
 * @brief Stylize filter definitions

 * Marble filter is based on JH-Labs MarbleFilter by Jerry Huxtable.
 * Copyright (c) 2006 Jerry Huxtable
 * and is licensed under the Apache License, Version 2.0.
 * http://www.apache.org/licenses/LICENSE-2.0

 * Glass Tiles filter is based on Paint.NET's Glass Blocks effect by Ed Harvey.
 * Copyright (c) 2009 Ed Harvey and Paint.NET Team
 * and is licensed under modified MIT License.
 */

#ifndef STYLIZE_FILTERS_H
#define STYLIZE_FILTERS_H

#include <stdint.h>
#include <stdbool.h>
#include "core.h"


/**
 * @brief Applies an "oil painting" effect to an image.
 * @ingroup group_stylize_filters
 * @param Input The image input data buffer.
 * @param Output The image output data buffer.
 * @param Width The width of the image in pixels.
 * @param Height The height of the image in pixels.
 * @param Stride The number of bytes in one row of pixels.
 * @param radius The radius of the effect. Range >= 1.
 * @param intensity The smoothness of the effect. Smaller values indicate less smoothness (less bins are used to calculate luminance).
 * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
 */
OC_STATUS ocularOilPaintFilter(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int radius, int intensity);

/**
 * @brief Simulates the image being observed through a layer of frosted glass by applying random pixel disturbance.
 * @ingroup group_stylize_filters
 * @param Input The input image data.
 * @param Output The output image data.
 * @param Height The height of the image.
 * @param Width The width of the image.
 * @param Channels The number of channels in the image.
 * @param Radius The radius of the blur. Recommend keeping this value around 2.
 * @param Range Controls the range of random pixel displacement. Greater values will produce a more pronounced effect,
 * making the image more distorted. Range [1 - 20]
 * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
 * */
OC_STATUS ocularFrostedGlassEffect(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius, int Range);

/**
 * @brief Applies a film grain effect to an image.
 * @ingroup group_stylize_filters
 * @param Input The image input data buffer.
 * @param Output The image output data buffer.
 * @param Width The width of the image in pixels.
 * @param Height The height of the image in pixels.
 * @param Channels The number of color channels in the image.
 * @param Strength The intensity of the grain. Range [0.0 - 100.0].
 * @param Softness The softness of the grain. Range [0.0 - 25.0].
 * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
 */
OC_STATUS ocularFilmGrainEffect(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, float Strength, float Softness);

/**
 * @brief Applies a relief (emboss) effect to the image.
 * @ingroup group_stylize_filters
 * @param Input The image input data buffer.
 * @param Output The image output data buffer.
 * @param Width The width of the image in pixels.
 * @param Height The height of the image in pixels.
 * @param Stride The number of bytes in one row of pixels.
 * @param Angle Controls the direction of the relief effect.
 * @param Offset Controls the base brightness of the relief effect. Range [0 - 255]. ~127 will produce a balanced effect.
 * Lower values will darken the image (emphasizing raised edges), higher values will lighten the image
 * (emphasizing sunken edges).
 * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
 */
OC_STATUS ocularReliefFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float Angle, int Offset);

/**
 * @brief Reduces image noise and emphasizes the details and textures within an image.
 * It achieves this by dividing the image into small, overlapping square regions and calculating the average color and
 * standard deviation for each region. The final result is a stylized image with a distinctive patchwork-like
 * appearance, reminiscent of brush strokes.
 * @ingroup group_stylize_filters
 * @param Input Input image buffer
 * @param Output Output image buffer
 * @param Width Image width
 * @param Height Image height
 * @param Stride Image stride (Width * Channels)
 * @param Radius Filter radius. Range [1 - 10].
 * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
 */
OC_STATUS ocularKuwaharaFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

/**
 * @brief Glow style options for portrait glow filter
 */
typedef enum
{
    OC_GLOW_STYLE_CLASSIC = 0, // Screen blend mode
    OC_GLOW_STYLE_MODERN = 1,  // Overlay blend mode
    OC_GLOW_STYLE_SUBTLE = 2   // Soft Light blend mode
} OcGlowStyle;

/**
 * @brief Applies a portrait glow effect similar to Photoshop, creating a soft, luminous glow around the subject.
 * The effect is achieved by blurring the image, applying exposure boost, and compositing it with the original
 * using various blend modes.
 * @ingroup group_stylize_filters
 * @param Input The image input data buffer.
 * @param Output The image output data buffer.
 * @param Width The width of the image in pixels.
 * @param Height The height of the image in pixels.
 * @param Stride The number of bytes in one row of pixels.
 * @param Style The glow style to use. Classic = Screen, Modern = Overlay, Subtle = Soft Light.
 * @param GlowRadius The radius of the Gaussian blur. Range [1 - 100].
 * @param ExposureBoost The exposure boost to apply to the blurred image. Range [0 - 200]. 100 = neutral, >100 = brighter.
 * @param Strength Controls the blend opacity. Range [0 - 100]. Higher values produce a more pronounced effect.
 * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
 */
OC_STATUS ocularPortraitGlowFilter(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, OcGlowStyle Style,
                                   int GlowRadius, int ExposureBoost, int Strength);

/**
 * @brief Creates a glass tile effect by dividing the image into tiles and applying
 * curved distortion within each tile using supersampling.
 * Based on Paint.NET's Glass Blocks effect by Ed Harvey. Mostly written from scratch
 * with multiple performance optimizations.
 * @ingroup group_ip_filters
 * @param Input - Input image buffer
 * @param Output - Output image buffer
 * @param Width - Image width in pixels
 * @param Height - Image height in pixels
 * @param Stride - Row stride in bytes
 * @param rotation - Rotation angle in degrees (-45 to 45)
 * @param tileSize - Size of each tile in pixels (2 to 200)
 * @param curvature - Curvature of the tiles (-20 to 20). Positive values create convex tiles, negative values create
 * concave tiles. Zero value creates flat tiles.
 * @param quality - Interpolation quality 1–5: 1 = no antialiasing, 2–5 = supersampling (4/7/10/13 samples)
 * @param edgeMode - How to handle pixels outside image bounds
 * @return OC_STATUS_OK on success, error code otherwise
 */
OC_STATUS ocularGlassTilesFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float rotation,
                                 int tileSize, float curvature, int quality, OcEdgeMode edgeMode);

/**
 * @brief Applies a marble effect to an image by displacing pixels using Perlin noise,
 * based on JH-Labs MarbleFilter. Uses supersampling for antialiasing.
 * @ingroup group_ip_filters
 * @param Input - Input image buffer
 * @param Output - Output image buffer
 * @param Width - Image width in pixels
 * @param Height - Image height in pixels
 * @param Stride - Row stride in bytes
 * @param scale - Scale of the marble veins (0.0–100.0). Higher values produce larger, smoother patterns.
 * @param turbulence - Turbulence/amount of displacement (0.00–1.00)
 * @param quality - Interpolation quality 1–5: 1 = no antialiasing, 2–5 = supersampling (4/7/10/13 samples)
 * @param edgeMode - How to handle pixels outside image bounds
 * @param seed - Random seed for reproducible marble patterns (0 = no offset)
 * @return OC_STATUS_OK on success, error code otherwise
 */
OC_STATUS ocularMarbleFilter(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                             float scale, float turbulence, int quality, OcEdgeMode edgeMode, unsigned int seed);

#endif /* STYLIZE_FILTERS_H */
