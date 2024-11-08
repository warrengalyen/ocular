#include "quanitize.h"
#include "util.h"
#include <math.h>
#include <stdlib.h>


/* --------------------------------------------------------------------------------------------*/
/* ------------------------------ Median Cut quantization -------------------------------------*/
/* --------------------------------------------------------------------------------------------*/

// Calculate the range of colors along each axis
static void calculateColorRange(OcColor* colors, int numColors, int* maxRange, int* splitAxis) {
    int minR = 255, maxR = 0;
    int minG = 255, maxG = 0;
    int minB = 255, maxB = 0;

    for (int i = 0; i < numColors; i++) {
        // Red channel
        if (colors[i].R < minR)
            minR = colors[i].R;
        if (colors[i].R > maxR)
            maxR = colors[i].R;
        // Green channel
        if (colors[i].G < minG)
            minG = colors[i].G;
        if (colors[i].G > maxG)
            maxG = colors[i].G;
        // Blue channel
        if (colors[i].B < minB)
            minB = colors[i].B;
        if (colors[i].B > maxB)
            maxB = colors[i].B;
    }

    int rangeR = maxR - minR;
    int rangeG = maxG - minG;
    int rangeB = maxB - minB;

    // Find axis with maximum range
    *maxRange = rangeR;
    *splitAxis = 0;
    if (rangeG > *maxRange) {
        *maxRange = rangeG;
        *splitAxis = 1;
    }
    if (rangeB > *maxRange) {
        *maxRange = rangeB;
        *splitAxis = 2;
    }
}

// Calculate average color in a bucket
static OcPaletteColor calculateAverageColor(OcColor* colors, int numColors) {
    long sumR = 0, sumG = 0, sumB = 0;

    for (int i = 0; i < numColors; i++) {
        sumR += colors[i].R;
        sumG += colors[i].G;
        sumB += colors[i].B;
    }

    OcPaletteColor avg = {
        .r = (unsigned char)(sumR / numColors),
        .g = (unsigned char)(sumG / numColors),
        .b = (unsigned char)(sumB / numColors),
        .name = "" // Empty name for generated colors
    };

    return avg;
}

// Improved color distance calculation using weighted components (median cut algorithm)
static inline double colorDistanceWeighted(OcColor* c1, OcColor* c2) {

    // Using perceptual weights: R: 0.299, G: 0.587, B: 0.114
    float dr = (c1->R - c2->R) * 0.299;
    float dg = (c1->G - c2->G) * 0.587;
    float db = (c1->B - c2->B) * 0.114;
    return dr * dr + dg * dg + db * db;
}

// Calculate variance for each channel in a color box
static void calculateVariance(ColorBox* box) {
    double sumR = 0, sumG = 0, sumB = 0;
    double sumR2 = 0, sumG2 = 0, sumB2 = 0;
    int totalCount = 0;

    for (int i = 0; i < box->numColors; i++) {
        double count = (double)box->colors[i].count;
        totalCount += box->colors[i].count;

        sumR += box->colors[i].r * count;
        sumG += box->colors[i].g * count;
        sumB += box->colors[i].b * count;

        sumR2 += box->colors[i].r * box->colors[i].r * count;
        sumG2 += box->colors[i].g * box->colors[i].g * count;
        sumB2 += box->colors[i].b * box->colors[i].b * count;
    }

    double invTotal = 1.0 / totalCount;
    double meanR = sumR * invTotal;
    double meanG = sumG * invTotal;
    double meanB = sumB * invTotal;

    box->variance[0] = (sumR2 * invTotal) - (meanR * meanR);
    box->variance[1] = (sumG2 * invTotal) - (meanG * meanG);
    box->variance[2] = (sumB2 * invTotal) - (meanB * meanB);
}

// Find the dominant axis based on weighted variance
static int findDominantAxis(ColorBox* box) {
    // Weight the variances by perceptual importance
    double varR = box->variance[0] * 0.299;
    double varG = box->variance[1] * 0.587;
    double varB = box->variance[2] * 0.114;

    if (varR >= varG && varR >= varB)
        return 0;
    if (varG >= varR && varG >= varB)
        return 1;
    return 2;
}

// Comparison functions for sorting (median cut algorithm)
static int compareR(const void* a, const void* b) { return ((ColorCount*)a)->r - ((ColorCount*)b)->r; }

static int compareG(const void* a, const void* b) { return ((ColorCount*)a)->g - ((ColorCount*)b)->g; }

static int compareB(const void* a, const void* b) { return ((ColorCount*)a)->b - ((ColorCount*)b)->b; }

bool generateOptimalPaletteMedianCut(unsigned char* image, int width, int height, int channels, int maxColors,
                                     OcPalette* palette) {

    if (image == NULL || palette == NULL) {
        return false;
    }
    maxColors = clamp(maxColors, 1, 256);
    

    // Initialize palette
    palette->num_colors = 0;
    palette->capacity = maxColors;
    palette->colors = malloc(maxColors * sizeof(OcPaletteColor));
    strncpy(palette->name, "Generated MedianCut Palette", 255);

    if (!palette->colors) {
        return false;
    }

    // Step 1: Count unique colors and their frequencies
    ColorCount* uniqueColors = NULL;
    int uniqueColorCount = 0;
    int colorCapacity = 4096; // Initial capacity

    uniqueColors = malloc(colorCapacity * sizeof(ColorCount));
    if (!uniqueColors) {
        free(palette->colors);
        return false;
    }

    // Hash table for quick color lookup
    int* colorHash = calloc(1 << 24, sizeof(int));
    if (!colorHash) {
        free(uniqueColors);
        free(palette->colors);
        return false;
    }

    // Count unique colors
    for (int i = 0; i < width * height; i++) {
        int idx = i * channels;
        unsigned char r = image[idx];
        unsigned char g = image[idx + 1];
        unsigned char b = image[idx + 2];
        int hash = (r << 16) | (g << 8) | b;

        if (colorHash[hash] == 0) {
            if (uniqueColorCount >= colorCapacity) {
                colorCapacity *= 2;
                ColorCount* newColors = realloc(uniqueColors, colorCapacity * sizeof(ColorCount));
                if (!newColors) {
                    free(colorHash);
                    free(uniqueColors);
                    free(palette->colors);
                    return false;
                }
                uniqueColors = newColors;
            }

            uniqueColors[uniqueColorCount].r = r;
            uniqueColors[uniqueColorCount].g = g;
            uniqueColors[uniqueColorCount].b = b;
            uniqueColors[uniqueColorCount].count = 1;
            colorHash[hash] = uniqueColorCount + 1;
            uniqueColorCount++;
        } else {
            uniqueColors[colorHash[hash] - 1].count++;
        }
    }

    free(colorHash);

    // If we have fewer unique colors than requested, just use those
    if (uniqueColorCount <= maxColors) {
        for (int i = 0; i < uniqueColorCount; i++) {
            palette->colors[i].r = uniqueColors[i].r;
            palette->colors[i].g = uniqueColors[i].g;
            palette->colors[i].b = uniqueColors[i].b;
            palette->colors[i].name[0] = '\0';
            palette->num_colors++;
        }
        free(uniqueColors);
        return true;
    }

    // Step 2: Create initial box containing all colors
    ColorBox* boxes = malloc(maxColors * sizeof(ColorBox));
    if (!boxes) {
        free(uniqueColors);
        free(palette->colors);
        return false;
    }

    boxes[0].colors = uniqueColors;
    boxes[0].numColors = uniqueColorCount;
    boxes[0].capacity = colorCapacity;
    int numBoxes = 1;

    // Step 3: Split boxes until we have enough colors
    while (numBoxes < maxColors) {
        // Find box with largest variance
        int boxToSplit = 0;
        double maxVariance = 0;

        for (int i = 0; i < numBoxes; i++) {
            calculateVariance(&boxes[i]);
            int axis = findDominantAxis(&boxes[i]);
            if (boxes[i].variance[axis] > maxVariance) {
                maxVariance = boxes[i].variance[axis];
                boxToSplit = i;
            }
        }

        if (maxVariance == 0)
            break; // No more meaningful splits possible

        // Sort colors along dominant axis
        int axis = findDominantAxis(&boxes[boxToSplit]);
        switch (axis) {
            case 0: qsort(boxes[boxToSplit].colors, boxes[boxToSplit].numColors, sizeof(ColorCount), compareR); break;
            case 1: qsort(boxes[boxToSplit].colors, boxes[boxToSplit].numColors, sizeof(ColorCount), compareG); break;
            case 2: qsort(boxes[boxToSplit].colors, boxes[boxToSplit].numColors, sizeof(ColorCount), compareB); break;
        }

        // Find split point that divides total pixel count most evenly
        int totalCount = 0;
        for (int i = 0; i < boxes[boxToSplit].numColors; i++) {
            totalCount += boxes[boxToSplit].colors[i].count;
        }

        int halfCount = totalCount / 2;
        int currentCount = 0;
        int splitPoint = 0;

        for (int i = 0; i < boxes[boxToSplit].numColors; i++) {
            currentCount += boxes[boxToSplit].colors[i].count;
            if (currentCount >= halfCount) {
                splitPoint = i + 1;
                break;
            }
        }

        // Create new box
        boxes[numBoxes].colors = boxes[boxToSplit].colors + splitPoint;
        boxes[numBoxes].numColors = boxes[boxToSplit].numColors - splitPoint;
        boxes[boxToSplit].numColors = splitPoint;
        numBoxes++;
    }

    // Step 4: Calculate final palette colors using weighted averages
    for (int i = 0; i < numBoxes; i++) {
        int totalCount = 0;
        double sumR = 0, sumG = 0, sumB = 0;

        for (int j = 0; j < boxes[i].numColors; j++) {
            int count = boxes[i].colors[j].count;
            totalCount += count;
            sumR += boxes[i].colors[j].r * count;
            sumG += boxes[i].colors[j].g * count;
            sumB += boxes[i].colors[j].b * count;
        }

        palette->colors[i].r = (unsigned char)(sumR / totalCount + 0.5);
        palette->colors[i].g = (unsigned char)(sumG / totalCount + 0.5);
        palette->colors[i].b = (unsigned char)(sumB / totalCount + 0.5);
        palette->colors[i].name[0] = '\0';
        palette->num_colors++;
    }

    // Clean up
    free(boxes);
    free(uniqueColors);

    return true;
}

/* --------------------------------------------------------------------------------------------*/
/* ------------------------------ Median Cut quantization -------------------------------------*/
/* --------------------------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------------------------*/
/* ------------------------------ Octree quantization -----------------------------------------*/
/* --------------------------------------------------------------------------------------------*/

// Create a new octree node
static OctreeNode* createNode(int level) {
    OctreeNode* node = (OctreeNode*)calloc(1, sizeof(OctreeNode));
    if (!node)
        return NULL;
    node->isLeaf = (level == 7) ? 1 : 0;
    return node;
}

static void freeNode(OctreeNode* node) {
    if (!node)
        return;
    for (int i = 0; i < 8; i++) {
        if (node->children[i]) {
            freeNode(node->children[i]);
            node->children[i] = NULL;
        }
    }
    free(node);
}

// Get appropriate child index for a color at a given level. Add weight factors for better color perception
static int getColorIndex(unsigned char r, unsigned char g, unsigned char b, int level) {
    // Apply perceptual weights (human eye is more sensitive to green)
    double weightedR = r * 0.299;
    double weightedG = g * 0.587;
    double weightedB = b * 0.114;

    int shift = 7 - level;
    int mask = 1 << shift;
    return ((((int)weightedR) & mask) >> shift) << 2 | ((((int)weightedG) & mask) >> shift) << 1 |
           ((((int)weightedB) & mask) >> shift);
}

// Add color to octree
static void addColor(Octree* tree, OctreeNode* node, unsigned char r, unsigned char g, unsigned char b, int level) {
    if (!node)
        return;

    // Accumulate color at each level for better averaging
    node->pixelCount++;
    node->redSum += r;
    node->greenSum += g;
    node->blueSum += b;

    if (level >= 7 || node->isLeaf) {
        node->isLeaf = 1;
        tree->numLeaves += (node->pixelCount == 1) ? 1 : 0;
        return;
    }

    int index = getColorIndex(r, g, b, level);
    if (!node->children[index]) {
        node->children[index] = createNode(level + 1);
        if (!node->children[index])
            return;

        if (level < 7) {
            node->children[index]->next = tree->reducibleNodes[level];
            tree->reducibleNodes[level] = node->children[index];
        }
    }

    addColor(tree, node->children[index], r, g, b, level + 1);
}

// Reduce octree by combining leaves
static void reduceTree(Octree* tree) {
    // Find deepest level with reducible nodes
    int level;
    for (level = 6; level >= 0; level--) {
        if (tree->reducibleNodes[level])
            break;
    }

    if (level < 0)
        return;

    OctreeNode* node = tree->reducibleNodes[level];
    tree->reducibleNodes[level] = node->next;

    // Ensure we're not losing color fidelity
    long totalPixels = 0;
    for (int i = 0; i < 8; i++) {
        if (node->children[i]) {
            totalPixels += node->children[i]->pixelCount;
        }
    }

    if (totalPixels > 0) {
        node->redSum = 0;
        node->greenSum = 0;
        node->blueSum = 0;
        node->pixelCount = 0;

        for (int i = 0; i < 8; i++) {
            if (node->children[i]) {
                node->redSum += node->children[i]->redSum;
                node->greenSum += node->children[i]->greenSum;
                node->blueSum += node->children[i]->blueSum;
                node->pixelCount += node->children[i]->pixelCount;
                freeNode(node->children[i]);
                node->children[i] = NULL;
                tree->numLeaves--;
            }
        }

        node->isLeaf = 1;
        tree->numLeaves++;
    }
}



bool generateOptimalPaletteOctree(unsigned char* image, int width, int height, int channels, int maxColors, OcPalette* palette) {
    if (!image || !palette || maxColors <= 0) {
        return false;
    }

    // Initialize palette
    palette->num_colors = 0;
    palette->capacity = maxColors;
    palette->colors = malloc(maxColors * sizeof(OcPaletteColor));
    strncpy(palette->name, "Generated Octree Palette", 255);

    if (!palette->colors) {
        return false;
    }

    // Initialize octree
    Octree tree = { 0 };
    tree.root = createNode(0);
    if (!tree.root) {
        free(palette->colors);
        return false;
    }
    tree.maxColors = maxColors;
    tree.numLeaves = 0;
    memset(tree.reducibleNodes, 0, sizeof(tree.reducibleNodes));

    // Add colors to octree
    for (int i = 0; i < width * height; i++) {
        int idx = i * channels;
        if (!tree.root) {
            free(palette->colors);
            return false;
        }

        addColor(&tree, tree.root, image[idx], image[idx + 1], image[idx + 2], 0);

        // Reduce tree if we have too many leaves
        while (tree.numLeaves > maxColors && tree.numLeaves > 1) {
            reduceTree(&tree);
        }
    }

    // Extract palette colors from leaves using a stack-based approach
    typedef struct {
        OctreeNode* node;
        int visited;
    } StackItem;

    StackItem* stack = malloc(sizeof(StackItem) * 512); // Adjust size if needed
    if (!stack) {
        freeNode(tree.root);
        free(palette->colors);
        return false;
    }

    int stackTop = 0;
    int paletteIndex = 0;

    // Initialize stack with root
    stack[stackTop].node = tree.root;
    stack[stackTop].visited = 0;
    stackTop++;

    // Iterative traversal
    while (stackTop > 0 && paletteIndex < maxColors) {
        stackTop--;
        OctreeNode* current = stack[stackTop].node;

        if (!current)
            continue;

        if (current->isLeaf && current->pixelCount > 0) {
            // Add color to palette
            palette->colors[paletteIndex].r = (unsigned char)(current->redSum / current->pixelCount);
            palette->colors[paletteIndex].g = (unsigned char)(current->greenSum / current->pixelCount);
            palette->colors[paletteIndex].b = (unsigned char)(current->blueSum / current->pixelCount);
            palette->colors[paletteIndex].name[0] = '\0';
            paletteIndex++;
            palette->num_colors++;
        } else {
            // Add children to stack
            for (int i = 0; i < 8; i++) {
                if (current->children[i]) {
                    stack[stackTop].node = current->children[i];
                    stack[stackTop].visited = 0;
                    stackTop++;

                    if (stackTop >= 512) {
                        free(stack);
                        freeNode(tree.root);
                        free(palette->colors);
                        return false;
                    }
                }
            }
        }
    }

    // Clean up
    free(stack);
    freeNode(tree.root);

    // Ensure we have at least one color
    if (palette->num_colors == 0) {
        palette->colors[0].r = 0;
        palette->colors[0].g = 0;
        palette->colors[0].b = 0;
        palette->num_colors = 1;
    }

    return true;
}

/* --------------------------------------------------------------------------------------------*/
/* ------------------------------ Octree quantization -----------------------------------------*/
/* --------------------------------------------------------------------------------------------*/