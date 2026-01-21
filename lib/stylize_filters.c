/**
 * @file: stylize_filters.c
 * @author Warren Galyen
 * Created: 10-4-2025
 * Last Updated: 10-4-2025
 * Last update: migrated oil paint, frosted glass, film grain,
 *              relief filters from ocular.c
 *
 * @brief Stylize filter implementations
 */

 #include "stylize_filters.h"
 #include "core.h"
 #include "util.h"
 #include "blur_filters.h"

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