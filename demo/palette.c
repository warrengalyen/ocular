#if defined(_MSC_VER)
    #define _CRT_SECURE_NO_WARNINGS
    #include <windows.h>
    #define access _access
#else
    #include <unistd.h>
#endif

#include "../lib/ocular.h"

#include <stdio.h>
#include "timing.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
    #define getcwd _getcwd
#else 
    #include <unistd.h>
#endif

void splitpath(const char* path, char* drv, char* dir, char* name, char* ext) {
    const char* end;
    const char* p;
    const char* s;
    if (path[0] && path[1] == ':') {
        if (drv) {
            *drv++ = *path++;
            *drv++ = *path++;
            *drv = '\0';
        }
    } else if (drv)
        *drv = '\0';
    for (end = path; *end && *end != ':';)
        end++;
    for (p = end; p > path && *--p != '\\' && *p != '/';)
        if (*p == '.') {
            end = p;
            break;
        }
    if (ext)
        for (s = end; (*ext = *s++);)
            ext++;
    for (p = end; p > path;)
        if (*--p == '\\' || *p == '/') {
            p++;
            break;
        }
    if (name) {
        for (s = p; s < end;)
            *name++ = *s++;
        *name = '\0';
    }
    if (dir) {
        for (s = path; s < p;)
            *dir++ = *s++;
        *dir = '\0';
    }
}

void render_palette_colors(const OcPalette* palette) {

    for (int i = 0; i < palette->num_colors; i++) {
        // Set background color
        printf("\033[48;2;%d;%d;%dm", palette->colors[i].r, palette->colors[i].g, palette->colors[i].b);

        // Set text color to white for better contrast
        printf("\033[38;2;255;255;255m %s \033[0m  ", palette->colors[i].name);
        
        // Insert a newline after every 8 colors
        if ((i + 1) % 8 == 0) {
            printf("\n");
        }
    }
    // Ensure the last line is terminated
    printf("\n");
}

int main(int argc, char** argv) {

    printf("Ocular Image Processing library\n");
    printf("Palette format demo\n");
    printf("https://github.com/warrengalyen/ocular/ \n");

    if (argc < 2) {
        printf("usage: \n ");
        printf("%s filename \n ", argv[0]);
        getchar();
        return 0;
    }

    char* in_file = argv[1];
    if (access(in_file, 0) == -1) {
        printf("load file: %s fail!\n", in_file);
        return -1;
    }
    char drive[3];
    char dir[256];
    char fname[256];
    char ext[256];
    char out_file[1024];
    splitpath(in_file, drive, dir, fname, ext);

    OcPalette palette;
    OC_STATUS status = ocularLoadPalette(in_file, &palette);
    if (status != OC_STATUS_OK) {
        printf("Failed to load palette: %s\n", ocularGetStatusString(status));
        ocularFreePalette(&palette);
        return -1;
    }

    printf("Palette name: %s\n", palette.name);
    printf("Total colors: %d\n", palette.num_colors);

    // Render palette colors in terminal
    // TODO: fix this for windows so characters are not mangled
    render_palette_colors(&palette);

    //sprintf(out_file, "bin/palettes/swatch_test.aco", drive, dir, fname);
    //save_aco_palette(out_file, &palette);

    // Must be called to free palette memory allocated during palette load
    ocularFreePalette(&palette);

    return 0;
}

