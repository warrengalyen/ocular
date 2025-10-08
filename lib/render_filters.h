/**
 * @file: render_filters.h
 * @author Warren Galyen
 * Created: 10-8-2025
 * Last Updated: 10-8-2025
 * Last update: added Clouds filter
 *
 * @brief Render filter definitions
 */

#ifndef OCULAR_RENDER_FILTERS_H
#define OCULAR_RENDER_FILTERS_H

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "core.h"
#include "util.h"
#include "noise.h"

// Noise generator types
typedef enum {
    OC_NOISE_PERLIN = 0,   // Classic Perlin noise
    OC_NOISE_SIMPLEX = 1   // Simplex noise (faster, less artifacts)
} OC_NoiseGenerator;

// Cloud generation parameters structure
typedef struct {
    float scale;                    // Scale of the noise (1.0-100.0, larger = smoother, smaller = more detail)
    int quality;                    // Quality/detail level (1-8, higher = more detail)
    unsigned char shadowColorR;     // Red component of shadow/sky color
    unsigned char shadowColorG;     // Green component of shadow/sky color  
    unsigned char shadowColorB;     // Blue component of shadow/sky color
    unsigned char highlightColorR;  // Red component of highlight/cloud color
    unsigned char highlightColorG;  // Green component of highlight/cloud color
    unsigned char highlightColorB;  // Blue component of highlight/cloud color
    float opacity;                  // Opacity of the cloud effect (0.0-100.0)
    int seed;                       // Random seed for noise generation (0 = auto-generate random seed)
    OC_NoiseGenerator generator;    // Noise generator type (OC_NOISE_PERLIN or OC_NOISE_SIMPLEX)
} CloudParams;

// Default cloud parameters (similar to Photoshop defaults)
static const CloudParams DEFAULT_CLOUD_PARAMS = {
    15.0f,             // scale (lower = more detail/density)
    5,                 // quality
    0,                 // shadowColorR
    0,                 // shadowColorG
    0,                 // shadowColorB
    255,               // highlightColorR
    255,               // highlightColorG
    255,               // highlightColorB
    100.0f,            // opacity
    0,                 // seed (0 = random)
    OC_NOISE_SIMPLEX   // generator (simplex is faster and has less artifacts)
};

/**
 * @brief Generate clouds using noise, similar to Photoshop's Render Clouds filter
 * @param Input Input image buffer (can be NULL to generate clouds from scratch)
 * @param Output Output image buffer
 * @param Width Width of the image in pixels
 * @param Height Height of the image in pixels
 * @param Channels Number of color channels
 * @param params Cloud generation parameters (use DEFAULT_CLOUD_PARAMS for defaults)
 *               see CloudParams struct in render_filters.h for more details
 * @return OC_STATUS_OK if successful, otherwise an error code
 */
OC_STATUS ocularRenderClouds(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, const CloudParams* params);

#endif /* OCULAR_RENDER_FILTERS_H */
