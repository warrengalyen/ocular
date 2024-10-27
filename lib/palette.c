#include "palette.h"
#include "color.h"

void read_gimp_palette(const char* filename, OcPalette* palette) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open palette file");
        return;
    }

    char line[256];
    palette->num_colors = 0; // Initialize the number of colors

    // Skip header lines
    while (fgets(line, sizeof(line), file)) {
        // Skip header, comments, columns lines
        if (line[0] == '#' || strncmp(line, "GIMP", 4) == 0 || strncmp(line, "Columns", 7) == 0) {
            continue;
        }
        if (strncmp(line, "Name", 4) == 0) {
            sscanf(line, "Name: %[^\n]", palette->name); // read the rest of the line, including spaces
            continue;
        }

        // Read color data
        int r, g, b;
        char color_name[256];
        if (sscanf(line, "%d %d %d %s", &r, &g, &b, color_name) == 4) {

            OcPaletteColor* color = &palette->colors[palette->num_colors];
            strncpy(color->name, color_name, 255);
            color->name[255] = '\0'; // Ensure null-termination
            color->r = r;
            color->g = g;
            color->b = b;
            palette->num_colors++;
        }
    }

    fclose(file);
};

void save_gimp_palette(const char* filename, const OcPalette* palette) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Failed to open file for writing");
        return;
    }

    // Write the GIMP palette header
    fprintf(file, "GIMP Palette\n");
    fprintf(file, "Name: %s\n", palette->name[0] == '\0' ? "Saved Colors" : palette->name);
    fprintf(file, "Columns: 16\n"); // Default column count
    fprintf(file, "#\n");

    // Write each color in the palette
    for (int i = 0; i < palette->num_colors; i++) {
        const OcPaletteColor* color = &palette->colors[i];
        fprintf(file, "%3d %3d %3d %s\n", color->r, color->g, color->b, color->name[0] == '\0' ? "Unknown" : color->name);
    }

    fclose(file);
}

void read_riff_palette(const char* filename, OcPalette* palette) {
    FILE* file = fopen(filename, "rb");
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

    palette->num_colors = num_entries;
    strncpy(palette->name, "RIFF Palette", 255);
    palette->name[255] = '\0';

    // Read color entries
    unsigned char r, g, b, flags;
    for (int i = 0; i < num_entries; i++) {

        fread(&r, 1, 1, file);
        fread(&g, 1, 1, file);
        fread(&b, 1, 1, file);
        fread(&flags, 1, 1, file);

        OcPaletteColor* color = &palette->colors[i];
        color->r = r;
        color->g = g;
        color->b = b;
        color->name[0] = '\0';
    }

    fclose(file);
}

void save_riff_palette(const char* filename, const OcPalette* palette) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        perror("Failed to open file for writing");
        return;
    }

    // 2 bytes for version
    // 2 bytes for color count
    // 4 * color count the color
    int chunk_body_size = 2 + 2 + 4 * palette->num_colors;
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
    unsigned short num_entries = palette->num_colors;
    fwrite(&num_entries, 2, 1, file);

    // Write color entries
    for (int i = 0; i < palette->num_colors; i++) {
        const OcPaletteColor* color = &palette->colors[i];
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

void read_swatches(FILE* file, OcPalette* palette, unsigned short version) {

    // Read color count
    unsigned short count;
    fread(&count, sizeof(unsigned short), 1, file);
    count = BIG_ENDIAN_16(count);
    palette->num_colors = count;

    for (int i = 0; i < count && palette->num_colors < MAX_PALETTE_COLORS; i++) {
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
        OcPaletteColor* color = &palette->colors[i];

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
                read_utf16_string(file, color_name, name_length);
                strncpy(color->name, color_name, name_length);
            }
        }
    }
}

void read_aco_palette(const char* filename, OcPalette* palette) {
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

    read_swatches(file, palette, version);
    strcpy(palette->name, "Adobe Swatch");
    if (version == 1) {
        // check for version 2
        fread(&version, sizeof(unsigned short), 1, file);
        version = BIG_ENDIAN_16(version); // Convert from big-endian
        if (version == 2) {
            read_swatches(file, palette, version);
        }
    }

    fclose(file);
}

// Only supports RGB color space
void save_aco_palette(const char* filename, const OcPalette* palette) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        perror("Failed to open file for writing");
        return;
    }

    // Write version 1 header and color count
    unsigned short version = BIG_ENDIAN_16(1);
    unsigned short count = BIG_ENDIAN_16(palette->num_colors);
    fwrite(&version, sizeof(unsigned short), 1, file);
    fwrite(&count, sizeof(unsigned short), 1, file);

    // Write version 1 color entries (without names)
    for (int i = 0; i < palette->num_colors; i++) {
        const OcPaletteColor* color = &palette->colors[i];

        // Color space (0 = RGB)
        unsigned short colorSpace = BIG_ENDIAN_16(0);
        // Convert 8-bit RGB to 16-bit values
        unsigned short w = BIG_ENDIAN_16(color->r << 8);
        unsigned short x = BIG_ENDIAN_16(color->g << 8);
        unsigned short y = BIG_ENDIAN_16(color->b << 8);
        unsigned short z = 0; // Not used for RGB

        fwrite(&colorSpace, sizeof(unsigned short), 1, file);
        fwrite(&w, sizeof(unsigned short), 1, file);
        fwrite(&x, sizeof(unsigned short), 1, file);
        fwrite(&y, sizeof(unsigned short), 1, file);
        fwrite(&z, sizeof(unsigned short), 1, file);
    }

    // Write version 2 header and color count
    version = BIG_ENDIAN_16(2);
    count = BIG_ENDIAN_16(palette->num_colors);
    fwrite(&version, sizeof(unsigned short), 1, file);
    fwrite(&count, sizeof(unsigned short), 1, file);

    // Write version 2 color entries (with names)
    for (int i = 0; i < palette->num_colors; i++) {
        const OcPaletteColor* color = &palette->colors[i];

        // Write color data (same as version 1)
        unsigned short colorSpace = BIG_ENDIAN_16(0);
        unsigned short w = BIG_ENDIAN_16(color->r << 8);
        unsigned short x = BIG_ENDIAN_16(color->g << 8);
        unsigned short y = BIG_ENDIAN_16(color->b << 8);
        unsigned short z = 0;

        fwrite(&colorSpace, sizeof(unsigned short), 1, file);
        fwrite(&w, sizeof(unsigned short), 1, file);
        fwrite(&x, sizeof(unsigned short), 1, file);
        fwrite(&y, sizeof(unsigned short), 1, file);
        fwrite(&z, sizeof(unsigned short), 1, file);

        // Write color name (UTF-16 format)
        size_t name_len = strlen(color->name);
        unsigned int str_len = BIG_ENDIAN_32(name_len + 1); // +1 for null terminator
        fwrite(&str_len, sizeof(unsigned int), 1, file);

        // Convert and write each character as UTF-16 Big Endian
        for (size_t j = 0; j < name_len; j++) {
            unsigned short unicode = BIG_ENDIAN_16((unsigned short)color->name[j]);
            fwrite(&unicode, sizeof(unsigned short), 1, file);
        }
        // Write null terminator
        unsigned short null_term = 0;
        fwrite(&null_term, sizeof(unsigned short), 1, file);
    }

    fclose(file);
}

void read_paintnet_palette(const char* filename, OcPalette* palette) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open Paint.NET palette file");
        return;
    }

    char line[256];
    palette->num_colors = 0;
    strncpy(palette->name, "Paint.NET Palette", 255);
    palette->name[255] = '\0';

    // Skip first line if it's the Paint.NET header
    if (fgets(line, sizeof(line), file)) {
        // convert header to lowercase
        for (int i = 0; i < sizeof(line); i++) {
            line[i] = tolower(line[i]);
        }
        if (strncmp(line, ";paint.net palette file", 22) != 0) {
            // If it's not a header, rewind to start of file
            rewind(file);
        }
    }

    while (fgets(line, sizeof(line), file) && palette->num_colors < MAX_PALETTE_COLORS) {
        // Skip comments and empty lines
        if (line[0] == ';' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }

        // Remove newline characters
        line[strcspn(line, "\r\n")] = 0;

        // Convert hex string to RGB values
        unsigned int hex_color;
        if (sscanf(line, "%X", &hex_color) == 1) {
            OcPaletteColor* color = &palette->colors[palette->num_colors];

            // Extract RGB components (Paint.NET uses ARGB format)
            color->r = (hex_color >> 16) & 0xFF;
            color->g = (hex_color >> 8) & 0xFF;
            color->b = hex_color & 0xFF;
            color->name[0] = '\0';

            palette->num_colors++;
        }
    }

    fclose(file);
}

void save_paintnet_palette(const char* filename, const OcPalette* palette) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Failed to open file for writing");
        return;
    }

    // Write Paint.NET palette header
    fprintf(file, ";paint.net Palette File\n");
    fprintf(file, ";Palette Name: Paint.net palette\n");
    fprintf(file, ";Colors: %d\n", palette->num_colors);

    // Write each color in ARGB format (fully opaque)
    for (int i = 0; i < palette->num_colors; i++) {
        const OcPaletteColor* color = &palette->colors[i];
        // FF prefix makes the color fully opaque (alpha = 255)
        fprintf(file, "FF%02X%02X%02X\n", color->r, color->g, color->b);
    }

    fclose(file);
}

void read_act_palette(const char* filename, OcPalette* palette) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open ACT file");
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // If file has color count and transparency index, read them
    if (file_size == 772) {
        unsigned short color_count;
        unsigned short transparency_index;
        fseek(file, 768, SEEK_SET);
        fread(&color_count, 2, 1, file);
        fread(&transparency_index, 2, 1, file);
        fseek(file, 0, SEEK_SET);
        color_count = BIG_ENDIAN_16(color_count);
        palette->num_colors = color_count;
    } else {
        palette->num_colors = file_size / 3;
    }

    // Initialize palette data
    strncpy(palette->name, "Adobe Color Table", 255);
    palette->name[255] = '\0';

    // Read RGB values (up to 256 colors)
    unsigned char rgb[3];
    for (int i = 0; i < palette->num_colors; i++) {
        fread(rgb, 1, 3, file);
        OcPaletteColor* color = &palette->colors[i];
        color->r = rgb[0];
        color->g = rgb[1];
        color->b = rgb[2];
        color->name[0] = '\0';
    }

    fclose(file);
}

void save_act_palette(const char* filename, const OcPalette* palette) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        perror("Failed to open file for writing");
        return;
    }

    // Write RGB values for up to 256 colors
    int colors_to_write = palette->num_colors > 256 ? 256 : palette->num_colors;

    // Pad with zeros up to 256 colors
    unsigned char zero_rgb[3] = { 0, 0, 0 };

    for (int i = 0; i < 256; i++) {
        if (i < colors_to_write) {
            const OcPaletteColor* color = &palette->colors[i];
            unsigned char rgb[3] = { color->r, color->g, color->b };
            fwrite(rgb, 1, 3, file);
        } else {
            fwrite(zero_rgb, 1, 3, file);
        }
    }

    // Write optional color count and transparency index
    unsigned short color_count = BIG_ENDIAN_16(colors_to_write);
    unsigned short transparency_index = 0; // No transparency by default
    fwrite(&color_count, 2, 1, file);
    fwrite(&transparency_index, 2, 1, file);

    fclose(file);
}

void read_ase_palette(const char* filename, OcPalette* palette) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open ASE file");
        return;
    }

    // Read header
    int signature;
    unsigned short version_major, version_minor;
    int block_count;

    fread(&signature, 4, 1, file);
    signature = BIG_ENDIAN_32(signature);

    if (signature != ASE_SIGNATURE) {
        fprintf(stderr, "Invalid ASE file signature\n");
        fclose(file);
        return;
    }

    fread(&version_major, 2, 1, file);
    fread(&version_minor, 2, 1, file);
    fread(&block_count, 4, 1, file);

    version_major = BIG_ENDIAN_16(version_major);
    version_minor = BIG_ENDIAN_16(version_minor);
    block_count = BIG_ENDIAN_32(block_count);

    // Initialize palette
    palette->num_colors = 0;
    strncpy(palette->name, "Adobe Swatch Exchange", 255);
    palette->name[255] = '\0';

    // Read blocks
    while (!feof(file) && palette->num_colors < MAX_PALETTE_COLORS) {
        unsigned short block_type;
        int block_length;

        if (fread(&block_type, 2, 1, file) != 1)
            break;
        if (fread(&block_length, 4, 1, file) != 1)
            break;

        block_type = BIG_ENDIAN_16(block_type);
        block_length = BIG_ENDIAN_32(block_length);

        if (block_type == ASE_BLOCK_COLOR) {
            unsigned short name_length;
            fread(&name_length, 2, 1, file);
            name_length = BIG_ENDIAN_16(name_length);

            // Read color name (UTF-16)
            char color_name[256] = { 0 };
            if (name_length > 0) {
                read_utf16_string(file, color_name, name_length);
            }

            // Read color model
            int color_model;
            fread(&color_model, 4, 1, file);
            color_model = BIG_ENDIAN_32(color_model);

            OcPaletteColor* color = &palette->colors[palette->num_colors];
            strncpy(color->name, color_name, 255);
            color->name[255] = '\0';

            // Read color values
            if (color_model == ASE_COLOR_RGB) {
                float r, g, b;
                fread(&r, 4, 1, file);
                fread(&g, 4, 1, file);
                fread(&b, 4, 1, file);

                // Convert from big-endian float and scale to 0-255
                union {
                    float f;
                    int i;
                } ur, ug, ub;
                ur.i = BIG_ENDIAN_32(*(int*)&r);
                ug.i = BIG_ENDIAN_32(*(int*)&g);
                ub.i = BIG_ENDIAN_32(*(int*)&b);

                color->r = (int)(ur.f * 255.0f);
                color->g = (int)(ug.f * 255.0f);
                color->b = (int)(ub.f * 255.0f);

                palette->num_colors++;
            } else if (color_model == ASE_COLOR_GRAY) {
                float gray;
                fread(&gray, 4, 1, file);

                union {
                    float f;
                    int i;
                } ug;
                ug.i = BIG_ENDIAN_32(*(int*)&gray);

                color->r = color->g = color->b = (int)(ug.f * 255.0f);
                palette->num_colors++;
            }
            // Skip color type (global or spot)
            fseek(file, 2, SEEK_CUR);
        } else if (block_type == ASE_BLOCK_GROUP_START) {
            // Skip group blocks
            fseek(file, block_length, SEEK_CUR);
        } else if (block_type == ASE_BLOCK_GROUP_END) {
            // Skip group end blocks
            fseek(file, block_length, SEEK_CUR);
        }
    }

    fclose(file);
}

PaletteFormat detect_palette_format(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open palette file");
        return FORMAT_UNKNOWN;
    }

    char header[32];
    size_t bytes_read = fread(header, 1, sizeof(header), file);
    fclose(file);

    if (bytes_read < 8) {
        return FORMAT_UNKNOWN;
    }

    // Check for RIFF PAL format
    if (memcmp(header, "RIFF", 4) == 0 && memcmp(header + 8, "PAL ", 4) == 0) {
        return FORMAT_RIFF;
    }

    // Check for ACO format (version 1 or 2)
    unsigned short version = (header[0] << 8) | header[1];
    if (version == 1 || version == 2) {
        return FORMAT_ACO;
    }

    // Check for GIMP palette (text format)
    if (strncmp(header, "GIMP", 4) == 0) {
        return FORMAT_GIMP;
    }

    // Check for Paint.NET palette
    // convert header to lowercase
    for (int i = 0; i < bytes_read; i++) {
        header[i] = tolower(header[i]);
    }
    if (bytes_read >= 22 && strncmp(header, ";paint.net palette file", 22) == 0) {
        return FORMAT_PAINTNET;
    }

    // Check for ACT format (exactly 768 or 772 bytes)
    FILE* act_file = fopen(filename, "rb");
    if (act_file) {
        fseek(act_file, 0, SEEK_END);
        long size = ftell(act_file);
        fclose(act_file);

        if (size == 768 || size == 772) {
            return FORMAT_ACT;
        }
    }

    // Check for ASE format
    FILE* ase_file = fopen(filename, "rb");
    if (ase_file) {
        int signature;
        if (fread(&signature, 4, 1, ase_file) == 1) {
            signature = BIG_ENDIAN_32(signature);
            if (signature == ASE_SIGNATURE) {
                fclose(ase_file);
                return FORMAT_ASE;
            }
        }
        fclose(ase_file);
    }

    // back to text file reading for GIMP palettes that might
    // not have the header at the start.
    file = fopen(filename, "r");
    if (file) {
        char line[256];
        if (fgets(line, sizeof(line), file)) {
            if (strstr(line, "GIMP") != NULL) {
                fclose(file);
                return FORMAT_GIMP;
            }
        }
        fclose(file);
    }

    return FORMAT_UNKNOWN;
}
