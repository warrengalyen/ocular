// If it is Windows, call the system API ShellExecuteA to open the picture
#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#define access _access
#else
#include <unistd.h>
#endif
#include "browse.h"
#define USE_SHELL_OPEN
#include "ocular.h"
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define SFD_IMPLEMENTATION

#include "sfd.h"
#include <inttypes.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

// Timing
#include <stdint.h>
#if defined(__APPLE__)
#include <mach/mach_time.h>
#elif defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else // __linux
#include <time.h>
#ifndef CLOCK_MONOTONIC //_RAW
#define CLOCK_MONOTONIC CLOCK_REALTIME
#endif
#endif
static uint64_t nanotimer() {
    static int ever = 0;
#if defined(__APPLE__)
    static mach_timebase_info_data_t frequency;
    if (!ever) {
        if (mach_timebase_info(&frequency) != KERN_SUCCESS) {
            return 0;
        }
        ever = 1;
    }
    return mach_absolute_time() * frequency.numer / frequency.denom;
#elif defined(_WIN32)
    static LARGE_INTEGER frequency;
    if (!ever) {
        QueryPerformanceFrequency(&frequency);
        ever = 1;
    }
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return (t.QuadPart * (uint64_t)1e9) / frequency.QuadPart;
#else // __linux
    struct timespec t;
    if (!ever) {
        if (clock_gettime(CLOCK_MONOTONIC, &spec) != 0) {
            return 0;
        }
        ever = 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &spec);
    return (t.tv_sec * (uint64_t)1e9) + t.tv_nsec;
#endif
}

static double now() {
    static uint64_t epoch = 0;
    if (!epoch) {
        epoch = nanotimer();
    }
    return (nanotimer() - epoch) / 1e9;
};

double calcElapsed(double start, double end) {
    double took = -start;
    return took + end;
}
#ifndef _MAX_DRIVE
#define _MAX_DRIVE 3
#endif
#ifndef _MAX_FNAME
#define _MAX_FNAME 256
#endif
#ifndef _MAX_EXT
#define _MAX_EXT 256
#endif
#ifndef _MAX_DIR
#define _MAX_DIR 256
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

unsigned char* loadImage(const char* filename, int* Width, int* Height, int* Channels) {
    return stbi_load(filename, Width, Height, Channels, 0);
}

void saveImage(const char* filename, int Width, int Height, int Channels, unsigned char* Output) {

    if (!stbi_write_jpg(filename, Width, Height, Channels, Output, 100)) {
        fprintf(stderr, "Writing to JPEG file failed.\n");
        return;
    }
#ifdef USE_SHELL_OPEN
    browse(filename);
#endif
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

    char* filename = NULL;
    if (argc < 2) {
        printf("usage: %s   image \n ", argv[0]);
        printf("e.g.: %s   d:\\image.jpg \n ", argv[0]);
        sfd_Options opt = {
            .title = "Open Image File",
            .filter_name = "Image File",
            .filter = "*.png|*.jpg",
        };

        filename = (char*)sfd_open_dialog(&opt);
        if (filename) {
            printf("Retrieved file: '%s'\n", filename);
        } else {
            printf("Open canceled\n");
            return -1;
        }
    } else
        filename = argv[1];


    char drive[3];
    char dir[256];
    char fname[256];
    char ext[256];
    char out_file[1024];
    splitpath(filename, drive, dir, fname, ext);
    sprintf(out_file, "%s%s%s_processed.jpg", drive, dir, fname);

    int Width = 0;                    // Image width
    int Height = 0;                   // Image height
    int Channels = 0;                 // Number of image channels
    unsigned char* inputImage = NULL; // Input image pointer

    int filesize = 0;
    ocularGetImageSize(filename, &Width, &Height, &filesize);
    printf("file: %s\nfilesize: %s\nwidth: %d\nheight: %d\n", filename, calculateSize(filesize), Width, Height);

    double startTime = now();
    inputImage = loadImage(filename, &Width, &Height, &Channels);
    double nLoadTime = calcElapsed(startTime, now());
    printf("Load time: %d ms.\n", (int)(nLoadTime * 1000));
    if ((Channels != 0) && (Width != 0) && (Height != 0)) {
        unsigned char* outputImg = (unsigned char*)stbi__malloc(Width * Channels * Height * sizeof(unsigned char));
        if (inputImage == NULL || outputImg == NULL) {
            printf("Load file: %s fail!\n", filename);
            return -1;
        }

        // ocularGrayscaleFilter(inputImage, outputImg, Width, Height, Width * Channels);
        // Channels = 1;
        // ocularSobelEdge(outputImg, outputImg, Width, Height);

        // ocularBilateralFilter(inputImage, outputImg, Width, Height, Width * Channels, 0.08, 0.12);

        int colorCoeff = 15;
        float cutLimit = 0.01;
        float contrast = 0.9;
        bool colorCast = ocularAutoWhiteBalance(inputImage, outputImg, Width, Height, Channels, Width * Channels, colorCoeff, cutLimit, contrast);
        if (colorCast) {
            printf("[✓] ColorCast \n");
        } else {
            printf("[x] ColorCast \n");
        }

        double nProcessTime = calcElapsed(startTime, now());
        printf("Processing time: %d ms.\n", (int)(nProcessTime * 1000));
        startTime = now();
        saveImage(out_file, Width, Height, Channels, outputImg);
        double nSaveTime = calcElapsed(startTime, now());
        printf("Save time: %d ms\n", (int)(nSaveTime * 1000));
        // Release occupied memory
        stbi_image_free(outputImg);
        stbi_image_free(inputImage);

    } else {
        printf("Load file: %s fail!\n", filename);
    }

    printf("press any key to exit. \n");
    getchar();
    return EXIT_SUCCESS;
}