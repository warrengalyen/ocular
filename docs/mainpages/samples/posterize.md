## Posterize Example {#page_examples_posterize}

The posterize filter reduces the number of unique colors in an image using k-means clustering to create a stylized, poster-like effect.

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

            // Apply posterize effect with 8 color levels
            OC_STATUS status = ocularPosterizeFilter(inputImage, outputImage, width, height, channels, 8); 
            
            if (status == OC_STATUS_OK) {
                stbi_write_jpg("test_posterize.jpg", width, height, channels, outputImage, 100);  
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
        <img src="images/posterize.jpg"/>
        <p>Before</p>
    </div>
    <div class="img-with-text">
        <img src="images/posterize_out.jpg" alt=""/>
        <p>After</p>
    </div>
</div>
@endhtmlonly
