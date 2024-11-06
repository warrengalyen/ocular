using System.Drawing.Imaging;
using System.Runtime.InteropServices;

namespace OcularDemo
{
    public partial class ResampleDialog : Form
    {
        private bool Locked;
        private Canvas canvas;
        private unsafe byte* Clone = (byte*)null;
        private ToolStripStatusLabel TimeUse;
        private int Stride;
        private int OldWidth;
        private int OldHeight;
        private int PixeFormat;

        [DllImport("kernel32.dll ")]
        private static extern bool QueryPerformanceCounter(ref long lpPerformanceCount);

        [DllImport("kernel32")]
        private static extern bool QueryPerformanceFrequency(ref long PerformanceFrequency);

        [DllImport("Kernel32.dll", EntryPoint = "RtlMoveMemory", SetLastError = true)]
        public static extern unsafe void CopyMemory(void* Dest, void* Src, int Length);

        private int WIDTHBYTES(int bytes) => (bytes * 8 + 31) / 32 * 4;

        public unsafe ResampleDialog(Canvas Parent, ToolStripStatusLabel Label, string Caption)
        {
            this.InitializeComponent();
            this.canvas = Parent;
            this.OldWidth = Parent.Image.Width;
            this.OldHeight = Parent.Image.Height;
            this.PixeFormat = (int)Parent.Image.PixelFormat;
            this.Text = Caption;
            this.TimeUse = Label;
            this.Stride = this.WIDTHBYTES(this.canvas.Image.Width * this.canvas.Channel);
            this.Clone = (byte*)(void*)Marshal.AllocHGlobal(this.canvas.Image.Height * this.Stride);
            ResampleDialog.CopyMemory((void*)this.Clone, (void*)Parent.ImageData, this.canvas.Image.Height * this.Stride);
        }

        private void ResampleDialog_Load(object sender, EventArgs e)
        {
            this.Locked = true;
            this.CmbMethod.SelectedIndex = 0;
            this.DstWUpDown.Minimum = (Decimal)(this.canvas.Image.Width / 4);
            this.DstWUpDown.Maximum = (Decimal)(this.canvas.Image.Width * 4);
            this.DstWUpDown.Value = (Decimal)this.canvas.Image.Width;
            this.lblSrcW.Text = this.canvas.Image.Width.ToString();
            this.DstHUpDown.Minimum = (Decimal)(this.canvas.Image.Height / 4);
            this.DstHUpDown.Maximum = (Decimal)(this.canvas.Image.Height * 4);
            this.DstHUpDown.Value = (Decimal)this.canvas.Image.Height;
            this.lblSrcH.Text = this.canvas.Image.Height.ToString();
            this.UpdateCanvas();
            this.canvas.Refresh();
            this.Locked = false;
        }

        private unsafe void UpdateCanvas()
        {
            if (this.Locked)
                return;
            BitmapData bitmapdata = this.canvas.Image.LockBits(new Rectangle(0, 0, this.canvas.Image.Width, this.canvas.Image.Height), ImageLockMode.ReadWrite, this.canvas.Image.PixelFormat);
            long PerformanceFrequency = 0;
            long lpPerformanceCount1 = 0;
            long lpPerformanceCount2 = 0;
            ResampleDialog.QueryPerformanceFrequency(ref PerformanceFrequency);
            ResampleDialog.QueryPerformanceCounter(ref lpPerformanceCount1);
            OcularInterop.ocularResamplingFilter(this.Clone, this.OldWidth, this.OldHeight, this.Stride, (byte*)bitmapdata.Scan0, this.canvas.Image.Width, this.canvas.Image.Height, bitmapdata.Stride);
            ResampleDialog.QueryPerformanceCounter(ref lpPerformanceCount2);
            this.TimeUse.Text = "Processing time: " + Math.Round((double)((lpPerformanceCount2 - lpPerformanceCount1) * 1000L) / (double)PerformanceFrequency, 2).ToString() + "ms.";
            this.canvas.Image.UnlockBits(bitmapdata);
            this.canvas.Refresh();
        }

        private unsafe void ResampleDialog_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (this.DialogResult != DialogResult.OK)
            {
                Bitmap bitmap = new Bitmap(this.OldWidth, this.OldHeight, (PixelFormat)this.PixeFormat);
                BitmapData bitmapdata = bitmap.LockBits(new Rectangle(0, 0, bitmap.Width, bitmap.Height), ImageLockMode.ReadWrite, bitmap.PixelFormat);

                if (bitmap.PixelFormat == PixelFormat.Format8bppIndexed)
                {
                    ColorPalette palette = bitmap.Palette;
                    for (int i = 0; i < palette.Entries.Length; ++i)
                    {
                        Color[] entries = palette.Entries;
                        Color color = Color.FromArgb((int)byte.MaxValue, i, i, i);
                        entries[i] = color;
                    }
                    bitmap.Palette = palette;
                }
                ResampleDialog.CopyMemory((void*)bitmapdata.Scan0, (void*)this.Clone, this.OldHeight * this.Stride);
                bitmap.UnlockBits(bitmapdata);
                this.canvas.Image = bitmap;
                this.canvas.Refresh();
            }
            if ((IntPtr)this.Clone == IntPtr.Zero)
                return;
            Marshal.FreeHGlobal((IntPtr)(void*)this.Clone);
        }

        private unsafe void btnCancel_Click(object sender, EventArgs e)
        {
            Bitmap bitmap = new Bitmap(this.OldWidth, this.OldHeight, (PixelFormat)this.PixeFormat);
            BitmapData bitmapdata = bitmap.LockBits(new Rectangle(0, 0, bitmap.Width, bitmap.Height), ImageLockMode.ReadWrite, bitmap.PixelFormat);
            if (bitmap.PixelFormat == PixelFormat.Format8bppIndexed)
            {
                ColorPalette palette = bitmap.Palette;
                for (int i = 0; i < palette.Entries.Length; ++i)
                {
                    Color[] entries = palette.Entries;
                    Color color = Color.FromArgb((int)byte.MaxValue, i, i, i);
                    entries[i] = color;
                }
                bitmap.Palette = palette;
            }
            ResampleDialog.CopyMemory((void*)bitmapdata.Scan0, (void*)this.Clone, this.OldHeight * this.Stride);
            bitmap.UnlockBits(bitmapdata);
            this.canvas.Image = bitmap;
            this.canvas.Refresh();
        }

        private void CmbMethod_SelectedIndexChanged(object sender, EventArgs e)
        {
            this.UpdateCanvas();
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            Bitmap bitmap = new Bitmap((int)this.DstWUpDown.Value, (int)this.DstHUpDown.Value, (PixelFormat)this.PixeFormat);
            if (bitmap.PixelFormat == PixelFormat.Format8bppIndexed)
            {
                ColorPalette palette = bitmap.Palette;
                for (int i = 0; i < palette.Entries.Length; ++i)
                {
                    Color[] entries = palette.Entries;
                    Color color = Color.FromArgb((int)byte.MaxValue, i, i, i);
                    entries[i] = color;
                }
                bitmap.Palette = palette;
            }
            this.canvas.Image = bitmap;
            this.UpdateCanvas();
        }
    }
}