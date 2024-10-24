#if defined(_MSC_VER)
    #define _CRT_SECURE_NO_WARNINGS
    #include <windows.h>
    #define access _access
#else
    #include <unistd.h>
#endif

#include "../lib/palette.h"

#include <stdio.h>
#include "timing.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

void render_palette_colors(const PaletteData* palette) {

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

    char palette_file[1024] = "bin\\palettes\\crayola-1958.gpl";
    char currentPath[1024];
    if (getcwd(currentPath, sizeof(currentPath)) != NULL) {
        char fullPath[2048];
        sprintf(fullPath, "%s\\%s", currentPath, palette_file);

        PaletteData palette;
        read_gimp_palette(fullPath, &palette);

        printf("Palette name: %s\n", palette.name);
        printf("Total colors: %d\n", palette.num_colors);

        // Render palette colors in terminal
        // TODO: fix this for windows so characters are not mangled
        render_palette_colors(&palette);
        char palette_file[1024] = "bin\\palettes\\test.gpl";
        sprintf(fullPath, "%s\\%s", currentPath, palette_file);
        save_gimp_palette(fullPath, &palette);
    }

    return 0;
}
