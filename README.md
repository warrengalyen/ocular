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

#### Basic Adjustments

- Average Luminance Threshold
- Brightness/Contrast
- Exposure
- Equalize
- Gamma
- Histogram Stretch (contrast stretching)
- Levels
- Luminance Threshold (like Photoshop Threshold)

#### Color Balance & Tone

- Color Balance
- Color Invert
- Color Temperature
- Color Matrix
- Channel Mixer
- Curves
- False Color (mix between two colors using luminance)
- Grayscale (desaturate)
- Highlight/Shadow/Tint
- Hue
- HSL
- Levels
- Monochrome
- RGB
- Saturation
- Sepia
- Skin Tone
- Split Toning
- Vibrance
- White Balance

#### Lighting Correction

- Haze (add/remove)
- Haze Removal (Dark Channel Prior)
- Retinex (Multi-Scale with Color Restoration)
- Backlight Repair (corrects extremely low or non-uniform lighting)
- Highlight/Shadow

#### Specialized Color Processing

- Lookup (Remap Colors/LUT)
- Chroma Key (Color Key / Remove Specific Color)
- Posterize
- Palettize
  - from file (remaps colors to closest color in palette file with optional dithering)
  - from image (reduces unique colors using color quantization with optional dithering)

#### Compositing & Blending

- Opacity
- Image Blending (supports 27 Photoshop modes)

#### 🧠 Automatic Enhancements

- Auto Contrast
- Auto Gamma Correction
- Auto Level
- Auto White Balance
- Auto Threshold

### Image Processing Filters

#### Blur/De-noise

- Average (Mean) Blur
- BEEPS (Bi-exponential Edge-preserving Smoothing)
- Bilateral (selectively blurs preserving edges and other details)
- Box Blur
- Exponential Blur
- Gaussian Blur
- Guided Filter
- Median Blur
- Motion Blur
- Radial Blur
- Surface Blur
- Skin Smoothing
- Zoom Blur

#### Morphology

- Erode/Dilate
- High Pass
- Min/Max
- Skeletonize (thinning)

#### Sharpening

- Sharpen
- Unsharp Mask

#### Edge Detection

- Canny Edge
- Gradient Edge
- Laplacian Edge (Laplacian of Gaussian)
- Prewitt Edge
- Roberts Edge
- Sobel Edge

#### Stylize/Artistic

- Frosted Glass
- Film Grain
- Glass Tiles
- Kuwahara
- Oil Paint
- Portrait Glow
- Relief (Emboss)

#### Pixelate

- Color Halftone
- Crystallize
- Fragment
- Mosaic
- Pointillize

#### Distortion

- Kaleidoscope
- Pinch
- Polar Coordinates
- Ripple
- Spherize
- Twirl
- Wave

#### Render

- Clouds

#### OCR Pre-preprocessing

- Despeckle (Salt & Pepper Noise Removal)
- Deskewing
- Skeletonize (thinning)

#### Misc

- 2D Convolution
- Resampling (resize) [Nearest-neighbor, bilinear, bicubic and lanzcos]
- Image Blending (supports 27 Photoshop modes)
- FFT (Fast Fourier Transform) [Low-pass, high-pass, band-pass, band-stop, custom]
- FFT Visualization (outputs frequency domain)

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
| `.ase`              | Adobe Swatch Exchange             |   ✓     |         |
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

## Licensing

Ocular is under the MIT license.

See the [LICENSE](LICENSE) file for details.
