/**
 * @file: stylize_filters.h
 * @author Warren Galyen
 * Created: 10-4-2025
 * Last Updated: 10-4-2025
 * Last update: migrated oil paint, frosted glass, film grain, 
 *              relief filters from ocular.h
 *
 * @brief Stylize filter definitions
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
OC_STATUS ocularFrostedGlassEffect(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                                    int Radius, int Range);

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
OC_STATUS ocularFilmGrainEffect(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, 
                                float Strength, float Softness);

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
 * @return OC_STATUS
 */
OC_STATUS ocularReliefFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float Angle, int Offset);

/**
 * @brief Wind technique types for the Wind effect filter.
 * @ingroup group_stylize_filters
 */
typedef enum {
    OC_WIND_TECHNIQUE_WIND = 0,     /* Standard wind effect with gradual streaking */
    OC_WIND_TECHNIQUE_BLAST = 1,    /* Intense wind effect with stronger streaks */
    OC_WIND_TECHNIQUE_STAGGER = 2   /* Alternating staggered wind effect */
} OcWindTechnique;

// Wind direction types for the Wind effect filter.
typedef enum {
    OC_WIND_FROM_LEFT = 0,   /* Wind blowing from left to right */
    OC_WIND_FROM_RIGHT = 1   /* Wind blowing from right to left */
} OcWindDirection;

/**
 * @brief Applies a wind effect to the image, creating horizontal motion blur streaks.
 * @ingroup group_stylize_filters
 * 
 * This filter simulates wind blowing across the image, creating horizontal streaks
 * similar to the Wind filter in Photoshop. The effect works by detecting bright
 * edges and extending them horizontally in the direction of the wind.
 * 
 * @param Input The image input data buffer.
 * @param Output The image output data buffer.
 * @param Width The width of the image in pixels.
 * @param Height The height of the image in pixels.
 * @param Stride The number of bytes in one row of pixels.
 * @param technique The wind technique to apply (Wind, Blast, or Stagger).
 * @param direction The direction of the wind (from left or from right).
 * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
 */
OC_STATUS ocularWindFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, 
                           OcWindTechnique technique, OcWindDirection direction);


#endif /* STYLIZE_FILTERS_H */
 