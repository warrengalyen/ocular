## BEEPS (Bi-exponential Edge-preserving Smoothing) Example {#page_examples_beeps}

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
  
            float photometricStandardDeviation = 255.0f;
            float spatialDecay = 0.1; // [0.01 - 0.250]
            int rangeFilter = 1;      //  [0,1 or 2] [Gaussian|Hyperbolic Secant|Euler Constant]

            OC_STATUS status = ocularBEEPSFilter(inputImage, outputImage, width, height, stride, photometricStandardDeviation, 
                              spatialDecay, rangeFilter);
            if (status == OC_STATUS_OK) {
                stbi_write_jpg("test_out.jpg", width, height, channels, outputImage, 100);  
            }
        }  
        free(outputImage);  
    }  
  
    stbi_image_free(inputImage);  
}
```

@htmlonly
<div class="sample-images">
    <div class="img-with-text">
        <img src="images/beeps.jpg"/>
        <p>Before</p>
    </div>
    <div class="img-with-text">
        <img src="images/beeps_out.jpg" alt=""/>
        <p>After</p>
    </div>
</div>
@endhtmlonly
