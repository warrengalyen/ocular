#ifndef OCULAR_DITHER_H
#define OCULAR_DITHER_H

#include "palette.h"

typedef enum {
    OC_DITHER_NONE,
    OC_DITHER_BURKES,
    OC_DITHER_FLOYD_STEINBERG,
    OC_DITHER_STUCKI,
    OC_DITHER_ATKINSON,
    OC_DITHER_SIERRA,
    OC_DITHER_SIERRA_TWO_ROW,
    OC_DITHER_SIERRA_LITE,
    OC_DITHER_JJN,
    OC_DITHER_SINGLE_NEIGHBOR,
    OC_DITHER_BAYER_4X4,
    OC_DITHER_BAYER_8X8,
} OcDitherMethod;

// Define dithering matrix structure
typedef struct {
    int width;          // Matrix width
    int height;         // Matrix height
    int xOffset;        // X offset of current pixel in matrix
    int yOffset;        // Y offset of current pixel in matrix
    float* weights;     // Weight values
    float divisor;      // Divisor for weights
} DitherMatrix;

// Structure for ordered dithering
typedef struct {
    int size;               // Matrix size (must be power of 2)
    const float* threshold; // Threshold values
} OrderedDitherMatrix;

/* Palette mapping  -------------------------------------------------*/

// for k-d tree nodes
typedef struct KDNode {
    OcColor* color;
    struct KDNode* left;
    struct KDNode* right;
    int depth;
} KDNode;

// Apply color remap using k-d tree for nearest neighbor search
bool applyColorRemapKDTree(unsigned char* input, unsigned char* output, int width, int height, int channels, OcPalette* palette);

/* End Palette mapping  ---------------------------------------------*/


bool applyErrorDiffusionDither(unsigned char* input, unsigned char* output, int width, int height, int channels,
                               OcPalette* palette, KDNode* tree, const DitherMatrix* matrix, float amount);

bool applyOrderedDither(unsigned char* input, unsigned char* output, int width, int height, int channels, OcPalette* palette,
                        const OrderedDitherMatrix* matrix, float amount);

bool applyDithering(unsigned char* input, unsigned char* output, int width, int height, int channels, OcPalette* palette,
                    OcDitherMethod method, float amount);                             

#endif // OCULAR_DITHER_H
