## Marble Example {#page_examples_marble}

The marble filter applies a marbling effect by displacing pixels using Perlin noise, based on JH-Labs MarbleFilter.

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

            OC_STATUS status = ocularMarbleFilter(inputImage, outputImage, width, height, stride,
                10.0f,   // scale - size of marble veins (0.0 to 100.0)
                0.5f,    // turbulence - amount of displacement (0.00 to 1.00)
                3,       // quality - 1-5 (1=no antialiasing, 2-5=supersampling)
                OC_EDGE_CLAMP,
                12345    // seed - for reproducible patterns (0 = no offset)
            );

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
        <img src="images/marble.jpg"/>
        <p>Before</p>
    </div>
    <div class="img-with-text">
        <img src="images/marble_out.jpg" alt=""/>
        <p>After</p>
    </div>
</div>
@endhtmlonly
