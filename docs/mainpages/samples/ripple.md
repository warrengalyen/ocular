## Ripple Distortion Example {#page_examples_ripple}

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
  
            int wavelength = 200; // [0, 200]
            int amplitude = 100; // [0, 100]
            float centerX = 0.5f; // normalized (0.0 = left edge, 0.5 = center, 1.0 = right edge)
            float centerY = 0.5f; // normalized (0.0 = top edge, 0.5 = center, 1.0 = bottom edge)
            float radius = 100.0f; // percentage
            float phase = 0.0f; // in degrees
            OC_STATUS status = ocularRippleDistortionFilter(inputImage, outputImage, width, height, stride, wavelength, amplitude, centerX, centerY, radiusPercentage, phase);
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
        <img src="images/ripple.jpg"/>
        <p>Before</p>
    </div>
    <div class="img-with-text">
        <img src="images/ripple_out.jpg" alt=""/>
        <p>After</p>
    </div>
</div>
@endhtmlonly
