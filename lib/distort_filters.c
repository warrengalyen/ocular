/**
 * @file: distort_filters.c
 * @author Warren Galyen
 * Created: 10-2-2025
 * Last Updated: 10-3-2025
 * Last update: added polar coordinates filter
 *
 * @brief Implementation of distortion filters
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


// Bilinear interpolation with clamp-to-edge sampling
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

OC_STATUS ocularTwirlDistortionFilter(unsigned char* input, unsigned char* output,
                                      int width, int height, int stride,
                                      float angle) {
    // Validate inputs
    if (input == NULL || output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    
    if (width <= 0 || height <= 0 || stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }
    
    // If angle is zero, just copy input to output
    if (angle == 0.0f) {
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
    float maxRadius = (width < height) ? centerX : centerY;
    
    // Convert angle from degrees to radians
    float angleRad = angle * M_PI / 180.0f;
    
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
    
    // Apply twirl distortion
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
                // At the center, no rotation needed (or would be undefined)
                srcX = x;
                srcY = y;
            } else {
                // Normalize distance (0 at center, 1 at edge)
                float normalizedDist = distance / maxRadius;
                
                // Calculate rotation amount that decreases from center to edge
                // Using quadratic falloff for smoother result (like Photoshop)
                float falloff = 1.0f - normalizedDist;
                falloff = falloff * falloff;  // Square for smoother falloff
                float rotationAmount = angleRad * falloff;
                
                // Get current angle from center
                float currentAngle = atan2f(dy, dx);
                
                // Apply rotation
                float newAngle = currentAngle - rotationAmount;
                
                // Calculate source coordinates using rotated angle
                srcX = centerX + distance * cosf(newAngle);
                srcY = centerY + distance * sinf(newAngle);
            }
            
            // Sample from source image using bilinear interpolation
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

OC_STATUS ocularRippleDistortionFilter(unsigned char* input, unsigned char* output,
                                       int width, int height, int stride,
                                       int amount, OcRippleSize size) {
    // Validate inputs
    if (input == NULL || output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    
    if (width <= 0 || height <= 0 || stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }
    
    // If amount is zero, just copy input to output
    if (amount == 0) {
        if (input != output) {
            memcpy(output, input, height * stride);
        }
        return OC_STATUS_OK;
    }
    
    // Clamp amount to valid range
    amount = clamp((float)amount, -999.0f, 999.0f);
    
    // Map size enum to wavelength (distance between wave peaks)
    float wavelength;
    switch (size) {
        case OC_RIPPLE_SMALL:
            wavelength = 10.0f;   // Many small ripples
            break;
        case OC_RIPPLE_LARGE:
            wavelength = 50.0f;   // Few large ripples
            break;
        case OC_RIPPLE_MEDIUM:
        default:
            wavelength = 25.0f;   // Medium ripples
            break;
    }
    
    // Convert amount (percent) to amplitude (pixels)
    // Scale based on image size for consistent visual appearance
    int channels = stride / width;
    
    // Calculate center point
    float centerX = (width - 1) / 2.0f;
    float centerY = (height - 1) / 2.0f;
    
    // Calculate maximum radius: distance from center to nearest edge
    float maxRadius = (width < height) ? centerX : centerY;
    
    // Convert amount to amplitude based on image size
    float amplitude = (amount / 100.0f) * (maxRadius / 10.0f);
    
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
    
    // Apply ripple distortion
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
                // Calculate ripple displacement using sine wave
                // The wave propagates radially outward from the center
                float wavePhase = (distance / wavelength) * 2.0f * M_PI;
                float displacement = sinf(wavePhase) * amplitude;
                
                // Apply displacement in radial direction (toward/away from center)
                // Normalize the direction vector
                float nx = dx / distance;  // Normal x (radial direction)
                float ny = dy / distance;  // Normal y (radial direction)
                
                // Apply radial displacement
                // Positive displacement moves away from center, negative moves toward center
                srcX = x + nx * displacement;
                srcY = y + ny * displacement;
            }
            
            // Sample from source image using bilinear interpolation
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

OC_STATUS ocularSpherizeDistortionFilter(unsigned char* input, unsigned char* output,
                                         int width, int height, int stride,
                                         int amount, OcSpherizeMode mode) {
    // Validate inputs
    if (input == NULL || output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    
    if (width <= 0 || height <= 0 || stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }
    
    // If amount is zero, just copy input to output
    if (amount == 0) {
        if (input != output) {
            memcpy(output, input, height * stride);
        }
        return OC_STATUS_OK;
    }
    
    // Clamp amount to valid range and convert to strength [-1, 1]
    float strength = clamp((float)amount, -100.0f, 100.0f) / 100.0f;
    
    int channels = stride / width;
    
    // Calculate center point
    float centerX = (width - 1) / 2.0f;
    float centerY = (height - 1) / 2.0f;
    
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
    
    // Determine the radius for normalization based on mode
    float normRadius;
    switch (mode) {
        case OC_SPHERIZE_NORMAL:
            // Use inscribed circle (smaller dimension)
            normRadius = (width < height) ? centerX : centerY;
            break;
        case OC_SPHERIZE_HORIZONTAL:
            // Use full width for horizontal cylinder
            normRadius = centerX;
            break;
        case OC_SPHERIZE_VERTICAL:
            // Use full height for vertical cylinder
            normRadius = centerY;
            break;
        default:
            normRadius = (width < height) ? centerX : centerY;
            break;
    }
    
    // Apply spherize distortion
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int dstIdx = (y * width + x) * channels;
            
            // Calculate normalized coordinates [-1, 1] range
            float normX = (x - centerX) / normRadius;
            float normY = (y - centerY) / normRadius;
            
            // Source coordinates (where to sample from)
            float srcX, srcY;
            
            // Apply spherize transformation based on mode
            switch (mode) {
                case OC_SPHERIZE_NORMAL: {
                    // Full spherical distortion (both axes)
                    float r2 = normX * normX + normY * normY;
                    
                    if (r2 > 1.0f) {
                        // Outside the unit circle - copy unchanged
                        for (int c = 0; c < channels; c++) {
                            output[dstIdx + c] = source[dstIdx + c];
                        }
                        continue;
                    }
                    
                    if (r2 < 0.0001f) {
                        // At center, no distortion
                        srcX = x;
                        srcY = y;
                    } else {
                        // Apply spherical projection using proper sphere mapping
                        float r = sqrtf(r2);  // Current radius [0, 1]
                        
                        // Calculate source radius using spherical projection formula
                        float srcRadius;
                        if (strength > 0) {
                            // Convex (bulge out) - map to sphere surface
                            // Use arcsine projection: compresses edges toward center
                            float angle = asinf(r);  // r is already [0,1]
                            srcRadius = angle / (M_PI / 2.0f);
                            // Blend with original based on strength
                            srcRadius = r * (1.0f - strength) + srcRadius * strength;
                        } else {
                            // Concave (pinch in) - inverse projection
                            // Use sine projection: expands edges outward
                            float angle = r * M_PI / 2.0f;  // Map [0,1] to [0, π/2]
                            srcRadius = sinf(angle);
                            // Blend with original based on strength
                            srcRadius = r * (1.0f - fabsf(strength)) + srcRadius * fabsf(strength);
                        }
                        
                        // Calculate scale factor (how much to scale the radius)
                        float scale = srcRadius / r;
                        
                        // Apply scale to normalized coordinates
                        normX *= scale;
                        normY *= scale;
                        
                        // Convert back to pixel coordinates
                        srcX = normX * normRadius + centerX;
                        srcY = normY * normRadius + centerY;
                    }
                    break;
                }
                    
                case OC_SPHERIZE_HORIZONTAL: {
                    // Horizontal cylindrical distortion
                    float absX = fabsf(normX);
                    
                    if (absX > 1.0f) {
                        // Outside valid range - copy unchanged
                        for (int c = 0; c < channels; c++) {
                            output[dstIdx + c] = source[dstIdx + c];
                        }
                        continue;
                    }
                    
                    if (absX < 0.0001f) {
                        srcX = x;
                        srcY = y;
                    } else {
                        // Apply cylindrical projection along X axis
                        float srcAbsX;
                        if (strength > 0) {
                            // Convex (bulge out)
                            float angle = asinf(absX);
                            srcAbsX = angle / (M_PI / 2.0f);
                            srcAbsX = absX * (1.0f - strength) + srcAbsX * strength;
                        } else {
                            // Concave (pinch in)
                            float angle = absX * M_PI / 2.0f;
                            srcAbsX = sinf(angle);
                            srcAbsX = absX * (1.0f - fabsf(strength)) + srcAbsX * fabsf(strength);
                        }
                        
                        // Calculate scale and preserve sign
                        float scale = srcAbsX / absX;
                        normX *= scale;
                        
                        // Convert back to pixel coordinates
                        srcX = normX * normRadius + centerX;
                        srcY = y;  // Y coordinate unchanged
                    }
                    break;
                }
                    
                case OC_SPHERIZE_VERTICAL: {
                    // Vertical cylindrical distortion
                    float absY = fabsf(normY);
                    
                    if (absY > 1.0f) {
                        // Outside valid range - copy unchanged
                        for (int c = 0; c < channels; c++) {
                            output[dstIdx + c] = source[dstIdx + c];
                        }
                        continue;
                    }
                    
                    if (absY < 0.0001f) {
                        srcX = x;
                        srcY = y;
                    } else {
                        // Apply cylindrical projection along Y axis
                        float srcAbsY;
                        if (strength > 0) {
                            // Convex (bulge out)
                            float angle = asinf(absY);
                            srcAbsY = angle / (M_PI / 2.0f);
                            srcAbsY = absY * (1.0f - strength) + srcAbsY * strength;
                        } else {
                            // Concave (pinch in)
                            float angle = absY * M_PI / 2.0f;
                            srcAbsY = sinf(angle);
                            srcAbsY = absY * (1.0f - fabsf(strength)) + srcAbsY * fabsf(strength);
                        }
                        
                        // Calculate scale and preserve sign
                        float scale = srcAbsY / absY;
                        normY *= scale;
                        
                        // Convert back to pixel coordinates
                        srcX = x;  // X coordinate unchanged
                        srcY = normY * normRadius + centerY;
                    }
                    break;
                }
                    
                default:
                    // No distortion
                    srcX = x;
                    srcY = y;
                    break;
            }
            
            // Sample from source image using bilinear interpolation
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

OC_STATUS ocularPolarCoordinatesFilter(unsigned char* input, unsigned char* output,
                                       int width, int height, int stride,
                                       OcPolarMode mode) {
    // Validate inputs
    if (input == NULL || output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    
    if (width <= 0 || height <= 0 || stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }
    
    int channels = stride / width;
    
    // Calculate center point
    float centerX = (width - 1) / 2.0f;
    float centerY = (height - 1) / 2.0f;
    
    // Maximum radius: use half of image height (Photoshop convention)
    // This ensures the bottom of the rectangular image maps to the outer circle
    float maxRadius = height / 2.0f;
    
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
    
    if (mode == OC_RECT_TO_POLAR) {
        // Rectangular to Polar conversion
        // Maps rectangular image onto a circle (like wrapping it around)
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int dstIdx = (y * width + x) * channels;
                
                // Calculate distance from center
                float dx = x - centerX;
                float dy = y - centerY;
                float distance = sqrtf(dx * dx + dy * dy);
                
                // Source coordinates
                float srcX, srcY;
                
                // Calculate angle from center
                // atan2 gives angle where 0 = right, π/2 = down, π = left, -π/2 = up
                float angle = atan2f(dy, dx);
                
                // Rotate by -90 degrees so top of circle (dy < 0) maps to center of source
                // This makes the left edge of the rectangular image appear at the top of the circle
                angle += M_PI / 2.0f;
                
                // Normalize to [0, 2*PI]
                if (angle < 0) angle += 2.0f * M_PI;
                if (angle >= 2.0f * M_PI) angle -= 2.0f * M_PI;
                
                // Map angle to horizontal position in source image (reversed for correct orientation)
                // Angle goes counter-clockwise, but we want to read the source from left to right clockwise
                srcX = width - (angle / (2.0f * M_PI)) * width;
                if (srcX >= width) srcX = 0; // wrap around
                
                // Handle wrapping at image edges
                if (srcX >= width) srcX = width - 1;
                if (srcX < 0) srcX = 0;
                
                // Map distance to vertical position in source image
                // Center (distance=0) maps to top of image (y=0)
                // Outer edge (distance=maxRadius) maps to bottom (y=height-1)
                srcY = (distance / maxRadius) * (height - 1);
                
                // Sample from source image using bilinear interpolation
                for (int c = 0; c < channels; c++) {
                    float value = bilinearSample(source, width, height, channels, c, srcX, srcY);
                    output[dstIdx + c] = (unsigned char)clamp(value, 0.0f, 255.0f);
                }
            }
        }
    } else {
        // Polar to Rectangular conversion
        // Unwraps circular pattern into rectangular coordinates
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int dstIdx = (y * width + x) * channels;
                
                // Map horizontal position to angle (inverse of rect-to-polar)
                // In rect-to-polar we do: srcX = width - (angle / 2π) * width
                // So to invert: angle = 2π * (1 - x/width)
                // Then subtract the π/2 rotation we added in rect-to-polar
                float normalizedX = (float)x / (float)width;
                float angle = 2.0f * M_PI * (1.0f - normalizedX) - M_PI / 2.0f;
                
                // Map vertical position to radius (inverse of rect-to-polar)
                // In rect-to-polar we do: srcY = (distance / maxRadius) * (height-1)
                // So to invert: distance = (y / (height-1)) * maxRadius
                float radius = (y / (float)(height - 1)) * maxRadius;
                
                // Calculate source coordinates using polar to Cartesian conversion
                float srcX = centerX + radius * cosf(angle);
                float srcY = centerY + radius * sinf(angle);
                
                // Sample from source image using bilinear interpolation
                for (int c = 0; c < channels; c++) {
                    float value = bilinearSample(source, width, height, channels, c, srcX, srcY);
                    output[dstIdx + c] = (unsigned char)clamp(value, 0.0f, 255.0f);
                }
            }
        }
    }
    
    // Free temporary buffer if allocated
    if (tempBuffer != NULL) {
        free(tempBuffer);
    }
    
    return OC_STATUS_OK;
}
