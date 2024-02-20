## 2D Convolution Examples {#page_examples_convolution}

```c
#include <stdio.h>  
#include <stdlib.h>  
  
#define STB_IMAGE_IMPLEMENTATION  
#include "stb_image/stb_image.h"  
#define STB_IMAGE_WRITE_IMPLEMENTATION  
#include "stb_image/stb_image_write.h"  
  
int main(void) {  
    int width, height, channels;  
    unsigned char* inputImage = stbi_load("test.jpg", &width, &height, &channels, 0);  
    if (inputImage) {  
        unsigned char* outputImage = (unsigned char*)calloc(width * channels * height * sizeof(unsigned char), 1);  
        if (outputImage) {  
  
            int stride = width * channels;  
  
            int Blurfilter[25] = {  
                0, 0, 1, 0, 0, 
                0, 1, 1, 1, 0,
                1, 1, 1, 1, 1,
                0, 1, 1, 1, 0,
                0, 0, 1, 0, 0,  
            };  
            ocularConvolution2DFilter(inputImage, outputImage, width, height, channels, Blurfilter, 5, 13, 0);  
  
            float MotionBlurfilter[81] = { 
                    1, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 1, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 1, 0, 0, 0, 0, 0, 0,  
                    0, 0, 0, 1, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 1, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 1, 0, 0, 0,  
                    0, 0, 0, 0, 0, 0, 1, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 1, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 1 };  
            ocularConvolution2DFilter(inputImage, outputImage, width, height, channels, MotionBlurfilter, 9, 9, 0);  
  
            // Edge detection  
            float Edgesfilter[25] = {  
                -1, 0, 0, 0, 0,
                0, -2, 0, 0, 0,
                0, 0, 6, 0, 0,
                0, 0, 0, -2, 0,
                0, 0, 0, 0, -1,  
            };  
            ocularConvolution2DFilter(inputImage, outputImage, width, height, channels, Edgesfilter, 5, 1, 0);  
  
            // Emboss  
            float Embossfilter[25] = { 
                    -1, -1, -1, -1, 0,
                    -1, -1, -1, 0, 1,
                    -1, -1, 0, 1, 1,
                    -1, 0, 1, 1, 1,
                    0, 1, 1, 1, 1 };  
            ocularConvolution2DFilter(inputImage, outputImage, width, height, channels, Embossfilter, 5, 1, 128);  
  
            stbi_write_jpg("test_out.jpg", width, height, channels, outputImage, 100);  
        }  
        free(outputImage);  
    }  
  
    stbi_image_free(inputImage);  
}
```
