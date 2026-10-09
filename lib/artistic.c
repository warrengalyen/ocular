/**
 * @file: artistic.c
 * @author Warren Galyen
 * Created: 10-9-2026
 * Last Updated: 10-9-2026
 * Last update: added paint daubs filter
 *
 * @brief Artistic filter implementations
 */

#include "artistic.h"
#include "core.h"
#include <float.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

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
                                    int PencilWidth, int StrokePressure, int PaperBrightness,
                                    unsigned char ForegroundR, unsigned char ForegroundG, unsigned char ForegroundB,
                                    unsigned char BackgroundR, unsigned char BackgroundG, unsigned char BackgroundB) {

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
    const float paperGain = 0.7f + 0.3f * (float)PaperBrightness / 50.0f;

    /* Per-channel paper (background) and pencil tint (foreground). Grayscale input uses their luma. */
    float paper[3], tint[3];
    if (channels == 1) {
        paper[0] = (0.299f * BackgroundR + 0.587f * BackgroundG + 0.114f * BackgroundB) * paperGain;
        tint[0]  = 0.299f * ForegroundR + 0.587f * ForegroundG + 0.114f * ForegroundB;
    } else {
        paper[0] = BackgroundR * paperGain;  tint[0] = (float)ForegroundR;
        paper[1] = BackgroundG * paperGain;  tint[1] = (float)ForegroundG;
        paper[2] = BackgroundB * paperGain;  tint[2] = (float)ForegroundB;
    }

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
                ink = ink + tint[c] - ink * tint[c] * (1.0f / 255.0f); /* screen with foreground (black = no change) */
                float v = paper[c] + (ink - paper[c]) * cov;     /* mix(paper, ink, cov) */
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

/* ---- helpers for the dry brush filter ------------------------------------ */
/* Clamp, posterize, hash and the box-line blur are shared with the colored pencil / cutout filters
 * (ocArtisticClamp01, ocCutoutPosterize, ocPencilHash01, ocCutoutBoxLine). */

/* One box blur pass (radius r) over an interleaved n-channel float buffer. */
static inline void ocDryBrushBoxBlur(float* buf, int Width, int Height, int n, int r, float* scratch) {
    int x, y, c;
    for (y = 0; y < Height; y++)
        for (c = 0; c < n; c++)
            ocCutoutBoxLine(buf + (size_t)y * Width * n + c, Width, n, r, scratch);
    for (x = 0; x < Width; x++)
        for (c = 0; c < n; c++)
            ocCutoutBoxLine(buf + (size_t)x * n + c, Height, Width * n, r, scratch);
}

/* Kuwahara: each pixel takes the mean color of the lowest-variance quadrant.
 * `means` holds, per pixel, `channels` color means followed by mean(l) and mean(l^2). */
static inline void ocDryBrushKuwahara(const unsigned char* Input, int Width, int Height, int Stride,
                                      int channels, int radius, float* means, float* out, float* scratch) {
    const int n = channels + 2;
    int h2 = (radius + 1) / 2;
    if (h2 < 1) h2 = 1;
    int x, y, c, q;

    for (y = 0; y < Height; y++) {
        const unsigned char* src = Input + (size_t)y * Stride;
        float* m = means + (size_t)y * Width * n;
        for (x = 0; x < Width; x++) {
            const unsigned char* p = src + x * channels;
            float l;
            for (c = 0; c < channels; c++) m[x * n + c] = p[c] * (1.0f / 255.0f);
            if (channels == 1) l = m[x * n];
            else l = 0.299f * m[x * n] + 0.587f * m[x * n + 1] + 0.114f * m[x * n + 2];
            m[x * n + channels]     = l;
            m[x * n + channels + 1] = l * l;
        }
    }

    ocDryBrushBoxBlur(means, Width, Height, n, h2, scratch);

    static const int quad[4][2] = { {-1, -1}, {1, -1}, {-1, 1}, {1, 1} };
    for (y = 0; y < Height; y++) {
        for (x = 0; x < Width; x++) {
            float best = FLT_MAX;
            float col[3] = { 0.0f, 0.0f, 0.0f };
            for (q = 0; q < 4; q++) {
                int qx = x + quad[q][0] * h2;
                int qy = y + quad[q][1] * h2;
                qx = qx < 0 ? 0 : (qx >= Width  ? Width  - 1 : qx);
                qy = qy < 0 ? 0 : (qy >= Height ? Height - 1 : qy);
                const float* m = means + ((size_t)qy * Width + qx) * n;
                float var = m[channels + 1] - m[channels] * m[channels];
                if (var < best) {
                    best = var;
                    for (c = 0; c < channels; c++) col[c] = m[c];
                }
            }
            for (c = 0; c < channels; c++) out[((size_t)y * Width + x) * channels + c] = col[c];
        }
    }
}

/* Separable Gaussian (sigma = 1, 7 taps, edge-clamped): src -> dst, vertical pass in place. */
static inline void ocDryBrushGauss1(const float* src, float* dst, int Width, int Height,
                                    int channels, float* scratch) {
    enum { R = 3 };
    float k[R + 1], sum;
    int i, x, y, c;

    sum = 1.0f;
    k[0] = 1.0f;
    for (i = 1; i <= R; i++) { k[i] = expf(-0.5f * (float)(i * i)); sum += 2.0f * k[i]; }
    for (i = 0; i <= R; i++) k[i] /= sum;

    for (y = 0; y < Height; y++) {                      /* horizontal: src -> dst */
        const float* s = src + (size_t)y * Width * channels;
        float* d = dst + (size_t)y * Width * channels;
        for (x = 0; x < Width; x++) {
            for (c = 0; c < channels; c++) {
                float acc = k[0] * s[x * channels + c];
                for (i = 1; i <= R; i++) {
                    int xl = x - i < 0 ? 0 : x - i;
                    int xr = x + i >= Width ? Width - 1 : x + i;
                    acc += k[i] * (s[xl * channels + c] + s[xr * channels + c]);
                }
                d[x * channels + c] = acc;
            }
        }
    }
    for (x = 0; x < Width; x++) {                       /* vertical: in place via scratch */
        for (c = 0; c < channels; c++) {
            float* base = dst + (size_t)x * channels + c;
            const size_t step = (size_t)Width * channels;
            for (y = 0; y < Height; y++) scratch[y] = base[y * step];
            for (y = 0; y < Height; y++) {
                float acc = k[0] * scratch[y];
                for (i = 1; i <= R; i++) {
                    int yu = y - i < 0 ? 0 : y - i;
                    int yd = y + i >= Height ? Height - 1 : y + i;
                    acc += k[i] * (scratch[yu] + scratch[yd]);
                }
                base[y * step] = acc;
            }
        }
    }
}

OC_STATUS ocularDryBrushFilter(const unsigned char* Input, unsigned char* Output,
                               int Width, int Height, int Stride,
                               int BrushSize, int BrushDetail, int Texture) {

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

    if (BrushSize < 0 || BrushSize > 10 ||
        BrushDetail < 0 || BrushDetail > 10 ||
        Texture < 1 || Texture > 3) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    const int maxDim = Width > Height ? Width : Height;
    const size_t pixels = (size_t)Width * (size_t)Height;

    float* means   = (float*)malloc(pixels * (size_t)(channels + 2) * sizeof(float));
    float* work    = (float*)malloc(pixels * (size_t)channels * sizeof(float));
    float* scratch = (float*)malloc((size_t)maxDim * sizeof(float));
    if (means == NULL || work == NULL || scratch == NULL) {
        free(means);
        free(work);
        free(scratch);
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

    /* 1. Kuwahara smoothing -> work (float, 0..1) */
    ocDryBrushKuwahara(Input, Width, Height, Stride, channels, 1 + BrushSize, means, work, scratch);

    /* 2. Unsharp mask (sigma 1.0, amount 0.5); `means` is large enough to hold the blur */
    float* blurred = means;
    ocDryBrushGauss1(work, blurred, Width, Height, channels, scratch);

    const float amount = 0.5f;
    const float levels = 3.0f + (float)BrushDetail;
    const float amp    = 0.02f + ((float)Texture - 1.0f) * 0.05f;

    int x, y, c;
    for (y = 0; y < Height; y++) {
        const float* w = work + (size_t)y * Width * channels;
        const float* b = blurred + (size_t)y * Width * channels;
        unsigned char* dst = Output + (size_t)y * Stride;
        for (x = 0; x < Width; x++) {
            float noise = (ocPencilHash01(x, y, 21u) - 0.5f) * amp;
            for (c = 0; c < channels; c++) {
                size_t o = (size_t)x * channels + c;
                float v = w[o] + (w[o] - b[o]) * amount;                       /* unsharp */
                float p = ocCutoutPosterize(v, levels) * 0.6f + v * 0.4f + noise;
                dst[o] = (unsigned char)(ocArtisticClamp01(p) * 255.0f + 0.5f);
            }
        }
    }

    free(means);
    free(work);
    free(scratch);
    return OC_STATUS_OK;
}

/* ---- helpers for the fresco filter ---------------------------------------- */
/* Clamp, posterize, hash, Kuwahara and the 7-tap Gaussian are shared with the other artistic filters
 * (ocArtisticClamp01, ocCutoutPosterize, ocPencilHash01, ocDryBrushKuwahara, ocDryBrushGauss1). */

/* Luma sample from a float plane with clamped borders */
static inline float ocFrescoPlaneAt(const float* L, int x, int y, int Width, int Height) {
    if (x < 0) x = 0; else if (x >= Width)  x = Width - 1;
    if (y < 0) y = 0; else if (y >= Height) y = Height - 1;
    return L[(size_t)y * Width + x];
}

OC_STATUS ocularFrescoFilter(const unsigned char* Input, unsigned char* Output,
                             int Width, int Height, int Stride,
                             int BrushSize, int BrushDetail, int Texture) {

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

    if (BrushSize < 0 || BrushSize > 10 ||
        BrushDetail < 0 || BrushDetail > 10 ||
        Texture < 1 || Texture > 3) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    const int maxDim = Width > Height ? Width : Height;
    const size_t pixels = (size_t)Width * (size_t)Height;

    float* means   = (float*)malloc(pixels * (size_t)(channels + 2) * sizeof(float));
    float* work    = (float*)malloc(pixels * (size_t)channels * sizeof(float));
    float* scratch = (float*)malloc((size_t)maxDim * sizeof(float));
    if (means == NULL || work == NULL || scratch == NULL) {
        free(means);
        free(work);
        free(scratch);
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

    int x, y, c;

    /* 1. Kuwahara smoothing -> work */
    ocDryBrushKuwahara(Input, Width, Height, Stride, channels, 1 + BrushSize, means, work, scratch);

    /* 2. Unsharp mask (sigma 1.0, amount 0.9), applied in place on `work`.
     *    `means` is large enough to hold the blurred copy. */
    float* buf = means;
    ocDryBrushGauss1(work, buf, Width, Height, channels, scratch);

    const float amount = 0.9f;
    for (size_t i = 0; i < pixels * (size_t)channels; i++) {
        work[i] += (work[i] - buf[i]) * amount;      /* intentionally not clamped yet */
    }

    /* 3. Luma plane of the sharpened image (reuses `means`, blur no longer needed) */
    float* L = means;
    for (size_t i = 0; i < pixels; i++) {
        const float* p = work + i * channels;
        L[i] = (channels == 1) ? p[0] : (0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]);
    }

    /* 4. Posterize + grain, then contrast curve and edge darkening */
    const float levels = 3.0f + (float)BrushDetail;
    const float amp    = 0.02f + ((float)Texture - 1.0f) * 0.05f;

    for (y = 0; y < Height; y++) {
        const float* w = work + (size_t)y * Width * channels;
        unsigned char* dst = Output + (size_t)y * Stride;
        for (x = 0; x < Width; x++) {

            /* Sobel gradient magnitude of the luma plane (clamped borders) */
            float a00 = ocFrescoPlaneAt(L, x - 1, y - 1, Width, Height);
            float a10 = ocFrescoPlaneAt(L, x,     y - 1, Width, Height);
            float a20 = ocFrescoPlaneAt(L, x + 1, y - 1, Width, Height);
            float a01 = ocFrescoPlaneAt(L, x - 1, y,     Width, Height);
            float a21 = ocFrescoPlaneAt(L, x + 1, y,     Width, Height);
            float a02 = ocFrescoPlaneAt(L, x - 1, y + 1, Width, Height);
            float a12 = ocFrescoPlaneAt(L, x,     y + 1, Width, Height);
            float a22 = ocFrescoPlaneAt(L, x + 1, y + 1, Width, Height);
            float gx = ((a20 + 2.0f * a21 + a22) - (a00 + 2.0f * a01 + a02)) * 0.125f;
            float gy = ((a02 + 2.0f * a12 + a22) - (a00 + 2.0f * a10 + a20)) * 0.125f;
            float edge = sqrtf(gx * gx + gy * gy);
            float dark = 1.0f - ocArtisticClamp01(edge * 2.5f) * 0.6f;

            float noise = (ocPencilHash01(x, y, 21u) - 0.5f) * amp;

            for (c = 0; c < channels; c++) {
                float v = w[(size_t)x * channels + c];
                float p = ocCutoutPosterize(v, levels) * 0.6f + v * 0.4f + noise;
                float q = ocArtisticClamp01((p - 0.5f) * 1.3f + 0.45f);
                q = powf(q, 1.15f) * dark;
                dst[x * channels + c] = (unsigned char)(ocArtisticClamp01(q) * 255.0f + 0.5f);
            }
        }
    }

    free(means);
    free(work);
    free(scratch);
    return OC_STATUS_OK;
}

/* ---- helpers for the paint daubs filter ----------------------------------- */
/* Clamp, hash, smoothstep, value noise, plane sampling and the Kuwahara pass are shared with the other
 * artistic filters (ocArtisticClamp01, ocPencilHash01, ocPencilSmoothstep, ocPencilValueNoise,
 * ocFrescoPlaneAt, ocDryBrushKuwahara). */

/* Three-octave value noise -> [0,1] */
static inline float ocPaintDaubsFbm(float x, float y, unsigned int seed) {
    return ocPencilValueNoise(x, y, seed) * 0.57f
         + ocPencilValueNoise(x * 2.03f, y * 2.03f, seed ^ 0x51u) * 0.29f
         + ocPencilValueNoise(x * 4.1f,  y * 4.1f,  seed ^ 0xa3u) * 0.14f;
}

/* Separable Gaussian of any sigma (radius ceil(3*sigma), max 8, edge-clamped).
 * src -> dst; the vertical pass runs in place on dst via a scratch line. */
static inline void ocPaintDaubsGauss(const float* src, float* dst, int Width, int Height,
                                     int channels, float sigma, float* scratch) {
    enum { MAXR = 8 };
    float k[MAXR + 1], sum;
    int R = (int)ceilf(3.0f * sigma);
    int i, x, y, c;
    if (R < 1) R = 1;
    if (R > MAXR) R = MAXR;

    sum = 1.0f;
    k[0] = 1.0f;
    for (i = 1; i <= R; i++) {
        k[i] = expf(-0.5f * (float)(i * i) / (sigma * sigma));
        sum += 2.0f * k[i];
    }
    for (i = 0; i <= R; i++) k[i] /= sum;

    for (y = 0; y < Height; y++) {
        const float* s = src + (size_t)y * Width * channels;
        float* d = dst + (size_t)y * Width * channels;
        for (x = 0; x < Width; x++) {
            for (c = 0; c < channels; c++) {
                float acc = k[0] * s[x * channels + c];
                for (i = 1; i <= R; i++) {
                    int xl = x - i < 0 ? 0 : x - i;
                    int xr = x + i >= Width ? Width - 1 : x + i;
                    acc += k[i] * (s[xl * channels + c] + s[xr * channels + c]);
                }
                d[x * channels + c] = acc;
            }
        }
    }
    for (x = 0; x < Width; x++) {
        for (c = 0; c < channels; c++) {
            float* base = dst + (size_t)x * channels + c;
            const size_t step = (size_t)Width * channels;
            for (y = 0; y < Height; y++) scratch[y] = base[y * step];
            for (y = 0; y < Height; y++) {
                float acc = k[0] * scratch[y];
                for (i = 1; i <= R; i++) {
                    int yu = y - i < 0 ? 0 : y - i;
                    int yd = y + i >= Height ? Height - 1 : y + i;
                    acc += k[i] * (scratch[yu] + scratch[yd]);
                }
                base[y * step] = acc;
            }
        }
    }
}

OC_STATUS ocularPaintDaubsFilter(const unsigned char* Input, unsigned char* Output,
                                 int Width, int Height, int Stride,
                                 int BrushSize, int Sharpness, int BrushType) {

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

    if (BrushSize < 1 || BrushSize > 50 ||
        Sharpness < 0 || Sharpness > 40 ||
        BrushType < OC_PAINTDAUBS_SIMPLE || BrushType > OC_PAINTDAUBS_SPARKLE) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    const int maxDim = Width > Height ? Width : Height;
    const size_t pixels = (size_t)Width * (size_t)Height;
    const size_t values = pixels * (size_t)channels;

    float* means   = (float*)malloc(pixels * (size_t)(channels + 2) * sizeof(float));
    float* work    = (float*)malloc(values * sizeof(float));
    float* scratch = (float*)malloc((size_t)maxDim * sizeof(float));
    if (means == NULL || work == NULL || scratch == NULL) {
        free(means);
        free(work);
        free(scratch);
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

    int x, y, c;
    size_t i;

    /* 1. Kuwahara smoothing; wide brushes use a 1.5x larger radius */
    const int wide = (BrushType == OC_PAINTDAUBS_WIDE_SHARP || BrushType == OC_PAINTDAUBS_WIDE_BLURRY);
    float rf = (float)BrushSize / 2.0f * (wide ? 1.5f : 1.0f);
    int radius = (int)(rf < 1.0f ? 1.0f : rf);
    ocDryBrushKuwahara(Input, Width, Height, Stride, channels, radius, means, work, scratch);

    /* 2. Sharpen or blur (`means` is large enough to hold the blurred copy) */
    const float sharp = (float)Sharpness / 40.0f;
    float* buf = means;

    if (BrushType == OC_PAINTDAUBS_WIDE_BLURRY) {
        ocPaintDaubsGauss(work, buf, Width, Height, channels, 1.5f, scratch);
        memcpy(work, buf, values * sizeof(float));
    } else {
        float sigma  = (BrushType == OC_PAINTDAUBS_WIDE_SHARP) ? 1.5f : 1.2f;
        float amount = sharp * ((BrushType == OC_PAINTDAUBS_WIDE_SHARP) ? 4.0f : 2.5f);
        ocPaintDaubsGauss(work, buf, Width, Height, channels, sigma, scratch);
        for (i = 0; i < values; i++) work[i] += (work[i] - buf[i]) * amount;   /* not clamped yet */
    }

    /* 3. Sparkle needs the luma plane of the sharpened image (reuses `means`) */
    float* L = means;
    if (BrushType == OC_PAINTDAUBS_SPARKLE) {
        for (i = 0; i < pixels; i++) {
            const float* p = work + i * channels;
            L[i] = (channels == 1) ? p[0] : (0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]);
        }
    }

    /* 4. Per-brush finish and output */
    for (y = 0; y < Height; y++) {
        const float* w = work + (size_t)y * Width * channels;
        unsigned char* dst = Output + (size_t)y * Stride;
        for (x = 0; x < Width; x++) {
            float n = 0.0f, add = 0.0f, mul = 1.0f;

            if (BrushType == OC_PAINTDAUBS_LIGHT_ROUGH || BrushType == OC_PAINTDAUBS_DARK_ROUGH) {
                n = ocPaintDaubsFbm((float)x / 2.0f, (float)y / 2.0f, 41u) - 0.5f;
                if (n < 0.0f) n = 0.0f;
                if (BrushType == OC_PAINTDAUBS_LIGHT_ROUGH) add = n * 0.35f;
                else                                        mul = 1.0f - n * 0.7f;
            } else if (BrushType == OC_PAINTDAUBS_SPARKLE) {
                float a00 = ocFrescoPlaneAt(L, x - 1, y - 1, Width, Height);
                float a10 = ocFrescoPlaneAt(L, x,     y - 1, Width, Height);
                float a20 = ocFrescoPlaneAt(L, x + 1, y - 1, Width, Height);
                float a01 = ocFrescoPlaneAt(L, x - 1, y,     Width, Height);
                float a21 = ocFrescoPlaneAt(L, x + 1, y,     Width, Height);
                float a02 = ocFrescoPlaneAt(L, x - 1, y + 1, Width, Height);
                float a12 = ocFrescoPlaneAt(L, x,     y + 1, Width, Height);
                float a22 = ocFrescoPlaneAt(L, x + 1, y + 1, Width, Height);
                float gx = ((a20 + 2.0f * a21 + a22) - (a00 + 2.0f * a01 + a02)) * 0.125f;
                float gy = ((a02 + 2.0f * a12 + a22) - (a00 + 2.0f * a10 + a20)) * 0.125f;
                float edge = sqrtf(gx * gx + gy * gy);
                add = ocArtisticClamp01(edge * 4.0f)
                    * ocPencilSmoothstep(0.75f, 0.95f, ocPencilHash01(x, y, 42u)) * 0.8f;
            }

            for (c = 0; c < channels; c++) {
                float v = w[(size_t)x * channels + c];
                v = (v + add) * mul;
                dst[x * channels + c] = (unsigned char)(ocArtisticClamp01(v) * 255.0f + 0.5f);
            }
        }
    }

    free(means);
    free(work);
    free(scratch);
    return OC_STATUS_OK;
}
