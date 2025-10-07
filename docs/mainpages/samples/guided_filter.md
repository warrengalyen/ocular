## Guided Filter Example {#page_examples_guided_filter}

The guided filter is an edge-preserving smoothing filter useful for denoising, detail enhancement, and HDR compression while maintaining important image structures.

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

            // Apply guided filter using input as its own guide
            OC_STATUS status = ocularGuidedFilter(inputImage, NULL, outputImage, width, height, stride, 6, 0.01f);
            
            // Using a separate guide image (must be the same size and channels as the input image)
            // unsigned char* guideImage = stbi_load("guide.jpg", &width, &height, &channels, 0);
            // OC_STATUS status = ocularGuidedFilter(inputImage, guideImage, outputImage, width, height, stride, 6, 0.01f);
            
            if (status == OC_STATUS_OK) {
                stbi_write_jpg("test_guided_filter.jpg", width, height, channels, outputImage, 100);  
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
        <img src="images/guided.jpg"/>
        <p>Before</p>
    </div>
    <div class="img-with-text">
        <img src="images/guided_out.jpg" alt=""/>
        <p>After</p>
    </div>
</div>
@endhtmlonly
