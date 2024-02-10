## 2D Convolution Examples {#page_examples_convolution}

```c
int Blurfilter[25] = {
   0, 0, 1, 0, 0,
   0, 1, 1, 1, 0,
   1, 1, 1, 1, 1,
   0, 1, 1, 1, 0,
   0, 0, 1, 0, 0,
};
ocularConvolution2DFilter(inputImage, outputImage, width, height, channels, Blurfilter, 5, 13, 0);

float MotionBlurfilter[81] = {
   1, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 1, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 1, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 1, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 1, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 1, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 1, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 1, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 1
};
ocularConvolution2DFilter(inputImage, outputImage, width, height, channels, MotionBlurfilter, 9, 9, 0);

// Edge detection
float Edgesfilter[25] = {
   -1, 0, 0, 0, 0,
   0, -2, 0, 0, 0,
   0, 0, 6, 0, 0,
   0, 0, 0, -2, 0,
   0, 0, 0, 0, -1,
};
ocularConvolution2DFilter(inputImage, outputImage, width, height, channels, Edgesfilter, 5, 1, 0);

// Emboss
float Embossfilter[25] = {
   -1, -1, -1, -1, 0,
   -1, -1, -1, 0, 1,
   -1, -1, 0, 1, 1,
   -1, 0, 1, 1, 1,
   0, 1, 1, 1, 1
};
ocularConvolution2DFilter(inputImage, outputImage, width, height, channels, Embossfilter, 5, 1, 128);
```
