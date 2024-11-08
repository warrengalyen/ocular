#ifndef OCULAR_FFT_H
#define OCULAR_FFT_H

#include <math.h>
#include <stdlib.h>
#include <complex.h>

// prevent conflict with use in ocular.c
#undef I
#define I_C _Complex_I // refine the use of I

// Function declarations
void fft_2d(complex* input, int width, int height);
void ifft_2d(complex* input, int width, int height);
void fft_1d(complex* input, int n);
void ifft_1d(complex* input, int n);
void transpose_matrix(complex *matrix, int width, int height);
void fft_shift_2d(complex* input, int width, int height);
void ifft_shift_2d(complex* input, int width, int height);

// Helper function declarations
int is_power_of_two(int n);
int next_power_of_two(int n);
void bit_reverse(complex* input, int n);

#endif // OCULAR_FFT_H
