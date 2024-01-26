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
// ref:https://github.com/nothings/stb/blob/master/stb_image.h
#define TJE_IMPLEMENTATION
#include "tiny_jpeg.h"
// ref:https://github.com/serge-rgb/TinyJPEG/blob/master/tiny_jpeg.h
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

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
// stores the current location of the file passed in
char saveFile[1024];
// Load images
unsigned char *loadImage(const char *filename, int *Width, int *Height,
                         int *Channels) {
  return stbi_load(filename, Width, Height, Channels, 0);
}

// Save image
void saveImage(const char *filename, int Width, int Height, int Channels,
               unsigned char *Output) {

  memcpy(saveFile + strlen(saveFile), filename, strlen(filename));
  *(saveFile + strlen(saveFile) + 1) = 0;
  // Save as jpg
  if (!tje_encode_to_file(saveFile, Width, Height, Channels, true, Output)) {
    fprintf(stderr, "Writing to JPEG file failed.\n");
    return;
  }
#ifdef USE_SHELL_OPEN
  browse(saveFile);
#endif
}

// split path function
void splitpath(const char *path, char *drv, char *dir, char *name, char *ext) {
  const char *end;
  const char *p;
  const char *s;
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

// Get the current file location passed in
void getCurrentFilePath(const char *filePath, char *saveFile) {
  char drive[_MAX_DRIVE];
  char dir[_MAX_DIR];
  char fname[_MAX_FNAME];
  char ext[_MAX_EXT];
  splitpath(filePath, drive, dir, fname, ext);
  int n = strlen(filePath);
  memcpy(saveFile, filePath, n);
  char *cur_saveFile = saveFile + (n - strlen(ext));
  cur_saveFile[0] = '_';
  cur_saveFile[1] = 0;
}

int main(int argc, char **argv) {

  printf("Ocular Image Processing library\n ");
  printf("repo: https://github.com/warrengalyen/ocular/ \n ");
  printf("Supports parsing the following image formats: \n ");
  printf("JPG, PNG, TGA, BMP, PSD, GIF, HDR, PIC \n ");

  // Check whether the parameters are correct
  if (argc < 2) {
    printf("Parameter error. \n ");
    printf("Please drag and drop the image file onto the executable file, or "
           "use the command "
           "line: demo.exe image \n ");
    printf("Please drag and drop the file e.g.: demo.exe d:\\image.jpg \n ");

    return 0;
  }

  char *szfile = argv[1];
  // Check if the input file exists
  if (access(szfile, 0) == -1) {
    printf(
        "The input file does not exist and the parameters are incorrect! \n ");
  }

  getCurrentFilePath(szfile, saveFile);

  int Width = 0;                    // Image width
  int Height = 0;                   // Image height
  int Channels = 0;                 // Number of image channels
  unsigned char *inputImage = NULL; // Input image pointer

  int filesize = 0;
  ocularGetImageSize(szfile, &Width, &Height, &filesize);
  printf("file: %s\nfilesize: %d\nwidth: %d\nheight: %d\n", szfile, filesize,
         Width, Height);

  double startTime = now();
  // Load images
  inputImage = loadImage(szfile, &Width, &Height, &Channels);

  double nLoadTime = calcElapsed(startTime, now());
  printf("Loading time: %d milliseconds!\n ", (int)(nLoadTime * 1000));
  if ((Channels != 0) && (Width != 0) && (Height != 0)) {

    // Allocate and load the same memory for processing and outputting results
    unsigned char *outputImg = (unsigned char *)stbi__malloc(
        Width * Channels * Height * sizeof(unsigned char));
    if (inputImage) {
      // If the image is loaded successfully, the content will be copied to the
      // output memory for easy processing.
      memcpy(outputImg, inputImage, Width * Channels * Height);
    } else {
      printf("Load file: %s fail!\n ", szfile);
    }
    startTime = now();
    float arrRho[100];
    float arrTheta[100];
    int nTNum = 200;
    int nTVal = 100;
    float Theta = 1.0f;
    ocularGrayscaleFilter(inputImage, outputImg, Width, Height,
                          Width * Channels);
    ocularSobelEdge(outputImg, outputImg, Width, Height);
    int nLine = ocularHoughLines(outputImg, Width, Height, nTNum, nTVal, Theta,
                                 100, arrRho, arrTheta);
    memcpy(outputImg, inputImage, Width * Channels * Height);
    for (int i = 0; i < nLine; i++) {
      if (arrTheta[i] == 90) {
        ocularDrawLine(outputImg, Width, Height, Width * Channels,
                       (int)arrRho[i], 0, (int)arrRho[i], Height - 1, 255, 0,
                       0);
      } else {
        int x1 = 0;
        int y1 = (int)(arrRho[i] / fastCos(arrTheta[i] * M_PI / 180.0f) + 0.5f);
        int x2 = Width - 1;
        int y2 = (int)((arrRho[i] - x2 * fastSin(arrTheta[i] * M_PI / 180.0f)) /
                           fastCos(arrTheta[i] * M_PI / 180.0f) +
                       0.5f);
        ocularDrawLine(outputImg, Width, Height, Width * Channels, x1, y1, x2,
                       y2, 255, 0, 0);
      }
    }

    // Processing algorithm
    double nProcessTime = now();
    printf("Processing time: %d milliseconds!\n ", (int)(nProcessTime * 1000));
    // Save the processed image
    startTime = now();

    saveImage("_done.jpg", Width, Height, Channels, outputImg);
    double nSaveTime = calcElapsed(startTime, now());

    printf("Saving took: %d milliseconds!\n ", (int)(nSaveTime * 1000));
    // Release occupied memory
    if (outputImg) {
      stbi_image_free(outputImg);
      outputImg = NULL;
    }

    if (inputImage) {
      stbi_image_free(inputImage);
      inputImage = NULL;
    }
  } else {
    printf("Load file: %s fail!\n", szfile);
  }

  getchar();
  printf("Press any key to exit the program \n");

  return EXIT_SUCCESS;
}