#ifndef OCULAR_COLOR_H
#define OCULAR_COLOR_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1310)
    #pragma warning(disable : 4996) // VS doesn't like fopen, but fopen_s is not
                                    // standard C so unusable here
#endif                              /*_MSC_VER */

#include "dlib_export.h"

    //--------------------------Color conversion--------------------------

    DLIB_EXPORT void rgb2yiq(unsigned char* R, unsigned char* G, unsigned char* B, short* Y, short* I, short* Q);

    DLIB_EXPORT void yiq2rgb(short* Y, short* I, short* Q, unsigned char* R, unsigned char* G, unsigned char* B);

    DLIB_EXPORT void
    rgb2hsv(const unsigned char* R, const unsigned char* G, const unsigned char* B, unsigned char* H, unsigned char* S, unsigned char* V);

    DLIB_EXPORT void
    hsv2rgb(const unsigned char* H, const unsigned char* S, const unsigned char* V, unsigned char* R, unsigned char* G, unsigned char* B);

    DLIB_EXPORT void rgb2ycbcr(unsigned char R, unsigned char G, unsigned char B, unsigned char* y, unsigned char* cb, unsigned char* cr);

    DLIB_EXPORT void ycbcr2rgb(unsigned char y, unsigned char Cb, unsigned char Cr, unsigned char* R, unsigned char* G, unsigned char* B);

    //--------------------------Color conversion--------------------------

#ifdef __cplusplus
}
#endif
#endif /* OCULAR_COLOR_H */