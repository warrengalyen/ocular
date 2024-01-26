# Ocular

An image processing library implemented in pure-C, with no external dependencies.

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

### Image Processing Filters

- Guassian Blur
- Box Blur
- Unsharp Mask
- Sharpen
- Bilinear resampling (resize)
- Crop
- Sobel Edge

### General

- Retrieve average image luminosity
- Retrieve average color
- Retrieve image size
- Hough tranform line detection

## TODO

Planned features include:

Bilteral filter, bokeh effect, deskewing, retinex, auto white balance,
skin smoothing and possibly deblurring.

Suggestions for new features are welcome. The main focus of this library is common and advanced image processing algorithms.
