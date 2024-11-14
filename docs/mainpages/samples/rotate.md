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
        OcInterpolationType interpolation = OC_INTERPOLATE_BICUBIC;

        // Calculate new dimensions if needed
        int newWidth, newHeight;
        bool keepSize = false; // set to true to keep the original dimensions while cropping corners
        bool useTransparency = true;
        if (!keepSize) {
            float angleRad = fabs(angle * M_PI / 180.0f);
            newWidth = (int)(width * fabs(cos(angleRad)) + height * fabs(sin(angleRad)));
            newHeight = (int)(width * fabs(sin(angleRad)) + height * fabs(cos(angleRad)));
        } else {
            newWidth = width;
            newHeight = height;
        }

        // allocate output buffer
        unsigned char* output = NULL;
        if (useTransparency) {
            output = (unsigned char*)malloc(newWidth * newHeight * (channels + 1));
            channels = channels == 1 ? 2 : 4;
        } else {
            output = (unsigned char*)malloc(newWidth * newHeight * channels);
        }

        if (outputImage) {  

            OC_STATUS status = ocularRotateImage(input, width, height, stride, output, newWidth, newHeight, angle,
                                                 useTransparency, interpolation, fillColorR, fillColorG, 
                                                 fillColorB);
            if (status == OC_STATUS_OK) {
                if (useTransparency) {
                    stbi_write_png("test_out.png", width, height, channels, outputImage, width * channels);  
                } else {
                    stbi_write_jpg("test_out.jpg", width, height, channels, outputImage, 100); 
                }
               
            }
        }  
        free(outputImage);  
    }  
  
    stbi_image_free(inputImage);  
}
```
