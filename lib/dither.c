#include "dither.h"
#include "color.h"
#include "util.h"

void applyErrorDiffusionDither(unsigned char* input, unsigned char* output, int width, int height, int channels, OcPalette* palette,
                               KDNode* tree, const DitherMatrix* matrix, float amount) {
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

    free(errorR);
    free(errorG);
    free(errorB);
}

void applyOrderedDither(unsigned char* input, unsigned char* output, int width, int height, int channels, OcPalette* palette,
                        const OrderedDitherMatrix* matrix, float amount) {

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

    // Cleanup
    freeKDTree(tree);
    free(colors);
}


void applyDithering(unsigned char* input, unsigned char* output, int width, int height, int channels, OcPalette* palette,
                    OcDitherMethod method, float amount) {
    // Convert palette to array of OcColor
    OcColor* colors = (OcColor*)malloc(palette->num_colors * sizeof(OcColor));
    for (int i = 0; i < palette->num_colors; i++) {
        colors[i].R = palette->colors[i].r;
        colors[i].G = palette->colors[i].g;
        colors[i].B = palette->colors[i].b;
    }

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
        default: applyColorRemapKDTree(input, output, width, height, channels, palette); goto cleanup;
    }

    if (matrix) {
        applyErrorDiffusionDither(input, output, width, height, channels, palette, tree, matrix, amount);
    } else if (orderedMatrix) {
        applyOrderedDither(input, output, width, height, channels, palette, orderedMatrix, amount);
    }

cleanup:
    freeKDTree(tree);
    free(colors);
}


