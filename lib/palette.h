#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COLORS 256

typedef struct {
    int r;
    int g;
    int b;
    char name[256];
} PaletteColor;

typedef struct {
    PaletteColor colors[MAX_COLORS];
    char name[256];
    int num_colors;
} PaletteData;

void read_gimp_palette(const char* filename, PaletteData* palette_data) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open palette file");
        return;
    }

    char line[256];
    palette_data->num_colors = 0; // Initialize the number of colors

    // Skip header lines
    while (fgets(line, sizeof(line), file)) {
        // Skip header, comments, columns lines
        if (line[0] == '#' || strncmp(line, "GIMP", 4) == 0 || strncmp(line, "Columns", 7) == 0) {
            continue;
        }
        if (strncmp(line, "Name", 4) == 0) {
            sscanf(line, "Name: %[^\n]", palette_data->name); // read the rest of the line, including spaces
            continue;
        }

        // Read color data
        int r, g, b;
        char color_name[256];
        if (sscanf(line, "%d %d %d %s", &r, &g, &b, color_name) == 4) {
            if (palette_data->num_colors >= MAX_COLORS) {
                break;
            }
            PaletteColor* color = &palette_data->colors[palette_data->num_colors];
            strncpy(color->name, color_name, 255);
            color->name[255] = '\0'; // Ensure null-termination
            color->r = r;
            color->g = g;
            color->b = b;
            palette_data->num_colors++;
        }
    }

    fclose(file);
};

void save_gimp_palette(const char* filename, const PaletteData* palette_data) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Failed to open file for writing");
        return;
    }

    // Write the GIMP palette header
    fprintf(file, "GIMP Palette\n");
    fprintf(file, "Name: %s\n", palette_data->name[0] == '\0' ? "Saved Colors" : palette_data->name);
    fprintf(file, "Columns: 16\n"); // Default column count
    fprintf(file, "#\n");

    // Write each color in the palette
    for (int i = 0; i < palette_data->num_colors; i++) {
        const PaletteColor* color = &palette_data->colors[i];
        fprintf(file, "%3d %3d %3d %s\n", color->r, color->g, color->b, color->name[0] == '\0' ? "Unknown" : color->name);
    }

    fclose(file);
}



