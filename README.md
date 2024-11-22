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

Note: RGBA byte-order is expected for all pixel data.

## Documentation

The documentation of the exported functions along with examples can be found
at [Library Documentation](https://www.mechanikadesign.com/docs/ocular/1.0).

## Features

### Color Adjustment Filters

- Grayscale
- RGB
- HSL
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
- Auto Contrast
- Auto Gamma Correction
- Auto Level
- Auto White Balance
- Equalize (like Photoshop)
- Histogram Stretch (contrast stretching)
- Auto Threshold (with 13 methods)
- Backlight Repair (corrects extremely low or non-uniform lighting)
- Image Blending (supports 27 Photoshop modes)
- Color Balance
- Retinex (Multi-Scale with Color Restoration)
- Palettize
  - from file (remaps colors to closest color in palette file with optional dithering)
  - from image (reduces unique colors using color quantization with optional dithering)

### Image Processing Filters

#### Blur/De-noise

- Bilateral (selectively blurs preserving edges and other details)
- Average (Mean) Blur
- Gaussian Blur
- Box Blur
- Motion Blur
- Zoom Blur
- Median Blur
- Radial Blur
- Surface Blur
- Skin Smoothing
- BEEPS (Bi-exponential Edge-preserving Smoothing)
- Exponential Blur

#### Morphology

- Erode/Dilate
- Min/Max
- High Pass

#### Sharpening

- Unsharp Mask
- Sharpen

#### Edge Detection

- Canny Edge
- Gradient Edge
- Sobel Edge

#### Artistic

- Pixelate (Mosaic)
- Oil Paint
- Frosted Glass
- Film Grain

#### OCR Pre-preprocessing

- Despeckle (Salt & Pepper Noise Removal)
- Deskewing

#### Misc

- 2D Convolution
- Resampling (resize) [Nearest-neighbor, bilinear, bicubic and lanzcos]

### General

- Retrieve average image luminosity
- Retrieve average color
- Retrieve image size
- Crop
- Flip (Vertical/Horizontal)
- Rotate image (bilinear and bicubic)
- Hough transform line detection

### Color Quantization

- Median Cut, Octree

### Dithering

- Ordered Bayer (4x4 and 8x8), Atkinson, Burkes, Sierra, Sierra Two-Row, Sierra Lite, Stucki, Jarvis-Judice-Ninke, Single Neighbor, Floyd-Steinberg

### Color Space Conversion

Note: These are primarly used for included filters and added on as needed basis.

- RGB <--> YIQ
- RGB <--> HSL
- RGB <--> HSV
- RGB <--> YCbCr
- RGB <--> CMYK
- RGB <--> CIELab

### Palettes

| File Extension      | Name                              |   Read  |  Write  |
|---------------------|-----------------------------------|:-------:|:-------:|
| `.aco`              | Adobe Color Swatch                |   ✓     |    ✓*   |
| `.act`              | Adobe Color Table                 |   ✓     |    ✓    |
| `.ase`              | Adobe Swatch Exchange             |   ✓    |         |
| `.gpl`              | GIMP Palette                      |   ✓     |    ✓    |
| `.pal, .psppalette` | Paint Shop Pro Palette            |   ✓     |    ✓    |
| `.pal`              | Microsoft RIFF Palette            |   ✓     |    ✓    |
| `.txt`              | Paint.NET Palette                 |   ✓     |    ✓    |

> **_NOTE:_**  *RGB only.

## Running

To compile the library, simply do the following:

```sh
cmake .
make
```

This will out the following 4 files:

- static library `lib\libocular.xxx`
- dynamic link library `bin\ocular.dll`
- cli image demo `bin\demo.exe`
- cli palette demo `bin\palette.exe`

## TODO

### GUI Demo Application

I'm working on a full-featured image viewer app in C# for showing-casing the library. 
This is currently available in the `demo-gui` branch and is very much a work-in-progress.

## Licensing

Ocular is under the MIT license.

See the [LICENSE](LICENSE) file for details.
