/**
 * @file: artistic.c
 * @author Warren Galyen
 * Created: 10-9-2026
 * Last Updated: 10-9-2026
 * Last update: added cutout filter
 *
 * @brief Artistic filter implementations
 */

#include "artistic.h"
#include "core.h"
#include <math.h>
#include <stddef.h>
#include <stdlib.h>

/* ---- shared helpers ------------------------------------------------------ */

/* Clamp v to [0,1] (a function, not a macro, so v is evaluated exactly once) */
static inline float ocArtisticClamp01(float v) {
    return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

/* ---- helpers for the colored pencil filter ------------------------------- */

static inline float ocPencilSmoothstep(float e0, float e1, float x) {
    float t = ocArtisticClamp01((x - e0) / (e1 - e0));
    return t * t * (3.0f - 2.0f * t);
}

/* Deterministic lattice hash -> [0,1) */
static inline float ocPencilHash01(int x, int y, unsigned int seed) {
    unsigned int h = (unsigned int)x * 374761393u + (unsigned int)y * 668265263u + seed * 2246822519u;
    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= h >> 16;
    return (float)(h & 0xFFFFFFu) * (1.0f / 16777216.0f);
}

/* Smooth value noise on a unit lattice -> [0,1] */
static inline float ocPencilValueNoise(float x, float y, unsigned int seed) {
    float fx = floorf(x), fy = floorf(y);
    int ix = (int)fx, iy = (int)fy;
    float tx = x - fx, ty = y - fy;
    float sx = tx * tx * (3.0f - 2.0f * tx);
    float sy = ty * ty * (3.0f - 2.0f * ty);
    float a = ocPencilHash01(ix,     iy,     seed);
    float b = ocPencilHash01(ix + 1, iy,     seed);
    float c = ocPencilHash01(ix,     iy + 1, seed);
    float d = ocPencilHash01(ix + 1, iy + 1, seed);
    float top = a + (b - a) * sx;
    float bot = c + (d - c) * sx;
    return top + (bot - top) * sy;
}

/* Luma in [0,1] at (x,y) with clamped borders. Assumes R,G,B channel order. */
static inline float ocPencilLuma(const unsigned char* Input, int x, int y,
                                 int Width, int Height, int Stride, int channels) {
    if (x < 0) x = 0; else if (x >= Width)  x = Width - 1;
    if (y < 0) y = 0; else if (y >= Height) y = Height - 1;
    const unsigned char* p = Input + x * channels + y * Stride;
    if (channels == 1) return p[0] * (1.0f / 255.0f);
    return (0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]) * (1.0f / 255.0f);
}

OC_STATUS ocularColoredPencilFilter(const unsigned char* Input, unsigned char* Output,
                                    int Width, int Height, int Stride,
                                    int PencilWidth, int StrokePressure, int PaperBrightness) {

    if (Input == NULL || Output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    if (Width <= 0 || Height <= 0 || Stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    int channels = Stride / Width;
    if (channels != 1 && channels != 3) {
        return OC_STATUS_ERR_NOTSUPPORTED;
    }

    if (PencilWidth < 1 || PencilWidth > 24 ||
        StrokePressure < 0 || StrokePressure > 15 ||
        PaperBrightness < 0 || PaperBrightness > 50) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    /* Derived constants (all per-image, hoisted out of the pixel loop) */
    const float pw       = (float)PencilWidth;
    const float pressure = (float)StrokePressure / 15.0f;
    const float paper    = 255.0f * (0.7f + 0.3f * (float)PaperBrightness / 50.0f); /* white paper */
    const float strokeLen = 5.0f + pw * 3.0f;
    const float strokeWid = 0.5f + pw * 0.45f;
    const float invLen = 1.0f / (strokeLen > 0.5f ? strokeLen : 0.5f);
    const float invWid = 1.0f / (strokeWid > 0.3f ? strokeWid : 0.3f);
    const float invSqrt2 = 0.70710678f;

    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {

            /* 3x3 luma neighborhood (clamped) for Sobel + center luma */
            float n[3][3];
            for (int j = 0; j < 3; j++)
                for (int i = 0; i < 3; i++)
                    n[j][i] = ocPencilLuma(Input, x + i - 1, y + j - 1, Width, Height, Stride, channels);

            float gx = ((n[0][2] + 2.0f * n[1][2] + n[2][2]) - (n[0][0] + 2.0f * n[1][0] + n[2][0])) * 0.125f;
            float gy = ((n[2][0] + 2.0f * n[2][1] + n[2][2]) - (n[0][0] + 2.0f * n[0][1] + n[0][2])) * 0.125f;
            float edge = sqrtf(gx * gx + gy * gy);

            float darkness = 1.0f - n[1][1];

            /* Two stroke fields: 45 deg (1,1) and -45 deg (1,-1), stretched along their direction */
            float fx = (float)x, fy = (float)y;
            float u1 = (fx + fy) * invSqrt2, v1 = (fy - fx) * invSqrt2;
            float u2 = (fx - fy) * invSqrt2, v2 = (fx + fy) * invSqrt2;
            float h1 = ocPencilValueNoise(u1 * invLen, v1 * invWid, 11u);
            float h2 = ocPencilValueNoise(u2 * invLen, v2 * invWid, 12u);

            /* Second direction only appears in darker areas -> cross-hatching in shadows */
            float hatch = h2 * ocPencilSmoothstep(0.3f, 0.8f, darkness);
            if (h1 > hatch) hatch = h1;

            float cov = ocArtisticClamp01(darkness * (0.55f + pressure)
                                        + edge * 3.0f
                                        - (1.0f - hatch) * 0.8f
                                        + 0.25f);

            int pPos = x * channels + y * Stride;
            for (int c = 0; c < channels; c++) {
                float ink = (float)Input[pPos + c] * 0.88f;      /* slightly darkened source color */
                float v = paper + (ink - paper) * cov;           /* mix(paper, ink, cov) */
                Output[pPos + c] = (unsigned char)(v < 0.0f ? 0.0f : (v > 255.0f ? 255.0f : v + 0.5f));
            }
        }
    }

    return OC_STATUS_OK;
}

/* ---- helpers for the cutout filter --------------------------------------- */

/* Snap v (0..1) to `levels` evenly spaced tones */
static inline float ocCutoutPosterize(float v, float levels) {
    float l = levels - 1.0f;
    if (l < 1.0f) l = 1.0f;
    return floorf(ocArtisticClamp01(v) * l + 0.5f) / l;
}

/* Edge-replicating running-mean box blur of one line (count samples, `step` floats apart). */
static inline void ocCutoutBoxLine(float* base, int count, int step, int r, float* scratch) {
    int i, k;
    float acc = 0.0f;
    const float norm = 1.0f / (float)(2 * r + 1);

    for (i = 0; i < count; i++) scratch[i] = base[(size_t)i * step];

    for (k = -r; k <= r; k++) {
        int kk = k < 0 ? 0 : (k >= count ? count - 1 : k);
        acc += scratch[kk];
    }
    for (i = 0; i < count; i++) {
        int hi = i + r + 1, lo = i - r;
        base[(size_t)i * step] = acc * norm;
        hi = hi >= count ? count - 1 : hi;
        lo = lo < 0 ? 0 : lo;
        acc += scratch[hi] - scratch[lo];
    }
}

/* Gaussian approximated by three box passes per axis (box sizes after Kutskir). */
static inline void ocCutoutGaussBlur(float* buf, int Width, int Height, int channels,
                                     float sigma, float* scratch) {
    int radii[3], n = 3, i;
    float wIdeal = sqrtf(12.0f * sigma * sigma / (float)n + 1.0f);
    int wl = (int)floorf(wIdeal);
    if (wl % 2 == 0) wl--;
    int wu = wl + 2;
    float mIdeal = (12.0f * sigma * sigma - (float)(n * wl * wl) - (float)(4 * n * wl) - (float)(3 * n))
                   / (float)(-4 * wl - 4);
    int m = (int)floorf(mIdeal + 0.5f);
    for (i = 0; i < n; i++) radii[i] = ((i < m ? wl : wu) - 1) / 2;

    for (i = 0; i < n; i++) {
        int r = radii[i], x, y, c;
        if (r <= 0) continue;
        for (y = 0; y < Height; y++)                 /* horizontal */
            for (c = 0; c < channels; c++)
                ocCutoutBoxLine(buf + (size_t)y * Width * channels + c, Width, channels, r, scratch);
        for (x = 0; x < Width; x++)                  /* vertical */
            for (c = 0; c < channels; c++)
                ocCutoutBoxLine(buf + (size_t)x * channels + c, Height, Width * channels, r, scratch);
    }
}

OC_STATUS ocularCutoutFilter(const unsigned char* Input, unsigned char* Output,
                             int Width, int Height, int Stride,
                             int NumberOfLevels, int EdgeSimplicity, int EdgeFidelity) {

    if (Input == NULL || Output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    if (Width <= 0 || Height <= 0 || Stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    int channels = Stride / Width;
    if (channels != 1 && channels != 3) {
        return OC_STATUS_ERR_NOTSUPPORTED;
    }

    if (NumberOfLevels < 2 || NumberOfLevels > 8 ||
        EdgeSimplicity < 0 || EdgeSimplicity > 10 ||
        EdgeFidelity < 1 || EdgeFidelity > 3) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    const size_t count = (size_t)Width * (size_t)Height * (size_t)channels;
    const int maxDim = Width > Height ? Width : Height;

    float* work    = (float*)malloc(count * sizeof(float));
    float* scratch = (float*)malloc((size_t)maxDim * sizeof(float));
    if (work == NULL || scratch == NULL) {
        free(work);
        free(scratch);
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

    /* Load into a normalised float working copy (tightly packed rows) */
    int x, y, c;
    for (y = 0; y < Height; y++) {
        const unsigned char* src = Input + (size_t)y * Stride;
        float* dst = work + (size_t)y * Width * channels;
        for (x = 0; x < Width * channels; x++) dst[x] = src[x] * (1.0f / 255.0f);
    }

    /* 1. Smooth: more simplicity = bigger blur, more fidelity = smaller blur */
    float sigma = 0.5f + (float)EdgeSimplicity * (1.1f - 0.3f * (float)EdgeFidelity);
    ocCutoutGaussBlur(work, Width, Height, channels, sigma, scratch);

    /* 2. Flatten tones, keeping each region's hue */
    const float levels = (float)NumberOfLevels;
    for (y = 0; y < Height; y++) {
        const float* src = work + (size_t)y * Width * channels;
        unsigned char* dst = Output + (size_t)y * Stride;
        for (x = 0; x < Width; x++) {
            const float* p = src + (size_t)x * channels;
            float l = (channels == 1) ? p[0] : (0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]);
            if (l < 1e-4f) l = 1e-4f;
            float k = ocCutoutPosterize(l, levels) / l;
            for (c = 0; c < channels; c++) {
                float v = ocCutoutPosterize(ocArtisticClamp01(p[c] * k), levels * 2.0f);
                dst[x * channels + c] = (unsigned char)(v * 255.0f + 0.5f);
            }
        }
    }

    free(work);
    free(scratch);
    return OC_STATUS_OK;
}