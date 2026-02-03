#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "morphology_filters.h"
#include "util.h"

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

OC_STATUS ocularErodeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius) {

    if (Input == NULL || Output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    if (Width <= 0 || Height <= 0 || Stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    // Ensure filter specific parameters are within valid ranges
    Radius = max(Radius, 1);

    int Channels = Stride / Width;

    if (Channels == 1) {
        for (int y = 0; y < Height; y++) {
            for (int x = 0; x < Width; x++) {
                int minPixelValue = 255;
                for (int ky = -Radius; ky <= Radius; ky++) {
                    for (int kx = -Radius; kx <= Radius; kx++) {
                        int offsetX = x + kx;
                        int offsetY = y + ky;
                        if (offsetX >= 0 && offsetX < Width && offsetY >= 0 && offsetY < Height) {
                            int pixelValue = Input[offsetY * Width + offsetX];
                            if (pixelValue < minPixelValue) {
                                minPixelValue = pixelValue;
                            }
                        }
                    }
                }
                for (int c = 0; c < Channels; c++) {
                    Output[y * Width + x + c] = minPixelValue;
                }
            }
        }
    }
    if (Channels == 3) {

        unsigned char* SrcR = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
        unsigned char* SrcG = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
        unsigned char* SrcB = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));

        unsigned char* DstR = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
        unsigned char* DstG = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
        unsigned char* DstB = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));

        SplitRGB(Input, SrcB, SrcG, SrcR, Width, Height, Stride);

        ocularErodeFilter(SrcR, DstR, Width, Height, Width, Radius);
        ocularErodeFilter(SrcG, DstG, Width, Height, Width, Radius);
        ocularErodeFilter(SrcB, DstB, Width, Height, Width, Radius);

        CombineRGB(DstB, DstG, DstR, Output, Width, Height, Stride);

        free(SrcR);
        free(SrcG);
        free(SrcB);
        free(DstR);
        free(DstG);
        free(DstB);
    }

    return OC_STATUS_OK;
}

OC_STATUS ocularDilateFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius) {

    if (Input == NULL || Output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    if (Width <= 0 || Height <= 0 || Stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    // Ensure filter specific parameters are within valid ranges
    Radius = max(Radius, 1);

    int Channels = Stride / Width;

    if (Channels == 1) {
        for (int y = 0; y < Height; y++) {
            for (int x = 0; x < Width; x++) {
                int maxPixelValue = 0;
                for (int ky = -Radius; ky <= Radius; ky++) {
                    for (int kx = -Radius; kx <= Radius; kx++) {
                        int offsetX = x + kx;
                        int offsetY = y + ky;
                        if (offsetX >= 0 && offsetX < Width && offsetY >= 0 && offsetY < Height) {
                            int pixelValue = Input[offsetY * Width + offsetX];
                            if (pixelValue > maxPixelValue) {
                                maxPixelValue = pixelValue;
                            }
                        }
                    }
                }
                for (int c = 0; c < Channels; c++) {
                    Output[y * Width + x + c] = maxPixelValue;
                }
            }
        }
    }
    if (Channels == 3) {

        unsigned char* SrcR = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
        unsigned char* SrcG = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
        unsigned char* SrcB = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));

        unsigned char* DstR = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
        unsigned char* DstG = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
        unsigned char* DstB = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));

        SplitRGB(Input, SrcB, SrcG, SrcR, Width, Height, Stride);

        ocularDilateFilter(SrcR, DstR, Width, Height, Width, Radius);
        ocularDilateFilter(SrcG, DstG, Width, Height, Width, Radius);
        ocularDilateFilter(SrcB, DstB, Width, Height, Width, Radius);

        CombineRGB(DstB, DstG, DstR, Output, Width, Height, Stride);

        free(SrcR);
        free(SrcG);
        free(SrcB);
        free(DstR);
        free(DstG);
        free(DstB);
    }

    return OC_STATUS_OK;
}

OC_STATUS ocularMinFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius) {

    if (Input == NULL || Output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    if (Width <= 0 || Height <= 0 || Stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    int Channels = Stride / Width;
    if (Channels != 1 && Channels != 3 && Channels != 4) {
        return OC_STATUS_ERR_NOTSUPPORTED;
    }

    // Ensure filter specific parameters are within valid ranges
    Radius = max(Radius, 1);

    // Pre-calculate circle mask
    const int maskSize = (Radius * 2 + 1);
    char* circleMask = (char*)malloc(maskSize * maskSize);
    int radiusSquared = Radius * Radius;

    for (int y = 0; y < maskSize; y++) {
        for (int x = 0; x < maskSize; x++) {
            int dx = x - Radius;
            int dy = y - Radius;
            circleMask[y * maskSize + x] = (dx * dx + dy * dy <= radiusSquared);
        }
    }

    // Process image in chunks for better cache utilization
    const int CHUNK_SIZE = 32;
    for (int blockY = 0; blockY < Height; blockY += CHUNK_SIZE) {
        for (int blockX = 0; blockX < Width; blockX += CHUNK_SIZE) {
            int endY = min(blockY + CHUNK_SIZE, Height);
            int endX = min(blockX + CHUNK_SIZE, Width);

            for (int y = blockY; y < endY; y++) {
                unsigned char* pOutput = Output + (y * Stride) + (blockX * Channels);

                for (int x = blockX; x < endX; x++) {
                    unsigned char minValues[4] = { 255, 255, 255, 255 };

                    // Scan only the pixels within the circle mask
                    for (int ky = 0; ky < maskSize; ky++) {
                        int ny = y + ky - Radius;
                        if (ny < 0 || ny >= Height)
                            continue;

                        unsigned char* pInput = Input + (ny * Stride);

                        for (int kx = 0; kx < maskSize; kx++) {
                            if (!circleMask[ky * maskSize + kx])
                                continue;

                            int nx = x + kx - Radius;
                            if (nx < 0 || nx >= Width)
                                continue;

                            unsigned char* pixel = pInput + (nx * Channels);

                            // Unrolled channel comparison
                            if (Channels >= 1 && pixel[0] < minValues[0])
                                minValues[0] = pixel[0];
                            if (Channels >= 2 && pixel[1] < minValues[1])
                                minValues[1] = pixel[1];
                            if (Channels >= 3 && pixel[2] < minValues[2])
                                minValues[2] = pixel[2];
                            if (Channels == 4 && pixel[3] < minValues[3])
                                minValues[3] = pixel[3];
                        }
                    }

                    for (int c = 0; c < Channels; c++) {
                        pOutput[c] = minValues[c];
                    }
                    pOutput += Channels;
                }
            }
        }
    }

    free(circleMask);

    return OC_STATUS_OK;
}

OC_STATUS ocularMaxFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius) {

    if (Input == NULL || Output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    if (Width <= 0 || Height <= 0 || Stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    int Channels = Stride / Width;
    if (Channels != 1 && Channels != 3 && Channels != 4) {
        return OC_STATUS_ERR_NOTSUPPORTED;
    }

    // Ensure filter specific parameters are within valid ranges
    Radius = max(Radius, 1);

    // Pre-calculate circle mask
    const int maskSize = (Radius * 2 + 1);
    char* circleMask = (char*)malloc(maskSize * maskSize);
    int radiusSquared = Radius * Radius;

    for (int y = 0; y < maskSize; y++) {
        for (int x = 0; x < maskSize; x++) {
            int dx = x - Radius;
            int dy = y - Radius;
            circleMask[y * maskSize + x] = (dx * dx + dy * dy <= radiusSquared);
        }
    }

    // Process image in chunks for better cache utilization
    const int CHUNK_SIZE = 32;
    for (int blockY = 0; blockY < Height; blockY += CHUNK_SIZE) {
        for (int blockX = 0; blockX < Width; blockX += CHUNK_SIZE) {
            int endY = min(blockY + CHUNK_SIZE, Height);
            int endX = min(blockX + CHUNK_SIZE, Width);

            for (int y = blockY; y < endY; y++) {
                unsigned char* pOutput = Output + (y * Stride) + (blockX * Channels);

                for (int x = blockX; x < endX; x++) {
                    unsigned char maxValues[4] = { 0, 0, 0, 0 };

                    // Scan only the pixels within the circle mask
                    for (int ky = 0; ky < maskSize; ky++) {
                        int ny = y + ky - Radius;
                        if (ny < 0 || ny >= Height)
                            continue;

                        unsigned char* pInput = Input + (ny * Stride);

                        for (int kx = 0; kx < maskSize; kx++) {
                            if (!circleMask[ky * maskSize + kx])
                                continue;

                            int nx = x + kx - Radius;
                            if (nx < 0 || nx >= Width)
                                continue;

                            unsigned char* pixel = pInput + (nx * Channels);

                            // Unrolled channel comparison
                            if (Channels >= 1 && pixel[0] > maxValues[0])
                                maxValues[0] = pixel[0];
                            if (Channels >= 2 && pixel[1] > maxValues[1])
                                maxValues[1] = pixel[1];
                            if (Channels >= 3 && pixel[2] > maxValues[2])
                                maxValues[2] = pixel[2];
                            if (Channels == 4 && pixel[3] > maxValues[3])
                                maxValues[3] = pixel[3];
                        }
                    }

                    // Write output values
                    for (int c = 0; c < Channels; c++) {
                        pOutput[c] = maxValues[c];
                    }
                    pOutput += Channels;
                }
            }
        }
    }

    free(circleMask);

    return OC_STATUS_OK;
}

OC_STATUS ocularHighPassFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius) {

    if (Input == NULL || Output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    if (Width <= 0 || Height <= 0 || Stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    int Channels = Stride / Width;
    if (Channels != 1 && Channels != 3 && Channels != 4) {
        return OC_STATUS_ERR_NOTSUPPORTED;
    }

    // Ensure filter specific parameters are within valid ranges
    Radius = max(Radius, 1);

    // Create temporary buffer for blur result
    unsigned char* blurBuffer = (unsigned char*)malloc((size_t)Height * Stride);
    if (blurBuffer == NULL) {
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

    // First apply Gaussian blur to get low frequency components
    ocularGaussianBlurFilter(Input, blurBuffer, Width, Height, Stride, Radius);

    // Subtract blurred image from original to get high frequency components
    for (int y = 0; y < Height; y++) {
        unsigned char* pInput = Input + (y * Stride);
        unsigned char* pBlur = blurBuffer + (y * Stride);
        unsigned char* pOutput = Output + (y * Stride);

        for (int x = 0; x < Width; x++) {
            for (int c = 0; c < (Channels == 4 ? 3 : Channels); c++) {
                // High pass = Original - Low pass (blur)
                // Add 128 to center the result around middle gray
                int highPass = 128 + (pInput[c] - pBlur[c]);
                pOutput[c] = ClampToByte(highPass);
            }

            // Preserve alpha channel if it exists
            if (Channels == 4) {
                pOutput[3] = pInput[3];
            }

            pInput += Channels;
            pBlur += Channels;
            pOutput += Channels;
        }
    }

    free(blurBuffer);

    return OC_STATUS_OK;
}

/* Skeletonize helpers: 3x3 neighborhood around (x,y). */
static int count_01_transitions(const unsigned char* grid, int w, int h, int x, int y) {
    int topLeft   = (y > 0 && x > 0) ? (grid[(y - 1) * w + (x - 1)] ? 1 : 0) : 0;
    int top       = (y > 0) ? (grid[(y - 1) * w + x] ? 1 : 0) : 0;
    int topRight  = (y > 0 && x < w - 1) ? (grid[(y - 1) * w + (x + 1)] ? 1 : 0) : 0;
    int right     = (x < w - 1) ? (grid[y * w + (x + 1)] ? 1 : 0) : 0;
    int bottomRight = (y < h - 1 && x < w - 1) ? (grid[(y + 1) * w + (x + 1)] ? 1 : 0) : 0;
    int bottom    = (y < h - 1) ? (grid[(y + 1) * w + x] ? 1 : 0) : 0;
    int bottomLeft = (y < h - 1 && x > 0) ? (grid[(y + 1) * w + (x - 1)] ? 1 : 0) : 0;
    int left      = (x > 0) ? (grid[y * w + (x - 1)] ? 1 : 0) : 0;
    int n = 0;
    if (top == 0 && topRight == 1) n++;
    if (topRight == 0 && right == 1) n++;
    if (right == 0 && bottomRight == 1) n++;
    if (bottomRight == 0 && bottom == 1) n++;
    if (bottom == 0 && bottomLeft == 1) n++;
    if (bottomLeft == 0 && left == 1) n++;
    if (left == 0 && topLeft == 1) n++;
    if (topLeft == 0 && top == 1) n++;
    return n;
}

static int count_foreground_neighbors(const unsigned char* grid, int w, int h, int x, int y) {
    int topLeft   = (y > 0 && x > 0) ? (grid[(y - 1) * w + (x - 1)] ? 1 : 0) : 0;
    int top       = (y > 0) ? (grid[(y - 1) * w + x] ? 1 : 0) : 0;
    int topRight  = (y > 0 && x < w - 1) ? (grid[(y - 1) * w + (x + 1)] ? 1 : 0) : 0;
    int right     = (x < w - 1) ? (grid[y * w + (x + 1)] ? 1 : 0) : 0;
    int bottomRight = (y < h - 1 && x < w - 1) ? (grid[(y + 1) * w + (x + 1)] ? 1 : 0) : 0;
    int bottom    = (y < h - 1) ? (grid[(y + 1) * w + x] ? 1 : 0) : 0;
    int bottomLeft = (y < h - 1 && x > 0) ? (grid[(y + 1) * w + (x - 1)] ? 1 : 0) : 0;
    int left      = (x > 0) ? (grid[y * w + (x - 1)] ? 1 : 0) : 0;
    return topLeft + top + topRight + right + bottomRight + bottom + bottomLeft + left;
}

static void get_neighbor_bits(const unsigned char* grid, int w, int h, int x, int y,
    int* topLeft, int* top, int* topRight, int* right, int* bottomRight, int* bottom, int* bottomLeft, int* left) {
    *topLeft     = (y > 0 && x > 0) ? (grid[(y - 1) * w + (x - 1)] ? 1 : 0) : 0;
    *top         = (y > 0) ? (grid[(y - 1) * w + x] ? 1 : 0) : 0;
    *topRight    = (y > 0 && x < w - 1) ? (grid[(y - 1) * w + (x + 1)] ? 1 : 0) : 0;
    *right       = (x < w - 1) ? (grid[y * w + (x + 1)] ? 1 : 0) : 0;
    *bottomRight = (y < h - 1 && x < w - 1) ? (grid[(y + 1) * w + (x + 1)] ? 1 : 0) : 0;
    *bottom      = (y < h - 1) ? (grid[(y + 1) * w + x] ? 1 : 0) : 0;
    *bottomLeft  = (y < h - 1 && x > 0) ? (grid[(y + 1) * w + (x - 1)] ? 1 : 0) : 0;
    *left        = (x > 0) ? (grid[y * w + (x - 1)] ? 1 : 0) : 0;
}

/* Thinning pass 1: delete pixel if connectivity and neighbor counts allow (YSC-WHH rules). */
static int thin_should_delete_pass1(int numTransitions, int numNeighbors,
    int topLeft, int top, int topRight, int right, int bottomRight, int bottom, int bottomLeft, int left) {
    if (numNeighbors < 2 || numNeighbors > 7) return 0;
    if (numTransitions == 1)
        return (top * right * bottom == 0) && (right * bottom * left == 0);
    if (numTransitions == 2)
        return ((top && right) && !(bottom || bottomLeft || left)) || ((right && bottom) && !(top || left || topLeft));
    return 0;
}

/* Thinning pass 2: same idea, different neighbor triplets. */
static int thin_should_delete_pass2(int numTransitions, int numNeighbors,
    int topLeft, int top, int topRight, int right, int bottomRight, int bottom, int bottomLeft, int left) {
    if (numNeighbors < 2 || numNeighbors > 7) return 0;
    if (numTransitions == 1)
        return (top * right * left == 0) && (top * bottom * left == 0);
    if (numTransitions == 2)
        return ((top && left) && !(right || bottomRight || bottom)) || ((bottom && left) && !(top || topRight || right));
    return 0;
}

OC_STATUS ocularSkeletonizeFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Threshold) {

    if (Input == NULL || Output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    if (Width <= 0 || Height <= 0 || Stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    int Channels = Stride / Width;
    if (Channels != 1 && Channels != 3 && Channels != 4) {
        return OC_STATUS_ERR_NOTSUPPORTED;
    }

    /* pixels with value < threshold become foreground. */
    Threshold = ClampToByte(Threshold);

    size_t n = (size_t)Width * Height;
    unsigned char* bin = (unsigned char*)malloc(n);
    if (bin == NULL) {
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

    if (Channels == 1) {
        for (int i = 0; i < (int)n; i++) {
            bin[i] = (Input[i] < Threshold) ? 255 : 0;
        }
    } else {
        for (int y = 0; y < Height; y++) {
            const unsigned char* row = Input + (y * Stride);
            for (int x = 0; x < Width; x++) {
                int r = row[x * Channels + 0];
                int g = row[x * Channels + 1];
                int b = row[x * Channels + 2];
                int lum = (int)(0.299f * r + 0.587f * g + 0.114f * b + 0.5f);
                bin[y * Width + x] = (lum < Threshold) ? 255 : 0;
            }
        }
    }

    /* Working copy for iterative thinning (algorithm uses 0/1; we use 0/255) */
    unsigned char* work = (unsigned char*)malloc(n);
    if (work == NULL) {
        free(bin);
        return OC_STATUS_ERR_OUTOFMEMORY;
    }
    for (size_t i = 0; i < n; i++) {
        work[i] = bin[i];
    }

    /* Zhang-Suen: two subiterations per round; repeat until no pixels removed.
     * Each subiteration: copy work to bin first, then mark deletions in bin (parallel update). */
    for (;;) {
        int changed = 0;

        /* Subiteration 1 */
        for (size_t i = 0; i < n; i++) bin[i] = work[i];
        for (int y = 1; y < Height - 1; y++) {
            for (int x = 1; x < Width - 1; x++) {
                int idx = y * Width + x;
                if (work[idx] == 0) continue;

                int numNeighbors = count_foreground_neighbors(work, Width, Height, x, y);
                int numTransitions = count_01_transitions(work, Width, Height, x, y);
                int topLeft, top, topRight, right, bottomRight, bottom, bottomLeft, left;
                get_neighbor_bits(work, Width, Height, x, y, &topLeft, &top, &topRight, &right, &bottomRight, &bottom, &bottomLeft, &left);

                if (!thin_should_delete_pass1(numTransitions, numNeighbors, topLeft, top, topRight, right, bottomRight, bottom, bottomLeft, left)) continue;

                bin[idx] = 0;
                changed = 1;
            }
        }
        for (size_t i = 0; i < n; i++) work[i] = bin[i];

        /* Subiteration 2 */
        for (size_t i = 0; i < n; i++) bin[i] = work[i];
        for (int y = 1; y < Height - 1; y++) {
            for (int x = 1; x < Width - 1; x++) {
                int idx = y * Width + x;
                if (work[idx] == 0) continue;

                int numNeighbors = count_foreground_neighbors(work, Width, Height, x, y);
                int numTransitions = count_01_transitions(work, Width, Height, x, y);
                int topLeft, top, topRight, right, bottomRight, bottom, bottomLeft, left;
                get_neighbor_bits(work, Width, Height, x, y, &topLeft, &top, &topRight, &right, &bottomRight, &bottom, &bottomLeft, &left);

                if (!thin_should_delete_pass2(numTransitions, numNeighbors, topLeft, top, topRight, right, bottomRight, bottom, bottomLeft, left)) continue;

                bin[idx] = 0;
                changed = 1;
            }
        }
        for (size_t i = 0; i < n; i++) work[i] = bin[i];

        if (!changed) break;
    }

    free(work);

    /* Write output: skeleton black on white background */
    if (Channels == 1) {
        for (size_t i = 0; i < n; i++) {
            Output[i] = bin[i] ? 0 : 255;
        }
    } else {
        for (int y = 0; y < Height; y++) {
            unsigned char* outRow = Output + (y * Stride);
            const unsigned char* binRow = bin + (y * Width);
            for (int x = 0; x < Width; x++) {
                unsigned char v = binRow[x] ? 0 : 255;
                outRow[x * Channels + 0] = v;
                outRow[x * Channels + 1] = v;
                outRow[x * Channels + 2] = v;
                if (Channels == 4) outRow[x * Channels + 3] = 255;
            }
        }
    }

    free(bin);
    return OC_STATUS_OK;
}

#ifdef __cplusplus
}
#endif
