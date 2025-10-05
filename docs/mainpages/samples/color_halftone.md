## Color Halftone Example {#page_examples_color_halftone}

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
  
            OC_STATUS status = ocularColorHalftoneFilter(input, output, width, height, stride, 5, 100.0f, 0.0f, 33.3f, 66.7f);

            // Other color halftone examples:
            // Fine halftone (smaller dots)
            // OC_STATUS status = ocularColorHalftoneFilter(input, output, width, height, stride, 5, 100.0f, 108.0f, 162.0f, 90.0f);
                
            // Coarse halftone (larger dots, more abstract)
            // OC_STATUS status = ocularColorHalftoneFilter(input, output, width, height, stride, 15, 100.0f, 108.0f, 162.0f, 90.0f);
            
            // No angle separation (aligned screens create moiré patterns for artistic effect)
            // OC_STATUS status = ocularColorHalftoneFilter(input, output, width, height, stride, 8, 100.0f, 45.0f, 45.0f, 45.0f);
            
            // Comic book style (same as classic but with larger dots)
            // OC_STATUS status = ocularColorHalftoneFilter(input, output, width, height, stride, 12, 100.0f, 108.0f, 162.0f, 90.0f);
            
            // Low density (lighter effect, more white space)
            // OC_STATUS status = ocularColorHalftoneFilter(input, output, width, height, stride, 8, 50.0f, 108.0f, 162.0f, 90.0f);

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
        <img src="images/color_halftone.jpg"/>
        <p>Before</p>
    </div>
    <div class="img-with-text">
        <img src="images/color_halftone_out.jpg" alt=""/>
        <p>After</p>
    </div>
</div>
@endhtmlonly
