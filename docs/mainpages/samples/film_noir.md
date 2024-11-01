## Film Noir Example {#page_examples_film_noir}

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
    
            int shadowCutoff = 10;
            int highlightCutoff = 90;
            int contrastBoost = 10;
            int contrastMidpoint = 50;
            int grainPercentage = 50;
            OC_STATUS status = ocularFilmNoirEffect(inputImage, outputImage, width, height, channels, stride, 
                            shadowCutoff, highlightCutoff, contrastBoost, contrastMidpoint, grainPercentage);
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
        <img src="film_noir.jpg"/>
        <p>Before</p>
    </div>
    <div class="img-with-text">
        <img src="film_noir_out.jpg" alt=""/>
        <p>After</p>
    </div>
</div>
@endhtmlonly
