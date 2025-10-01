/**
 * @file: fft.h
 * @author Warren Galyen
 * Created: 10-1-2024
 * Last Updated: 10-1-2024
 * Last update: initial implementation
 *
 * @brief Fast Fourier Transform implementation for image processing
 */

#ifndef OCULAR_FFT_H
#define OCULAR_FFT_H

#include "core.h"
#include <complex.h>
#include <math.h>

/**
 * @brief Complex number structure for FFT operations
 */
typedef struct {
    float real;
    float imag;
} OcComplex;

/**
 * @brief FFT filter types for frequency domain filtering
 */
typedef enum {
    OC_FFT_LOWPASS,     // Low-pass filter (blur effect)
    OC_FFT_HIGHPASS,    // High-pass filter (edge enhancement)
    OC_FFT_BANDPASS,    // Band-pass filter
    OC_FFT_BANDSTOP,    // Band-stop filter (notch filter)
    OC_FFT_CUSTOM       // Custom filter kernel
} OcFFTFilterType;

/**
 * @brief Parameters for FFT filtering
 */
typedef struct {
    OcFFTFilterType filterType;
    float cutoffFreq;       // Cutoff frequency (0.0 to 1.0)
    float cutoffFreq2;      // Second cutoff for band filters (0.0 to 1.0)
    float* customKernel;    // Custom filter kernel (frequency domain)
    int kernelSize;         // Size of custom kernel
    float sharpness;        // Filter sharpness/rolloff (1.0 = sharp, higher = smoother)
} OcFFTFilterParams;

// Core FFT functions
OC_STATUS ocularFFT(OcComplex* data, int n, bool inverse);
OC_STATUS ocularFFT2D(OcComplex* data, int width, int height, bool inverse);

// Utility functions
OC_STATUS ocularCreateFFTFilterKernel(OcFFTFilterParams* params, int width, int height, OcComplex** kernel);
OC_STATUS ocularApplyFFTKernel(OcComplex* imageFFT, OcComplex* kernel, int width, int height);

/**
 * @brief Applies FFT-based filtering to an image
 * @ingroup group_ip_filters
 * @param Input The image input data buffer
 * @param Output The image output data buffer
 * @param Width The width of the image in pixels
 * @param Height The height of the image in pixels
 * @param Stride The number of bytes in one row of pixels
 * @param params FFT filter parameters
 * @return OC_STATUS_OK if successful, otherwise an error code
 */
OC_STATUS ocularFFTFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, OcFFTFilterParams* params);

#endif /* OCULAR_FFT_H */
