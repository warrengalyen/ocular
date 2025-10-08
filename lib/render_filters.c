/**
 * @file: render_filters.c
 * @author Warren Galyen
 * Created: 10-8-2025
 * Last Updated: 10-8-2025
 * Last update: added Clouds filter
 *
 * @brief Implementation of render filters
 */

#include "render_filters.h"

OC_STATUS ocularRenderClouds(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, const CloudParams* params) {
    if (Output == NULL || params == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }

    if (Width <= 0 || Height <= 0 || Channels < 3) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    // Validate and clamp parameters
    float scale = clamp(params->scale, 1.0f, 100.0f);
    int quality = clamp(params->quality, 1, 8);
    float opacity = clamp(params->opacity, 0.0f, 100.0f);

    // Calculate scale as a fraction of the image's smallest dimension
    float internalScale;
    if (Width > Height) {
        internalScale = (scale * 0.01f) * (float)Height;
    } else {
        internalScale = (scale * 0.01f) * (float)Width;
    }

    // Invert scale for noise calculation
    if (internalScale > 0.0f) {
        internalScale = 1.0f / internalScale;
    }

    // Convert opacity to 0-1 range
    float opacityFactor = opacity / 100.0f;

    // Generate random seed if not provided
    int seed = params->seed;
    if (seed == 0) {
        seed = (int)time(NULL) ^ (Width * Height);
    }

    // Use seed to generate random offsets for noise variation
    srand(seed);
    float offsetX = (float)(rand() % 10000);
    float offsetY = (float)(rand() % 10000);

    // Fixed internal parameters for fractal noise
    const float persistence = 0.5f;
    const float lacunarity = 2.0f;

    // Generate cloud noise for each pixel
    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            int pixelIndex = (y * Width + x) * Channels;

            // Get original pixel values (from Input if provided, otherwise use shadow color as background)
            unsigned char origR, origG, origB;
            if (Input != NULL) {
                origR = Input[pixelIndex];
                origG = Input[pixelIndex + 1];
                origB = Input[pixelIndex + 2];
            } else {
                origR = params->shadowColorR;
                origG = params->shadowColorG;
                origB = params->shadowColorB;
            }

            // Calculate world coordinates with random offset based on seed
            float worldX = (float)x * internalScale + offsetX;
            float worldY = (float)y * internalScale + offsetY;

            // Generate fractal noise using selected generator and quality parameter
            float noise = 0.0f;
            if (params->generator == OC_NOISE_SIMPLEX) {
                // Use Simplex noise (faster, less artifacts)
                float value = 0.0f;
                float amplitude = 1.0f;
                float frequency = 1.0f;
                float maxValue = 0.0f;

                for (int i = 0; i < quality; i++) {
                    value += simplexNoise2D(worldX * frequency, worldY * frequency) * amplitude;
                    maxValue += amplitude;
                    amplitude *= persistence;
                    frequency *= lacunarity;
                }
                noise = value / maxValue;
            } else {
                // Use Perlin noise (classic)
                noise = fractalBrownianMotion(worldX, worldY, quality, persistence, lacunarity);
            }

            // Normalize noise from [-1, 1] to [0, 1] range
            // (both perlinNoise2D and simplexNoise2D return [-1, 1])
            noise = (noise + 1.0f) * 0.5f;
            noise = clamp(noise, 0.0f, 1.0f);

            // Blend between shadow and highlight colors to create cloud effect
            unsigned char cloudR = (unsigned char)(params->shadowColorR + noise * (params->highlightColorR - params->shadowColorR));
            unsigned char cloudG = (unsigned char)(params->shadowColorG + noise * (params->highlightColorG - params->shadowColorG));
            unsigned char cloudB = (unsigned char)(params->shadowColorB + noise * (params->highlightColorB - params->shadowColorB));

            // Blend cloud effect with original image based on opacity
            // At 100% opacity: full cloud effect
            // At 0% opacity: original image unchanged
            unsigned char r = (unsigned char)(origR * (1.0f - opacityFactor) + cloudR * opacityFactor);
            unsigned char g = (unsigned char)(origG * (1.0f - opacityFactor) + cloudG * opacityFactor);
            unsigned char b = (unsigned char)(origB * (1.0f - opacityFactor) + cloudB * opacityFactor);

            // Write blended pixel data
            Output[pixelIndex] = r;     // Red
            Output[pixelIndex + 1] = g; // Green
            Output[pixelIndex + 2] = b; // Blue

            // Set alpha channel if present
            if (Channels >= 4) {
                Output[pixelIndex + 3] = 255; // Fully opaque
            }
        }
    }

    return OC_STATUS_OK;
}