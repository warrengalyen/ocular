#ifndef OCULAR_DITHER_H
#define OCULAR_DITHER_H

#include "palette.h"
// #include "util.h"

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

// Define dithering matrices

static const DitherMatrix BURKES_MATRIX = {
    .width = 5,
    .height = 2,
    .xOffset = 2,
    .yOffset = 0,
    .divisor = 32.0f,
    .weights = (float[]){
        0, 0, 0, 8, 4,
        2, 4, 8, 4, 2
    }
};

static const float FLOYD_STEINBERG_WEIGHTS[] = {
    0, 0, 7,
    3, 5, 1
};
static const DitherMatrix FLOYD_STEINBERG_MATRIX = {
    .width = 3,
    .height = 2,
    .xOffset = 0,
    .yOffset = 0,
    .weights = (float*)FLOYD_STEINBERG_WEIGHTS,
    .divisor = 16.0f
};

static const float STUCKI_WEIGHTS[] = {
    0, 0, 8, 4, 2,
    2, 4, 8, 4, 2,
    1, 2, 4, 2, 1
};
static const DitherMatrix STUCKI_MATRIX = {
    .width = 5,
    .height = 3,
    .xOffset = 0,
    .yOffset = 0,
    .weights = (float*)STUCKI_WEIGHTS,
    .divisor = 42.0f
};

static const float ATKINSON_WEIGHTS[] = {
    0, 0, 1, 1,
    1, 1, 1, 0,
    0, 1, 0, 0
};
static const DitherMatrix ATKINSON_MATRIX = {
    .width = 4,
    .height = 3,
    .xOffset = 0,
    .yOffset = 0,
    .weights = (float*)ATKINSON_WEIGHTS,
    .divisor = 8.0f
};

static const float SIERRA_WEIGHTS[] = {
    0, 0, 0, 5, 3,
    2, 4, 5, 4, 2,
    0, 2, 3, 2, 0
};
static const DitherMatrix SIERRA_MATRIX = {
    .width = 5,
    .height = 3,
    .xOffset = 2,
    .yOffset = 0,
    .weights = (float*)SIERRA_WEIGHTS,
    .divisor = 32.0f
};

static const float SIERRA_TWO_ROW_WEIGHTS[] = {
    0, 0, 0, 4, 3,
    1, 2, 3, 2, 1
};
static const DitherMatrix SIERRA_TWO_ROW_MATRIX = {
    .width = 5,
    .height = 2,
    .xOffset = 2,
    .yOffset = 0,
    .weights = (float*)SIERRA_TWO_ROW_WEIGHTS,
    .divisor = 16.0f
};

static const float SIERRA_LITE_WEIGHTS[] = {
    0, 0, 2,
    1, 1, 0
};
static const DitherMatrix SIERRA_LITE_MATRIX = {
    .width = 3,
    .height = 2,
    .xOffset = 1,
    .yOffset = 0,
    .weights = (float*)SIERRA_LITE_WEIGHTS,
    .divisor = 4.0f
};

static const float JJN_WEIGHTS[] = {
    0, 0, 0, 7, 5,
    3, 5, 7, 5, 3,
    1, 3, 5, 3, 1
};
static const DitherMatrix JJN_MATRIX = {
    .width = 5,
    .height = 3,
    .xOffset = 2,
    .yOffset = 0,
    .weights = (float*)JJN_WEIGHTS,
    .divisor = 48.0f
};

static const float SINGLE_NEIGHBOR_WEIGHTS[] = {
    0, 1
};
static const DitherMatrix SINGLE_NEIGHBOR_MATRIX = {
    .width = 2,
    .height = 1,
    .xOffset = 0,
    .yOffset = 0,
    .weights = (float*)SINGLE_NEIGHBOR_WEIGHTS,
    .divisor = 1.0f
};

// Structure for ordered dithering
typedef struct {
    int size;           // Matrix size (must be power of 2)
    const float* threshold;   // Threshold values
} OrderedDitherMatrix;

// 4x4 Bayer matrix
static const float BAYER_4X4_THRESHOLD[] = {
     0,  8,  2, 10,
    12,  4, 14,  6,
     3, 11,  1,  9,
    15,  7, 13,  5
};
static const OrderedDitherMatrix BAYER_4X4_MATRIX = {
    .size = 4,
    .threshold = (float*)BAYER_4X4_THRESHOLD
};

// 8x8 Bayer matrix
static const float BAYER_8X8_THRESHOLD[] = {
     0, 32,  8, 40,  2, 34, 10, 42,
    48, 16, 56, 24, 50, 18, 58, 26,
    12, 44,  4, 36, 14, 46,  6, 38,
    60, 28, 52, 20, 62, 30, 54, 22,
     3, 35, 11, 43,  1, 33,  9, 41,
    51, 19, 59, 27, 49, 17, 57, 25,
    15, 47,  7, 39, 13, 45,  5, 37,
    63, 31, 55, 23, 61, 29, 53, 21
};
static const OrderedDitherMatrix BAYER_8X8_MATRIX = {
    .size = 8,
    .threshold = (float*)BAYER_8X8_THRESHOLD
};

bool applyErrorDiffusionDither(unsigned char* input, unsigned char* output, int width, int height, int channels,
                               OcPalette* palette, KDNode* tree, const DitherMatrix* matrix, float amount);

bool applyOrderedDither(unsigned char* input, unsigned char* output, int width, int height, int channels, OcPalette* palette,
                        const OrderedDitherMatrix* matrix, float amount);

bool applyDithering(unsigned char* input, unsigned char* output, int width, int height, int channels, OcPalette* palette,
                    OcDitherMethod method, float amount);                             

#endif // OCULAR_DITHER_H
