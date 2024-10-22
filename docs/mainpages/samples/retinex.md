## Retinex Example {#page_examples_retinex}

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
  
            OcRetinexMode mode = RETINEX_UNIFORM;  // [RETINEX_UNIFORM|RETINEX_LOW|RETINEX_HIGH]
            float scale = 240.0f;  // [16.0 - 250.0]
            float numScales = 3.0f;  // [1.0 - 8.0]
            float dynamic = 1.2f;  // [0.05 - 4.0]
            ocularMultiscaleRetinex(inputImage, outputImage, width, height, channels, mode, scale, numScales, dynamic);
  
            stbi_write_jpg("test_out.jpg", width, height, channels, outputImage, 100);  
        }  
        free(outputImage);  
    }  
  
    stbi_image_free(inputImage);  
}
```

@htmlonly
<div class="sample-images">
    <div class="img-with-text">
        <img src="retinex.jpg"/>
        <p>Before</p>
    </div>
    <div class="img-with-text">
        <img src="retinex_out.jpg" alt=""/>
        <p>After</p>
    </div>
</div>
@endhtmlonly
