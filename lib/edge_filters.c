#include "edge_filters.h"


OC_STATUS ocularPrewittEdgeDetect(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels) {

    if (!Input || !Output)
        return OC_STATUS_ERR_NULLREFERENCE;
    if (Width <= 0 || Height <= 0 || Channels <= 0)
        return OC_STATUS_ERR_INVALIDPARAMETER;

    if (Channels != 1)
        return OC_STATUS_ERR_NOTSUPPORTED;

    // Prewitt kernels for x and y directions
    const int Gx[3][3] = { { -1, 0, 1 }, { -1, 0, 1 }, { -1, 0, 1 } };

    const int Gy[3][3] = { { -1, -1, -1 }, { 0, 0, 0 }, { 1, 1, 1 } };

    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            int px = 0, py = 0;

            // Apply kernels
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    unsigned char pixel = getMirroredPixel(Input, x + kx, y + ky, Width, Height);
                    px += pixel * Gx[ky + 1][kx + 1];
                    py += pixel * Gy[ky + 1][kx + 1];
                }
            }

            // Calculate magnitude
            int magnitude = (int)sqrt((double)(px * px + py * py));
            Output[y * Width + x] = ClampToByte(magnitude);
        }
    }

    return OC_STATUS_OK;
}

OC_STATUS ocularRobertsEdgeDetect(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels) {
    if (!Input || !Output)
        return OC_STATUS_ERR_NULLREFERENCE;
    if (Width <= 0 || Height <= 0 || Channels <= 0)
        return OC_STATUS_ERR_INVALIDPARAMETER;

    if (Channels != 1)
        return OC_STATUS_ERR_NOTSUPPORTED;

    // Roberts Cross kernels
    const int Gx[2][2] = { { 1, 0 }, { 0, -1 } };

    const int Gy[2][2] = { { 0, 1 }, { -1, 0 } };

    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            int px = 0, py = 0;

            // Apply 2x2 kernels
            for (int ky = 0; ky < 2; ky++) {
                for (int kx = 0; kx < 2; kx++) {
                    unsigned char pixel = getMirroredPixel(Input, x + kx, y + ky, Width, Height);
                    px += pixel * Gx[ky][kx];
                    py += pixel * Gy[ky][kx];
                }
            }

            // Calculate magnitude
            int magnitude = (int)sqrt((double)(px * px + py * py));
            Output[y * Width + x] = ClampToByte(magnitude);
        }
    }

    return OC_STATUS_OK;
}

OC_STATUS ocularLaplacianEdgeDetect(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, float sigma) {
    if (!Input || !Output) 
        return OC_STATUS_ERR_NULLREFERENCE;
    if (Width <= 0 || Height <= 0 || Channels <= 0)
        return OC_STATUS_ERR_INVALIDPARAMETER; 

    // Verify single channel input
    if (Channels != 1)
        return OC_STATUS_ERR_NOTSUPPORTED;

    // Calculate kernel size based on sigma (usually 2-3 times sigma on each side)
    int kernelRadius = (int)(3 * sigma + 0.5);
    int kernelSize = 2 * kernelRadius + 1;

    // Allocate kernel
    float* kernel = (float*)malloc(kernelSize * kernelSize * sizeof(float));
    if (!kernel) 
        return OC_STATUS_ERR_OUTOFMEMORY;

    // Generate LoG kernel
    float sum = 0.0f;
    float twoSigmaSquared = 2.0f * sigma * sigma;
    
    for (int y = -kernelRadius; y <= kernelRadius; y++) {
        for (int x = -kernelRadius; x <= kernelRadius; x++) {
            float distance = x * x + y * y;
            int idx = (y + kernelRadius) * kernelSize + (x + kernelRadius);
            
            // LoG equation: -1/(πσ⁴)[1 - (x² + y²)/(2σ²)]exp[-(x² + y²)/(2σ²)]
            kernel[idx] = -1.0f / (M_PI * sigma * sigma * sigma * sigma) * 
                         (1.0f - distance / twoSigmaSquared) * 
                         exp(-distance / twoSigmaSquared);
            
            sum += kernel[idx];
        }
    }

    // Normalize kernel to ensure zero sum
    float mean = sum / (kernelSize * kernelSize);
    for (int i = 0; i < kernelSize * kernelSize; i++) {
        kernel[i] -= mean;
    }

    // Apply LoG filter
    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            float sum = 0.0f;

            // Apply kernel
            for (int ky = -kernelRadius; ky <= kernelRadius; ky++) {
                for (int kx = -kernelRadius; kx <= kernelRadius; kx++) {
                    unsigned char pixel = getMirroredPixel(Input, x + kx, y + ky, Width, Height);
                    int kidx = (ky + kernelRadius) * kernelSize + (kx + kernelRadius);
                    sum += pixel * kernel[kidx];
                }
            }

            // Store absolute value of result
            Output[y * Width + x] = ClampToByte(fabs(sum));
        }
    }

    // Clean up
    free(kernel);
    return OC_STATUS_OK;
}

OC_STATUS ocularCannyEdgeDetect(const unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels,
                                CannyNoiseFilter kernel_size, int weak_threshold, int strong_threshold) {

    if (Input == NULL || Output == NULL) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    if (Width <= 0 || Height <= 0) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }
    if (Channels != 1) {
        return OC_STATUS_ERR_NOTSUPPORTED;
    }

    // Ensure filter specific parameters are within valid ranges
    if (kernel_size != CannyGaus3x3 && kernel_size != CannyGaus5x5) {
        kernel_size = CannyGaus3x3;
    }
    weak_threshold = clamp(weak_threshold, 0, 255);
    strong_threshold = clamp(strong_threshold, 0, 255);

    // Sobel, as for why this can be used instead of Gaussian derivative, see
    // https://medium.com/@haidarlina4/sobel-vs-canny-edge-detection-techniques-step-by-step-implementation-11ae6103a56a
    const int8_t Gx[] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };

    // Upward minus downward, to achieve the difference in the y direction, dy
    const int8_t Gy[] = { 1, 2, 1, 0, 0, 0, -1, -2, -1 };

    int offset_xy = 1; // for kernel = 3
    int8_t* kernel = (int8_t*)Gaus3x3;
    int kernel_div = Gaus3x3Div;

    if (kernel_size == CannyGaus5x5) {
        offset_xy = 2;
        kernel = (int8_t*)Gaus5x5;
        kernel_div = Gaus5x5Div;
    }

    float* G_ = (float*)calloc(Width * Height, sizeof(double));
    float* M_ = (float*)calloc(Width * Height, sizeof(double));
    unsigned char* s_ = (unsigned char*)calloc(Width * Height, sizeof(unsigned char));

    // gaussian filter
    for (int x = 0; x < Width; x++) {
        for (int y = 0; y < Height; y++) {
            int pos = x + (y * Width);
            if (x < offset_xy || x >= (Width - offset_xy) || y < offset_xy || y >= (Height - offset_xy)) {
                // The first and last rows, the first and last columns, get the source value
                Output[pos] = Input[pos];
                continue;
            }
            int convolve = 0;
            int k = 0;
            // Calculate convolution using relevant methods
            for (int kx = -offset_xy; kx <= offset_xy; kx++) {
                for (int ky = -offset_xy; ky <= offset_xy; ky++) {
                    convolve += (Input[pos + (kx + (ky * Width))] * kernel[k]);
                    k++;
                }
            }

            Output[pos] = (unsigned char)((double)convolve / (double)kernel_div);
        }
    }

    // apply sobel kernels
    offset_xy = 1; // 3x3
    for (int x = offset_xy; x < Width - offset_xy; x++) {
        for (int y = offset_xy; y < Height - offset_xy; y++) {
            // The first and last rows, the first and last columns are skipped and not processed.
            double convolve_X = 0.0;
            double convolve_Y = 0.0;
            int k = 0;
            int src_pos = x + (y * Width);

            for (int ky = -offset_xy; ky <= offset_xy; ky++) {
                for (int kx = -offset_xy; kx <= offset_xy; kx++) {
                    convolve_X += Output[src_pos + (kx + (ky * Width))] * Gx[k];
                    convolve_Y += Output[src_pos + (kx + (ky * Width))] * Gy[k];

                    k++;
                }
            }

            // gradient hypot & direction
            int segment = 0;

            if (convolve_X == 0.0 || convolve_Y == 0.0) {
                G_[src_pos] = 0;
            } else {
                // calculate the intensity
                G_[src_pos] = ((sqrt((convolve_X * convolve_X) + (convolve_Y * convolve_Y))));
                // direction
                double theta = atan2(convolve_Y, convolve_X); // radians. atan2 range: -PI,+PI,
                                                              // theta : 0 - 2PI
                // convert to degrees
                theta = theta * (360.0 / (2.0 * M_PI)); // degrees

                if ((theta <= 22.5 && theta >= -22.5) || (theta <= -157.5) || (theta >= 157.5)) {
                    // close the horizontal line
                    segment = 1; // "-"
                } else if ((theta > 22.5 && theta <= 67.5) || (theta > -157.5 && theta <= -112.5)) {
                    // close to the 45 degree diagonal
                    segment = 2; // "/"
                } else if ((theta > 67.5 && theta <= 112.5) || (theta >= -112.5 && theta < -67.5)) {
                    // close vertical line
                    segment = 3; // "|"
                } else if ((theta >= -67.5 && theta < -22.5) || (theta > 112.5 && theta < 157.5)) {
                    // another diagonal line close to 135 degrees
                    segment = 4; // "\"
                } else {
                    // std::cout << "error " << theta << std::endl;
                }
            }

            s_[src_pos] = (unsigned char)segment;
        }
    }

    // The 9-square grid range is set to 0 if it is not the largest in the direction.
    // local maxima: non maxima suppression
    memcpy(M_, G_, Width * Height * sizeof(double));

    for (int x = 1; x < Width - 1; x++) {
        for (int y = 1; y < Height - 1; y++) {
            // The first and last rows, the first and last columns are skipped and not processed.
            int pos = x + (y * Width);

            switch (s_[pos]) {
            case 1:
                // close to the horizontal line
                if (G_[pos - 1] >= G_[pos] || G_[pos + 1] > G_[pos]) {
                    // If there is one on the left and right that is larger than itself, set it to 0
                    M_[pos] = 0;
                }
                break;
            case 2:
                // close to the 45 degree diagonal
                if (G_[pos - (Width - 1)] >= G_[pos] || G_[pos + (Width - 1)] > G_[pos]) {
                    M_[pos] = 0;
                }
                break;
            case 3:
                // close to vertical line
                if (G_[pos - (Width)] >= G_[pos] || G_[pos + (Width)] > G_[pos]) {
                    // If there is one above or below that is larger than itself, set it to 0
                    M_[pos] = 0;
                }
                break;
            case 4:
                // another diagonal line close to 135 degrees
                if (G_[pos - (Width + 1)] >= G_[pos] || G_[pos + (Width + 1)] > G_[pos]) {
                    M_[pos] = 0;
                }
                break;
            default: M_[pos] = 0; break;
            }
        }
    }


    // double threshold
    for (int x = 0; x < Width; x++) {
        for (int y = 0; y < Height; y++) {
            int src_pos = x + (y * Width);
            if (M_[src_pos] > strong_threshold) {
                Output[src_pos] = 255;
            } else if (M_[src_pos] > weak_threshold) {
                Output[src_pos] = 100;
            } else {
                Output[src_pos] = 0;
            }
        }
    }

    // Connect the border
    // edges with hysteresis
    for (int x = 1; x < Width - 1; x++) {
        for (int y = 1; y < Height - 1; y++) {
            // The first and last rows, the first and last columns are skipped and not processed.
            int src_pos = x + (y * Width);
            if (Output[src_pos] == 255) {
                Output[src_pos] = 255;
            } else if (Output[src_pos] == 100) {
                if (Output[src_pos - 1] == 255 || Output[src_pos + 1] == 255 || Output[src_pos - 1 - Width] == 255 ||
                    Output[src_pos + 1 - Width] == 255 || Output[src_pos + Width] == 255 || Output[src_pos + Width - 1] == 255 ||
                    Output[src_pos + Width + 1] == 255) {
                    Output[src_pos] = 255;
                    // In the 9-square grid range of 100, if there is 255, adjust it to 255
                } else {
                    // Otherwise return to 0
                    Output[src_pos] = 0;
                }
            } else {
                Output[src_pos] = 0;
            }
        }
    }

    free(G_);
    free(M_);
    free(s_);

    return OC_STATUS_OK;
}

OC_STATUS ocularSobelEdgeDetect(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels) {

    if ((Input == NULL) || (Output == NULL)) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    if ((Width <= 0) || (Height <= 0)) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    if (Channels != 1) {
        return OC_STATUS_ERR_NOTSUPPORTED;
    }

    unsigned char* SqrLut = (unsigned char*)malloc(65026 * sizeof(unsigned char));
    if (SqrLut == NULL) {
        return OC_STATUS_ERR_OUTOFMEMORY;
    }
    unsigned char* RowCopy = (unsigned char*)malloc((Width + 2) * 3 * sizeof(unsigned char));
    if (RowCopy == NULL) {
        return OC_STATUS_ERR_OUTOFMEMORY;
    }

    unsigned char *First = RowCopy, *Second = RowCopy + (Width + 2), *Third = RowCopy + (Width + 2) * 2;

    for (int Y = 0; Y < 65026; Y++)
        SqrLut[Y] = (unsigned char)ClampToByte((sqrtf(Y + 0.0f) + 0.5f));

    memcpy(Second, Input, 1);
    memcpy(Second + 1, Input, Width);
    memcpy(Second + Width + 1, Input + Width - 1, 1);

    memcpy(First, Second, Width + 2);

    memcpy(Third, Input + Width, 1);
    memcpy(Third + 1, Input + Width, Width);
    memcpy(Third + Width + 1, Input + Width + Width - 1, 1);

    for (int Y = 0; Y < Height; Y++) {
        unsigned char* LinePD = Output + Y * Width;
        if (Y != 0) {
            unsigned char* Temp = First;
            First = Second;
            Second = Third;
            Third = Temp;
        }
        if (Y == Height - 1) {
            memcpy(Third, Second, Width + 2);
        } else {
            memcpy(Third, Input + (Y + 1) * Width, 1);
            memcpy(Third + 1, Input + (Y + 1) * Width, Width);
            memcpy(Third + Width + 1, Input + (Y + 1) * Width + Width - 1, 1);
        }
        for (int X = 0; X < Width; X++) {
            int Hori = First[X] + 2 * First[X + 1] + First[X + 3] - (Third[X] + 2 * Third[X + 1] + Third[X + 2]);
            int Vert = First[X] + 2 * Second[X] + Third[X] - (First[X + 2] + 2 * Second[X + 2] + Third[X + 2]);
            int Value = Hori * Hori + Vert * Vert;
            LinePD[X] = SqrLut[min(Value, 65025)];
        }
    }
    if (RowCopy) {
        free(RowCopy);
    }
    if (SqrLut) {
        free(SqrLut);
    }

    return OC_STATUS_OK;
}

OC_STATUS ocularGradientEdgeDetect(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels) {

    if ((Input == NULL) || (Output == NULL)) {
        return OC_STATUS_ERR_NULLREFERENCE;
    }
    if ((Width <= 0) || (Height <= 0)) {
        return OC_STATUS_ERR_INVALIDPARAMETER;
    }

    if (Channels != 1) {
        return OC_STATUS_ERR_NOTSUPPORTED;
    }

    unsigned char* RowCopy = (unsigned char*)malloc((Width + 2) * 3 * Channels);
    if (RowCopy == NULL) {
        return OC_STATUS_ERR_OUTOFMEMORY;
    }
    unsigned char* SqrValue = (unsigned char*)malloc(65026 * sizeof(unsigned char));
    if (SqrValue == NULL) {
        return OC_STATUS_ERR_OUTOFMEMORY;
    }
    unsigned char* First = RowCopy;
    unsigned char* Second = RowCopy + (Width + 2) * Channels;
    unsigned char* Third = RowCopy + (Width + 2) * 2 * Channels;

    for (int y = 0; y < 65026; y++)
        SqrValue[y] = (int)(sqrtf(y * 1.0) + 0.49999f);

    memcpy(Second, Input, Channels);
    memcpy(Second + Channels, Input, Width * Channels); //     Copy data to the middle position
    memcpy(Second + (Width + 1) * Channels, Input + (Width - 1) * Channels, Channels);

    memcpy(First, Second, (Width + 2) * Channels); //     The first row is the same as the second row

    memcpy(Third, Input + Width * Channels, Channels); //     Copy the second row of data
    memcpy(Third + Channels, Input + Width * Channels, Width * Channels);
    memcpy(Third + (Width + 1) * Channels, Input + Width * Channels + (Width - 1) * Channels, Channels);

    for (int y = 0; y < Height; y++) {
        unsigned char* LinePD = Output + y * Width;
        if (y != 0) {
            unsigned char* Temp = First;
            First = Second;
            Second = Third;
            Third = Temp;
        }
        if (y == Height - 1) {
            memcpy(Third, Second, (Width + 2) * Channels);
        } else {
            memcpy(Third, Input + (y + 1) * Width * Channels, Channels);
            memcpy(Third + Channels, Input + (y + 1) * Width * Channels,
                   Width * Channels); //     Since the data of the previous row is backed up, there is no problem even if Src and Dest are the same
            memcpy(Third + (Width + 1) * Channels, Input + (y + 1) * Width * Channels + (Width - 1) * Channels, Channels);
        }
        for (int x = 0; x < Width; x++) {
            int GradientH = 0, GradientV = 0;
            if (x == 0) {
                GradientH = First[x + 0] + First[x + 1] + First[x + 2] - (Third[x + 0] + Third[x + 1] + Third[x + 2]);
            } else {
                GradientH = GradientH - First[x - 1] + First[x + 2] + Third[x - 1] - Third[x + 2];
            }
            GradientV = First[x + 0] + Second[x + 0] * 2 + Third[x + 0] - (First[x + 2] + Second[x + 2] * 2 + Third[x + 2]);
            int Value = (GradientH * GradientH + GradientV * GradientV) >> 1;
            if (Value > 65025)
                LinePD[x] = 255;
            else
                LinePD[x] = SqrValue[Value];
        }
    }
    free(RowCopy);
    free(SqrValue);

    return OC_STATUS_OK;
}