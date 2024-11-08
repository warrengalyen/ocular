/**
 * @file: palette.h
 * @author Warren Galyen
 * Created: 11-8-2024
 * Last Updated: 11-8-2024
 * Last update: add median cut and octree quantization
 *
 * @brief Ocular color quantization functions
 */


#ifndef OCULAR_QUANITIZE_H
#define OCULAR_QUANITIZE_H

#include <stdbool.h>
#include "color.h"
#include "core.h"
#include "palette.h"

typedef enum {
    OC_QUANTIZE_MEDIAN_CUT,
    OC_QUANTIZE_OCTREE,
    OC_QUANTIZE_NEUQUANT,
} OcQuantizeMethod;

/* --------------------------------------------------------------------------------------------*/
/* ------------------------------ Median Cut quantization -------------------------------------*/
/* --------------------------------------------------------------------------------------------*/

typedef struct {
    unsigned char r, g, b;
    int count; // Track pixel frequency
} ColorCount;

typedef struct {
    ColorCount* colors;
    int numColors;
    int capacity;
    unsigned char minR, maxR;
    unsigned char minG, maxG;
    unsigned char minB, maxB;
    double variance[3]; // Variance for each channel
} ColorBox;

/* --------------------------------------------------------------------------------------------*/
/* ------------------------------ Octree quantization -----------------------------------------*/
/* --------------------------------------------------------------------------------------------*/

typedef struct OctreeNode {
    int isLeaf;
    int pixelCount;
    long redSum, greenSum, blueSum;
    struct OctreeNode* children[8];
    struct OctreeNode* next; // For reduction linked list
} OctreeNode;

typedef struct {
    OctreeNode* root;
    OctreeNode* reducibleNodes[8]; // Array of linked lists for each level
    int numLeaves;
    int maxColors;
} Octree;

// Generates an optimal palette using the median cut algorithm
bool generateOptimalPaletteMedianCut(unsigned char* image, int width, int height, int channels, int maxColors, OcPalette* palette);

// Generates an optimal palette using the octree algorithm
bool generateOptimalPaletteOctree(unsigned char* image, int width, int height, int channels, int maxColors, OcPalette* palette);

#endif /* OCULAR_QUANITIZE_H */
