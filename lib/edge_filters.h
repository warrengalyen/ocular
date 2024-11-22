#ifndef OCULAR_EDGE_FILTERS_H
#define OCULAR_EDGE_FILTERS_H

#include "core.h"
#include "util.h"
#include <stdint.h>

/**
 * @brief Performs edge detection using Prewitt operator. Similiar to Sobel, but with a kernel that is more uniform 
 * and does not place as much emphasis on the center pixels.
 * @param Input The image input data buffer.    
 * @param Output The image output data buffer.
 * @param Width The width of the image in pixels.
 * @param Height The height of the image in pixels.
 * @param Channels The numbers of color channels in the image.
 * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
 */
OC_STATUS ocularPrewittEdgeDetect(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels);

/**
 * @brief Performs edge detection using Roberts Cross operator. Emphasizes edge detection along the
 * diagonals of the image.
 * @param Input The image input data buffer.
 * @param Output The image output data buffer.
 * @param Width The width of the image in pixels.
 * @param Height The height of the image in pixels.
 * @param Channels The numbers of color channels in the image.
 * @return OC_STATUS 
 */
OC_STATUS ocularRobertsEdgeDetect(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels);

/**
 * @brief Performs edge detection using Laplacian of Gaussian (LoG). Smooths the image with a Gaussian filter before 
 * applying the Laplacian operator to highlight edges. This combination allows for effective edge detection while 
 * minimizing the impact of noise.
 * @param Input The image input data buffer.
 * @param Output The image output data buffer.
 * @param Width The width of the image in pixels.
 * @param Height The height of the image in pixels.
 * @param Channels The numbers of color channels in the image.
 * @param sigma The sigma value for the LoG filter.
 * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
 */
OC_STATUS ocularLaplacianEdgeDetect(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, float sigma);

typedef enum
{
    CannyGaus3x3,
    CannyGaus5x5
} CannyNoiseFilter;

// Gaussian blur
// 3 x 3 kernel
static const int8_t Gaus3x3[] = { 1, 2, 1, 2, 4, 2, // * 1/16
                                    1, 2, 1 };
static const int Gaus3x3Div = 16;

static const int8_t Gaus5x5[] = { 2, 4, 5,  4, 2, 4, 9, 12, 9, 4, 5, 12, 15, 12, 5, // * 1/159
                                    4, 9, 12, 9, 4, 2, 4, 5,  4, 2 };
static const int Gaus5x5Div = 159;

/**
 * @brief Performs Canny edge detection on an image.
 * This is one of the most reliable methods of edge detection.
 * @ingroup group_ip_filters
 * @param Input The image input data buffer (expects grayscale image).
 * @param Output The image output data buffer.
 * @param Width The width of the image in pixels.
 * @param Height The height of the image in pixels.
 * @param kernel_size The type of gaussian noise to apply. Gaus3x3 or Gaus5x5. Good default Gaus3x3
 * @param weak_threshold Pixel values below this limit are discarded. Range [0 - 255]. Good default 200.
 * @param strong_threshold Pixel values above this limit are to be considered edge pixels. Range [0 - 255]. 
 * Good default 200.
 * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
 */
OC_STATUS ocularCannyEdgeDetect(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels,
                            CannyNoiseFilter kernel_size, int weak_threshold, int strong_threshold);

/** @brief Applies a sobel edge detection filter
 *  @ingroup group_ip_filters
 *  @param Input The image input data buffer (expects grayscale image).
 *  @param Output The image output data buffer.
 *  @param Width The width of the image in pixels.
 *  @param Height The height of the image in pixels.
 *  @param Channels The numbers of color channels in the image.
 *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
 */
OC_STATUS ocularSobelEdgeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels);

/**
 * @brief Performs edge detection on an image based on the Sobel operator.
 * @ingroup group_ip_filters group_inplace
 * @param Input The image input data buffer (expects grayscale image).
 * @param Output The image output data buffer.
 * @param Width The width of the image in pixels.
 * @param Height The height of the image in pixels.
 * @param Channels The numbers of color channels in the image.
 * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
 */
OC_STATUS ocularGradientEdgeDetect(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels);

#endif // OCULAR_EDGE_FILTERS_H