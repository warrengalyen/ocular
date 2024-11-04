/**
 * @file: color.h
 * @author Warren Galyen
 * Created: 2-12-2024
 * Last Updated: 10-28-2024
 * Last update: added CMYK <--> RGB conversion
 *
 * @brief Ocular color conversion functions
 */

#ifndef OCULAR_COLOR_H
#define OCULAR_COLOR_H


/** @brief RGB to YIQ color space conversion.
 * @ingroup group_color_convert
 */
void rgb2yiq(unsigned char* R, unsigned char* G, unsigned char* B, short* Y, short* I, short* Q);

/** @brief YIQ to RGB color space conversion.
 * @ingroup group_color_convert
 */
void yiq2rgb(short* Y, short* I, short* Q, unsigned char* R, unsigned char* G, unsigned char* B);

/**
 * @brief RGB to HSL color space conversion.
 * @ingroup group_color_convert
 */
void rgb2hsl(float r, float g, float b, float* h, float* s, float* l);

/**
 * @brief HSL to RGB color space conversion.
 * @ingroup group_color_convert
 */
void hsl2rgb(float h, float s, float l, float* r, float* g, float* b);

/** @brief RGB to HSV color space conversion.
 * @ingroup group_color_convert
 */
void rgb2hsv(unsigned char R, unsigned char G, unsigned char B, unsigned char* H, unsigned char* S, unsigned char* V);

/** @brief HSV to RGB color space conversion.
 * @ingroup group_color_convert
 */
void hsv2rgb(unsigned char H, unsigned char S, unsigned char V, unsigned char* R, unsigned char* G, unsigned char* B);

/** @brief RGB to YCbCr color space conversion.
 * @ingroup group_color_convert
 */
void rgb2ycbcr(unsigned char R, unsigned char G, unsigned char B, unsigned char* y, unsigned char* cb, unsigned char* cr);

/** @brief YCbCr to RGB color space conversion.
 * @ingroup group_color_convert
 */
void ycbcr2rgb(unsigned char y, unsigned char Cb, unsigned char Cr, unsigned char* R, unsigned char* G, unsigned char* B);

/** @brief RGB to CMYK color space conversion.
 * @ingroup group_color_convert
 */
void rgb2cmyk(unsigned char R, unsigned char G, unsigned char B, float* c, float* m, float* y, float* k);

/** @brief CMYK to RGB color space conversion.
 * @ingroup group_color_convert
 */
void cmyk2rgb(float c, float m, float y, float k, unsigned char* R, unsigned char* G, unsigned char* B);

/** @brief CIELAB to RGB color space conversion.
 * @ingroup group_color_convert
 */
void lab2rgb(double L, double a, double b, unsigned char* R, unsigned char* G, unsigned char* B);

/** @brief RGB to CIELAB color space conversion.
 * @ingroup group_color_convert
 */
void rgb2lab(unsigned char R, unsigned char G, unsigned char B, double* L, double* a, double* b);

#endif  /* OCULAR_COLOR_H */
