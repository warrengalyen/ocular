/**
 * @file: hazeremoval.h
 * @author Warren Galyen
 * Created: 9-30-2025
 * Last Updated: 9-30-2025
 * Last update: initial implementation
 *
 * @brief Utility functions for dark channel prior haze removal implementation
 */

#ifndef OCULAR_HAZEREMOVAL_H
#define OCULAR_HAZEREMOVAL_H

#include "core.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>


// Utility functions for haze removal (internal use)
void minFilterHorizontal(float* input, float* output, int width, int height, int radius);
void minFilterVertical(float* input, float* output, int width, int height, int radius);
void calculateDarkChannelFast(unsigned char* input, float* darkChannel, int width, int height, int stride, int radius);
float estimateAtmosphericLight(unsigned char* input, float* darkChannel, int width, int height, int stride, float maxAtm);
void estimateTransmissionFast(unsigned char* input, float* transmission, int width, int height, int stride, 
                             float atmLight, int radius, float omega);
void boxFilterFast(float* input, float* output, int width, int height, int radius);
void guidedFilterFast(float* input, float* guide, float* output, int width, int height, int radius, float epsilon);
void recoverScene(unsigned char* input, unsigned char* output, int width, int height, int stride,
                 float* transmission, float atmLight, float t0);


#endif /* OCULAR_HAZEREMOVAL_H */
