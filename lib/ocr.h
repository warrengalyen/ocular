/**
 * @file: ocr.h
 * @author Warren Galyen
 * Created: 10-15-2024
 * Last Updated: 10-15-2024
 * Last update: added getMedian function for Despeckle filter
 *
 * @brief Ocular utility functions that support OCR pre-processing image processing features.
 */

#ifndef OCULAR_OCR_H
#define OCULAR_OCR_H

#include <stdbool.h>

typedef struct {
    float Theta;
    int Radius;
    int Intensity;
    float RelativeIntensity;
} HoughLine;

typedef struct {
    int x;
    int y;
    int Width;
    int Height;
} OcRect;

// Determine the median of the current pixel window (used for Despeckle)
unsigned char getMedian(unsigned char* window, int size);

// Determine if an image is primarily text based.
bool isTextImage(unsigned char* Input, int Width, int Height);

float calcSkewAngle(unsigned char* Input, int Width, int Height, OcRect* CheckRectPtr, int maxSkewToDetect, int stepsPerDegree,
                    int localPeakRadius, int nLineCount);


#endif  /* OCULAR_OCR_H */
