#if defined(_MSC_VER)
    #define _CRT_SECURE_NO_WARNINGS
    #include <windows.h>
    #define access _access
#else

    #include <unistd.h>

#endif

#include "../lib/ocular.h"
// #include "../lib/retinex.h"
#include "../lib/interpolate.h"

#if __has_include("test_filters.h")
    #include "test_filters.h"
#endif

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

#define AUTO_OPEN_OUTPUT_IMAGE // comment out to disable

#include <stdio.h>
#include "timing.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

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

    printf("Ocular Image Processing library\n");
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
        unsigned char* output = (unsigned char*)calloc(width * channels * height * sizeof(unsigned char), 1);
        if (output) {
            double startTime = now();
            printf("Processing image...\n");

            // In-work filter, not available.
            applyFilmNoirEffect(input, output, width, height, stride, 10.0, 90.0, 10.0, 50.0, 0.0);

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
