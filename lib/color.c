/**
 * @file: color.c
 * @author Warren Galyen
 * Created: 2-12-2024
 * Last Updated: 2-12-2024
 * Last update: migrate functions
 *
 * @brief Ocular color conversion implementations
 */

#include "color.h"
#include "util.h"

void rgb2yiq(unsigned char* R, unsigned char* G, unsigned char* B, short* Y, short* I, short* Q) {
    *Y = (short)((int)(0.299f * 65536) * *R + (int)(0.587f * 65536) * *G + (int)(0.114f * 65536) * *B) >> 16;
    *I = (short)((int)(0.595f * 65536) * *R - (int)(0.274453f * 65536) * *G - (int)(0.321263f * 65536) * *B) >> 16;
    *Q = (short)((int)(0.211456f * 65536) * *R - (int)(0.522591f * 65536) * *G + (int)(0.311135f * 65536) * *B) >> 16;
}

void yiq2rgb(short* Y, short* I, short* Q, unsigned char* R, unsigned char* G, unsigned char* B) {
    *R = ClampToByte((int)(Y + ((((int)(0.9563 * 65536)) * (*I)) + ((int)(0.6210 * 65536)) * (*Q))) >> 16);
    *G = ClampToByte((int)(Y - ((((int)(0.2721 * 65536)) * (*I)) + ((int)(0.6474 * 65536)) * (*Q))) >> 16);
    *B = ClampToByte((int)(Y + ((((int)(1.7046 * 65536)) * (*Q)) - ((int)(1.1070 * 65536)) * (*I))) >> 16);
}

void rgb2hsl(float r, float g, float b, float* h, float* s, float* l) {

    r /= 255;
    g /= 255;
    b /= 255;

    float max = max(max(r, g), b);
    float min = min(min(r, g), b);

    *h = *s = *l = (max + min) / 2;

    if (max == min) {
        h = s = 0; // achromatic
    } else {
        float d = max - min;
        *s = (*l > 0.5) ? d / (2 - max - min) : d / (max + min);

        if (max == r) {
            *h = (g - b) / d + (g < b ? 6 : 0);
        } else if (max == g) {
            *h = (b - r) / d + 2;
        } else if (max == b) {
            *h = (r - g) / d + 4;
        }

        *h /= 6;
    }
}

/*
 * Converts an HUE to r, g or b.
 * returns float in the set [0, 1].
 */
float hue2rgb(float p, float q, float t) {

    if (t < 0)
        t += 1;
    if (t > 1)
        t -= 1;
    if (t < 1. / 6)
        return p + (q - p) * 6 * t;
    if (t < 1. / 2)
        return q;
    if (t < 2. / 3)
        return p + (q - p) * (2. / 3 - t) * 6;

    return p;
}

void hsl2rgb(float h, float s, float l, unsigned char* r, unsigned char* g, unsigned* b) {

    if (s == 0) {
        *r = *g = *b = l; // achromatic
    } else {
        float q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        float p = 2 * l - q;
        *r = hue2rgb(p, q, h + 1. / 3) * 255;
        *g = hue2rgb(p, q, h) * 255;
        *b = hue2rgb(p, q, h - 1. / 3) * 255;
    }
}


void rgb2hsv(const unsigned char* R, const unsigned char* G, const unsigned char* B, unsigned char* H, unsigned char* S, unsigned char* V) {
    int r = *R;
    int g = *G;
    int b = *B;

    int h, s;
    int nMax = max3(r, g, b);
    int nMin = min3(r, g, b);
    int diff = nMax - nMin;

    if (diff == 0) {
        h = 0;
        s = 0;
    } else {
        if (nMin == b) {
            h = 60 * (g - r) / diff + 60;
        } else if (nMin == r) {
            h = 60 * (b - g) / diff + 180;
        } else {
            h = 60 * (r - b) / diff + 300;
        }
        // normalize 0-359°
        // if (h < 0)    h += 360; if (h >= 360) h -= 360;
        if (!((unsigned)(int)(h) < (360))) {
            if (h < 0)
                h += 360;
            else
                h -= 360;
        }
        if (nMax == 0) {
            s = 0;
        } else {
            s = 255 * diff / nMax;
        }
    }

    *H = (unsigned char)(h >> 1); // 0-179
    *S = (unsigned char)s;        // 0-255
    *V = (unsigned char)nMax;     // 0-255
}

void hsv2rgb(const unsigned char* H, const unsigned char* S, const unsigned char* V, unsigned char* R, unsigned char* G, unsigned char* B) {

    if (S > 0) {
        int r, g, b;
        r = *V;
        g = *V;
        b = *V;
        float h = *H * (6.0f / 180.0f); // 0-180° -> 0.0-1.0
        int i = (int)h;
        int f = (int)(256 * (h - (float)i));
        int VS = (*V * *S) >> 8;
        int VSF = VS * f;
        switch (i) {
        case 0:
            b -= VS;
            g = b + (VSF >> 8);
            break;
        case 1:
            r = *V - (VSF >> 8);
            b -= VS;
            break;
        case 2:
            r -= VS;
            b = r + (VSF >> 8);
            break;
        case 3:
            r -= VS;
            g -= (VSF >> 8);
            break;
        case 4:
            g -= VS;
            r = g + (VSF >> 8);
            break;
        case 5:
            g -= VS;
            b -= (VSF >> 8);
            break;
        default: break;
        }
        *R = (unsigned char)(r);
        *G = (unsigned char)(g);
        *B = (unsigned char)(b);
    } else {
        *R = *V;
        *G = *V;
        *B = *V;
    }
}

void rgb2ycbcr(unsigned char R, unsigned char G, unsigned char B, unsigned char* y, unsigned char* cb, unsigned char* cr) {
    *y = (unsigned char)((19595 * R + 38470 * G + 7471 * B) >> 16);
    *cb = (unsigned char)(((36962 * (B - *y)) >> 16) + 128);
    *cr = (unsigned char)(((46727 * (R - *y)) >> 16) + 128);
}

void ycbcr2rgb(unsigned char y, unsigned char Cb, unsigned char Cr, unsigned char* R, unsigned char* G, unsigned char* B) {
    int y_fixed = (y << 20) + (1 << 19); // rounding
    int r, g, b;
    int cr = Cr - 128;
    int cb = Cb - 128;
    r = y_fixed + cr * float2fixed(1.40200f);
    g = y_fixed + (cr * -float2fixed(0.71414f)) + ((cb * -float2fixed(0.34414f)) * 0xffff0000);
    b = y_fixed + cb * float2fixed(1.77200f);
    r >>= 20;
    g >>= 20;
    b >>= 20;
    if ((unsigned)r > 255) {
        if (r < 0)
            r = 0;
        else
            r = 255;
    }
    if ((unsigned)g > 255) {
        if (g < 0)
            g = 0;
        else
            g = 255;
    }
    if ((unsigned)b > 255) {
        if (b < 0)
            b = 0;
        else
            b = 255;
    }
    *R = (unsigned char)r;
    *G = (unsigned char)g;
    *B = (unsigned char)b;
}
