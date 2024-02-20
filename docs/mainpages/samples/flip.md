## Flip/Mirror Example {#page_examples_flip}

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
  
            // flip image  
            ocularFlipImage(inputImage, outputImage, width, height, stride, OC_DIRECTION_VERTICAL);  
              
            // mirror image  
            ocularFlipImage(inputImage, outputImage, width, height, stride, OC_DIRECTION_HORIZONTAL);  
  
            stbi_write_jpg("test_out.jpg", width, height, channels, outputImage, 100);  
        }  
        free(outputImage);  
    }  
  
    stbi_image_free(inputImage);  
}
```