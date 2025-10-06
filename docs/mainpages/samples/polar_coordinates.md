## Polar Coordinates Example {#page_examples_polar_coordinates}

The polar coordinates filter converts images between rectangular (Cartesian) and polar coordinate systems, similar to Photoshop's Polar Coordinates filter. This is useful for creating circular patterns from linear images or unwrapping circular patterns into rectangular form.

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

            // Convert rectangular image to polar coordinates
            // This wraps the image around a circle (center maps to outer edge)
            OC_STATUS status = ocularPolarCoordinatesFilter(inputImage, outputImage, width, height, stride, OC_RECT_TO_POLAR); 
            
            if (status == OC_STATUS_OK) {
                stbi_write_jpg("test_polar.jpg", width, height, channels, outputImage, 100);  
                
                // To convert back from polar to rectangular coordinates:
                // OC_STATUS status = ocularPolarCoordinatesFilter(inputImage, outputImage, width, height, stride, OC_POLAR_TO_RECT);
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
        <img src="images/polar_coordinates.jpg"/>
        <p>Before</p>
    </div>
    <div class="img-with-text">
        <img src="images/polar_coordinates_out.jpg" alt=""/>
        <p>After</p>
    </div>
</div>
@endhtmlonly
