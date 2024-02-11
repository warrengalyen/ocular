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

Note: Big-Endian RGBA byte-order is expected for all pixel data.

## Documentation

The documentation of the exported functions can be found
at [Library Documentation](https://www.mechanikadesign.com/docs/ocular/1.0).

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
- Median Blur
- Radial Blur
- Unsharp Mask
- Sharpen
- Lanczos resampling (resize)
- Sobel Edge
- Deskewing

### General

- Retrieve average image luminosity
- Retrieve average color
- Retrieve image size
- Crop
- Flip (Vertical/Horizontal)
- Bilinear image rotation
- Hough transform line detection

### Color Space Conversion

Note: These are primarly used for included filters and added on as needed basis.

- RGB <--> YIQ
- RGB <--> HSV
- RGB <--> YCbCr

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

### GUI Demo Application

I'm working on a full-featured image viewer app in C# for showing-casing library features using the compiled DLL.

### Library

I have a very long list of features to implement, some advanced features include:

retinex, automatic red-eye removal, skin smoothing and possibly deblurring.


