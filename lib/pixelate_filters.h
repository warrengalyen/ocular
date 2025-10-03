/**
 * @file: pixelate_filters.h
 * @author Warren Galyen
 * Created: 10-3-2025
 * Last Updated: 10-3-2025
 * Last update: added pointillize filter
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


#endif /* PIXELATE_FILTERS_H */

