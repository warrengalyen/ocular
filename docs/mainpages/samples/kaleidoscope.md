## Kaleidoscope Example {#page_examples_kaleidoscope}

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
  
           // Apply kaleidoscope effect
            OC_STATUS status = ocularKaleidoscopeFilter(inputImage, outputImage, width, height, stride,
                6,      // mirrors - number of mirror segments (2-20)
                0.0f,   // angle - primary rotation angle in degrees (0-360)
                0.0f,   // angle2 - secondary rotation angle in degrees (0-360)
                0.5f,   // centerX - center X position (0.0-1.0, 0.5 = center)
                0.5f,   // centerY - center Y position (0.0-1.0, 0.5 = center)
                100.0f  // radius - effect radius percentage (0-100, 0 = full image)
            );

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
        <img src="images/kaleidoscope.jpg"/>
        <p>Before</p>
    </div>
    <div class="img-with-text">
        <img src="images/kaleidoscope_out.jpg" alt=""/>
        <p>After</p>
    </div>
</div>
@endhtmlonly
