#include "fft.h"
#include <string.h>

// Helper function to get index in 1D array from 2D coordinates
static inline int get_index(int i, int j, int width) {
    return i * width + j;
}

// Perform 2D FFT on the input image
void fft_2d(complex *input, int width, int height) {
    complex *temp_row = (complex *)malloc(width * sizeof(complex));
    
    // Perform 1D FFT on each row
    for (int i = 0; i < height; i++) {
        // Copy row to temporary array
        for (int j = 0; j < width; j++) {
            temp_row[j] = input[get_index(i, j, width)];
        }
        fft_1d(temp_row, width);
        // Copy back
        for (int j = 0; j < width; j++) {
            input[get_index(i, j, width)] = temp_row[j];
        }
    }
    
    // Transpose and perform 1D FFT on columns
    complex *temp_col = (complex *)malloc(height * sizeof(complex));
    for (int j = 0; j < width; j++) {
        // Copy column to temporary array
        for (int i = 0; i < height; i++) {
            temp_col[i] = input[get_index(i, j, width)];
        }
        fft_1d(temp_col, height);
        // Copy back
        for (int i = 0; i < height; i++) {
            input[get_index(i, j, width)] = temp_col[i];
        }
    }
    
    free(temp_row);
    free(temp_col);
}

// Perform 1D FFT using Cooley-Tukey algorithm
void fft_1d(complex *input, int n) {
    if (!is_power_of_two(n)) {
        return; // Should handle error properly in production code
    }

    bit_reverse(input, n);

    // Cooley-Tukey FFT algorithm
    for (int step = 1; step < n; step *= 2) {
        double theta = -2.0 * M_PI / (2.0 * step);
        complex w = cexp(_Complex_I * theta);
        
        for (int group = 0; group < n; group += 2 * step) {
            complex w_k = 1;
            
            for (int k = 0; k < step; k++) {
                complex t = w_k * input[group + k + step];
                complex u = input[group + k];
                input[group + k] = u + t;
                input[group + k + step] = u - t;
                w_k *= w;
            }
        }
    }
}

// Perform inverse 2D FFT
void ifft_2d(complex *input, int width, int height) {
    // Take complex conjugate
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            input[get_index(i, j, width)] = conj(input[get_index(i, j, width)]);
        }
    }
    
    // Perform forward FFT
    fft_2d(input, width, height);
    
    // Take complex conjugate and scale
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            input[get_index(i, j, width)] = conj(input[get_index(i, j, width)]) / (width * height);
        }
    }
}

// Helper function to transpose complex matrix
void transpose_matrix(complex *matrix, int width, int height) {
    complex *temp = (complex *)malloc(width * height * sizeof(complex));
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            temp[get_index(j, i, height)] = matrix[get_index(i, j, width)];
        }
    }
    
    memcpy(matrix, temp, width * height * sizeof(complex));
    free(temp);
}

// Helper function to check if number is power of 2
int is_power_of_two(int n) {
    return n && !(n & (n - 1));
}

// Helper function to get next power of 2
int next_power_of_two(int n) {
    int power = 1;
    while (power < n) {
        power *= 2;
    }
    return power;
}

// Helper function to perform bit reversal
void bit_reverse(complex *input, int n) {
    int j = 0;
    for (int i = 0; i < n - 1; i++) {
        if (i < j) {
            complex temp = input[i];
            input[i] = input[j];
            input[j] = temp;
        }
        int k = n >> 1;
        while (k <= j) {
            j -= k;
            k >>= 1;
        }
        j += k;
    }
}

// Helper function to swap quadrants of the matrix
static void swap_quadrants(complex *input, int x1, int y1, int x2, int y2, int width, int height) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            complex temp = input[get_index(i + y1, j + x1, width)];
            input[get_index(i + y1, j + x1, width)] = input[get_index(i + y2, j + x2, width)];
            input[get_index(i + y2, j + x2, width)] = temp;
        }
    }
}

// Perform FFT shift (moves zero-frequency component to center)
void fft_shift_2d(complex *input, int width, int height) {
    int half_width = width / 2;
    int half_height = height / 2;
    
    // Swap quadrants 1 and 4
    swap_quadrants(input, 0, 0, half_width, half_height, half_width, half_height);
    
    // Swap quadrants 2 and 3
    swap_quadrants(input, half_width, 0, 0, half_height, half_width, half_height);
}

// IFFT shift is the same operation as FFT shift
void ifft_shift_2d(complex *input, int width, int height) {
    fft_shift_2d(input, width, height);
}

void fft_convolve_2d(unsigned char* input, float* kernel, unsigned char* output, int width, int height, int stride, int kernel_width, int kernel_height) {
    int channels = stride / width;
    if (!input || !kernel || !output)
        return;
    if (width <= 0 || height <= 0 || kernel_width <= 0 || kernel_height <= 0)
        return;

    // Get optimal FFT size (power of 2)
    int opt_width = 1, opt_height = 1;
    while (opt_width < width + kernel_width - 1)
        opt_width <<= 1;
    while (opt_height < height + kernel_height - 1)
        opt_height <<= 1;

    // Process each channel separately
    for (int ch = 0; ch < channels; ch++) {
        // Allocate padded arrays
        complex* input_padded = (complex*)malloc(opt_width * opt_height * sizeof(complex));
        complex* kernel_padded = (complex*)malloc(opt_width * opt_height * sizeof(complex));
        if (!input_padded || !kernel_padded) {
            free(input_padded);
            free(kernel_padded);
            return;
        }

        // Zero padding
        memset(input_padded, 0, opt_width * opt_height * sizeof(complex));
        memset(kernel_padded, 0, opt_width * opt_height * sizeof(complex));

        // Copy input image to padded array for current channel
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                input_padded[y * opt_width + x] = 
                    input[y * stride + x * channels + ch] + 0.0 * I_C;
            }
        }

        // Copy and flip kernel (convolution requires kernel to be flipped)
        int kCenterX = kernel_width / 2;
        int kCenterY = kernel_height / 2;
        for (int y = 0; y < kernel_height; y++) {
            for (int x = 0; x < kernel_width; x++) {
                int src_y = kernel_height - 1 - y; // Flip vertically
                int src_x = kernel_width - 1 - x;  // Flip horizontally
                kernel_padded[y * opt_width + x] = 
                    kernel[src_y * kernel_width + src_x] + 0.0 * I_C;
            }
        }

        // Perform FFT on both input and kernel
        fft_2d(input_padded, opt_width, opt_height);
        fft_2d(kernel_padded, opt_width, opt_height);

        // Multiply in frequency domain
        for (int i = 0; i < opt_height; i++) {
            for (int j = 0; j < opt_width; j++) {
                int idx = i * opt_width + j;
                input_padded[idx] *= kernel_padded[idx];
            }
        }

        // Perform inverse FFT
        ifft_2d(input_padded, opt_width, opt_height);

        // Calculate kernel sum for normalization
        float kernel_sum = 0.0f;
        for (int i = 0; i < kernel_width * kernel_height; i++) {
            kernel_sum += kernel[i];
        }
        if (kernel_sum == 0.0f)
            kernel_sum = 1.0f;

        // Copy result back to output for current channel
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                double real_val = creal(input_padded[y * opt_width + x]) / kernel_sum;
                output[y * stride + x * channels + ch] = 
                    (unsigned char)fmin(fmax(real_val, 0), 255);
            }
        }

        // Clean up
        free(input_padded);
        free(kernel_padded);
    }
}