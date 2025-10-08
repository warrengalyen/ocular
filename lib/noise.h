/**
 * @file: noise.h
 * @author Warren Galyen
 * Created: 10-1-2024
 * Last Updated: 10-1-2025
 * Last update: initial implementation
 *
 * @brief Perlin noise generation functions for procedural content creation
 */

#ifndef OCULAR_NOISE_H
#define OCULAR_NOISE_H

#include <math.h>
#include <stdlib.h>

// Perlin noise permutation table (classic Ken Perlin implementation)
static const int PERLIN_PERMUTATION[512] = {
    151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225,
    140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148,
    247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32,
    57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175,
    74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122,
    60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54,
    65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169,
    200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64,
    52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212,
    207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213,
    119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
    129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104,
    218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241,
    81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157,
    184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93,
    222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180,
    151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225,
    140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148,
    247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32,
    57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175,
    74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122,
    60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54,
    65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169,
    200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64,
    52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212,
    207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213,
    119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
    129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104,
    218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241,
    81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157,
    184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93,
    222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
};

// Gradient vectors for Perlin noise
static const float PERLIN_GRADIENTS[12][2] = {
    {1, 1}, {-1, 1}, {1, -1}, {-1, -1},
    {1, 0}, {-1, 0}, {0, 1}, {0, -1},
    {1, 1}, {-1, 1}, {1, -1}, {-1, -1}
};

/**
 * @brief Smooth interpolation function (smoothstep)
 * @param t Interpolation parameter (0.0 to 1.0)
 * @return Smoothed value
 */
static inline float noiseFade(float t) {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

/**
 * @brief Linear interpolation between two values
 * @param a First value
 * @param b Second value
 * @param t Interpolation factor (0.0 to 1.0)
 * @return Interpolated value
 */
static inline float noiseLerp(float a, float b, float t) {
    return a + t * (b - a);
}

/**
 * @brief Dot product for gradient calculation
 * @param hash Hash value for gradient selection
 * @param x X coordinate
 * @param y Y coordinate
 * @return Dot product result
 */
static inline float noiseGrad(int hash, float x, float y) {
    int h = hash & 11;
    return PERLIN_GRADIENTS[h][0] * x + PERLIN_GRADIENTS[h][1] * y;
}

// Simplex noise constants
#define F2 0.3660254037844386f  // (sqrt(3) - 1) / 2
#define G2 0.21132486540518713f // (3 - sqrt(3)) / 6

/**
 * @brief Generate 2D Simplex noise (faster than Perlin, less directional artifacts)
 * @param x X coordinate
 * @param y Y coordinate
 * @return Noise value in range [-1, 1]
 */
static float simplexNoise2D(float x, float y) {
    // Skew the input space to determine which simplex cell we're in
    float s = (x + y) * F2;
    int i = (int)floor(x + s);
    int j = (int)floor(y + s);
    
    // Unskew the cell origin back to (x,y) space
    float t = (i + j) * G2;
    float X0 = i - t;
    float Y0 = j - t;
    float x0 = x - X0;
    float y0 = y - Y0;
    
    // Determine which simplex we are in
    int i1, j1;
    if (x0 > y0) {
        i1 = 1; j1 = 0;  // Lower triangle, XY order: (0,0)->(1,0)->(1,1)
    } else {
        i1 = 0; j1 = 1;  // Upper triangle, YX order: (0,0)->(0,1)->(1,1)
    }
    
    // Offsets for middle corner in (x,y) unskewed coords
    float x1 = x0 - i1 + G2;
    float y1 = y0 - j1 + G2;
    // Offsets for last corner in (x,y) unskewed coords
    float x2 = x0 - 1.0f + 2.0f * G2;
    float y2 = y0 - 1.0f + 2.0f * G2;
    
    // Work out the hashed gradient indices of the three simplex corners
    int ii = i & 255;
    int jj = j & 255;
    int gi0 = PERLIN_PERMUTATION[ii + PERLIN_PERMUTATION[jj]] & 11;
    int gi1 = PERLIN_PERMUTATION[ii + i1 + PERLIN_PERMUTATION[jj + j1]] & 11;
    int gi2 = PERLIN_PERMUTATION[ii + 1 + PERLIN_PERMUTATION[jj + 1]] & 11;
    
    // Calculate the contribution from the three corners
    float n0, n1, n2;
    
    float t0 = 0.5f - x0 * x0 - y0 * y0;
    if (t0 < 0) {
        n0 = 0.0f;
    } else {
        t0 *= t0;
        n0 = t0 * t0 * (PERLIN_GRADIENTS[gi0][0] * x0 + PERLIN_GRADIENTS[gi0][1] * y0);
    }
    
    float t1 = 0.5f - x1 * x1 - y1 * y1;
    if (t1 < 0) {
        n1 = 0.0f;
    } else {
        t1 *= t1;
        n1 = t1 * t1 * (PERLIN_GRADIENTS[gi1][0] * x1 + PERLIN_GRADIENTS[gi1][1] * y1);
    }
    
    float t2 = 0.5f - x2 * x2 - y2 * y2;
    if (t2 < 0) {
        n2 = 0.0f;
    } else {
        t2 *= t2;
        n2 = t2 * t2 * (PERLIN_GRADIENTS[gi2][0] * x2 + PERLIN_GRADIENTS[gi2][1] * y2);
    }
    
    // Add contributions from each corner to get the final noise value.
    // The result is scaled to return values in the interval [-1,1].
    return 70.0f * (n0 + n1 + n2);
}

/**
 * @brief Generate 2D Perlin noise
 * @param x X coordinate
 * @param y Y coordinate
 * @return Noise value in range [-1, 1]
 */
static float perlinNoise2D(float x, float y) {
    // Find the unit grid cell containing the point
    int X = (int)floor(x) & 255;
    int Y = (int)floor(y) & 255;
    
    // Get relative x,y coordinates within the cell
    x -= floor(x);
    y -= floor(y);
    
    // Compute fade curves for each coordinate
    float u = noiseFade(x);
    float v = noiseFade(y);
    
    // Hash coordinates of the 4 square corners
    int A = PERLIN_PERMUTATION[X] + Y;
    int AA = PERLIN_PERMUTATION[A];
    int AB = PERLIN_PERMUTATION[A + 1];
    int B = PERLIN_PERMUTATION[X + 1] + Y;
    int BA = PERLIN_PERMUTATION[B];
    int BB = PERLIN_PERMUTATION[B + 1];
    
    // And add blended results from 4 corners of the cell
    return noiseLerp(noiseLerp(noiseGrad(AA, x, y),
                               noiseGrad(BA, x - 1, y), u),
                     noiseLerp(noiseGrad(AB, x, y - 1),
                               noiseGrad(BB, x - 1, y - 1), u), v);
}

/**
 * @brief Generate Fractal Brownian Motion (fBm) - combines multiple octaves of noise
 * @param x X coordinate
 * @param y Y coordinate
 * @param octaves Number of noise octaves to combine
 * @param persistence How much each octave contributes (0.0-1.0)
 * @param lacunarity Frequency multiplier between octaves (usually 2.0)
 * @return Normalized noise value in range [0, 1]
 */
static float fractalBrownianMotion(float x, float y, int octaves, float persistence, float lacunarity) {
    float value = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float maxValue = 0.0f; // Used for normalizing result to 0.0 - 1.0
    
    for (int i = 0; i < octaves; i++) {
        value += perlinNoise2D(x * frequency, y * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    
    return value / maxValue;
}

/**
 * @brief Generate 1D Perlin noise
 * @param x X coordinate
 * @return Noise value in range [-1, 1]
 */
static float perlinNoise1D(float x) {
    // Find the unit grid cell containing the point
    int X = (int)floor(x) & 255;
    
    // Get relative x coordinate within the cell
    x -= floor(x);
    
    // Compute fade curve
    float u = noiseFade(x);
    
    // Hash coordinates of the 2 line endpoints
    int A = PERLIN_PERMUTATION[X];
    int B = PERLIN_PERMUTATION[X + 1];
    
    // Simple 1D gradient (just use the hash value as gradient)
    float gradA = (float)(A & 7) / 7.0f - 0.5f;
    float gradB = (float)(B & 7) / 7.0f - 0.5f;
    
    // Interpolate between the two gradients
    return noiseLerp(gradA * x, gradB * (x - 1), u);
}

/**
 * @brief Generate 3D Perlin noise
 * @param x X coordinate
 * @param y Y coordinate
 * @param z Z coordinate
 * @return Noise value in range [-1, 1]
 */
static float perlinNoise3D(float x, float y, float z) {
    // Find the unit grid cell containing the point
    int X = (int)floor(x) & 255;
    int Y = (int)floor(y) & 255;
    int Z = (int)floor(z) & 255;
    
    // Get relative x,y,z coordinates within the cell
    x -= floor(x);
    y -= floor(y);
    z -= floor(z);
    
    // Compute fade curves for each coordinate
    float u = noiseFade(x);
    float v = noiseFade(y);
    float w = noiseFade(z);
    
    // Hash coordinates of the 8 cube corners
    int A = PERLIN_PERMUTATION[X] + Y;
    int AA = PERLIN_PERMUTATION[A] + Z;
    int AB = PERLIN_PERMUTATION[A + 1] + Z;
    int B = PERLIN_PERMUTATION[X + 1] + Y;
    int BA = PERLIN_PERMUTATION[B] + Z;
    int BB = PERLIN_PERMUTATION[B + 1] + Z;
    
    // Trilinear interpolation (simplified - would need proper 3D gradients)
    float noise000 = perlinNoise2D(x, y);
    float noise001 = perlinNoise2D(x, y);
    float noise010 = perlinNoise2D(x, y);
    float noise011 = perlinNoise2D(x, y);
    float noise100 = perlinNoise2D(x, y);
    float noise101 = perlinNoise2D(x, y);
    float noise110 = perlinNoise2D(x, y);
    float noise111 = perlinNoise2D(x, y);
    
    // Interpolate in Z direction
    float noise00 = noiseLerp(noise000, noise001, w);
    float noise01 = noiseLerp(noise010, noise011, w);
    float noise10 = noiseLerp(noise100, noise101, w);
    float noise11 = noiseLerp(noise110, noise111, w);
    
    // Interpolate in Y direction
    float noise0 = noiseLerp(noise00, noise01, v);
    float noise1 = noiseLerp(noise10, noise11, v);
    
    // Interpolate in X direction
    return noiseLerp(noise0, noise1, u);
}

/**
 * @brief Generate ridge noise (absolute value of noise with offset)
 * @param x X coordinate
 * @param y Y coordinate
 * @param octaves Number of noise octaves
 * @param persistence How much each octave contributes
 * @param lacunarity Frequency multiplier between octaves
 * @param offset Offset value (typically 1.0)
 * @return Ridge noise value
 */
static float ridgeNoise(float x, float y, int octaves, float persistence, float lacunarity, float offset) {
    float value = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float maxValue = 0.0f;
    
    for (int i = 0; i < octaves; i++) {
        float noise = perlinNoise2D(x * frequency, y * frequency);
        value += fabsf(noise + offset) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    
    return value / maxValue;
}

/**
 * @brief Generate turbulence noise (absolute value of noise)
 * @param x X coordinate
 * @param y Y coordinate
 * @param octaves Number of noise octaves
 * @param persistence How much each octave contributes
 * @param lacunarity Frequency multiplier between octaves
 * @return Turbulence noise value
 */
static float turbulenceNoise(float x, float y, int octaves, float persistence, float lacunarity) {
    float value = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float maxValue = 0.0f;
    
    for (int i = 0; i < octaves; i++) {
        value += fabsf(perlinNoise2D(x * frequency, y * frequency)) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    
    return value / maxValue;
}

#endif /* OCULAR_NOISE_H */
