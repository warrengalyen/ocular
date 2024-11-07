#include "palette.h"
#include "color.h"
#include "util.h"


/* Palette mapping  -------------------------------------------------*/

int compareColors(const void* a, const void* b, int axis) {
    OcColor* c1 = (OcColor*)a;
    OcColor* c2 = (OcColor*)b;

    switch (axis) {
        case 0: return c1->R - c2->R;
        case 1: return c1->G - c2->G;
        case 2: return c1->B - c2->B;
    }
    return 0;
}

void qsortColors(OcColor* colors, int numColors, int axis) {
    if (numColors <= 1)
        return;

    // Simple partition implementation
    OcColor pivot = colors[numColors / 2];
    int i = 0;
    int j = numColors - 1;

    while (i <= j) {
        while (compareColors(&colors[i], &pivot, axis) < 0)
            i++;
        while (compareColors(&colors[j], &pivot, axis) > 0)
            j--;

        if (i <= j) {
            OcColor temp = colors[i];
            colors[i] = colors[j];
            colors[j] = temp;
            i++;
            j--;
        }
    }

    if (j > 0)
        qsortColors(colors, j + 1, axis);
    if (i < numColors)
        qsortColors(colors + i, numColors - i, axis);
}

KDNode* buildKDTree(OcColor* colors, int numColors) {
    if (numColors == 0)
        return NULL;

    KDNode* node = (KDNode*)malloc(sizeof(KDNode));
    node->depth = 0;

    // Sort colors based on current axis (R->G->B)
    int axis = node->depth % 3;
    qsortColors(colors, numColors, axis);

    // Find median
    int medianIdx = numColors / 2;
    node->color = &colors[medianIdx];

    // Recursively build left and right subtrees
    node->left = buildKDTree(colors, medianIdx);
    if (node->left)
        node->left->depth = node->depth + 1;

    node->right = buildKDTree(colors + medianIdx + 1, numColors - medianIdx - 1);
    if (node->right)
        node->right->depth = node->depth + 1;

    return node;
}

float colorDistance(OcColor* c1, OcColor* c2) {
    float dr = c1->R - c2->R;
    float dg = c1->G - c2->G;
    float db = c1->B - c2->B;
    return dr * dr + dg * dg + db * db;
}

void searchNearest(KDNode* node, OcColor* target, OcColor** best, float* bestDist) {
    if (!node)
        return;

    float dist = colorDistance(target, node->color);
    if (dist < *bestDist) {
        *bestDist = dist;
        *best = node->color;
    }

    int axis = node->depth % 3;
    int diff;
    switch (axis) {
    case 0: diff = target->R - node->color->R; break;
    case 1: diff = target->G - node->color->G; break;
    case 2: diff = target->B - node->color->B; break;
    }

    KDNode* first = diff < 0 ? node->left : node->right;
    KDNode* second = diff < 0 ? node->right : node->left;

    searchNearest(first, target, best, bestDist);

    if (diff * diff < *bestDist) {
        searchNearest(second, target, best, bestDist);
    }
}

OcColor* findNearestNeighbor(KDNode* root, OcColor* target) {
    if (!root)
        return NULL;

    OcColor* best = root->color;
    float bestDist = colorDistance(target, best);

    searchNearest(root, target, &best, &bestDist);
    return best;
}

void freeKDTree(KDNode* node) {
    if (!node)
        return;
    freeKDTree(node->left);
    freeKDTree(node->right);
    free(node);
}

void applyColorRemapKDTree(unsigned char* input, unsigned char* output, int width, int height, int channels, OcPalette* palette) {

    // convert palette to array of OcColor
    OcColor* colors = (OcColor*)malloc(palette->num_colors * sizeof(OcColor));
    for (int i = 0; i < palette->num_colors; i++) {
        colors[i].R = palette->colors[i].r;
        colors[i].G = palette->colors[i].g;
        colors[i].B = palette->colors[i].b;
    }

    KDNode* root = buildKDTree(colors, palette->num_colors);

    // Process each pixel
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * channels;

            // Create color point from input pixel
            OcColor inputColor = { input[idx], input[idx + 1], input[idx + 2] };

            // Find nearest color in palette using k-d tree
            OcColor* nearestColor = findNearestNeighbor(root, &inputColor);

            // Apply nearest color to output
            output[idx] = nearestColor->R;
            output[idx + 1] = nearestColor->G;
            output[idx + 2] = nearestColor->B;

            // Preserve alpha if present
            if (channels == 4) {
                output[idx + 3] = input[idx + 3];
            }
        }
    }

    // Clean up k-d tree
    freeKDTree(root);
    free(colors);
}

/* End Palette mapping  --------------------------------------------*/

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

    fread(&signature, 4, 1, file);
    signature = BIG_ENDIAN_32(signature);

    if (signature != ASE_SIGNATURE) {
        fclose(file);
        return OC_STATUS_ERR_NOTSUPPORTED;
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

    palette->capacity = DEFAULT_PALETTE_CAPACITY;
    palette->colors = malloc(palette->capacity * sizeof(OcPaletteColor));

    // Read blocks
    while (!feof(file)) {
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

                r = swap_float_endian(r);
                g = swap_float_endian(g);
                b = swap_float_endian(b);

                // Convert from big-endian float and scale to 0-255
                // union {
                //     float f;
                //     int i;
                // } ur, ug, ub;
                // ur.i = BIG_ENDIAN_32(*(int*)&r);
                // ug.i = BIG_ENDIAN_32(*(int*)&g);
                // ub.i = BIG_ENDIAN_32(*(int*)&b);

                color->r = (int)(r * 255.0f);
                color->g = (int)(g * 255.0f);
                color->b = (int)(b * 255.0f);

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
            } else if (color_model == ASE_COLOR_CMYK) {
                float c, m, y, k;
                fread(&c, 4, 1, file);
                fread(&m, 4, 1, file);
                fread(&y, 4, 1, file);
                fread(&k, 4, 1, file);

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
                fread(&L, 4, 1, file);
                fread(&a, 4, 1, file);
                fread(&B, 4, 1, file);

                L = swap_float_endian(L);
                a = swap_float_endian(a);
                B = swap_float_endian(B);

                // scale from float to percentage
                L *= 100.0f;

                printf("lab: %f %f %f\n", L, a, B);

                unsigned char r, g, b;
                lab2rgb(L, a, B, &r, &g, &b);
                color->r = r;
                color->g = g;
                color->b = b;

                printf("rgb: %d %d %d\n", color->r, color->g, color->b);
                palette->num_colors++;
            } else {
                fclose(file);
                return OC_STATUS_ERR_NOTSUPPORTED;
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
    return OC_STATUS_OK;
}

PaletteFormat detect_palette_format(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
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
