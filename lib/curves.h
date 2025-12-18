/**
 * @file: curves.h
 * @author Warren Galyen
 * Created: 12-17-2025
 * Last Updated: 12-17-2025
 * Last update: initial implementation
 *
 * @brief Ocular curves filter defintions
 */

#ifndef OCULAR_CURVES_H
#define OCULAR_CURVES_H

#include <stdint.h>
#include <stdbool.h>
#include "core.h"

/**
 * @struct OcCurvePoint
 * @brief A point on the curve defined by input and output values
 * @var input The input value (0-255)
 * @var output The output value (0-255)
 */
typedef struct {
    uint8_t input;
    uint8_t output;
} OcCurvePoint;

/**
 * @struct OcCurve
 * @brief Curve definition with control points for tone mapping
 * @var points Array of curve control points
 * @var pointCount Number of control points
 * @var lut Lookup table (256 entries) for fast curve application
 */
typedef struct {
    OcCurvePoint* points;
    int pointCount;
    uint8_t lut[256];  // Precomputed lookup table
} OcCurve;

/**
 * @brief Creates a new curve structure with default linear mapping
 * @return Allocated OcCurve structure, or NULL if allocation fails
 */
OcCurve* createCurve(void);

/**
 * @brief Destroys and frees a curve structure
 * @param curve The curve to destroy
 */
void destroyCurve(OcCurve* curve);

/**
 * @brief Adds or updates a control point on the curve
 * @param curve The curve to modify
 * @param input Input value (0-255)
 * @param output Output value (0-255)
 * @return OC_STATUS_OK if successful, otherwise an error code
 */
OC_STATUS curveAddPoint(OcCurve* curve, uint8_t input, uint8_t output);

/**
 * @brief Removes a control point from the curve
 * @param curve The curve to modify
 * @param input Input value of the point to remove
 * @return OC_STATUS_OK if successful, otherwise an error code
 */
OC_STATUS curveRemovePoint(OcCurve* curve, uint8_t input);

/**
 * @brief Generates the lookup table for the curve based on control points
 * Uses Catmull-Rom interpolation for smooth curves
 * @param curve The curve to build
 * @return OC_STATUS_OK if successful, otherwise an error code
 */
OC_STATUS curveBuild(OcCurve* curve);

/**
 * @brief Applies separate tone curve adjustments to Red, Green, Blue channels and luminance.
 * @ingroup group_color_filters
 * @param Input The image input data buffer
 * @param Output The image output data buffer
 * @param Width The width of the image in pixels
 * @param Height The height of the image in pixels
 * @param Stride The number of bytes in one row of pixels
 * @param curveR Curve for red channel (can be NULL to use identity)
 * @param curveG Curve for green channel (can be NULL to use identity)
 * @param curveB Curve for blue channel (can be NULL to use identity)
 * @param curveL Curve for luminance channel (can be NULL to skip luminance adjustment)
 * @return OC_STATUS_OK if successful, otherwise an error code
 */
OC_STATUS ocularCurvesFilter(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride,
                             const OcCurve* curveR, const OcCurve* curveG, const OcCurve* curveB, const OcCurve* curveL);

#endif /* OCULAR_CURVES_H */
