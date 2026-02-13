## Channel Mixer Example {#page_examples_channel_mixer}

The default example below uses **Red–blue channel swap**. The mixer array has 16 floats: 4 rows (R out, G out, B out, Gray) × 4 inputs each (R, G, B, Constant). RGB and Gray rows use the same modifier scale: percentages in [-2.0, 2.0] (e.g. 1.0 = 100%). Gray is calculated internally from the Gray row; a default ITU luminance uses 0.21, 0.72, 0.07 (R, G, B). Constant modifiers are in [-255.0, 255.0].

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

            /* Identity (no change) — RGB & Gray: [-2.0, 2.0], Constant: [-255, 255]
            float mixer[] = {
                1.0f, 0.0f, 0.0f, 0.0f,   // Red out   = 100% R
                0.0f, 1.0f, 0.0f, 0.0f,   // Green out = 100% G
                0.0f, 0.0f, 1.0f, 0.0f,   // Blue out  = 100% B
                0.21f, 0.72f, 0.07f, 0.0f  // Gray row (default ITU: R, G, B same scale as RGB)
            };
            bool monochrome = false;
            bool preserveLuminance = false;
            */

            /* Grayscale (monochrome)
            float mixer[] = {
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.21f, 0.72f, 0.07f, 0.0f   // Gray row (default ITU)
            };
            bool monochrome = true;
            bool preserveLuminance = false;
            */

            /* Warm tint
            float mixer[] = {
                1.0f, 0.08f, 0.0f, 10.0f,   // Red: +8% G, constant +10
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.9f, -15.0f,  // Blue: 90%, constant -15
                0.21f, 0.72f, 0.07f, 0.0f
            };
            bool monochrome = false;
            bool preserveLuminance = true;
            */

            /* Cool tint
            float mixer[] = {
                0.9f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.08f, 0.0f,
                0.0f, 0.0f, 1.0f, 15.0f,
                0.21f, 0.72f, 0.07f, 0.0f
            };
            bool monochrome = false;
            bool preserveLuminance = true;
            */

            /* Red–blue channel swap */
            float mixer[] = {
                0.0f, 0.0f, 1.0f, 0.0f,   /* Red out   = 100% Blue in */
                0.0f, 1.0f, 0.0f, 0.0f,   /* Green out = 100% Green in */
                1.0f, 0.0f, 0.0f, 0.0f,   /* Blue out  = 100% Red in */
                0.21f, 0.72f, 0.07f, 0.0f   /* Gray row (default ITU) */
            };
            bool monochrome = false;
            bool preserveLuminance = false;

            /* Brightness lift (constants only)
            float mixer[] = {
                1.0f, 0.0f, 0.0f, 25.0f,
                0.0f, 1.0f, 0.0f, 25.0f,
                0.0f, 0.0f, 1.0f, 25.0f,
                0.21f, 0.72f, 0.07f, 25.0f
            };
            bool monochrome = false;
            bool preserveLuminance = false;
            */

            /* Infrared-style (strong red, weak G/B)
            float mixer[] = {
                1.0f, 0.0f, 0.0f, 0.0f,
                0.31f, 0.31f, 0.31f, 0.0f,
                0.31f, 0.31f, 0.31f, 0.0f,
                0.21f, 0.72f, 0.07f, 0.0f
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
