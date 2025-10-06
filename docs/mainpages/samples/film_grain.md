## Film Grain Example {#page_examples_film_grain}

The film grain effect simulates the texture and noise characteristics of traditional film photography, adding a vintage or artistic quality to digital images.

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

            // Apply film grain effect with strength 80.8 and softness 2.3
            OC_STATUS status = ocularFilmGrainEffect(inputImage, outputImage, width, height, channels, 80.8f, 2.3f); 
            
            if (status == OC_STATUS_OK) {
                stbi_write_jpg("test_film_grain.jpg", width, height, channels, outputImage, 100);  
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
        <img src="images/film_grain.jpg"/>
        <p>Before</p>
    </div>
    <div class="img-with-text">
        <img src="images/film_grain_out.jpg" alt=""/>
        <p>After</p>
    </div>
</div>
@endhtmlonly
