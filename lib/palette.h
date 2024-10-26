#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "color.h"

// Convert a 16-bit value from big-endian to little-endian
#define BIG_ENDIAN_16(x) (((x) >> 8) | ((x) << 8))

// Convert a 32-bit value from big-endian to little-endian
#define BIG_ENDIAN_32(x) ((((x) >> 24) & 0xFF) | (((x) >> 8) & 0xFF00) | (((x) << 8) & 0xFF0000) | (((x) << 24) & 0xFF000000))

#define MAX_PALETTE_COLORS 2048

typedef enum
{
    FORMAT_UNKNOWN,
    FORMAT_GIMP,
    FORMAT_RIFF,
    FORMAT_ACO,
    FORMAT_PAINTNET
} PaletteFormat;

// Adobe-specific structures
typedef struct {
    unsigned short colorSpace;
    unsigned short w; // Color components
    unsigned short x;
    unsigned short y;
    unsigned short z;
} AcoColorEntry;

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

void read_utf16_string(FILE* file, char* output, int length) {
    unsigned short wide_buffer[256]; // Temporary buffer for wide characters
    int chars_read;

    // Read characters until we hit a null terminator or reach length
    for (int i = 0; i < length && i < 256; i++) {
        if (fread(&wide_buffer[i], sizeof(unsigned short), 1, file) != 1) {
            break;
        }
        wide_buffer[i] = BIG_ENDIAN_16(wide_buffer[i]);
        if (wide_buffer[i] == 0) {
            chars_read = i;
            break;
        }
        chars_read = i + 1;
    }

    // Convert wide character string to multibyte (char array)
    wcstombs(output, wide_buffer, length);
}

void read_swatches(FILE* file, OcPalette* palette_data, unsigned short version) {

    // Read color count
    unsigned short count;
    fread(&count, sizeof(unsigned short), 1, file);
    count = BIG_ENDIAN_16(count);
    palette_data->num_colors = count;

    for (int i = 0; i < count && palette_data->num_colors < MAX_PALETTE_COLORS; i++) {
        AcoColorEntry entry;
        fread(&entry.colorSpace, sizeof(unsigned short), 1, file);
        fread(&entry.w, sizeof(unsigned short), 1, file);
        fread(&entry.x, sizeof(unsigned short), 1, file);
        fread(&entry.y, sizeof(unsigned short), 1, file);
        fread(&entry.z, sizeof(unsigned short), 1, file);

        // Convert from big-endian
        entry.colorSpace = BIG_ENDIAN_16(entry.colorSpace);
        entry.w = BIG_ENDIAN_16(entry.w);
        entry.x = BIG_ENDIAN_16(entry.x);
        entry.y = BIG_ENDIAN_16(entry.y);
        entry.z = BIG_ENDIAN_16(entry.z);

        // Convert to RGB and store
        OcPaletteColor* color = &palette_data->colors[i];

        switch (entry.colorSpace) {
            case 0: // RGB color space
                color->r = entry.w >> 8;
                color->g = entry.x >> 8;
                color->b = entry.y >> 8;
                color->name[0] = '\0';
                break;
            case 1: // HSB color space
                unsigned char h, s, v;
                unsigned char r, g, b;
                h = entry.w >> 8;
                s = entry.x >> 8;
                v = entry.y >> 8;

                hsv2rgb(h, s, v, &r, &g, &b);
                color->r = r;
                color->g = g;
                color->b = b;
                color->name[0] = '\0';
                break;
            case 8: // Grayscale
                unsigned char gray = entry.w >> 8;
                color->r = gray;
                color->g = gray;
                color->b = gray;
                color->name[0] = '\0';
                break;
            default:
                fprintf(stderr, "Unsupported ACO color space\n");
                break;
                // TODO: implement CMYK and LAB color space conversion
        }

        if (version == 2) {
            unsigned int name_length;
            fread(&name_length, sizeof(unsigned int), 1, file);
            name_length = BIG_ENDIAN_32(name_length);

            if (name_length > 0) {
                char color_name[256];
                read_utf16_string(file, color_name, name_length );
                strncpy(color->name, color_name, name_length);
            }
        }
    }
}

void read_aco_palette(const char* filename, OcPalette* palette_data) {
    FILE* file = fopen(filename, "rb"); // Open in binary mode
    if (!file) {
        perror("Failed to open ACO file");
        return;
    }

    unsigned short version;

    // Read version
    fread(&version, sizeof(unsigned short), 1, file);
    version = BIG_ENDIAN_16(version); // Convert from big-endian

    if (version != 1 && version != 2) {
        fprintf(stderr, "Invalid ACO version information\n");
        fclose(file);
        return;
    }

    read_swatches(file, palette_data, version);
    strcpy(palette_data->name, "Adobe Swatch");
    if (version == 1) {
        // check for version 2
        fread(&version, sizeof(unsigned short), 1, file);
        version = BIG_ENDIAN_16(version); // Convert from big-endian
        if (version == 2) {
            read_swatches(file, palette_data, version);
        }
    }

    fclose(file);
}

void save_aco_palette(const char* filename, const OcPalette* palette) {


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

