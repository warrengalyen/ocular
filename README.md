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
- Auto White Balance

### Image Processing Filters

- 2D Convolution
- Bilateral (selectively blurs preserving edges and other details)
- Guassian Blur
- Box Blur
- Motion Blur
- Unsharp Mask
- Sharpen
- Lanzcos resampling (resize)
- Sobel Edge

### General

- Retrieve average image luminosity
- Retrieve average color
- Retrieve image size
- Crop
- Hough tranform line detection

## TODO

Planned features include:

Bokeh effect, deskewing, retinex, automatic red-eye removal, skin smoothing and possibly deblurring.

Suggestions for new features are welcome. The main focus of this library is common and advanced image processing algorithms.
