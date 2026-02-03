#ifndef OCULAR_MORPHOLOGY_FILTERS_H
#define OCULAR_MORPHOLOGY_FILTERS_H

#include "core.h"
#include "util.h"
#include "blur_filters.h"

/**
 * @brief Performs a minimum rank filter that replaces the central pixel with the darkest one in the radius.
 *  @ingroup group_ip_filters
 *  @param Input The image input data buffer.
 *  @param Output The image output data buffer.
 *  @param Width The width of the image in pixels.
 *  @param Height The height of the image in pixels.
 *  @param Stride The number of bytes in one row of pixels.
 *  @param Radius A radius in pixels to use for calculation, >= 0
 *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
 */
OC_STATUS ocularErodeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

/**
 * @brief Performs a maximum rank filter that replaces the central pixel with the lightest one in the radius.
 *  @ingroup group_ip_filters
 *  @param Input The image input data buffer.
 *  @param Output The image output data buffer.
 *  @param Width The width of the image in pixels.
 *  @param Height The height of the image in pixels.
 *  @param Stride The number of bytes in one row of pixels.
 *  @param Radius A radius in pixels to use for calculation, >= 0
 *  @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
 */
OC_STATUS ocularDilateFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

/**
 * @brief Apply a minimum filter. This is the same as Erode except it uses a circular kernel that accounts for edges.
 * @ingroup group_ip_filters
 * @param Input The image input data buffer
 * @param Output The image output data buffer
 * @param Width The width of the image in pixels
 * @param Height The height of the image in pixels
 * @param Stride The number of bytes in one row of pixels
 * @param Radius The radius of the kernel. Range [1 - 256]
 * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
 */
OC_STATUS ocularMinFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

/**
 * @brief Apply a maximum filter. This is the same as Dilate except it uses a circular kernel that accounts for edges.
 * @ingroup group_ip_filters
 * @param Input The image input data buffer
 * @param Output The image output data buffer
 * @param Width The width of the image in pixels
 * @param Height The height of the image in pixels
 * @param Stride The number of bytes in one row of pixels
 * @param Radius The radius of the kernel. Range [1 - 256]
 * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
 */
OC_STATUS ocularMaxFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

/**
 * @brief Perform a high pass filter that emphasizes high-frequency components (edges and details) while reducing
 * low-frequency components (smooth areas)
 * @ingroup group_ip_filters
 * @param Input The image input data buffer.
 * @param Output The image output data buffer.
 * @param Width The width of the image in pixels.
 * @param Height The height of the image in pixels.
 * @param Stride The number of bytes in one row of pixels.
 * @param Radius Controls the cutoff frequency - larger values will remove more low frequency content, resulting in
 * a more pronounced high pass effect. Range [1 - 512]
 * @return OC_STATUS_OK if successful, otherwise an error code (see core.h)
 */
OC_STATUS ocularHighPassFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius);

#endif /* OCULAR_MORPHOLOGY_FILTERS_H */
