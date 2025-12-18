/**
 * @file: curves.c
 * @author Warren Galyen
 * Created: 12-17-2025
 * Last Updated: 12-17-2025
 * Last update: initial implementation
 *
 * @brief Ocular curves filter implementation
 */

#include "curves.h"
#include "util.h"
#include "color.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/**
 * @brief Catmull-Rom interpolation for smooth curve generation
 * @param p0, p1, p2, p3 The four control points
 * @param t Parameter 0.0 to 1.0
 * @return Interpolated value
 */
static float catmullRom(float p0, float p1, float p2, float p3, float t) {
    float t2 = t * t;
    float t3 = t2 * t;
    
    float v0 = (p2 - p0) * 0.5f;
    float v1 = (p3 - p1) * 0.5f;
    
    return (2.0f * p1 - 2.0f * p2 + v0 + v1) * t3 +
           (-3.0f * p1 + 3.0f * p2 - 2.0f * v0 - v1) * t2 +
           v0 * t +
           p1;
}

/**
 * @brief Compares two curve points by input value (for qsort)
 */
static int comparePoints(const void* a, const void* b) {
    const OcCurvePoint* pa = (const OcCurvePoint*)a;
    const OcCurvePoint* pb = (const OcCurvePoint*)b;
    return (int)pa->input - (int)pb->input;
}

OcCurve* createCurve(void) {
    OcCurve* curve = (OcCurve*)malloc(sizeof(OcCurve));
    if (curve == NULL) {
        return NULL;
    }

    curve->pointCount = 2;
    curve->points = (OcCurvePoint*)malloc(2 * sizeof(OcCurvePoint));
    
    if (curve->points == NULL) {
        free(curve);
        return NULL;
    }

    // Initialize with linear mapping (0->0, 255->255)
    curve->points[0].input = 0;
    curve->points[0].output = 0;
    curve->points[1].input = 255;
    curve->points[1].output = 255;

    // Initialize lookup table with linear mapping
    for (int i = 0; i < 256; i++) {
        curve->lut[i] = (uint8_t)i;
    }

    return curve;
}

void destroyCurve(OcCurve* curve) {
    if (curve == NULL) {
        return;
    }
    
    if (curve->points != NULL) {
        free(curve->points);
    }
    
    free(curve);
}

OC_STATUS curveAddPoint(OcCurve* curve, uint8_t input, uint8_t output) {
    if (curve == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }

    // Check if a point with this input already exists
    for (int i = 0; i < curve->pointCount; i++) {
        if (curve->points[i].input == input) {
            curve->points[i].output = output;
            return OC_STATUS_OK;
        }
    }

    // Add new point
    OcCurvePoint* newPoints = (OcCurvePoint*)realloc(curve->points, 
                                                       (curve->pointCount + 1) * sizeof(OcCurvePoint));
    if (newPoints == NULL) {
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

    curve->points = newPoints;
    curve->points[curve->pointCount].input = input;
    curve->points[curve->pointCount].output = output;
    curve->pointCount++;

    // Sort points by input value
    qsort(curve->points, curve->pointCount, sizeof(OcCurvePoint), comparePoints);

    return OC_STATUS_OK;
}

OC_STATUS curveRemovePoint(OcCurve* curve, uint8_t input) {
    if (curve == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }

    // Cannot remove points if only 2 remain (endpoints)
    if (curve->pointCount <= 2) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    // Find and remove the point
    for (int i = 0; i < curve->pointCount; i++) {
        if (curve->points[i].input == input) {
            // Don't allow removal of endpoints
            if (i == 0 || i == curve->pointCount - 1) {
                return OC_STATUS_ERR_INVALIDPARAMETER;
            }

            // Shift remaining points
            for (int j = i; j < curve->pointCount - 1; j++) {
                curve->points[j] = curve->points[j + 1];
            }

            curve->pointCount--;

            // Resize array
            OcCurvePoint* newPoints = (OcCurvePoint*)realloc(curve->points, 
                                                               curve->pointCount * sizeof(OcCurvePoint));
            if (newPoints != NULL) {
                curve->points = newPoints;
            }

            return OC_STATUS_OK;
        }
    }

    return OC_STATUS_ERR_INVALIDPARAMETER;
}

OC_STATUS curveBuild(OcCurve* curve) {
    if (curve == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }

    if (curve->pointCount < 2) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    // Sort points by input value (in case they're out of order)
    qsort(curve->points, curve->pointCount, sizeof(OcCurvePoint), comparePoints);

    // Generate lookup table using Catmull-Rom interpolation
    for (int x = 0; x < 256; x++) {
        float value = (float)x;

        // Find the segment this value belongs to
        int segmentIndex = 0;
        for (int i = 0; i < curve->pointCount - 1; i++) {
            if (x >= curve->points[i].input && x <= curve->points[i + 1].input) {
                segmentIndex = i;
                break;
            }
        }

        // Get the four control points for Catmull-Rom interpolation
        float p0, p1, p2, p3;

        if (segmentIndex == 0) {
            // First segment: duplicate first point
            p0 = (float)curve->points[0].output;
            p1 = (float)curve->points[0].output;
            p2 = (float)curve->points[1].output;
            p3 = (curve->pointCount > 2) ? (float)curve->points[2].output : (float)curve->points[1].output;
        } else if (segmentIndex == curve->pointCount - 2) {
            // Last segment: duplicate last point
            p0 = (float)curve->points[segmentIndex - 1].output;
            p1 = (float)curve->points[segmentIndex].output;
            p2 = (float)curve->points[segmentIndex + 1].output;
            p3 = (float)curve->points[segmentIndex + 1].output;
        } else {
            // Middle segment
            p0 = (float)curve->points[segmentIndex - 1].output;
            p1 = (float)curve->points[segmentIndex].output;
            p2 = (float)curve->points[segmentIndex + 1].output;
            p3 = (float)curve->points[segmentIndex + 2].output;
        }

        // Calculate the parameter t (0.0 to 1.0) within this segment
        int x1 = curve->points[segmentIndex].input;
        int x2 = curve->points[segmentIndex + 1].input;
        float range = (float)(x2 - x1);
        float t = (range > 0.0f) ? (value - x1) / range : 0.0f;

        // Interpolate
        float result = catmullRom(p0, p1, p2, p3, t);

        // Clamp and convert to byte
        curve->lut[x] = ClampToByte((int)(result + 0.5f));
    }

    return OC_STATUS_OK;
}

OC_STATUS ocularCurvesFilter(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                             const OcCurve* curveR, const OcCurve* curveG, const OcCurve* curveB, const OcCurve* curveL) {
    if (Input == NULL || Output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }

    if (Width <= 0 || Height <= 0 || Stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    int Channels = Stride / Width;
    if (Channels < 3) {
        return OC_STATUS_ERR_NOTSUPPORTED;
    }

    int pixelCount = Width * Height;

    // If all curves are NULL, just copy input to output
    if (curveR == NULL && curveG == NULL && curveB == NULL && curveL == NULL) {
        memcpy(Output, Input, pixelCount * Channels);
        return OC_STATUS_OK;
    }

    // If luminance curve is NULL, use optimized RGB-only path
    if (curveL == NULL) {
        for (int i = 0; i < pixelCount; i++) {
            int idx = i * Channels;
            
            // Apply curves to each channel, or copy if curve is NULL
            Output[idx] = (curveR != NULL) ? curveR->lut[Input[idx]] : Input[idx];           // R
            Output[idx + 1] = (curveG != NULL) ? curveG->lut[Input[idx + 1]] : Input[idx + 1];   // G
            Output[idx + 2] = (curveB != NULL) ? curveB->lut[Input[idx + 2]] : Input[idx + 2];   // B
            
            // Copy alpha channel if present
            if (Channels > 3) {
                Output[idx + 3] = Input[idx + 3];
            }
        }
    } else {
        // Apply RGB curves first, then luminance curve (preserves hue and saturation)
        for (int i = 0; i < pixelCount; i++) {
            int idx = i * Channels;
            
            unsigned char R, G, B;
            
            // Apply RGB channel curves first
            R = (curveR != NULL) ? curveR->lut[Input[idx]] : Input[idx];
            G = (curveG != NULL) ? curveG->lut[Input[idx + 1]] : Input[idx + 1];
            B = (curveB != NULL) ? curveB->lut[Input[idx + 2]] : Input[idx + 2];
            
            // Convert to HSL to apply luminance curve while preserving hue and saturation
            float h, s, l;
            rgb2hsl((float)R, (float)G, (float)B, &h, &s, &l);
            
            // Apply luminance curve to the L component
            uint8_t lum = (uint8_t)(l * 255.0f);
            uint8_t newLum = curveL->lut[lum];
            float newL = (float)newLum / 255.0f;
            
            // Convert back to RGB
            float newR, newG, newB;
            hsl2rgb(h, s, newL, &newR, &newG, &newB);
            
            Output[idx] = ClampToByte((int)newR);
            Output[idx + 1] = ClampToByte((int)newG);
            Output[idx + 2] = ClampToByte((int)newB);
            
            // Copy alpha channel if present
            if (Channels > 3) {
                Output[idx + 3] = Input[idx + 3];
            }
        }
    }

    return OC_STATUS_OK;
}
