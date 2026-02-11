/**
 * @file: auxiliary.h
 * @author Warren Galyen
 * Created: 2-29-2024
 * Last Updated: 2-29-2024
 * Last update: initial implementation
 *
 * @brief Contains additional functions for image processing 
 *        Mostly unused currently.
 */

#ifndef OCULAR_AUXILIARY_H
#define OCULAR_AUXILIARY_H

#include "util.h"
#include "core.h"

OC_STATUS GetValidCoordinate(int Width, int Height, int Left, int Right, int Top, int Bottom, OcEdgeMode Edge, 
                             OcImage** Row, OcImage** Col);

OC_STATUS GetExpandImage(unsigned char* Src, unsigned char** Dest, int Left, int Right, int Top, int Bottom, 
                          OcEdgeMode Edge);

OC_STATUS SplitRGBA(unsigned char* Input, int Width, int Height, int Stride, OcImage** Red, OcImage** Green, 
                    OcImage** Blue, OcImage** Alpha);

OC_STATUS CombineRGBA(unsigned char* Output, int Width, int Height, int Stride, OcImage* Red, OcImage* Green, 
                      OcImage* Blue, OcImage* Alpha);

void CopyAlphaChannel(unsigned char* Src, unsigned char* Dest, int Width, int Height, int Stride);

// Bilinear interpolation
void GetPixelBilinear(const unsigned char* Input, int Width, int Height, int Stride, float x, float y,
                      OcEdgeMode edgeMode, unsigned char* pixel, int channels);

void GetPixelWithEdgeBehavior(unsigned char* Input, int Width, int Height, int Stride, int x, int y, 
                            OcEdgeMode edgeMode, unsigned char* pixel, int channels);


#endif // OCULAR_AUXILIARY_H
