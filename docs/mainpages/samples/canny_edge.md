## Canny Edge Detection Example {#page_examples_canny_edge}

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
  
            int lower_threshold = 30; // pixels below this value are ignored  
            int upper_threshold = 100; // pixels above this value are considered as edge pixels  
            
            // Gaussian Noise reduction kernel   
            // CannyGaus3x3 = 3 x 3 Gaussian  
            // CannyGaus5x5 = 5 x 5 Gaussian
            OC_STATUS status;  
            status = ocularGrayscaleFilter(inputImage, inputImage, width, height, stride);
            if (status == OC_STATUS_OK) {
                channels = 1; // grayscale filter converts data to single channel, so reset channels  
                status = ocularCannyEdgeDetect(inputImage, outputImage, width, height, channels, CannyGaus3x3, lower_threshold, upper_threshold);  
                if (status == OC_STATUS_OK) {
                    stbi_write_jpg("test_out.jpg", width, height, channels, outputImage, 100);  
                }
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
        <img src="images/canny_edge.jpg"/>
        <p>Before</p>
    </div>
    <div class="img-with-text">
        <img src="images/canny_edge_out.jpg" alt=""/>
        <p>After</p>
    </div>
</div>
@endhtmlonly