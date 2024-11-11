## Palettize From Image Example {#page_examples_palettize_image}

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
  
            // reduce unique colors to 25 and apply 50% dithering 
            OC_STATUS status =  ocularPaletteFromImage(inputImage, outputImage, width, height, channels, 
                                                        OC_QUANTIZE_MEDIAN_CUT, 25, OC_DITHER_FLOYD_STEINBERG, 50);
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
        <img src="images/palettize.jpg"/>
        <p>Before</p>
    </div>
    <div class="img-with-text">
        <img src="images/palettize_image_out.jpg" alt=""/>
        <p>After</p>
    </div>
</div>
@endhtmlonly
