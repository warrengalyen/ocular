/**
 * @file: interpolate.h
 * @author Warren Galyen
 * Created: 5-23-2024
 * Last Updated: 5-23-2024
 * Last update: initial implementation
 *
 * @brief Ocular interpolation/resize method definitions
 */

#ifndef OCULAR_INTERPOLATE_H
#define OCULAR_INTERPOLATE_H

#include "util.h"


static inline int mirrorCoord(int coord, int size) {
    if (coord < 0) {
        return -coord;
    }
    if (coord >= size) {
        return 2 * size - coord - 2;
    }
    return coord;
}

// Nearest-neighbor interpolation function
static void nearestNeighborResize(unsigned char* Src, unsigned char* Dest, int srcWidth, int srcHeight, int dstWidth, int dstHeight, int Channels) {
    float xRatio = (float)srcWidth / dstWidth;
    float yRatio = (float)srcHeight / dstHeight;

    for (int dstY = 0; dstY < dstHeight; dstY++) {
        for (int dstX = 0; dstX < dstWidth; dstX++) {
            int srcX = (int)(xRatio * dstX);
            int srcY = (int)(yRatio * dstY);

            for (int channel = 0; channel < Channels; channel++) {
                Dest[(dstY * dstWidth + dstX) * Channels + channel] = Src[(srcY * srcWidth + srcX) * Channels + channel];
            }
        }
    }
}

// Cubic interpolation helper function
static float cubicInterpolate(float p0, float p1, float p2, float p3, float fraction) {
    float a = (-0.5 * p0) + (1.5 * p1) - (1.5 * p2) + (0.5 * p3);
    float b = p0 - (2.5 * p1) + (2 * p2) - (0.5 * p3);
    float c = (-0.5 * p0) + (0.5 * p2);
    float d = p1;
    return a * fraction * fraction * fraction + b * fraction * fraction + c * fraction + d;
}

// Bicubic interpolation for a 4x4 patch
static float bicubicInterpolate(float patch[4][4], float xFraction, float yFraction) {
    float colInterpolates[4];
    for (int i = 0; i < 4; i++) {
        colInterpolates[i] = cubicInterpolate(patch[i][0], patch[i][1], patch[i][2], patch[i][3], yFraction);
    }
    return cubicInterpolate(colInterpolates[0], colInterpolates[1], colInterpolates[2], colInterpolates[3], xFraction);
}

static void bicubicResize(unsigned char* Src, unsigned char* Dest, int srcWidth, int srcHeight, int dstWidth, int dstHeight, int Channels) {
    float xRatio = (float)srcWidth / dstWidth;
    float yRatio = (float)srcHeight / dstHeight;
    for (int dstY = 0; dstY < dstHeight; dstY++) {
        for (int dstX = 0; dstX < dstWidth; dstX++) {
            float srcX = xRatio * dstX;
            float srcY = yRatio * dstY;
            int srcXInt = (int)srcX;
            int srcYInt = (int)srcY;
            float xFraction = srcX - srcXInt;
            float yFraction = srcY - srcYInt;

            for (int channel = 0; channel < Channels; channel++) {
                float patch[4][4] = { 0 };
                for (int m = -1; m < 3; m++) {
                    for (int n = -1; n < 3; n++) {
                        int srcXM = srcXInt + m;
                        int srcYN = srcYInt + n;
                        srcXM = srcXM < 0 ? 0 : (srcXM >= srcWidth ? srcWidth - 1 : srcXM);
                        srcYN = srcYN < 0 ? 0 : (srcYN >= srcHeight ? srcHeight - 1 : srcYN);
                        patch[m + 1][n + 1] = Src[(srcYN * srcWidth + srcXM) * Channels + channel];
                    }
                }

                float interpolatedValue = bicubicInterpolate(patch, xFraction, yFraction);
                Dest[(dstY * dstWidth + dstX) * Channels + channel] = (unsigned char)fmaxf(0.0f, fminf(255.0f, interpolatedValue));
            }
        }
    }
}

static float bilinearInterpolate(unsigned char topLeft, unsigned char topRight, 
                                unsigned char bottomLeft, unsigned char bottomRight,
                                float xFraction, float yFraction) {
    return topLeft * (1 - xFraction) * (1 - yFraction) + 
           topRight * xFraction * (1 - yFraction) +
           bottomLeft * yFraction * (1 - xFraction) + 
           bottomRight * xFraction * yFraction;
}

static void bilinearResize(unsigned char* Src, unsigned char* Dest, int srcWidth, int srcHeight, int dstWidth, int dstHeight, int Channels) {
    float xRatio = (float)(srcWidth - 1) / dstWidth;
    float yRatio = (float)(srcHeight - 1) / dstHeight;
    for (int dstY = 0; dstY < dstHeight; dstY++) {
        for (int dstX = 0; dstX < dstWidth; dstX++) {
            float srcX = xRatio * dstX;
            float srcY = yRatio * dstY;
            int srcXInt = (int)srcX;
            int srcYInt = (int)srcY;
            float xFraction = srcX - srcXInt;
            float yFraction = srcY - srcYInt;

            for (int channel = 0; channel < Channels; channel++) {
                unsigned char topLeft = Src[(srcYInt * srcWidth + srcXInt) * Channels + channel];
                unsigned char topRight = Src[(srcYInt * srcWidth + srcXInt + 1) * Channels + channel];
                unsigned char bottomLeft = Src[((srcYInt + 1) * srcWidth + srcXInt) * Channels + channel];
                unsigned char bottomRight = Src[((srcYInt + 1) * srcWidth + srcXInt + 1) * Channels + channel];

                float interpolatedValue = bilinearInterpolate(topLeft, topRight, bottomLeft, bottomRight,
                                                            xFraction, yFraction);

                Dest[(dstY * dstWidth + dstX) * Channels + channel] = (unsigned char)interpolatedValue;
            }
        }
    }
}

static void lanzcosResize(unsigned char* Input, unsigned int Width, unsigned int Height, unsigned int Stride, unsigned char* Output,
                          int newWidth, int newHeight, int dstStride) {

    int Channels = Stride / Width;
    int dstOffset = dstStride - Channels * newWidth;
    float xFactor = (float)Width / newWidth;
    float yFactor = (float)Height / newHeight;

    int ymax = Height - 1;
    int xmax = Width - 1;

    for (int y = 0; y < newHeight; y++) {
        float oy = (float)y * yFactor;
        int oy1 = (int)oy;
        int oy2 = (oy1 == ymax) ? oy1 : oy1 + 1;
        float dy1 = oy - (float)oy1;
        float dy2 = 1.0f - dy1;

        unsigned char* tp1 = Input + oy1 * Stride;
        unsigned char* tp2 = Input + oy2 * Stride;

        for (int x = 0; x < newWidth; x++) {
            float ox = (float)x * xFactor;
            int ox1 = (int)ox;
            int ox2 = (ox1 == xmax) ? ox1 : ox1 + 1;
            float dx1 = ox - (float)ox1;
            float dx2 = 1.0f - dx1;
            unsigned char* p1 = tp1 + ox1 * Channels;
            unsigned char* p2 = tp1 + ox2 * Channels;
            unsigned char* p3 = tp2 + ox1 * Channels;
            unsigned char* p4 = tp2 + ox2 * Channels;

            for (int i = 0; i < Channels; i++, Output++, p1++, p2++, p3++, p4++) {
                *Output = (unsigned char)(dy2 * (dx2 * (*p1) + dx1 * (*p2)) + dy1 * (dx2 * (*p3) + dx1 * (*p4)));
            }
        }
        Output += dstOffset;
    }
}

static void bilinearRotate(unsigned char* Input, int Width, int Height, int Stride, unsigned char* Output, int newWidth, int newHeight,
                               float angle, bool useTransparency, unsigned char fillColorR, unsigned char fillColorG, unsigned char fillColorB) {

    int Channels = Stride / Width;
    float oldXradius = (float)(Width - 1) / 2;
    float oldYradius = (float)(Height - 1) / 2;
    float newXradius = (float)(newWidth - 1) / 2;
    float newYradius = (float)(newHeight - 1) / 2;

    float angleRad = -angle * M_PI / 180.0f;
    float angleCos = fastCos(angleRad);
    float angleSin = fastSin(angleRad);

    int lastHeight = Height - 1;    
    int lastWidth = Width - 1;
    int dstOffset = newWidth * Channels - newWidth * Channels;

    int outputChannels = (Channels == 1 && useTransparency) ? 2 : (useTransparency ? 4 : Channels);

    // Set up fill colors based on number of channels and transparency setting
    unsigned char fillColors[4] = { fillColorR, fillColorG, fillColorB, 255 };
    if (Channels == 1) {
        fillColors[0] = fillColorR; // Use R value for grayscale
    }
    if (useTransparency) {
        fillColors[3] = 0; // Alpha channel for out-of-bounds
    }

    float cy = -newYradius;
    for (int y = 0; y < newHeight; y++) {
        const float tx = angleSin * cy + oldXradius;
        const float ty = angleCos * cy + oldYradius;

        float cx = -newXradius;
        for (int x = 0; x < newWidth; x++) {
            // Calculate source pixel position
            const float ox = tx + angleCos * cx;
            const float oy = ty - angleSin * cx;
            const int ox1 = (int)ox;
            const int oy1 = (int)oy;

            // Check if position is valid
            if ((ox1 < 0) || (oy1 < 0) || (ox1 >= Width) || (oy1 >= Height)) {
                // If we have an alpha channel and useTransparency is true, set it to 0
                if (useTransparency) {
                    Output[(y * newWidth + x) * outputChannels + (outputChannels - 1)] = fillColors[3];
                } else {
                    // Fill with background color
                    for (int c = 0; c < outputChannels; c++) {
                        Output[(y * newWidth + x) * outputChannels + c] = fillColors[c];
                    }
                }
            } else {

                // Calculate source pixel position
                const float ox = tx + angleCos * cx;
                const float oy = ty - angleSin * cx;
                const int ox1 = (int)ox;
                const int oy1 = (int)oy;

                // Get mirrored coordinates for interpolation
                int x1 = mirrorCoord(ox1, Width);
                int x2 = mirrorCoord(ox1 + 1, Width);
                int y1 = mirrorCoord(oy1, Height);
                int y2 = mirrorCoord(oy1 + 1, Height);

                float xFraction = ox - ox1;
                float yFraction = oy - oy1;

                // Interpolate each channel
                for (int c = 0; c < Channels; c++) {
                    unsigned char topLeft = Input[(y1 * Width + x1) * Channels + c];
                    unsigned char topRight = Input[(y1 * Width + x2) * Channels + c];
                    unsigned char bottomLeft = Input[(y2 * Width + x1) * Channels + c];
                    unsigned char bottomRight = Input[(y2 * Width + x2) * Channels + c];

                    Output[(y * newWidth + x) * outputChannels + c] =
                            (unsigned char)bilinearInterpolate(topLeft, topRight, bottomLeft, bottomRight, xFraction, yFraction);
                }

                // Set the alpha channel if transparency is used
                if (Channels <= 3 && useTransparency) {
                    Output[(y * newWidth + x) * outputChannels + (outputChannels - 1)] = 255; // Fully opaque
                }
            }
            cx++;
        }
        cy++;
    }
}

static void nearestNeighborRotate(unsigned char* Input, int Width, int Height, int Stride, unsigned char* Output, int newWidth, int newHeight,
                               float angle, bool useTransparency, unsigned char fillColorR, unsigned char fillColorG, unsigned char fillColorB) {

    int Channels = Stride / Width;
    float oldXradius = (float)(Width - 1) / 2;
    float oldYradius = (float)(Height - 1) / 2;
    float newXradius = (float)(newWidth - 1) / 2;
    float newYradius = (float)(newHeight - 1) / 2;

    float angleRad = -angle * M_PI / 180.0f;
    float angleCos = fastCos(angleRad);
    float angleSin = fastSin(angleRad);

    int lastHeight = Height - 1;    
    int lastWidth = Width - 1;

    int outputChannels = (Channels == 1 && useTransparency) ? 2 : (useTransparency ? 4 : Channels);

    // Set up fill colors based on number of channels and transparency setting
    unsigned char fillColors[4] = { fillColorR, fillColorG, fillColorB, 255 };
    if (Channels == 1) {
        fillColors[0] = fillColorR; // Use R value for grayscale
    }
    if (useTransparency) {
        fillColors[3] = 0; // Alpha channel for out-of-bounds
    }

    float cy = -newYradius;
    for (int y = 0; y < newHeight; y++) {
        const float tx = angleSin * cy + oldXradius;
        const float ty = angleCos * cy + oldYradius;

        float cx = -newXradius;
        for (int x = 0; x < newWidth; x++) {
            // Calculate source pixel position
            const float ox = tx + angleCos * cx;
            const float oy = ty - angleSin * cx;
            
            // Round to nearest neighbor (add 0.5 for proper rounding)
            const int ox1 = (int)(ox + 0.5f);
            const int oy1 = (int)(oy + 0.5f);

            // Check if position is valid
            if ((ox1 < 0) || (oy1 < 0) || (ox1 >= Width) || (oy1 >= Height)) {
                // If we have an alpha channel and useTransparency is true, set it to 0
                if (useTransparency) {
                    Output[(y * newWidth + x) * outputChannels + (outputChannels - 1)] = fillColors[3];
                } else {
                    // Fill with background color
                    for (int c = 0; c < outputChannels; c++) {
                        Output[(y * newWidth + x) * outputChannels + c] = fillColors[c];
                    }
                }
            } else {
                // Use mirror coordinates for edge handling
                int srcX = mirrorCoord(ox1, Width);
                int srcY = mirrorCoord(oy1, Height);

                // Copy pixel directly from source (nearest neighbor)
                for (int c = 0; c < Channels; c++) {
                    Output[(y * newWidth + x) * outputChannels + c] = Input[(srcY * Width + srcX) * Channels + c];
                }

                // Set the alpha channel if transparency is used
                if (Channels <= 3 && useTransparency) {
                    Output[(y * newWidth + x) * outputChannels + (outputChannels - 1)] = 255; // Fully opaque
                }
            }
            cx++;
        }
        cy++;
    }
}

static void bicubicRotate(unsigned char* Input, int Width, int Height, int Stride, unsigned char* Output, int newWidth, int newHeight,
                           float angle, bool useTransparency, unsigned char fillColorR, unsigned char fillColorG, unsigned char fillColorB) {

    int Channels = Stride / Width;
    float oldXradius = (float)(Width - 1) / 2;
    float oldYradius = (float)(Height - 1) / 2;
    float newXradius = (float)(newWidth - 1) / 2;
    float newYradius = (float)(newHeight - 1) / 2;

    float angleRad = -angle * M_PI / 180.0f;
    float angleCos = fastCos(angleRad);
    float angleSin = fastSin(angleRad);

    int lastHeight = Height - 1;
    int lastWidth = Width - 1;
    int dstOffset = newWidth * Channels - newWidth * Channels;

    int outputChannels = (Channels == 1 && useTransparency) ? 2 : (useTransparency ? 4 : Channels);

    // Set up fill colors based on number of channels and transparency setting
    unsigned char fillColors[4] = { fillColorR, fillColorG, fillColorB, 255 };
    if (Channels == 1) {
        fillColors[0] = fillColorR; // Use R value for grayscale
    }
    if (useTransparency) {
        fillColors[3] = 0; // Alpha channel for out-of-bounds
    }

    float cy = -newYradius;
    for (int y = 0; y < newHeight; y++) {
        const float tx = angleSin * cy + oldXradius;
        const float ty = angleCos * cy + oldYradius;

        float cx = -newXradius;
        for (int x = 0; x < newWidth; x++) {
            // Calculate source pixel position
            const float ox = tx + angleCos * cx;
            const float oy = ty - angleSin * cx;
            const int ox1 = (int)ox;
            const int oy1 = (int)oy;

            // Check if we have enough pixels around for bicubic interpolation
            if ((ox1 < 0) || (oy1 < 0) || (ox1 >= Width) || (oy1 >= Height)) {
                // If we have an alpha channel and useTransparency is true, set it to 0
                if (useTransparency) {
                    Output[(y * newWidth + x) * outputChannels + (outputChannels - 1)] = fillColors[3];
                    // Fill color channels with background color
                    for (int c = 0; c < Channels; c++) {
                        Output[(y * newWidth + x) * outputChannels + c] = fillColors[c];
                    }
                } else {
                    // Fill with background color
                    for (int c = 0; c < outputChannels; c++) {
                        Output[(y * newWidth + x) * outputChannels + c] = fillColors[c];
                    }
                }
            } else {
                float xFraction = ox - ox1;
                float yFraction = oy - oy1;

                // Interpolate each channel
                for (int c = 0; c < Channels; c++) {
                    float patch[4][4] = { 0 };
                    // Fill the 4x4 patch for bicubic interpolation
                    for (int m = -1; m < 3; m++) {
                        for (int n = -1; n < 3; n++) {
                            int srcX = ox1 + m;
                            int srcY = oy1 + n;

                            // Only mirror if we're actually out of bounds
                            if (srcX < 0 || srcX >= Width) {
                                srcX = mirrorCoord(srcX, Width);
                            }
                            if (srcY < 0 || srcY >= Height) {
                                srcY = mirrorCoord(srcY, Height);
                            }
                            
                            patch[m + 1][n + 1] = Input[(srcY * Width + srcX) * Channels + c];
                        }
                    }

                    float interpolatedValue = bicubicInterpolate(patch, xFraction, yFraction);
                    Output[(y * newWidth + x) * outputChannels + c] = 
                        (unsigned char)fmaxf(0.0f, fminf(255.0f, interpolatedValue));
                }

                // Set the alpha channel if transparency is used
                if (Channels <= 3 && useTransparency) {
                    Output[(y * newWidth + x) * outputChannels + (outputChannels - 1)] = 255; // Fully opaque
                }
            }
            cx++;
        }
        cy++;
    }
}


#endif  /* OCULAR_INTERPOLATE_H */
