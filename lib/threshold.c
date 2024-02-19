/**
 * @file: threshold.c
 * @author Warren Galyen
 * Created: 2-19-2024
 * Last Updated: 2-19-2024
 * Last update: initial implementation
 *
 * @brief Ocular histogram threshold method implementations
 */

#include <string.h>
#include <math.h>
#include <float.h>
#include <stdbool.h>
#include "threshold.h"

int GetMeanThreshold(int histogram[]) {
    int sum = 0, amount = 0;
    for (int y = 0; y < 256; y++) {
        amount += histogram[y];
        sum += y * histogram[y];
    }
    return sum / amount;
}

int GetHuangFuzzyThreshold(int histogram[]) {
    int x, y;
    int first, last;
    int threshold = -1;
    float bestEntropy, entropy;
    bestEntropy = FLT_MAX;
    // Find the first and last non-zero level values
    for (first = 0; first < 256 && histogram[first] == 0; first++)
        ;
    for (last = 255; last > first && histogram[last] == 0; last--)
        ;
    if (first == last)
        return first; // There is only one color in the image
    if (first + 1 == last)
        return first; // There are only two colors in the image

    // Calculate the cumulative histogram and the corresponding cumulative histogram with weights
    int S[last + 1];
    int W[last + 1]; // For extremely large images, the data stored in this array may exceed the representation range of int.
                     // You may consider using the long type instead.

    S[0] = histogram[0];
    for (y = first > 1 ? first : 1; y <= last; y++) {
        S[y] = S[y - 1] + histogram[y];
        W[y] = W[y - 1] + y * histogram[y];
    }

    // Establish the loop tables used in equations (4) and (6)
    float sum[last + 1 - first];
    int lenSum = sizeof(sum) / sizeof(sum[0]);
    for (y = 1; y < lenSum; y++) {
        float mu = 1 / (1 + (double)y / (last - first)); // Eq. (4)
        sum[y] = -mu * log(mu) - (1 - mu) * log(1 - mu); // Eq. (6)
    }

    // Iterative calculation of the optimal threshold
    for (y = first; y <= last; y++) {
        entropy = 0;
        int mu = (int)round((double)W[y] / S[y]);
        for (x = first; x <= y; x++) {
            entropy += sum[abs(x - mu)] * histogram[x];
        }
        mu = (int)round((double)(W[last] - W[y]) / (S[last] - S[y]));
        for (x = y + 1; x <= last; x++) {
            entropy += sum[abs(x - mu)] * histogram[x];
        }
        if (bestEntropy > entropy) {
            bestEntropy = entropy; // take the minimum entropy as the best threshold
            threshold = y;
        }
    }
    return threshold;
}

int GetMinimumThreshold(int histogram[], int* histogramSmooth[]) {
    int y, iter = 0;
    int histogramC[256] = { 0 };
    int histogramCC[256] = { 0 }; // The process of averaging will destroy the original data, so we copy it
    for (y = 0; y < 256; y++) {
        histogramC[y] = histogram[y];
        histogramCC[y] = histogram[y];
    }

    // Smooth the histogram by averaging three points
    while (isBimodal(histogramCC) == false) {                                 // determine if it is already a bimodal image
        histogramCC[0] = (histogramC[0] + histogramC[0] + histogramC[1]) / 3; // The first point
        for (y = 1; y < 255; y++)
            histogramCC[y] = (histogramC[y - 1] + histogramC[y] + histogramC[y + 1]) / 3; // The middle point
        histogramC[255] = (histogramC[254] + histogramC[255] + histogramC[255]) / 3;      // The last point
        memcpy(histogramC, histogramCC, sizeof(float) * 256);
        iter++;
        if (iter >= 1000)
            return -1; // histogram cannot smooth bimodal, return error code;
    }
    for (y = 0; y < 256; y++)
        histogramSmooth[y] = &histogramCC[y];
    // The threshold is extremely low between the two peaks
    bool peakFound = false;
    for (y = 1; y < 255; y++) {
        if (histogramCC[y - 1] < histogramCC[y] && histogramCC[y + 1] < histogramCC[y])
            peakFound = true;
        if (peakFound == true && histogramCC[y - 1] >= histogramCC[y] && histogramCC[y + 1] >= histogramCC[y])
            return y - 1;
    }
    return -1;
}

int GetIntermodesThreshold(int histogram[], int* histogramSmooth[]) {
    int y, iter = 0, index;
    int histogramC[256] = { 0 };
    int histogramCC[256] = { 0 }; // The process of averaging will destroy the original data, so we copy it
    for (y = 0; y < 256; y++) {
        histogramC[y] = histogram[y];
        histogramCC[y] = histogram[y];
    }

    // Smooth the histogram by averaging three points
    while (isBimodal(histogramCC) == false) {                                 // determine if it is already a bimodal image
        histogramCC[0] = (histogramC[0] + histogramC[0] + histogramC[1]) / 3; // The first point
        for (y = 1; y < 255; y++)
            histogramCC[y] = (histogramC[y - 1] + histogramC[y] + histogramC[y + 1]) / 3; // The middle point
        histogramC[255] = (histogramC[254] + histogramC[255] + histogramC[255]) / 3;      // The last point
        memcpy(histogramC, histogramCC, sizeof(float) * 256);
        iter++;
        if (iter >= 10000)
            return -1; // histogram cannot smooth bimodal, return error code;
    }
    for (y = 0; y < 256; y++)
        histogramSmooth[y] = &histogramCC[y];
    // The threshold is the average of two peaks
    int peak[2];
    for (y = 1, index = 0; y < 255; y++) {
        if (histogramCC[y - 1] < histogramCC[y] && histogramCC[y + 1] < histogramCC[y])
            peak[index++] = y - 1;
    }
    return (peak[0] + peak[1] / 2);
}

int GetPTileThreshold(int histogram[], int Tile) {
    Tile = 50;
    int y, amount = 0, sum = 0;
    for (y = 0; y < 256; y++)
        amount += histogram[y]; // total number of pixels
    for (y = 0; y < 256; y++) {
        sum += histogram[y];
        if (sum >= amount * Tile / 100)
            return y;
    }
    return -1;
}

int GetIterativeBestThreshold(int histogram[]) {
    int x, iter = 0;
    int meanValueOne, meanValueTwo, sumOne, sumTwo, sumIntegralOne, sumIntegralTwo;
    int minValue, maxValue;
    int threshold, newThreshold;

    for (minValue = 0; minValue < 256 && histogram[minValue] == 0; minValue++)
        ;
    for (maxValue = 255; maxValue > minValue && histogram[minValue] == 0; maxValue--)
        ;

    if (maxValue == minValue)
        return maxValue; // There is only one color in the image
    if (minValue + 1 == maxValue)
        return minValue; // There are only two colors in the image

    threshold = minValue;
    newThreshold = (maxValue + minValue) >> 1;
    while (threshold != newThreshold) { // When the last two iterations have the same threshold, the iteration ends.
        sumOne = 0;
        sumIntegralOne = 0;
        sumTwo = 0;
        sumIntegralTwo = 0;
        threshold = newThreshold;
        // According to the threshold, the image is divided into two parts: target and background,
        // and the average gray value of the two parts is obtained.
        for (x = minValue; x <= threshold; x++) {
            sumIntegralOne += histogram[x] * x;
            sumOne += histogram[x];
        }
        meanValueOne = sumIntegralOne / sumOne;
        for (x = threshold + 1; x <= maxValue; x++) {
            sumIntegralTwo += histogram[x] * x;
            sumTwo += histogram[x];
        }
        meanValueTwo = sumIntegralTwo / sumTwo;
        newThreshold = (meanValueOne + meanValueTwo) >> 1; // Find new threshold
        iter++;
        if (iter >= 1000)
            return -1;
    }
    return threshold;
}

int GetOSTUThreshold(int histogram[]) {
    int x, y, amount = 0;
    int pixelBack = 0, pixelFore = 0, pixelIntegralBack = 0, pixelIntegralFore = 0, pixelIntegral = 0;
    float omegaBack, omegaFore, microBack, microFore, sigmaB, sigma; // variance between classes
    int minValue, maxValue;
    int threshold = 0;

    for (minValue = 0; minValue < 256 && histogram[minValue] == 0; minValue++)
        ;
    for (maxValue = 255; maxValue > minValue && histogram[minValue] == 0; maxValue--)
        ;

    if (maxValue == minValue)
        return maxValue; // There is only one color in the image
    if (minValue + 1 == maxValue)
        return minValue; // There are only two colors in the image

    for (y = minValue; y <= maxValue; y++)
        amount += histogram[y]; // total number of pixels

    pixelIntegral = 0;
    for (y = minValue; y <= maxValue; y++)
        pixelIntegral += histogram[y] * y;
    sigmaB = -1;
    for (y = minValue; y < maxValue; y++) {
        pixelBack = pixelBack + histogram[y];
        pixelFore = amount - pixelBack;
        omegaBack = (float)pixelBack / amount;
        omegaFore = (float)pixelFore / amount;
        pixelIntegralBack += histogram[y] * y;
        pixelIntegralFore = pixelIntegral - pixelIntegralBack;
        microBack = (float)pixelIntegralBack / pixelBack;
        microFore = (float)pixelIntegralFore / pixelFore;
        sigma = omegaBack * omegaFore * (microBack - microFore) * (microBack - microFore);
        if (sigma > sigmaB) {
            sigmaB = sigma;
            threshold = y;
        }
    }
    return threshold;
}

int Get1DMaxEntropyThreshold(int histogram[]) {
    int x, y, amount = 0;
    float histogramD[256] = { 0 };
    float sumIntegral, entropyBack, entropyFore, maxEntropy;
    int minValue = 255, maxValue = 0;
    int threshold = 0;

    for (minValue = 0; minValue < 256 && histogram[minValue] == 0; minValue++)
        ;
    for (maxValue = 255; maxValue > minValue && histogram[minValue] == 0; maxValue--)
        ;

    if (maxValue == minValue)
        return maxValue; // There is only one color in the image
    if (minValue + 1 == maxValue)
        return minValue; // There are only two colors in the image

    for (y = minValue; y <= maxValue; y++)
        amount += histogram[y]; // total number of pixels

    for (y = minValue; y <= maxValue; y++)
        histogramD[y] = (float)histogram[y] / amount + 1e-17;

    maxEntropy = FLT_MIN;
    for (y = minValue + 1; y < maxValue; y++) {
        sumIntegral = 0;
        for (x = minValue; x <= y; x++)
            sumIntegral += histogramD[x];
        entropyBack = 0;
        for (x = minValue; x <= y; x++)
            entropyBack += (-histogramD[x] / sumIntegral * log(histogramD[x] / sumIntegral));
        entropyFore = 0;
        for (x = y + 1; x <= maxValue; x++)
            entropyFore += (-histogramD[x] / (1 - sumIntegral) * log(histogramD[x] / (1 - sumIntegral)));
        if (maxEntropy < entropyBack + entropyFore) {
            threshold = y;
            maxEntropy = entropyBack + entropyFore;
        }
    }
    return threshold;
}

int GetMomentPreservingThreshold(int histogram[]) {
    int y, index = 0, amount = 0;
    float avec[256] = { 0 };
    float x2, x1, x0, Min;

    for (y = 0; y <= 256; y++)
        amount += histogram[y]; // total number of pixels
    for (y = 0; y < 256; y++)
        avec[y] = (float)A(histogram, y) / amount; // The threshold is chosen such that A(y,t)/A(y,n) is closest to x0.

    // The following finds x0.
    x2 = (double)(B(histogram, 255) * C(histogram, 255) - A(histogram, 255) * D(histogram, 255)) /
            (double)(A(histogram, 255) * C(histogram, 255) - B(histogram, 255) * B(histogram, 255));
    x1 = (double)(B(histogram, 255) * D(histogram, 255) - C(histogram, 255) * C(histogram, 255)) /
            (double)(A(histogram, 255) * C(histogram, 255) - B(histogram, 255) * B(histogram, 255));
    x0 = 0.5 - (B(histogram, 255) / A(histogram, 255) + x2 / 2) / sqrt(x2 * x2 - 4 * x1);

    for (y = 0, Min = FLT_MAX; y < 256; y++) {
        if (fabs(avec[y] - x0) < Min) {
            Min = fabs(avec[y] - x0);
            index = y;
        }
    }
    return index;
}

int GetKittlerMinError(int histogram[]) {
    int x, y;
    int minValue, maxValue;
    int threshold;
    int pixelBack, pixelFore;
    float omegaBack, omegaFore, minSigma, sigma, sigmaBack, sigmaFore;
    for (minValue = 0; minValue < 256 && histogram[minValue] == 0; minValue++)
        ;
    for (maxValue = 255; maxValue > minValue && histogram[minValue] == 0; maxValue--)
        ;
    if (maxValue == minValue)
        return maxValue; // There is only one color in the image
    if (minValue + 1 == maxValue)
        return minValue; // There are only two colors in the image
    threshold = -1;
    minSigma = 1E+20;
    for (y = minValue; y < maxValue; y++) {
        pixelBack = 0, pixelFore = 0, omegaBack = 0, omegaFore = 0;
        for (x = minValue; x <= y; x++) {
            pixelBack += histogram[x];
            omegaBack = omegaBack + x * histogram[x];
        }
        for (x = y + 1; x <= maxValue; x++) {
            pixelFore += histogram[x];
            omegaFore = omegaFore + x * histogram[x];
        }
        omegaBack = omegaBack / pixelBack;
        omegaFore = omegaFore / pixelFore;
        sigmaBack = 0, sigmaFore = 0;
        for (x = minValue; x <= y; x++)
            sigmaBack = sigmaBack + (x - omegaBack) * (x - omegaBack) * histogram[x];
        for (x = y + 1; x <= maxValue; x++)
            sigmaFore = sigmaFore + (x - omegaFore) * (x - omegaFore) * histogram[x];
        if (sigmaBack == 0 || sigmaFore == 0) {
            if (threshold == -1)
                threshold = y;
        } else {
            sigmaBack = sqrt(sigmaBack / pixelBack);
            sigmaFore = sqrt(sigmaFore / pixelFore);
            sigma = 1 + 2 * (pixelBack * log(sigmaBack / pixelBack) + pixelFore * log(sigmaFore / pixelFore));
            if (sigma < minSigma) {
                minSigma = sigma;
                threshold = y;
            }
        }
    }
    return threshold;
}

int GetIsoDataThreshold(int histogram[]) {
    int i, l, toth, totl, h, g = 0;
    for (i = 1; i < 256; i++) {
        if (histogram[i] > 0) {
            g = i + 1;
            break;
        }
    }
    while (true) {
        l = 0;
        totl = 0;
        for (i = 0; i < g; i++) {
            totl = totl + histogram[i];
            l = l + (histogram[i] * i);
        }
        h = 0;
        toth = 0;
        for (i = g + 1; i < 256; i++) {
            toth += histogram[i];
            h += (histogram[i] * i);
        }
        if (totl > 0 && toth > 0) {
            l /= totl;
            h /= toth;
            if (g == (int)round((l + h) / 2.0))
                break;
        }
        g++;
        if (g > 256 - 2) {
            return 0;
        }
    }
    return g;
}

int GetShanbhagThreshold(int histogram[]) {
    int threshold = 0;
    int ih, it;
    int first_bin;
    int last_bin;
    float term;
    float tot_ent;                 // total entropy
    float min_ent;                 // max entropy
    float ent_back;                // entropy of the background pixels at a given threshold
    float ent_obj;                 // entropy of the object pixels at a given threshold
    float norm_histo[256] = { 0 }; // normalized histogram
    float p1[256] = { 0 };         // cumulative normalized histogram
    float p2[256] = { 0 };

    int total = 0;
    for (ih = 0; ih < 256; ih++)
        total += histogram[ih];

    for (ih = 0; ih < 256; ih++)
        norm_histo[ih] = (float)histogram[ih] / total;

    p1[0] = norm_histo[0];
    p2[0] = 1.0 - p1[0];
    for (ih = 1; ih < 256; ih++) {
        p1[ih] = p1[ih - 1] + norm_histo[ih];
        p2[ih] = 1.0 - p1[ih];
    }

    // Determine the first non-zero bin
    first_bin = 0;
    for (ih = 0; ih < 256; ih++) {
        if (!(abs(p1[ih]) < FLT_EPSILON)) {
            first_bin = ih;
            break;
        }
    }

    // Determine the last non-zero bin
    last_bin = 255;
    for (ih = 255; ih >= first_bin; ih--) {
        if (!(fabs(p2[ih]) < FLT_EPSILON)) {
            last_bin = ih;
            break;
        }
    }

    // Calculate the total entropy of each gray-level and find the
    // threshold that maximizes it.
    threshold = -1;
    min_ent = FLT_MAX;

    for (it = first_bin; it <= last_bin; it++) {
        // Entropy of the background pixels
        ent_back = 0.0;
        term = 0.5 / p1[it];
        for (ih = 1; ih <= it; ih++) {
            ent_back -= norm_histo[ih] * log(1.0 - term * p1[ih - 1]);
        }
        ent_back *= term;

        // Entropy of the object pixels
        ent_obj = 0.0;
        term = 0.5 / p2[it];
        for (ih = it + 1; ih < 256; ih++) {
            ent_obj -= norm_histo[ih] * log(1.0 - term * p2[ih]);
        }
        ent_obj *= term;

        // Total entropy
        tot_ent = fabs(ent_back - ent_obj);

        if (tot_ent < min_ent) {
            min_ent = tot_ent;
            threshold = it;
        }
    }
    return threshold;
}

int GetYenThreshold(int histogram[]) {
    int threshold;
    int ih, it;
    float crit;
    float max_crit;
    float norm_histo[256] = { 0 }; // normalized histogram
    float p1[256] = { 0 };         // cumulative normalized histogram
    float p1_sq[256] = { 0 };
    float p2_sq[256] = { 0 };

    int total = 0;
    for (ih = 0; ih < 256; ih++)
        total += histogram[ih];


    for (ih = 0; ih < 256; ih++)
        norm_histo[ih] = (float)histogram[ih] / total;

    p1[0] = norm_histo[0];
    for (ih = 1; ih < 256; ih++)
        p1[ih] = p1[ih - 1] + norm_histo[ih];

    p1_sq[0] = norm_histo[0] * norm_histo[0];
    for (ih = 1; ih < 256; ih++)
        p1_sq[ih] = p1_sq[ih - 1] + norm_histo[ih] * norm_histo[ih];

    p2_sq[256 - 1] = 0.0;
    for (ih = 254; ih >= 0; ih--)
        p2_sq[ih] = p2_sq[ih + 1] + norm_histo[ih + 1] * norm_histo[ih + 1];

    // Find the threshold that maximizes the criterion
    threshold = -1;
    max_crit = FLT_MIN;
    for (it = 0; it < 256; it++) {
        crit = -1.0 * ((p1_sq[it] * p2_sq[it]) > 0.0 ? log(p1_sq[it] * p2_sq[it]) : 0.0) +
                2 * ((p1[it] * (1.0 - p1[it])) > 0.0 ? log(p1[it] * (1.0 - p1[it])) : 0.0);
        if (crit > max_crit) {
            max_crit = crit;
            threshold = it;
        }
    }
    return threshold;
}

float A(int histogram[], int index) {
    float sum = 0;
    for (int y = 0; y <= index; y++) {
        sum += histogram[y];
    }
    return sum;
}

float B(int histogram[], int index) {
    float sum = 0;
    for (int y = 0; y <= index; y++) {
        sum += (float)y * histogram[y];
    }
    return sum;
}

float C(int histogram[], int index) {
    float sum = 0;
    for (int y = 0; y <= index; y++) {
        sum += (float)y * y * histogram[y];
    }
    return sum;
}

float D(int histogram[], int index) {
    float sum = 0;
    for (int y = 0; y <= index; y++) {
        sum += (float)y * y * y * histogram[y];
    }
    return sum;
}

bool isBimodal(int histogram[]) {
    // Count the peaks of the histogram, only peak number 2 is double-peaked
    int count = 0;
    for (int y = 1; y < 255; y++) {
        if (histogram[y - 1] < histogram[y] && histogram[y + 1] < histogram[y]) {
            count++;
            if (count > 2)
                return false;
        }
    }
    if (count == 2)
        return true;
    else
        return false;
}
