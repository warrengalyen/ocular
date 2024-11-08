#if defined(_MSC_VER)
    #define _CRT_SECURE_NO_WARNINGS
    #include <windows.h>
    #define access _access
#else

    #include <unistd.h>

#endif

#include "../lib/ocular.h"
#include "../lib/dither.h"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

#if __has_include("test_filters.h")
    // #include "test_filters.h"
#endif

#define AUTO_OPEN_OUTPUT_IMAGE // comment out to disable

#include <stdio.h>
#include "timing.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
// #include <float.h>

#if defined(AUTO_OPEN_OUTPUT_IMAGE)
    #if defined(_WIN32) || defined(_WIN64)
        #include <direct.h>
        #define getcwd _getcwd
        #include <shellapi.h> // for ShellExecute
    #endif
#endif

static const char* calculateSize(uint64_t bytes) {
    char* suffix[] = { "B", "KB", "MB", "GB", "TB" };
    char length = sizeof(suffix) / sizeof(suffix[0]);

    int i = 0;
    double dblBytes = bytes;

    if (bytes > 1024) {
        for (i = 0; (bytes / 1024) > 0 && i < length - 1; i++, bytes /= 1024)
            dblBytes = bytes / 1024.0;
    }

    static char output[200];
    sprintf(output, "%.02lf %s", dblBytes, suffix[i]);
    return output;
}

char saveFile[1024];

uint8_t* loadImage(const char* filename, int32_t* Width, int32_t* Height, int32_t* Channels) {
    return stbi_load(filename, Width, Height, Channels, 0);
}

void saveImage(const char* filename, int32_t Width, int32_t Height, int32_t Channels, uint8_t* Output) {
    if (!stbi_write_jpg(filename, Width, Height, Channels, Output, 100)) {
        fprintf(stderr, "save file fail.\n");
        return;
    } else {
        printf("file saved to: %s\n", filename);
    }
}

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

    printf("Ocular Image Processing library v%s\n", ocularGetVersion());
    printf("https://github.com/warrengalyen/ocular/ \n");

    if (argc < 2) {
        printf("usage: \n ");
        printf("%s filename \n ", argv[0]);
        printf("%s image.jpg \n ", argv[0]);
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
    sprintf(out_file, "%s%s%s_out.jpg", drive, dir, fname);

    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* input = NULL;
    input = loadImage(in_file, &width, &height, &channels);
    int stride = width * channels;

    if (input) {
        // Make sure we allocate enough memory for the output
        unsigned char* output = (unsigned char*)malloc(width * height * channels);
        if (output) {
            double startTime = now();
            printf("Processing image...\n");


            // Load the palette
            char palettePath[1024];
            char workingDir[1024];
            
            // Add error checking for getcwd
            if (getcwd(workingDir, sizeof(workingDir)) == NULL) {
                fprintf(stderr, "Error getting current working directory\n");
                return -1;
            }

            // Check if the directory exists before trying to load
            sprintf(palettePath, "%s\\bin\\palettes\\gimp\\crayola-1958.gpl", workingDir);
            if (access(palettePath, F_OK) == -1) {
                fprintf(stderr, "Palette file not found at: %s\n", palettePath);
                return -1;
            }

            // OC_STATUS status = ocularPalettetizeFromFile(input, output, width, height, channels, palettePath, OC_DITHER_FLOYD_STEINBERG, 50);
            OC_STATUS status =
                    ocularPalettetizeFromImage(input, output, width, height, channels, OC_QUANTIZE_OCTREE, 25, OC_DITHER_FLOYD_STEINBERG, 50);
            if (status != OC_STATUS_OK) {
                fprintf(stderr, "Error palettetizing image: %s\n", ocularGetStatusString(status));
                return -1;
            }

            double elapsed = calcElapsed(startTime, now());
            printf("elapsed time: %d ms.\n ", (int)(elapsed * 1000));
            
            saveImage(out_file, width, height, channels, output);

// Open the output image in the associated application
#if defined(AUTO_OPEN_OUTPUT_IMAGE)
    #if defined(_WIN32) || defined(_WIN64)
        char currentPath[1024];
        if (getcwd(currentPath, sizeof(currentPath)) != NULL) {
            char fullPath[2048];
            sprintf(fullPath, "%s\\%s", currentPath, out_file);
            ShellExecute(0, 0, fullPath, 0, 0, SW_SHOW);
        }
    #elif defined(__APPLE__)
        char openCommand[1024];
        sprintf(openCommand, "open %s", out_file);
        system(openCommand);
    #elif defined(__linux__)
        char openCommand[1024];
        sprintf(openCommand, "xdg-open %s", out_file);
        system(openCommand);
    #endif
#endif
            free(output);
        }
        free(input);
    } else {
        printf("load file: %s fail !\n", in_file);
    }

    return 0;
}

