#include "util.h"

void HistogramAddShort(unsigned short* X, unsigned short* Y) {

    for (int i = 0; i <= 248; i += 8) {
        *(Y + i) = Y[i] + X[i];
    }
}

void HistogramSubAddShort(unsigned short* X, unsigned short* Y, unsigned short* Z) {

    for (int i = 0; i <= 248; i += 8) {
        *(Z + i) = (Y[i] + Z[i]) - X[i];
    }
}

unsigned char HistogramCalc(unsigned short* Hist, unsigned char Value, unsigned short* Intensity) {
    int Weight = 0, Sum = 0, Divisor = 0;
    unsigned short* Offset = Intensity + 255 - Value;
    for (int Y = 0; Y < 256; Y++) {
        Weight = Hist[Y] * Offset[Y];
        Sum += Weight * Y;
        Divisor += Weight;
    }
    if (Divisor > 0)
        return (Sum + (Divisor >> 1)) / Divisor; // rounding
    else
        return Value;
}

void HistogramCalc2(unsigned short* Hist, int Intensity, unsigned char* Pixel, int Threshold) {
    int Low, High, Sum = 0, Weight = 0;
    Low = Intensity - Threshold;
    High = Intensity + Threshold;
    if (Low < 0)
        Low = 0;
    if (High > 255)
        High = 255;
    for (int i = Low; i <= High; i++) {
        Weight += Hist[i];
        Sum += Hist[i] * i;
    }
    if (Weight != 0)
        *Pixel = Sum / Weight;
}

int GetMirrorPos(int Length, int Pos) {
    if (Pos < 0) {
        Pos = -Pos;
        while (Pos > Length)
            Pos -= Length;
    } else if (Pos >= Length) {
        Pos = Length - (Pos - Length + 2);
        while (Pos < 0)
            Pos += Length;
    }
    return Pos;
}

void GetOffsetPos(int* Pos, int Length, int Left, int Right) {
    for (int X = -Left; X < Length + Right; X++) {
        Pos[X + Left] = GetMirrorPos(Length, X);
    }
}