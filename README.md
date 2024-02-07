<br />
<p align="center">
  <img src="docs/images/ocular-logo-main.png" align="center"></img>

  <p align="center">
    An image processing library implemented in pure-C
  </p>
</p>

## About

Ocular is a library for common and advanced image processing
algorithms, without the use of any external dependencies. Suggestions for new features are welcome.

## Documentation

The documentation of the exported functions can be found
at [Library Documentation](https://warrengalyen.github.io/ocular/).

## Features

### Color Adjustment Filters

- Grayscale
- RGB
- Average Luminance Threshold
- Color matrix
- Sepia
- Chroma Key
- Lookup (remap colors)
- Saturation
- Gamma
- Brightness/Contrast
- Exposure
- False color (mix between two colors using luminance)
- Haze (add/remove)
- Opacity
- Levels (like Photoshop)
- Hue
- Highlight/Shadow/Tint
- Highlight/Shadow
- Monochrome
- Color Invert
- Luminance Threshold (similar to Photoshop Threshold filter)
- White Balance
- Vibrance
- Skin Tone
- Auto Level
- Auto White Balance

### Image Processing Filters

- 2D Convolution
- Bilateral (selectively blurs preserving edges and other details)
- Gaussian Blur
- Box Blur
- Motion Blur
- Unsharp Mask
- Sharpen
- Lanczos resampling (resize)
- Sobel Edge

### General

- Retrieve average image luminosity
- Retrieve average color
- Retrieve image size
- Crop
- Flip (Vertical/Horizontal)
- Hough transform line detection

### Color Conversion

-

## Running

To compile the library, simply do the following:

```sh
cmake .
make
```

This will out the following 3 files:

- static library `lib\libocular.xxx`
- dynamic link library `bin\ocular.dll`
- console demo `bin\demo.exe`

## TODO

Planned features include:

Bokeh effect, deskewing, retinex, automatic red-eye removal, skin smoothing and possibly deblurring.


