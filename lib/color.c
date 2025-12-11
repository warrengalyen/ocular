/*
    http://www.easyrgb.com/index.php?X=MATH&H=02text2
    Observer	2° (CIE 1931)	10° (CIE 1964)	Note
    Illuminant	X2	Y2	Z2	X10	Y10	Z10
    A	109.850	100.000	35.585	111.144	100.000	35.200	Incandescent/tungsten
    B	99.0927	100.000	85.313	99.178;	100.000	84.3493	Old direct sunlight at noon
    C	98.074	100.000	118.232	97.285	100.000	116.145	Old daylight
    D50	96.422	100.000	82.521	96.720	100.000	81.427	ICC profile PCS
    D55	95.682	100.000	92.149	95.799	100.000	90.926	Mid-morning daylight
    D65	95.047	100.000	108.883	94.811	100.000	107.304	Daylight, sRGB, Adobe-RGB
    D75	94.972	100.000	122.638	94.416	100.000	120.641	North sky daylight
    E	100.000	100.000	100.000	100.000	100.000	100.000	Equal energy
    F1	92.834	100.000	103.665	94.791	100.000	103.191	Daylight Fluorescent
    F2	99.187	100.000	67.395	103.280	100.000	69.026	Cool fluorescent
    F3	103.754	100.000	49.861	108.968	100.000	51.965	White Fluorescent
    F4	109.147	100.000	38.813	114.961	100.000	40.963	Warm White Fluorescent
    F5	90.872	100.000	98.723	93.369	100.000	98.636	Daylight Fluorescent
    F6	97.309	100.000	60.191	102.148	100.000	62.074	Lite White Fluorescent
    F7	95.044	100.000	108.755	95.792	100.000	107.687	Daylight fluorescent, D65 simulator
    F8	96.413	100.000	82.333	97.115	100.000	81.135	Sylvania F40, D50 simulator
    F9	100.365	100.000	67.868	102.116	100.000	67.826	Cool White Fluorescent
    F10	96.174	100.000	81.712	99.001	100.000	83.134	Ultralume 50, Philips TL85
    F11	100.966	100.000	64.370	103.866	100.000	65.627	Ultralume 40, Philips TL84
    F12	108.046	100.000	39.228	111.428	100.000	40.353	Ultralume 30, Philips TL83
*/


#include "color.h"
#include "util.h"
#include <math.h>

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
        *h = *s = 0; // achromatic
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

void hsl2rgb(float h, float s, float l, float* r, float* g, float* b) {

    if (s == 0) {
        *r = *g = *b = l * 255 + 0.5; // achromatic
    } else {
        float q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        float p = 2 * l - q;
        *r = hue2rgb(p, q, h + 1. / 3) * 255 + 0.5;
        *g = hue2rgb(p, q, h) * 255 + 0.5;
        *b = hue2rgb(p, q, h - 1. / 3) * 255 + 0.5;
    }
}


void rgb2hsv(const unsigned char R, const unsigned char G, const unsigned char B, unsigned char* H, unsigned char* S, unsigned char* V) {
    int r = R;
    int g = G;
    int b = B;

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

void hsv2rgb(const unsigned char H, const unsigned char S, const unsigned char V, unsigned char* R, unsigned char* G, unsigned char* B) {

    if (S > 0) {
        int r, g, b;
        r = V;
        g = V;
        b = V;
        float h = H * (6.0f / 180.0f); // 0-180° -> 0.0-1.0
        int i = (int)h;
        int f = (int)(256 * (h - (float)i));
        int VS = (V * S) >> 8;
        int VSF = VS * f;
        switch (i) {
        case 0:
            b -= VS;
            g = b + (VSF >> 8);
            break;
        case 1:
            r = V - (VSF >> 8);
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
        *R = V;
        *G = V;
        *B = V;
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
    g = y_fixed + (cr * -float2fixed(0.71414f)) + ((cb * -float2fixed(0.34414f)) & 0xffff0000);
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

void cmyk2rgb(float c, float m, float y, float k, unsigned char* R, unsigned char* G, unsigned char* B) {
    // Convert from CMYK values (0-1.0) to RGB (0-255)
    float r = 255.0f * (1.0f - c) * (1.0f - k);
    float g = 255.0f * (1.0f - m) * (1.0f - k);
    float b = 255.0f * (1.0f - y) * (1.0f - k);
    
    // Round and clamp values to valid byte range
    *R = (unsigned char)(r + 0.5f);
    *G = (unsigned char)(g + 0.5f);
    *B = (unsigned char)(b + 0.5f);
}

void rgb2cmyk(unsigned char R, unsigned char G, unsigned char B, float* c, float* m, float* y, float* k) {
    // Convert RGB (0-255) to floating point (0-1)
    float r = R / 255.0f;
    float g = G / 255.0f;
    float b = B / 255.0f;
    
    // Find k (black)
    *k = 1.0f - fmaxf(r, fmaxf(g, b));
    
    // Handle pure black case
    if (*k == 1.0f) {
        *c = 0.0f;
        *m = 0.0f;
        *y = 0.0f;
    } else {
        // Calculate CMY values
        *c = (1.0f - r - *k) / (1.0f - *k);
        *m = (1.0f - g - *k) / (1.0f - *k);
        *y = (1.0f - b - *k) / (1.0f - *k);
    }
}

void rgb2xyz(unsigned char R, unsigned char G, unsigned char B, double* X, double* Y, double* Z) {

    double var_R = (double)R / 255.0;
    double var_G = (double)G / 255.0;
    double var_B = (double)B / 255.0;

    if (var_R > 0.04045f)   
        var_R = pow(((var_R + 0.055f) / 1.055f), 2.4f);
    else
        var_R = var_R / 12.92f;

    if (var_G > 0.04045)
        var_G = pow(((var_G + 0.055f) / 1.055f), 2.4f);
    else
        var_G = var_G / 12.92f;

    if (var_B > 0.04045f)
        var_B = pow(((var_B + 0.055f) / 1.055f), 2.4f);
    else
        var_B = var_B / 12.92f;

    var_R *= 100.0;
    var_G *= 100.0;
    var_B *= 100.0;


    // Observer. = 2°, Illuminant = D65
    *X = var_R * 0.4124564 + var_G * 0.3575761 + var_B * 0.1804375;
    *Y = var_R * 0.2126729 + var_G * 0.7151522 + var_B * 0.0721750;
    *Z = var_R * 0.0193339 + var_G * 0.1191920 + var_B * 0.9503041;
}

void lab2xyz(double L, double a, double b, double* X, double* Y, double* Z) {

    double ref_X = 95.047;
    double ref_Y = 100.000;
    double ref_Z = 108.883;

    double var_Y = (L + 16.0) / 116.0;
    double var_X = a / 500.0 + var_Y;
    double var_Z = var_Y - b / 200.0;

    const double eps = pow(6.0 / 29.0, 3);
    const double m = 1.0 / 3.0 * pow(eps, -2);
    const double c = 4.0 / 29.0;

    if (pow(var_Y, 3) > eps) {
        var_Y = pow(var_Y, 3);
    } else {
        var_Y = (var_Y - c) / m;
    }

    if (pow(var_X, 3) > eps) {
        var_X = pow(var_X, 3);
    } else {
        var_X = (var_X - c) / m;
    }

    if (pow(var_Z, 3) > eps) {
        var_Z = pow(var_Z, 3);
    } else {
        var_Z = (var_Z - c) / m;
    }

    *X = ref_X * var_X; // ref_X =  95.047     Observer= 2°, Illuminant= D65
    *Y = ref_Y * var_Y; // ref_Y = 100.000
    *Z = ref_Z * var_Z; // ref_Z = 108.883
}

void xyz2lab(double X, double Y, double Z, double* L, double* a, double* b) {

    // See table above
    double ref_X = 95.047;
    double ref_Y = 100.0;
    double ref_Z = 108.883;

    double var_X = X / ref_X;
    double var_Y = Y / ref_Y;
    double var_Z = Z / ref_Z;

    const double eps = pow(6.0 / 29.0, 3);     // 0.008856
    const double m = 1.0 / 3.0 * pow(eps, -2); // 7.787
    const double c = 4.0 / 29.0;               // 16.0/116

    if (var_X > eps) {
        var_X = pow(var_X, 1.0 / 3.0);
    } else {
        var_X = m * var_X + c;
    }

    if (var_Y > eps) {
        var_Y = pow(var_Y, 1.0 / 3.0);
    } else {
        var_Y = m * var_Y + c;
    }

    if (var_Z > eps) {
        var_Z = pow(var_Z, 1.0 / 3.0);
    } else {
        var_Z = m * var_Z + c;
    }

    *L = 116.0 * var_Y - 16.0;
    *a = 500.0 * (var_X - var_Y);
    *b = 200.0 * (var_Y - var_Z);
}

void xyz2rgb(double X, double Y, double Z, unsigned char* R, unsigned char* G, unsigned char* B) {

    double var_X = X / 100.0;
    double var_Y = Y / 100.0;
    double var_Z = Z / 100.0;

    double var_R = var_X * 3.2404542 + var_Y * -1.5371385 + var_Z * -0.4985314;
    double var_G = var_X * -0.9692660 + var_Y * 1.8760108 + var_Z * 0.0415560;
    double var_B = var_X * 0.0556434 + var_Y * -0.2040259 + var_Z * 1.0572252;

    if (var_R > 0.0031308) {
        var_R = 1.055 * pow(var_R, (1.0 / 2.4)) - 0.055;
    } else {
        var_R = 12.92 * var_R;
    }

    if (var_G > 0.0031308) {
        var_G = 1.055 * pow(var_G, (1.0 / 2.4)) - 0.055;
    } else {
        var_G = 12.92 * var_G;
    }

    if (var_B > 0.0031308) {    
        var_B = 1.055 * pow(var_B, (1.0 / 2.4)) - 0.055;
    } else {
        var_B = 12.92 * var_B;
    }

    *R = (unsigned char)(var_R * 255.0 + 0.5);
    *G = (unsigned char)(var_G * 255.0 + 0.5);
    *B = (unsigned char)(var_B * 255.0 + 0.5);
}

void lab2rgb(double L, double a, double b, unsigned char* R, unsigned char* G, unsigned char* B) {
    
    double X, Y, Z;

    lab2xyz(L, a, b, &X, &Y, &Z);

    xyz2rgb(X, Y, Z, R, G, B);

}

void rgb2lab(unsigned char R, unsigned char G, unsigned char B, double* L, double* a, double* b) {
    double X, Y, Z;
    rgb2xyz(R, G, B, &X, &Y, &Z);
    xyz2lab(X, Y, Z, L, a, b);
}
