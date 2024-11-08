#include "dither.h"
#include "color.h"
#include "util.h"

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

/* Palette mapping  -------------------------------------------------*/

// Calculate Euclidean distance between two colors
static inline float colorDistance(OcColor* c1, OcColor* c2) {
    float dr = c1->R - c2->R;
    float dg = c1->G - c2->G;
    float db = c1->B - c2->B;
    return dr * dr + dg * dg + db * db;
}

// Compare colors for qsort based on current axis
static inline int compareColors(const void* a, const void* b, int axis) {
    OcColor* c1 = (OcColor*)a;
    OcColor* c2 = (OcColor*)b;

    switch (axis) {
    case 0: return c1->R - c2->R;
    case 1: return c1->G - c2->G;
    case 2: return c1->B - c2->B;
    }
    return 0;
}

// Sort colors along a given axis
static void qsortColors(OcColor* colors, int numColors, int axis) {
    if (numColors <= 1)
        return;

    // Simple partition implementation
    OcColor pivot = colors[numColors / 2];
    int i = 0;
    int j = numColors - 1;

    while (i <= j) {
        while (compareColors(&colors[i], &pivot, axis) < 0)
            i++;
        while (compareColors(&colors[j], &pivot, axis) > 0)
            j--;

        if (i <= j) {
            OcColor temp = colors[i];
            colors[i] = colors[j];
            colors[j] = temp;
            i++;
            j--;
        }
    }

    if (j > 0)
        qsortColors(colors, j + 1, axis);
    if (i < numColors)
        qsortColors(colors + i, numColors - i, axis);
}

// Build k-d tree from array of colors
static KDNode* buildKDTree(OcColor* colors, int numColors) {
    if (numColors == 0)
        return NULL;

    KDNode* node = (KDNode*)malloc(sizeof(KDNode));
    node->depth = 0;

    // Sort colors based on current axis (R->G->B)
    int axis = node->depth % 3;
    qsortColors(colors, numColors, axis);

    // Find median
    int medianIdx = numColors / 2;
    node->color = &colors[medianIdx];

    // Recursively build left and right subtrees
    node->left = buildKDTree(colors, medianIdx);
    if (node->left)
        node->left->depth = node->depth + 1;

    node->right = buildKDTree(colors + medianIdx + 1, numColors - medianIdx - 1);
    if (node->right)
        node->right->depth = node->depth + 1;

    return node;
}

// Recursively search for nearest neighbor in k-d tree
static void searchNearest(KDNode* node, OcColor* target, OcColor** best, float* bestDist) {
    if (!node)
        return;

    float dist = colorDistance(target, node->color);
    if (dist < *bestDist) {
        *bestDist = dist;
        *best = node->color;
    }

    int axis = node->depth % 3;
    int diff;
    switch (axis) {
    case 0: diff = target->R - node->color->R; break;
    case 1: diff = target->G - node->color->G; break;
    case 2: diff = target->B - node->color->B; break;
    }

    KDNode* first = diff < 0 ? node->left : node->right;
    KDNode* second = diff < 0 ? node->right : node->left;

    searchNearest(first, target, best, bestDist);

    if (diff * diff < *bestDist) {
        searchNearest(second, target, best, bestDist);
    }
}

// Find nearest neighbor in k-d tree
static OcColor* findNearestNeighbor(KDNode* root, OcColor* target) {
    if (!root)
        return NULL;

    OcColor* best = root->color;
    float bestDist = colorDistance(target, best);

    searchNearest(root, target, &best, &bestDist);
    return best;
}

// Free k-d tree memory
static void freeKDTree(KDNode* node) {
    if (!node)
        return;
    freeKDTree(node->left);
    freeKDTree(node->right);
    free(node);
}

bool applyColorRemapKDTree(unsigned char* input, unsigned char* output, int width, int height, int channels, OcPalette* palette) {

    bool success = false;

    // convert palette to array of OcColor
    OcColor* colors = (OcColor*)malloc(palette->num_colors * sizeof(OcColor));
    for (int i = 0; i < palette->num_colors; i++) {
        colors[i].R = palette->colors[i].r;
        colors[i].G = palette->colors[i].g;
        colors[i].B = palette->colors[i].b;
    }

    KDNode* root = buildKDTree(colors, palette->num_colors);

    // Process each pixel
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * channels;

            // Create color point from input pixel
            OcColor inputColor = { input[idx], input[idx + 1], input[idx + 2] };

            // Find nearest color in palette using k-d tree
            OcColor* nearestColor = findNearestNeighbor(root, &inputColor);

            // Apply nearest color to output
            output[idx] = nearestColor->R;
            output[idx + 1] = nearestColor->G;
            output[idx + 2] = nearestColor->B;

            // Preserve alpha if present
            if (channels == 4) {
                output[idx + 3] = input[idx + 3];
            }
        }
    }

    success = true;

    // Clean up k-d tree
    freeKDTree(root);
    free(colors);

    return success;
}

/* End Palette mapping  --------------------------------------------*/

bool applyErrorDiffusionDither(unsigned char* input, unsigned char* output, int width, int height, int channels, OcPalette* palette,
                               KDNode* tree, const DitherMatrix* matrix, float amount) {

    bool success = false;
    // Allocate error buffers
    float* errorR = (float*)calloc(width * height, sizeof(float));
    float* errorG = (float*)calloc(width * height, sizeof(float));
    float* errorB = (float*)calloc(width * height, sizeof(float));

    amount = amount / 100.0f; // Convert to 0-1 range

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * channels;
            int bufIdx = y * width + x;

            // Add accumulated error to current pixel
            OcColor inputColor = { (unsigned char)fmin(255, fmax(0, input[idx] + errorR[bufIdx] * amount)),
                                   (unsigned char)fmin(255, fmax(0, input[idx + 1] + errorG[bufIdx] * amount)),
                                   (unsigned char)fmin(255, fmax(0, input[idx + 2] + errorB[bufIdx] * amount)) };

            // Find nearest palette color
            OcColor* nearestColor = findNearestNeighbor(tree, &inputColor);

            // Calculate error
            float errR = inputColor.R - nearestColor->R;
            float errG = inputColor.G - nearestColor->G;
            float errB = inputColor.B - nearestColor->B;

            // Distribute error according to matrix
            for (int my = 0; my < matrix->height; my++) {
                int py = y + my - matrix->yOffset;
                if (py >= height)
                    continue;

                for (int mx = 0; mx < matrix->width; mx++) {
                    int px = x + mx - matrix->xOffset;
                    if (px < 0 || px >= width)
                        continue;

                    float weight = matrix->weights[my * matrix->width + mx] / matrix->divisor;
                    if (weight == 0)
                        continue;

                    int offset = py * width + px;
                    errorR[offset] += errR * weight;
                    errorG[offset] += errG * weight;
                    errorB[offset] += errB * weight;
                }
            }

            // Write output pixel
            output[idx] = nearestColor->R;
            output[idx + 1] = nearestColor->G;
            output[idx + 2] = nearestColor->B;
            if (channels == 4) {
                output[idx + 3] = input[idx + 3];
            }
        }
    }

    success = true;

    free(errorR);
    free(errorG);
    free(errorB);

    return success;
}

bool applyOrderedDither(unsigned char* input, unsigned char* output, int width, int height, int channels, OcPalette* palette,
                        const OrderedDitherMatrix* matrix, float amount) {

    bool success = false;

    // Convert palette to array of OcColor for KD-tree
    OcColor* colors = (OcColor*)malloc(palette->num_colors * sizeof(OcColor));
    for (int i = 0; i < palette->num_colors; i++) {
        colors[i].R = palette->colors[i].r;
        colors[i].G = palette->colors[i].g;
        colors[i].B = palette->colors[i].b;
    }

    // Build KD-tree for faster nearest neighbor search
    KDNode* tree = buildKDTree(colors, palette->num_colors);

    // Scale amount to 0-1 range
    amount = amount / 100.0f;

    // Process each pixel
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * channels;

            // Get threshold from matrix (repeating pattern)
            float threshold = matrix->threshold[(y % matrix->size) * matrix->size + (x % matrix->size)];
            threshold = threshold * amount;

            // Apply threshold to each channel
            OcColor inputColor = { (unsigned char)fmin(255, fmax(0, input[idx] + threshold)),
                                   (unsigned char)fmin(255, fmax(0, input[idx + 1] + threshold)),
                                   (unsigned char)fmin(255, fmax(0, input[idx + 2] + threshold)) };

            // Find nearest palette color
            OcColor* nearestColor = findNearestNeighbor(tree, &inputColor);

            // Write output pixel
            output[idx] = nearestColor->R;
            output[idx + 1] = nearestColor->G;
            output[idx + 2] = nearestColor->B;
            if (channels == 4) {
                output[idx + 3] = input[idx + 3];
            }
        }
    }

    success = true;

    // Cleanup
    freeKDTree(tree);
    free(colors);

    return success;
}


bool applyDithering(unsigned char* input, unsigned char* output, int width, int height, int channels, OcPalette* palette,
                    OcDitherMethod method, float amount) {
    // Convert palette to array of OcColor
    OcColor* colors = (OcColor*)malloc(palette->num_colors * sizeof(OcColor));
    for (int i = 0; i < palette->num_colors; i++) {
        colors[i].R = palette->colors[i].r;
        colors[i].G = palette->colors[i].g;
        colors[i].B = palette->colors[i].b;
    }

    bool success = false;

    // Build KD-tree for faster nearest neighbor search
    KDNode* tree = buildKDTree(colors, palette->num_colors);

    // Select dithering matrix based on method
    const DitherMatrix* matrix = NULL;
    const OrderedDitherMatrix* orderedMatrix = NULL;
    switch (method) {
        case OC_DITHER_ATKINSON: matrix = &ATKINSON_MATRIX; break;
        case OC_DITHER_BURKES: matrix = &BURKES_MATRIX; break;
        case OC_DITHER_FLOYD_STEINBERG: matrix = &FLOYD_STEINBERG_MATRIX; break;
        case OC_DITHER_STUCKI: matrix = &STUCKI_MATRIX; break;
        case OC_DITHER_SIERRA: matrix = &SIERRA_MATRIX; break;
        case OC_DITHER_SIERRA_TWO_ROW: matrix = &SIERRA_TWO_ROW_MATRIX; break;
        case OC_DITHER_SIERRA_LITE: matrix = &SIERRA_LITE_MATRIX; break;
        case OC_DITHER_JJN: matrix = &JJN_MATRIX; break;
        case OC_DITHER_SINGLE_NEIGHBOR: matrix = &SINGLE_NEIGHBOR_MATRIX; break;
        case OC_DITHER_BAYER_4X4: orderedMatrix = &BAYER_4X4_MATRIX; break;
        case OC_DITHER_BAYER_8X8: orderedMatrix = &BAYER_8X8_MATRIX; break;
        case OC_DITHER_NONE:
        default: 
            if (applyColorRemapKDTree(input, output, width, height, channels, palette)) {
                success = true;
            }
            goto cleanup;
    }

    if (matrix) {
        if (applyErrorDiffusionDither(input, output, width, height, channels, palette, tree, matrix, amount)) {
            success = true;
        }
    } else if (orderedMatrix) {
        if (applyOrderedDither(input, output, width, height, channels, palette, orderedMatrix, amount)) {
            success = true;
        }
    }

cleanup:
    freeKDTree(tree);
    free(colors);

    return success;
}


