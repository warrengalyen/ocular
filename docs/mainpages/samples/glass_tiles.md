## Glass Tiles Example {#page_examples_glass_tiles}

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

            OC_STATUS status = ocularGlassTilesFilter(inputImage, outputImage, width, height, stride,
                0.0f,    // rotation - angle in degrees (-45 to 45)
                20,      // tileSize - size of each tile in pixels (2 to 200)
                8.0f,    // curvature - curvature of the tiles (-20 to 20)
                5,       // quality - interpolation 1-5 (1=no AA, 2-5=supersampling)
                OC_EDGE_WRAP
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
        <img src="images/glass_tiles.jpg"/>
        <p>Before</p>
    </div>
    <div class="img-with-text">
        <img src="images/glass_tiles_out.jpg" alt=""/>
        <p>After</p>
    </div>
</div>
@endhtmlonly
