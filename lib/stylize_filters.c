/**
 * @file: stylize_filters.c
 * @author Warren Galyen
 * Created: 10-4-2025
 * Last Updated: 10-13-2026
 * Last update: added extrude filter
 *
 * @brief Stylize filter implementations
 */

 #include "stylize_filters.h"
 #include "core.h"
 #include "util.h"
 #include "blur_filters.h"
 #include "blend.h"
 #include "auxiliary.h"
 #include "noise.h"
 #include <math.h>

OC_STATUS ocularOilPaintFilter(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int radius, int intensity) {

    if (Input == NULL || Output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    if (Width <= 0 || Height <= 0 || Stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    int Channels = Stride / Width;
    if (Channels != 3) {
        return OC_STATUS_ERR_NOTSUPPORTED;
    }

    // Ensure filter specific parameters are within valid ranges
    radius = clamp(radius, 1, 200);
    intensity = clamp(intensity, 1, 100);

    // Pre-calculate squared radius for distance check
    const int radiusSquared = radius * radius;

    // Process image in chunks for better cache utilization
    const int CHUNK_SIZE = 32;

    // Allocate arrays on heap to prevent stack overflow
    int* intensityCount = (int*)calloc(intensity, sizeof(int));
    int* sumR = (int*)calloc(intensity, sizeof(int));
    int* sumG = (int*)calloc(intensity, sizeof(int));
    int* sumB = (int*)calloc(intensity, sizeof(int));

    if (!intensityCount || !sumR || !sumG || !sumB) {
        free(intensityCount);
        free(sumR);
        free(sumG);
        free(sumB);
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

    // Process image in chunks
    for (int blockY = 0; blockY < Height; blockY += CHUNK_SIZE) {
        for (int blockX = 0; blockX < Width; blockX += CHUNK_SIZE) {
            int endY = min(blockY + CHUNK_SIZE, Height);
            int endX = min(blockX + CHUNK_SIZE, Width);

            for (int y = blockY; y < endY; y++) {
                for (int x = blockX; x < endX; x++) {
                    // Reset arrays using faster method
                    if (intensity <= 32) {
                        // For small intensity values, direct assignment is faster
                        for (int i = 0; i < intensity; i++) {
                            intensityCount[i] = 0;
                            sumR[i] = 0;
                            sumG[i] = 0;
                            sumB[i] = 0;
                        }
                    } else {
                        memset(intensityCount, 0, intensity * sizeof(int));
                        memset(sumR, 0, intensity * sizeof(int));
                        memset(sumG, 0, intensity * sizeof(int));
                        memset(sumB, 0, intensity * sizeof(int));
                    }

                    // Sample the neighborhood using circular mask
                    for (int dy = -radius; dy <= radius; dy++) {
                        int dy2 = dy * dy;
                        for (int dx = -radius; dx <= radius; dx++) {
                            // Early skip pixels outside circular radius
                            if (dx * dx + dy2 > radiusSquared)
                                continue;

                            int sampleX = x + dx;
                            int sampleY = y + dy;

                            // Mirror pixels at boundaries (optimized)
                            sampleX = sampleX < 0 ? -sampleX : (sampleX >= Width ? 2 * Width - sampleX - 2 : sampleX);
                            sampleY = sampleY < 0 ? -sampleY : (sampleY >= Height ? 2 * Height - sampleY - 2 : sampleY);

                            // Get pixel color (using pointer arithmetic)
                            const unsigned char* pixel = Input + (sampleY * Width + sampleX) * Channels;
                            int r = pixel[0];
                            int g = pixel[1];
                            int b = pixel[2];

                            // Calculate intensity level (optimized)
                            int intensityLevel = ((r + g + b) * intensity) / (3 * 255);
                            // No need for clamp since division guarantees range

                            // Accumulate values
                            intensityCount[intensityLevel]++;
                            sumR[intensityLevel] += r;
                            sumG[intensityLevel] += g;
                            sumB[intensityLevel] += b;
                        }
                    }

                    // Find dominant intensity level (optimized)
                    int maxCount = intensityCount[0];
                    int maxIndex = 0;
                    for (int i = 1; i < intensity; i++) {
                        if (intensityCount[i] > maxCount) {
                            maxCount = intensityCount[i];
                            maxIndex = i;
                        }
                    }

                    // Write output pixel
                    unsigned char* outPixel = Output + (y * Width + x) * Channels;
                    if (maxCount > 0) {
                        outPixel[0] = ClampToByte(sumR[maxIndex] / maxCount);
                        outPixel[1] = ClampToByte(sumG[maxIndex] / maxCount);
                        outPixel[2] = ClampToByte(sumB[maxIndex] / maxCount);
                    } else {
                        const unsigned char* inPixel = Input + (y * Width + x) * Channels;
                        outPixel[0] = inPixel[0];
                        outPixel[1] = inPixel[1];
                        outPixel[2] = inPixel[2];
                    }
                }
            }
        }
    }

    // Clean up
    free(intensityCount);
    free(sumR);
    free(sumG);
    free(sumB);

    return OC_STATUS_OK;
}

OC_STATUS ocularFrostedGlassEffect(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius, int Range) {

    if (Input == NULL || Output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    if (Width <= 0 || Height <= 0 || Stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    // Ensure filter specific parameters are within valid ranges
    Radius = max(Radius, 1);
    Range = clamp(Range, 1, 20);

    int Channels = Stride / Width;

    // First the image is blurred by Gaussian filtering, then the blurred image is randomly sampled in the neighborhood,
    // giving the image a certain degree of random disturbance and blur.
    ocularGaussianBlurFilter(Input, Output, Width, Height, Stride, Radius);

    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            float randomOffsetX = (rand() / (float)RAND_MAX) - 0.5f;
            float randomOffsetY = (rand() / (float)RAND_MAX) - 0.5f;
            int offsetX = (int)(randomOffsetX * (Range * 2 - 1));
            int offsetY = (int)(randomOffsetY * (Range * 2 - 1));

            // Reflect pixels that are out of bounds
            int newY = y + offsetY;
            int newX = x + offsetX;

            if (newY < 0)
                newY = -newY;
            if (newY >= Height)
                newY = 2 * Height - newY - 2;
            if (newX < 0)
                newX = -newX;
            if (newX >= Width)
                newX = 2 * Width - newX - 2;

            int src_idx = (newY * Width + newX) * Channels;
            int dst_idx = (y * Width + x) * Channels;

            for (int c = 0; c < Channels; c++) {
                Output[dst_idx + c] = Input[src_idx + c];
            }
        }
    }

    return OC_STATUS_OK;
}

OC_STATUS ocularFilmGrainEffect(unsigned char* input, unsigned char* output, int width, int height, int channels, float strength, float softness) {

    if (input == NULL || output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    if (width <= 0 || height <= 0 || channels <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    // Clamp filter specific parameters to valid ranges
    strength = clamp(strength, 0.0f, 100.0f) * 0.5f; // Reduced strength impact
    softness = clamp(softness, 0.0f, 25.0f);

    // Pre-calculate noise values for better performance
    float* noiseValues = (float*)malloc(width * height * sizeof(float));
    for (int i = 0; i < width * height; i++) {
        noiseValues[i] = (float)(rand() % 256) / 255.0f;
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float noise = noiseValues[y * width + x];

            // Apply softness by averaging with neighboring noise (only if needed)
            if (softness > 0) {
                float blur = 0;
                int samples = 0;
                int radius = (int)(softness / 4); // Reduced radius for better performance

                for (int dy = -radius; dy <= radius; dy += 2) { // Skip pixels for speed
                    for (int dx = -radius; dx <= radius; dx += 2) {
                        int nx = x + dx;
                        int ny = y + dy;

                        if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                            blur += noiseValues[ny * width + nx];
                            samples++;
                        }
                    }
                }
                noise = (noise + (blur / samples)) / 2.0f;
            }

            // Center the noise around 0 for neutral brightness
            float grainOffset = (noise - 0.5f) * (strength / 100.0f);

            // Apply to all channels
            int baseIdx = (y * width + x) * channels;
            for (int c = 0; c < channels; c++) {
                float pixel = input[baseIdx + c];
                // Add noise while preserving original brightness
                pixel += grainOffset * 255.0f;
                output[baseIdx + c] = (unsigned char)fmax(0, fmin(255, pixel));
            }
        }
    }

    free(noiseValues);

    return OC_STATUS_OK;
}

OC_STATUS ocularReliefFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float Angle, int Offset) {
    if (Input == NULL || Output == NULL)
        return OC_STATUS_ERR_NULLREFERENCE;
    if (Width <= 0 || Height <= 0)
        return OC_STATUS_ERR_INVALIDPARAMETER;
    if (Offset < 0 || Offset > 255)
        return OC_STATUS_ERR_INVALIDPARAMETER;

    int channels = Stride / Width;

    // Convert angle to radians
    double radian = Angle * M_PI / 180.0;

    // Calculate kernel based on angle
    float kernel[3][3] = { { cos(radian + M_PI), cos(radian + 3.0 * M_PI / 4.0), cos(radian + M_PI / 2.0) },
                           { cos(radian - M_PI / 2.0), 0, cos(radian + M_PI / 2.0) },
                           { cos(radian - M_PI / 2.0), cos(radian - M_PI / 4.0), cos(radian) } };

    // Process each pixel, including borders
    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            for (int c = 0; c < channels; c++) {
                float sum = 0;

                // Apply convolution with mirrored borders
                for (int ky = -1; ky <= 1; ky++) {
                    for (int kx = -1; kx <= 1; kx++) {
                        // Mirror coordinates if they're outside the image bounds
                        int px = x + kx;
                        int py = y + ky;

                        // Mirror boundary handling
                        if (px < 0)
                            px = -px;
                        if (py < 0)
                            py = -py;
                        if (px >= Width)
                            px = 2 * (Width - 1) - px;
                        if (py >= Height)
                            py = 2 * (Height - 1) - py;

                        sum += kernel[ky + 1][kx + 1] * Input[(py * Width + px) * channels + c];
                    }
                }

                // Add offset and clamp result
                int result = (int)(sum + Offset);
                Output[(y * Width + x) * channels + c] = (unsigned char)fmin(fmax(result, 0), 255);
            }
        }
    }

    return OC_STATUS_OK;
}

OC_STATUS ocularKuwaharaFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius) {
    if (!Input || !Output || Width <= 0 || Height <= 0 || Radius < 1)
        return OC_STATUS_ERR_INVALIDPARAMETER;

    int Channels = Stride / Width;
    if (Channels != 1 && Channels != 3)
        return OC_STATUS_ERR_INVALIDPARAMETER;

    // Make sure radius is odd
    if (!(Radius & 1))
        Radius++;
    
    // Clamp radius to valid range
    if (Radius > 10)
        Radius = 10;

    // Pre-calculate kernel bounds
    int kernelSize = Radius * 2 + 1;
    int kernelArea = kernelSize * kernelSize;

    // Process each pixel
    for (int y = 0; y < Height; y++) {
        // Pre-calculate y bounds to reduce checks in inner loop
        int yMin = (y >= Radius) ? -Radius : -y;
        int yMax = (y + Radius < Height) ? Radius : Height - 1 - y;

        for (int x = 0; x < Width; x++) {
            // Pre-calculate x bounds to reduce checks in inner loop
            int xMin = (x >= Radius) ? -Radius : -x;
            int xMax = (x + Radius < Width) ? Radius : Width - 1 - x;

            // Use sum and sum of squares for single-pass variance calculation
            // Q0: top-left, Q1: top-right, Q2: bottom-left, Q3: bottom-right
            int sums[4][3] = { { 0 } };
            int sumSquares[4][3] = { { 0 } };
            int counts[4] = { 0 };

            // Single pass: calculate both sum and sum of squares
            for (int ky = yMin; ky <= yMax; ky++) {
                int py = y + ky;
                int pyStride = py * Stride;
                
                for (int kx = xMin; kx <= xMax; kx++) {
                    int px = x + kx;
                    
                    // Determine quadrant using bit operations (faster than conditionals)
                    // Q0: kx<=0 && ky<=0, Q1: kx>0 && ky<=0, Q2: kx<=0 && ky>0, Q3: kx>0 && ky>0
                    int quadrant = ((kx > 0) << 1) | (ky > 0);
                    
                    int idx = pyStride + px * Channels;
                    counts[quadrant]++;
                    
                    // Accumulate sum and sum of squares in one pass
                    for (int c = 0; c < Channels; c++) {
                        int val = Input[idx + c];
                        sums[quadrant][c] += val;
                        sumSquares[quadrant][c] += val * val;
                    }
                }
            }

            // Calculate means and variances using sum and sum of squares
            int outIdx = y * Stride + x * Channels;
            for (int c = 0; c < Channels; c++) {
                float minVariance = 1e10f;
                float selectedMean = 0.0f;

                for (int q = 0; q < 4; q++) {
                    if (counts[q] > 0) {
                        // Calculate mean: sum / count
                        float mean = (float)sums[q][c] / counts[q];
                        
                        // Calculate variance: E[X^2] - E[X]^2
                        float meanSquared = mean * mean;
                        float variance = ((float)sumSquares[q][c] / counts[q]) - meanSquared;

                        if (variance < minVariance) {
                            minVariance = variance;
                            selectedMean = mean;
                        }
                    }
                }

                // Clamp and convert to byte
                int result = (int)(selectedMean + 0.5f);
                Output[outIdx + c] = (unsigned char)((result < 0) ? 0 : (result > 255) ? 255 : result);
            }
        }
    }

    return OC_STATUS_OK;
}

OC_STATUS ocularPortraitGlowFilter(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                                   OcGlowStyle Style, int GlowRadius, int ExposureBoost, int Strength) {
    if (Input == NULL || Output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    if (Width <= 0 || Height <= 0 || Stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    int Channels = Stride / Width;
    if (Channels != 3 && Channels != 4) {
        return OC_STATUS_ERR_NOTSUPPORTED;
    }

    // Clamp parameters to valid ranges
    GlowRadius = clamp(GlowRadius, 1, 100);
    ExposureBoost = clamp(ExposureBoost, 0, 200);
    Strength = clamp(Strength, 0, 100);

    // Map exposure boost (0-200) to exposure values (0.0 to +2.0)
    float exposure = ((float)ExposureBoost / 100.0f) * 2.0f;

    OcBlendMode blendMode;
    switch (Style) {
        case OC_GLOW_STYLE_CLASSIC:
            blendMode = OC_BLEND_SCREEN;
            break;
        case OC_GLOW_STYLE_MODERN:
            blendMode = OC_BLEND_OVERLAY;
            break;
        case OC_GLOW_STYLE_SUBTLE:
            blendMode = OC_BLEND_SOFTLIGHT;
            break;
        default:
            blendMode = OC_BLEND_SCREEN;
            break;
    }

    // Allocate temporary buffer for blurred image
    size_t imageSize = (size_t)Height * Stride;
    unsigned char* blurredImage = (unsigned char*)malloc(imageSize);
    if (blurredImage == NULL) {
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

    memcpy(blurredImage, Input, imageSize);

    // Apply Gaussian blur
    float sigma = (float)GlowRadius / 2.0f;
    OC_STATUS blurStatus = ocularGaussianBlurFilter(blurredImage, blurredImage, Width, Height, Stride, sigma);
    if (blurStatus != OC_STATUS_OK) {
        free(blurredImage);
        return blurStatus;
    }

    // Apply exposure boost to blurred image
    if (exposure != 0.0f) {
        // Create exposure lookup table
        unsigned char exposureMap[256] = { 0 };
        for (int pixel = 0; pixel < 256; pixel++) {
            exposureMap[pixel] = ClampToByte((float)pixel * pow(2.0, exposure));
        }

        // Apply exposure map to blurred image
        for (int y = 0; y < Height; y++) {
            unsigned char* pBlurred = blurredImage + (y * Stride);
            for (int x = 0; x < Width; x++) {
                pBlurred[0] = exposureMap[pBlurred[0]]; // R
                pBlurred[1] = exposureMap[pBlurred[1]]; // G
                pBlurred[2] = exposureMap[pBlurred[2]]; // B

                pBlurred += Channels;
            }
        }
    }

    // Blend the blurred/exposed image with the original using the selected blend mode
    for (int y = 0; y < Height; y++) {
        const unsigned char* pInput = Input + (y * Stride);
        const unsigned char* pBlurred = blurredImage + (y * Stride);
        unsigned char* pOutput = Output + (y * Stride);

        for (int x = 0; x < Width; x++) {
            int baseR = pInput[0];
            int baseG = pInput[1];
            int baseB = pInput[2];
            int mixR = pBlurred[0];
            int mixG = pBlurred[1];
            int mixB = pBlurred[2];

            int resR, resG, resB;
            layerBlend(baseR, baseG, baseB, mixR, mixG, mixB, &resR, &resG, &resB, blendMode, Strength);

            pOutput[0] = (unsigned char)resR;
            pOutput[1] = (unsigned char)resG;
            pOutput[2] = (unsigned char)resB;

            pInput += Channels;
            pBlurred += Channels;
            pOutput += Channels;
        }
    }

    free(blurredImage);

    return OC_STATUS_OK;
}

// Check if point is on surface (Glass Tiles filter)
static inline int isOnSurface(float u, float v, int Width, int Height) {
    return (u >= 0.0f && u <= (float)(Width - 1) && v >= 0.0f && v <= (float)(Height - 1));
}

OC_STATUS ocularGlassTilesFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float rotation,
                                 int tileSize, float curvature, int quality, OcEdgeMode edgeMode) {

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

    // Validate filter-specific parameters
    if (rotation < -45.0f || rotation > 45.0f) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }
    if (tileSize < 2 || tileSize > 200) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }
    if (curvature < -20.0f || curvature > 20.0f) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }
    if (quality < 1 || quality > 5) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    // Quality 1–5 → 1, 4, 7, 10, 13 samples (no antialiasing at 1, supersampling above 1)
    int numSamples = 1 + (quality - 1) * 3;

    // Pre-compute settings
    float halfWidth = (float)Width / 2.0f;
    float halfHeight = (float)Height / 2.0f;

    // Rotation
    float rotRad = -rotation * M_PI / 180.0f;
    float sinRot = sinf(rotRad);
    float cosRot = cosf(rotRad);

    // Adjust tile size as a ratio of the smaller dimension of image
    if (Width < Height) {
        tileSize = (tileSize * Width) * 0.005f;
    } else {
        tileSize = (tileSize * Height) * 0.005f;
    }

    // Tile scale for wave function
    float tileScale = (float)M_PI / (float)tileSize;

    float adjustedCurvature = curvature * curvature / 10.0f * (curvature >= 0.0f ? 1.0f : -1.0f);

    // Precalculated supersampling offsets (reusable rotated-grid table)
    float xOffsets[OC_SUPERSAMPLE_MAX_SAMPLES];
    float yOffsets[OC_SUPERSAMPLE_MAX_SAMPLES];
    ocularSupersampleOffsets(numSamples, sinRot, cosRot, xOffsets, yOffsets);

    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            int pPos = y * Stride + x * channels;

            // Get original pixel for EDGE_ORIGINAL mode
            unsigned char originalPixel[4];
            for (int c = 0; c < channels; c++) {
                originalPixel[c] = Input[pPos + c];
            }

            // Translate to center
            float i = (float)x - halfWidth;
            float j = (float)y - halfHeight;

            // Accumulator for anti-aliased result
            float accumR = 0.0f, accumG = 0.0f, accumB = 0.0f;

            // Supersampling loop (1 = no AA, 2–5 = 4/7/10/13 samples)
            for (int p = 0; p < numSamples; p++) {
                // Initial coordinates after applying supersampling offsets
                float initialU = i + xOffsets[p];
                float initialV = j - yOffsets[p];

                // Rotate coordinates to align with the effect's orientation
                float rotatedS = cosRot * initialU + sinRot * initialV;
                float rotatedT = -sinRot * initialU + cosRot * initialV;

                // Apply wave function: tan gives sharp tile edges (glass tiles)
                float nS = rotatedS * tileScale;
                float nT = rotatedT * tileScale;
                float functionS = tanf(nS);
                float functionT = tanf(nT);
                /* Clamp to avoid inf/nan when n is near ±π/2 */
                if (functionS > 20.0f)
                    functionS = 20.0f;
                else if (functionS < -20.0f)
                    functionS = -20.0f;
                if (functionT > 20.0f)
                    functionT = 20.0f;
                else if (functionT < -20.0f)
                    functionT = -20.0f;

                // Apply curvature transformation to create the tile effect
                float transformedS = rotatedS + adjustedCurvature * functionS;
                float transformedT = rotatedT + adjustedCurvature * functionT;

                // Rotate back to the original coordinate space
                float finalU = cosRot * transformedS - sinRot * transformedT;
                float finalV = sinRot * transformedS + cosRot * transformedT;

                // Translate back to image coordinates
                float preliminaryX = halfWidth + finalU;
                float preliminaryY = halfHeight + finalV;

                // Get sample based on edge behavior
                unsigned char samplePixel[4] = { 0, 0, 0, 0 };

                if (isOnSurface(preliminaryX, preliminaryY, Width, Height)) {
                    // Fast path - direct sample
                    int floorX = (int)preliminaryX;
                    int floorY = (int)preliminaryY;
                    int samplePos = floorY * Stride + floorX * channels;
                    for (int c = 0; c < channels; c++) {
                        samplePixel[c] = Input[samplePos + c];
                    }
                } else {
                    // Edge behavior
                    GetPixelBilinear(Input, Width, Height, Stride, preliminaryX, preliminaryY, edgeMode, samplePixel, channels);
                }

                // Accumulate samples
                if (channels == 1) {
                    accumR += (float)samplePixel[0];
                } else {
                    accumR += (float)samplePixel[0];
                    accumG += (float)samplePixel[1];
                    accumB += (float)samplePixel[2];
                }
            }

            // Average and write output
            float invSamples = 1.0f / (float)numSamples;
            if (channels == 1) {
                Output[pPos] = (unsigned char)(accumR * invSamples + 0.5f);
            } else {
                Output[pPos + 0] = (unsigned char)(accumR * invSamples + 0.5f);
                Output[pPos + 1] = (unsigned char)(accumG * invSamples + 0.5f);
                Output[pPos + 2] = (unsigned char)(accumB * invSamples + 0.5f);
            }
        }
    }

    return OC_STATUS_OK;
}

/* Compute deterministic coordinate offset from seed for reproducible marble patterns. */
static inline void marbleSeedOffsets(int seed, float* outX, float* outY) {
    if (seed == 0) {
        *outX = 0.0f;
        *outY = 0.0f;
        return;
    }
    unsigned int s = (unsigned int)seed;
    *outX = (float)((s * 1103515245u + 12345u) & 0x7FFFFFFFu) / 2147483647.0f * 1000.0f;
    *outY = (float)((s * 134775813u + 1u) & 0x7FFFFFFFu) / 2147483647.0f * 1000.0f;
}

OC_STATUS ocularMarbleFilter(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                             float scale, float turbulence, int quality, OcEdgeMode edgeMode, unsigned int seed) {

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

    /* Validate filter-specific parameters */
    if (scale < 0.0f || scale > 100.0f) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }
    if (turbulence < 0.0f || turbulence > 1.0f) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }
    if (quality < 1 || quality > 5) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    // Quality 1–5 → 1, 4, 7, 10, 13 samples
    int numSamples = 1 + (quality - 1) * 3;

    /* Scale as a fraction of the image's smallest dimension */
    int minDim = (Width < Height) ? Width : Height;
    float effectiveScale = (scale * 0.01f) * (float)minDim;
    if (effectiveScale < 0.001f)
        effectiveScale = 1.0f;
    float xScale = effectiveScale;
    float yScale = effectiveScale;

    // Pre-compute sin/cos displacement tables
    const float TWO_PI = 6.283185307179586f;
    float sinTable[256];
    float cosTable[256];
    for (int i = 0; i < 256; i++) {
        float angle = TWO_PI * (float)i / 256.0f * turbulence;
        sinTable[i] = -yScale * sinf(angle);
        cosTable[i] = yScale * cosf(angle);
    }

    // Seed-based coordinate offset for reproducible patterns
    float seedX, seedY;
    marbleSeedOffsets((int)seed, &seedX, &seedY);

    // Supersampling offsets (use sin=0, cos=1)
    float xOffsets[OC_SUPERSAMPLE_MAX_SAMPLES];
    float yOffsets[OC_SUPERSAMPLE_MAX_SAMPLES];
    ocularSupersampleOffsets(numSamples, 0.0f, 1.0f, xOffsets, yOffsets);

    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            int pPos = y * Stride + x * channels;

            float accumR = 0.0f, accumG = 0.0f, accumB = 0.0f;

            for (int p = 0; p < numSamples; p++) {
                // Sample coordinate with supersampling offset
                float px = (float)x + xOffsets[p];
                float py = (float)y + yOffsets[p];

                // Displacement map
                float nx = px / xScale + seedX;
                float ny = py / yScale + seedY;
                float noiseVal = perlinNoise2D(nx, ny);
                int displacement = (int)(127.0f * (1.0f + noiseVal));
                if (displacement < 0)
                    displacement = 0;
                else if (displacement > 255)
                    displacement = 255;

                // Inverse transform
                float srcX = px + sinTable[displacement];
                float srcY = py + cosTable[displacement];

                unsigned char samplePixel[4] = { 0, 0, 0, 0 };
                if (srcX >= 0.0f && srcX <= (float)(Width - 1) && srcY >= 0.0f && srcY <= (float)(Height - 1)) {
                    int floorX = (int)srcX;
                    int floorY = (int)srcY;
                    int samplePos = floorY * Stride + floorX * channels;
                    for (int c = 0; c < channels; c++) {
                        samplePixel[c] = Input[samplePos + c];
                    }
                } else {
                    GetPixelBilinear(Input, Width, Height, Stride, srcX, srcY, edgeMode, samplePixel, channels);
                }

                if (channels == 1) {
                    accumR += (float)samplePixel[0];
                } else {
                    accumR += (float)samplePixel[0];
                    accumG += (float)samplePixel[1];
                    accumB += (float)samplePixel[2];
                }
            }

            float invSamples = 1.0f / (float)numSamples;
            if (channels == 1) {
                Output[pPos] = (unsigned char)(accumR * invSamples + 0.5f);
            } else {
                Output[pPos + 0] = (unsigned char)(accumR * invSamples + 0.5f);
                Output[pPos + 1] = (unsigned char)(accumG * invSamples + 0.5f);
                Output[pPos + 2] = (unsigned char)(accumB * invSamples + 0.5f);
            }
        }
    }

    return OC_STATUS_OK;
}

/* ---- helpers for the wind filter ----------------------------------------- */

/* Salted deterministic hash -> [0,1) */
static inline float ocWindHash01(int x, int y, unsigned int stream, unsigned int seed) {
    unsigned int h = (unsigned int)x * 374761393u + (unsigned int)y * 668265263u
                   + stream * 2246822519u + seed * 3266489917u;
    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= h >> 16;
    return (float)(h & 0xFFFFFFu) * (1.0f / 16777216.0f);
}

static inline int ocWindClampX(int x, int Width) {
    return x < 0 ? 0 : (x >= Width ? Width - 1 : x);
}

/* Luma in [0,1] of pixel x in a row, with clamped borders. Assumes R,G,B order. */
static inline float ocWindLuma(const unsigned char* row, int x, int Width, int channels) {
    const unsigned char* p = row + (size_t)ocWindClampX(x, Width) * channels;
    if (channels == 1) return p[0] * (1.0f / 255.0f);
    return (0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]) * (1.0f / 255.0f);
}

OC_STATUS ocularWindFilter(const unsigned char* Input, unsigned char* Output,
                           int Width, int Height, int Stride,
                           int Method, int FromRight, unsigned int Seed) {

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

    if (Method < OC_WIND_METHOD_WIND || Method > OC_WIND_METHOD_STAGGER ||
        FromRight < 0 || FromRight > 1) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    /* Longest streak or stagger shift in pixels */
    const int lmax = (Method == OC_WIND_METHOD_WIND)  ? 16 :
                     (Method == OC_WIND_METHOD_BLAST) ? 40 : 12;
    const int d = FromRight ? -1 : 1;                  /* downwind step: left wind -> +x */
    const float strength = (Method == OC_WIND_METHOD_BLAST) ? 8.0f : 4.0f;

    int x, y, c, k;
    for (y = 0; y < Height; y++) {
        const unsigned char* row = Input + (size_t)y * Stride;
        unsigned char* dst = Output + (size_t)y * Stride;

        for (x = 0; x < Width; x++) {
            const unsigned char* px = row + (size_t)x * channels;
            unsigned char* out = dst + (size_t)x * channels;

            if (Method == OC_WIND_METHOD_STAGGER) {
                /* Row split into segments of 8..40 px, each shifted downwind by 0..lmax px */
                int segLen = 8 + (int)(ocWindHash01(0, y, 41u, Seed) * 32.0f);
                int phase  = (int)(ocWindHash01(1, y, 42u, Seed) * (float)segLen);
                int a = x * d + phase;
                int seg = a / segLen;
                if (a % segLen < 0) seg--;                                  /* floor division */
                float r = ocWindHash01(seg, y, 43u, Seed);
                int shift = (int)(r * r * (float)lmax);
                const unsigned char* sp = row + (size_t)ocWindClampX(x - d * shift, Width) * channels;
                for (c = 0; c < channels; c++) out[c] = sp[c];
                continue;
            }

            /* Wind / Blast: find the strongest streak reaching this pixel from upwind */
            float bestW = 0.0f;
            int bestS = x;
            for (k = 1; k <= lmax; k++) {
                int s = x - d * k;
                float r = ocWindHash01(s, y, 44u, Seed);
                float len = r * r * (float)lmax;
                if (len < 1.0f) len = 1.0f;
                if ((float)k > len) continue;

                float edge = fabsf(ocWindLuma(row, s, Width, channels) - ocWindLuma(row, s - d, Width, channels)) * strength;
                if (edge > 1.0f) edge = 1.0f;
                float wgt = edge * (1.0f - (float)k / (len + 1.0f));
                if (wgt > bestW) {
                    bestW = wgt;
                    bestS = s;
                }
            }

            if (bestW > 0.0f) {
                const unsigned char* sp = row + (size_t)ocWindClampX(bestS, Width) * channels;
                for (c = 0; c < channels; c++) {
                    float v = (float)px[c] + ((float)sp[c] - (float)px[c]) * bestW;
                    out[c] = (unsigned char)(v < 0.0f ? 0.0f : (v > 255.0f ? 255.0f : v + 0.5f));
                }
            } else {
                for (c = 0; c < channels; c++) out[c] = px[c];
            }
        }
    }

    return OC_STATUS_OK;
}

/* ---- helpers for the extrude filter -------------------------------------- */
/* ocWindHash01 (wind filter helpers above) is reused for the random cell heights. */

static inline float ocExtrudeClamp01(float v) {
    return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

/* Parameter interval t in [0,1] for which |p - (m + o*t)| <= half; returns 0 if none. */
static inline int ocExtrudeInterval(float p, float m, float o, float half, float* lo, float* hi) {
    if (fabsf(o) < 1e-6f) {
        if (fabsf(p - m) <= half) { *lo = 0.0f; *hi = 1.0f; return 1; }
        return 0;
    }
    float a = (p - m - half) / o, b = (p - m + half) / o;
    float l = fmaxf(fminf(a, b), 0.0f);
    float h = fminf(fmaxf(a, b), 1.0f);
    if (l <= h) { *lo = l; *hi = h; return 1; }
    return 0;
}

static inline void ocExtrudeStore(unsigned char* out, const float* col, float k, int channels) {
    for (int c = 0; c < channels; c++) {
        out[c] = (unsigned char)(ocExtrudeClamp01(col[c] * k) * 255.0f + 0.5f);
    }
}

OC_STATUS ocularExtrudeFilter(const unsigned char* Input, unsigned char* Output,
                              int Width, int Height, int Stride,
                              int Type, int Size, int Depth,
                              int DepthMode, int SolidFront, int MaskIncomplete,
                              unsigned int Seed) {

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

    if (Type < OC_EXTRUDE_BLOCKS || Type > OC_EXTRUDE_PYRAMIDS ||
        DepthMode < OC_EXTRUDE_DEPTH_RANDOM || DepthMode > OC_EXTRUDE_DEPTH_LEVEL_BASED ||
        Size < 2 || Size > 255 ||
        Depth < 1 || Depth > 255) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    const int cs = Size;
    const int gw = (Width + cs - 1) / cs;
    const int gh = (Height + cs - 1) / cs;
    const size_t cells = (size_t)gw * (size_t)gh;

    float* colors  = (float*)malloc(cells * (size_t)channels * sizeof(float));
    float* heights = (float*)malloc(cells * sizeof(float));
    if (colors == NULL || heights == NULL) {
        free(colors);
        free(heights);
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

    int x, y, c, gx, gy;

    /* 1. Per-cell average color (sparse sampling) and height */
    const int step = (cs / 8) > 1 ? (cs / 8) : 1;
    for (gy = 0; gy < gh; gy++) {
        for (gx = 0; gx < gw; gx++) {
            int x0 = gx * cs, x1 = x0 + cs < Width ? x0 + cs : Width;
            int y0 = gy * cs, y1 = y0 + cs < Height ? y0 + cs : Height;
            float acc[3] = { 0.0f, 0.0f, 0.0f };
            float cnt = 0.0f;
            for (y = y0; y < y1; y += step) {
                const unsigned char* row = Input + (size_t)y * Stride;
                for (x = x0; x < x1; x += step) {
                    for (c = 0; c < channels; c++) acc[c] += row[x * channels + c] * (1.0f / 255.0f);
                    cnt += 1.0f;
                }
            }
            size_t ci = (size_t)gy * gw + gx;
            for (c = 0; c < channels; c++) colors[ci * channels + c] = cnt > 0.0f ? acc[c] / cnt : 0.0f;

            if (DepthMode == OC_EXTRUDE_DEPTH_LEVEL_BASED) {
                const float* a = colors + ci * channels;
                float l = (channels == 1) ? a[0] : (0.299f * a[0] + 0.587f * a[1] + 0.114f * a[2]);
                heights[ci] = ocExtrudeClamp01(l);
            } else {
                heights[ci] = ocWindHash01(gx, gy, 21u, Seed);
            }
        }
    }

    /* 2. Geometry constants */
    const float bcx = (float)Width / 2.0f, bcy = (float)Height / 2.0f;
    float halfDiag = hypotf((float)Width, (float)Height) / 2.0f;
    if (halfDiag < 1.0f) halfDiag = 1.0f;
    const float reach = (float)Depth * 0.25f;
    const int kRange = (int)ceilf(reach / (float)cs) + 1;
    const float half = (float)cs / 2.0f;

    /* 3. Render */
    for (y = 0; y < Height; y++) {
        const unsigned char* row = Input + (size_t)y * Stride;
        unsigned char* dst = Output + (size_t)y * Stride;
        for (x = 0; x < Width; x++) {
            const unsigned char* px = row + (size_t)x * channels;
            unsigned char* out = dst + (size_t)x * channels;
            for (c = 0; c < channels; c++) out[c] = px[c];       /* default: unchanged */

            const float fx = (float)x + 0.5f, fy = (float)y + 0.5f;
            gx = x / cs;
            gy = y / cs;

            if (Type == OC_EXTRUDE_PYRAMIDS) {
                int incomplete = ((gx + 1) * cs > Width) || ((gy + 1) * cs > Height);
                if (MaskIncomplete && incomplete) continue;

                size_t ci = (size_t)gy * gw + gx;
                const float* col = colors + ci * channels;
                float h = heights[ci];

                float u = fx - ((float)(gx * cs) + half);
                float v = fy - ((float)(gy * cs) + half);
                float hgt = h * (float)Depth / 255.0f * (float)cs;
                float nx, ny;
                if (fabsf(u) >= fabsf(v)) { nx = (u >= 0.0f ? 1.0f : -1.0f) * hgt; ny = 0.0f; }
                else                      { nx = 0.0f; ny = (v >= 0.0f ? 1.0f : -1.0f) * hgt; }
                float norm = sqrtf(nx * nx + ny * ny + half * half);
                float lambert = (nx * -0.5f + ny * -0.5f + half * 0.70710678f) / norm;
                if (lambert < 0.0f) lambert = 0.0f;
                ocExtrudeStore(out, col, 0.35f + 0.9f * lambert, channels);
                continue;
            }

            /* Blocks: find the tallest candidate block covering this pixel */
            float bestH = 0.0f;
            int bestCx = 0, bestCy = 0, bestFace = -1;
            int dx, dy;
            for (dy = -kRange; dy <= kRange; dy++) {
                int cy = gy + dy;
                if (cy < 0 || cy >= gh) continue;
                for (dx = -kRange; dx <= kRange; dx++) {
                    int cx = gx + dx;
                    if (cx < 0 || cx >= gw) continue;
                    if (MaskIncomplete && (((cx + 1) * cs > Width) || ((cy + 1) * cs > Height))) continue;

                    float h = heights[(size_t)cy * gw + cx];
                    if (bestFace >= 0 && bestH >= h) continue;

                    float mx = (float)(cx * cs) + half, my = (float)(cy * cs) + half;
                    float off = reach * h / halfDiag;
                    float ox = (mx - bcx) * off, oy = (my - bcy) * off;

                    float ixl, ixh, iyl, iyh;
                    if (!ocExtrudeInterval(fx, mx, ox, half, &ixl, &ixh)) continue;
                    if (!ocExtrudeInterval(fy, my, oy, half, &iyl, &iyh)) continue;
                    float lo = fmaxf(ixl, iyl), hi = fminf(ixh, iyh);
                    if (lo > hi) continue;

                    int face;
                    if (hi >= 1.0f)       face = 0;                          /* front */
                    else if (ixh < iyh)   face = (ox > 0.0f) ? 1 : 2;        /* left / right side */
                    else                  face = (oy > 0.0f) ? 3 : 4;        /* top / bottom side */

                    bestH = h; bestCx = cx; bestCy = cy; bestFace = face;
                }
            }
            if (bestFace < 0) continue;

            size_t ci = (size_t)bestCy * gw + bestCx;
            const float* col = colors + ci * channels;
            if (bestFace == 0) {
                if (SolidFront) {
                    ocExtrudeStore(out, col, 1.0f, channels);
                } else {
                    float mx = (float)(bestCx * cs) + half, my = (float)(bestCy * cs) + half;
                    float off = reach * bestH / halfDiag;
                    int sx = (int)floorf(fx - (mx - bcx) * off);
                    int sy = (int)floorf(fy - (my - bcy) * off);
                    int rx0 = bestCx * cs, ry0 = bestCy * cs;
                    sx = sx < rx0 ? rx0 : (sx > rx0 + cs - 1 ? rx0 + cs - 1 : sx);
                    sy = sy < ry0 ? ry0 : (sy > ry0 + cs - 1 ? ry0 + cs - 1 : sy);
                    sx = sx < 0 ? 0 : (sx > Width  - 1 ? Width  - 1 : sx);   /* partial cells at the edge */
                    sy = sy < 0 ? 0 : (sy > Height - 1 ? Height - 1 : sy);
                    const unsigned char* sp = Input + (size_t)sy * Stride + (size_t)sx * channels;
                    for (c = 0; c < channels; c++) out[c] = sp[c];
                }
            } else {
                float k = (bestFace == 1) ? 0.85f : (bestFace == 2) ? 0.55f : (bestFace == 3) ? 0.95f : 0.45f;
                ocExtrudeStore(out, col, k, channels);
            }
        }
    }

    free(colors);
    free(heights);
    return OC_STATUS_OK;
}
