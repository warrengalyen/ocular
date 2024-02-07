#include "browse.h"
#include "../lib/ocular.h"

#if __has_include("test_filters.h")
    #include "test_filters.h"
#endif

#define USE_SHELL_OPEN

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

#define SFD_IMPLEMENTATION

#include "sfd.h"
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include "timing.h"
#include <stdint.h>

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

    int Width = 0;  // Image width
    int Height = 0; // Image height
    int filesize = 0;

    ocularGetImageSize(filename, &Width, &Height, &filesize);
    printf("file: %s\nfilesize: %s\nwidth: %d\nheight: %d\n", filename, calculateSize(filesize), Width, Height);

    OcImage* SrcImg;

    double startTime = now();
    SrcImg->Data = loadImage(filename, &SrcImg->Width, &SrcImg->Height, &SrcImg->Channels);
    SrcImg->Stride = SrcImg->Width * SrcImg->Channels;
    SrcImg->Depth = OC_DEPTH_8U;
    SrcImg->Reserved = 0;

    double nLoadTime = calcElapsed(startTime, now());
    printf("Load time: %d ms.\n", (int)(nLoadTime * 1000));
    if ((SrcImg->Channels != 0) && (SrcImg->Width != 0) && (SrcImg->Height != 0)) {
        if (SrcImg->Data == NULL) {
            printf("Load file: %s fail!\n", filename);
            return -1;
        }

        int status;

        // Create new image data structure for output
        OcImage* DstImg;
        status = ocularCreateImage(SrcImg->Width, SrcImg->Height, SrcImg->Depth, SrcImg->Channels, &DstImg);
        if (status != OC_STATUS_OK)
            printf("Failed to create new image!");

        // draw lines detected using linear hough transformation
        //        ocularGrayscaleFilter(inputImage, outputImg, Width, Height, Width * Channels);
        //        ocularSobelEdgeFilter(outputImg, outputImg, Width, Height);

        //        int nLines = 0;
        //        LineParameter lineParams;
        //
        //        ocularHoughLineDetection(outputImg, nLines, lineParams, Width, Height, 100);


        // ocularBilateralFilter(inputImage, outputImg, Width, Height, Width * Channels, 0.08, 0.12);

        //        int colorCoeff = 15;
        //        float cutLimit = 0.01;
        //        float contrast = 0.9;
        //        bool colorCast = ocularAutoWhiteBalance(inputImage, outputImg, Width, Height, Channels, Width * Channels,
        //        colorCoeff, cutLimit, contrast); if (colorCast) {
        //            printf("[✓] ColorCast \n");
        //        } else {
        //            printf("[x] ColorCast \n");
        //        }


        //        float Blurfilter[25] = {
        //            0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
        //        };
        //
        //        ocularConvolution2DFilter(inputImage, outputImg, Width, Height, Channels, Blurfilter, 10, 13, 0);

        // ocularMotionBlurFilter(SrcImg->Data, DstImg->Data, SrcImg->Width, SrcImg->Height, SrcImg->Channels, 5, 30);

        // ocularFlipImage(SrcImg->Data, DstImg->Data, SrcImg->Width, SrcImg->Height, SrcImg->Stride, OC_DIRECTION_VERTICAL);

        ocularRotateBilinear(SrcImg->Data, SrcImg->Width, SrcImg->Height, SrcImg->Stride, DstImg->Data, SrcImg->Width, SrcImg->Height, 120,
                             255, 255, 255);

        if (status != OC_STATUS_OK)
            printf("Image processing failed!");

        double nProcessTime = calcElapsed(startTime, now());
        printf("Processing time: %d ms.\n", (int)(nProcessTime * 1000));
        startTime = now();
        saveImage(out_file, DstImg->Width, DstImg->Height, DstImg->Channels, DstImg->Data);
        double nSaveTime = calcElapsed(startTime, now());
        printf("Save time: %d ms\n", (int)(nSaveTime * 1000));

        // Release occupied memory
        ocularFreeImage(&SrcImg);
        ocularFreeImage(&DstImg);
    } else {
        printf("Load file: %s fail!\n", filename);
    }

    getchar();
    printf("press any key to exit. \n");

    return EXIT_SUCCESS;
}
