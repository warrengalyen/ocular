/**
 * @file: pixelate_filters.c
 * @author Warren Galyen
 * Created: 10-3-2025
 * Last Updated: 10-4-2025
 * Last update: added crystallize filter
 *
 * @brief Implementation of pixelation and artistic filters
 */

#include "pixelate_filters.h"
#include "core.h"
#include "util.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <float.h>

OC_STATUS ocularMosaicFilter(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int blockSize) {

    if (Input == NULL || Output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    if (Width <= 0 || Height <= 0 || Stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    int channels = Stride / Width;
    if (channels != 1 && channels != 3) {
        return OC_STATUS_ERR_NOTSUPPORTED;
    }

    // Ensure filter specific parameters are within valid ranges
    blockSize = max(blockSize, 1);

    int pPos;
    for (int y = 0; y < Height; y += blockSize) {
        for (int x = 0; x < Width; x += blockSize) {

            // Find the average color value of the pixels in a block
            int numPix = 0;
            int avg[3];
            int blockAvg[3];
            memset(avg, 0, sizeof avg);
            for (int dy = 0; dy < blockSize; dy++) {
                for (int dx = 0; dx < blockSize; dx++) {
                    int xOffset = x + dx;
                    int yOffset = y + dy;

                    // If possible, add the pixel value to the average
                    if (yOffset < Height && xOffset < Width) {
                        pPos = xOffset * channels + yOffset * Stride;
                        for (int c = 0; c < channels; c++) {
                            avg[c] += Input[pPos + c];
                        }
                        numPix++;
                    }
                }
            }
            for (int c = 0; c < channels; c++) {
                blockAvg[c] = avg[c] / numPix;
            }

            for (int dy = 0; dy < blockSize; dy++) {
                for (int dx = 0; dx < blockSize; dx++) {
                    int xOffset = x + dx;
                    int yOffset = y + dy;

                    // The pixel is the value of the upper left pixel in the block
                    if (yOffset < Height - 1 && xOffset < Width - 1) {
                        pPos = xOffset * channels + yOffset * Stride;
                        for (int c = 0; c < channels; c++) {
                            Output[pPos + c] = blockAvg[c];
                        }
                    }
                }
            }
        }
    }

    return OC_STATUS_OK;
}

// Simple random number generator for pointillize (LCG)
static unsigned int pointillize_rand_state = 1;

static void pointillize_srand(unsigned int seed) {
    pointillize_rand_state = seed;
}

static int pointillize_rand(void) {
    pointillize_rand_state = pointillize_rand_state * 1103515245 + 12345;
    return (unsigned int)(pointillize_rand_state / 65536) % 32768;
}

static int pointillize_rand_range(int min, int max) {
    if (min >= max) return min;
    return min + (pointillize_rand() % (max - min + 1));
}

// Helper function to clamp values
static inline float clampf(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// Helper function to draw a smooth anti-aliased filled circle
static void drawFilledCircle(unsigned char* output, int width, int height, int stride,
                             int centerX, int centerY, int radius,
                             unsigned char r, unsigned char g, unsigned char b) {
    int channels = stride / width;
    float radiusF = (float)radius;
    
    // Calculate bounding box for the circle (add 1 pixel for anti-aliasing)
    int minX = centerX - radius - 1;
    int maxX = centerX + radius + 1;
    int minY = centerY - radius - 1;
    int maxY = centerY + radius + 1;
    
    // Clamp to image boundaries
    if (minX < 0) minX = 0;
    if (maxX >= width) maxX = width - 1;
    if (minY < 0) minY = 0;
    if (maxY >= height) maxY = height - 1;
    
    // Draw the anti-aliased circle
    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            // Calculate distance from pixel center to circle center
            float dx = x - centerX + 0.5f;
            float dy = y - centerY + 0.5f;
            float distance = sqrtf(dx * dx + dy * dy);
            
            // Calculate alpha based on distance (for anti-aliasing)
            float alpha = 0.0f;
            if (distance <= radiusF - 0.5f) {
                // Fully inside
                alpha = 1.0f;
            } else if (distance < radiusF + 0.5f) {
                // Edge pixels - blend for smooth edges
                alpha = radiusF + 0.5f - distance;
                if (alpha < 0.0f) alpha = 0.0f;
                if (alpha > 1.0f) alpha = 1.0f;
            }
            
            // Apply the color with alpha blending
            if (alpha > 0.0f) {
                int idx = (y * width + x) * channels;
                
                // Get background color (white)
                unsigned char bgR = output[idx + 0];
                unsigned char bgG = output[idx + 1];
                unsigned char bgB = output[idx + 2];
                
                // Alpha blend: result = foreground * alpha + background * (1 - alpha)
                output[idx + 0] = (unsigned char)(r * alpha + bgR * (1.0f - alpha));
                output[idx + 1] = (unsigned char)(g * alpha + bgG * (1.0f - alpha));
                output[idx + 2] = (unsigned char)(b * alpha + bgB * (1.0f - alpha));
                
                if (channels == 4) {
                    output[idx + 3] = 255; // Full opacity
                }
            }
        }
    }
}

// Helper function to sample color from where the dot will be placed
// This ensures the dot color matches the actual image content at that position
static void sampleColorAtPosition(unsigned char* input, int width, int height, int stride,
                                 int posX, int posY,
                                 unsigned char* sampledR, unsigned char* sampledG, unsigned char* sampledB) {
    int channels = stride / width;
    
    // Clamp position to image boundaries
    if (posX < 0) posX = 0;
    if (posX >= width) posX = width - 1;
    if (posY < 0) posY = 0;
    if (posY >= height) posY = height - 1;
    
    // Sample the pixel at this exact position
    int idx = (posY * width + posX) * channels;
    *sampledR = input[idx + 0];
    *sampledG = input[idx + 1];
    *sampledB = input[idx + 2];
}

OC_STATUS ocularPointillizeFilter(unsigned char* input, unsigned char* output,
                                  int width, int height, int stride,
                                  int cellSize,
                                  unsigned char bgR, unsigned char bgG, unsigned char bgB) {
    // Validate inputs
    if (input == NULL || output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    
    if (width <= 0 || height <= 0 || stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }
    
    if (input == output) {
        return OC_STATUS_ERR_INVALIDPARAMETER; // Cannot operate in-place
    }
    
    // Validate cell size
    if (cellSize < 3 || cellSize > 100) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }
    
    int channels = stride / width;
    
    // Fill output with specified background color
    for (int i = 0; i < height * stride; i += channels) {
        output[i + 0] = bgR;
        output[i + 1] = bgG;
        output[i + 2] = bgB;
        if (channels == 4) {
            output[i + 3] = 255;
        }
    }
    
    // Calculate number of cells
    int cellsX = (width + cellSize - 1) / cellSize;
    int cellsY = (height + cellSize - 1) / cellSize;
    
    // Calculate dot radius - make dots larger than half cell size for better coverage
    // Using 0.65 * cellSize gives good overlap and minimizes white background
    // (Photoshop uses approximately this ratio for dense dot coverage)
    int dotRadius = (int)(cellSize * 0.65f);
    if (dotRadius < 2) dotRadius = 2;
    
    // Initialize random number generator with time-based seed for randomness
    pointillize_srand((unsigned int)time(NULL));
    
    // Process each cell
    for (int cellY = 0; cellY < cellsY; cellY++) {
        for (int cellX = 0; cellX < cellsX; cellX++) {
            // Calculate cell boundaries
            int cellStartX = cellX * cellSize;
            int cellStartY = cellY * cellSize;
            int cellEndX = cellStartX + cellSize;
            int cellEndY = cellStartY + cellSize;
            
            // Clamp to image boundaries
            if (cellEndX > width) cellEndX = width;
            if (cellEndY > height) cellEndY = height;
            
            // Place dot at random position within the cell (pointillist style)
            int dotX = pointillize_rand_range(cellStartX, cellEndX - 1);
            int dotY = pointillize_rand_range(cellStartY, cellEndY - 1);
            
            // Sample the color from where the dot will actually be placed
            // This gives the most accurate color representation
            unsigned char dotR, dotG, dotB;
            sampleColorAtPosition(input, width, height, stride,
                                dotX, dotY,
                                &dotR, &dotG, &dotB);
            
            // Draw the dot at the random position with the sampled color
            drawFilledCircle(output, width, height, stride,
                           dotX, dotY, dotRadius,
                           dotR, dotG, dotB);
        }
    }
    
    return OC_STATUS_OK;
}

// Helper function for basic antialiasing between two values
static inline float basicAA(float a, float b, float x) {
    if (x < a) return 0.0f;
    if (x >= b) return 1.0f;
    return (x - a) / (b - a);
}

// Helper to sample a channel value with clamping
static inline unsigned char sampleChannel(const unsigned char* input, int width, int height, 
                                         int stride, int x, int y, int channel) {
    if (x < 0) x = 0;
    if (x >= width) x = width - 1;
    if (y < 0) y = 0;
    if (y >= height) y = height - 1;
    
    int channels = stride / width;
    return input[(y * width + x) * channels + channel];
}

OC_STATUS ocularColorHalftoneFilter(unsigned char* input, unsigned char* output,
                                    int width, int height, int stride,
                                    int radius, float dotDensity,
                                    float cyanAngle, float magentaAngle, float yellowAngle) {
    // Validate inputs
    if (input == NULL || output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    
    if (width <= 0 || height <= 0 || stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }
    
    if (input == output) {
        return OC_STATUS_ERR_INVALIDPARAMETER; // Cannot operate in-place
    }
    
    // Validate radius
    if (radius < 4 || radius > 100) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }
    
    // Validate dot density
    if (dotDensity < 0.0f || dotDensity > 100.0f) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }
    
    int channels = stride / width;
    if (channels < 3) {
        return OC_STATUS_ERR_INVALIDPARAMETER; // Need at least RGB
    }
    
    // Start with white background (halftone dots will be drawn as ink on white)
    memset(output, 255, height * stride);
    
    // Grid spacing
    float gridSpacing = (float)radius;
    float halfRadius = gridSpacing * 0.5f;
    
    // Convert density from 0-100 to 0-1 scale
    float densityNormalized = dotDensity * 0.01f;
    
    // Density scalar
    float densityScale = sqrtf(2.0f) * halfRadius * densityNormalized;
    
    // Pre-calculate density lookup table based on CMY conversion
    // (Invert RGB to CMY, square for better luminance control)
    float densityLookup[256];
    for (int i = 0; i < 256; i++) {
        float cmyValue = (float)i / 255.0f;
        cmyValue = 1.0f - (cmyValue * cmyValue); // CMY conversion with squaring
        densityLookup[i] = cmyValue * densityScale;
    }
    
    // Process each channel separately with its own angle
    float angles[3] = {cyanAngle, magentaAngle, yellowAngle};
    
    // Neighboring grid offsets for overlap checking
    int xCheck[4] = {-1, 0, 1, 0};
    int yCheck[4] = {0, -1, 0, 1};
    
    for (int ch = 0; ch < 3; ch++) {
        // Convert angle to radians
        float angleRad = angles[ch] * M_PI / 180.0f;
        float cosTheta = cosf(angleRad);
        float sinTheta = sinf(angleRad);
        
        // Process each pixel
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                // Transform pixel to rotated grid space
                float srcX = (float)x * cosTheta + (float)y * sinTheta;
                float srcY = -(float)x * sinTheta + (float)y * cosTheta;
                
                // Snap to nearest grid point
                float gridX = floorf((srcX - halfRadius) / gridSpacing + 0.5f) * gridSpacing + halfRadius;
                float gridY = floorf((srcY - halfRadius) / gridSpacing + 0.5f) * gridSpacing + halfRadius;
                
                // Transform grid point back to image space
                float dstX = gridX * cosTheta - gridY * sinTheta;
                float dstY = gridX * sinTheta + gridY * cosTheta;
                
                // Sample the channel value at grid point
                unsigned char sampleValue = sampleChannel(input, width, height, stride, 
                                                         (int)dstX, (int)dstY, ch);
                
                // Calculate distance from current pixel to grid center
                float dx = (float)x - dstX;
                float dy = (float)y - dstY;
                float distance = sqrtf(dx * dx + dy * dy) + 1.0f;
                
                // Get dot radius for this intensity
                float dotRadius = densityLookup[sampleValue];
                
                // Apply antialiasing on the dot edge
                float coverage = 1.0f - basicAA(distance - 1.0f, distance, dotRadius);
                
                // Check for overlap with neighboring grid dots if this dot is large enough
                if (distance < halfRadius) {
                    for (int i = 0; i < 4; i++) {
                        // Calculate neighboring grid position
                        float neighborGridX = gridX + xCheck[i] * gridSpacing;
                        float neighborGridY = gridY + yCheck[i] * gridSpacing;
                        
                        // Transform back to image space
                        float neighborDstX = neighborGridX * cosTheta - neighborGridY * sinTheta;
                        float neighborDstY = neighborGridX * sinTheta + neighborGridY * cosTheta;
                        
                        // Sample neighbor
                        unsigned char neighborValue = sampleChannel(input, width, height, stride,
                                                                   (int)neighborDstX, (int)neighborDstY, ch);
                        
                        // Calculate distance to neighbor grid center
                        float ndx = (float)x - neighborDstX;
                        float ndy = (float)y - neighborDstY;
                        float neighborDist = sqrtf(ndx * ndx + ndy * ndy);
                        
                        // Get neighbor dot radius
                        float neighborRadius = densityLookup[neighborValue];
                        
                        // Calculate coverage from neighbor
                        float neighborCoverage = 1.0f - basicAA(neighborDist, neighborDist + 1.0f, neighborRadius);
                        
                        // Keep the minimum coverage (darkest/most ink)
                        if (neighborCoverage < coverage) {
                            coverage = neighborCoverage;
                        }
                    }
                }
                
                // Convert coverage to final color (0 = full ink, 1 = no ink/white)
                int idx = (y * width + x) * channels + ch;
                output[idx] = (unsigned char)(255.0f * coverage);
            }
        }
    }
    
    // Handle alpha channel if present
    if (channels == 4) {
        for (int i = 0; i < width * height; i++) {
            output[i * channels + 3] = 255;
        }
    }
    
    return OC_STATUS_OK;
}

OC_STATUS ocularFragmentFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride) {
    if (Input == NULL || Output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    if (Width <= 0 || Height <= 0 || Stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    int Channels = Stride / Width;

    // Define the four offset positions for the fragment effect
    // These create a multi-exposure look similar to Photoshop's Fragment filter
    // Four diagonal offsets: up-left, up-right, down-left, down-right
    const int offsets[4][2] = {
        { -4, -4 }, // Up-left
        { 4, -4 },  // Up-right
        { -4, 4 },  // Down-left
        { 4, 4 }    // Down-right
    };

    // Create temporary buffer if doing in-place operation
    unsigned char* source = Input;
    unsigned char* tempBuffer = NULL;

    if (Input == Output) {
        tempBuffer = (unsigned char*)malloc(Height * Stride);
        if (tempBuffer == NULL) {
            return OC_STATUS_ERR_OUTOFMEMORY;
        }
        memcpy(tempBuffer, Input, Height * Stride);
        source = tempBuffer;
    }

    // Process each pixel
    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            // Accumulate values from all four offset positions
            int sumR = 0, sumG = 0, sumB = 0, sumA = 0;

            for (int i = 0; i < 4; i++) {
                int offsetX = x + offsets[i][0];
                int offsetY = y + offsets[i][1];

                // Clamp coordinates to image bounds
                offsetX = clamp(offsetX, 0, Width - 1);
                offsetY = clamp(offsetY, 0, Height - 1);

                int srcIdx = (offsetY * Width + offsetX) * Channels;

                // Accumulate channel values
                if (Channels >= 3) {
                    sumR += source[srcIdx + 0];
                    sumG += source[srcIdx + 1];
                    sumB += source[srcIdx + 2];
                } else if (Channels == 1) {
                    sumR += source[srcIdx];
                }

                if (Channels == 4) {
                    sumA += source[srcIdx + 3];
                }
            }

            // Average the accumulated values (divide by 4)
            int dstIdx = (y * Width + x) * Channels;

            if (Channels >= 3) {
                Output[dstIdx + 0] = (unsigned char)(sumR / 4);
                Output[dstIdx + 1] = (unsigned char)(sumG / 4);
                Output[dstIdx + 2] = (unsigned char)(sumB / 4);
            } else if (Channels == 1) {
                Output[dstIdx] = (unsigned char)(sumR / 4);
            }

            if (Channels == 4) {
                Output[dstIdx + 3] = (unsigned char)(sumA / 4);
            }
        }
    }

    // Free temporary buffer if allocated
    if (tempBuffer != NULL) {
        free(tempBuffer);
    }

    return OC_STATUS_OK;
}

// Hash table entry for cell lookup
typedef struct HashEntry {
    int gridX, gridY;
    int sumR, sumG, sumB, sumA;
    int count;
    int cellIndex; // Index in the cells array
    struct HashEntry* next;
} HashEntry;

typedef struct {
    HashEntry** buckets;
    int size;
} HashMap;

typedef struct {
    int gridX, gridY;
    int sumR, sumG, sumB, sumA;
    int count;
} WorleyCell;

// Simple hash function for deterministic random points
static unsigned int hash(unsigned int x, unsigned int y) {
    unsigned int h = (x * 374761393U) + (y * 668265263U);
    h ^= h >> 13;
    h *= 1274126177U;
    h ^= h >> 16;
    return h;
}

// Hash function for grid coordinates
static unsigned int gridHash(int gridX, int gridY, int tableSize) {
    return (unsigned int)((gridX * 73856093) ^ (gridY * 19349663)) % tableSize;
}

// Create hash map
static HashMap* createHashMap(int expectedCells) {
    HashMap* map = (HashMap*)malloc(sizeof(HashMap));
    if (!map)
        return NULL;

    map->size = expectedCells * 2;
    map->buckets = (HashEntry**)calloc(map->size, sizeof(HashEntry*));
    if (!map->buckets) {
        free(map);
        return NULL;
    }
    return map;
}

// Add cell to hash map
static void addToHashMap(HashMap* map, int gridX, int gridY, int cellIndex) {
    unsigned int idx = gridHash(gridX, gridY, map->size);
    HashEntry* entry = (HashEntry*)malloc(sizeof(HashEntry));
    if (entry) {
        entry->gridX = gridX;
        entry->gridY = gridY;
        entry->cellIndex = cellIndex;
        entry->next = map->buckets[idx];
        map->buckets[idx] = entry;
    }
}

// Find cell index in hash map
static int findCellIndex(HashMap* map, int gridX, int gridY) {
    unsigned int idx = gridHash(gridX, gridY, map->size);
    HashEntry* entry = map->buckets[idx];

    while (entry) {
        if (entry->gridX == gridX && entry->gridY == gridY) {
            return entry->cellIndex;
        }
        entry = entry->next;
    }

    return -1;
}

static void freeHashMap(HashMap* map) {
    if (!map)
        return;
    for (int i = 0; i < map->size; i++) {
        HashEntry* entry = map->buckets[i];
        while (entry) {
            HashEntry* next = entry->next;
            free(entry);
            entry = next;
        }
    }
    free(map->buckets);
    free(map);
}

// Generate random point in grid cell
static void getRandomPoint(int gridX, int gridY, float cellSize, float* pointX, float* pointY) {
    unsigned int h = hash(gridX, gridY);
    *pointX = (float)gridX * cellSize + ((float)(h & 0xFFFF) / 65536.0f) * cellSize;
    *pointY = (float)gridY * cellSize + ((float)((h >> 16) & 0xFFFF) / 65536.0f) * cellSize;
}

OC_STATUS ocularCrystallizeFilter(unsigned char* input, unsigned char* output, int width, int height, int stride, int cellSize) {
    // Validate inputs
    if (input == NULL || output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    if (width <= 0 || height <= 0 || stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }
    if (input == output) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    // Clamp cell size to valid range
    cellSize = clamp(cellSize, 3, 100);

    int channels = stride / width;
    if (channels < 1 || channels > 4) {
        return OC_STATUS_ERR_NOTSUPPORTED;
    }

    // Convert to float for Worley noise
    float fCellSize = (float)cellSize;

    // Calculate maximum number of cells needed
    int maxCellsNeeded = ((width / cellSize) + 3) * ((height / cellSize) + 3);
    maxCellsNeeded = (int)(maxCellsNeeded * 1.5f);
    if (maxCellsNeeded < 1000)
        maxCellsNeeded = 1000;

    WorleyCell* cells = (WorleyCell*)calloc(maxCellsNeeded, sizeof(WorleyCell));
    int numActiveCells = 0;

    if (cells == NULL) {
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

    // Create hash map for O(1) lookups
    HashMap* cellMap = createHashMap(maxCellsNeeded);
    if (!cellMap) {
        free(cells);
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

    // First pass: Assign pixels to Worley cells and accumulate colors
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Calculate which grid cell this pixel belongs to
            int gridX = (int)(x / fCellSize);
            int gridY = (int)(y / fCellSize);

            // Find or create cell for this grid position using hash map
            int cellIndex = findCellIndex(cellMap, gridX, gridY);

            // Create new cell if not found
            if (cellIndex == -1 && numActiveCells < maxCellsNeeded) {
                cellIndex = numActiveCells++;
                cells[cellIndex].gridX = gridX;
                cells[cellIndex].gridY = gridY;
                cells[cellIndex].sumR = 0;
                cells[cellIndex].sumG = 0;
                cells[cellIndex].sumB = 0;
                cells[cellIndex].sumA = 0;
                cells[cellIndex].count = 0;
                addToHashMap(cellMap, gridX, gridY, cellIndex);
            }

            // Accumulate color values
            if (cellIndex >= 0) {
                int srcIdx = (y * width + x) * channels;
                cells[cellIndex].count++;

                if (channels >= 3) {
                    cells[cellIndex].sumR += input[srcIdx + 0];
                    cells[cellIndex].sumG += input[srcIdx + 1];
                    cells[cellIndex].sumB += input[srcIdx + 2];
                } else if (channels == 1) {
                    cells[cellIndex].sumR += input[srcIdx];
                }

                if (channels == 4) {
                    cells[cellIndex].sumA += input[srcIdx + 3];
                }
            }
        }
    }

    // Pre-calculate random points
    int maxGridX = (int)(width / fCellSize) + 3;
    int maxGridY = (int)(height / fCellSize) + 3;
    int totalGridCells = maxGridX * maxGridY;
    float* precomputedPointsX = (float*)malloc(totalGridCells * sizeof(float));
    float* precomputedPointsY = (float*)malloc(totalGridCells * sizeof(float));

    if (!precomputedPointsX || !precomputedPointsY) {
        free(precomputedPointsX);
        free(precomputedPointsY);
        freeHashMap(cellMap);
        free(cells);
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

    for (int gy = 0; gy < maxGridY; gy++) {
        for (int gx = 0; gx < maxGridX; gx++) {
            int idx = gy * maxGridX + gx;
            getRandomPoint(gx, gy, fCellSize, &precomputedPointsX[idx], &precomputedPointsY[idx]);
        }
    }

    // Second pass: Generate output
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float fx = (float)x;
            float fy = (float)y;

            // Calculate grid cell coordinates
            int gridX = (int)(fx / fCellSize);
            int gridY = (int)(fy / fCellSize);

            float minDistSq = FLT_MAX;
            float secondMinDistSq = FLT_MAX;
            int nearestGridX = gridX;
            int nearestGridY = gridY;
            int secondNearestGridX = gridX;
            int secondNearestGridY = gridY;

            // Check 3x3 grid around the point for nearest cells
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int checkX = gridX + dx;
                    int checkY = gridY + dy;

                    if (checkX >= 0 && checkX < maxGridX && checkY >= 0 && checkY < maxGridY) {
                        // Use precomputed points
                        int idx = checkY * maxGridX + checkX;
                        float pointX = precomputedPointsX[idx];
                        float pointY = precomputedPointsY[idx];

                        // Calculate squared distance
                        float dx_dist = fx - pointX;
                        float dy_dist = fy - pointY;
                        float distSq = dx_dist * dx_dist + dy_dist * dy_dist;

                        if (distSq < minDistSq) {
                            secondMinDistSq = minDistSq;
                            secondNearestGridX = nearestGridX;
                            secondNearestGridY = nearestGridY;
                            minDistSq = distSq;
                            nearestGridX = checkX;
                            nearestGridY = checkY;
                        } else if (distSq < secondMinDistSq) {
                            secondMinDistSq = distSq;
                            secondNearestGridX = checkX;
                            secondNearestGridY = checkY;
                        }
                    }
                }
            }

            // Use hash map for cell lookups
            int nearestCellIndex = findCellIndex(cellMap, nearestGridX, nearestGridY);
            int secondNearestCellIndex = findCellIndex(cellMap, secondNearestGridX, secondNearestGridY);

            int dstIdx = (y * width + x) * channels;

            // Calculate smooth blend factor
            float blendFactor = 0.0f;
            if (secondMinDistSq > minDistSq && minDistSq > 0.0f) {
                float distRatio = sqrtf(minDistSq) / (sqrtf(minDistSq) + sqrtf(secondMinDistSq));
                if (distRatio > 0.45f && distRatio < 0.55f) {
                    blendFactor = (distRatio - 0.45f) / 0.1f;
                    blendFactor = blendFactor * blendFactor * (3.0f - 2.0f * blendFactor);
                    blendFactor *= 0.5f;
                }
            }

            // Blend between two nearest cells if they're close
            if (blendFactor > 0.0f && nearestCellIndex >= 0 && secondNearestCellIndex >= 0 && cells[nearestCellIndex].count > 0 &&
                cells[secondNearestCellIndex].count > 0) {

                float invBlendFactor = 1.0f - blendFactor;

                if (channels >= 3) {
                    float avgR1 = (float)(cells[nearestCellIndex].sumR / cells[nearestCellIndex].count);
                    float avgG1 = (float)(cells[nearestCellIndex].sumG / cells[nearestCellIndex].count);
                    float avgB1 = (float)(cells[nearestCellIndex].sumB / cells[nearestCellIndex].count);

                    float avgR2 = (float)(cells[secondNearestCellIndex].sumR / cells[secondNearestCellIndex].count);
                    float avgG2 = (float)(cells[secondNearestCellIndex].sumG / cells[secondNearestCellIndex].count);
                    float avgB2 = (float)(cells[secondNearestCellIndex].sumB / cells[secondNearestCellIndex].count);

                    output[dstIdx + 0] = (unsigned char)(avgR1 * invBlendFactor + avgR2 * blendFactor);
                    output[dstIdx + 1] = (unsigned char)(avgG1 * invBlendFactor + avgG2 * blendFactor);
                    output[dstIdx + 2] = (unsigned char)(avgB1 * invBlendFactor + avgB2 * blendFactor);
                } else if (channels == 1) {
                    float avg1 = (float)(cells[nearestCellIndex].sumR / cells[nearestCellIndex].count);
                    float avg2 = (float)(cells[secondNearestCellIndex].sumR / cells[secondNearestCellIndex].count);
                    output[dstIdx] = (unsigned char)(avg1 * invBlendFactor + avg2 * blendFactor);
                }

                if (channels == 4) {
                    float avgA1 = (float)(cells[nearestCellIndex].sumA / cells[nearestCellIndex].count);
                    float avgA2 = (float)(cells[secondNearestCellIndex].sumA / cells[secondNearestCellIndex].count);
                    output[dstIdx + 3] = (unsigned char)(avgA1 * invBlendFactor + avgA2 * blendFactor);
                }
            } else if (nearestCellIndex >= 0 && cells[nearestCellIndex].count > 0) {
                // Use single cell color
                if (channels >= 3) {
                    output[dstIdx + 0] = (unsigned char)(cells[nearestCellIndex].sumR / cells[nearestCellIndex].count);
                    output[dstIdx + 1] = (unsigned char)(cells[nearestCellIndex].sumG / cells[nearestCellIndex].count);
                    output[dstIdx + 2] = (unsigned char)(cells[nearestCellIndex].sumB / cells[nearestCellIndex].count);
                } else if (channels == 1) {
                    output[dstIdx] = (unsigned char)(cells[nearestCellIndex].sumR / cells[nearestCellIndex].count);
                }

                if (channels == 4) {
                    output[dstIdx + 3] = (unsigned char)(cells[nearestCellIndex].sumA / cells[nearestCellIndex].count);
                }
            } else {
                // Fallback - copy original pixel
                int srcIdx = (y * width + x) * channels;
                for (int c = 0; c < channels; c++) {
                    output[dstIdx + c] = input[srcIdx + c];
                }
            }
        }
    }

    // Clean up
    free(precomputedPointsX);
    free(precomputedPointsY);
    freeHashMap(cellMap);
    free(cells);

    return OC_STATUS_OK;
}