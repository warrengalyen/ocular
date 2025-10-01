/**
 * @file: fft.c
 * @author Warren Galyen
 * Created: 10-1-2024
 * Last Updated: 10-1-2024
 * Last update: initial implementation
 *
 * @brief Fast Fourier Transform implementation for image processing
 */

#include "fft.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Helper function to check if a number is a power of 2
static bool isPowerOfTwo(int n) {
    return n > 0 && (n & (n - 1)) == 0;
}

// Helper function to find the next power of 2
static int nextPowerOfTwo(int n) {
    int power = 1;
    while (power < n) {
        power <<= 1;
    }
    return power;
}

// Bit-reversal permutation for FFT
static void bitReverse(OcComplex* data, int n) {
    int j = 0;
    for (int i = 1; i < n; i++) {
        int bit = n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        
        if (i < j) {
            // Swap data[i] and data[j]
            OcComplex temp = data[i];
            data[i] = data[j];
            data[j] = temp;
        }
    }
}

// Core 1D FFT implementation using Cooley-Tukey algorithm
OC_STATUS ocularFFT(OcComplex* data, int n, bool inverse) {
    if (data == NULL || n <= 0) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    
    if (!isPowerOfTwo(n)) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }
    
    // Bit-reversal permutation
    bitReverse(data, n);
    
    // FFT computation
    for (int len = 2; len <= n; len <<= 1) {
        double angle = (inverse ? 2.0 : -2.0) * M_PI / len;
        OcComplex wlen = {cos(angle), sin(angle)};
        
        for (int i = 0; i < n; i += len) {
            OcComplex w = {1.0, 0.0};
            
            for (int j = 0; j < len / 2; j++) {
                OcComplex u = data[i + j];
                OcComplex v = {
                    data[i + j + len / 2].real * w.real - data[i + j + len / 2].imag * w.imag,
                    data[i + j + len / 2].real * w.imag + data[i + j + len / 2].imag * w.real
                };
                
                data[i + j] = (OcComplex){u.real + v.real, u.imag + v.imag};
                data[i + j + len / 2] = (OcComplex){u.real - v.real, u.imag - v.imag};
                
                // Update w = w * wlen
                OcComplex temp_w = {
                    w.real * wlen.real - w.imag * wlen.imag,
                    w.real * wlen.imag + w.imag * wlen.real
                };
                w = temp_w;
            }
        }
    }
    
    // Normalize for inverse FFT
    if (inverse) {
        for (int i = 0; i < n; i++) {
            data[i].real /= n;
            data[i].imag /= n;
        }
    }
    
    return OC_STATUS_OK;
}

// 2D FFT implementation
OC_STATUS ocularFFT2D(OcComplex* data, int width, int height, bool inverse) {
    if (data == NULL || width <= 0 || height <= 0) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    
    // FFT on rows
    for (int y = 0; y < height; y++) {
        OC_STATUS status = ocularFFT(&data[y * width], width, inverse);
        if (status != OC_STATUS_OK) {
            return status;
        }
    }
    
    // Allocate temporary column buffer
    OcComplex* column = (OcComplex*)malloc(height * sizeof(OcComplex));
    if (column == NULL) {
        return OC_STATUS_ERR_OUTOFMEMORY;
    }
    
    // FFT on columns
    for (int x = 0; x < width; x++) {
        // Extract column
        for (int y = 0; y < height; y++) {
            column[y] = data[y * width + x];
        }
        
        // Apply FFT to column
        OC_STATUS status = ocularFFT(column, height, inverse);
        if (status != OC_STATUS_OK) {
            free(column);
            return status;
        }
        
        // Put column back
        for (int y = 0; y < height; y++) {
            data[y * width + x] = column[y];
        }
    }
    
    free(column);
    return OC_STATUS_OK;
}

// Create FFT filter kernel based on parameters
OC_STATUS ocularCreateFFTFilterKernel(OcFFTFilterParams* params, int width, int height, OcComplex** kernel) {
    if (params == NULL || kernel == NULL || width <= 0 || height <= 0) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    
    *kernel = (OcComplex*)calloc(width * height, sizeof(OcComplex));
    if (*kernel == NULL) {
        return OC_STATUS_ERR_OUTOFMEMORY;
    }
    
    int centerX = width / 2;
    int centerY = height / 2;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Calculate distance from center (normalized frequency)
            float dx = (float)(x - centerX) / width;
            float dy = (float)(y - centerY) / height;
            float freq = sqrt(dx * dx + dy * dy);
            
            float filterValue = 0.0f;
            
            switch (params->filterType) {
                case OC_FFT_LOWPASS:
                    if (freq <= params->cutoffFreq) {
                        filterValue = 1.0f;
                    } else {
                        // Smooth rolloff
                        float rolloff = (freq - params->cutoffFreq) * params->sharpness;
                        filterValue = exp(-rolloff * rolloff);
                    }
                    break;
                    
                case OC_FFT_HIGHPASS:
                    if (freq >= params->cutoffFreq) {
                        filterValue = 1.0f;
                    } else {
                        // Smooth rolloff
                        float rolloff = (params->cutoffFreq - freq) * params->sharpness;
                        filterValue = 1.0f - exp(-rolloff * rolloff);
                    }
                    break;
                    
                case OC_FFT_BANDPASS:
                    if (freq >= params->cutoffFreq && freq <= params->cutoffFreq2) {
                        filterValue = 1.0f;
                    } else {
                        float rolloff1 = (params->cutoffFreq - freq) * params->sharpness;
                        float rolloff2 = (freq - params->cutoffFreq2) * params->sharpness;
                        filterValue = exp(-rolloff1 * rolloff1) * exp(-rolloff2 * rolloff2);
                    }
                    break;
                    
                case OC_FFT_BANDSTOP:
                    if (freq >= params->cutoffFreq && freq <= params->cutoffFreq2) {
                        float rolloff = fmin((freq - params->cutoffFreq), (params->cutoffFreq2 - freq)) * params->sharpness;
                        filterValue = 1.0f - exp(-rolloff * rolloff);
                    } else {
                        filterValue = 1.0f;
                    }
                    break;
                    
                case OC_FFT_CUSTOM:
                    // Use custom kernel if provided
                    if (params->customKernel != NULL && params->kernelSize > 0) {
                        int kernelX = (x * params->kernelSize) / width;
                        int kernelY = (y * params->kernelSize) / height;
                        if (kernelX < params->kernelSize && kernelY < params->kernelSize) {
                            filterValue = params->customKernel[kernelY * params->kernelSize + kernelX];
                        }
                    }
                    break;
            }
            
            (*kernel)[y * width + x].real = filterValue;
            (*kernel)[y * width + x].imag = 0.0f;
        }
    }
    
    return OC_STATUS_OK;
}

// Apply FFT kernel to image FFT
OC_STATUS ocularApplyFFTKernel(OcComplex* imageFFT, OcComplex* kernel, int width, int height) {
    if (imageFFT == NULL || kernel == NULL || width <= 0 || height <= 0) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    
    for (int i = 0; i < width * height; i++) {
        // Complex multiplication: (a + bi) * (c + di) = (ac - bd) + (ad + bc)i
        float real = imageFFT[i].real * kernel[i].real - imageFFT[i].imag * kernel[i].imag;
        float imag = imageFFT[i].real * kernel[i].imag + imageFFT[i].imag * kernel[i].real;
        
        imageFFT[i].real = real;
        imageFFT[i].imag = imag;
    }
    
    return OC_STATUS_OK;
}

// Main FFT filter function
OC_STATUS ocularFFTFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, OcFFTFilterParams* params) {
    if (Input == NULL || Output == NULL || params == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    
    if (Width <= 0 || Height <= 0 || Stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }
    
    int Channels = Stride / Width;
    if (Channels != 1 && Channels != 3 && Channels != 4) {
        return OC_STATUS_ERR_NOTSUPPORTED;
    }
    
    // Find next power of 2 for FFT dimensions
    int fftWidth = nextPowerOfTwo(Width);
    int fftHeight = nextPowerOfTwo(Height);
    
    // Allocate FFT buffers
    OcComplex* imageFFT = (OcComplex*)calloc(fftWidth * fftHeight, sizeof(OcComplex));
    OcComplex* kernel = NULL;
    
    if (imageFFT == NULL) {
        return OC_STATUS_ERR_OUTOFMEMORY;
    }
    
    OC_STATUS status = OC_STATUS_OK;
    
    // Create filter kernel
    status = ocularCreateFFTFilterKernel(params, fftWidth, fftHeight, &kernel);
    if (status != OC_STATUS_OK) {
        free(imageFFT);
        return status;
    }
    
    // Process each channel separately
    for (int c = 0; c < Channels; c++) {
        // Copy image data to FFT buffer (zero-padded)
        memset(imageFFT, 0, fftWidth * fftHeight * sizeof(OcComplex));
        
        for (int y = 0; y < Height; y++) {
            for (int x = 0; x < Width; x++) {
                int srcIdx = y * Stride + x * Channels + c;
                int dstIdx = y * fftWidth + x;
                imageFFT[dstIdx].real = (float)Input[srcIdx] / 255.0f;
                imageFFT[dstIdx].imag = 0.0f;
            }
        }
        
        // Forward FFT
        status = ocularFFT2D(imageFFT, fftWidth, fftHeight, false);
        if (status != OC_STATUS_OK) {
            break;
        }
        
        // Apply filter kernel
        status = ocularApplyFFTKernel(imageFFT, kernel, fftWidth, fftHeight);
        if (status != OC_STATUS_OK) {
            break;
        }
        
        // Inverse FFT
        status = ocularFFT2D(imageFFT, fftWidth, fftHeight, true);
        if (status != OC_STATUS_OK) {
            break;
        }
        
        // Copy result back to output image
        for (int y = 0; y < Height; y++) {
            for (int x = 0; x < Width; x++) {
                int srcIdx = y * fftWidth + x;
                int dstIdx = y * Stride + x * Channels + c;
                
                // Convert back to 8-bit and clamp
                float value = imageFFT[srcIdx].real * 255.0f;
                Output[dstIdx] = (unsigned char)ClampToByte((int)(value + 0.5f));
            }
        }
    }
    
    // Cleanup
    free(imageFFT);
    free(kernel);
    
    return status;
}

// Visualize frequency domain (magnitude spectrum)
OC_STATUS ocularFFTVisualize(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, bool logScale) {
    if (Input == NULL || Output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    
    if (Width <= 0 || Height <= 0 || Stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }
    
    int Channels = Stride / Width;
    if (Channels != 1 && Channels != 3 && Channels != 4) {
        return OC_STATUS_ERR_NOTSUPPORTED;
    }
    
    // Find next power of 2 for FFT dimensions
    int fftWidth = nextPowerOfTwo(Width);
    int fftHeight = nextPowerOfTwo(Height);
    
    // Allocate FFT buffer
    OcComplex* imageFFT = (OcComplex*)calloc(fftWidth * fftHeight, sizeof(OcComplex));
    if (imageFFT == NULL) {
        return OC_STATUS_ERR_OUTOFMEMORY;
    }
    
    // Process first channel (or convert to grayscale if multi-channel)
    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            int srcIdx = y * Stride + x * Channels;
            int dstIdx = y * fftWidth + x;
            
            if (Channels == 1) {
                imageFFT[dstIdx].real = (float)Input[srcIdx] / 255.0f;
            } else {
                // Convert to grayscale using luminance formula
                float gray = (0.299f * Input[srcIdx] + 0.587f * Input[srcIdx + 1] + 0.114f * Input[srcIdx + 2]) / 255.0f;
                imageFFT[dstIdx].real = gray;
            }
            imageFFT[dstIdx].imag = 0.0f;
        }
    }
    
    // Forward FFT
    OC_STATUS status = ocularFFT2D(imageFFT, fftWidth, fftHeight, false);
    if (status != OC_STATUS_OK) {
        free(imageFFT);
        return status;
    }
    
    // Calculate magnitude spectrum and find max for normalization
    float maxMagnitude = 0.0f;
    for (int i = 0; i < fftWidth * fftHeight; i++) {
        float magnitude = sqrt(imageFFT[i].real * imageFFT[i].real + imageFFT[i].imag * imageFFT[i].imag);
        if (magnitude > maxMagnitude) {
            maxMagnitude = magnitude;
        }
    }
    
    // Create visualization with DC component at center (fftshift)
    // We need to map from output coordinates to FFT coordinates
    // The goal is to put DC (0,0) at the center of the output image
    
    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            // Calculate the position relative to the center of the output image
            int centerX = Width / 2;
            int centerY = Height / 2;
            
            // Calculate offset from center
            int offsetX = x - centerX;
            int offsetY = y - centerY;
            
            // Map to FFT coordinates
            // Positive offsets map to positive FFT coordinates
            // Negative offsets map to the end of the FFT array (due to periodicity)
            int fftX = offsetX >= 0 ? offsetX : fftWidth + offsetX;
            int fftY = offsetY >= 0 ? offsetY : fftHeight + offsetY;
            
            // Clamp to valid bounds
            fftX = fftX < 0 ? 0 : (fftX >= fftWidth ? fftWidth - 1 : fftX);
            fftY = fftY < 0 ? 0 : (fftY >= fftHeight ? fftHeight - 1 : fftY);
            
            int fftIdx = fftY * fftWidth + fftX;
            
            // Calculate magnitude
            float magnitude = sqrt(imageFFT[fftIdx].real * imageFFT[fftIdx].real + 
                                 imageFFT[fftIdx].imag * imageFFT[fftIdx].imag);
            
            // Normalize and apply scaling
            float normalized = magnitude / (maxMagnitude + 1e-10f);
            
            if (logScale) {
                // Logarithmic scaling for better visualization
                normalized = log(1.0f + normalized * 255.0f) / log(256.0f);
            }
            
            unsigned char pixelValue = (unsigned char)(normalized * 255.0f);
            
            // Set output pixel(s)
            int outIdx = y * Stride + x * Channels;
            for (int c = 0; c < Channels; c++) {
                Output[outIdx + c] = pixelValue;
            }
        }
    }
    
    free(imageFFT);
    return OC_STATUS_OK;
}

