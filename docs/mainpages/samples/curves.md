## Curves Example {#page_examples_curves}

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
            
            // Create curves for RGB channels and luminance
            OcCurve* curveR = createCurve();
            OcCurve* curveG = createCurve();
            OcCurve* curveB = createCurve();
            OcCurve* curveL = createCurve();
            
            if (curveR && curveG && curveB && curveL) {
                // Create an S-curve for red channel (contrast enhancement)
                curveAddPoint(curveR, 64, 32);      // Darken shadows
                curveAddPoint(curveR, 192, 224);    // Brighten highlights
                
                // Create a linear curve for green (no change)
                // Default linear mapping is already set, no points needed
                
                // Boost blue channel slightly
                curveAddPoint(curveB, 128, 140);
                
                // Create S-curve for luminance (overall contrast)
                curveAddPoint(curveL, 50, 25);      // Darken shadows
                curveAddPoint(curveL, 128, 128);    // Keep midtones
                curveAddPoint(curveL, 205, 230);    // Brighten highlights
                
                // Build lookup tables for all curves
                curveBuild(curveR);
                curveBuild(curveG);
                curveBuild(curveB);
                curveBuild(curveL);
                
                // Apply curves filter
                // Pass NULL for green to use identity mapping (no change)
                OC_STATUS status = ocularCurvesFilter(inputImage, outputImage, width, height, stride,
                                                      curveR, NULL, curveB, curveL);
                if (status == OC_STATUS_OK) {
                    stbi_write_jpg("test_out.jpg", width, height, channels, outputImage, 100);  
                }
                
                // Clean up curves
                destroyCurve(curveR);
                destroyCurve(curveG);
                destroyCurve(curveB);
                destroyCurve(curveL);
            }
        }  
        free(outputImage);  
    }  
  
    stbi_image_free(inputImage);
    return 0;
}
```
