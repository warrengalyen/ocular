/**
 * @file: threshold.h
 * @author Warren Galyen
 * Created: 2-19-2024
 * Last Updated: 2-19-2024
 * Last update: initial implementation
 *
 * @brief Ocular histogram threshold methods
 * Many of these algorithms were adapted from ImageJ: http://fiji.sc/wiki/index.php/Auto_Threshold
 */

#ifndef OCULAR_THRESHOLD_H
#define OCULAR_THRESHOLD_H

#include <stdbool.h>

/**
 * @brief Threshold based on gray average
 * @param histogram Histogram of grayscale image
 * @return Calculated threshold value
 */
int GetMeanThreshold(int histogram[]);

/**
 * @brief Huang-type threshold algorithm based on fuzzy set
 * http://www.ktl.elf.stuba.sk/study/vacso/Zadania-Cvicenia/Cvicenie_3/TimA2/Huang_E016529624.pdf
 * @param histogram Histogram of grayscale image
 * @return Calculated threshold value, -1 if none found.
 */
int GetHuangFuzzyThreshold(int histogram[]);

/**
 * @brief Threshold based on the valley bottom
 * This method is applied to an image with a clear bimodal histogram, which looks for a double peak valley as a threshold.
 * References:
 * J. M. S. Prewitt and M. L. Mendelsohn, "The analysis of cell images," in
 * annals of the New York Academy of Sciences, vol. 128, pp. 1035-1053, 1966.
 * C. A. Glasbey, "An analysis of histogram-based thresholding algorithms,"
 * CVGIP: Graphical Models and Image Processing, vol. 55, pp. 532-537, 1993.
 * @param histogram Histogram of grayscale image
 * @param[out] histogramSmooth Return smoothed histogram
 * @return Calculated threshold value, -1 if none found.
 */
int GetMinimumThreshold(int histogram[], int* histogramSmooth[]);

/**
 * @brief Threshold based on bimodal averages
 * This method is applied to an image with a clear bimodal histogram, which looks for a double peak valley as a threshold.
 * References:
 * J. M. S. Prewitt and M. L. Mendelsohn, "The analysis of cell images," in
 * annals of the New York Academy of Sciences, vol. 128, pp. 1035-1053, 1966.
 * C. A. Glasbey, "An analysis of histogram-based thresholding algorithms,"
 * CVGIP: Graphical Models and Image Processing, vol. 55, pp. 532-537, 1993.
 * @param histogram Histogram of grayscale image
 * @param[out] histogramSmooth Return smoothed histogram
 * @return Calculated threshold value, -1 if none found.
 */
int GetIntermodesThreshold(int histogram[], int* histogramSmooth[]);

/**
 * @brief Percentage based threshold
 * @param histogram Histogram of grayscale image
 * @param Tile The percentage of the background area in an image
 * @return Calculated threshold value, -1 if none found.
 */
int GetPTileThreshold(int histogram[], int Tile);

/**
 * @brief Iterative method to obtain the threshold
 * @param histogram Histogram of grayscale image
 * @return Calculated threshold value, -1 if none found.
 */
int GetIterativeBestThreshold(int histogram[]);

/**
 * @brief Otsu's threshold algorithm
 * Reference:
 * M. Emre Celebi 6.15.2007, Fourier Library https://sourceforge.net/projects/fourier-ipal/
 * @param histogram Histogram of grayscale image
 * @return Calculated threshold value
 */
int GetOSTUThreshold(int histogram[]);

/**
 * @brief Max Entropy thresholding method
 * Reference:
 * Kapur J.N., Sahoo P.K., and Wong A.K.C. (1985) "A New Method for
 * Gray-Level Picture Thresholding Using the Entropy of the Histogram"
 * Graphical Models and Image Processing, 29(3): 273-285
 * M. Emre Celebi 06.15.2007
 * @param histogram Histogram of grayscale image
 * @return Calculated threshold value
 */
int Get1DMaxEntropyThreshold(int histogram[]);

/**
 * @brief Moment preserving threshold methods
 * References:
 * http://fiji.sc/wiki/index.php/Auto_Threshold#Huang
 * W. Tsai, "Moment-preserving thresholding: a new approach," Computer
 * Vision, Graphics, and Image Processing, vol. 29, pp. 377-393, 1985.
 *
 * C. A. Glasbey, "An analysis of histogram-based thresholding algorithms,"
 * CVGIP: Graphical Models and Image Processing, vol. 55, pp. 532-537, 1993.
 * @param histogram Histogram of grayscale image
 * @return Calculated threshold value
 */
int GetMomentPreservingThreshold(int histogram[]);

/**
 * @brief Minimum Error thresholding method
 * References:
 * Kittler and J. Illingworth, "Minimum error thresholding," Pattern Recognition, vol. 19, pp. 41-47, 1986.
 * C. A. Glasbey, "An analysis of histogram-based thresholding algorithms," CVGIP: Graphical Models and Image
 * Processing, vol. 55, pp. 532-537, 1993.
 * @param histogram Histogram of grayscale image
 * @return Calculated threshold value
 */
int GetKittlerMinError(int histogram[]);

/**
 * @brief Iterative procedure based on the isodata algorithm.
 * Reference:
 * Picture Thresholding Using an Iterative Selection Method. (1978). IEEE Transactions on Systems, Man, and Cybernetics,
 * 8(8), 630–632. doi:10.1109/tsmc.1978.4310039
 * @param histogram Histogram of grayscale image
 * @return Calculated threshold value
 */
int GetIsoDataThreshold(int histogram[]);

/**
 * @brief Implements Shanbhag’s thresholding method
 * Reference:
 * Shanbhag, A. G. (1994). Utilization of Information Measure as a Means of Image Thresholding. CVGIP: Graphical
 * Models and Image Processing, 56(5), 414–419. doi:10.1006/cgip.1994.1037
 * @param histogram Histogram of grayscale image
 * @return Calculated threshold value
 */
int GetShanbhagThreshold(int histogram[]);

/**
 * @brief Implements Yen’s thresholding method
 * Reference:
 * Jui-Cheng Yen, Fu-Juay Chang, & Shyang Chang. (1995). A new criterion for automatic multilevel thresholding.
 * IEEE Transactions on Image Processing, 4(3), 370–378. doi:10.1109/83.366472
 * @param histogram Histogram of grayscale image
 * @return Calculated threshold value
 */
int GetYenThreshold(int histogram[]);

// used for Moment Preserving method
float A(int histogram[], int index);

// used for Moment Preserving method
float B(int histogram[], int index);

// used for Moment Preserving method
float C(int histogram[], int index);

// used for Moment Preserving method
float D(int histogram[], int index);

// Check whether the histogram is bimodal
bool isBimodal(int histogram[]);


#endif  /* OCULAR_THRESHOLD_H */
