## Pinch Distortion Example {#page_examples_pinch}

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
  
            // Apply pinch distortion (positive = pinch, negative = bulge)
            OC_STATUS status = ocularPinchDistortionFilter(inputImage, outputImage, width, height, stride, 50.0f);
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
        <img src="images/pinch.jpg"/>
        <p>Before</p>
    </div>
    <div class="img-with-text">
        <img src="images/pinch_out.jpg" alt=""/>
        <p>After</p>
    </div>
</div>
@endhtmlonly
