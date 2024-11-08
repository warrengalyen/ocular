#ifndef SEAM_CARVE_H
#define SEAM_CARVE_H

#include <math.h>
#include <stdlib.h>

typedef enum {
    VERTICAL,
    HORIZONTAL
} SeamDirection;

unsigned char* create_energy_image(unsigned char* image, int width, int height) {
    
    unsigned char* energy_image = (unsigned char*)malloc(width * height);
    if (!energy_image) {
        return NULL;
    }

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int idx = (y * width + x) * 3;
            int gx = 0;
            int gy = 0;
            for (int c = 0; c < 3; c++) {
                gx += abs(image[idx + c - 3] - image[idx + c + 3]) + abs(image[idx + c - width * 3] - image[idx + c + width * 3]);
                gy += abs(image[idx + c - width * 3 - 3] - image[idx + c + width * 3 + 3]) + abs(image[idx + c - width * 3 + 3] - image[idx + c + width * 3 - 3]);
            }
            energy_image[y * width + x] = (unsigned char)(sqrt(gx * gx + gy * gy) / 12);
        }
    }

    return energy_image;
}

double* create_cumulative_energy_map(unsigned char* energy_image, int width, int height, SeamDirection seam_direction) {

    double* cumulative_energy_map = (double*)malloc(width * height * sizeof(double));
    if (!cumulative_energy_map) {
        return NULL;
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            cumulative_energy_map[y * width + x] = (double)energy_image[y * width + x];
        }
    }

    if (seam_direction == VERTICAL) {
        for (int y = 1; y < height; y++) {
            for (int x = 0; x < width; x++) {
                double a = cumulative_energy_map[(y - 1) * width + ((x > 0) ? (x - 1) : x)];
                double b = cumulative_energy_map[(y - 1) * width + x];
                double c = cumulative_energy_map[(y - 1) * width + ((x < width - 1) ? (x + 1) : x)];
                cumulative_energy_map[y * width + x] += fmin(a, fmin(b, c));
            }
        }
    } else {
        for (int x = 1; x < width; x++) {
            for (int y = 0; y < height; y++) {
                double a = cumulative_energy_map[((y > 0) ? (y - 1) : y) * width + (x - 1)];
                double b = cumulative_energy_map[y * width + (x - 1)];
                double c = cumulative_energy_map[((y < height - 1) ? (y + 1) : y) * width + (x - 1)];
                cumulative_energy_map[y * width + x] += fmin(a, fmin(b, c));
            }
        }
    }

    return cumulative_energy_map;
}

int* find_optimal_seam(double* cumulative_energy_map, int width, int height, SeamDirection seam_direction) {

    int* path;
    if (seam_direction == VERTICAL) {
        path = (int*)malloc(height * sizeof(int));
        if (!path) {
            return NULL;
        }
        int min_index = 0;
        double min_val = cumulative_energy_map[(height - 1) * width];
        for (int x = 1; x < width; x++) {
            if (cumulative_energy_map[(height - 1) * width + x] < min_val) {
                min_val = cumulative_energy_map[(height - 1) * width + x];
                min_index = x;
            }
        }
        path[height - 1] = min_index;

        for (int y = height - 2; y >= 0; y--) {
            int x = path[y + 1];
            double a = cumulative_energy_map[y * width + ((x > 0) ? (x - 1) : x)];
            double b = cumulative_energy_map[y * width + x];
            double c = cumulative_energy_map[y * width + ((x < width - 1) ? (x + 1) : x)];

            if (b < a && b < c) {
                path[y] = x;
            } else if (a < c) {
                path[y] = x - 1;
            } else {
                path[y] = x + 1;
            }
        }
    } else {
        path = (int*)malloc(width * sizeof(int));
        if (!path) {
            return NULL;
        }
        int min_index = 0;
        double min_val = cumulative_energy_map[width - 1];
        for (int y = 1; y < height; y++) {
            if (cumulative_energy_map[y * width + width - 1] < min_val) {
                min_val = cumulative_energy_map[y * width + width - 1];
                min_index = y;
            }
        }
        path[width - 1] = min_index;

        for (int x = width - 2; x >= 0; x--) {
            int y = path[x + 1];
            double a = cumulative_energy_map[((y > 0) ? (y - 1) : y) * width + x];
            double b = cumulative_energy_map[y * width + x];
            double c = cumulative_energy_map[((y < height - 1) ? (y + 1) : y) * width + x];

            if (b < a && b < c) {
                path[x] = y;
            } else if (a < c) {
                path[x] = y - 1;
            } else {
                path[x] = y + 1;
            }
        }
    }

    return path;
}

unsigned char* reduce_image(unsigned char* image, int width, int height, int* optimal_seam, SeamDirection seam_direction) {

    int new_width = width - (seam_direction == VERTICAL);
    int new_height = height - (seam_direction == HORIZONTAL);
    unsigned char* new_image = (unsigned char*)malloc(new_width * new_height * 3);
    if (!new_image) {
        return NULL;
    }

    if (seam_direction == VERTICAL) {
        // Loop through each row and copy pixels except the seam pixel
        for (int y = 0; y < height; y++) {
            int new_x = 0;
            for (int x = 0; x < width; x++) {
                if (x == optimal_seam[y]) {
                    // Skip the pixel in the optimal seam
                    continue;
                }
                // Copy the pixel from the old image to the new one
                new_image[(y * new_width + new_x) * 3 + 0] = image[(y * width + x) * 3 + 0]; // R
                new_image[(y * new_width + new_x) * 3 + 1] = image[(y * width + x) * 3 + 1]; // G
                new_image[(y * new_width + new_x) * 3 + 2] = image[(y * width + x) * 3 + 2]; // B
                new_x++;
            }
        }
    } else {
        // HORIZONTAL seam removal
        // Loop through each column and copy pixels except the seam pixel
        for (int x = 0; x < width; x++) {
            int new_y = 0;
            for (int y = 0; y < height; y++) {
                if (y == optimal_seam[x]) {
                    // Skip the pixel in the optimal seam
                    continue;
                }
                // Copy the pixel from the old image to the new one
                new_image[(new_y * width + x) * 3 + 0] = image[(y * width + x) * 3 + 0]; // R
                new_image[(new_y * width + x) * 3 + 1] = image[(y * width + x) * 3 + 1]; // G
                new_image[(new_y * width + x) * 3 + 2] = image[(y * width + x) * 3 + 2]; // B
                new_y++;
            }
        }
    }

    return new_image;
}

ocularSeamCarve(unsigned char* input, unsigned char** output, int width, int height, SeamDirection seam_direction, int iterations, int* new_width, int* new_height) {

    for (int i = 0; i < iterations; i++) {
        unsigned char* energy_image = create_energy_image(input, width, height);
        double* cumulative_energy_map = create_cumulative_energy_map(energy_image, width, height, seam_direction);
        int* optimal_seam = find_optimal_seam(cumulative_energy_map, width, height, seam_direction);
        
        *output = reduce_image(input, width, height, optimal_seam, seam_direction);

        free(energy_image);
        free(cumulative_energy_map);
        free(optimal_seam);
    }
}

#endif  /* SEAM_CARVE_H */
