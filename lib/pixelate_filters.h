/**
 * @file: pixelate_filters.h
 * @author Warren Galyen
 * Created: 10-3-2025
 * Last Updated: 10-3-2025
 * Last update: added color halftone filter
 *
 * @brief Pixelation and artistic filter definitions
 */

#ifndef PIXELATE_FILTERS_H
#define PIXELATE_FILTERS_H

#include <stdint.h>
#include <stdbool.h>
#include "core.h"


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


#endif /* PIXELATE_FILTERS_H */

