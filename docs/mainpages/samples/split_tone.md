## Split Toning Example {#page_examples_split_tone}

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
  
  
            OcColor highlightColor = {255, 200, 150};
            OcColor shadowColor = {150, 200, 255};
            OC_STATUS status = ocularSplitToningFilter(inputImage, outputImage, width, height, stride, highlightColor, shadowColor, 0.0f, 50.0f);  
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
        <img src="images/split-tone.jpg"/>
        <p>Before</p>
    </div>
    <div class="img-with-text">
        <img src="images/split-tone_out.jpg" alt=""/>
        <p>After</p>
    </div>
</div>
@endhtmlonly
