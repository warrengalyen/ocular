#if defined(_MSC_VER)
    #define _CRT_SECURE_NO_WARNINGS
    #include <windows.h>
    #define access _access
#else
    #include <unistd.h>
#endif

#include "util.h"
#include "config.h"
#include "../lib/ocular.h"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

#if __has_include("test_filters.h")
    // #include "test_filters.h"
#endif
#if __has_include("todo_filters.h")
    #include "todo_filters.h"
#endif

#define AUTO_OPEN_OUTPUT_IMAGE // comment out to disable

#include <stdio.h>
#include "timing.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <lcms2.h>


#ifdef _WIN32
    #include <stdlib.h>
    #define realpath(N, R) _fullpath((R), (N), _MAX_PATH)
#endif


#if defined(AUTO_OPEN_OUTPUT_IMAGE)
    #if defined(_WIN32) || defined(_WIN64)
        #include <direct.h>
        #define getcwd _getcwd
        #include <shellapi.h> // for ShellExecute
    #endif
#endif


char saveFile[1024];

uint8_t* loadImage(const char* filename, int32_t* Width, int32_t* Height, int32_t* Channels) {
    return stbi_load(filename, Width, Height, Channels, 0);
}

void saveImage(const char* filename, int32_t Width, int32_t Height, int32_t Channels, uint8_t* Output, bool usePNG) {
    
    if (!usePNG) {
        // ignore alpha channel if it exists (not supported by JPEG)
        if (Channels > 3) {
            Channels = 3;
        }
        if (!stbi_write_jpg(filename, Width, Height, Channels, Output, 100)) {
            fprintf(stderr, "save file fail.\n");
        return;
        } else {
            printf("file saved to: %s\n", filename);
        }
    } else {
        if (!stbi_write_png(filename, Width, Height, Channels, Output, Width * Channels)) {
            fprintf(stderr, "save file fail.\n");
            return;
        } else {
            printf("file saved to: %s\n", filename);
        }
    }
}

void openOutputImage(const char* outputFile) {
#if defined(AUTO_OPEN_OUTPUT_IMAGE)
    #if defined(_WIN32) || defined(_WIN64)
    char currentPath[1024];
    if (getcwd(currentPath, sizeof(currentPath)) != NULL) {
        char fullPath[2048];
        sprintf(fullPath, "%s\\%s", currentPath, outputFile);
        ShellExecute(0, 0, fullPath, 0, 0, SW_SHOW);
    }
    #elif defined(__APPLE__)
    char openCommand[1024];
    sprintf(openCommand, "open %s", outputFile);
    system(openCommand);
    #elif defined(__linux__)
    char openCommand[1024];
    sprintf(openCommand, "xdg-open %s", outputFile);
    system(openCommand);
    #endif
#endif
}

char* getFullPathFromSymlink(const char* filename) {
    static char fullPath[1024]; // Static to return the address
    if (realpath(filename, fullPath) == NULL) {
        fprintf(stderr, "Error resolving symlink for: %s\n", filename);
        return NULL;
    }
    return fullPath;
}

int applyFilterFromConfig(const char* configFile, unsigned char *input, unsigned char *output, int width, int height, 
                            int *channels, int stride) {

    if (!configFile) {
        fprintf(stderr, "Config file provided is invalid.\n");
        return -1;
    }

    Config config;
    parseConfigFile(configFile, &config);

    if (strcmp(config.function, "ocularMotionBlurFilter") == 0) {
        ocularMotionBlurFilter(input, output, width, height, stride, (int)config.params[0].value.float_val, (int)config.params[1].value.float_val);
    } else if (strcmp(config.function, "ocularZoomBlurFilter") == 0) {
        ocularZoomBlur(input, output, width, height, stride, (int)config.params[0].value.float_val, config.params[1].value.float_val, width / 2, height / 2);
    } else if (strcmp(config.function, "ocularSurfaceBlurFilter") == 0) {
        ocularSurfaceBlurFilter(input, output, width, height, stride, (int)config.params[0].value.float_val, (int)config.params[1].value.float_val);
    } else if (strcmp(config.function, "ocularBacklightRepair") == 0) {
        ocularBacklightRepair(input, output, width, height, stride);
    } else if (strcmp(config.function, "ocularCannyEdgeDetect") == 0) {
        ocularGrayscaleFilter(input, input, width, height, stride);
        *channels = 1;
        ocularCannyEdgeDetect(input, output, width, height, *channels, (int)config.params[0].value.float_val,
                              (int)config.params[1].value.float_val, (int)config.params[2].value.float_val);
    } else if (strcmp(config.function, "ocularColorBalance") == 0) {
        ocularColorBalance(input, output, width, height, stride, (int)config.params[0].value.float_val, 
                           (int)config.params[1].value.float_val, (int)config.params[2].value.float_val,
                           (int)config.params[3].value.float_val, config.params[4].value.bool_val);
    } else if (strcmp(config.function, "ocularBilateralFilter") == 0) {
        ocularBilateralFilter(input, output, width, height, stride, config.params[0].value.float_val, config.params[1].value.float_val);
    } else if (strcmp(config.function, "ocularFilmGrainEffect") == 0) {
        ocularFilmGrainEffect(input, output, width, height, *channels, config.params[0].value.float_val, config.params[1].value.float_val);
    } else if (strcmp(config.function, "ocularSharpenFilter") == 0) {
        ocularSharpenFilter(input, output, width, height, stride, config.params[0].value.float_val);
    } else if (strcmp(config.function, "ocularUnsharpMaskFilter") == 0) {
        ocularUnsharpMaskFilter(input, output, width, height, stride, config.params[0].value.float_val, config.params[1].value.float_val, config.params[2].value.float_val);
    } else if (strcmp(config.function, "ocularColoredPencilFilter") == 0) {
        ocularColoredPencilFilter(input, output, width, height, stride, (int)config.params[0].value.float_val,
                                  (int)config.params[1].value.float_val, (int)config.params[2].value.float_val,
                                  (unsigned char)config.params[3].value.float_val, (unsigned char)config.params[4].value.float_val,
                                  (unsigned char)config.params[5].value.float_val, (unsigned char)config.params[6].value.float_val,
                                  (unsigned char)config.params[7].value.float_val, (unsigned char)config.params[8].value.float_val);
    } else if (strcmp(config.function, "ocularDryBrushFilter") == 0) {
        ocularDryBrushFilter(input, output, width, height, stride, (int)config.params[0].value.float_val,
                             (int)config.params[1].value.float_val, (int)config.params[2].value.float_val);
    } else if (strcmp(config.function, "ocularFrescoFilter") == 0) {
        ocularFrescoFilter(input, output, width, height, stride, (int)config.params[0].value.float_val,
                           (int)config.params[1].value.float_val, (int)config.params[2].value.float_val);
    } else if (strcmp(config.function, "ocularPaintDaubsFilter") == 0) {
        ocularPaintDaubsFilter(input, output, width, height, stride, (int)config.params[0].value.float_val,
                               (int)config.params[1].value.float_val, (int)config.params[2].value.float_val);
    } else if (strcmp(config.function, "ocularPaletteKnifeFilter") == 0) {
        ocularPaletteKnifeFilter(input, output, width, height, stride, (int)config.params[0].value.float_val,
                                 (int)config.params[1].value.float_val, (int)config.params[2].value.float_val);
    } else if (strcmp(config.function, "ocularPlasticWrapFilter") == 0) {
        ocularPlasticWrapFilter(input, output, width, height, stride, (int)config.params[0].value.float_val,
                                (int)config.params[1].value.float_val, (int)config.params[2].value.float_val);
    } else if (strcmp(config.function, "ocularPosterEdgesFilter") == 0) {
        ocularPosterEdgesFilter(input, output, width, height, stride, (int)config.params[0].value.float_val,
                                (int)config.params[1].value.float_val, (int)config.params[2].value.float_val);
    } else if (strcmp(config.function, "ocularRoughPastelsFilter") == 0) {
        // invert may be written as a boolean (true/false) or as a number (0/1)
        int invert = (config.params[6].type == PARAM_BOOL) ? (int)config.params[6].value.bool_val
                                                           : (config.params[6].value.float_val != 0.0f);
        ocularRoughPastelsFilter(input, output, width, height, stride, (int)config.params[0].value.float_val,
                                 (int)config.params[1].value.float_val, (int)config.params[2].value.float_val,
                                 (int)config.params[3].value.float_val, (int)config.params[4].value.float_val,
                                 (int)config.params[5].value.float_val, invert);
    } else if (strcmp(config.function, "ocularWatercolorFilter") == 0) {
        ocularWatercolorFilter(input, output, width, height, stride, (int)config.params[0].value.float_val,
                               (int)config.params[1].value.float_val, (int)config.params[2].value.float_val);
    } else if (strcmp(config.function, "ocularUnderpaintingFilter") == 0) {
        // invert may be written as a boolean (true/false) or as a number (0/1)
        int invert = (config.params[6].type == PARAM_BOOL) ? (int)config.params[6].value.bool_val
                                                           : (config.params[6].value.float_val != 0.0f);
        ocularUnderpaintingFilter(input, output, width, height, stride, (int)config.params[0].value.float_val,
                                  (int)config.params[1].value.float_val, (int)config.params[2].value.float_val,
                                  (int)config.params[3].value.float_val, (int)config.params[4].value.float_val,
                                  (int)config.params[5].value.float_val, invert);
    } else {
        return -1;
    }

    return 0;
}



int main(int argc, char** argv) {

    printf("Ocular Image Processing library v%s\n", ocularGetVersion());
    printf("https://github.com/warrengalyen/ocular/ \n");


    // char* profilePath = NULL;
    // profilePath = getFullPathFromSymlink("./test_files/color_profiles/cmyk.icm");

    // uint8_t cmyk[4] = {0, 35, 15, 1};
    // uint8_t rgb[3];
    // cmykToRgb(cmyk, NULL, profilePath, rgb);
    // printf("CMYK(%d, %d, %d, %d) -> RGB(%d, %d, %d)\n", cmyk[0], cmyk[1], cmyk[2], cmyk[3], rgb[0], rgb[1], rgb[2]);

    // return 0;

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
        if (argc == 2) {

            // Make sure we allocate enough memory for the output
            unsigned char* output = (unsigned char*)malloc(width * height * channels);
            if (output) {

                double startTime = now();
                printf("Processing image...\n");

                //ocularUnsharpMaskFilterEx(input, output, width, height, stride, 60.0, 2.0, 10);
                //ocularSharpenExFilter(input, output, width, height, stride, 5, 1.2, 20);
                ocularSharpenFilter(input, output, width, height, stride, 1.2);

                //ocularMezzotintFilter(input, output, width, height, stride, OC_MEZZOTINT_DOT, 50, 10, OC_STIPPLE_NONE);

                double elapsed = calcElapsed(startTime, now());
                printf("elapsed time: %d ms.\n ", (int)(elapsed * 1000));

                sprintf(out_file, "%s%s%s_out.jpg", drive, dir, fname);
                saveImage(out_file, width, height, channels, output, false);

                free(output);

                // Open the output image in the associated application if enabled
                openOutputImage(out_file);
            }
        } else if (argc > 2) {

            // Make sure we allocate enough memory for the output
            unsigned char* output = (unsigned char*)malloc(width * height * channels);
            if (output) {
                char* configFile = NULL;
                configFile = getFullPathFromSymlink(argv[2]);
                if (access(configFile, F_OK) == 0) {
                    double startTime = now();
                    printf("Processing image...\n");

                    if (applyFilterFromConfig(configFile, input, output, width, height, &channels, stride) != -1) {
                        double elapsed = calcElapsed(startTime, now());
                        printf("elapsed time: %d ms.\n ", (int)(elapsed * 1000));

                        saveImage(out_file, width, height, channels, output, false);

                        // Open the output image in the associated application if enabled
                        openOutputImage(out_file);
                    } else {
                        printf("error applying filter from config file: %s\n", configFile);
                    }
                } else {
                    printf("config file not found: %s\n", configFile);
                }
            }

            free(output);
        }
        free(input);
    } else {
        printf("load file: %s fail !\n", in_file);
    }

    return 0;
}

