#ifndef OCULAR_CORE_H
#define OCULAR_CORE_H

#include <stdbool.h>

/** @enum OcEdgeMode
 * @brief Edge handling mode to apply when certain filters radius' are out of image bounds.
 */
typedef enum {
    OC_EDGE_WRAP = 0,  // repeat edge pixel
    OC_EDGE_MIRROR = 1 // mirror edge pixel
} OcEdgeMode;

/**
 * @todo Implement these within the filters
 * @brief Return status codes for filter exported functions
 */
typedef enum {
    OC_STATUS_OK = 0,                   //	Normal, no errors
    OC_STATUS_ERR_OUTOFMEMORY = 1,      //	Out of memory
    OC_STATUS_ERR_STACKOVERFLOW = 2,    //	Stack overflow
    OC_STATUS_ERR_NULLREFERENCE = 3,    //	Empty reference
    OC_STATUS_ERR_INVALIDPARAMETER = 4, //	The parameters are not within the normal range
    OC_STATUS_ERR_PARAMISMATCH = 5,     //	Parameter mismatch
    OC_STATUS_ERR_INDEXOUTOFRANGE = 6,
    OC_STATUS_ERR_NOTSUPPORTED = 7,
    OC_STATUS_ERR_OVERFLOW = 8,
    OC_STATUS_ERR_FILENOTFOUND = 9,
    OC_STATUS_ERR_UNKNOWN
} OC_STATUS;

/**
 * @enum OC_BITDEPTH
 * @brief  Image bit depth data types
 */
typedef enum {
    OC_DEPTH_8U = 0,  //	unsigned char
    OC_DEPTH_8S = 1,  //	char
    OC_DEPTH_16S = 2, //	short
    OC_DEPTH_32S = 3, //    int
    OC_DEPTH_32F = 4, //	float
    OC_DEPTH_64F = 5, //	double
} OC_BITDEPTH;

/**
 * @struct OcImage Image data structure
 * @var Width The width of the image
 * @var Height The height of the image
 * @var Stride The number of bytes occupied by a line element
 * @var Channels Number of color channels
 * @var Depth The image bit depth data type
 * @var Data image buffer data
 * @var Reserved Reserved use (not yet used)
 */
typedef struct {
    int Width;
    int Height;
    int Stride;
    int Channels;
    int Depth;
    unsigned char* Data;
    int Reserved;
} OcImage;


void* AllocMemory(unsigned int Size, bool ZeroMemory);
void FreeMemory(void* ptr);

// Obtain the number of bytes actually occupied by an element based on the type of the OcImage element.
int OC_ELEMENT_SIZE(int Depth);

/**
 * @brief Allocates a new image data structure in memory.
 * @ingroup group_ip_utility
 * @param Width The width of the image.
 * @param Height The height of the image.
 * @param Depth The color depth of the image.
 * @param Channel The number of color channels of the image.
 * @param[out] image The returned image data structure.
 * @return 0 if success, otherwise fail.
 */
OC_STATUS ocularCreateImage(int Width, int Height, int Depth, int Channel, OcImage** image);

/**
 * @brief Releases a created image data structure from memory.
 * @ingroup group_ip_utility
 * @param image The image data structure that needs to be released.
 * @return 0 if success, otherwise fail.
 */
OC_STATUS ocularFreeImage(OcImage** image);

/**
 * @brief Clone an existing image.
 * @ingroup group_ip_utility
 * @param Input The image data structure to copy.
 * @param[out] Output The returned image data structure. This should be empty and not previously allocated.
 * @return 0 if success, otherwise fail.
 */
OC_STATUS ocularCloneImage(OcImage* Input, OcImage** Output);

/**
 * @brief Transpose an image by swapping the height and width data.
 * @param Input The data structure of the source image to be processed. Must be single channel.
 * @param Output The data structure of the processed image.
 */
OC_STATUS ocularTransposeImage(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride);

#endif // OCULAR_CORE_H
