/**
 * @file: pixelate_filters.c
 * @author Warren Galyen
 * Created: 10-3-2025
 * Last Updated: 10-3-2025
 * Last update: added color halftone filter
 *
 * @brief Implementation of pixelation and artistic filters
 */

#include "pixelate_filters.h"
#include "core.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

