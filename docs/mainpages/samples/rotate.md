## Rotate Example {#page_examples_rotate}

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
       
        int stride = width * channels;  

        unsigned char fillColorR = 237;
        unsigned char fillColorG = 29;
        unsigned char fillColorB = 36;
        float angle = 45.0f;
        OcInterpolationMode interpolation = OC_INTERPOLATE_BICUBIC;
        bool preserveSize = false; // set to true to keep original dimensions (corners cropped), false to enlarge to fit
        bool useTransparency = true;

        // Initialize output dimensions (will be calculated by the function)
        int newWidth = 0;
        int newHeight = 0;

        // allocate output buffer (will be resized after rotation if needed)
        // For preserveSize = false, allocate a buffer large enough for rotated image
        unsigned char* output = NULL;
        if (preserveSize) {
            if (useTransparency) {
                output = (unsigned char*)malloc(width * height * (channels == 1 ? 2 : 4));
            } else {
                output = (unsigned char*)malloc(width * height * channels);
            }
        } else {
            // Allocate buffer large enough for rotated image (approximate)
            int tempWidth = (int)(width * 1.5f);
            int tempHeight = (int)(height * 1.5f);
            if (useTransparency) {
                output = (unsigned char*)malloc(tempWidth * tempHeight * (channels == 1 ? 2 : 4));
            } else {
                output = (unsigned char*)malloc(tempWidth * tempHeight * channels);
            }
        }

        if (output) {  
            OC_STATUS status = ocularRotateImage(input, width, height, stride, output, &newWidth, &newHeight, angle,
                                                 preserveSize, useTransparency, interpolation, fillColorR, fillColorG, 
                                                 fillColorB);
            if (status == OC_STATUS_OK) {
                int outputChannels = (channels == 1 && useTransparency) ? 2 : (useTransparency ? 4 : channels);
                if (useTransparency) {
                    stbi_write_png("test_out.png", newWidth, newHeight, outputChannels, output, newWidth * outputChannels);  
                } else {
                    stbi_write_jpg("test_out.jpg", newWidth, newHeight, outputChannels, output, 100); 
                }
            }
            free(output);
        }  
    }  
  
    stbi_image_free(inputImage);  
}
```
