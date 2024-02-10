## Auto White Balance Example {#page_examples_autowb}

```c
int colorCoeff = 15;  
float cutLimit = 0.01;  
float contrast = 0.9;  
bool colorCast = ocularAutoWhiteBalance(inputImage, outputImage, width, height, channels, stride colorCoeff, cutLimit, contrast);  
if (colorCast) {  
    printf("[✓] ColorCast\n");  
} else {  
    printf("[x] ColorCast\n");  
}
```