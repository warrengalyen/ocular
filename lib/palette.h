#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_PALETTE_COLORS 2048

typedef struct {
    int r;
    int g;
    int b;
    char name[256];
} OcPaletteColor;

typedef struct {
    char name[256];
    int num_colors;
    OcPaletteColor colors[MAX_PALETTE_COLORS];
} OcPalette;

void read_gimp_palette(const char* filename, OcPalette* palette_data) {
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
 
            OcPaletteColor* color = &palette_data->colors[palette_data->num_colors];
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

void save_gimp_palette(const char* filename, const OcPalette* palette_data) {
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
        const OcPaletteColor* color = &palette_data->colors[i];
        fprintf(file, "%3d %3d %3d %s\n", color->r, color->g, color->b, color->name[0] == '\0' ? "Unknown" : color->name);
    }

    fclose(file);
}

void read_riff_palette(const char* filename, OcPalette* palette_data) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open RIFF palette file");
        return;
    }

    // Read RIFF header
    char riff_id[4];
    int file_size;
    char pad_id[4];

    fread(riff_id, 1, 4, file);
    fread(&file_size, 4, 1, file);
    fread(pad_id, 1, 4, file);

    // Verify RIFF and PAL identifiers
    if (memcmp(riff_id, "RIFF", 4) != 0 || memcmp(pad_id, "PAL ", 4) != 0) {
        fprintf(stderr, "Invalid RIFF palette format\n");
        fclose(file);
        return;
    }

    // Skip "data" chunk identifier and size
    fseek(file, 8, SEEK_CUR);

    // Read palette version and number of entries
    unsigned short version;
    unsigned short num_entries;
    fread(&version, 2, 1, file);
    fread(&num_entries, 2, 1, file);

    palette_data->num_colors = num_entries;
    strncpy(palette_data->name, "RIFF Palette", 255);
    palette_data->name[255] = '\0';

    // Read color entries
    unsigned char r, g, b, flags;
    for (int i = 0; i < num_entries; i++) {

        fread(&r, 1, 1, file);
        fread(&g, 1, 1, file);
        fread(&b, 1, 1, file);
        fread(&flags, 1, 1, file);

        OcPaletteColor* color = &palette_data->colors[i];
        color->r = r;
        color->g = g;
        color->b = b;
        color->name[0] = '\0';
    }

    fclose(file);
}

void save_riff_palette(const char* filename, const OcPalette* palette_data) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        perror("Failed to open file for writing");
        return;
    }

    // 2 bytes for version
    // 2 bytes for color count
    // 4 * color count the color
    int chunk_body_size = 2 + 2 + 4 * palette_data->num_colors;
    // 4 bytes for "data"
    // 4 bytes for chunk size
    int chunk_total_size = 4 + 4 + chunk_body_size;
    // 4 bytes for "RIFF"
    // 4 bytes for file size
    // 4 bytes for "PAL "
    // 4 bytes for data chunk
    int file_size = 4 + 4 + 4 + chunk_total_size;

    // Write RIFF header
    fwrite("RIFF", 1, 4, file);
    fwrite(&chunk_total_size, 4, 1, file);
    // Write form type
    fwrite("PAL ", 1, 4, file);

    // Write "data" chunk identifier and size
    fwrite("data", 1, 4, file);
    fwrite(&chunk_body_size, 4, 1, file);

    // Write palette version and number of entries
    unsigned short version = 0x0300; // Common version for RIFF PAL files
    fwrite(&version, 2, 1, file);
    unsigned short num_entries = palette_data->num_colors;
    fwrite(&num_entries, 2, 1, file);

    // Write color entries
    for (int i = 0; i < palette_data->num_colors; i++) {
        const OcPaletteColor* color = &palette_data->colors[i];
        unsigned char r = (unsigned char)color->r;
        unsigned char g = (unsigned char)color->g;
        unsigned char b = (unsigned char)color->b;
        unsigned char flags = 0; // Reserved, usually set to 0
        fwrite(&r, 1, 1, file);
        fwrite(&g, 1, 1, file);
        fwrite(&b, 1, 1, file);
        fwrite(&flags, 1, 1, file);
    }

    fclose(file);
}
void read_paintnet_palette(const char* filename, OcPalette* palette_data) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open Paint.NET palette file");
        return;
    }

    char line[256];
    palette_data->num_colors = 0;
    strncpy(palette_data->name, "Paint.NET Palette", 255);
    palette_data->name[255] = '\0';

    // Skip first line if it's the Paint.NET header
    if (fgets(line, sizeof(line), file)) {
        if (strncmp(line, ";paint.net Palette File", 22) != 0) {
            // If it's not a header, rewind to start of file
            rewind(file);
        }
    }

    while (fgets(line, sizeof(line), file) && palette_data->num_colors < MAX_PALETTE_COLORS) {
        // Skip comments and empty lines
        if (line[0] == ';' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }

        // Remove newline characters
        line[strcspn(line, "\r\n")] = 0;

        // Convert hex string to RGB values
        unsigned int hex_color;
        if (sscanf(line, "%X", &hex_color) == 1) {
            OcPaletteColor* color = &palette_data->colors[palette_data->num_colors];
            
            // Extract RGB components (Paint.NET uses ARGB format)
            color->r = (hex_color >> 16) & 0xFF;
            color->g = (hex_color >> 8) & 0xFF;
            color->b = hex_color & 0xFF;
            color->name[0] = '\0';
            
            palette_data->num_colors++;
        }
    }

    fclose(file);
}

void save_paintnet_palette(const char* filename, const OcPalette* palette_data) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Failed to open file for writing");
        return;
    }

    // Write Paint.NET palette header
    fprintf(file, ";paint.net Palette File\n");
    fprintf(file, ";Palette Name: Paint.net palette\n");
    fprintf(file, ";Colors: %d\n", palette_data->num_colors);
    
    // Write each color in ARGB format (fully opaque)
    for (int i = 0; i < palette_data->num_colors; i++) {
        const OcPaletteColor* color = &palette_data->colors[i];
        // FF prefix makes the color fully opaque (alpha = 255)
        fprintf(file, "FF%02X%02X%02X\n", color->r, color->g, color->b);
    }

    fclose(file);
}

