/**
 * @file: palette.h
 * @author Warren Galyen
 * Created: 10-23-2024
 * Last Updated: 10-29-2024
 * Last update: improved palette loading memory management
 *
 * @brief Ocular palette file format import/export functions
 */

#ifndef OCULAR_PALETTE_H
#define OCULAR_PALETTE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "core.h"

// Convert a 16-bit value from big-endian to little-endian
#define BIG_ENDIAN_16(x) (((x) >> 8) | ((x) << 8))

// Convert a 32-bit value from big-endian to little-endian
#define BIG_ENDIAN_32(x) ((((x) >> 24) & 0xFF) | (((x) >> 8) & 0xFF00) | (((x) << 8) & 0xFF0000) | (((x) << 24) & 0xFF000000))

#define DEFAULT_PALETTE_CAPACITY 16

typedef enum
{
    FORMAT_UNKNOWN,
    FORMAT_GIMP,
    FORMAT_RIFF,
    FORMAT_ACO,
    FORMAT_PAINTNET,
    FORMAT_ACT,
    FORMAT_ASE
} PaletteFormat;

typedef struct {
    int r;
    int g;
    int b;
    char name[256];
} OcPaletteColor;

typedef struct {
    char name[256];
    int num_colors;
    int capacity;
    OcPaletteColor* colors;
} OcPalette;

// ACO-specific structures
typedef struct {
    unsigned short colorSpace;
    unsigned short w; // Color components
    unsigned short x;
    unsigned short y;
    unsigned short z;
} AcoColorEntry;

// ASE-specific structures and constants
#define ASE_SIGNATURE 0x41534546 // "ASEF" in hex
#define ASE_VERSION_MAJOR 1
#define ASE_VERSION_MINOR 0

#define ASE_BLOCK_COLOR 0x0001
#define ASE_BLOCK_GROUP_START 0xc001
#define ASE_BLOCK_GROUP_END 0xc002

#define ASE_COLOR_RGB 0x52474220  // "RGB " in hex
#define ASE_COLOR_CMYK 0x434D594B // "CMYK" in hex
#define ASE_COLOR_LAB 0x4C414220  // "LAB " in hex
#define ASE_COLOR_GRAY 0x47726179 // "Gray" in hex

/* Palette mapping  -------------------------------------------------*/

// for k-d tree nodes
typedef struct KDNode {
    OcColor* color;
    struct KDNode* left;
    struct KDNode* right;
    int depth;
} KDNode;

// Compare colors for qsort based on current axis
int compareColors(const void* a, const void* b, int axis);

// Build k-d tree from array of colors
KDNode* buildKDTree(OcColor* colors, int numColors);

// Calculate Euclidean distance between two colors
float colorDistance(OcColor* c1, OcColor* c2);

// Recursively search for nearest neighbor in k-d tree
void searchNearest(KDNode* node, OcColor* target, OcColor** best, float* bestDist);

// Find nearest neighbor in k-d tree
OcColor* findNearestNeighbor(KDNode* root, OcColor* target);

// Free k-d tree memory
void freeKDTree(KDNode* node);

// Apply color remap using k-d tree for nearest neighbor search
void applyColorRemapKDTree(unsigned char* input, unsigned char* output, int width, int height, int channels, OcPalette* palette);


/* End Palette mapping  ---------------------------------------------*/


// Convert a float from big-endian to little-endian, byte by byte, maintaining precision
static float swap_float_endian(float value);

/**
 * @brief Frees a palette.
 * @ingroup group_palette
 * @param palette The palette to free.
 */
void ocularFreePalette(OcPalette* palette);

// allocates more memory for palette if needed
bool resize_palette(OcPalette* palette);

/**
 * Read a GIMP palette file.
 * @ingroup group_palette
 * @param filename The path to the GIMP palette file.
 * @param palette_data The palette to load the data into.
 */
OC_STATUS read_gimp_palette(const char* filename, OcPalette* palette_data);

/**
 * Save palette data to a GIMP palette file.
 * @ingroup group_palette
 * @param filename The path to the file to write to.
 * @param palette The palette to save.
 */
OC_STATUS save_gimp_palette(const char* filename, const OcPalette* palette);

/**
 * Read a Microsoft RIFF palette file.
 * @ingroup group_palette
 * @param filename The path to the RIFF palette file.
 * @param palette The palette to load the data into.
 */
OC_STATUS read_riff_palette(const char* filename, OcPalette* palette);

/**
 * Save palette data to a Microsoft RIFF palette file.
 * @ingroup group_palette
 * @param filename The path to the file to write to.
 * @param palette The palette to save.
 */
OC_STATUS save_riff_palette(const char* filename, const OcPalette* palette);

/**
 * Read a UTF-16 encoded string from a file.    
 * @param file The file to read from.
 * @param output The buffer to store the string in.
 * @param length The maximum length of the string to read.
 */
void read_utf16_string(FILE* file, char* output, int length);

/**
 * Read the color swatches from an ACO file.
 * @param file The file to read from.
 * @param palette_data The palette to load the data into.
 * @param version The version of the ACO file.
 * @return 0 on success, -1 on failure
 */
int read_swatches(FILE* file, OcPalette* palette_data, unsigned short version);

/**
 * Read an Adobe Color Swatch (.aco) file.
 * @ingroup group_palette
 * @param filename The path to the ACO file.
 * @param palette The palette to load the data into.
 */
OC_STATUS read_aco_palette(const char* filename, OcPalette* palette);

/**
 * Save palette data to an Adobe Color Swatch (.aco) file. Currently only supports RGB colorspace
 * @ingroup group_palette
 * @param filename The path to the file to write to.
 * @param palette The palette to save.
 */
OC_STATUS save_aco_palette(const char* filename, const OcPalette* palette);

/**
 * Read a Paint.NET palette file.
 * @ingroup group_palette
 * @param filename The path to the Paint.NET palette file.
 * @param palette_data The palette to load the data into.
 */
OC_STATUS read_paintnet_palette(const char* filename, OcPalette* palette_data);

/**
 * Save palette data to a Paint.NET palette file.
 * @ingroup group_palette
 * @param filename The path to the file to write to.
 * @param palette The palette to save.
 */
OC_STATUS save_paintnet_palette(const char* filename, const OcPalette* palette);

/**
 * Read an Adobe Color Table (.act) file.
 * @ingroup group_palette
 * @param filename The path to the ACT file.
 * @param palette The palette to load the data into.
 */
OC_STATUS read_act_palette(const char* filename, OcPalette* palette);

/**
 * Save palette data to an Adobe Color Table (.act) file.
 * @ingroup group_palette
 * @param filename The path to the file to write to.
 * @param palette The palette to save.
 */
OC_STATUS save_act_palette(const char* filename, const OcPalette* palette);

/**
 * Read an Adobe Swatch Exchange (.ase) file. Currently only supports RGB and Grayscale color spaces.
 * @ingroup group_palette
 * @param filename The path to the ASE file.
 * @param palette The palette to load the data into.
 */
OC_STATUS read_ase_palette(const char* filename, OcPalette* palette);

/**
 * Detect the format of a palette file.
 * @param filename The path to the palette file.
 * @return The format of the palette file.
 */
PaletteFormat detect_palette_format(const char* filename);

#endif /* OCULAR_PALETTE_H */