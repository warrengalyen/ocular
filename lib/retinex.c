
/**
 * @file: retinex.h
 * @author Warren Galyen
 * Created: 10-21-2024
 * Last Updated: 10-21-2024
 * Last update: initial implementation
 *
 * @brief Ocular utility functions implementations that support retinex filter.
 */

/* Based on:
 * MSRCR Retinex
 * (Multi - Scale Retinex with Color Restoration)
 * 2003 Fabien Pelisson<Fabien.Pelisson @inrialpes.fr>
 * Retinex GIMP plug-in
 */

#include "retinex.h"

void retinex_scales_distribution(float* scales, int nscales, int mode, int s) {
    if (nscales == 1) { /* For one filter we choose the median scale */
        scales[0] = (float)s / 2;
    } else if (nscales == 2) { /* For two filters we choose the median and maximum scale */
        scales[0] = (float)s / 2;
        scales[1] = (float)s;
    } else {
        float size_step = (float)s / (float)nscales;
        int i;

        switch (mode) {
            case 0: // uniform
                for (i = 0; i < nscales; ++i)
                    scales[i] = 2.0f + (float)i * size_step;
                break;

            case 1: // low
                size_step = (float)log(s - 2.0f) / (float)nscales;
                for (i = 0; i < nscales; ++i)
                    scales[i] = 2.0f + (float)pow(10, (i * size_step) / log(10));
                break;

            case 2: // high
                size_step = (float)log(s - 2.0) / (float)nscales;
                for (i = 0; i < nscales; ++i)
                    scales[i] = s - (float)pow(10, (i * size_step) / log(10));
                break;

            default: break;
        }
    }
}

// Calculate the mean and variance.
void compute_mean_var(float* src, float* mean, float* var, int size, int bytes) {
    float vsquared;
    int i, j;
    float* psrc;

    vsquared = 0.0f;
    *mean = 0.0f;
    for (i = 0; i < size; i += bytes) {
        psrc = src + i;
        for (j = 0; j < 3; j++) {
            *mean += psrc[j];
            vsquared += psrc[j] * psrc[j];
        }
    }

    *mean /= (float)size;    
    vsquared /= (float)size; 
    *var = (vsquared - (*mean * *mean));
    *var = (float)sqrt(*var); 
}

// Calculate the coefficients for the Gaussian filter.
void compute_coefs3(gauss3_coefs* c, float sigma) {
    /*
     * Papers:  "Recursive Implementation of the gaussian filter.",
     *          Ian T. Young , Lucas J. Van Vliet, Signal Processing 44, Elsevier 1995.
     * formula: 11b       computation of q
     *          8c        computation of b0..b1
     *          10        alpha is normalization constant B
     */
    float q, q2, q3;

    q = 0;

    if (sigma >= 2.5f) {
        q = 0.98711f * sigma - 0.96330f;
    } else if ((sigma >= 0.5f) && (sigma < 2.5f)) {
        q = 3.97156f - 4.14554f * (float)sqrt((double)1 - 0.26891 * sigma);
    } else {
        q = 0.1147705018520355224609375f;
    }

    q2 = q * q;
    q3 = q * q2;
    c->b[0] = (1.57825f + (2.44413f * q) + (1.4281f * q2) + (0.422205f * q3));
    c->b[1] = ((2.44413f * q) + (2.85619f * q2) + (1.26661f * q3));
    c->b[2] = (-((1.4281f * q2) + (1.26661f * q3)));
    c->b[3] = ((0.422205f * q3));
    c->B = 1.0f - ((c->b[1] + c->b[2] + c->b[3]) / c->b[0]);
    c->sigma = sigma;
    c->N = 3;
}

void gausssmooth(float* in, float* out, int size, int rowstride, gauss3_coefs* c) {
    /*
     * Papers:  "Recursive Implementation of the gaussian filter.",
     *          Ian T. Young , Lucas J. Van Vliet, Signal Processing 44, Elsevier 1995.
     * formula: 9a        forward filter
     *          9b        backward filter
     *          fig7      algorithm
     */
    int i, n, bufferSize;
    float *w1, *w2;

    /* forward pass */
    bufferSize = size + 3;
    size -= 1;
    w1 = (float*)malloc(bufferSize * sizeof(float));
    w2 = (float*)malloc(bufferSize * sizeof(float));
    w1[0] = in[0];
    w1[1] = in[0];
    w1[2] = in[0];
    for (i = 0, n = 3; i <= size; i++, n++) {
        w1[n] = (float)(c->B * in[i * rowstride] + ((c->b[1] * w1[n - 1] + c->b[2] * w1[n - 2] + c->b[3] * w1[n - 3]) / c->b[0]));
    }
    /* backward pass */
    w2[size + 1] = w1[size + 3];
    w2[size + 2] = w1[size + 3];
    w2[size + 3] = w1[size + 3];
    for (i = size, n = i; i >= 0; i--, n--) {
        w2[n] = out[i * rowstride] = (float)(c->B * w1[n] + ((c->b[1] * w2[n + 1] + c->b[2] * w2[n + 2] + c->b[3] * w2[n + 3]) / c->b[0]));
    }

    free(w1);
    free(w2);
}
