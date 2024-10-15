#include "ocr.h"
#include "util.h"


unsigned char getMedian(unsigned char* window, int size) {
    // Simple bubble sort
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (window[j] > window[j + 1]) {
                unsigned char temp = window[j];
                window[j] = window[j + 1];
                window[j + 1] = temp;
            }
        }
    }
    return window[size / 2];
}

bool isTextImage(unsigned char* Input, int Width, int Height) {
    const int blacklimit = 20;
    const int greylimit = 140;
    const int contrast_offset = 80;

    int prev_color[256];
    int cur_color[256];

    for (int i = 0; i < 256; i++) {
        cur_color[i] = 0;
        prev_color[i] = 0;
    }

    for (int i = 0; i <= blacklimit; i++) {
        // black
        cur_color[i] = 100;
        prev_color[i] = 100000;
    }

    for (int i = blacklimit + 1 + contrast_offset; i <= greylimit; i++) {
        // gray
        cur_color[i] = 10;
        prev_color[i] = 10000;
    }

    for (int i = greylimit + 1 + contrast_offset; i <= 255; i++) {
        // white
        cur_color[i] = 1;
        prev_color[i] = 1000;
    }
    int line_count = 0;


    int n = -1;
    for (int y = 0; y < Height; y += 10) {
        n++;
        int white_amt = 0;
        unsigned char* buffer = Input + y * Width;
        int x = 0;
        for (x = 1; x < Width; x++) {
            const unsigned char prev_pixel = buffer[(x - 1)];
            const unsigned char cur_pixel = buffer[x];

            if ((prev_color[prev_pixel]) && (cur_color[cur_pixel])) {
                // whether it is white
                if ((prev_color[prev_pixel] + cur_color[cur_pixel]) == 1001) {
                    white_amt++;
                }
            }
        }
        // white row
        if (((float)white_amt / (float)x) > 0.85f) {
            line_count++;
        }
    }

    float line_count_ratio = (n != 0.f) ? (float)line_count / (float)n : 0.0f;

    if (line_count_ratio < 0.4f || line_count_ratio > 1.0f) {
        return false;
    }

    return true;
}

float calcSkewAngle(unsigned char* Input, int Width, int Height, OcRect* CheckRectPtr, int maxSkewToDetect, int stepsPerDegree,
                    int localPeakRadius, int nLineCount) {
    OcRect CheckRect = *CheckRectPtr;
    // Make sure the specified area is within the original image range
    CheckRect.x = clamp(CheckRect.x, 0, Width - 1);
    CheckRect.y = clamp(CheckRect.y, 0, Height - 1);
    CheckRect.Width = clamp(CheckRect.Width, 1, Width - 1);
    CheckRect.Height = clamp(CheckRect.Height, 1, Height - 1);

    // processing parameters
    maxSkewToDetect = clamp(maxSkewToDetect, 0, 91);
    localPeakRadius = clamp(localPeakRadius, 1, 10);
    stepsPerDegree = clamp(stepsPerDegree, 1, 10);
    int houghHeight = (2 * maxSkewToDetect * stepsPerDegree);
    float thetaStep = (2 * maxSkewToDetect * M_PI / 180) / houghHeight;
    int halfWidth = Width >> 1;
    int halfHeight = Height >> 1;
    // calculate hough map width
    int halfHoughWidth = (int)sqrtf((float)(halfWidth * halfWidth + halfHeight * halfHeight));
    int houghWidth = (halfHoughWidth * 2);
    float minTheta = 90.0f - maxSkewToDetect;
    unsigned short* houghMap = (unsigned short*)calloc(houghHeight * houghWidth, sizeof(unsigned short));
    float* sinMap = (float*)malloc(houghHeight * sizeof(float));
    float* cosMap = (float*)malloc(houghHeight * sizeof(float));
    HoughLine* HoughLines = (HoughLine*)calloc(houghHeight * houghWidth, sizeof(HoughLine));
    if (houghMap == NULL || sinMap == NULL || cosMap == NULL || HoughLines == NULL) {
        if (houghMap) {
            free(houghMap);
            houghMap = NULL;
        }
        if (sinMap) {
            free(sinMap);
            sinMap = NULL;
        }
        if (cosMap) {
            free(cosMap);
            cosMap = NULL;
        }
        if (HoughLines) {
            free(HoughLines);
            HoughLines = NULL;
        }
        return 0.0f;
    } else {
        // precomputed sin and cos tables
        float mt = (minTheta * M_PI / 180.0f);
        for (int i = 0; i < houghHeight; i++) {
            float cur_weight = mt + (i * thetaStep);
            sinMap[i] = fastSin(cur_weight);
            cosMap[i] = fastCos(cur_weight);
        }
    }
    int startX = -halfWidth + CheckRect.x;
    int startY = -halfHeight + CheckRect.y;
    int stopX = Width - halfWidth - (Width - CheckRect.Width);
    int stopY = Height - halfHeight - (Height - CheckRect.Height) - 1;
    int offset = Width - CheckRect.Width;


    unsigned char* src = Input + CheckRect.y * Width + CheckRect.x;
    unsigned char* srcBelow = src + Width;

    for (int Y = startY; Y < stopY; Y++) {
        for (int X = startX; X < stopX; X++, src++, srcBelow++) {
            if ((*src < 128) && (*srcBelow >= 128)) {
                for (int theta = 0; theta < houghHeight; theta++) {
                    int radius = (int)(cosMap[theta] * X - sinMap[theta] * Y) + halfHoughWidth;

                    if ((radius < 0) || (radius >= houghWidth)) {
                        continue;
                    }

                    houghMap[theta * houghWidth + radius]++;
                }
            }
        }
        src += offset;
        srcBelow += offset;
    }


    // find the maximum value of the hough map
    float maxMapIntensity = 0.0000000001f;
    for (int theta = 0; theta < houghHeight; theta++) {
        unsigned short* houghMapLine = houghMap + theta * houghWidth;
        for (int radius = 0; radius < houghWidth; radius++) {
            maxMapIntensity = max(maxMapIntensity, houghMapLine[radius]);
        }
    }
    int minLineIntensity = Width / 10;

    // collects straight lines greater than or equal to the specified intensity

    int lineIntensity = 0;
    bool foundGreater = false;
    int lineSize = 0;
    for (int theta = 0; theta < houghHeight; theta++) {
        unsigned short* houghMapLine = houghMap + theta * houghWidth;
        for (int radius = 0; radius < houghWidth; radius++) {
            // get current intensity
            lineIntensity = houghMapLine[radius];

            if (lineIntensity < minLineIntensity) {
                continue;
            }

            foundGreater = false;

            // check adjacent edges
            for (int t = theta - localPeakRadius, ttMax = theta + localPeakRadius; t < ttMax; t++) {
                // skip map value
                if (t < 0) {
                    continue;
                }
                if (t >= houghHeight) {
                    break;
                }

                // if it is not a local maximum jump out
                if (foundGreater == true) {
                    break;
                }
                for (int r = radius - localPeakRadius, trMax = radius + localPeakRadius; r < trMax; r++) {
                    // skip map value
                    if (r < 0) {
                        continue;
                    }
                    if (r >= houghWidth) {
                        break;
                    }
                    // compare current value with adjacent edge
                    if (houghMap[t * houghWidth + r] > lineIntensity) {
                        foundGreater = true;
                        break;
                    }
                }
            }
            // it may be a local maximum record it
            if (!foundGreater) {
                HoughLine tempVar;
                tempVar.Theta = 90.0f - maxSkewToDetect + (theta) / stepsPerDegree;
                tempVar.Radius = (radius - halfHoughWidth);
                tempVar.Intensity = lineIntensity;
                tempVar.RelativeIntensity = lineIntensity / maxMapIntensity;
                HoughLines[lineSize] = tempVar;
                lineSize++;
            }
        }
    }

    float skewAngle = 0;
    if (lineSize > 0) {
        // sort from large to small
        HoughLine temp;
        for (int i = 0; i < lineSize; i++) {
            for (int j = 0; j < lineSize - 1; j++) {
                if (HoughLines[j].Intensity < HoughLines[j + 1].Intensity) {
                    temp = HoughLines[j + 1];
                    HoughLines[j + 1] = HoughLines[j];
                    HoughLines[j] = temp;
                }
            }
        }

        int n = min(nLineCount, lineSize);

        float sumIntensity = 0;

        for (int i = 0; i < n; i++) {
            if (HoughLines[i].RelativeIntensity > 0.5f) {
                skewAngle += (HoughLines[i].Theta * HoughLines[i].RelativeIntensity);
                sumIntensity += HoughLines[i].RelativeIntensity;
            }
        }
        skewAngle = skewAngle / sumIntensity;
    }
    if (houghMap) {
        free(houghMap);
        houghMap = NULL;
    }
    if (sinMap) {
        free(sinMap);
        sinMap = NULL;
    }
    if (cosMap) {
        free(cosMap);
        cosMap = NULL;
    }
    if (HoughLines) {
        free(HoughLines);
        HoughLines = NULL;
    }
    if (skewAngle != 0) {
        return skewAngle - 90.0f;
    }
    return skewAngle;
}