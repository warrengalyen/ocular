## Color Balance Example {#page_examples_color_balance}

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
  
            int redBalance = 23;    // Range: -100 to 100
            int greenBalance = 0;  // Range: -100 to 100
            int blueBalance = 14;    // Range: -100 to 100
            OcToneBalanceMode mode = SHADOWS; // [SHADOWS, MIDTONES, HIGHLIGHTS]
            bool preserveLuminosity = true;
            
            OC_STATUS status = ocularColorBalance(input, output, width, height, stride,
                                                  redBalance, greenBalance, blueBalance,
                                                  mode, preserveLuminosity);
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
        <img src="color_balance.jpg"/>
        <p>Before</p>
    </div>
    <div class="img-with-text">
        <img src="color_balance_out.jpg" alt=""/>
        <p>After</p>
    </div>
</div>
@endhtmlonly
