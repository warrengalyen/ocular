## Auto White Balance Example {#page_examples_autowb}

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
  
            int colorCoeff = 15;  
            float cutLimit = 0.01;  
            float contrast = 0.9;  
            bool colorCast = false;
            OC_STATUS status = ocularAutoWhiteBalance(inputImage, outputImage, width, height, channels, stride, colorCoeff, cutLimit, contrast, &colorCast);
            if (status == OC_STATUS_OK) {
                if (colorCast) {  
                    printf("[✓] ColorCast\n");  
                } else {  
                    printf("[x] ColorCast\n");  
                }  

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
        <img src="images/awb.jpg"/>
        <p>Before</p>
    </div>
    <div class="img-with-text">
        <img src="images/awb_out.jpg" alt=""/>
        <p>After</p>
    </div>
</div>
@endhtmlonly