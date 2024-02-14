## Canny Edge Detection Example {#page_examples_canny_edge}

```c
int lower_threshold = 30; // pixels below this value are ignored
int upper_threshold = 100; // pixels above this value are considered as edge pixels

// Gaussian Noise reduction kernel 
// CannyGaus3x3 = 3 x 3 Gaussian
// CannyGaus5x5 = 5 x 5 Gaussian

ocularGrayscaleFilter(input, input, width, height, stride);  
channels = 1; // grayscale filter converts data to single channel, so reset channels
ocularCannyEdgeDetect(input, output, width, height, channels, CannyGaus3x3, lower_threshold, upper_threshold);
```