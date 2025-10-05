/**
 * @file: pixelate_filters.h
 * @author Warren Galyen
 * Created: 10-3-2025
 * Last Updated: 10-4-2025
 * Last update: added crystallize filter
 *
 * @brief Pixelation filter definitions
 */

#ifndef PIXELATE_FILTERS_H
#define PIXELATE_FILTERS_H

#include <stdint.h>
#include <stdbool.h>
#include "core.h"


/**
 * @brief Applies a mosaic effect to an image.
 * 
 * This filter divides the image into blocks of a given size, and then averages 
 * the color values of all pixels within each block to create a single color value 
 * for that block. The resulting image resembles a grid of uniform blocks with the 
 * average color of each block.
 * 
 * @ingroup group_pixelate_filters
 * @param Input The image input data buffer.
 * @param Output The image output data buffer.
 * @param Width The width of the image in pixels.
 * @param Height The height of the image in pixels.
 * @param Stride The number of bytes in one row of pixels.
 * @param blockSize The width and height of the desired pixelation block (in pixels).
 * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
 */
OC_STATUS ocularMosaicFilter(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int blockSize);

/**
 * @brief Applies a pointillize effect to an image
 * 
 * This filter creates a pointillist painting effect similar to Photoshop's 
 * Pointillize filter. The image is divided into cells, and each cell is 
 * represented by a smooth, anti-aliased colored dot placed at a random position 
 * within that cell. The dot's color is sampled from the exact pixel position 
 * where the dot will be placed, ensuring accurate color representation and 
 * preserving the original image's color variation and detail.
 * 
 * The filter creates an artistic effect reminiscent of pointillism painting 
 * techniques pioneered by Georges Seurat and Paul Signac, where the image is 
 * composed of distinct, randomly placed dots of pure color. Areas not covered 
 * by dots are filled with the specified background color.
 * 
 * @ingroup group_pixelate_filters
 * @param input Input image buffer
 * @param output Output image buffer (must be different from input)
 * @param width Image width in pixels
 * @param height Image height in pixels
 * @param stride Row stride (typically width * channels)
 * @param cellSize Size of each cell in pixels (3-100)
 *                 Smaller values = more detailed, more dots
 *                 Larger values = more abstract, fewer larger dots
 * @param bgR Background color red channel (0-255)
 * @param bgG Background color green channel (0-255)
 * @param bgB Background color blue channel (0-255)
 * @return OC_STATUS_OK on success, error code otherwise
 */
OC_STATUS ocularPointillizeFilter(unsigned char* input, unsigned char* output,
                                  int width, int height, int stride,
                                  int cellSize,
                                  unsigned char bgR, unsigned char bgG, unsigned char bgB);

/**
 * @brief Applies a color halftone effect to an image
 * 
 * This filter simulates the effect of traditional halftone printing. For each
 * color channel, the image is divided into a grid of circles and the size of the
 * circles is proportional to the brightness of the pixels in that cell. Each
 * channel uses a different screen angle to simulate the offset printing process
 * and avoid moiré patterns.
 * 
 * Typical screen angles for realistic halftone:
 *   - Red/Cyan channel: 105° or 15°
 *   - Green/Magenta channel: 75° or 165°
 *   - Blue/Yellow channel: 90° or 0°
 * 
 * @ingroup group_pixelate_filters
 * @param input Input image buffer (RGB or RGBA)
 * @param output Output image buffer (must be different from input)
 * @param width Image width in pixels
 * @param height Image height in pixels
 * @param stride Row stride (typically width * channels)
 * @param radius Maximum dot radius in pixels (4-100)
 *               Controls the size of the halftone grid
 * @param dotDensity Dot density percentage (0-100)
 *                   Controls the maximum size of halftone dots
 *                   100 = full density (dots can extend to grid corners)
 *                   0 = no dots (white output)
 * @param cyanAngle Screen angle for first channel (Red) in degrees (0-360)
 * @param magentaAngle Screen angle for second channel (Green) in degrees (0-360)
 * @param yellowAngle Screen angle for third channel (Blue) in degrees (0-360)
 * @return OC_STATUS_OK on success, error code otherwise
 */
OC_STATUS ocularColorHalftoneFilter(unsigned char* input, unsigned char* output,
                                    int width, int height, int stride,
                                    int radius, float dotDensity,
                                    float cyanAngle, float magentaAngle, float yellowAngle);


/**
 * @brief Applies a fragment effect to the image, similar to Photoshop's Fragment filter.
 *
 * This filter creates four copies of the pixels, offsets them from each other, and averages them
 * to produce a fragmented, multi-exposure appearance. The effect creates a subtle motion blur
 * with diagonal offset copies.
 *
 * @ingroup group_pixelate_filters group_inplace
 * @param Input The image input data buffer.
 * @param Output The image output data buffer (can be same as input for in-place operation).
 * @param Width The width of the image in pixels.
 * @param Height The height of the image in pixels.
 * @param Stride The number of bytes in one row of pixels.
 * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
 */
OC_STATUS ocularFragmentFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

/**
 * @brief Applies a crystallize effect to an image, similar to Photoshop's Crystallize filter.
 *
 * This filter creates a crystal-like or stained glass appearance by dividing the image into
 * irregular polygonal cells using Worley noise. Each cell is filled with the average
 * color of all pixels within that cell, creating a mosaic of colored polygons that resembles
 * crystalline structures.
 *
 * The filter generates random seed points distributed across the image, then assigns each
 * pixel to its nearest seed point. All pixels belonging to the same cell are then colored
 * with their average color value, resulting in smooth, flat-colored polygonal regions.
 *
 * @ingroup group_pixelate_filters
 * @param input Input image buffer
 * @param output Output image buffer (must be different from input)
 * @param width Image width in pixels
 * @param height Image height in pixels
 * @param stride The number of bytes in one row of pixels.
 * @param cellSize Approximate size of each crystal cell in pixels (3-100)
 *                 Smaller values = more detailed, more cells
 *                 Larger values = more abstract, fewer larger cells
 * @return OC_STATUS_OK on success, error code otherwise
 */
OC_STATUS ocularCrystallizeFilter(unsigned char* input, unsigned char* output,
                                  int width, int height, int stride,
                                  int cellSize);


#endif /* PIXELATE_FILTERS_H */

