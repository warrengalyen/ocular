/**
 * @file: artistic.c
 * @author Warren Galyen
 * Created: 10-9-2026
 * Last Updated: 10-10-2026
 * Last update: added rough pastels filter
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

/* ---- helpers for the palette knife filter --------------------------------- */
/* Clamp, posterize, the box blur pass and the Kuwahara pass are shared with the other artistic filters
 * (ocArtisticClamp01, ocCutoutPosterize, ocDryBrushBoxBlur, ocDryBrushKuwahara). */

/* Box radii whose three successive passes approximate a Gaussian of `sigma` (after Kutskir). */
static inline void ocPaletteKnifeBoxRadii(float sigma, int radii[3]) {
    const int n = 3;
    float wIdeal = sqrtf(12.0f * sigma * sigma / (float)n + 1.0f);
    int wl = (int)floorf(wIdeal);
    if (wl % 2 == 0) wl--;
    if (wl < 1) wl = 1;
    int wu = wl + 2;
    float mIdeal = (12.0f * sigma * sigma - (float)(n * wl * wl) - (float)(4 * n * wl) - (float)(3 * n))
                   / (float)(-4 * wl - 4);
    int m = (int)floorf(mIdeal + 0.5f);
    for (int i = 0; i < n; i++) radii[i] = ((i < m ? wl : wu) - 1) / 2;
}

OC_STATUS ocularPaletteKnifeFilter(const unsigned char* Input, unsigned char* Output,
                                   int Width, int Height, int Stride,
                                   int StrokeSize, int StrokeDetail, int Softness) {

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

    if (StrokeSize < 1 || StrokeSize > 50 ||
        StrokeDetail < 1 || StrokeDetail > 3 ||
        Softness < 0 || Softness > 10) {
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

    /* 1. Kuwahara smoothing with radius strokeSize / 3 (minimum 1) */
    float rf = (float)StrokeSize / 3.0f;
    int radius = (int)(rf < 1.0f ? 1.0f : rf);
    ocDryBrushKuwahara(Input, Width, Height, Stride, channels, radius, means, work, scratch);

    /* 2. Half-strength posterize */
    const float levels = 2.0f + (float)StrokeDetail * 3.0f;
    size_t i;
    for (i = 0; i < values; i++) {
        work[i] = ocCutoutPosterize(work[i], levels) * 0.5f + work[i] * 0.5f;
    }

    /* 3. Softness blur (sigma = softness * 0.5, skipped below 0.3) */
    float sigma = (float)Softness * 0.5f;
    if (sigma >= 0.3f) {
        int radii[3], k;
        ocPaletteKnifeBoxRadii(sigma, radii);
        for (k = 0; k < 3; k++) {
            if (radii[k] > 0) ocDryBrushBoxBlur(work, Width, Height, channels, radii[k], scratch);
        }
    }

    /* 4. Output */
    int x, y, c;
    for (y = 0; y < Height; y++) {
        const float* w = work + (size_t)y * Width * channels;
        unsigned char* dst = Output + (size_t)y * Stride;
        for (x = 0; x < Width; x++) {
            for (c = 0; c < channels; c++) {
                dst[x * channels + c] =
                    (unsigned char)(ocArtisticClamp01(w[(size_t)x * channels + c]) * 255.0f + 0.5f);
            }
        }
    }

    free(means);
    free(work);
    free(scratch);
    return OC_STATUS_OK;
}

/* ---- helpers for the plastic wrap filter ---------------------------------- */
/* Clamp, smoothstep, plane sampling, the box blur pass and the box-radius helper are shared with the
 * other artistic filters (ocArtisticClamp01, ocPencilSmoothstep, ocFrescoPlaneAt, ocDryBrushBoxBlur,
 * ocPaletteKnifeBoxRadii). */

OC_STATUS ocularPlasticWrapFilter(const unsigned char* Input, unsigned char* Output,
                                  int Width, int Height, int Stride,
                                  int HighlightStrength, int Detail, int Smoothness) {

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

    if (HighlightStrength < 0 || HighlightStrength > 20 ||
        Detail < 1 || Detail > 15 ||
        Smoothness < 1 || Smoothness > 15) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    const int maxDim = Width > Height ? Width : Height;
    const size_t pixels = (size_t)Width * (size_t)Height;

    float* L       = (float*)malloc(pixels * sizeof(float));
    float* scratch = (float*)malloc((size_t)maxDim * sizeof(float));
    if (L == NULL || scratch == NULL) {
        free(L);
        free(scratch);
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

    int x, y, c;

    /* 1. Luma plane, then smooth it (three box passes approximating a Gaussian) */
    for (y = 0; y < Height; y++) {
        const unsigned char* src = Input + (size_t)y * Stride;
        float* row = L + (size_t)y * Width;
        for (x = 0; x < Width; x++) {
            const unsigned char* p = src + x * channels;
            row[x] = (channels == 1) ? p[0] * (1.0f / 255.0f)
                                     : (0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]) * (1.0f / 255.0f);
        }
    }

    float sigma = 0.5f + (float)Smoothness * 0.5f + (15.0f - (float)Detail) * 0.2f;
    int radii[3], k;
    ocPaletteKnifeBoxRadii(sigma, radii);
    for (k = 0; k < 3; k++) {
        if (radii[k] > 0) ocDryBrushBoxBlur(L, Width, Height, 1, radii[k], scratch);
    }

    /* 2. Light and half-vector (view direction is straight on) */
    float lx = -0.45f, ly = -0.65f, lz = 1.0f;
    float ll = sqrtf(lx * lx + ly * ly + lz * lz);
    lx /= ll; ly /= ll; lz /= ll;
    float hx = lx, hy = ly, hz = lz + 1.0f;
    float hl = sqrtf(hx * hx + hy * hy + hz * hz);
    hx /= hl; hy /= hl; hz /= hl;

    const float hs = (float)HighlightStrength / 20.0f;
    const float slope = 14.0f;

    /* 3. Shade each pixel */
    for (y = 0; y < Height; y++) {
        unsigned char* dst = Output + (size_t)y * Stride;
        const unsigned char* src = Input + (size_t)y * Stride;
        for (x = 0; x < Width; x++) {
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

            /* Surface normal normalize(-k*gx, -k*gy, 1) */
            float nx = -slope * gx, ny = -slope * gy;
            float inv = 1.0f / sqrtf(nx * nx + ny * ny + 1.0f);
            nx *= inv; ny *= inv;
            float nz = inv;

            float ndh = nx * hx + ny * hy + nz * hz;
            if (ndh < 0.0f) ndh = 0.0f;
            float tilt = 1.0f - nz;
            float spec = powf(ndh, 30.0f) * ocPencilSmoothstep(0.01f, 0.12f, tilt)
                       + ocPencilSmoothstep(0.25f, 0.6f, tilt) * 0.25f;

            float add = spec * hs * 1.3f;
            for (c = 0; c < channels; c++) {
                float v = src[x * channels + c] * (1.0f / 255.0f);
                dst[x * channels + c] =
                    (unsigned char)(ocArtisticClamp01(v * 0.88f + add) * 255.0f + 0.5f);
            }
        }
    }

    free(L);
    free(scratch);
    return OC_STATUS_OK;
}

/* ---- helpers for the poster edges filter ---------------------------------- */
/* Clamp, posterize, smoothstep, plane sampling, the box blur pass and the box-radius helper are shared
 * with the other artistic filters (ocArtisticClamp01, ocCutoutPosterize, ocPencilSmoothstep,
 * ocFrescoPlaneAt, ocDryBrushBoxBlur, ocPaletteKnifeBoxRadii). */

OC_STATUS ocularPosterEdgesFilter(const unsigned char* Input, unsigned char* Output,
                                  int Width, int Height, int Stride,
                                  int EdgeThickness, int EdgeIntensity, int Posterization) {

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

    if (EdgeThickness < 0 || EdgeThickness > 10 ||
        EdgeIntensity < 0 || EdgeIntensity > 10 ||
        Posterization < 0 || Posterization > 6) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    const int maxDim = Width > Height ? Width : Height;
    const size_t pixels = (size_t)Width * (size_t)Height;

    float* L       = (float*)malloc(pixels * sizeof(float));
    float* scratch = (float*)malloc((size_t)maxDim * sizeof(float));
    if (L == NULL || scratch == NULL) {
        free(L);
        free(scratch);
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

    int x, y, c, k;

    /* 1. Luma plane, blurred (three box passes approximating a Gaussian) */
    for (y = 0; y < Height; y++) {
        const unsigned char* src = Input + (size_t)y * Stride;
        float* row = L + (size_t)y * Width;
        for (x = 0; x < Width; x++) {
            const unsigned char* p = src + x * channels;
            row[x] = (channels == 1) ? p[0] * (1.0f / 255.0f)
                                     : (0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]) * (1.0f / 255.0f);
        }
    }

    float sigma = 0.6f + (float)EdgeThickness * 0.6f;
    int radii[3];
    ocPaletteKnifeBoxRadii(sigma, radii);
    for (k = 0; k < 3; k++) {
        if (radii[k] > 0) ocDryBrushBoxBlur(L, Width, Height, 1, radii[k], scratch);
    }

    /* 2. Edge ink and posterize */
    const float inten  = (float)EdgeIntensity / 10.0f;
    const float levels = (float)Posterization + 2.0f;

    for (y = 0; y < Height; y++) {
        const unsigned char* src = Input + (size_t)y * Stride;
        unsigned char* dst = Output + (size_t)y * Stride;
        for (x = 0; x < Width; x++) {
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

            float ink = ocPencilSmoothstep(0.015f, 0.06f, edge * (1.0f + inten * 3.0f))
                      * (0.55f + inten * 0.45f);

            for (c = 0; c < channels; c++) {
                float v = src[x * channels + c] * (1.0f / 255.0f);
                float o = ocCutoutPosterize(v, levels) * (1.0f - ink);
                dst[x * channels + c] = (unsigned char)(ocArtisticClamp01(o) * 255.0f + 0.5f);
            }
        }
    }

    free(L);
    free(scratch);
    return OC_STATUS_OK;
}

/* ---- helpers for the rough pastels filter --------------------------------- */
/* Clamp, hash, smoothstep, value noise, fbm, the box blur pass and the box-radius helper are shared with the
 * other artistic filters (ocArtisticClamp01, ocPencilHash01, ocPencilSmoothstep, ocPencilValueNoise,
 * ocPaintDaubsFbm, ocDryBrushBoxBlur, ocPaletteKnifeBoxRadii). */

static inline float ocPastelRemEuclid(float a, float m) {
    float r = fmodf(a, m);
    return r < 0.0f ? r + m : r;
}

/* Basket weave of threads p px wide with `noise` irregularity */
static inline float ocPastelWeave(float x, float y, float p, float noise) {
    int cx = (int)floorf(x / p), cy = (int)floorf(y / p);
    int overH = ((((cx + cy) % 2) + 2) % 2) == 0;
    float u = ocPastelRemEuclid(x, p) / p;
    float v = ocPastelRemEuclid(y, p) / p;
    float ridge = overH ? sinf(v * 3.14159265f) : sinf(u * 3.14159265f);
    float n = ocPencilValueNoise(x / (p * 0.7f), y / (p * 2.0f), 97u);
    return ridge * (1.0f - noise) + n * noise;
}

/* Procedural texture height (0..1) at pixel coordinates; s scales it (1 = 100 %) */
static inline float ocPastelTexture(int kind, float x, float y, float s) {
    x /= s;
    y /= s;
    switch (kind) {
    case OC_PASTEL_TEXTURE_BRICK: {
        const float bw = 24.0f, bh = 12.0f;
        float row = floorf(y / bh);
        float xo = x + (((int)row % 2 == 0) ? 0.0f : bw / 2.0f);
        float u = ocPastelRemEuclid(xo, bw), v = ocPastelRemEuclid(y, bh);
        float d = fminf(fminf(u, bw - u), fminf(v, bh - v));
        return ocPencilSmoothstep(0.6f, 2.2f, d) * (0.85f + 0.15f * ocPencilValueNoise(x / 3.0f, y / 3.0f, 91u));
    }
    case OC_PASTEL_TEXTURE_BURLAP:
        return ocPastelWeave(x, y, 7.0f, 0.35f);
    case OC_PASTEL_TEXTURE_CANVAS:
        return ocPastelWeave(x, y, 3.5f, 0.18f);
    case OC_PASTEL_TEXTURE_SANDSTONE:
        return ocPaintDaubsFbm(x / 7.0f, y / 7.0f, 93u) * 0.7f
             + ocPencilHash01((int)x, (int)y, 94u) * 0.3f;
    default:
        return 0.5f;
    }
}

/* Bilinear sample of an interleaved float image at pixel coordinates (centres at .5), clamped. */
static inline void ocPastelSample(const float* buf, int Width, int Height, int ch,
                                  float x, float y, float* out) {
    float fx = x - 0.5f, fy = y - 0.5f;
    fx = fx < 0.0f ? 0.0f : (fx > (float)(Width - 1) ? (float)(Width - 1) : fx);
    fy = fy < 0.0f ? 0.0f : (fy > (float)(Height - 1) ? (float)(Height - 1) : fy);
    int x0 = (int)floorf(fx), y0 = (int)floorf(fy);
    int x1 = x0 + 1 < Width ? x0 + 1 : Width - 1;
    int y1 = y0 + 1 < Height ? y0 + 1 : Height - 1;
    float ax = fx - (float)x0, ay = fy - (float)y0;
    for (int c = 0; c < ch; c++) {
        float p00 = buf[((size_t)y0 * Width + x0) * ch + c];
        float p10 = buf[((size_t)y0 * Width + x1) * ch + c];
        float p01 = buf[((size_t)y1 * Width + x0) * ch + c];
        float p11 = buf[((size_t)y1 * Width + x1) * ch + c];
        float top = p00 + (p10 - p00) * ax;
        float bot = p01 + (p11 - p01) * ax;
        out[c] = top + (bot - top) * ay;
    }
}

OC_STATUS ocularRoughPastelsFilter(const unsigned char* Input, unsigned char* Output,
                                   int Width, int Height, int Stride,
                                   int StrokeLength, int StrokeDetail,
                                   int Texture, int Scaling, int Relief,
                                   int Light, int Invert) {

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

    if (StrokeLength < 0 || StrokeLength > 40 ||
        StrokeDetail < 1 || StrokeDetail > 20 ||
        Texture < OC_PASTEL_TEXTURE_BRICK || Texture > OC_PASTEL_TEXTURE_SANDSTONE ||
        Scaling < 50 || Scaling > 200 ||
        Relief < 0 || Relief > 50 ||
        Light < OC_PASTEL_LIGHT_TOP || Light > OC_PASTEL_LIGHT_TOP_LEFT) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    const int maxDim = Width > Height ? Width : Height;
    const size_t values = (size_t)Width * (size_t)Height * (size_t)channels;
    const int doStrokes = StrokeLength >= 1;

    /* 1. Blurred copy used as the colour source for the strokes */
    float* soft    = NULL;
    float* scratch = NULL;
    if (doStrokes) {
        soft    = (float*)malloc(values * sizeof(float));
        scratch = (float*)malloc((size_t)maxDim * sizeof(float));
        if (soft == NULL || scratch == NULL) {
            free(soft);
            free(scratch);
            return OC_STATUS_ERR_OUTOFMEMORY;
        }
        int x, y;
        for (y = 0; y < Height; y++) {
            const unsigned char* src = Input + (size_t)y * Stride;
            float* dst = soft + (size_t)y * Width * channels;
            for (x = 0; x < Width * channels; x++) dst[x] = src[x] * (1.0f / 255.0f);
        }
        float det0  = (float)(StrokeDetail - 1) / 19.0f;
        float sigma = 0.6f + (1.0f - det0) * 1.2f;
        int radii[3], pass;
        ocPaletteKnifeBoxRadii(sigma, radii);
        for (pass = 0; pass < 3; pass++) {
            if (radii[pass] > 0) ocDryBrushBoxBlur(soft, Width, Height, channels, radii[pass], scratch);
        }
    }

    /* Constants */
    const float det      = (float)(StrokeDetail - 1) / 19.0f;
    const float len      = 2.0f + (float)StrokeLength;
    const float wid      = 1.5f;
    const float ux       = 0.70710678f, uy = -0.70710678f;           /* stroke direction (1,-1) */
    const float grainLen = 2.0f + (float)StrokeLength * 0.5f;

    const float s          = (float)Scaling / 100.0f;
    const float reliefGain = (float)Relief / 50.0f * 3.0f;
    const float sg         = Invert ? -1.0f : 1.0f;
    /* Unit light direction per OcPastelLight (literals: a static initializer must be a constant expression) */
    static const float lightX[8] = { 0.0f,  0.70710678f, 1.0f,  0.70710678f, 0.0f, -0.70710678f, -1.0f, -0.70710678f };
    static const float lightY[8] = { -1.0f, -0.70710678f, 0.0f,  0.70710678f, 1.0f,  0.70710678f,  0.0f, -0.70710678f };
    const float lx = lightX[Light], ly = lightY[Light];

    int x, y, c;
    float cs[3], p[3];

    for (y = 0; y < Height; y++) {
        const unsigned char* src = Input + (size_t)y * Stride;
        unsigned char* dst = Output + (size_t)y * Stride;
        for (x = 0; x < Width; x++) {

            for (c = 0; c < channels; c++) p[c] = src[x * channels + c] * (1.0f / 255.0f);

            /* 2. Chalk strokes */
            if (doStrokes) {
                float fx = (float)x + 0.5f, fy = (float)y + 0.5f;
                float u = fx * ux + fy * uy;
                float v = -fx * uy + fy * ux;
                int lane = (int)floorf(v / wid);
                float off = ocPencilHash01(lane, 0, 51u) * len;
                float t = ocPastelRemEuclid((u + off) / len, 1.0f);
                float du = (0.5f - t) * len;
                ocPastelSample(soft, Width, Height, channels, fx + du * ux, fy + du * uy, cs);

                float xf = (float)x, yf = (float)y;
                float gu = xf * ux + yf * uy;
                float gv = -xf * uy + yf * ux;
                float n = (ocPencilValueNoise(gu / grainLen, gv / 0.8f, 52u) - 0.5f) * 0.12f
                        + (0.5f - fabsf(t - 0.5f)) * 0.06f;

                for (c = 0; c < channels; c++) {
                    float m = cs[c] + (p[c] - cs[c]) * det * 0.6f;   /* mix(stroke, original) */
                    p[c] = m + n;
                }
            }

            /* 3. Texture relief, strongest in dark areas */
            if (Relief > 0) {
                float l = (channels == 1) ? p[0] : (0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]);
                float weight = 0.15f + 0.85f * (1.0f - ocArtisticClamp01(l));

                float fx = (float)x, fy = (float)y;
                float gx = (ocPastelTexture(Texture, fx + 1.0f, fy, s) - ocPastelTexture(Texture, fx - 1.0f, fy, s)) * 0.5f * sg;
                float gy = (ocPastelTexture(Texture, fx, fy + 1.0f, s) - ocPastelTexture(Texture, fx, fy - 1.0f, s)) * 0.5f * sg;

                /* Lambert shading of the height field, relative to a flat surface */
                float nx = -reliefGain * gx, ny = -reliefGain * gy;
                float sh = (nx * lx + ny * ly + 1.0f) / sqrtf(nx * nx + ny * ny + 1.0f);
                sh = sh < 0.0f ? 0.0f : (sh > 2.0f ? 2.0f : sh);
                float f = 1.0f + (sh - 1.0f) * weight;
                for (c = 0; c < channels; c++) p[c] *= f;
            }

            for (c = 0; c < channels; c++) {
                dst[x * channels + c] = (unsigned char)(ocArtisticClamp01(p[c]) * 255.0f + 0.5f);
            }
        }
    }

    free(soft);
    free(scratch);
    return OC_STATUS_OK;
}
