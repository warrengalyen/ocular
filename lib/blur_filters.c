#include "blur_filters.h"


static inline void boxfilterRow(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, int Radius) {
    int iRadius = Radius + 1;
    int iScale = (int)((256.0f * 256.0f) / (2 * Radius + 1));
    int iWidthStep = Width * Channels;
    int iWidthStepDec = (Width - 1) * Channels;
    int iRadChannels = Radius * Channels;
    int iRadChannelsPlus = (iRadChannels + Channels);
    switch (Channels) {
    case 1: {
        for (int y = 0; y < Height; y++) {
            //  Process left edge
            int iY = y * iWidthStep;
            int sum = Input[iY] * Radius;
            for (int x = 0; x < iRadius; x++) {
                int p = (y * Width + x) * Channels;
                sum += Input[p];
            }
            Output[iY] = (unsigned char)((sum * iScale) >> 16);
            for (int x = 1; x < iRadius; x++) {
                int pLeft = iY + x * Channels;
                int p0 = pLeft + iRadChannels;

                sum += Input[p0];
                sum -= Input[iY];
                Output[pLeft] = (unsigned char)((sum * iScale) >> 16);
            }

            // core zone
            for (int x = iRadius; x < Width - Radius; x++) {
                int pKernal = iY + x * Channels;

                int i0 = pKernal + iRadChannels;
                int i1 = pKernal - iRadChannelsPlus;

                sum += Input[i0];
                sum -= Input[i1];

                Output[pKernal] = (unsigned char)((sum * iScale) >> 16);
            }

            // Process right edge
            for (int x = Width - Radius; x < Width; x++) {
                int iRight = iY + x * Channels;
                int i0 = iY + iWidthStepDec;
                int i1 = iRight - iRadChannelsPlus;

                sum += Input[i0];
                sum -= Input[i1];
                Output[iRight] = (unsigned char)((sum * iScale) >> 16);
            }
        }
        break;
    }
    case 3: {
        for (int y = 0; y < Height; y++) {
            //  Process left edge

            int iY = y * iWidthStep;
            int sumR = Input[iY] * Radius;
            int sumG = Input[iY + 1] * Radius;
            int sumB = Input[iY + 2] * Radius;
            for (int x = 0; x < iRadius; x++) {
                int i = iY + x * Channels;
                sumR += Input[i];
                sumG += Input[i + 1];
                sumB += Input[i + 2];
            }
            Output[iY] = (unsigned char)((sumR * iScale) >> 16);
            Output[iY + 1] = (unsigned char)((sumG * iScale) >> 16);
            Output[iY + 2] = (unsigned char)((sumB * iScale) >> 16);
            for (int x = 1; x < iRadius; x++) {
                int iLeft = iY + x * Channels;
                int i0 = iLeft + iRadChannels;

                sumR += Input[i0];
                sumR -= Input[iY];
                sumG += Input[i0 + 1];
                sumG -= Input[iY + 1];
                sumB += Input[i0 + 2];
                sumB -= Input[iY + 2];
                Output[iLeft] = (unsigned char)((sumR * iScale) >> 16);
                Output[iLeft + 1] = (unsigned char)((sumG * iScale) >> 16);
                Output[iLeft + 2] = (unsigned char)((sumB * iScale) >> 16);
            }

            // core zone
            for (int x = iRadius; x < Width - Radius; x++) {
                int iKernal = iY + x * Channels;

                int i0 = iKernal + iRadChannels;
                int i1 = iKernal - iRadChannelsPlus;

                sumR += Input[i0];
                sumR -= Input[i1];

                sumG += Input[i0 + 1];
                sumG -= Input[i1 + 1];

                sumB += Input[i0 + 2];
                sumB -= Input[i1 + 2];
                Output[iKernal] = (unsigned char)((sumR * iScale) >> 16);
                Output[iKernal + 1] = (unsigned char)((sumG * iScale) >> 16);
                Output[iKernal + 2] = (unsigned char)((sumB * iScale) >> 16);
            }

            // Process right edge
            for (int x = Width - Radius; x < Width; x++) {
                int iRight = iY + x * Channels;
                int i0 = iY + iWidthStepDec;
                int i1 = iRight - iRadChannelsPlus;

                sumR += Input[i0];
                sumR -= Input[i1];

                sumG += Input[i0 + 1];
                sumG -= Input[i1 + 1];

                sumB += Input[i0 + 2];
                sumB -= Input[i1 + 2];
                Output[iRight] = (unsigned char)((sumR * iScale) >> 16);
                Output[iRight + 1] = (unsigned char)((sumG * iScale) >> 16);
                Output[iRight + 2] = (unsigned char)((sumB * iScale) >> 16);
            }
        }
        break;
    }
    case 4: {
        for (int y = 0; y < Height; y++) {
            //  Process left edge
            int iY = y * iWidthStep;
            int sumR = Input[iY] * Radius;
            int sumG = Input[iY + 1] * Radius;
            int sumB = Input[iY + 2] * Radius;
            for (int x = 0; x < iRadius; x++) {
                int i = iY + x * Channels;
                sumR += Input[i];
                sumG += Input[i + 1];
                sumB += Input[i + 2];
            }
            Output[iY] = (unsigned char)((sumR * iScale) >> 16);
            Output[iY + 1] = (unsigned char)((sumG * iScale) >> 16);
            Output[iY + 2] = (unsigned char)((sumB * iScale) >> 16);
            Output[iY + 3] = Input[iY + 3];
            for (int x = 1; x < iRadius; x++) {
                int iLeft = iY + x * Channels;
                int i0 = iLeft + iRadChannels;
                sumR += Input[i0];
                sumR -= Input[iLeft];
                sumG += Input[i0 + 1];
                sumG -= Input[iLeft + 1];
                sumB += Input[i0 + 2];
                sumB -= Input[iLeft + 2];
                Output[iLeft] = (unsigned char)((sumR * iScale) >> 16);
                Output[iLeft + 1] = (unsigned char)((sumG * iScale) >> 16);
                Output[iLeft + 2] = (unsigned char)((sumB * iScale) >> 16);
                Output[iLeft + 3] = Input[iLeft + 3];
            }

            // core zone
            for (int x = iRadius; x < Width - Radius; x++) {
                int iKernal = iY + x * Channels;

                int i0 = iKernal + iRadChannels;
                int i1 = iKernal - iRadChannelsPlus;

                sumR += Input[i0];
                sumR -= Input[i1];

                sumG += Input[i0 + 1];
                sumG -= Input[i1 + 1];

                sumB += Input[i0 + 2];
                sumB -= Input[i1 + 2];
                Output[iKernal] = (unsigned char)((sumR * iScale) >> 16);
                Output[iKernal + 1] = (unsigned char)((sumG * iScale) >> 16);
                Output[iKernal + 2] = (unsigned char)((sumB * iScale) >> 16);
                Output[iKernal + 3] = Input[iKernal + 3];
            }

            // Process right edge
            for (int x = Width - Radius; x < Width; x++) {
                int iRight = iY + x * Channels;
                int i0 = iY + iWidthStepDec;
                int i1 = iRight - iRadChannelsPlus;

                sumR += Input[i0];
                sumR -= Input[i1];

                sumG += Input[i0 + 1];
                sumG -= Input[i1 + 1];

                sumB += Input[i0 + 2];
                sumB -= Input[i1 + 2];
                Output[iRight] = (unsigned char)((sumR * iScale) >> 16);
                Output[iRight + 1] = (unsigned char)((sumG * iScale) >> 16);
                Output[iRight + 2] = (unsigned char)((sumB * iScale) >> 16);
                Output[iRight + 3] = Input[iRight + 3];
            }
        }
        break;
    }
    default: break;
    }
}

static inline void boxfilterCol(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, int Radius) {
    int iScale = (int)((256.0f * 256.0f) / (2 * Radius + 1));
    int iWidthStep = Width * Channels;
    int iWidthStepDec = (Height - 1) * iWidthStep;
    int iRadWidthStep = Radius * iWidthStep;
    int iRadWidthStepDec = (iRadWidthStep + iWidthStep);
    int iHeightRadius = Height - Radius;
    int iRadius = Radius + 1;
    switch (Channels) {
    case 1: {
        for (int x = 0; x < Width; x++) {
            //  Process left edge
            int iX = x * Channels;
            int sum = Input[iX] * Radius;
            for (int y = 0; y < iRadius; y++) {
                int i = (y * Width + x) * Channels;
                sum += Input[i];
            }
            Output[x] = (unsigned char)((sum * iScale) >> 16);

            for (int y = 1; y < iRadius; y++) {
                int i = iX + y * iWidthStep;

                int i0 = i + iRadWidthStep;
                int i1 = x * Channels;

                sum += Input[i0];
                sum -= Input[i1];
                Output[i] = (unsigned char)((sum * iScale) >> 16);
            }

            // core zone
            for (int y = iRadius; y < iHeightRadius; y++) {

                int iKernal = iX + y * iWidthStep;
                int i0 = iKernal + iRadWidthStep;
                int i1 = iKernal - iRadWidthStepDec;

                sum += Input[i0];
                sum -= Input[i1];
                Output[iKernal] = (unsigned char)((sum * iScale) >> 16);
            }

            // Process right edge
            for (int y = iHeightRadius; y < Height; y++) {
                int iRight = iX + y * iWidthStep;

                int i0 = iWidthStepDec + x * Channels;
                int i1 = iRight - iRadWidthStepDec;

                sum += Input[i0];
                sum -= Input[i1];
                Output[iRight] = (unsigned char)((sum * iScale) >> 16);
            }
        }

        break;
    }
    case 3: {
        for (int x = 0; x < Width; x++) {
            //  Process left edge
            int iX = x * Channels;
            int sumR = Input[iX] * Radius;
            int sumG = Input[iX + 1] * Radius;
            int sumB = Input[iX + 2] * Radius;
            for (int y = 0; y < iRadius; y++) {
                int i = iX + y * iWidthStep;
                sumR += Input[i];
                sumG += Input[i + 1];
                sumB += Input[i + 2];
            }
            Output[iX] = (unsigned char)((sumR * iScale) >> 16);
            Output[iX + 1] = (unsigned char)((sumG * iScale) >> 16);
            Output[iX + 2] = (unsigned char)((sumB * iScale) >> 16);

            for (int y = 1; y < iRadius; y++) {
                int i = iX + y * iWidthStep;
                int i0 = i + iRadWidthStep;

                sumR += Input[i0];
                sumR -= Input[iX];
                sumG += Input[i0 + 1];
                sumG -= Input[iX + 1];
                sumB += Input[i0 + 2];
                sumB -= Input[iX + 2];
                Output[i] = (unsigned char)((sumR * iScale) >> 16);
                Output[i + 1] = (unsigned char)((sumG * iScale) >> 16);
                Output[i + 2] = (unsigned char)((sumB * iScale) >> 16);
            }

            // core zone
            for (int y = iRadius; y < iHeightRadius; y++) {

                int iKernal = iX + y * iWidthStep;

                int i0 = iKernal + iRadWidthStep;
                int i1 = iKernal - iRadWidthStepDec;

                sumR += Input[i0];
                sumR -= Input[i1];
                sumG += Input[i0 + 1];
                sumG -= Input[i1 + 1];
                sumB += Input[i0 + 2];
                sumB -= Input[i1 + 2];
                Output[iKernal] = (unsigned char)((sumR * iScale) >> 16);
                Output[iKernal + 1] = (unsigned char)((sumG * iScale) >> 16);
                Output[iKernal + 2] = (unsigned char)((sumB * iScale) >> 16);
            }

            // Process right edge
            for (int y = iHeightRadius; y < Height; y++) {
                int iRight = iX + y * iWidthStep;
                int i0 = iWidthStepDec + iX;
                int i1 = iRight - iRadWidthStepDec;

                sumR += Input[i0];
                sumR -= Input[i1];
                sumG += Input[i0 + 1];
                sumG -= Input[i1 + 1];
                sumB += Input[i0 + 2];
                sumB -= Input[i1 + 2];
                Output[iRight] = (unsigned char)((sumR * iScale) >> 16);
                Output[iRight + 1] = (unsigned char)((sumG * iScale) >> 16);
                Output[iRight + 2] = (unsigned char)((sumB * iScale) >> 16);
            }
        }

        break;
    }
    case 4: {
        for (int x = 0; x < Width; x++) {
            //  Process left edge
            int iX = x * Channels;
            int sumR = Input[iX] * Radius;
            int sumG = Input[iX + 1] * Radius;
            int sumB = Input[iX + 2] * Radius;
            for (int y = 0; y < iRadius; y++) {
                int i = iX + y * iWidthStep;
                sumR += Input[i];
                sumG += Input[i + 1];
                sumB += Input[i + 2];
            }
            Output[iX] = (unsigned char)((sumR * iScale) >> 16);
            Output[iX + 1] = (unsigned char)((sumG * iScale) >> 16);
            Output[iX + 2] = (unsigned char)((sumB * iScale) >> 16);
            Output[iX + 3] = Input[iX + 3];
            for (int y = 1; y < iRadius; y++) {
                int i = iX + y * iWidthStep;
                int i0 = i + iRadWidthStep;
                sumR += Input[i0];
                sumR -= Input[iX];
                sumG += Input[i0 + 1];
                sumG -= Input[iX + 1];
                sumB += Input[i0 + 2];
                sumB -= Input[iX + 2];
                Output[i] = (unsigned char)((sumR * iScale) >> 16);
                Output[i + 1] = (unsigned char)((sumG * iScale) >> 16);
                Output[i + 2] = (unsigned char)((sumB * iScale) >> 16);
                Output[i + 3] = Input[i + 3];
            }

            // core zone
            for (int y = iRadius; y < iHeightRadius; y++) {

                int iKernal = iX + y * iWidthStep;
                int i0 = iKernal + iRadWidthStep;
                int i1 = iKernal - iRadWidthStepDec;
                sumR += Input[i0];
                sumR -= Input[i1];
                sumG += Input[i0 + 1];
                sumG -= Input[i1 + 1];
                sumB += Input[i0 + 2];
                sumB -= Input[i1 + 2];
                Output[iKernal] = (unsigned char)((sumR * iScale) >> 16);
                Output[iKernal + 1] = (unsigned char)((sumG * iScale) >> 16);
                Output[iKernal + 2] = (unsigned char)((sumB * iScale) >> 16);
                Output[iKernal + 3] = Input[iKernal + 3];
            }

            // Process right edge
            for (int y = iHeightRadius; y < Height; y++) {
                int iRight = iX + y * iWidthStep;

                int i0 = iWidthStepDec + iX;
                int i1 = iRight - iRadWidthStepDec;
                sumR += Input[i0];
                sumR -= Input[i1];
                sumG += Input[i0 + 1];
                sumG -= Input[i1 + 1];
                sumB += Input[i0 + 2];
                sumB -= Input[i1 + 2];
                Output[iRight] = (unsigned char)((sumR * iScale) >> 16);
                Output[iRight + 1] = (unsigned char)((sumG * iScale) >> 16);
                Output[iRight + 2] = (unsigned char)((sumB * iScale) >> 16);
                Output[iRight + 3] = Input[iRight + 3];
            }
        }
        break;
    }
    default: break;
    }
}

OC_STATUS ocularBoxBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius) {

    if (Input == NULL || Output == NULL)
        return OC_STATUS_ERR_NULLREFERENCE;
    if (Width <= 0 || Height <= 0 || Stride <= 0)
        return OC_STATUS_ERR_INVALIDPARAMETER;

    int Channels = Stride / Width;
    if (Channels != 1 && Channels != 3 && Channels != 4)
        return OC_STATUS_ERR_NOTSUPPORTED;

    // Ensure Radius is within valid range
    Radius = clamp(Radius, 1, 127);

    unsigned char* temp = (unsigned char*)malloc(Width * Height * Channels);
    if (temp == NULL) {
        return OC_STATUS_ERR_OUTOFMEMORY;
    }
    boxfilterRow(Input, temp, Width, Height, Channels, Radius);
    boxfilterCol(temp, Output, Width, Height, Channels, Radius);
    free(temp);

    return OC_STATUS_OK;
}

static void gaussianHorizontal(unsigned char* bufferPerLine, const unsigned char* lpRowInitial, unsigned char* lpColumn, int width,
                               int height, int Channels, int Nwidth, float a0a1, float a2a3, float b1b2, float cprev, float cnext) {
    int HeightStep = Channels * height;
    int WidthSubOne = width - 1;
    if (Channels == 3) {
        float prevOut[3];
        prevOut[0] = (lpRowInitial[0] * cprev);
        prevOut[1] = (lpRowInitial[1] * cprev);
        prevOut[2] = (lpRowInitial[2] * cprev);
        for (int x = 0; x < width; ++x) {
            prevOut[0] = ((lpRowInitial[0] * (a0a1)) - (prevOut[0] * (b1b2)));
            prevOut[1] = ((lpRowInitial[1] * (a0a1)) - (prevOut[1] * (b1b2)));
            prevOut[2] = ((lpRowInitial[2] * (a0a1)) - (prevOut[2] * (b1b2)));
            bufferPerLine[0] = (unsigned char)prevOut[0];
            bufferPerLine[1] = (unsigned char)prevOut[1];
            bufferPerLine[2] = (unsigned char)prevOut[2];
            bufferPerLine += Channels;
            lpRowInitial += Channels;
        }
        lpRowInitial -= Channels;
        lpColumn += HeightStep * WidthSubOne;
        bufferPerLine -= Channels;
        prevOut[0] = (lpRowInitial[0] * cnext);
        prevOut[1] = (lpRowInitial[1] * cnext);
        prevOut[2] = (lpRowInitial[2] * cnext);

        for (int x = WidthSubOne; x >= 0; --x) {
            prevOut[0] = ((lpRowInitial[0] * (a2a3)) - (prevOut[0] * (b1b2)));
            prevOut[1] = ((lpRowInitial[1] * (a2a3)) - (prevOut[1] * (b1b2)));
            prevOut[2] = ((lpRowInitial[2] * (a2a3)) - (prevOut[2] * (b1b2)));
            bufferPerLine[0] += (unsigned char)prevOut[0];
            bufferPerLine[1] += (unsigned char)prevOut[1];
            bufferPerLine[2] += (unsigned char)prevOut[2];
            lpColumn[0] = bufferPerLine[0];
            lpColumn[1] = bufferPerLine[1];
            lpColumn[2] = bufferPerLine[2];
            lpRowInitial -= Channels;
            lpColumn -= HeightStep;
            bufferPerLine -= Channels;
        }
    } else if (Channels == 4) {
        float prevOut[4];

        prevOut[0] = (lpRowInitial[0] * cprev);
        prevOut[1] = (lpRowInitial[1] * cprev);
        prevOut[2] = (lpRowInitial[2] * cprev);
        prevOut[3] = (lpRowInitial[3] * cprev);
        for (int x = 0; x < width; ++x) {
            prevOut[0] = ((lpRowInitial[0] * (a0a1)) - (prevOut[0] * (b1b2)));
            prevOut[1] = ((lpRowInitial[1] * (a0a1)) - (prevOut[1] * (b1b2)));
            prevOut[2] = ((lpRowInitial[2] * (a0a1)) - (prevOut[2] * (b1b2)));
            prevOut[3] = ((lpRowInitial[3] * (a0a1)) - (prevOut[3] * (b1b2)));

            bufferPerLine[0] = (unsigned char)prevOut[0];
            bufferPerLine[1] = (unsigned char)prevOut[1];
            bufferPerLine[2] = (unsigned char)prevOut[2];
            bufferPerLine[3] = (unsigned char)prevOut[3];
            bufferPerLine += Channels;
            lpRowInitial += Channels;
        }
        lpRowInitial -= Channels;
        lpColumn += HeightStep * WidthSubOne;
        bufferPerLine -= Channels;

        prevOut[0] = (lpRowInitial[0] * cnext);
        prevOut[1] = (lpRowInitial[1] * cnext);
        prevOut[2] = (lpRowInitial[2] * cnext);
        prevOut[3] = (lpRowInitial[3] * cnext);

        for (int x = WidthSubOne; x >= 0; --x) {
            prevOut[0] = ((lpRowInitial[0] * a2a3) - (prevOut[0] * b1b2));
            prevOut[1] = ((lpRowInitial[1] * a2a3) - (prevOut[1] * b1b2));
            prevOut[2] = ((lpRowInitial[2] * a2a3) - (prevOut[2] * b1b2));
            prevOut[3] = ((lpRowInitial[3] * a2a3) - (prevOut[3] * b1b2));
            bufferPerLine[0] += (unsigned char)prevOut[0];
            bufferPerLine[1] += (unsigned char)prevOut[1];
            bufferPerLine[2] += (unsigned char)prevOut[2];
            bufferPerLine[3] += (unsigned char)prevOut[3];
            lpColumn[0] = bufferPerLine[0];
            lpColumn[1] = bufferPerLine[1];
            lpColumn[2] = bufferPerLine[2];
            lpColumn[3] = bufferPerLine[3];
            lpRowInitial -= Channels;
            lpColumn -= HeightStep;
            bufferPerLine -= Channels;
        }
    } else if (Channels == 1) {
        float prevOut = (lpRowInitial[0] * cprev);

        for (int x = 0; x < width; ++x) {
            prevOut = ((lpRowInitial[0] * (a0a1)) - (prevOut * (b1b2)));
            bufferPerLine[0] = (unsigned char)prevOut;
            bufferPerLine += Channels;
            lpRowInitial += Channels;
        }
        lpRowInitial -= Channels;
        lpColumn += HeightStep * WidthSubOne;
        bufferPerLine -= Channels;

        prevOut = (lpRowInitial[0] * cnext);

        for (int x = WidthSubOne; x >= 0; --x) {
            prevOut = ((lpRowInitial[0] * a2a3) - (prevOut * b1b2));
            bufferPerLine[0] += (unsigned char)prevOut;
            lpColumn[0] = bufferPerLine[0];
            lpRowInitial -= Channels;
            lpColumn -= HeightStep;
            bufferPerLine -= Channels;
        }
    }
}

static void gaussianVertical(unsigned char* bufferPerLine, const unsigned char* lpRowInitial, unsigned char* lpColInitial, int height,
                             int width, int Channels, float a0a1, float a2a3, float b1b2, float cprev, float cnext) {

    int WidthStep = Channels * width;
    int HeightSubOne = height - 1;
    if (Channels == 3) {
        float prevOut[3];
        prevOut[0] = (lpRowInitial[0] * cprev);
        prevOut[1] = (lpRowInitial[1] * cprev);
        prevOut[2] = (lpRowInitial[2] * cprev);

        for (int y = 0; y < height; y++) {
            prevOut[0] = ((lpRowInitial[0] * a0a1) - (prevOut[0] * b1b2));
            prevOut[1] = ((lpRowInitial[1] * a0a1) - (prevOut[1] * b1b2));
            prevOut[2] = ((lpRowInitial[2] * a0a1) - (prevOut[2] * b1b2));
            bufferPerLine[0] = (unsigned char)prevOut[0];
            bufferPerLine[1] = (unsigned char)prevOut[1];
            bufferPerLine[2] = (unsigned char)prevOut[2];
            bufferPerLine += Channels;
            lpRowInitial += Channels;
        }
        lpRowInitial -= Channels;
        bufferPerLine -= Channels;
        lpColInitial += WidthStep * HeightSubOne;
        prevOut[0] = (lpRowInitial[0] * cnext);
        prevOut[1] = (lpRowInitial[1] * cnext);
        prevOut[2] = (lpRowInitial[2] * cnext);
        for (int y = HeightSubOne; y >= 0; y--) {
            prevOut[0] = ((lpRowInitial[0] * a2a3) - (prevOut[0] * b1b2));
            prevOut[1] = ((lpRowInitial[1] * a2a3) - (prevOut[1] * b1b2));
            prevOut[2] = ((lpRowInitial[2] * a2a3) - (prevOut[2] * b1b2));
            bufferPerLine[0] += (unsigned char)prevOut[0];
            bufferPerLine[1] += (unsigned char)prevOut[1];
            bufferPerLine[2] += (unsigned char)prevOut[2];
            lpColInitial[0] = bufferPerLine[0];
            lpColInitial[1] = bufferPerLine[1];
            lpColInitial[2] = bufferPerLine[2];
            lpRowInitial -= Channels;
            lpColInitial -= WidthStep;
            bufferPerLine -= Channels;
        }
    } else if (Channels == 4) {
        float prevOut[4];

        prevOut[0] = (lpRowInitial[0] * cprev);
        prevOut[1] = (lpRowInitial[1] * cprev);
        prevOut[2] = (lpRowInitial[2] * cprev);
        prevOut[3] = (lpRowInitial[3] * cprev);

        for (int y = 0; y < height; y++) {
            prevOut[0] = ((lpRowInitial[0] * a0a1) - (prevOut[0] * b1b2));
            prevOut[1] = ((lpRowInitial[1] * a0a1) - (prevOut[1] * b1b2));
            prevOut[2] = ((lpRowInitial[2] * a0a1) - (prevOut[2] * b1b2));
            prevOut[3] = ((lpRowInitial[3] * a0a1) - (prevOut[3] * b1b2));
            bufferPerLine[0] = (unsigned char)prevOut[0];
            bufferPerLine[1] = (unsigned char)prevOut[1];
            bufferPerLine[2] = (unsigned char)prevOut[2];
            bufferPerLine[3] = (unsigned char)prevOut[3];
            bufferPerLine += Channels;
            lpRowInitial += Channels;
        }
        lpRowInitial -= Channels;
        bufferPerLine -= Channels;
        lpColInitial += WidthStep * HeightSubOne;
        prevOut[0] = (lpRowInitial[0] * cnext);
        prevOut[1] = (lpRowInitial[1] * cnext);
        prevOut[2] = (lpRowInitial[2] * cnext);
        prevOut[3] = (lpRowInitial[3] * cnext);
        for (int y = HeightSubOne; y >= 0; y--) {
            prevOut[0] = ((lpRowInitial[0] * a2a3) - (prevOut[0] * b1b2));
            prevOut[1] = ((lpRowInitial[1] * a2a3) - (prevOut[1] * b1b2));
            prevOut[2] = ((lpRowInitial[2] * a2a3) - (prevOut[2] * b1b2));
            prevOut[3] = ((lpRowInitial[3] * a2a3) - (prevOut[3] * b1b2));
            bufferPerLine[0] += (unsigned char)prevOut[0];
            bufferPerLine[1] += (unsigned char)prevOut[1];
            bufferPerLine[2] += (unsigned char)prevOut[2];
            bufferPerLine[3] += (unsigned char)prevOut[3];
            lpColInitial[0] = bufferPerLine[0];
            lpColInitial[1] = bufferPerLine[1];
            lpColInitial[2] = bufferPerLine[2];
            lpColInitial[3] = bufferPerLine[3];
            lpRowInitial -= Channels;
            lpColInitial -= WidthStep;
            bufferPerLine -= Channels;
        }
    } else if (Channels == 1) {
        float prevOut = 0;
        prevOut = (lpRowInitial[0] * cprev);
        for (int y = 0; y < height; y++) {
            prevOut = ((lpRowInitial[0] * a0a1) - (prevOut * b1b2));
            bufferPerLine[0] = (unsigned char)prevOut;
            bufferPerLine += Channels;
            lpRowInitial += Channels;
        }
        lpRowInitial -= Channels;
        bufferPerLine -= Channels;
        lpColInitial += WidthStep * HeightSubOne;
        prevOut = (lpRowInitial[0] * cnext);
        for (int y = HeightSubOne; y >= 0; y--) {
            prevOut = ((lpRowInitial[0] * a2a3) - (prevOut * b1b2));
            bufferPerLine[0] += (unsigned char)prevOut;
            lpColInitial[0] = bufferPerLine[0];
            lpRowInitial -= Channels;
            lpColInitial -= WidthStep;
            bufferPerLine -= Channels;
        }
    }
}

OC_STATUS ocularGaussianBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, float GaussianSigma) {

    if (Input == NULL || Output == NULL)
        return OC_STATUS_ERR_NULLREFERENCE;
    if (Width <= 0 || Height <= 0 || Stride <= 0)
        return OC_STATUS_ERR_INVALIDPARAMETER;

    int Channels = Stride / Width;
    if (Channels != 1 && Channels != 3 && Channels != 4)
        return OC_STATUS_ERR_NOTSUPPORTED;

    // Ensure filter specific parameters are within valid ranges
    GaussianSigma = max(GaussianSigma, 0.0f);

    float a0, a1, a2, a3, b1, b2, cprev, cnext;

    CalGaussianCoeff(GaussianSigma, &a0, &a1, &a2, &a3, &b1, &b2, &cprev, &cnext);

    float a0a1 = (a0 + a1);
    float a2a3 = (a2 + a3);
    float b1b2 = (b1 + b2);

    int bufferSizePerThread = (Width > Height ? Width : Height) * Channels;
    unsigned char* bufferPerLine = (unsigned char*)malloc((size_t)bufferSizePerThread);
    unsigned char* tempData = (unsigned char*)malloc((size_t)Height * Stride);
    if (bufferPerLine == NULL || tempData == NULL) {
        if (tempData) {
            free(tempData);
        }
        if (bufferPerLine) {
            free(bufferPerLine);
        }
        return OC_STATUS_ERR_OUTOFMEMORY;
    }
    for (int y = 0; y < Height; ++y) {
        unsigned char* lpRowInitial = Input + Stride * y;
        unsigned char* lpColInitial = tempData + y * Channels;
        gaussianHorizontal(bufferPerLine, lpRowInitial, lpColInitial, Width, Height, Channels, Width, a0a1, a2a3, b1b2, cprev, cnext);
    }
    int HeightStep = Height * Channels;
    for (int x = 0; x < Width; ++x) {
        unsigned char* lpColInitial = Output + x * Channels;
        unsigned char* lpRowInitial = tempData + HeightStep * x;
        gaussianVertical(bufferPerLine, lpRowInitial, lpColInitial, Height, Width, Channels, a0a1, a2a3, b1b2, cprev, cnext);
    }

    free(bufferPerLine);
    free(tempData);

    return OC_STATUS_OK;
}

OC_STATUS ocularExponentialBlur(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, float Radius) {

    if (Input == NULL || Output == NULL)
        return OC_STATUS_ERR_NULLREFERENCE;
    if (Width <= 0 || Height <= 0 || Channels <= 0)
        return OC_STATUS_ERR_INVALIDPARAMETER;

    Radius = max(Radius, 1);

    // Create temporary buffer
    unsigned char* temp = (unsigned char*)malloc(Width * Height * Channels);
    if (!temp)
        return OC_STATUS_ERR_OUTOFMEMORY;

    // Pre-calculate weights for the kernel
    int kernel_size = (int)ceil(Radius) * 2 + 1;
    float* weights = (float*)malloc(kernel_size * sizeof(float));
    if (!weights) {
        free(temp);
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

    // Calculate exponential kernel weights once
    float sigma = Radius * 0.5f;
    float alpha = 1.0f / (2.0f * sigma * sigma);
    float total_weight = 0.0f;
    for (int i = 0; i < kernel_size; i++) {
        int offset = i - (kernel_size / 2);
        weights[i] = expf(-alpha * offset * offset);
        total_weight += weights[i];
    }

    // Normalize weights
    for (int i = 0; i < kernel_size; i++) {
        weights[i] /= total_weight;
    }

    // Horizontal pass
    for (int y = 0; y < Height; y++) {
        for (int c = 0; c < Channels; c++) {
            for (int x = 0; x < Width; x++) {
                float sum = 0.0f;
                int center_idx = (y * Width + x) * Channels + c;

                for (int i = 0; i < kernel_size; i++) {
                    int offset = i - (kernel_size / 2);
                    int sx = x + offset;

                    if (sx >= 0 && sx < Width) {
                        sum += Input[(y * Width + sx) * Channels + c] * weights[i];
                    } else {
                        // Mirror boundary conditions
                        sx = sx < 0 ? -sx : (2 * Width - sx - 2);
                        sum += Input[(y * Width + sx) * Channels + c] * weights[i];
                    }
                }

                temp[center_idx] = (unsigned char)(sum + 0.5f);
            }
        }
    }

    // Vertical pass
    for (int x = 0; x < Width; x++) {
        for (int c = 0; c < Channels; c++) {
            for (int y = 0; y < Height; y++) {
                float sum = 0.0f;
                int center_idx = (y * Width + x) * Channels + c;

                for (int i = 0; i < kernel_size; i++) {
                    int offset = i - (kernel_size / 2);
                    int sy = y + offset;

                    if (sy >= 0 && sy < Height) {
                        sum += temp[(sy * Width + x) * Channels + c] * weights[i];
                    } else {
                        // Mirror boundary conditions
                        sy = sy < 0 ? -sy : (2 * Height - sy - 2);
                        sum += temp[(sy * Width + x) * Channels + c] * weights[i];
                    }
                }

                Output[center_idx] = (unsigned char)(sum + 0.5f);
            }
        }
    }

    free(weights);
    free(temp);

    return OC_STATUS_OK;
}

OC_STATUS ocularSurfaceBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius, int Threshold) {

    if (Input == NULL || Output == NULL)
        return OC_STATUS_ERR_NULLREFERENCE;
    if (Width <= 0 || Height <= 0 || Stride <= 0)
        return OC_STATUS_ERR_INVALIDPARAMETER;

    int Channel = Stride / Width;
    if ((Channel != 1) && (Channel != 3))
        return OC_STATUS_ERR_NOTSUPPORTED;

    // Ensure Radius and Threshold are within valid ranges
    Radius = clamp(Radius, 1, 127);
    Threshold = clamp(Threshold, 2, 255);

    if (Channel == 1) {
        unsigned short* ColHist = (unsigned short*)_aligned_malloc(256 * (Width + Radius + Radius) * sizeof(unsigned short), 32);
        if (ColHist == NULL)
            return OC_STATUS_ERR_OUTOFMEMORY;
        unsigned short* Hist = (unsigned short*)_aligned_malloc(256 * sizeof(unsigned short), 32);
        if (Hist == NULL)
            return OC_STATUS_ERR_OUTOFMEMORY;

        unsigned short* Intensity = (unsigned short*)_aligned_malloc(511 * sizeof(unsigned short), 32); // Avoid abs when a negative value is used
        if (Intensity == NULL)
            return OC_STATUS_ERR_OUTOFMEMORY;
        unsigned short* Level = (unsigned short*)_aligned_malloc(256 * sizeof(unsigned short), 32);
        if (Level == NULL)
            return OC_STATUS_ERR_OUTOFMEMORY;

        int* RowOffset = (int*)malloc((Width + Radius + Radius) * sizeof(int));
        if (RowOffset == NULL)
            return OC_STATUS_ERR_OUTOFMEMORY;
        int* ColOffset = (int*)malloc((Height + Radius + Radius) * sizeof(int));
        if (ColOffset == NULL)
            return OC_STATUS_ERR_OUTOFMEMORY;

        GetOffsetPos(RowOffset, Width, Radius, Radius);
        GetOffsetPos(ColOffset, Height, Radius, Radius);

        memset(ColHist, 0, 256 * (Width + Radius + Radius) * sizeof(unsigned short)); //	Make sure to clear

        for (int Y = 0; Y < 256; Y++)
            Level[Y] = Y;

        for (int Y = -255; Y <= 255; Y++) {
            int Factor = (255 - abs(Y) * 100 / Threshold);
            if (Factor < 0)
                Factor = 0;
            Intensity[Y + 255] = Factor / 2;
        }

        for (int Y = 0; Y < Height; Y++) {
            if (Y == 0) //	The first row of column histograms
            {
                for (int K = -Radius; K <= Radius; K++) {
                    unsigned char* LinePS = Input + ColOffset[K + Radius] * Stride;
                    for (int X = -Radius; X < Width + Radius; X++) {
                        ColHist[(X + Radius) * 256 + LinePS[RowOffset[X + Radius]]]++;
                    }
                }
            } else //	Column histogram for other rows, update it
            {
                unsigned char* LinePS = Input + ColOffset[Y - 1] * Stride;
                for (int X = -Radius; X < Width + Radius; X++) // Delete the histogram data for the row that is out of range
                {
                    ColHist[(X + Radius) * 256 + LinePS[RowOffset[X + Radius]]]--;
                }

                LinePS = Input + ColOffset[Y + Radius + Radius] * Stride;
                for (int X = -Radius; X < Width + Radius; X++) // Increase the histogram data for the line in the incoming range
                {
                    ColHist[(X + Radius) * 256 + LinePS[RowOffset[X + Radius]]]++;
                }
            }

            memset(Hist, 0, 256 * sizeof(unsigned short)); //	Each row of histogram data is cleared first

            unsigned char* LinePS = Input + Y * Stride;
            unsigned char* LinePD = Output + Y * Stride;

            for (int X = 0; X < Width; X++) {
                if (X == 0) {
                    for (int K = -Radius; K <= Radius; K++) //	First pixel, needs to be recalculated
                        HistogramAddShort(ColHist + (K + Radius) * 256, Hist);
                } else {
                    HistogramSubAddShort(ColHist + (RowOffset[X - 1] + Radius) * 256, ColHist + (RowOffset[X + Radius + Radius] + Radius) * 256,
                                         Hist); //	The other pixels in the line can be deleted and added in turn.
                }

                LinePD[X] = HistogramCalc(Hist, LinePS[X], Intensity);
            }
        }
        _aligned_free(ColHist);
        _aligned_free(Hist);
        _aligned_free(Intensity);
        _aligned_free(Level);
        free(RowOffset);
        free(ColOffset);
    } else {
        unsigned char* SrcB = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
        unsigned char* SrcG = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
        unsigned char* SrcR = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
        unsigned char* DstB = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
        unsigned char* DstG = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
        unsigned char* DstR = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));

        SplitRGB(Input, SrcB, SrcG, SrcR, Width, Height, Stride);
        {
            ocularSurfaceBlurFilter(SrcB, DstB, Width, Height, Width, Radius, Threshold);
            ocularSurfaceBlurFilter(SrcG, DstG, Width, Height, Width, Radius, Threshold);
            ocularSurfaceBlurFilter(SrcR, DstR, Width, Height, Width, Radius, Threshold);
        }
        CombineRGB(DstB, DstG, DstR, Output, Width, Height, Stride);

        free(SrcB);
        free(SrcG);
        free(SrcR);
        free(DstB);
        free(DstG);
        free(DstR);
    }
}

OC_STATUS ocularZoomBlur(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int sampleRadius, float blurAmount,
                         int centerX, int centerY) {

    if (Input == NULL || Output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    if (Width <= 0 || Height <= 0 || Stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    // Ensure filter specific parameters are within valid ranges
    sampleRadius = clamp(sampleRadius, 10, 200);
    blurAmount = clamp(blurAmount, 0.1f, 1.0f);
    centerX = clamp(centerX, 0, Width - 1);
    centerY = clamp(centerY, 0, Height - 1);

    int Channels = Stride / Width;
    float maxDistance = sqrtf(Width * Width + Height * Height) / 2.0f;

    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            float dx = (float)(x - centerX);
            float dy = (float)(y - centerY);
            float distance = sqrtf(dx * dx + dy * dy);
            float strength = distance / maxDistance;

            float r = 0, g = 0, b = 0;
            float totalWeight = 0;

            int actualSamples = (int)(strength * sampleRadius);
            for (int sample = 0; sample <= actualSamples; sample++) {
                float t = (float)sample / (float)actualSamples;
                float weight = 1.0f - t;
                float sampleX = x + dx * t * blurAmount;
                float sampleY = y + dy * t * blurAmount;

                // Clamp sample coordinates to image boundaries
                int ix = (int)fmaxf(0, fminf(sampleX, Width - 1));
                int iy = (int)fmaxf(0, fminf(sampleY, Height - 1));
                int index = (iy * Stride) + (ix * Channels);

                r += Input[index] * weight;
                g += Input[index + 1] * weight;
                b += Input[index + 2] * weight;
                totalWeight += weight;
            }

            int outIndex = (y * Stride) + (x * Channels);
            if (totalWeight > 0) {
                Output[outIndex] = (unsigned char)(r / totalWeight);
                Output[outIndex + 1] = (unsigned char)(g / totalWeight);
                Output[outIndex + 2] = (unsigned char)(b / totalWeight);
            } else {
                Output[outIndex] = Input[outIndex];
                Output[outIndex + 1] = Input[outIndex + 1];
                Output[outIndex + 2] = Input[outIndex + 2];
            }

            if (Channels == 4) {
                Output[outIndex + 3] = Input[outIndex + 3];
            }
        }
    }

    return OC_STATUS_OK;
}

OC_STATUS ocularAverageBlur(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius) {
    if (Input == NULL || Output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    if (Width <= 0 || Height <= 0 || Stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    int channels = Stride / Width;
    if (channels != 3) {
        return OC_STATUS_ERR_NOTSUPPORTED;
    }

    // Ensure filter specific parameters are within valid ranges
    Radius = max(Radius, 1);

    // Allocate integral image buffers for each channel
    int* integralR = (int*)malloc((Width + 1) * (Height + 1) * sizeof(int));
    int* integralG = (int*)malloc((Width + 1) * (Height + 1) * sizeof(int));
    int* integralB = (int*)malloc((Width + 1) * (Height + 1) * sizeof(int));

    if (!integralR || !integralG || !integralB) {
        free(integralR);
        free(integralG);
        free(integralB);
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

    // Initialize first row and column of integral images
    for (int i = 0; i <= Width; i++) {
        integralR[i] = integralG[i] = integralB[i] = 0;
    }
    for (int i = 0; i <= Height; i++) {
        integralR[i * (Width + 1)] = integralG[i * (Width + 1)] = integralB[i * (Width + 1)] = 0;
    }

    // Build integral images
    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            int pos = (y * Width + x) * channels;
            int ipos = (y + 1) * (Width + 1) + (x + 1);

            integralR[ipos] = Input[pos] + integralR[ipos - 1] + integralR[ipos - (Width + 1)] - integralR[ipos - (Width + 2)];

            integralG[ipos] = Input[pos + 1] + integralG[ipos - 1] + integralG[ipos - (Width + 1)] - integralG[ipos - (Width + 2)];

            integralB[ipos] = Input[pos + 2] + integralB[ipos - 1] + integralB[ipos - (Width + 1)] - integralB[ipos - (Width + 2)];
        }
    }

    // Process each pixel using integral images
    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            // Calculate box boundaries with edge handling
            int x1 = max(0, x - Radius);
            int y1 = max(0, y - Radius);
            int x2 = min(Width - 1, x + Radius);
            int y2 = min(Height - 1, y + Radius);

            // Calculate area for normalization
            int area = (x2 - x1 + 1) * (y2 - y1 + 1);

            // Calculate sum using integral image
            int pos1 = (y1 * (Width + 1) + x1);
            int pos2 = (y1 * (Width + 1) + x2 + 1);
            int pos3 = ((y2 + 1) * (Width + 1) + x1);
            int pos4 = ((y2 + 1) * (Width + 1) + x2 + 1);

            int sumR = integralR[pos4] - integralR[pos2] - integralR[pos3] + integralR[pos1];
            int sumG = integralG[pos4] - integralG[pos2] - integralG[pos3] + integralG[pos1];
            int sumB = integralB[pos4] - integralB[pos2] - integralB[pos3] + integralB[pos1];

            // Write output
            int outPos = (y * Width + x) * channels;
            Output[outPos] = ClampToByte(sumR / area);
            Output[outPos + 1] = ClampToByte(sumG / area);
            Output[outPos + 2] = ClampToByte(sumB / area);
        }
    }

    // Clean up
    free(integralR);
    free(integralG);
    free(integralB);

    return OC_STATUS_OK;
}

OC_STATUS ocularMedianBlur(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Radius) {

    if (Input == NULL || Output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    if (Width <= 0 || Height <= 0 || Stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    int Channel = Stride / Width;
    if ((Channel != 1) && (Channel != 3)) {
        return OC_STATUS_ERR_NOTSUPPORTED;
    }

    // Ensure filter specific parameters are within valid ranges
    Radius = max(Radius, 1);

    const int Level = 256;

    if (Channel == 1) {

        int* histogram = (int*)malloc(Level * sizeof(int));
        if (histogram == NULL) {
            return OC_STATUS_ERR_OUTOFMEMORY;
        }
        for (int Y = 0; Y < Height; Y++) {
            unsigned char* LinePS = Input + Y * Stride;
            unsigned char* LinePD = Output + Y * Stride;
            memset(histogram, 0, Level * sizeof(int)); // all assigned values are 0
            int CutPoint = -1;
            int Balance = 0;

            for (int J = max(Y - Radius, 0); J <= min(Y + Radius, Height - 1); J++) {
                for (int I = max(0 - Radius, 0); I <= min(0 + Radius, Width - 1); I++) {
                    int Value = Input[J * Stride + I];
                    // Calculate the two-dimensional histogram of the first point in each row.
                    // The horizontal direction of the histogram is the Feature coordinate and
                    // the vertical direction is the Value coordinate.
                    histogram[Value]++;
                    Balance--;
                }
            }
            for (int X = 0; X < Width; X++) {

                if (Balance < 0) // the balance of the first point must be less than 0
                {
                    for (; Balance < 0 && CutPoint != Level - 1; CutPoint++) {
                        Balance += 2 * histogram[CutPoint + 1];
                    }
                } else if (Balance > 0) // If the balance value is greater than 0, move the middle value to the left
                {
                    for (; Balance > 0 && CutPoint != 0; CutPoint--) {
                        Balance -= 2 * histogram[CutPoint];
                    }
                }
                LinePD[X] = CutPoint;
                if ((X - Radius) >= 0) {
                    for (int J = max(Y - Radius, 0); J <= min(Y + Radius, Height - 1); J++) // the column of data to be moved out
                    {
                        int Value = Input[J * Stride + X - Radius];
                        histogram[Value]--;
                        if (Value <= CutPoint) // If the value moved out is less than the current median value
                            Balance--;
                        else
                            Balance++;
                    }
                }
                if ((X + Radius + 1) <= Width - 1) {
                    for (int J = max(Y - Radius, 0); J <= min(Y + Radius, Height - 1); J++) {
                        int Value = Input[J * Stride + X + Radius + 1];
                        histogram[Value]++;
                        if (Value <= CutPoint) // If the value moved out is less than the current median value
                            Balance++;
                        else
                            Balance--;
                    }
                }
            }
        }
        free(histogram);
    } else {
        unsigned char* SrcR = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
        unsigned char* SrcG = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
        unsigned char* SrcB = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));

        unsigned char* DstR = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
        unsigned char* DstG = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));
        unsigned char* DstB = (unsigned char*)malloc(Width * Height * sizeof(unsigned char));

        SplitRGB(Input, SrcB, SrcG, SrcR, Width, Height, Stride);

        // TODO: Investigate using parallel processing here
        ocularMedianBlur(SrcR, DstR, Width, Height, Width, Radius);
        ocularMedianBlur(SrcG, DstG, Width, Height, Width, Radius);
        ocularMedianBlur(SrcB, DstB, Width, Height, Width, Radius);

        CombineRGB(DstB, DstG, DstR, Output, Width, Height, Stride);

        free(SrcR);
        free(SrcG);
        free(SrcB);
        free(DstR);
        free(DstG);
        free(DstB);
    }

    return OC_STATUS_OK;
}

OC_STATUS ocularMotionBlurFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int Distance, int Angle) {

    if (Input == NULL || Output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    if (Width <= 0 || Height <= 0 || Stride <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    // Ensure filter specific parameters are within valid ranges
    Distance = max(Distance, 1); // Changed from min to max since we want at least 1 pixel distance
    Angle = clamp(Angle, -180, 180);

    int channels = Stride / Width;

    // Pre-calculate angle and direction
    float radian = ((float)(Angle % 360) + 180.0f) / 180.0f * M_PI;
    int dx = (int)((float)Distance * fastCos(radian) + 0.5f);
    int dy = (int)((float)Distance * fastSin(radian) + 0.5f);

    // Pre-calculate sign to avoid branching in loop
    int sign = (dx == 0) ? 0 : (dx < 0 ? -1 : 1);
    int absDistance = abs(dx);

    // Process image in chunks for better cache utilization
    const int CHUNK_SIZE = 32;
    for (int blockY = 0; blockY < Height; blockY += CHUNK_SIZE) {
        for (int blockX = 0; blockX < Width; blockX += CHUNK_SIZE) {
            int endY = min(blockY + CHUNK_SIZE, Height);
            int endX = min(blockX + CHUNK_SIZE, Width);

            // Pre-allocate arrays for accumulation
            int sums[4] = { 0 }; // Support up to 4 channels
            float invCount;

            for (int y = blockY; y < endY; y++) {
                unsigned char* pInput = Input + (y * Stride);
                unsigned char* pOutput = Output + (y * Stride);

                for (int x = blockX; x < endX; x++) {
                    // Reset sums for new pixel
                    memset(sums, 0, sizeof(sums));
                    int count = 0;

                    // Vectorizable inner loop
                    for (int p = 0; p < absDistance; p++) {
                        int yOffset = y + p * sign;
                        int xOffset = x + p * sign;

                        if (yOffset >= 0 && yOffset < Height && xOffset >= 0 && xOffset < Width) {
                            unsigned char* pOffset = Input + (yOffset * Stride + xOffset * channels);
                            count++;

                            // Unrolled channel accumulation
                            switch (channels) {
                            case 4:
                                sums[3] += pOffset[3];
                                // fallthrough
                            case 3:
                                sums[2] += pOffset[2];
                                sums[1] += pOffset[1];
                                sums[0] += pOffset[0];
                                break;
                            case 1: sums[0] += pOffset[0]; break;
                            }
                        }
                    }

                    // Write output
                    if (count == 0) {
                        // Direct copy if no valid samples
                        memcpy(pOutput + x * channels, pInput + x * channels, channels);
                    } else {
                        // Pre-calculate inverse count for multiplication instead of division
                        invCount = 1.0f / count;
                        switch (channels) {
                        case 4:
                            pOutput[x * channels + 3] = ClampToByte(sums[3] * invCount + 0.5f);
                            // fallthrough
                        case 3:
                            pOutput[x * channels + 2] = ClampToByte(sums[2] * invCount + 0.5f);
                            pOutput[x * channels + 1] = ClampToByte(sums[1] * invCount + 0.5f);
                            pOutput[x * channels + 0] = ClampToByte(sums[0] * invCount + 0.5f);
                            break;
                        case 1: pOutput[x * channels] = ClampToByte(sums[0] * invCount + 0.5f); break;
                        }
                    }
                }
            }
        }
    }

    return OC_STATUS_OK;
}

OC_STATUS ocularRadialBlur(unsigned char* Input, unsigned char* Output, int Width, int Height, int Stride, int centerX, int centerY, int intensity) {

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

    // Ensure filter specific parameters are within valid ranges
    centerX = clamp(centerX, 0, Width - 1);
    centerY = clamp(centerY, 0, Height - 1);
    intensity = clamp(intensity, 1, 100);

    // Pre-calculate angle increment and inverse intensity
    const float angleIncrement = 0.005f;
    const float invIntensity = 1.0f / intensity;

    // Process image in chunks for better cache utilization
    const int CHUNK_SIZE = 32;

    if (channels == 1) {
        for (int blockY = 0; blockY < Height; blockY += CHUNK_SIZE) {
            for (int blockX = 0; blockX < Width; blockX += CHUNK_SIZE) {
                int endY = min(blockY + CHUNK_SIZE, Height);
                int endX = min(blockX + CHUNK_SIZE, Width);

                for (int y = blockY; y < endY; y++) {
                    unsigned char* pOut = Output + y * Stride;

                    for (int x = blockX; x < endX; x++) {
                        float dx = (float)(x - centerX);
                        float dy = (float)(y - centerY);
                        float distance = sqrtf(dx * dx + dy * dy);
                        float angle = atan2f(dy, dx);

                        float sum = 0.0f;

                        // Unroll the intensity loop for better performance
                        for (int n = 0; n < intensity; n += 4) {
                            // Process 4 samples at once
                            for (int k = 0; k < 4 && (n + k) < intensity; k++) {
                                float curAngle = angle + (n + k) * angleIncrement;
                                float cosAngle = fastCos(curAngle);
                                float sinAngle = fastSin(curAngle);

                                int newX = (int)(distance * cosAngle + centerX);
                                int newY = (int)(distance * sinAngle + centerY);

                                // Clamp coordinates
                                newX = min(Width - 1, max(0, newX));
                                newY = min(Height - 1, max(0, newY));

                                sum += Input[newY * Stride + newX];
                            }
                        }

                        pOut[x] = ClampToByte(sum * invIntensity);
                    }
                }
            }
        }
    }
    if (channels == 3) {
        for (int blockY = 0; blockY < Height; blockY += CHUNK_SIZE) {
            for (int blockX = 0; blockX < Width; blockX += CHUNK_SIZE) {
                int endY = min(blockY + CHUNK_SIZE, Height);
                int endX = min(blockX + CHUNK_SIZE, Width);

                float sums[3];

                for (int y = blockY; y < endY; y++) {
                    unsigned char* pOut = Output + y * Stride;

                    for (int x = blockX; x < endX; x++) {
                        float dx = (float)(x - centerX);
                        float dy = (float)(y - centerY);
                        float distance = sqrtf(dx * dx + dy * dy);
                        float angle = atan2f(dy, dx);

                        sums[0] = sums[1] = sums[2] = 0.0f;

                        // Unroll the intensity loop
                        for (int n = 0; n < intensity; n += 4) {
                            // Process 4 samples at once
                            for (int k = 0; k < 4 && (n + k) < intensity; k++) {
                                float curAngle = angle + (n + k) * angleIncrement;
                                float cosAngle = fastCos(curAngle);
                                float sinAngle = fastSin(curAngle);

                                int newX = (int)(distance * cosAngle + centerX);
                                int newY = (int)(distance * sinAngle + centerY);

                                // Clamp coordinates
                                newX = min(Width - 1, max(0, newX));
                                newY = min(Height - 1, max(0, newY));

                                const unsigned char* pIn = Input + (newY * Stride + newX * 3);
                                sums[0] += pIn[0];
                                sums[1] += pIn[1];
                                sums[2] += pIn[2];
                            }
                        }

                        pOut[x * 3] = ClampToByte(sums[0] * invIntensity);
                        pOut[x * 3 + 1] = ClampToByte(sums[1] * invIntensity);
                        pOut[x * 3 + 2] = ClampToByte(sums[2] * invIntensity);
                    }
                }
            }
        }
    }

    return OC_STATUS_OK;
}