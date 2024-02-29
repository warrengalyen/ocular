#ifndef OCULAR_AUXILIARY_H
#define OCULAR_AUXILIARY_H

#include "util.h"

OC_STATUS GetValidCoordinate(int Width, int Height, int Left, int Right, int Top, int Bottom, OcEdgeMode Edge, OcImage** Row, OcImage** Col);

OC_STATUS GetExpandImage(unsigned char* Src, unsigned char** Dest, int Left, int Right, int Top, int Bottom, OcEdgeMode Edge);

OC_STATUS SplitRGBA(unsigned char* Input, int Width, int Height, int Stride, OcImage** Red, OcImage** Green, OcImage** Blue, OcImage** Alpha);

OC_STATUS CombineRGBA(unsigned char* Output, int Width, int Height, int Stride, OcImage* Red, OcImage* Green, OcImage* Blue, OcImage* Alpha);

void CopyAlphaChannel(unsigned char* Src, unsigned char* Dest, int Width, int Height, int Stride);

#endif // OCULAR_AUXILIARY_H
