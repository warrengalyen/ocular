/**
 * @file: retinex.h
 * @author Warren Galyen
 * Created: 10-21-2024
 * Last Updated: 10-21-2024
 * Last update: initial implementation
 *
 * @brief Ocular utility functions definitions that support retinex filter.
 */

/* Based on:
 * MSRCR Retinex
 * (Multi - Scale Retinex with Color Restoration) 
 * 2003 Fabien Pelisson<Fabien.Pelisson @inrialpes.fr>
 * Retinex GIMP plug-in
*/

#ifndef OCULAR_RETINEX_H
#define OCULAR_RETINEX_H

#include <math.h>
#include <stdlib.h>

#define MAX_RETINEX_SCALES 8
#define MIN_GAUSSIAN_SCALE 16
#define MAX_GAUSSIAN_SCALE 250
#define SCALE_WIDTH 150
#define ENTRY_WIDTH 4

typedef struct {
    int scale;       /* Maximum Retinex scale */
    int nscales;     /* Number of scales        */
    int scales_mode; /* Retinex scale calculation mode：UNIFORM, LOW, HIGH */
    float cvar;      /* Multiplier factor used to adjust the variance of the color dynamic range  */
} RetinexParams;

static float RetinexScales[MAX_RETINEX_SCALES];

typedef struct {
    int N;
    float sigma;
    double B;
    double b[4];
} gauss3_coefs;

// Calculate scale values for desired distribution.
void retinex_scales_distribution(float* scales, int nscales, int mode, int s);

// Calculate the mean and variance.
void compute_mean_var(float* src, float* mean, float* var, int size, int bytes);

// Calculate the coefficients for the Gaussian filter.
void compute_coefs3(gauss3_coefs* c, float sigma);

void gausssmooth(float* in, float* out, int size, int rowstride, gauss3_coefs* c);

#endif  /* OCULAR_RETINEX_H */
