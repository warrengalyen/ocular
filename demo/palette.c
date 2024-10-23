#if defined(_MSC_VER)
    #define _CRT_SECURE_NO_WARNINGS
    #include <windows.h>
    #define access _access
#else
    #include <unistd.h>
#endif

#include "../lib/palette.h"

#define AUTO_OPEN_OUTPUT_IMAGE // comment out to disable

#include <stdio.h>
#include "timing.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <direct.h>
    #define getcwd _getcwd
    #include <shellapi.h> // for ShellExecute
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

int main(int argc, char** argv) {

    printf("Ocular Image Processing library\n");
    printf("Palette format demo\n");
    printf("https://github.com/warrengalyen/ocular/ \n");

    char palette_file[1024] = "bin\\palettes\\breeze.gpl";
    char currentPath[1024];
    if (getcwd(currentPath, sizeof(currentPath)) != NULL) {
        char fullPath[2048];
        sprintf(fullPath, "%s\\%s", currentPath, palette_file);

        PaletteData palette;
        read_gimp_palette(fullPath, &palette);
        // print palette
        for (int i = 0; i < palette.num_colors; i++) {
            printf("color %d: %s, r: %d, g: %d, b: %d\n", i, palette.colors[i].name, palette.colors[i].r, palette.colors[i].g,
                   palette.colors[i].b);
        }

        char palette_file[1024] = "bin\\palettes\\test.gpl";
        sprintf(fullPath, "%s\\%s", currentPath, palette_file);
        save_gimp_palette(fullPath, &palette);
    }

    return 0;
}
