#include "util.h"
#include "ocular.h"

OC_STATUS GetValidCoordinate(int Width, int Height, int Left, int Right, int Top, int Bottom, OcEdgeMode Edge, OcImage** Row, OcImage** Col) {

    if ((Left < 0) || (Right < 0) || (Top < 0) || (Bottom < 0))
        return OC_STATUS_ERR_INVALIDPARAMETER;

    OC_STATUS Ret = ocularCreateImage(Width + Left + Right, 1, OC_DEPTH_32S, 1, Row);
    if (Ret != OC_STATUS_OK)
        return Ret;
    Ret = ocularCreateImage(1, Height + Top + Bottom, OC_DEPTH_32S, 1, Col);
    if (Ret != OC_STATUS_OK)
        return Ret;

    int XX, YY, *RowPos = (int*)(*Row)->Data, *ColPos = (int*)(*Col)->Data;

    for (int X = -Left; X < Width + Right; X++) {
        if (X < 0) {
            if (Edge == OC_EDGE_WRAP) // Repeating edge pixels
                RowPos[X + Left] = 0;
            else {
                XX = -X;
                while (XX >= Width)
                    XX -= Width; // mirror data
                RowPos[X + Left] = XX;
            }
        } else if (X >= Width) {
            if (Edge == OC_EDGE_WRAP)
                RowPos[X + Left] = Width - 1;
            else {
                XX = Width - (X - Width + 2);
                while (XX < 0)
                    XX += Width;
                RowPos[X + Left] = XX;
            }
        } else {
            RowPos[X + Left] = X;
        }
    }

    for (int Y = -Top; Y < Height + Bottom; Y++) {
        if (Y < 0) {
            if (Edge == OC_EDGE_WRAP)
                ColPos[Y + Top] = 0;
            else {
                YY = -Y;
                while (YY >= Height)
                    YY -= Height;
                ColPos[Y + Top] = YY;
            }
        } else if (Y >= Height) {
            if (Edge == OC_EDGE_WRAP)
                ColPos[Y + Top] = Height - 1;
            else {
                YY = Height - (Y - Height + 2);
                while (YY < 0)
                    YY += Height;
                ColPos[Y + Top] = YY;
            }
        } else {
            ColPos[Y + Top] = Y;
        }
    }
    return OC_STATUS_OK;
}

OC_STATUS GetExpandImage(OcImage* Src, OcImage** Dest, int Left, int Right, int Top, int Bottom, OcEdgeMode Edge) {

    if (Src == NULL)
        return OC_STATUS_ERR_NULLREFERENCE;
    if (Src->Data == NULL)
        return OC_STATUS_ERR_NULLREFERENCE;

    int X, Y, SrcWidth, SrcHeight, SrcStride, DstWidth, DstHeight, DstStride, Channels;
    unsigned char *LinePS, *LinePD;

    SrcWidth = Src->Width;
    SrcHeight = Src->Height;
    DstWidth = SrcWidth + Left + Right;
    DstHeight = SrcHeight + Top + Bottom;

    if (*Dest != NULL) {
        if ((*Dest)->Data == NULL)
            return OC_STATUS_ERR_NULLREFERENCE;
        if ((*Dest)->Depth != OC_DEPTH_8U || (*Dest)->Width != DstWidth || (*Dest)->Height != DstHeight)
            return OC_STATUS_ERR_INVALIDPARAMETER;
    } else {
        OC_STATUS Ret = ocularCreateImage(DstWidth, DstHeight, Src->Depth, Src->Channels, Dest);
        if (Ret != OC_STATUS_OK)
            return Ret;
    }

    SrcStride = Src->Stride;
    DstStride = (*Dest)->Stride;
    Channels = Src->Channels;

    OcImage *Row, *Col;
    GetValidCoordinate(SrcWidth, SrcHeight, Left, Right, Top, Bottom, Edge, &Row, &Col);
    int *RowPos = (int*)Row->Data, *ColPos = (int*)Col->Data;

    for (Y = 0; Y < SrcHeight; Y++) {
        LinePD = (*Dest)->Data + (Y + Top) * DstStride;
        LinePS = Src->Data + Y * SrcStride;
        for (X = 0; X < Left; X++) {
            memcpy(LinePD, LinePS + RowPos[X] * Channels, Channels);

            LinePD += Channels;
        }
        memcpy(LinePD, LinePS, SrcWidth * Channels);
        LinePD += SrcWidth * Channels;

        for (X = Left + SrcWidth; X < Left + SrcWidth + Right; X++) {
            memcpy(LinePD, LinePS + RowPos[X] * Channels, Channels);
            LinePD += Channels;
        }
    }
    for (Y = 0; Y < Top; Y++) {
        memcpy((*Dest)->Data + Y * DstStride, (*Dest)->Data + (Top + ColPos[Y]) * DstStride, DstStride);
    }

    for (Y = Top + SrcHeight; Y < Top + SrcHeight + Bottom; Y++) {
        memcpy((*Dest)->Data + Y * DstStride, (*Dest)->Data + (Top + ColPos[Y]) * DstStride, DstStride);
    }
    ocularFreeImage(&Row);
    ocularFreeImage(&Col);
    return OC_STATUS_OK;
}

OC_STATUS SplitRGBA(unsigned char* Input, int Width, int Height, int Stride, OcImage** Red, OcImage** Green, OcImage** Blue, OcImage** Alpha) {

    if (Input == NULL)
        return OC_STATUS_ERR_NULLREFERENCE;

    int Channels = Stride / Width;

    OC_STATUS Ret;
    Ret = ocularCreateImage(Width, Height, OC_DEPTH_8U, 1, Red);
    if (Ret != OC_STATUS_OK)
        goto FreeMemory;
    Ret = ocularCreateImage(Width, Height, OC_DEPTH_8U, 1, Green);
    if (Ret != OC_STATUS_OK)
        goto FreeMemory;
    Ret = ocularCreateImage(Width, Height, OC_DEPTH_8U, 1, Blue);
    if (Ret != OC_STATUS_OK)
        goto FreeMemory;
    if (Channels == 4) {
        Ret = ocularCreateImage(Width, Height, OC_DEPTH_8U, 1, Alpha);
        if (Ret != OC_STATUS_OK)
            goto FreeMemory;
    }

    int X, Y, Block;
    unsigned char *LinePS, *LinePB, *LinePG, *LinePR, *LinePA;
    const int BlockSize = 8;
    Block = Width / BlockSize;

    if (Channels == 3) {
        for (Y = 0; Y < Height; Y++) {
            LinePS = Input + Y * Stride;
            LinePB = (*Blue)->Data + Y * (*Blue)->Stride;
            LinePG = (*Green)->Data + Y * (*Green)->Stride;
            LinePR = (*Red)->Data + Y * (*Red)->Stride;
            for (X = 0; X < Block * BlockSize; X += BlockSize) {
                LinePB[0] = LinePS[0];
                LinePG[0] = LinePS[1];
                LinePR[0] = LinePS[2];
                LinePB[1] = LinePS[3];
                LinePG[1] = LinePS[4];
                LinePR[1] = LinePS[5];
                LinePB[2] = LinePS[6];
                LinePG[2] = LinePS[7];
                LinePR[2] = LinePS[8];
                LinePB[3] = LinePS[9];
                LinePG[3] = LinePS[10];
                LinePR[3] = LinePS[11];
                LinePB[4] = LinePS[12];
                LinePG[4] = LinePS[13];
                LinePR[4] = LinePS[14];
                LinePB[5] = LinePS[15];
                LinePG[5] = LinePS[16];
                LinePR[5] = LinePS[17];
                LinePB[6] = LinePS[18];
                LinePG[6] = LinePS[19];
                LinePR[6] = LinePS[20];
                LinePB[7] = LinePS[21];
                LinePG[7] = LinePS[22];
                LinePR[7] = LinePS[23];
                LinePB += 8;
                LinePG += 8;
                LinePR += 8;
                LinePS += 24;
            }
            while (X < Width) {
                LinePB[0] = LinePS[0];
                LinePG[0] = LinePS[1];
                LinePR[0] = LinePS[2];
                LinePB++;
                LinePG++;
                LinePR++;
                LinePS += 3;
                X++;
            }
        }
    } else if (Channels == 4) {
        for (Y = 0; Y < Height; Y++) {
            LinePS = Input + Y * Stride;
            LinePB = (*Blue)->Data + Y * (*Blue)->Stride;
            LinePG = (*Green)->Data + Y * (*Green)->Stride;
            LinePR = (*Red)->Data + Y * (*Red)->Stride;
            LinePA = (*Alpha)->Data + Y * (*Alpha)->Stride;
            for (X = 0; X < Block * BlockSize; X += BlockSize) {
                LinePB[0] = LinePS[0];
                LinePG[0] = LinePS[1];
                LinePR[0] = LinePS[2];
                LinePA[0] = LinePS[3];
                LinePB[1] = LinePS[4];
                LinePG[1] = LinePS[5];
                LinePR[1] = LinePS[6];
                LinePA[1] = LinePS[7];
                LinePB[2] = LinePS[8];
                LinePG[2] = LinePS[9];
                LinePR[2] = LinePS[10];
                LinePA[2] = LinePS[11];
                LinePB[3] = LinePS[12];
                LinePG[3] = LinePS[13];
                LinePR[3] = LinePS[14];
                LinePA[3] = LinePS[15];
                LinePB[4] = LinePS[16];
                LinePG[4] = LinePS[17];
                LinePR[4] = LinePS[18];
                LinePA[4] = LinePS[19];
                LinePB[5] = LinePS[20];
                LinePG[5] = LinePS[21];
                LinePR[5] = LinePS[22];
                LinePA[5] = LinePS[23];
                LinePB[6] = LinePS[24];
                LinePG[6] = LinePS[25];
                LinePR[6] = LinePS[26];
                LinePA[6] = LinePS[27];
                LinePB[7] = LinePS[28];
                LinePG[7] = LinePS[29];
                LinePR[7] = LinePS[30];
                LinePA[7] = LinePS[31];
                LinePB += 8;
                LinePG += 8;
                LinePR += 8;
                LinePA += 8;
                LinePS += 32;
            }
            while (X < Width) {
                LinePB[0] = LinePS[0];
                LinePG[0] = LinePS[1];
                LinePR[0] = LinePS[2];
                LinePA[0] = LinePS[3];
                LinePB++;
                LinePG++;
                LinePR++;
                LinePA++;
                LinePS += 4;
                X++;
            }
        }
    }
    return OC_STATUS_OK;

FreeMemory:
    if (*Red != NULL)
        ocularFreeImage(Red);
    if (*Green != NULL)
        ocularFreeImage(Green);
    if (*Blue != NULL)
        ocularFreeImage(Blue);
    return Ret;
}

OC_STATUS CombineRGBA(unsigned char* Output, int Width, int Height, int Stride, OcImage* Red, OcImage* Green, OcImage* Blue, OcImage* Alpha) {

    if (Output == NULL)
        return OC_STATUS_ERR_NULLREFERENCE;

    int Channels = Stride / Width;

    if (Channels == 4) {
        if (Alpha == NULL || Alpha->Data == NULL || Alpha->Channels != 1)
            return OC_STATUS_ERR_NULLREFERENCE;
    }

    int X, Y, Block;
    unsigned char *LinePD, *LinePB, *LinePG, *LinePR, *LinePA;
    const int BlockSize = 8;
    Block = Width / BlockSize;

    if (Channels == 3) {
        for (Y = 0; Y < Height; Y++) {
            LinePD = Output + Y * Stride;
            LinePB = Blue->Data + Y * Blue->Stride;
            LinePG = Green->Data + Y * Green->Stride;
            LinePR = Red->Data + Y * Red->Stride;
            for (X = 0; X < Block * BlockSize; X += BlockSize) {
                LinePD[0] = LinePB[0];
                LinePD[1] = LinePG[0];
                LinePD[2] = LinePR[0];
                LinePD[3] = LinePB[1];
                LinePD[4] = LinePG[1];
                LinePD[5] = LinePR[1];
                LinePD[6] = LinePB[2];
                LinePD[7] = LinePG[2];
                LinePD[8] = LinePR[2];
                LinePD[9] = LinePB[3];
                LinePD[10] = LinePG[3];
                LinePD[11] = LinePR[3];
                LinePD[12] = LinePB[4];
                LinePD[13] = LinePG[4];
                LinePD[14] = LinePR[4];
                LinePD[15] = LinePB[5];
                LinePD[16] = LinePG[5];
                LinePD[17] = LinePR[5];
                LinePD[18] = LinePB[6];
                LinePD[19] = LinePG[6];
                LinePD[20] = LinePR[6];
                LinePD[21] = LinePB[7];
                LinePD[22] = LinePG[7];
                LinePD[23] = LinePR[7];
                LinePB += 8;
                LinePG += 8;
                LinePR += 8;
                LinePD += 24;
            }
            while (X < Width) {
                LinePD[0] = LinePB[0];
                LinePD[1] = LinePG[0];
                LinePD[2] = LinePR[0];
                LinePB++;
                LinePG++;
                LinePR++;
                LinePD += 3;
                X++;
            }
        }
    } else if (Channels == 4) {
        for (Y = 0; Y < Height; Y++) {
            LinePD = Output + Y * Stride;
            LinePB = Blue->Data + Y * Blue->Stride;
            LinePG = Green->Data + Y * Green->Stride;
            LinePR = Red->Data + Y * Red->Stride;
            LinePA = Alpha->Data + Y * Alpha->Stride;
            for (X = 0; X < Block * BlockSize; X += BlockSize) {
                LinePD[0] = LinePB[0];
                LinePD[1] = LinePG[0];
                LinePD[2] = LinePR[0];
                LinePD[3] = LinePA[0];
                LinePD[4] = LinePB[1];
                LinePD[5] = LinePG[1];
                LinePD[6] = LinePR[1];
                LinePD[7] = LinePA[1];
                LinePD[8] = LinePB[2];
                LinePD[9] = LinePG[2];
                LinePD[10] = LinePR[2];
                LinePD[11] = LinePA[2];
                LinePD[12] = LinePB[3];
                LinePD[13] = LinePG[3];
                LinePD[14] = LinePR[3];
                LinePD[15] = LinePA[3];
                LinePD[16] = LinePB[4];
                LinePD[17] = LinePG[4];
                LinePD[18] = LinePR[4];
                LinePD[19] = LinePA[4];
                LinePD[20] = LinePB[5];
                LinePD[21] = LinePG[5];
                LinePD[22] = LinePR[5];
                LinePD[23] = LinePA[5];
                LinePD[24] = LinePB[6];
                LinePD[25] = LinePG[6];
                LinePD[26] = LinePR[6];
                LinePD[27] = LinePA[6];
                LinePD[28] = LinePB[7];
                LinePD[29] = LinePG[7];
                LinePD[30] = LinePR[7];
                LinePD[31] = LinePA[7];
                LinePB += 8;
                LinePG += 8;
                LinePR += 8;
                LinePA += 8;
                LinePD += 32;
            }
            while (X < Width) {
                LinePD[0] = LinePB[0];
                LinePD[1] = LinePG[0];
                LinePD[2] = LinePR[0];
                LinePD[3] = LinePA[0];
                LinePB++;
                LinePG++;
                LinePD++;
                LinePA++;
                LinePD += 4;
                X++;
            }
        }
    }
    return OC_STATUS_OK;
}

void CopyAlphaChannel(unsigned char* Src, unsigned char* Dest, int Width, int Height, int Stride) {
    int Channels = Stride / Width;

    if (Channels != 4)
        return;

    unsigned char *SrcP = Src, *DestP = Dest;
    int Y, Index = 3;
    for (Y = 0; Y < Width * Height; Y++, Index += 4) {
        SrcP[Index] = DestP[Index];
    }
}