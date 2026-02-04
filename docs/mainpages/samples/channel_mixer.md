## Channel Mixer Example {#page_examples_channel_mixer}

The default example below uses **Example 5 (Red–blue channel swap)**. The mixer array has 16 ints: 4 output channels (R, G, B, Gray) × 4 inputs each (R, G, B, Constant). Constant is clamped to [-255, 255].

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

            /* Identity (no change)
            int mixer[] = {
                255, 0, 0, 0,   // Red out   = 100% R
                0, 255, 0, 0,   // Green out = 100% G
                0, 0, 255, 0,   // Blue out  = 100% B
                77, 150, 29, 0  // Gray = BT.601 luminance
            };
            bool monochrome = false;
            bool preserveLuminance = false;
            */

            /* Grayscale (monochrome)
            int mixer[] = {
                255, 0, 0, 0,
                0, 255, 0, 0,
                0, 0, 255, 0,
                77, 150, 29, 0   // Gray = luminance
            };
            bool monochrome = true;
            bool preserveLuminance = false;
            */

            /* Warm tint
            int mixer[] = {
                255, 20, 0, 10,   // Red: +G, +constant
                0, 255, 0, 0,
                0, 0, 230, -15    // Blue: reduced
            };
            bool monochrome = false;
            bool preserveLuminance = true;
            */

            /* Cool tint
            int mixer[] = {
                230, 0, 0, 0,
                0, 255, 20, 0,
                0, 0, 255, 15
            };
            bool monochrome = false;
            bool preserveLuminance = true;
            */

            /* Red–blue channel swap */
            int mixer[] = {
                0, 0, 255, 0,   /* Red out   = Blue in */
                0, 255, 0, 0,   /* Green out = Green in */
                255, 0, 0, 0,   /* Blue out  = Red in */
                29, 150, 77, 0  /* Gray row (BT.601-style weights) */
            };
            bool monochrome = false;
            bool preserveLuminance = false;

            /* Brightness lift (constants only)
            int mixer[] = {
                255, 0, 0, 25,
                0, 255, 0, 25,
                0, 0, 255, 25,
                77, 150, 29, 25
            };
            bool monochrome = false;
            bool preserveLuminance = false;
            */

            /* Infrared-style (strong red, weak G/B)
            int mixer[] = {
                255, 0, 0, 0,
                80, 80, 80, 0,
                80, 80, 80, 0,
                77, 150, 29, 0
            };
            bool monochrome = false;
            bool preserveLuminance = true;
            */

            OC_STATUS status = ocularChannelMixerFilter(inputImage, outputImage, width, height, stride,
                                                        mixer, monochrome, preserveLuminance);
            if (status == OC_STATUS_OK) {
                stbi_write_jpg("test_out.jpg", width, height, channels, outputImage, 100);
            }
        }
        free(outputImage);
    }

    stbi_image_free(inputImage);
    return 0;
}
```

@htmlonly
<div class="sample-images">
    <div class="img-with-text">
        <img src="images/channel_mixer.jpg"/>
        <p>Before</p>
    </div>
    <div class="img-with-text">
        <img src="images/channel_mixer_out.jpg" alt=""/>
        <p>After (Red–blue swap)</p>
    </div>
</div>
@endhtmlonly
