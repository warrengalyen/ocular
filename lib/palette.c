#include "palette.h"
#include "color.h"
#include "util.h"
#include "miniz.h"
#include <stdint.h>

static float swap_float_endian(float value) {
    float result;
    unsigned char* src = (unsigned char*)&value;  // Treat float as a byte array
    unsigned char* dst = (unsigned char*)&result; // Destination for swapped float

    // Swap the bytes manually
    dst[0] = src[3];
    dst[1] = src[2];
    dst[2] = src[1];
    dst[3] = src[0];

    return result;
}

void ocularFreePalette(OcPalette* palette) {
    if (palette) {
        free(palette->colors);
        palette->colors = NULL;
        palette->num_colors = 0;
        palette->capacity = 0;
        palette->name[0] = '\0';
    }
}

bool resize_palette(OcPalette* palette) {
    if (palette->num_colors >= palette->capacity) {
        palette->capacity *= 2;
        OcPaletteColor* colors = realloc(palette->colors, palette->capacity * sizeof(OcPaletteColor));
        if (!colors) {
            perror("Failed to reallocate memory for palette");
            return false;
        }
        palette->colors = colors;
    }
    return true;
}

OC_STATUS read_gimp_palette(const char* filename, OcPalette* palette) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        return OC_STATUS_ERR_FILENOTFOUND;
    }

    char line[256];
    palette->num_colors = 0;
    palette->capacity = DEFAULT_PALETTE_CAPACITY;
    palette->colors = malloc(palette->capacity * sizeof(OcPaletteColor));

    if (!palette->colors) {
        fclose(file);
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

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

        // Ensure we have enough capacity for new color
        if (!resize_palette(palette)) {
            break;
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
    return OC_STATUS_OK;
};

OC_STATUS save_gimp_palette(const char* filename, const OcPalette* palette) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        return OC_STATUS_ERR_FILENOTFOUND;
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
    return OC_STATUS_OK;
}

OC_STATUS read_riff_palette(const char* filename, OcPalette* palette) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return OC_STATUS_ERR_FILENOTFOUND;
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
        fclose(file);
        return OC_STATUS_ERR_NOTSUPPORTED;
    }

    // Skip "data" chunk identifier and size
    fseek(file, 8, SEEK_CUR);

    // Read palette version and number of entries
    unsigned short version;
    unsigned short num_entries;
    fread(&version, 2, 1, file);
    fread(&num_entries, 2, 1, file);

    palette->num_colors = num_entries;
    palette->capacity = DEFAULT_PALETTE_CAPACITY;
    palette->colors = malloc(palette->capacity * sizeof(OcPaletteColor));
    strncpy(palette->name, "RIFF Palette", 255);
    palette->name[255] = '\0';

    // Read color entries
    unsigned char r, g, b, flags;
    for (int i = 0; i < num_entries; i++) {

        // Ensure we have enough capacity for new color
        if (!resize_palette(palette)) {
            break;
        }

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
    return OC_STATUS_OK;
}

OC_STATUS save_riff_palette(const char* filename, const OcPalette* palette) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        return OC_STATUS_ERR_FILENOTFOUND;
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
    return OC_STATUS_OK;
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

int read_swatches(FILE* file, OcPalette* palette, unsigned short version) {

    // Read color count
    unsigned short count;
    fread(&count, sizeof(unsigned short), 1, file);
    count = BIG_ENDIAN_16(count);
    palette->num_colors = count;

    palette->capacity = DEFAULT_PALETTE_CAPACITY;
    palette->colors = malloc(palette->capacity * sizeof(OcPaletteColor));

    unsigned char r, g, b;
    for (int i = 0; i < count; i++) {

        if (!resize_palette(palette)) {
            break;
        }

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
            h = entry.w >> 8;
            s = entry.x >> 8;
            v = entry.y >> 8;

            hsv2rgb(h, s, v, &r, &g, &b);
            color->r = r;
            color->g = g;
            color->b = b;
            color->name[0] = '\0';
            break;
        case 2: // CMYK color space
            float c, m, y, k;
            c = entry.w >> 8;
            m = entry.x >> 8;
            y = entry.y >> 8;
            k = entry.z >> 8;

            cmyk2rgb(c, m, y, k, &r, &g, &b);
            color->r = r;
            color->g = g;
            color->b = b;
            color->name[0] = '\0';
            break;
        case 7: // LAB color space
            double L, A, B;
            entry.w = entry.w >> 8;
            entry.x = entry.x >> 8;
            entry.y = entry.y >> 8;

            rgb2lab(entry.w, entry.x, entry.y, &L, &A, &B);
            color->r = (unsigned char)L;
            color->g = (unsigned char)A;
            color->b = (unsigned char)B;
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
            // Unsupported color space
            return -1;
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

    return 0;
}

OC_STATUS read_aco_palette(const char* filename, OcPalette* palette) {
    FILE* file = fopen(filename, "rb"); // Open in binary mode
    if (!file) {
        return OC_STATUS_ERR_FILENOTFOUND;
    }

    unsigned short version;

    // Read version
    fread(&version, sizeof(unsigned short), 1, file);
    version = BIG_ENDIAN_16(version); // Convert from big-endian

    if (version != 1 && version != 2) {
        // Invalid ACO version information
        fclose(file);
        return OC_STATUS_ERR_NOTSUPPORTED;
    }

    if (read_swatches(file, palette, version) != 0) {
        fclose(file);
        return OC_STATUS_ERR_NOTSUPPORTED;
    }
    strcpy(palette->name, "Adobe Swatch");
    if (version == 1) {
        // check for version 2
        fread(&version, sizeof(unsigned short), 1, file);
        version = BIG_ENDIAN_16(version); // Convert from big-endian
        if (version == 2) {
            if (read_swatches(file, palette, version) != 0) {
                fclose(file);
                return OC_STATUS_ERR_NOTSUPPORTED;
            }
        }
    }

    fclose(file);
    return OC_STATUS_OK;
}

// Only supports RGB color space
OC_STATUS save_aco_palette(const char* filename, const OcPalette* palette) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        return OC_STATUS_ERR_FILENOTFOUND;
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
    return OC_STATUS_OK;
}

OC_STATUS read_paintnet_palette(const char* filename, OcPalette* palette) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        return OC_STATUS_ERR_FILENOTFOUND;
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

    palette->capacity = DEFAULT_PALETTE_CAPACITY;
    palette->colors = malloc(palette->capacity * sizeof(OcPaletteColor));

    while (fgets(line, sizeof(line), file)) {

        // Skip comments and empty lines
        if (line[0] == ';' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }

        // Remove newline characters
        line[strcspn(line, "\r\n")] = 0;

        // Convert hex string to RGB values
        unsigned int hex_color;
        if (sscanf(line, "%X", &hex_color) == 1) {

            if (!resize_palette(palette)) {
                break;
            }

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
    return OC_STATUS_OK;
}

OC_STATUS save_paintnet_palette(const char* filename, const OcPalette* palette) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        return OC_STATUS_ERR_FILENOTFOUND;
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
    return OC_STATUS_OK;
}

OC_STATUS read_act_palette(const char* filename, OcPalette* palette) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return OC_STATUS_ERR_FILENOTFOUND;
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

    palette->capacity = DEFAULT_PALETTE_CAPACITY;
    palette->colors = malloc(palette->capacity * sizeof(OcPaletteColor));
    if (palette->colors == NULL) {
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

    // Read RGB values (up to 256 colors)
    unsigned char rgb[3];
    for (int i = 0; i < palette->num_colors; i++) {

        if (!resize_palette(palette)) {
            break;
        }

        fread(rgb, 1, 3, file);
        OcPaletteColor* color = &palette->colors[i];
        color->r = rgb[0];
        color->g = rgb[1];
        color->b = rgb[2];
        color->name[0] = '\0';
    }

    fclose(file);
    return OC_STATUS_OK;
}

OC_STATUS save_act_palette(const char* filename, const OcPalette* palette) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        return OC_STATUS_ERR_FILENOTFOUND;
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
    return OC_STATUS_OK;
}

OC_STATUS read_ase_palette(const char* filename, OcPalette* palette) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return OC_STATUS_ERR_FILENOTFOUND;
    }

    // Read header
    int signature;
    unsigned short version_major, version_minor;
    int block_count;

    if (fread(&signature, 4, 1, file) != 1) {
        fclose(file);
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }
    signature = BIG_ENDIAN_32(signature);

    if (signature != ASE_SIGNATURE) {
        fclose(file);
        return OC_STATUS_ERR_NOTSUPPORTED;
    }

    if (fread(&version_major, 2, 1, file) != 1 ||
        fread(&version_minor, 2, 1, file) != 1 ||
        fread(&block_count, 4, 1, file) != 1) {
        fclose(file);
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    version_major = BIG_ENDIAN_16(version_major);
    version_minor = BIG_ENDIAN_16(version_minor);
    block_count = BIG_ENDIAN_32(block_count);

    if (block_count < 0 || block_count > 1000000) {
        fclose(file);
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    // Initialize palette
    palette->num_colors = 0;
    strncpy(palette->name, "Adobe Swatch Exchange", 255);
    palette->name[255] = '\0';

    palette->capacity = DEFAULT_PALETTE_CAPACITY;
    palette->colors = malloc(palette->capacity * sizeof(OcPaletteColor));
    if (!palette->colors) {
        fclose(file);
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

    // Read blocks
    while (!feof(file) && !ferror(file)) {
        unsigned short block_type;
        int block_length;

        if (fread(&block_type, 2, 1, file) != 1)
            break;
        if (fread(&block_length, 4, 1, file) != 1)
            break;

        block_type = BIG_ENDIAN_16(block_type);
        block_length = BIG_ENDIAN_32(block_length);

        // Validate block_length
        if (block_length < 0 || block_length > 10000000) {
            free(palette->colors);
            palette->colors = NULL;
            fclose(file);
            return OC_STATUS_ERR_INVALIDPARAMETER;
        }

        if (block_type == ASE_BLOCK_COLOR) {
            unsigned short name_length;
            if (fread(&name_length, 2, 1, file) != 1) {
                free(palette->colors);
                palette->colors = NULL;
                fclose(file);
                return OC_STATUS_ERR_INVALIDPARAMETER;
            }
            name_length = BIG_ENDIAN_16(name_length);

            if (name_length < 0 || name_length > 512) { 
                free(palette->colors);
                palette->colors = NULL;
                fclose(file);
                return OC_STATUS_ERR_INVALIDPARAMETER;
            }

            // Ensure we have space for the color
            if (!resize_palette(palette)) {
                free(palette->colors);
                palette->colors = NULL;
                fclose(file);
                return OC_STATUS_ERR_OUTOFMEMORY;
            }

            OcPaletteColor* color = &palette->colors[palette->num_colors];

            // Read color name (UTF-16)
            char color_name[256] = { 0 };
            if (name_length > 0) {
                read_utf16_string(file, color_name, name_length);
            }

            // Read color model
            int color_model;
            if (fread(&color_model, 4, 1, file) != 1) {
                free(palette->colors);
                palette->colors = NULL;
                fclose(file);
                return OC_STATUS_ERR_INVALIDPARAMETER;
            }
            color_model = BIG_ENDIAN_32(color_model);

            strncpy(color->name, color_name, 255);
            color->name[255] = '\0';

            // Read color values
            if (color_model == ASE_COLOR_RGB) {
                float r, g, b;
                if (fread(&r, 4, 1, file) != 1 ||
                    fread(&g, 4, 1, file) != 1 ||
                    fread(&b, 4, 1, file) != 1) {
                    free(palette->colors);
                    palette->colors = NULL;
                    fclose(file);
                    return OC_STATUS_ERR_INVALIDPARAMETER;
                }

                r = swap_float_endian(r);
                g = swap_float_endian(g);
                b = swap_float_endian(b);

                color->r = ClampToByte((int)(r * 255.0f));
                color->g = ClampToByte((int)(g * 255.0f));
                color->b = ClampToByte((int)(b * 255.0f));

                palette->num_colors++;
            } else if (color_model == ASE_COLOR_GRAY) {
                float gray;
                if (fread(&gray, 4, 1, file) != 1) {
                    free(palette->colors);
                    palette->colors = NULL;
                    fclose(file);
                    return OC_STATUS_ERR_INVALIDPARAMETER;
                }

                union {
                    float f;
                    int i;
                } ug;
                ug.i = BIG_ENDIAN_32(*(int*)&gray);

                color->r = color->g = color->b = ClampToByte((int)(ug.f * 255.0f));
                palette->num_colors++;
            } else if (color_model == ASE_COLOR_CMYK) {
                float c, m, y, k;
                if (fread(&c, 4, 1, file) != 1 ||
                    fread(&m, 4, 1, file) != 1 ||
                    fread(&y, 4, 1, file) != 1 ||
                    fread(&k, 4, 1, file) != 1) {
                    free(palette->colors);
                    palette->colors = NULL;
                    fclose(file);
                    return OC_STATUS_ERR_INVALIDPARAMETER;
                }

                // use custom function to maintain precision
                c = swap_float_endian(c);
                m = swap_float_endian(m);
                y = swap_float_endian(y);
                k = swap_float_endian(k);

                unsigned char r, g, b;
                cmyk2rgb(c, m, y, k, &r, &g, &b);
                color->r = r;
                color->g = g;
                color->b = b;

                palette->num_colors++;
            } else if (color_model == ASE_COLOR_LAB) {
                float L, a, B;
                if (fread(&L, 4, 1, file) != 1 ||
                    fread(&a, 4, 1, file) != 1 ||
                    fread(&B, 4, 1, file) != 1) {
                    free(palette->colors);
                    palette->colors = NULL;
                    fclose(file);
                    return OC_STATUS_ERR_INVALIDPARAMETER;
                }

                L = swap_float_endian(L);
                a = swap_float_endian(a);
                B = swap_float_endian(B);

                // scale from float to percentage
                L *= 100.0f;

                unsigned char r, g, b;
                lab2rgb(L, a, B, &r, &g, &b);
                color->r = r;
                color->g = g;
                color->b = b;

                palette->num_colors++;
            } else {
                free(palette->colors);
                palette->colors = NULL;
                fclose(file);
                return OC_STATUS_ERR_NOTSUPPORTED;
            }

            // Skip color type (global or spot)
            if (fseek(file, 2, SEEK_CUR) != 0) {
                free(palette->colors);
                palette->colors = NULL;
                fclose(file);
                return OC_STATUS_ERR_INVALIDPARAMETER;
            }
        } else if (block_type == ASE_BLOCK_GROUP_START) {
            // Skip group blocks
            if (fseek(file, block_length, SEEK_CUR) != 0) {
                free(palette->colors);
                palette->colors = NULL;
                fclose(file);
                return OC_STATUS_ERR_INVALIDPARAMETER;
            }
        } else if (block_type == ASE_BLOCK_GROUP_END) {
            // Skip group end blocks
            if (fseek(file, block_length, SEEK_CUR) != 0) {
                free(palette->colors);
                palette->colors = NULL;
                fclose(file);
                return OC_STATUS_ERR_INVALIDPARAMETER;
            }
        }
    }

    // Check for file errors
    if (ferror(file)) {
        free(palette->colors);
        palette->colors = NULL;
        fclose(file);
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    fclose(file);
    return OC_STATUS_OK;
}

/* Affinity Designer .afpalette (binary chunk format) */

#define AF_MAGIC 0x414BFF00u
#define AF_HEADER_SIZE 80u
#define AF_CRC_START 76u /* headerSize - 4; CRC includes last 4 header bytes */
#define AF_END_MARKER_SIZE 5u
#define AF_FAT_FOOTER_SIZE 107u /* #FAT chunk after end marker */

typedef struct {
    const uint8_t* data;
    size_t size;
    size_t pos;
} AfReader;

typedef struct {
    uint8_t* data;
    size_t size;
    size_t capacity;
} AfBuffer;

static bool af_buffer_reserve(AfBuffer* buf, size_t extra) {
    size_t needed = buf->size + extra;
    if (needed <= buf->capacity) {
        return true;
    }
    size_t new_cap = buf->capacity ? buf->capacity * 2 : 256;
    while (new_cap < needed) {
        new_cap *= 2;
    }
    uint8_t* next = (uint8_t*)realloc(buf->data, new_cap);
    if (!next) {
        return false;
    }
    buf->data = next;
    buf->capacity = new_cap;
    return true;
}

static bool af_buffer_append(AfBuffer* buf, const void* bytes, size_t len) {
    if (!len) {
        return true;
    }
    if (!af_buffer_reserve(buf, len)) {
        return false;
    }
    memcpy(buf->data + buf->size, bytes, len);
    buf->size += len;
    return true;
}

static bool af_buffer_append_u8(AfBuffer* buf, uint8_t v) {
    return af_buffer_append(buf, &v, 1);
}

static bool af_buffer_append_u32(AfBuffer* buf, uint32_t v) {
    return af_buffer_append(buf, &v, sizeof(v));
}

static bool af_buffer_append_u64(AfBuffer* buf, uint64_t v) {
    return af_buffer_append(buf, &v, sizeof(v));
}

static bool af_buffer_append_f32(AfBuffer* buf, float v) {
    return af_buffer_append(buf, &v, sizeof(v));
}

static size_t af_find_pattern(const uint8_t* data, size_t size, size_t start, const char* pattern) {
    size_t pat_len = strlen(pattern);
    if (pat_len == 0 || start >= size) {
        return size;
    }
    for (size_t i = start; i + pat_len <= size; i++) {
        if (memcmp(data + i, pattern, pat_len) == 0) {
            return i;
        }
    }
    return size;
}

static bool af_reader_load(const char* filename, AfReader* reader, uint8_t** owned_out) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return false;
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return false;
    }
    long file_size = ftell(file);
    if (file_size < 0) {
        fclose(file);
        return false;
    }
    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return false;
    }
    uint8_t* data = (uint8_t*)malloc((size_t)file_size);
    if (!data) {
        fclose(file);
        return false;
    }
    size_t read_bytes = fread(data, 1, (size_t)file_size, file);
    fclose(file);
    if (read_bytes != (size_t)file_size) {
        free(data);
        return false;
    }
    reader->data = data;
    reader->size = (size_t)file_size;
    reader->pos = 0;
    *owned_out = data;
    return true;
}

static bool af_reader_read_u32(AfReader* r, uint32_t* out) {
    if (r->pos + sizeof(uint32_t) > r->size) {
        return false;
    }
    memcpy(out, r->data + r->pos, sizeof(uint32_t));
    r->pos += sizeof(uint32_t);
    return true;
}

static bool af_reader_read_u16(AfReader* r, uint16_t* out) {
    if (r->pos + sizeof(uint16_t) > r->size) {
        return false;
    }
    memcpy(out, r->data + r->pos, sizeof(uint16_t));
    r->pos += sizeof(uint16_t);
    return true;
}

static bool af_reader_read_f32(AfReader* r, float* out) {
    if (r->pos + sizeof(float) > r->size) {
        return false;
    }
    memcpy(out, r->data + r->pos, sizeof(float));
    r->pos += sizeof(float);
    return true;
}

static bool af_reader_read_ascii(AfReader* r, size_t len, char* out, size_t out_size) {
    if (r->pos + len > r->size || out_size == 0) {
        return false;
    }
    size_t copy = len < out_size - 1 ? len : out_size - 1;
    memcpy(out, r->data + r->pos, copy);
    out[copy] = '\0';
    r->pos += len;
    return true;
}

static bool af_reader_read_utf8_name(AfReader* r, char* out, size_t out_size) {
    uint32_t len = 0;
    if (!af_reader_read_u32(r, &len) || len == 0 || len >= out_size) {
        return false;
    }
    return af_reader_read_ascii(r, len, out, out_size);
}

static bool af_parse_abgr_at(AfReader* r, size_t dloc_pos, unsigned char* red, unsigned char* green,
                             unsigned char* blue) {
    r->pos = dloc_pos + 5;
    float rf = 0, gf = 0, bf = 0;
    if (!af_reader_read_f32(r, &rf) || !af_reader_read_f32(r, &gf) || !af_reader_read_f32(r, &bf)) {
        return false;
    }
    *red = ClampToByte((int)(rf * 255.0f + 0.5f));
    *green = ClampToByte((int)(gf * 255.0f + 0.5f));
    *blue = ClampToByte((int)(bf * 255.0f + 0.5f));
    return true;
}

static size_t af_find_b1_vlap(const uint8_t* data, size_t size, size_t start) {
    for (size_t i = start; i + 5 <= size; i++) {
        if (data[i] == 0xB1 && memcmp(data + i + 1, "VlaP", 4) == 0) {
            return i;
        }
    }
    return size;
}

static bool af_find_color_type(const uint8_t* data, size_t size, size_t start, size_t window, size_t* type_pos,
                               char type_out[5]) {
    static const char* const k_types[] = {"ABGR", "ABAL", "KYMC", "ALSH", "YARG", NULL};
    size_t end = start + window;
    if (end > size) {
        end = size;
    }
    for (size_t i = start; i + 4 <= end; i++) {
        for (int t = 0; k_types[t] != NULL; t++) {
            if (memcmp(data + i, k_types[t], 4) == 0) {
                memcpy(type_out, k_types[t], 4);
                type_out[4] = '\0';
                *type_pos = i;
                return true;
            }
        }
    }
    return false;
}

static bool af_parse_color_type(AfReader* r, size_t type_pos, const char* type, unsigned char* red,
                                unsigned char* green, unsigned char* blue) {
    if (memcmp(type, "ABGR", 4) == 0) {
        size_t dloc = af_find_pattern(r->data, r->size, type_pos, "Dloc_");
        if (dloc >= r->size) {
            return false;
        }
        return af_parse_abgr_at(r, dloc, red, green, blue);
    }

    if (memcmp(type, "ABAL", 4) == 0) {
        size_t loc = af_find_pattern(r->data, r->size, type_pos, "<loc_");
        if (loc >= r->size) {
            return false;
        }
        r->pos = loc + 5;
        uint16_t l = 0, a = 0, b = 0;
        if (!af_reader_read_u16(r, &l) || !af_reader_read_u16(r, &a) || !af_reader_read_u16(r, &b)) {
            return false;
        }
        double L = (double)l / 65535.0 * 100.0;
        double A = (double)a / 65535.0 * 256.0 - 128.0;
        double B = (double)b / 65535.0 * 256.0 - 128.0;
        lab2rgb(L, A, B, red, green, blue);
        return true;
    }

    if (memcmp(type, "KYMC", 4) == 0) {
        size_t loc = af_find_pattern(r->data, r->size, type_pos, "Hloc_");
        if (loc >= r->size) {
            return false;
        }
        r->pos = loc + 5;
        float c = 0, m = 0, y = 0, k = 0;
        if (!af_reader_read_f32(r, &c) || !af_reader_read_f32(r, &m) || !af_reader_read_f32(r, &y) ||
            !af_reader_read_f32(r, &k)) {
            return false;
        }
        cmyk2rgb(c, m, y, k, red, green, blue);
        return true;
    }

    if (memcmp(type, "ALSH", 4) == 0) {
        size_t dloc = af_find_pattern(r->data, r->size, type_pos, "Dloc_");
        if (dloc >= r->size) {
            return false;
        }
        r->pos = dloc + 5;
        float h = 0, s = 0, l = 0;
        if (!af_reader_read_f32(r, &h) || !af_reader_read_f32(r, &s) || !af_reader_read_f32(r, &l)) {
            return false;
        }
        float rf = 0, gf = 0, bf = 0;
        hsl2rgb(h, s, l, &rf, &gf, &bf);
        *red = ClampToByte((int)rf);
        *green = ClampToByte((int)gf);
        *blue = ClampToByte((int)bf);
        return true;
    }

    if (memcmp(type, "YARG", 4) == 0) {
        size_t loc = af_find_pattern(r->data, r->size, type_pos, "<loc_");
        if (loc >= r->size) {
            return false;
        }
        r->pos = loc + 5;
        float gray = 0;
        if (!af_reader_read_f32(r, &gray)) {
            return false;
        }
        unsigned char g = ClampToByte((int)(gray * 255.0f + 0.5f));
        *red = *green = *blue = g;
        return true;
    }

    return false;
}

OC_STATUS read_afpalette_palette(const char* filename, OcPalette* palette) {
    if (!filename || !palette) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }

    AfReader reader;
    uint8_t* owned = NULL;
    if (!af_reader_load(filename, &reader, &owned)) {
        return OC_STATUS_ERR_FILENOTFOUND;
    }

    OC_STATUS status = OC_STATUS_OK;
    if (reader.size < 8) {
        status = OC_STATUS_ERR_NOTSUPPORTED;
        goto cleanup;
    }

    uint32_t magic = 0;
    uint32_t version = 0;
    memcpy(&magic, reader.data, sizeof(magic));
    memcpy(&version, reader.data + 4, sizeof(version));
    if (magic != AF_MAGIC || version < 8 || version > 20) {
        status = OC_STATUS_ERR_NOTSUPPORTED;
        goto cleanup;
    }

    ocularFreePalette(palette);
    palette->num_colors = 0;
    palette->capacity = DEFAULT_PALETTE_CAPACITY;
    palette->colors = (OcPaletteColor*)malloc((size_t)palette->capacity * sizeof(OcPaletteColor));
    if (!palette->colors) {
        status = OC_STATUS_ERR_OUTOFMEMORY;
        goto cleanup;
    }
    palette->name[0] = '\0';

    size_t nclp = af_find_pattern(reader.data, reader.size, 0, "NClP");
    if (nclp < reader.size) {
        reader.pos = nclp + 4;
        uint32_t name_len = 0;
        if (af_reader_read_u32(&reader, &name_len) && name_len > 0 && name_len < sizeof(palette->name)) {
            af_reader_read_ascii(&reader, name_len, palette->name, sizeof(palette->name));
        }
    }
    if (palette->name[0] == '\0') {
        strncpy(palette->name, "Affinity Palette", sizeof(palette->name) - 1);
        palette->name[sizeof(palette->name) - 1] = '\0';
    }

    size_t b1_vlap = af_find_b1_vlap(reader.data, reader.size, 0);
    if (b1_vlap >= reader.size) {
        status = OC_STATUS_ERR_NOTSUPPORTED;
        goto cleanup;
    }

    reader.pos = b1_vlap + 5;
    uint32_t color_count = 0;
    if (!af_reader_read_u32(&reader, &color_count) || color_count == 0) {
        status = OC_STATUS_ERR_NOTSUPPORTED;
        goto cleanup;
    }

    size_t colors_end = af_find_pattern(reader.data, reader.size, reader.pos, "VNaP");
    if (colors_end >= reader.size) {
        colors_end = reader.size;
    }

    size_t search_from = reader.pos;
    uint32_t parsed = 0;
    while (parsed < color_count && search_from < colors_end) {
        size_t color_marker = af_find_pattern(reader.data, reader.size, search_from, "rloC");
        if (color_marker >= colors_end) {
            break;
        }

        char type[5];
        size_t type_pos = 0;
        if (!af_find_color_type(reader.data, reader.size, color_marker + 4, 48, &type_pos, type)) {
            search_from = color_marker + 4;
            continue;
        }

        unsigned char r = 0, g = 0, b = 0;
        if (!af_parse_color_type(&reader, type_pos, type, &r, &g, &b)) {
            search_from = color_marker + 4;
            continue;
        }

        if (!resize_palette(palette)) {
            status = OC_STATUS_ERR_OUTOFMEMORY;
            goto cleanup;
        }

        OcPaletteColor* entry = &palette->colors[palette->num_colors];
        entry->r = r;
        entry->g = g;
        entry->b = b;
        snprintf(entry->name, sizeof(entry->name), "Color %d", palette->num_colors + 1);
        palette->num_colors++;
        parsed++;
        search_from = color_marker + 4;
    }

    size_t vnap = af_find_pattern(reader.data, reader.size, 0, "VNaP");
    if (vnap < reader.size && palette->num_colors > 0) {
        reader.pos = vnap + 4;
        uint32_t skip = 0;
        uint32_t name_count = 0;
        if (af_reader_read_u32(&reader, &skip) && af_reader_read_u32(&reader, &name_count)) {
            uint32_t limit = name_count < (uint32_t)palette->num_colors ? name_count : (uint32_t)palette->num_colors;
            for (uint32_t i = 0; i < limit; i++) {
                char name_buf[256];
                if (!af_reader_read_utf8_name(&reader, name_buf, sizeof(name_buf))) {
                    break;
                }
                strncpy(palette->colors[i].name, name_buf, sizeof(palette->colors[i].name) - 1);
                palette->colors[i].name[sizeof(palette->colors[i].name) - 1] = '\0';
            }
        }
    }

    if (palette->num_colors == 0) {
        status = OC_STATUS_ERR_NOTSUPPORTED;
    }

cleanup:
    free(owned);
    if (status != OC_STATUS_OK) {
        ocularFreePalette(palette);
    }
    return status;
}

static void af_rgb_to_lab16(unsigned char r, unsigned char g, unsigned char b, uint16_t* out_l, uint16_t* out_a,
                            uint16_t* out_b) {
    double L = 0, a = 0, bv = 0;
    rgb2lab(r, g, b, &L, &a, &bv);
    *out_l = (uint16_t)(L / 100.0 * 65535.0 + 0.5);
    *out_a = (uint16_t)((a + 128.0) / 256.0 * 65535.0 + 0.5);
    *out_b = (uint16_t)((bv + 128.0) / 256.0 * 65535.0 + 0.5);
}

static bool af_write_lab_suffix(AfBuffer* buf, uint16_t l, uint16_t a, uint16_t b) {
    static const uint8_t loc_tag[] = {0x3C, 0x6C, 0x6F, 0x63, 0x5F};
    static const uint8_t suffix[] = {0xFF, 0xFF, 0x00, 0x00};
    if (!af_buffer_append(buf, loc_tag, sizeof(loc_tag)) || !af_buffer_append(buf, &l, sizeof(l)) ||
        !af_buffer_append(buf, &a, sizeof(a)) || !af_buffer_append(buf, &b, sizeof(b)) ||
        !af_buffer_append(buf, suffix, sizeof(suffix))) {
        return false;
    }
    return true;
}

static bool af_write_color_first_abal(AfBuffer* buf, unsigned char r, unsigned char g, unsigned char b) {
    /* Leading 01 00 00 00 00 00 is written by after_swatch_count before this block */
    static const uint8_t prefix[] = {0x53, 0x6C, 0x69, 0x46, 0x01, 0x00, 0x00, 0x00, 0x6C, 0x6C, 0x69, 0x46,
                                     0x00, 0x00, 0x00, 0x02, 0x31, 0x72, 0x6C, 0x6F, 0x43, 0x01, 0x01, 0x00,
                                     0x00, 0x00, 0x00, 0x41, 0x42, 0x41, 0x4C, 0x01, 0x00, 0x00, 0x02};
    uint16_t l = 0, a = 0, bv = 0;
    af_rgb_to_lab16(r, g, b, &l, &a, &bv);
    if (!af_buffer_append(buf, prefix, sizeof(prefix))) {
        return false;
    }
    return af_write_lab_suffix(buf, l, a, bv);
}

static bool af_write_swatch_lead(AfBuffer* buf, int swatch_index) {
    uint8_t lead[6] = {0x01, (uint8_t)(swatch_index * 2), 0x00, 0x00, 0x00, 0x01};
    return af_buffer_append(buf, lead, sizeof(lead));
}

static bool af_write_color_next_abal(AfBuffer* buf, int color_index, unsigned char r, unsigned char g,
                                     unsigned char b) {
    static const uint8_t mid[] = {0x53, 0x6C, 0x69, 0x46, 0x31, 0x72, 0x6C, 0x6F, 0x43};
    uint8_t id[6] = {0x01, (uint8_t)(color_index * 2 + 1), 0x00, 0x00, 0x00, 0x01};
    static const uint8_t type[] = {0x41, 0x42, 0x41, 0x4C};
    uint16_t l = 0, a = 0, bv = 0;
    af_rgb_to_lab16(r, g, b, &l, &a, &bv);

    if (!af_buffer_append(buf, mid, sizeof(mid)) || !af_buffer_append(buf, id, sizeof(id)) ||
        !af_buffer_append(buf, type, sizeof(type))) {
        return false;
    }
    return af_write_lab_suffix(buf, l, a, bv);
}

static void af_patch_u64(uint8_t* data, size_t offset, uint64_t value) {
    if (data && offset + sizeof(uint64_t) <= SIZE_MAX) {
        memcpy(data + offset, &value, sizeof(uint64_t));
    }
}

static void af_patch_u32(uint8_t* data, size_t offset, uint32_t value) {
    if (data && offset + sizeof(uint32_t) <= SIZE_MAX) {
        memcpy(data + offset, &value, sizeof(uint32_t));
    }
}

/* CRC-32 (polynomial 0xEDB88320) */
static uint32_t af_compute_crc32(const uint8_t* data, size_t offset, size_t length) {
    static uint32_t table[256];
    static bool table_init = false;
    if (!table_init) {
        for (uint32_t i = 0; i < 256; i++) {
            uint32_t c = i;
            for (int j = 0; j < 8; j++) {
                c = (c & 1u) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
            }
            table[i] = c;
        }
        table_init = true;
    }

    uint32_t crc = 0xFFFFFFFFu;
    size_t end = offset + length;
    for (size_t i = offset; i < end; i++) {
        crc = table[(crc ^ data[i]) & 0xFFu] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFFu;
}

OC_STATUS save_afpalette_palette(const char* filename, const OcPalette* palette) {
    if (!filename || !palette || palette->num_colors <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    /* Version 8 header/footer from Affinity-exported palettes */
    static const uint8_t af_header_template[AF_HEADER_SIZE] = {
        0x00, 0xFF, 0x4B, 0x41, 0x08, 0x00, 0x00, 0x00, 0x68, 0x74, 0x77, 0x53, 0x23, 0x49, 0x6E, 0x66,
        0xFF, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xEB, 0x4D, 0x4E, 0x54, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x50, 0x72, 0x6F, 0x74, 0x03, 0x00, 0x00, 0x00, 0x23, 0x46, 0x69, 0x6C, 0x00, 0xFF, 0x4B, 0x53};

    static const uint8_t af_end_marker[AF_END_MARKER_SIZE] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF};
    /* #FAT footer from Affinity-exported flatUI.afpalette; u64/CRC slots zeroed and patched after build */
    static const uint8_t af_fat_template[AF_FAT_FOOTER_SIZE] = {
        0x23, 0x46, 0x41, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEB, 0x4D, 0x4E, 0x54,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
        0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x53,
        0x77, 0x61, 0x74, 0x63, 0x68, 0x65, 0x73, 0x2E, 0x64, 0x61, 0x74};

    const char* palette_name =
        (palette->name[0] != '\0') ? palette->name : "ColorSnatch Palette";
    size_t name_len = strlen(palette_name);

    AfBuffer buf = {0};
    if (!af_buffer_append(&buf, af_header_template, AF_HEADER_SIZE)) {
        free(buf.data);
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

    static const uint8_t vlap_open[] = {0x01, 0x00, 0x56, 0x6C, 0x61, 0x50, 0x01, 0x00};
    static const uint8_t plcn_tag[] = {0x2B, 0x4E, 0x43, 0x6C, 0x50};
    static const uint8_t b1vlap[] = {0xB1, 0x56, 0x6C, 0x61, 0x50};
    /* After swatch count: 01 00 00 00 then 00 00 before first color block */
    static const uint8_t after_swatch_count[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00};

    if (!af_buffer_append(&buf, vlap_open, sizeof(vlap_open)) || !af_buffer_append(&buf, plcn_tag, sizeof(plcn_tag)) ||
        !af_buffer_append_u32(&buf, (uint32_t)name_len) || !af_buffer_append(&buf, palette_name, name_len) ||
        !af_buffer_append(&buf, b1vlap, sizeof(b1vlap)) ||
        !af_buffer_append_u32(&buf, (uint32_t)palette->num_colors) ||
        !af_buffer_append(&buf, after_swatch_count, sizeof(after_swatch_count))) {
        free(buf.data);
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

    for (int i = 0; i < palette->num_colors; i++) {
        const OcPaletteColor* color = &palette->colors[i];
        bool ok = (i == 0) ? af_write_color_first_abal(&buf, (unsigned char)color->r, (unsigned char)color->g,
                                                       (unsigned char)color->b)
                             : af_write_color_next_abal(&buf, i, (unsigned char)color->r, (unsigned char)color->g,
                                                        (unsigned char)color->b);
        if (!ok) {
            free(buf.data);
            return OC_STATUS_ERR_OUTOFMEMORY;
        }
        /* Affinity chains swatches: trailing lead after each color except the last. */
        if (i + 1 < palette->num_colors) {
            if (!af_write_swatch_lead(&buf, i + 1)) {
                free(buf.data);
                return OC_STATUS_ERR_OUTOFMEMORY;
            }
        }
    }

    static const uint8_t vnap_marker = 0xAB;
    static const uint8_t vnap_tag[] = {0x56, 0x4E, 0x61, 0x50};
    size_t names_block_size = 0;
    for (int i = 0; i < palette->num_colors; i++) {
        const char* nm = palette->colors[i].name[0] ? palette->colors[i].name : "Color";
        names_block_size += 4 + strlen(nm);
    }

    if (!af_buffer_append(&buf, &vnap_marker, 1) || !af_buffer_append(&buf, vnap_tag, sizeof(vnap_tag)) ||
        !af_buffer_append_u32(&buf, (uint32_t)names_block_size) ||
        !af_buffer_append_u32(&buf, (uint32_t)palette->num_colors)) {
        free(buf.data);
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

    for (int i = 0; i < palette->num_colors; i++) {
        const char* nm = palette->colors[i].name[0] ? palette->colors[i].name : "Color";
        uint32_t nm_len = (uint32_t)strlen(nm);
        if (!af_buffer_append_u32(&buf, nm_len) || !af_buffer_append(&buf, nm, nm_len)) {
            free(buf.data);
            return OC_STATUS_ERR_OUTOFMEMORY;
        }
    }

    if (!af_buffer_append(&buf, af_end_marker, sizeof(af_end_marker)) ||
        !af_buffer_append(&buf, af_fat_template, sizeof(af_fat_template))) {
        free(buf.data);
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

    uint64_t file_size = (uint64_t)buf.size;
    size_t fat_pos = buf.size - AF_FAT_FOOTER_SIZE;
    uint64_t fat_offset = (uint64_t)fat_pos;
    /* Bytes checksummed: last 4 header bytes + body through end marker (see flatUI.afpalette). */
    uint64_t crc_length = file_size - AF_HEADER_SIZE - AF_FAT_FOOTER_SIZE;

    af_patch_u64(buf.data, 0x10, fat_offset);
    af_patch_u64(buf.data, 0x18, file_size);
    af_patch_u64(buf.data, 0x20, crc_length);
    af_patch_u64(buf.data, fat_pos + 20, file_size);
    af_patch_u64(buf.data, fat_pos + 28, crc_length);
    af_patch_u64(buf.data, fat_pos + 72, crc_length);
    af_patch_u64(buf.data, fat_pos + 80, crc_length);

    uint32_t crc = af_compute_crc32(buf.data, AF_CRC_START, (size_t)crc_length);
    af_patch_u32(buf.data, fat_pos + 88, crc);

    FILE* out = fopen(filename, "wb");
    if (!out) {
        free(buf.data);
        return OC_STATUS_ERR_FILENOTFOUND;
    }
    size_t written = fwrite(buf.data, 1, buf.size, out);
    fclose(out);
    free(buf.data);

    return (written == buf.size) ? OC_STATUS_OK : OC_STATUS_ERR_UNKNOWN;
}

/* Procreate .swatches (ZIP + Swatches.json) */

#define SWATCHES_JSON_ENTRY "Swatches.json"

static const char* procreate_skip_json_ws(const char* p) {
    while (*p && isspace((unsigned char)*p)) {
        p++;
    }
    return p;
}

static const char* procreate_parse_json_string(const char* p, char* out, size_t out_size) {
    p = procreate_skip_json_ws(p);
    if (*p != '"') {
        return NULL;
    }
    p++;
    size_t i = 0;
    while (*p && *p != '"') {
        if (*p == '\\' && p[1]) {
            p++;
            if (i + 1 < out_size) {
                out[i++] = *p++;
            } else {
                p++;
            }
            continue;
        }
        if (i + 1 < out_size) {
            out[i++] = *p++;
        } else {
            p++;
        }
    }
    if (*p != '"') {
        return NULL;
    }
    out[i] = '\0';
    return p + 1;
}

static const char* procreate_parse_json_number(const char* p, double* value) {
    p = procreate_skip_json_ws(p);
    char* end = NULL;
    *value = strtod(p, &end);
    if (end == p) {
        return NULL;
    }
    return end;
}

static const char* procreate_find_json_key(const char* json, const char* key) {
    char pattern[128];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char* hit = strstr(json, pattern);
    if (!hit) {
        return NULL;
    }
    const char* p = hit + strlen(pattern);
    p = procreate_skip_json_ws(p);
    if (*p != ':') {
        return NULL;
    }
    return procreate_skip_json_ws(p + 1);
}

static void procreate_hsb_to_rgb(float hue, float saturation, float brightness, unsigned char* r,
                                 unsigned char* g, unsigned char* b) {
    float h_deg = hue * 360.0f;
    float s_pct = saturation * 100.0f;
    float v_pct = brightness * 100.0f;

    unsigned char h = (unsigned char)(h_deg / 2.0f + 0.5f);
    unsigned char s = (unsigned char)(s_pct * 255.0f / 100.0f + 0.5f);
    unsigned char v = (unsigned char)(v_pct * 255.0f / 100.0f + 0.5f);
    hsv2rgb(h, s, v, r, g, b);
}

static void procreate_rgb_to_hsb(unsigned char r, unsigned char g, unsigned char b, float* hue,
                                 float* saturation, float* brightness) {
    unsigned char h = 0, s = 0, v = 0;
    rgb2hsv(r, g, b, &h, &s, &v);
    *hue = (h * 2.0f) / 360.0f;
    *saturation = s / 255.0f;
    *brightness = v / 255.0f;
}

static bool procreate_parse_swatch_object(const char* obj, unsigned char* r, unsigned char* g,
                                          unsigned char* b) {
    const char* components = strstr(obj, "\"components\"");
    if (components) {
        const char* arr = strchr(components, '[');
        if (arr) {
            double c0 = 0, c1 = 0, c2 = 0;
            const char* p = procreate_parse_json_number(arr + 1, &c0);
            if (!p) {
                return false;
            }
            p = procreate_skip_json_ws(p);
            if (*p == ',') {
                p = procreate_parse_json_number(p + 1, &c1);
            }
            if (!p) {
                return false;
            }
            p = procreate_skip_json_ws(p);
            if (*p == ',') {
                p = procreate_parse_json_number(p + 1, &c2);
            }
            if (!p) {
                return false;
            }
            *r = ClampToByte((int)(c0 * 255.0 + 0.5));
            *g = ClampToByte((int)(c1 * 255.0 + 0.5));
            *b = ClampToByte((int)(c2 * 255.0 + 0.5));
            return true;
        }
    }

    double hue = 0, sat = 0, bri = 0;
    bool have_h = false, have_s = false, have_b = false;

    const char* hkey = procreate_find_json_key(obj, "hue");
    if (hkey && procreate_parse_json_number(hkey, &hue)) {
        have_h = true;
    }
    const char* skey = procreate_find_json_key(obj, "saturation");
    if (skey && procreate_parse_json_number(skey, &sat)) {
        have_s = true;
    }
    const char* bkey = procreate_find_json_key(obj, "brightness");
    if (bkey && procreate_parse_json_number(bkey, &bri)) {
        have_b = true;
    }

    if (!have_h || !have_s || !have_b) {
        return false;
    }

    procreate_hsb_to_rgb((float)hue, (float)sat, (float)bri, r, g, b);
    return true;
}

static const char* procreate_unwrap_palette_object(const char* json) {
    json = procreate_skip_json_ws(json);
    if (*json == '[') {
        json = procreate_skip_json_ws(json + 1);
        if (*json == '{') {
            return json;
        }
        return NULL;
    }
    if (*json == '{') {
        return json;
    }
    return NULL;
}

static bool procreate_parse_swatches_json(const char* json, OcPalette* palette) {
    const char* root = procreate_unwrap_palette_object(json);
    if (!root) {
        return false;
    }

    palette->num_colors = 0;
    palette->capacity = DEFAULT_PALETTE_CAPACITY;
    palette->colors = malloc(palette->capacity * sizeof(OcPaletteColor));
    if (!palette->colors) {
        return false;
    }

    const char* name_val = procreate_find_json_key(root, "name");
    if (name_val) {
        procreate_parse_json_string(name_val, palette->name, sizeof(palette->name));
    } else {
        strncpy(palette->name, "Procreate Palette", sizeof(palette->name) - 1);
        palette->name[sizeof(palette->name) - 1] = '\0';
    }

    const char* swatches = procreate_find_json_key(root, "swatches");
    if (!swatches || *swatches != '[') {
        return palette->num_colors > 0;
    }

    swatches = procreate_skip_json_ws(swatches + 1);
    while (*swatches && *swatches != ']') {
        swatches = procreate_skip_json_ws(swatches);
        if (*swatches == ',') {
            swatches++;
            continue;
        }
        if (*swatches == 'n' && strncmp(swatches, "null", 4) == 0) {
            swatches += 4;
            continue;
        }
        if (*swatches != '{') {
            break;
        }

        const char* obj_start = swatches;
        int depth = 0;
        do {
            if (*swatches == '{') {
                depth++;
            } else if (*swatches == '}') {
                depth--;
            }
            swatches++;
        } while (*swatches && depth > 0);

        if (depth != 0) {
            break;
        }

        size_t obj_len = (size_t)(swatches - obj_start);
        char* obj_buf = malloc(obj_len + 1);
        if (!obj_buf) {
            return false;
        }
        memcpy(obj_buf, obj_start, obj_len);
        obj_buf[obj_len] = '\0';

        unsigned char r = 0, g = 0, b = 0;
        if (procreate_parse_swatch_object(obj_buf, &r, &g, &b)) {
            if (!resize_palette(palette)) {
                free(obj_buf);
                return false;
            }
            OcPaletteColor* color = &palette->colors[palette->num_colors];
            color->r = r;
            color->g = g;
            color->b = b;
            snprintf(color->name, sizeof(color->name), "Color %d", palette->num_colors + 1);
            palette->num_colors++;
        }
        free(obj_buf);

        swatches = procreate_skip_json_ws(swatches);
        if (*swatches == ',') {
            swatches++;
        }
    }

    return palette->num_colors > 0;
}

static char* procreate_extract_swatches_json(const char* filename, size_t* out_size) {
    size_t size = 0;
    void* data = mz_zip_extract_archive_file_to_heap(filename, SWATCHES_JSON_ENTRY, &size, 0);
    if (!data) {
        data = mz_zip_extract_archive_file_to_heap(filename, "swatches.json", &size, 0);
    }
    if (!data) {
        return NULL;
    }
    char* json = malloc(size + 1);
    if (!json) {
        mz_free(data);
        return NULL;
    }
    memcpy(json, data, size);
    json[size] = '\0';
    mz_free(data);
    if (out_size) {
        *out_size = size;
    }
    return json;
}

OC_STATUS read_procreate_palette(const char* filename, OcPalette* palette) {
    if (!filename || !palette) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }

    size_t json_size = 0;
    char* json = procreate_extract_swatches_json(filename, &json_size);
    if (!json) {
        return OC_STATUS_ERR_FILENOTFOUND;
    }

    OC_STATUS status = OC_STATUS_OK;
    if (!procreate_parse_swatches_json(json, palette)) {
        ocularFreePalette(palette);
        status = OC_STATUS_ERR_NOTSUPPORTED;
    }

    free(json);
    return status;
}

static int procreate_json_append(char* buf, size_t cap, size_t* pos, const char* text) {
    int n = snprintf(buf + *pos, cap - *pos, "%s", text);
    if (n < 0 || (size_t)n >= cap - *pos) {
        return -1;
    }
    *pos += (size_t)n;
    return 0;
}

static int procreate_json_append_escaped_string(char* buf, size_t cap, size_t* pos, const char* str) {
    if (procreate_json_append(buf, cap, pos, "\"") != 0) {
        return -1;
    }
    for (const char* p = str; *p; p++) {
        if (*p == '"' || *p == '\\') {
            if (procreate_json_append(buf, cap, pos, "\\") != 0) {
                return -1;
            }
        }
        char ch[2] = { *p, '\0' };
        if (procreate_json_append(buf, cap, pos, ch) != 0) {
            return -1;
        }
    }
    return procreate_json_append(buf, cap, pos, "\"");
}

static OC_STATUS procreate_write_swatches_zip(const char* filename, const void* json, size_t json_len) {
    remove(filename);

    if (!mz_zip_add_mem_to_archive_file_in_place(filename, SWATCHES_JSON_ENTRY, json, json_len, NULL, 0,
                                                 MZ_NO_COMPRESSION)) {
        remove(filename);
        return OC_STATUS_ERR_UNKNOWN;
    }

    return OC_STATUS_OK;
}

OC_STATUS save_procreate_palette(const char* filename, const OcPalette* palette) {
    if (!filename || !palette) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }

    int count = palette->num_colors > PROCREATE_MAX_SWATCHES ? PROCREATE_MAX_SWATCHES : palette->num_colors;
    if (count <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    const char* palette_name =
        (palette->name[0] != '\0') ? palette->name : "ColorSnatch Palette";

    size_t cap = 4096 + (size_t)count * 256;
    char* json = malloc(cap);
    if (!json) {
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

    size_t pos = 0;
    if (procreate_json_append(json, cap, &pos, "{\"name\":") != 0 ||
        procreate_json_append_escaped_string(json, cap, &pos, palette_name) != 0 ||
        procreate_json_append(json, cap, &pos, ",\"swatches\":[") != 0) {
        free(json);
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

    for (int i = 0; i < count; i++) {
        const OcPaletteColor* color = &palette->colors[i];
        float hue = 0, sat = 0, bri = 0;
        procreate_rgb_to_hsb((unsigned char)color->r, (unsigned char)color->g, (unsigned char)color->b,
                             &hue, &sat, &bri);

        char entry[256];
        snprintf(entry, sizeof(entry),
                 "%s{\"hue\":%.17g,\"saturation\":%.17g,\"brightness\":%.17g,\"alpha\":1,\"colorSpace\":0}",
                 (i > 0) ? "," : "", hue, sat, bri);
        if (procreate_json_append(json, cap, &pos, entry) != 0) {
            free(json);
            return OC_STATUS_ERR_OUTOFMEMORY;
        }
    }

    if (procreate_json_append(json, cap, &pos, "]}") != 0) {
        free(json);
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

    remove(filename);
    OC_STATUS status = procreate_write_swatches_zip(filename, json, pos);
    free(json);
    return status;
}

PaletteFormat detect_palette_format(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return FORMAT_UNKNOWN;
    }

    char header[32];
    size_t bytes_read = fread(header, 1, sizeof(header), file);
    fclose(file);

    if (bytes_read < 4) {
        return FORMAT_UNKNOWN;
    }

    const char* ext = strrchr(filename, '.');
    const bool is_swatches_ext = (ext && strcasecmp(ext, ".swatches") == 0);
    const bool is_zip =
        (header[0] == 'P' && header[1] == 'K' && header[2] == 3 && header[3] == 4);

    /* .swatches must be a ZIP archive; a text file with this extension is not Procreate. */
    if (is_swatches_ext) {
        return is_zip ? FORMAT_PROCREATE : FORMAT_UNKNOWN;
    }

    if (is_zip) {
        return FORMAT_PROCREATE;
    }

    {
        uint32_t magic = 0;
        memcpy(&magic, header, sizeof(magic));
        if (magic == AF_MAGIC) {
            return FORMAT_AFPALETTE;
        }
    }

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
