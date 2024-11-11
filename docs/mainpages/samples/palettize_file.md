## Palettize From File Example {#page_examples_palettize_file}

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
        
            // apply 50% dithering 
            // output image below is remapped from a 64 color palette file
            OC_STATUS status =  ocularPalettizeFromFile(inputImage, outputImage, width, height, channels, palettePath, 
                                                        OC_DITHER_FLOYD_STEINBERG, 50);
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
        <img src="images/palettize_file_out.jpg" alt=""/>
        <p>After</p>
    </div>
</div>
@endhtmlonly
