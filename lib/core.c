#include "core.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#include <malloc.h>
#endif

void* AllocMemory(unsigned int Size, bool ZeroMemory) {
    
    void* ptr = _mm_malloc(Size, 32);
    if (ptr != NULL)
        if (ZeroMemory == true)
            memset(ptr, 0, Size);

    return ptr;
}

void FreeMemory(void* ptr) {
    if (ptr != NULL)
        _mm_free(ptr);
}

int OC_ELEMENT_SIZE(int Depth) {
    int Size;
    switch (Depth) {
        case OC_DEPTH_8U: Size = sizeof(unsigned char); break;
        case OC_DEPTH_8S: Size = sizeof(char); break;
        case OC_DEPTH_16S: Size = sizeof(short); break;
        case OC_DEPTH_32S: Size = sizeof(int); break;
        case OC_DEPTH_32F: Size = sizeof(float); break;
        case OC_DEPTH_64F: Size = sizeof(double); break;
        default: Size = 0; break;
    }
    return Size;
}

OC_STATUS ocularCreateImage(int Width, int Height, int Depth, int Channels, OcImage** image) {

    if (Width < 1 || Height < 1)
        return OC_STATUS_ERR_INVALIDPARAMETER;
    if (Channels != 1 && Channels != 2 && Channels != 3 && Channels != 4)
        return OC_STATUS_ERR_INVALIDPARAMETER;
    *image = (OcImage*)AllocMemory(sizeof(OcImage), false);
    (*image)->Width = Width;
    (*image)->Height = Height;
    (*image)->Depth = Depth;
    (*image)->Channels = Channels;
    (*image)->Stride = WIDTHBYTES(Width * Channels * OC_ELEMENT_SIZE(Depth));
    (*image)->Data = (unsigned char*)AllocMemory((*image)->Height * (*image)->Stride, true);
    if ((*image)->Data == NULL) {
        FreeMemory(*image);
        return OC_STATUS_ERR_OUTOFMEMORY;
    }
    (*image)->Reserved = 0;
    return OC_STATUS_OK;
}

OC_STATUS ocularFreeImage(OcImage** image) {
    if ((*image) == NULL)
        return OC_STATUS_ERR_NULLREFERENCE;
    if ((*image)->Data == NULL) {
        FreeMemory((*image));
        return OC_STATUS_ERR_OUTOFMEMORY;
    } else {
        // Release in proper order
        FreeMemory((*image)->Data);
        FreeMemory((*image));
        return OC_STATUS_OK;
    }
}

OC_STATUS ocularCloneImage(OcImage* Input, OcImage** Output) {
    if (Input == NULL)
        return OC_STATUS_ERR_NULLREFERENCE;
    if (Input->Data == NULL)
        return OC_STATUS_ERR_NULLREFERENCE;
    OC_STATUS ret = ocularCreateImage(Input->Width, Input->Height, Input->Depth, Input->Channels, Output);
    if (ret == OC_STATUS_OK)
        memcpy((*Output)->Data, Input->Data, (*Output)->Height * (*Output)->Stride);
    return ret;
}

OC_STATUS ocularTransposeImage(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride) {
    int Channels = Stride / Width;

    if (Input == NULL || Output == NULL)
        return OC_STATUS_ERR_NULLREFERENCE;
    if (Channels != 1 && Channels != 3 && Channels != 4)
        return OC_STATUS_ERR_NOTSUPPORTED;

    int OutStride = Height * Channels;

    for (int i = 0; i < Height; i++) {
        for (int j = 0; j < Width; j++) {
            for (int c = 0; c < Channels; c++) {
                Output[j * OutStride + i * Channels + c] = Input[i * Stride + j * Channels + c];
            }
        }
    }

    return OC_STATUS_OK;
}