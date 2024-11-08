#include <math.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum {
    CORRELATION,
    CHI_SQUARE,
    BHATTACHARYYA
} ComparisonMethod;

// calculate histogram of an image
void calculateHistogram(const uint8_t* image, int width, int height, int channels, int* histogram) {
    for (int i = 0; i < 256; i++) {
        histogram[i] = 0;
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = (y * width + x) * channels;
            int gray = (image[index] + image[index + 1] + image[index + 2]) / 3; // Convert to grayscale
            histogram[gray]++;
        }
    }
}

// normalize histogram
void normalizeHistogram(int* histogram, double* normalizedHistogram) {
    int total = 0;
    for (int i = 0; i < 256; i++) {
        total += histogram[i];
    }
    for (int i = 0; i < 256; i++) {
        normalizedHistogram[i] = (double)histogram[i] / total;
    }
}

// Function to calculate correlation
double correlation(const double* hist1, const double* hist2) {
    double sum1 = 0, sum2 = 0, sum1Sq = 0, sum2Sq = 0, pSum = 0;
    for (int i = 0; i < 256; i++) {
        sum1 += hist1[i];
        sum2 += hist2[i];
        sum1Sq += hist1[i] * hist1[i];
        sum2Sq += hist2[i] * hist2[i];
        pSum += hist1[i] * hist2[i];
    }
    double num = pSum - (sum1 * sum2 / 256);
    double den = sqrt((sum1Sq - (sum1 * sum1 / 256)) * (sum2Sq - (sum2 * sum2 / 256));
    return (den == 0) ? 0 : num / den;
}

// Function to calculate chi-square
double chiSquare(const double* hist1, const double* hist2) {
    double chiSquareValue = 0;
    for (int i = 0; i < 256; i++) {
        if (hist1[i] + hist2[i] > 0) {
            chiSquareValue += ((hist1[i] - hist2[i]) * (hist1[i] - hist2[i])) / (hist1[i] + hist2[i]);
        }
    }
    return chiSquareValue;
}

// Function to calculate Bhattacharyya distance
double bhattacharyya(const double* hist1, const double* hist2) {
    double sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += sqrt(hist1[i] * hist2[i]);
    }
    return -log(sum); // Return negative log of the sum
}

// Calculates the difference between two images based on their histograms.
// Several techniques are available, including correlation, chi-square, and Bhattacharyya distance.
// Effective in comparing images with slight lighting variations, but not robust to spatial changes in the image.
double compareHistograms(const uint8_t* img1, const uint8_t* img2, int width, int height, int channels, ComparisonMethod method) {
    int hist1[256] = {0};
    int hist2[256] = {0};
    double normHist1[256] = {0};
    double normHist2[256] = {0};

    // Calculate histograms
    calculateHistogram(img1, width, height, channels, hist1);
    calculateHistogram(img2, width, height, channels, hist2);

    // Normalize histograms
    normalizeHistogram(hist1, normHist1);
    normalizeHistogram(hist2, normHist2);

    // Calculate and return score based on the selected method
    switch (method) {
        case CORRELATION:
            return correlation(normHist1, normHist2);
        case CHI_SQUARE:
            return chiSquare(normHist1, normHist2);
        case BHATTACHARYYA:
            return bhattacharyya(normHist1, normHist2);
        default:
            printf("Invalid comparison method.\n");
            return -1; // Return an error value
    }
}

// Mean Squared Error (MSE)
// Measures the average squared difference between pixels in two images.
// Effective in comparing images with significant lighting variations.
double calculateMSE(const uint8_t* img1, const uint8_t* img2, int width, int height, int channels) {
    double mse = 0.0;
    int totalPixels = width * height * channels;

    for (int i = 0; i < totalPixels; i++) {
        int diff = img1[i] - img2[i];
        mse += diff * diff;
    }

    return mse / totalPixels; // Return the average MSE
}

// Peak Signal-to-Noise Ratio (PSNR)
// Measures the peak error between two images.
// Effective in comparing images with significant lighting variations.  
double calculatePSNR(const uint8_t* img1, const uint8_t* img2, int width, int height, int channels) {
    double mse = 0.0;
    int totalPixels = width * height;

    for (int i = 0; i < totalPixels * channels; i++) {
        int diff = img1[i] - img2[i];
        mse += diff * diff;
    }

    mse /= totalPixels * channels;

    if (mse == 0) {
        return INFINITY; // PSNR is infinite if images are identical
    }

    double maxPixelValue = 255.0;
    return 10.0 * log10((maxPixelValue * maxPixelValue) / mse);
}

// Structural Similarity Index (SSIM)
// Measures the similarity between two images based on luminance, contrast, and structure.
// Effective in comparing images with significant lighting variations and spatial changes.
double calculateSSIM(const uint8_t* img1, const uint8_t* img2, int width, int height, int channels) {
    double ssim = 0.0;
    const double C1 = 6.5025, C2 = 58.5225;

    for (int c = 0; c < channels; c++) {
        double mean1 = 0, mean2 = 0, var1 = 0, var2 = 0, covar = 0;

        // Calculate means
        for (int i = 0; i < width * height; i++) {
            mean1 += img1[i * channels + c];
            mean2 += img2[i * channels + c];
        }
        mean1 /= (width * height);
        mean2 /= (width * height);

        // Calculate variances and covariance
        for (int i = 0; i < width * height; i++) {
            double diff1 = img1[i * channels + c] - mean1;
            double diff2 = img2[i * channels + c] - mean2;
            var1 += diff1 * diff1;
            var2 += diff2 * diff2;
            covar += diff1 * diff2;
        }
        var1 /= (width * height - 1);
        var2 /= (width * height - 1);
        covar /= (width * height - 1);

        // Calculate SSIM
        double numerator = (2 * mean1 * mean2 + C1) * (2 * covar + C2);
        double denominator = (mean1 * mean1 + mean2 * mean2 + C1) * (var1 + var2 + C2);
        ssim += numerator / denominator;
    }

    return ssim / channels;
}

