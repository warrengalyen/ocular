## Wave Distortion Example {#page_examples_wave}

The wave distortion filter creates wave-like distortions similar to Photoshop's Wave filter. It supports multiple wave generators with different wavelengths, amplitudes, and wave types (sine, triangle, square).

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
  
            // Example 1: Gentle sine waves with single generator
            OC_STATUS status = ocularWaveDistortionFilter(
                inputImage, outputImage, width, height, stride,
                1,              // numGenerators - number of waves to combine
                201, 202,       // minWavelength, maxWavelength - wave size in pixels (shorter = tighter waves)
                36, 27,         // minAmplitude, maxAmplitude - displacement in pixels
                100, 100,       // scaleX, scaleY - horizontal and vertical scale (100% = full effect)
                OC_WAVE_SINE,   // waveType - OC_WAVE_SINE, OC_WAVE_TRIANGLE, or OC_WAVE_SQUARE
                0               // seed - 0 for random, or specific value for reproducible results
            );
            
            if (status == OC_STATUS_OK) {
                stbi_write_jpg("wave_sine_out.jpg", width, height, channels, outputImage, 100);  
            }
            
            // Example 2: Sharp square waves with multiple generators
            status = ocularWaveDistortionFilter(
                inputImage, outputImage, width, height, stride,
                3,              // numGenerators - multiple waves for complex patterns
                20, 40,         // minWavelength, maxWavelength - tighter waves
                15, 25,         // minAmplitude, maxAmplitude - stronger displacement
                100, 100,       // scaleX, scaleY - full effect in both directions
                OC_WAVE_SQUARE, // waveType - sharp square waves
                0               // seed - random seed
            );
            
            if (status == OC_STATUS_OK) {
                stbi_write_jpg("wave_square_out.jpg", width, height, channels, outputImage, 100);  
            }
            
            // Example 3: Horizontal-only waves (vertical displacement)
            status = ocularWaveDistortionFilter(
                inputImage, outputImage, width, height, stride,
                2,              // numGenerators
                30, 80,         // minWavelength, maxWavelength - mixed wave sizes
                8, 15,          // minAmplitude, maxAmplitude - moderate displacement
                0, 100,         // scaleX, scaleY - horizontal-only effect
                OC_WAVE_TRIANGLE, // waveType - triangle waves
                0               // seed - random
            );
            
            if (status == OC_STATUS_OK) {
                stbi_write_jpg("wave_horizontal_out.jpg", width, height, channels, outputImage, 100);  
            }
            
            // Example 4: Vertical-only waves (horizontal displacement)
            status = ocularWaveDistortionFilter(
                inputImage, outputImage, width, height, stride,
                2,              // numGenerators
                25, 50,         // minWavelength, maxWavelength - shorter waves
                10, 20,         // minAmplitude, maxAmplitude - strong displacement
                100, 0,         // scaleX, scaleY - vertical-only effect
                OC_WAVE_SINE,   // waveType - smooth sine waves
                0               // seed - random
            );
            
            if (status == OC_STATUS_OK) {
                stbi_write_jpg("wave_vertical_out.jpg", width, height, channels, outputImage, 100);  
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
        <img src="images/wave.jpg"/>
        <p>Before</p>
    </div>
    <div class="img-with-text">
        <img src="images/wave_out.jpg" alt="Sine waves"/>
        <p>After</p>
    </div>
</div>
@endhtmlonly
