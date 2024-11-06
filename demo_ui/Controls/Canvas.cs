using System.Diagnostics;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

namespace OcularDemo
{
    public partial class Canvas : UserControl
    {
        private Bitmap image;
        private string imageFile = "";
        private double zoom = 1.0;
        private unsafe byte* imageData = (byte*)null;
        private int channel = 3;
        private int stride;

        [DllImport("ocular.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        private static extern void ocularGrayscaleFilter(
          IntPtr Src,
          IntPtr Dest,
          int Width,
          int Height,
          int Stride);

        private PixelFormat ChannelToPixelFormat(int Channel)
        {
            switch (Channel)
            {
                case 1:
                    return PixelFormat.Format8bppIndexed;
                case 3:
                    return PixelFormat.Format24bppRgb;
                case 4:
                    return PixelFormat.Format32bppArgb;
                default:
                    return PixelFormat.Undefined;
            }
        }

        private int PixelFormatToChannel(PixelFormat Format)
        {
            if (Format <= PixelFormat.Format32bppRgb)
            {
                if (Format == PixelFormat.Format24bppRgb)
                    return 3;
                if (Format != PixelFormat.Format32bppRgb)
                    return Format == (PixelFormat)8207 ? 3 : 0;
            }
            else
            {
                if (Format == PixelFormat.Format8bppIndexed)
                    return 1;
                if (Format != PixelFormat.Format32bppArgb)
                    return Format == (PixelFormat)8207 ? 3 : 0;
            }
            return 4;
        }

        public unsafe int Channel
        {
            get => this.channel;
            set
            {
                if (this.image != null)
                {
                    int width = this.image.Width;
                    int height = this.image.Height;
                    if (this.channel != value)
                    {
                        Bitmap bitmap = new Bitmap(this.image.Width, this.image.Height, this.ChannelToPixelFormat(value));
                        if (value == 1)
                        {
                            ColorPalette palette = bitmap.Palette;
                            for (int index1 = 0; index1 < palette.Entries.Length; ++index1)
                            {
                                Color[] entries = palette.Entries;
                                int index2 = index1;
                                int num = index1;
                                Color color = Color.FromArgb((int)byte.MaxValue, num, num, num);
                                entries[index2] = color;
                            }
                            bitmap.Palette = palette;
                        }
                        BitmapData bitmapdata1 = this.image.LockBits(new Rectangle(0, 0, this.image.Width, this.image.Height), ImageLockMode.ReadWrite, this.ChannelToPixelFormat(this.channel));
                        BitmapData bitmapdata2 = bitmap.LockBits(new Rectangle(0, 0, bitmap.Width, bitmap.Height), ImageLockMode.ReadWrite, this.ChannelToPixelFormat(value));
                        this.imageData = (byte*)(void*)bitmapdata2.Scan0;
                        this.stride = bitmapdata2.Stride;
                        Stopwatch stopwatch = new Stopwatch();
                        if (this.channel == 3 && value == 1)
                        {
                            stopwatch.Start();
                            Canvas.ocularGrayscaleFilter(bitmapdata1.Scan0, bitmapdata2.Scan0, width, height, bitmapdata1.Stride);
                            stopwatch.Stop();
                        }
                        else
                        {
                            for (int index3 = 0; index3 < height; ++index3)
                            {
                                byte* numPtr1 = (byte*)((IntPtr)(void*)bitmapdata1.Scan0 + index3 * bitmapdata1.Stride);
                                byte* numPtr2 = (byte*)((IntPtr)(void*)bitmapdata2.Scan0 + index3 * bitmapdata2.Stride);
                                switch (value)
                                {
                                    case 1:
                                        for (int index4 = 0; index4 < width; ++index4)
                                        {
                                            numPtr2[index4] = (byte)((int)*numPtr1 + (int)numPtr1[1] + (int)numPtr1[1] + (int)numPtr1[2] >> 2);
                                            numPtr1 += this.channel;
                                        }
                                        break;
                                    case 3:
                                        if (this.channel == 1)
                                        {
                                            for (int index5 = 0; index5 < width; ++index5)
                                            {
                                                *numPtr2 = (byte)(*(sbyte*)(numPtr2 + 1) = *(sbyte*)(numPtr2 + 2) = (sbyte)numPtr1[index5]);
                                                numPtr2 += 3;
                                            }
                                            break;
                                        }
                                        for (int index6 = 0; index6 < width; ++index6)
                                        {
                                            *numPtr2 = (byte)((int)*numPtr1 * (int)numPtr1[3] / (int)byte.MaxValue);
                                            numPtr2[1] = (byte)((int)numPtr1[1] * (int)numPtr1[3] / (int)byte.MaxValue);
                                            numPtr2[2] = (byte)((int)numPtr1[2] * (int)numPtr1[3] / (int)byte.MaxValue);
                                            numPtr1 += 4;
                                            numPtr2 += 3;
                                        }
                                        break;
                                    case 4:
                                        if (this.channel == 1)
                                        {
                                            for (int index7 = 0; index7 < width; ++index7)
                                            {
                                                *numPtr2 = (byte)(*(sbyte*)(numPtr2 + 1) = *(sbyte*)(numPtr2 + 2) = (sbyte)numPtr1[index7]);
                                                numPtr2[3] = byte.MaxValue;
                                                numPtr2 += 4;
                                            }
                                            break;
                                        }
                                        for (int index8 = 0; index8 < width; ++index8)
                                        {
                                            *numPtr2 = *numPtr1;
                                            numPtr2[1] = numPtr1[1];
                                            numPtr2[2] = numPtr1[2];
                                            numPtr2[3] = byte.MaxValue;
                                            numPtr1 += 3;
                                            numPtr2 += 4;
                                        }
                                        break;
                                }
                            }
                        }
                        this.image.UnlockBits(bitmapdata1);
                        bitmap.UnlockBits(bitmapdata2);
                        this.image.Dispose();
                        this.image = bitmap;
                        this.Invalidate();
                    }
                }
                this.channel = value;
            }
        }

        public unsafe Bitmap Image
        {
            get => this.image;
            set
            {
                if (value == null)
                    return;
                int channel = this.PixelFormatToChannel(value.PixelFormat);
                switch (channel)
                {
                    case 1:
                    case 3:
                    case 4:
                        this.image = value;
                        this.channel = channel;
                        BitmapData bitmapData = this.image.LockBits(new Rectangle(0, 0, this.image.Width, this.image.Height), ImageLockMode.ReadWrite, this.image.PixelFormat);
                        this.imageData = (byte*)(void*)bitmapData.Scan0;
                        this.stride = bitmapData.Stride;
                        this.image.UnlockBits(bitmapData);
                        this.Invalidate();
                        break;
                }
            }
        }

        public unsafe byte* ImageData => this.imageData;

        public int Stride => this.stride;

        public unsafe string ImageFile
        {
            get => this.imageFile;
            set
            {
                this.imageFile = value;
                if (!(this.imageFile != ""))
                    return;
                Bitmap bitmap = (Bitmap)System.Drawing.Image.FromStream((Stream)new MemoryStream(File.ReadAllBytes(this.imageFile)));
                int channel = this.PixelFormatToChannel(bitmap.PixelFormat);

                switch (channel)
                {
                    case 1:
                    case 3:
                    case 4:
                        this.image = bitmap;
                        this.channel = channel;
                        BitmapData bitmapData = this.image.LockBits(new Rectangle(0, 0, this.image.Width, this.image.Height), ImageLockMode.ReadWrite, this.image.PixelFormat);
                        this.imageData = (byte*)(void*)bitmapData.Scan0;
                        this.stride = bitmapData.Stride;
                        this.image.UnlockBits(bitmapData);
                        this.Invalidate();
                        break;
                    default:
                        bitmap.Dispose();
                        break;
                }
            }
        }

        public double Zoom
        {
            get => this.zoom;
            set
            {
                this.zoom = value;
                this.Invalidate();
            }
        }

        public Canvas()
        {
            this.InitializeComponent();
            this.ResizeRedraw = true;
            this.SetStyle(ControlStyles.UserPaint | ControlStyles.AllPaintingInWmPaint | ControlStyles.OptimizedDoubleBuffer, true);
            this.SetStyle(ControlStyles.UserMouse, true);
        }

        protected override void OnLoad(EventArgs e)
        {
            Color color = Color.FromArgb((int)byte.MaxValue, 223, 223, 223);
            Color white = Color.White;
            Bitmap bitmap = new Bitmap(16, 16);
            Graphics graphics = Graphics.FromImage((System.Drawing.Image)bitmap);
            graphics.FillRectangle((Brush)new SolidBrush(color), 0, 0, 8, 8);
            graphics.FillRectangle((Brush)new SolidBrush(white), 8, 0, 8, 8);
            graphics.FillRectangle((Brush)new SolidBrush(white), 0, 8, 8, 8);
            graphics.FillRectangle((Brush)new SolidBrush(color), 8, 8, 8, 8);
            this.BackgroundImage = (System.Drawing.Image)bitmap;
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            if (this.image == null)
                return;
            this.Size = new Size((int)((double)this.image.Width * this.zoom), (int)((double)this.image.Height * this.zoom));
            e.Graphics.InterpolationMode = InterpolationMode.NearestNeighbor;
            e.Graphics.DrawImage((System.Drawing.Image)this.image, new Rectangle(0, 0, this.Width, this.Height), new Rectangle(0, 0, this.image.Width, this.image.Height), GraphicsUnit.Pixel);
        }


    }
}
