/**
 * @file: blend.h
 * @author Warren Galyen
 * Created: 2-21-2024
 * Last Updated: 2-21-2024
 * Last update: initial implementation
 *
 * @brief Contains image layer color blending functions
 */

#ifndef OCUALR_BLEND_H
#define OCUALR_BLEND_H

#include <stdlib.h>
#include "util.h"
#include "color.h"

typedef enum {
    OC_BLEND_NORMAL,
    OC_BLEND_DISSOLVE,
    OC_BLEND_DARKEN,
    OC_BLEND_MULTIPLY,
    OC_BLEND_COLORBURN,
    OC_BLEND_LINEARBURN,
    OC_BLEND_DARK,
    OC_BLEND_LIGHTEN,
    OC_BLEND_SCREEN,
    OC_BLEND_COLORDODGE,
    OC_BLEND_LINEARDODGE,
    OC_BLEND_LIGHTERCOLOR,
    OC_BLEND_OVERLAY,
    OC_BLEND_SOFTLIGHT,
    OC_BLEND_HARDLIGHT,
    OC_BLEND_VIVIDLIGHT,
    OC_BLEND_LINEARLIGHT,
    OC_BLEND_PINLIGHT,
    OC_BLEND_HARDMIX,
    OC_BLEND_DIFFERENCE,
    OC_BLEND_EXCLUSION,
    OC_BLEND_SUBTRACT,
    OC_BLEND_DIVIDE,
    OC_BLEND_HUE,
    OC_BLEND_SATURATION,
    OC_BLEND_COLOR,
    OC_BLEND_LUMINOSITY
} OcBlendMode;

// normal mode
static inline void BlendNormal(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB, int alpha) {
    *resR = (baseR * (100 - alpha) + mixR * alpha) / 100;
    *resG = (baseG * (100 - alpha) + mixG * alpha) / 100;
    *resB = (baseB * (100 - alpha) + mixB * alpha) / 100;
}

// uniform random number
static inline double AverageRandom(double a, double b) {
    double res = 0;
    do {
        res = rand() * (1.0 / RAND_MAX);
    } while (res <= a || res > b);
    return res;
};

// dissolution mode
static inline void BlendDissolve(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB, int alpha) {
    int threshold = (int)(AverageRandom(0, 1.0) * 100.0);
    if (threshold > alpha) {
        *resR = mixR;
        *resG = mixG;
        *resB = mixB;
    } else {
        *resR = baseR;
        *resG = baseG;
        *resB = baseB;
    }
};

// dim mode
static inline void BlendDarken(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB, int alpha) {
    *resR = (baseR > mixR ? mixR : baseR);
    *resG = (baseG > mixG ? mixG : baseG);
    *resB = (baseB > mixB ? mixB : baseB);
    *resR = (*resR * alpha + baseR * (100 - alpha)) / 100;
    *resG = (*resG * alpha + baseG * (100 - alpha)) / 100;
    *resB = (*resB * alpha + baseB * (100 - alpha)) / 100;
};

// multiply mode
static inline void BlendMultiply(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB, int alpha) {
    *resR = baseR * mixR / 255;
    *resG = baseG * mixG / 255;
    *resB = baseB * mixB / 255;
    *resR = (*resR * alpha + baseR * (100 - alpha)) / 100;
    *resG = (*resG * alpha + baseG * (100 - alpha)) / 100;
    *resB = (*resB * alpha + baseB * (100 - alpha)) / 100;
};

// burn mode
static inline void BlendColorBurn(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB, int alpha) {
    *resR = clamp((baseR + mixR - 255) * 255 / (mixR + 1), 0, 255);
    *resG = clamp((baseG + mixG - 255) * 255 / (mixG + 1), 0, 255);
    *resB = clamp((baseB + mixB - 255) * 255 / (mixB + 1), 0, 255);

    *resR = (*resR * alpha + baseR * (100 - alpha)) / 100;
    *resG = (*resG * alpha + baseG * (100 - alpha)) / 100;
    *resB = (*resB * alpha + baseB * (100 - alpha)) / 100;
};

// linear burn mode
static inline void BlendLinearBurn(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB, int alpha) {
    *resR = clamp(baseR + mixR - 255, 0, 255);
    *resG = clamp(baseG + mixG - 255, 0, 255);
    *resB = clamp(baseB + mixB - 255, 0, 255);
    *resR = (*resR * alpha + baseR * (100 - alpha)) / 100;
    *resG = (*resG * alpha + baseG * (100 - alpha)) / 100;
    *resB = (*resB * alpha + baseB * (100 - alpha)) / 100;
};

// dark mode
static inline void BlendDark(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB, int alpha) {
    int baseSum = 0, mixSum = 0;
    baseSum = baseR + baseG + baseB;
    mixSum = mixR + mixG + mixB;
    if (baseSum < mixSum) {
        *resR = baseR;
        *resG = baseG;
        *resB = baseB;
    } else {
        *resR = mixR;
        *resG = mixG;
        *resB = mixB;
    }
    *resR = (*resR * alpha + baseR * (100 - alpha)) / 100;
    *resG = (*resG * alpha + baseG * (100 - alpha)) / 100;
    *resB = (*resB * alpha + baseB * (100 - alpha)) / 100;
};


// brighten mode
static inline void BlendLighten(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB, int alpha) {
    *resR = (baseR > mixR ? baseR : mixR);
    *resG = (baseG > mixG ? baseG : mixG);
    *resB = (baseB > mixB ? baseB : mixB);
    *resR = (*resR * alpha + baseR * (100 - alpha)) / 100;
    *resG = (*resG * alpha + baseG * (100 - alpha)) / 100;
    *resB = (*resB * alpha + baseB * (100 - alpha)) / 100;
};

// screen mode
static inline void BlendScreen(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB, int alpha) {
    *resR = 255 - (255 - mixR) * (255 - baseR) / 255;
    *resG = 255 - (255 - mixG) * (255 - baseG) / 255;
    *resB = 255 - (255 - mixB) * (255 - baseB) / 255;
    *resR = clamp((*resR * alpha + baseR * (100 - alpha)) / 100, 0, 255);
    *resG = clamp((*resG * alpha + baseG * (100 - alpha)) / 100, 0, 255);
    *resB = clamp((*resB * alpha + baseB * (100 - alpha)) / 100, 0, 255);
};

// color dodge mode
static inline void BlendColorDodge(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB, int alpha) {
    *resR = clamp(baseR + (baseR * mixR) / (256 - mixR), 0, 255);
    *resG = clamp(baseG + (baseG * mixG) / (256 - mixG), 0, 255);
    *resB = clamp(baseB + (baseB * mixB) / (256 - mixB), 0, 255);
    *resR = (*resR * alpha + baseR * (100 - alpha)) / 100;
    *resG = (*resG * alpha + baseG * (100 - alpha)) / 100;
    *resB = (*resB * alpha + baseB * (100 - alpha)) / 100;
};


// linear dodge mode
static inline void BlendLinearDodge(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB, int alpha) {
    *resR = clamp(baseR + mixR, 0, 255);
    *resG = clamp(baseG + mixG, 0, 255);
    *resB = clamp(baseB + mixB, 0, 255);
    *resR = (*resR * alpha + baseR * (100 - alpha)) / 100;
    *resG = (*resG * alpha + baseG * (100 - alpha)) / 100;
    *resB = (*resB * alpha + baseB * (100 - alpha)) / 100;
};

// lighter mode
static inline void BlendLighterColor(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB, int alpha) {
    int baseSum = 0, mixSum = 0;
    baseSum = baseR + baseG + baseB;
    mixSum = mixR + mixG + mixB;
    if (baseSum > mixSum) {
        *resR = baseR;
        *resG = baseG;
        *resB = baseB;
    } else {
        *resR = mixR;
        *resG = mixG;
        *resB = mixB;
    }
    *resR = (*resR * alpha + baseR * (100 - alpha)) / 100;
    *resG = (*resG * alpha + baseG * (100 - alpha)) / 100;
    *resB = (*resB * alpha + baseB * (100 - alpha)) / 100;
};


// overlay mode
static inline void BlendOverlay(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB, int alpha) {
    *resR = clamp(((baseR <= 128) ? (mixR * baseR / 128) : (255 - (255 - mixR) * (255 - baseR) / 128)), 0, 255);
    *resG = clamp(((baseG <= 128) ? (mixG * baseG / 128) : (255 - (255 - mixG) * (255 - baseG) / 128)), 0, 255);
    *resB = clamp(((baseB <= 128) ? (mixB * baseB / 128) : (255 - (255 - mixB) * (255 - baseB) / 128)), 0, 255);
    *resR = (*resR * alpha + baseR * (100 - alpha)) / 100;
    *resG = (*resG * alpha + baseG * (100 - alpha)) / 100;
    *resB = (*resB * alpha + baseB * (100 - alpha)) / 100;
};

// soft light mode
static inline void BlendSoftLight(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB, int alpha) {
    *resR = clamp(mixR > 128 ?
                          ((int)((float)baseR + ((float)mixR + (float)mixR - 255.0f) * ((sqrt((float)baseR / 255.0f)) * 255.0f - (float)baseR) / 255.0f)) :
                          ((int)((float)baseR + ((float)mixR + (float)mixR - 255.0f) * ((float)baseR - (float)baseR * (float)baseR / 255.0f) / 255.0f)),
                  0, 255);
    *resG = clamp(mixG > 128 ?
                          ((int)((float)baseG + ((float)mixG + (float)mixG - 255.0f) * ((sqrt((float)baseG / 255.0f)) * 255.0f - (float)baseG) / 255.0f)) :
                          ((int)((float)baseG + ((float)mixG + (float)mixG - 255.0f) * ((float)baseG - (float)baseG * (float)baseG / 255.0f) / 255.0f)),
                  0, 255);
    *resB = clamp(mixB > 128 ?
                          ((int)((float)baseB + ((float)mixB + (float)mixB - 255.0f) * ((sqrt((float)baseB / 255.0f)) * 255.0f - (float)baseB) / 255.0f)) :
                          ((int)((float)baseB + ((float)mixB + (float)mixB - 255.0f) * ((float)baseB - (float)baseB * (float)baseB / 255.0f) / 255.0f)),
                  0, 255);
    *resR = (*resR * alpha + baseR * (100 - alpha)) / 100;
    *resG = (*resG * alpha + baseG * (100 - alpha)) / 100;
    *resB = (*resB * alpha + baseB * (100 - alpha)) / 100;
};

// hard light mode
static inline void BlendHardLight(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB, int alpha) {
    *resR = clamp(mixR <= 128 ? (mixR * baseR / 128) : (255 - (255 - mixR) * (255 - baseR) / 128), 0, 255);
    *resG = clamp(mixG <= 128 ? (mixG * baseG / 128) : (255 - (255 - mixG) * (255 - baseG) / 128), 0, 255);
    *resB = clamp(mixB <= 128 ? (mixB * baseB / 128) : (255 - (255 - mixB) * (255 - baseB) / 128), 0, 255);
    *resR = (*resR * alpha + baseR * (100 - alpha)) / 100;
    *resG = (*resG * alpha + baseG * (100 - alpha)) / 100;
    *resB = (*resB * alpha + baseB * (100 - alpha)) / 100;
};

// vivid light mode
static inline void BlendVividLight(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB, int alpha) {
    if (mixR <= 128)
        *resR = (baseR - (255 - baseR) * (255 - 2 * mixR) / (2 * mixR + 1));
    else
        *resR = baseR + (baseR * (2 * mixR - 255)) / (255 - (2 * mixR - 255) + 1);
    if (mixG <= 128)
        *resG = (baseG - (255 - baseG) * (255 - 2 * mixG) / (2 * mixG + 1));
    else
        *resG = baseG + (baseG * (2 * mixG - 255)) / (255 - (2 * mixG - 255) + 1);
    if (mixB <= 128)
        *resB = (baseB - (255 - baseB) * (255 - 2 * mixB) / (2 * mixB + 1));
    else
        *resB = baseB + (baseB * (2 * mixB - 255)) / (255 - (2 * mixB - 255) + 1);
    *resR = clamp(*resR, 0, 255);
    *resG = clamp(*resG, 0, 255);
    *resB = clamp(*resB, 0, 255);
    *resR = (*resR * alpha + baseR * (100 - alpha)) / 100;
    *resG = (*resG * alpha + baseG * (100 - alpha)) / 100;
    *resB = (*resB * alpha + baseB * (100 - alpha)) / 100;
};


// linear light mode
static inline void BlendLinearLight(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB, int alpha) {
    *resR = clamp(2 * mixR + baseR - 255, 0, 255);
    *resG = clamp(2 * mixG + baseG - 255, 0, 255);
    *resB = clamp(2 * mixB + baseB - 255, 0, 255);
    *resR = (*resR * alpha + baseR * (100 - alpha)) / 100;
    *resG = (*resG * alpha + baseG * (100 - alpha)) / 100;
    *resB = (*resB * alpha + baseB * (100 - alpha)) / 100;
};


// pinlight mode
static inline void BlendPinLight(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB, int alpha) {
    int res = 0;
    int a = mixR + mixR - 255;
    int b = mixR + mixR;
    if (baseR < a)
        res = b - 255;
    if (baseR >= a && baseR < b)
        res = baseR;
    if (baseR > b)
        res = b;
    *resR = clamp(res, 0, 255);

    a = mixG + mixG - 255;
    b = mixG + mixG;
    if (baseG < a)
        res = b - 255;
    if (baseG >= a && baseG < b)
        res = baseG;
    if (baseG > b)
        res = b;
    *resG = clamp(res, 0, 255);

    a = mixB + mixB - 255;
    b = mixB + mixB;
    if (baseB < a)
        res = b - 255;
    if (baseB >= a && baseB < b)
        res = baseB;
    if (baseB > b)
        res = b;
    *resB = clamp(res, 0, 255);
    *resR = clamp((*resR * alpha + baseR * (100 - alpha)) / 100, 0, 255);
    *resG = clamp((*resG * alpha + baseG * (100 - alpha)) / 100, 0, 255);
    *resB = clamp((*resB * alpha + baseB * (100 - alpha)) / 100, 0, 255);
};

// hard mix mode
static inline void BlendHardMix(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB, int alpha) {
    *resR = baseR + mixR < 255 ? 0 : 255;
    *resG = baseG + mixG < 255 ? 0 : 255;
    *resB = baseB + mixB < 255 ? 0 : 255;
    *resR = (*resR * alpha + baseR * (100 - alpha)) / 100;
    *resG = (*resG * alpha + baseG * (100 - alpha)) / 100;
    *resB = (*resB * alpha + baseB * (100 - alpha)) / 100;
};

// difference mode
static inline void BlendDifference(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB, int alpha) {
    *resR = abs(mixR - baseR);
    *resG = abs(mixG - baseG);
    *resB = abs(mixB - baseB);
    *resR = (*resR * alpha + baseR * (100 - alpha)) / 100;
    *resG = (*resG * alpha + baseG * (100 - alpha)) / 100;
    *resB = (*resB * alpha + baseB * (100 - alpha)) / 100;
};

// exclusion mode
static inline void BlendExclusion(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB, int alpha) {
    *resR = clamp((mixR + baseR) - mixR * baseR / 128, 0, 255);
    *resG = clamp((mixG + baseG) - mixG * baseG / 128, 0, 255);
    *resB = clamp((mixB + baseB) - mixB * baseB / 128, 0, 255);
    *resR = (*resR * alpha + baseR * (100 - alpha)) / 100;
    *resG = (*resG * alpha + baseG * (100 - alpha)) / 100;
    *resB = (*resB * alpha + baseB * (100 - alpha)) / 100;
};

// subtract mode
static inline void BlendSubtract(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB, int alpha) {
    *resR = clamp(baseR - mixR, 0, 255);
    *resG = clamp(baseG - mixG, 0, 255);
    *resB = clamp(baseB - mixB, 0, 255);
    *resR = (*resR * alpha + baseR * (100 - alpha)) / 100;
    *resG = (*resG * alpha + baseG * (100 - alpha)) / 100;
    *resB = (*resB * alpha + baseB * (100 - alpha)) / 100;
};

// divide mode
static inline void BlendDivide(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB, int alpha) {
    *resR = clamp(255 * baseR / (mixR + 1), 0, 255);
    *resG = clamp(255 * baseG / (mixG + 1), 0, 255);
    *resB = clamp(255 * baseB / (mixB + 1), 0, 255);
    *resR = (*resR * alpha + baseR * (100 - alpha)) / 100;
    *resG = (*resG * alpha + baseG * (100 - alpha)) / 100;
    *resB = (*resB * alpha + baseB * (100 - alpha)) / 100;
};

// hue mode
static inline void BlendHue(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB, int alpha) {
    float baseH = 0, baseS = 0, baseV = 0;
    float mixH = 0, mixS = 0, mixV = 0;
    rgb2hsv(baseR, baseG, baseB, (unsigned char*)&baseH, (unsigned char*)&baseS, (unsigned char*)&baseV);
    rgb2hsv(mixR, mixG, mixB, (unsigned char*)&mixH, (unsigned char*)&mixS, (unsigned char*)&mixV);
    unsigned char R = 0, G = 0, B = 0;
    hsv2rgb((unsigned char)mixH, (unsigned char)baseS, (unsigned char)baseV, &R, &G, &B);
    *resR = (R * alpha + baseR * (100 - alpha)) / 100;
    *resG = (G * alpha + baseG * (100 - alpha)) / 100;
    *resB = (B * alpha + baseB * (100 - alpha)) / 100;
};

// saturation mode
static inline void BlendSaturation(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB, int alpha) {
    float baseH = 0, baseS = 0, baseV = 0;
    float mixH = 0, mixS = 0, mixV = 0;
    rgb2hsv(baseR, baseG, baseB, (unsigned char*)&baseH, (unsigned char*)&baseS, (unsigned char*)&baseV);
    rgb2hsv(mixR, mixG, mixB, (unsigned char*)&mixH, (unsigned char*)&mixS, (unsigned char*)&mixV);
    unsigned char R = 0, G = 0, B = 0;
    hsv2rgb((unsigned char)baseH, (unsigned char)mixS, (unsigned char)baseV, &R, &G, &B);
    *resR = (R * alpha + baseR * (100 - alpha)) / 100;
    *resG = (G * alpha + baseG * (100 - alpha)) / 100;
    *resB = (B * alpha + baseB * (100 - alpha)) / 100;
};


// luminosity mode
static inline void BlendLuminosity(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB, int alpha) {
    float baseH = 0, baseS = 0, baseV = 0;
    float mixH = 0, mixS = 0, mixV = 0;
    rgb2hsv(baseR, baseG, baseB, (unsigned char*)&baseH, (unsigned char*)&baseS, (unsigned char*)&baseV);
    rgb2hsv(mixR, mixG, mixB, (unsigned char*)&mixH, (unsigned char*)&mixS, (unsigned char*)&mixV);
    unsigned char R = 0, G = 0, B = 0;
    hsv2rgb((unsigned char)baseH, (unsigned char)baseS, (unsigned char)mixV, &R, &G, &B);
    *resR = (R * alpha + baseR * (100 - alpha)) / 100;
    *resG = (G * alpha + baseG * (100 - alpha)) / 100;
    *resB = (B * alpha + baseB * (100 - alpha)) / 100;
};

// color mode
static inline void BlendColor(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB, int alpha) {
    float baseH = 0, baseS = 0, baseV = 0;
    float mixH = 0, mixS = 0, mixV = 0;
    rgb2hsv(baseR, baseG, baseB, (unsigned char*)&baseH, (unsigned char*)&baseS, (unsigned char*)&baseV);
    rgb2hsv(mixR, mixG, mixB, (unsigned char*)&mixH, (unsigned char*)&mixS, (unsigned char*)&mixV);
    unsigned char R = 0, G = 0, B = 0;
    hsv2rgb((unsigned char)mixH, (unsigned char)mixS, (unsigned char)baseV, &R, &G, &B);
    *resR = (R * alpha + baseR * (100 - alpha)) / 100;
    *resG = (G * alpha + baseG * (100 - alpha)) / 100;
    *resB = (B * alpha + baseB * (100 - alpha)) / 100;
};

static inline void layerBlend(int baseR, int baseG, int baseB, int mixR, int mixG, int mixB, int* resR, int* resG, int* resB,
                              OcBlendMode blendMode, int alpha) {
    switch (blendMode) {
    case OC_BLEND_NORMAL: BlendNormal(baseR, baseG, baseB, mixR, mixG, mixB, resR, resG, resB, alpha); break;
    case OC_BLEND_DISSOLVE: BlendDissolve(baseR, baseG, baseB, mixR, mixG, mixB, resR, resG, resB, alpha); break;
    case OC_BLEND_DARKEN: BlendDarken(baseR, baseG, baseB, mixR, mixG, mixB, resR, resG, resB, alpha); break;
    case OC_BLEND_MULTIPLY: BlendMultiply(baseR, baseG, baseB, mixR, mixG, mixB, resR, resG, resB, alpha); break;
    case OC_BLEND_COLORBURN: BlendColorBurn(baseR, baseG, baseB, mixR, mixG, mixB, resR, resG, resB, alpha); break;
    case OC_BLEND_LINEARBURN: BlendLinearBurn(baseR, baseG, baseB, mixR, mixG, mixB, resR, resG, resB, alpha); break;
    case OC_BLEND_DARK: BlendDark(baseR, baseG, baseB, mixR, mixG, mixB, resR, resG, resB, alpha); break;
    case OC_BLEND_LIGHTEN: BlendLighten(baseR, baseG, baseB, mixR, mixG, mixB, resR, resG, resB, alpha); break;
    case OC_BLEND_SCREEN: BlendScreen(baseR, baseG, baseB, mixR, mixG, mixB, resR, resG, resB, alpha); break;
    case OC_BLEND_COLORDODGE: BlendColorDodge(baseR, baseG, baseB, mixR, mixG, mixB, resR, resG, resB, alpha); break;
    case OC_BLEND_LINEARDODGE: BlendLinearDodge(baseR, baseG, baseB, mixR, mixG, mixB, resR, resG, resB, alpha); break;
    case OC_BLEND_LIGHTERCOLOR: BlendLighterColor(baseR, baseG, baseB, mixR, mixG, mixB, resR, resG, resB, alpha); break;
    case OC_BLEND_OVERLAY: BlendOverlay(baseR, baseG, baseB, mixR, mixG, mixB, resR, resG, resB, alpha); break;
    case OC_BLEND_SOFTLIGHT: BlendSoftLight(baseR, baseG, baseB, mixR, mixG, mixB, resR, resG, resB, alpha); break;
    case OC_BLEND_HARDLIGHT: BlendHardLight(baseR, baseG, baseB, mixR, mixG, mixB, resR, resG, resB, alpha); break;
    case OC_BLEND_VIVIDLIGHT: BlendVividLight(baseR, baseG, baseB, mixR, mixG, mixB, resR, resG, resB, alpha); break;
    case OC_BLEND_LINEARLIGHT: BlendLinearLight(baseR, baseG, baseB, mixR, mixG, mixB, resR, resG, resB, alpha); break;
    case OC_BLEND_PINLIGHT: BlendPinLight(baseR, baseG, baseB, mixR, mixG, mixB, resR, resG, resB, alpha); break;
    case OC_BLEND_HARDMIX: BlendHardMix(baseR, baseG, baseB, mixR, mixG, mixB, resR, resG, resB, alpha); break;
    case OC_BLEND_DIFFERENCE: BlendDifference(baseR, baseG, baseB, mixR, mixG, mixB, resR, resG, resB, alpha); break;
    case OC_BLEND_EXCLUSION: BlendExclusion(baseR, baseG, baseB, mixR, mixG, mixB, resR, resG, resB, alpha); break;
    case OC_BLEND_SUBTRACT: BlendSubtract(baseR, baseG, baseB, mixR, mixG, mixB, resR, resG, resB, alpha); break;
    case OC_BLEND_DIVIDE: BlendDivide(baseR, baseG, baseB, mixR, mixG, mixB, resR, resG, resB, alpha); break;
    case OC_BLEND_HUE: BlendHue(baseR, baseG, baseB, mixR, mixG, mixB, resR, resG, resB, alpha); break;
    case OC_BLEND_SATURATION: BlendSaturation(baseR, baseG, baseB, mixR, mixG, mixB, resR, resG, resB, alpha); break;
    case OC_BLEND_COLOR: BlendColor(baseR, baseG, baseB, mixR, mixG, mixB, resR, resG, resB, alpha); break;
    case OC_BLEND_LUMINOSITY: BlendLuminosity(baseR, baseG, baseB, mixR, mixG, mixB, resR, resG, resB, alpha); break;
    default: break;
    }
};

#endif /* OCUALR_BLEND_H */
