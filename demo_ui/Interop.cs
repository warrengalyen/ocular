using System.Runtime.InteropServices;

namespace OcularDemo
{
    internal static class OcularInterop
    {
        [DllImport("Kernel32.dll", EntryPoint = "RtlMoveMemory", SetLastError = true)]
        public static extern unsafe void CopyMemory(void* Dest, void* Src, int Length);

        // Parameters for Levels filter
        public struct ocularLevelParams
        {
            // color level minimum
            int levelMinimum;
            // color scale median
            int levelMiddle;
            // maximum value of color scale
            int levelMaximum;
            // minimum output value
            int minOutput;
            // maximum output value
            int maxOutput;
            // whether to apply
            bool Enable;
        }

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularBrightnessFilter(
            byte* Src, byte* Dest, int Width, int Height, int Stride,
            float Brightness);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularContrastFilter(
            byte* Src, byte* Dest, int Width, int Height, int Stride,
            float Contrast);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularWhiteBalanceFilter(
            byte* Src, byte* Dest, int Width, int Height, int Stride,
            float Temperature, float Tint);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularBilateralFilter(
            byte* Src, byte* Dest, int Width, int Height, int Stride,
            float SigmaSpatial, float SigmaRange);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularExposureFilter(
            byte* Src, byte* Dest, int Width, int Height, int Stride,
            float Exposure);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularVibranceFilter(
            byte* Src, byte* Dest, int Width, int Height, int Stride,
            float Vibrance);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularLuminanceThresholdFilter(
            byte* Src, byte* Dest, int Width, int Height, int Stride,
            byte Threshold);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularGaussianBlurFilter(
            byte* Src, byte* Dest, int Width, int Height, int Stride,
            float GaussianSigma);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularBoxBlurFilter(
            byte* Src, byte* Dest, int Width, int Height, int Stride,
            int Radius);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularResamplingFilter(
            byte* Src, int Width, int Height, int Stride, byte* Dest,
            int newWidth, int newHeight, int dstStride);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularSharpenExFilter(
            byte* Src, byte* Dest, int Width, int Height, int Stride,
            float Radius, float sharpness, int intensity);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularUnsharpMaskFilter(
            byte* Src, byte* Dest, int Width, int Height, int Stride,
            float GaussianSigma, int intensity);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularRGBFilter(
            byte* Src, byte* Dest, int Width, int Height, int Stride,
            float redAdjustment, float greenAdjustment, float blueAdjustment);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularSepiaFilter(
            byte* Src, byte* Dest, int Width, int Height, int Stride,
            int intensity);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularSaturationFilter(
            byte* Src, byte* Dest, int Width, int Height, int Stride,
            float saturation);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularHueFilter(
            byte* Src, byte* Dest, int Width, int Height, int Stride,
            float hueAdjust);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularLookupFilter(
            byte* Src, byte* Dest, byte* lookupTable,
            int Width, int Height, int Stride,
            float intensity);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularChromaKeyFilter(
            byte* Src, byte* Dest, int Width, int Height, int Stride,
            byte replaceR, byte replaceG, byte replaceB, float thresholdSensitivity, float smoothing);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularColorMatrixFilter(
            byte* Src, byte* Dest, int Width, int Height, int Stride,
            float* colorMatrix, float intensity);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularAverageColor(
              byte* Src, int Width, int Height, int Stride,
              ref byte AverageR, ref byte AverageG, ref byte AverageB, ref byte AverageA);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularLuminosity(
             byte* Src, int Width, int Height, int Stride,
             ref byte Luminance);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularFalseColorFilter(
            byte* Src, byte* Dest, int Width, int Height, int Stride,
            byte firstColorR, byte firstColorG, byte firstColorB,
            byte secondColorR, byte secondColorG, byte secondColorB, int intensity);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularHazeFilter(
            byte* Src, byte* Dest, int Width, int Height, int Stride,
            float distance, float slope, int intensity);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularOpacityFilter(
            byte* Src, int Width, int Height, int Stride,
            float opacity);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularLevelsFilter(
            byte* Src, byte* Dest, int Width, int Height, int Stride,
            ref ocularLevelParams redLevelParams,
            ref ocularLevelParams greenLevelParams,
            ref ocularLevelParams blueLevelParams);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularHighlightShadowTintFilter(
           byte* Src, byte* Dest, int Width, int Height, int Stride,
           float shadowTintR, float shadowTintG, float shadowTintB,
           float highlightTintR, float highlightTintG, float highlightTintB,
           float shadowTintIntensity, float highlightTintIntensity);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularHighlightShadowFilter(
            byte* Src, byte* Dest, int Width, int Height, int Stride,
            float shadows, float highlights);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularMonochromeFilter(
            byte* Src, byte* Dest, int Width, int Height, int Stride,
            float filterColorR, float filterColorG, float filterColorB, int intensity);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularColorInvertFilter(
            byte* Src, byte* Dest, int Width, int Height, int Stride);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularSolidColorGenerator(
            byte* Dest, int Width, int Height, int Stride,
            byte colorR, byte colorG, byte colorB, byte colorAlpha);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe bool ocularAutoWhiteBalance(
            byte* Src, byte* Dest, int Width, int Height, int Channels, int Stride,
            int colorCoeff = 15, float cutLimit = 0.01f, float contrast = 0.9f);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularAutoLevel(
             byte* Src, byte* Dest, int Width, int Height, int Stride,
             float CutLimit = 0.001f);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularSkinToneFilter(
             byte* Src, byte* Dest, int Width, int Height, int Channels, int Stride,
             float skinToneAdjust, float skinHue, float skinHueThreshold,
             float maxSaturationShift, int upperSkinToneColor);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularCropFilter(
             byte* Src, byte* Dest, int Width, int Height, int Stride, byte* Output,
             int cropX, int cropY, int dstWidth, int dstHeight, int dstStride);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularSobelEdgeFilter(
            byte* Src, byte* Dest, int Width, int Height);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe int ocularHoughLines(
            byte* Src, int Width, int Height,
            int lineIntensity, int Threshold, float resTheta, int numLine,
            ref float Radius, ref float Theta);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularDrawLine(
           byte* Dest, int Width, int Height, int Stride,
           int x1, int y1, int x2, int y2,
           byte R, byte G, byte B);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularGrayscaleFilter(
            byte* Src, byte* Dest, int Width, int Height, int Stride);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularGammaFilter(
            byte* Src, byte* Dest, int Width, int Height, int Stride,
            float[] gammaRGB);

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularMotionBlurFilter(
            byte* Src, byte* Dest, int Width, int Height, int Stride,
            int distance, int angle);


        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe void ocularRadialBlur(
            byte* Src, byte* Dest, int Width, int Height, int Channels,
            int centerX, int centerY, int angle);
    }
}
