/**
 * @file: distort_filters.c
 * @brief Implementation of distortion filters for image manipulation
 */

#include "distort_filters.h"
#include "core.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "util.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


// Bilinear interpolation with clamp-to-edge sampling (matches Photoshop behavior)
// Any coordinate outside image bounds is clamped to nearest valid pixel, then interpolated
static inline float bilinearSample(uint8_t* image, int width, int height, 
                                   int channels, int channel,
                                   float x, float y) {
    // Clamp input coordinates to valid image bounds [0, width-1] x [0, height-1]
    x = clamp(x, 0.0f, (float)(width - 1));
    y = clamp(y, 0.0f, (float)(height - 1));
    
    // Get integer and fractional parts for interpolation
    int x0 = (int)x;
    int y0 = (int)y;
    float fx = x - x0;
    float fy = y - y0;
    
    // Clamp neighbor indices to ensure they stay within bounds (edge handling)
    int x1 = (x0 + 1 < width) ? x0 + 1 : x0;
    int y1 = (y0 + 1 < height) ? y0 + 1 : y0;
    
    // Get the four surrounding pixels (with clamped edge pixels repeated)
    uint8_t p00 = image[(y0 * width + x0) * channels + channel];
    uint8_t p10 = image[(y0 * width + x1) * channels + channel];
    uint8_t p01 = image[(y1 * width + x0) * channels + channel];
    uint8_t p11 = image[(y1 * width + x1) * channels + channel];
    
    // Bilinear interpolation
    float top = p00 * (1.0f - fx) + p10 * fx;
    float bottom = p01 * (1.0f - fx) + p11 * fx;
    return top * (1.0f - fy) + bottom * fy;
}

OC_STATUS ocularPinchDistortionFilter(unsigned char* input, unsigned char* output, int width, int height, int stride, float amount) {
    // Validate inputs
    if (input == NULL || output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    
    if (width <= 0 || height <= 0 || stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }
    
    // Clamp amount to valid range
    amount = clamp(amount, -100.0f, 100.0f);
    
    // If amount is zero, just copy input to output
    if (amount == 0.0f) {
        if (input != output) {
            memcpy(output, input, height * stride);
        }
        return OC_STATUS_OK;
    }
    
    int channels = stride / width;
    
    // Calculate center point
    float centerX = (width - 1) / 2.0f;
    float centerY = (height - 1) / 2.0f;
    
    // Calculate maximum radius: distance from center to nearest edge
    // (half of width or height, whichever is smaller - inscribed circle)
    float maxRadius = (width < height) ? centerX : centerY;
    
    // Convert amount from [-100, 100] to a usable strength value
    // Positive = pinch (pull inward), Negative = bulge (push outward)
    // Use same scaling for both pinch and bulge
    float strength = amount / 100.0f;  // Range: -1.0 to +1.0
    
    // Create temporary buffer if doing in-place operation
    uint8_t* source = input;
    uint8_t* tempBuffer = NULL;
    
    if (input == output) {
        tempBuffer = (uint8_t*)malloc(height * stride);
        if (tempBuffer == NULL) {
            return OC_STATUS_ERR_OUTOFMEMORY;
        }
        memcpy(tempBuffer, input, height * stride);
        source = tempBuffer;
    }
    
    // Apply distortion
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int dstIdx = (y * width + x) * channels;
            
            // Calculate distance from center
            float dx = x - centerX;
            float dy = y - centerY;
            float distance = sqrtf(dx * dx + dy * dy);
            
            // Pixels outside the radius remain untouched
            if (distance > maxRadius) {
                for (int c = 0; c < channels; c++) {
                    output[dstIdx + c] = source[dstIdx + c];
                }
                continue;
            }
            
            // Source coordinates (where to sample from)
            float srcX, srcY;
            
            if (distance < 0.0001f) {
                // At the center, no distortion needed
                srcX = x;
                srcY = y;
            } else {
                // Normalize distance (0 at center, 1 at edge of radius)
                float normalizedDist = distance / maxRadius;
                
                // Apply distortion formula
                float distortionFactor;
                
                if (amount > 0) {
                    // Pinch: use inverse power function to properly stretch edges toward center
                    // normalized^(1/(1+strength)) where strength ∈ [0, 1]
                    // amount = 100 → strength = 1.0 → exponent = 0.5 (square root)
                    // amount = 50  → strength = 0.5 → exponent = 0.667
                    // This creates smooth stretching without tight center compression
                    distortionFactor = powf(normalizedDist, 1.0f / (1.0f + strength));
                } else {
                    // Bulge: use power function with additive strength
                    // normalized^(1-strength) where strength ∈ [-1, 0]
                    // amount = -100 → strength = -1.0 → exponent = 2.0 (square)
                    // amount = -50  → strength = -0.5 → exponent = 1.5
                    distortionFactor = powf(normalizedDist, 1.0f - strength);
                }
                
                // Calculate new radius (where to sample from)
                float newRadius = maxRadius * distortionFactor;
                
                // Calculate source coordinates
                float angle = atan2f(dy, dx);
                srcX = centerX + newRadius * cosf(angle);
                srcY = centerY + newRadius * sinf(angle);
            }
            
            // Sample from source image using bilinear interpolation
            // Clamp remapped coordinates to valid bounds,
            // then bilinearly interpolate. This keeps the outer border intact with no holes.
            for (int c = 0; c < channels; c++) {
                float value = bilinearSample(source, width, height, channels, c, srcX, srcY);
                output[dstIdx + c] = (unsigned char)clamp(value, 0.0f, 255.0f);
            }
        }
    }
    
    // Free temporary buffer if allocated
    if (tempBuffer != NULL) {
        free(tempBuffer);
    }
    
    return OC_STATUS_OK;
}
