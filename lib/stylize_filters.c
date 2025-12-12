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

OC_STATUS ocularWindFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                            OcWindTechnique technique, OcWindDirection direction) {
    
    if (Input == NULL || Output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    if (Width <= 0 || Height <= 0 || Stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }
    
    int Channels = Stride / Width;
    
    // First, copy input to output as a base
    memcpy(Output, Input, Height * Stride);
    
    // Define wind parameters based on technique
    int windLength, windStrength, threshold;
    float fadeRate;
    
    switch(technique) {
        case OC_WIND_TECHNIQUE_WIND:
            windLength = 15;       // Longer, thinner streaks
            windStrength = 1;
            fadeRate = 0.92f;      // Very gradual fade for longer streaks
            threshold = 20;        // More selective edge detection
            break;
        case OC_WIND_TECHNIQUE_BLAST:
            windLength = 35;       // Much longer, thinner streaks
            windStrength = 1;
            fadeRate = 0.88f;      // Gradual fade for long streaks
            threshold = 15;        // Moderate edge detection
            break;
        case OC_WIND_TECHNIQUE_STAGGER:
            windLength = 12;       // Longer chunks
            windStrength = 1;
            fadeRate = 1.0f;       // No fade within chunks
            threshold = 18;        // More selective edge detection
            break;
        default:
            windLength = 15;
            windStrength = 1;
            fadeRate = 0.92f;
            threshold = 20;
            break;
    }
    
    // Allocate temporary buffer for edge detection
    int* edgeMap = (int*)calloc(Width * Height, sizeof(int));
    if (edgeMap == NULL) {
        return OC_STATUS_ERR_OUTOFMEMORY;
    }
    
    // Detect edges where brightness increases sharply in wind direction
    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            int idx = (y * Width + x) * Channels;
            
            // Calculate luminance of current pixel
            int lum = (Input[idx] * 299 + Input[idx + 1] * 587 + Input[idx + 2] * 114) / 1000;
            
            // Check neighbor in direction opposite to wind
            int neighborX;
            if (direction == OC_WIND_FROM_LEFT) {
                // Wind from left, check pixel to the left
                neighborX = (x > 0) ? x - 1 : x;
            } else {
                // Wind from right, check pixel to the right  
                neighborX = (x < Width - 1) ? x + 1 : x;
            }
            
            int neighborIdx = (y * Width + neighborX) * Channels;
            int neighborLum = (Input[neighborIdx] * 299 + Input[neighborIdx + 1] * 587 + Input[neighborIdx + 2] * 114) / 1000;
            
            // Detect where brightness increases sharply (bright edge facing wind)
            int diff = lum - neighborLum;
            
            // Mark as edge if brightness increases significantly
            if (diff > threshold) {
                edgeMap[y * Width + x] = 255;  // Mark as edge
            }
        }
    }
    
    // Apply wind effect
    for (int pass = 0; pass < windStrength; pass++) {
        for (int y = 0; y < Height; y++) {
            // For stagger effect, skip alternate rows
            if (technique == OC_WIND_TECHNIQUE_STAGGER && (y % 2) != (pass % 2)) {
                continue;
            }
            
            // Determine scan direction
            int startX, endX, stepX;
            if (direction == OC_WIND_FROM_LEFT) {
                startX = 1;
                endX = Width;
                stepX = 1;
            } else {
                startX = Width - 2;
                endX = -1;
                stepX = -1;
            }
            
            for (int x = startX; x != endX; x += stepX) {
                int edgeStrength = edgeMap[y * Width + x];
                if (edgeStrength == 0) {
                    continue;
                }
                
                int sourceIdx = (y * Width + x) * Channels;
                
                // Get the source pixel values
                unsigned char srcR = Output[sourceIdx];
                unsigned char srcG = Output[sourceIdx + 1];
                unsigned char srcB = Output[sourceIdx + 2];
                
                // Get source luminance
                int srcLum = (srcR * 299 + srcG * 587 + srcB * 114) / 1000;
                
                // Create the wind streak
                float intensity = 1.0f;
                
                for (int len = 1; len < windLength; len++) {
                    int targetX = x + (stepX * len);
                    
                    // Check boundaries
                    if (targetX < 0 || targetX >= Width) {
                        break;
                    }
                    
                    int targetIdx = (y * Width + targetX) * Channels;
                    
                    // Get target pixel values
                    unsigned char targetR = Output[targetIdx];
                    unsigned char targetG = Output[targetIdx + 1];
                    unsigned char targetB = Output[targetIdx + 2];
                    
                    // Calculate target luminance
                    int targetLum = (targetR * 299 + targetG * 587 + targetB * 114) / 1000;
                    
                    // Only extend bright pixels over darker ones (Photoshop behavior)
                    if (srcLum > targetLum) {
                        // Apply intensity fade
                        intensity *= fadeRate;
                        if (intensity < 0.05f) {
                            break;
                        }
                        
                        // For thinner streaks, use more selective blending
                        // Only blend if the intensity is significant
                        if (intensity > 0.2f) {
                            // Blend source over target with intensity
                            int newR = (int)(srcR * intensity + targetR * (1.0f - intensity));
                            int newG = (int)(srcG * intensity + targetG * (1.0f - intensity));
                            int newB = (int)(srcB * intensity + targetB * (1.0f - intensity));
                            
                            Output[targetIdx] = (unsigned char)clamp(newR, 0, 255);
                            Output[targetIdx + 1] = (unsigned char)clamp(newG, 0, 255);
                            Output[targetIdx + 2] = (unsigned char)clamp(newB, 0, 255);
                        }
                    }
                    
                    if (Channels == 4) {
                        Output[targetIdx + 3] = Input[targetIdx + 3];
                    }
                }
            }
        }
    }
    
    free(edgeMap);
    
    return OC_STATUS_OK;
}