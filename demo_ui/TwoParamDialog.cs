using System.Diagnostics;
using System.Runtime.InteropServices;

namespace OcularDemo
{
    public partial class TwoParamDialog : Form
    {
        private Canvas canvas;
        private unsafe byte* Clone = (byte*)null;
        private ToolStripStatusLabel TimeUse;
        private int Stride;
        private bool Locked;
        private FilterType filterType;

        [DllImport("Kernel32.dll", EntryPoint = "RtlMoveMemory", SetLastError = true)]
        public static extern unsafe void CopyMemory(void* Dest, void* Src, int Length);


        [DllImport("kernel32.dll ")]
        private static extern bool QueryPerformanceCounter(ref long lpPerformanceCount);

        [DllImport("kernel32")]
        private static extern bool QueryPerformanceFrequency(ref long PerformanceFrequency);

        private int WIDTHBYTES(int bytes) => (bytes * 8 + 31) / 32 * 4;

        public unsafe TwoParamDialog(Canvas Parent, ToolStripStatusLabel Label, string Caption, FilterType type)
        {
            InitializeComponent();
            this.canvas = Parent;
            this.Text = Caption;
            this.filterType = type;
            this.TimeUse = Label;
            this.Stride = this.WIDTHBYTES(this.canvas.Image.Width * this.canvas.Channel);
            this.Clone = (byte*)(void*)Marshal.AllocHGlobal(this.canvas.Image.Height * this.Stride);
            CopyMemory((void*)this.Clone, (void*)Parent.ImageData, this.canvas.Image.Height * this.Stride);
        }

        private unsafe void UpdateCanvas()
        {
            Console.WriteLine("test");
            if (this.Locked)
                return;
            if (this.ChkPreview.Checked)
            {
                Stopwatch stopwatch = new Stopwatch();
                stopwatch.Start();

                switch (filterType)
                {
                    case FilterType.AUTO_WB:
                        // We don't need to modify the color coefficent parameter unless we want to return if the image has color casting.
                        OcularInterop.ocularAutoWhiteBalance(this.Clone, this.canvas.ImageData, this.canvas.Image.Width, this.canvas.Image.Height, this.canvas.Channel, this.canvas.Stride, 15, (float)this.AdjustLevel.Value * 0.0001f, (float)this.AdjustLevel2.Value * 0.01f);
                        break;
                    case FilterType.BRIGHTNESS_CONTRAST:
                        //OcularInterop.ocularBrightnessFilter(this.Clone, this.canvas.ImageData, this.canvas.Image.Width, this.canvas.Image.Height, this.canvas.Stride, this.AdjustLevel.Value * 0.01f);
                        OcularInterop.ocularContrastFilter(this.canvas.ImageData, this.canvas.ImageData, this.canvas.Image.Width, this.canvas.Image.Height, this.canvas.Stride, (float)this.AdjustLevel2.Value * 0.01f);
                        break;
                    case FilterType.WHITE_BALANCE:
                        OcularInterop.ocularWhiteBalanceFilter(this.Clone, this.canvas.ImageData, this.canvas.Image.Width, this.canvas.Image.Height, this.canvas.Stride, (float)this.AdjustLevel.Value, (float)this.AdjustLevel2.Value);
                        break;
                    case FilterType.BILATERAL:
                        OcularInterop.ocularBilateralFilter(this.Clone, this.canvas.ImageData, this.canvas.Image.Width, this.canvas.Image.Height, this.canvas.Stride, (float)this.AdjustLevel.Value * 0.01f, (float)this.AdjustLevel2.Value * 0.01f);
                        OcularInterop.ocularBilateralFilter(this.Clone, this.canvas.ImageData, this.canvas.Image.Width, this.canvas.Image.Height, this.canvas.Stride, (float)this.AdjustLevel.Value * 0.01f, (float)this.AdjustLevel2.Value * 0.01f);
                        break;
                    case FilterType.UNSHARP_MASK:
                        OcularInterop.ocularUnsharpMaskFilter(this.Clone, this.canvas.ImageData, this.canvas.Image.Width, this.canvas.Image.Height, this.canvas.Stride, (float)this.AdjustLevel.Value, this.AdjustLevel2.Value);
                        break;
                    case FilterType.SHADOW_HIGHLIGHT:
                        OcularInterop.ocularHighlightShadowFilter(this.Clone, this.canvas.ImageData, this.canvas.Image.Width, this.canvas.Image.Height, this.canvas.Stride, (float)this.AdjustLevel.Value * 0.01f, (float)this.AdjustLevel2.Value * 0.01f);
                        break;
                    case FilterType.MOTION_BLUR:
                        OcularInterop.ocularMotionBlurFilter(this.Clone, this.canvas.ImageData, this.canvas.Image.Width, this.canvas.Image.Height, this.canvas.Stride, this.AdjustLevel.Value, this.AdjustLevel2.Value);
                        break;
                    default:
                        throw new NotImplementedException();
                }

                stopwatch.Stop();
                this.TimeUse.Text = "Processing time: " + stopwatch.ElapsedMilliseconds.ToString() + "ms";
            }
            else
            {
                CopyMemory((void*)this.canvas.ImageData, (void*)this.Clone, this.canvas.Image.Height * this.Stride);
            }
            this.canvas.Refresh();
        }

        private void AdjustLevel_Scroll(object sender, EventArgs e)
        {
            this.AdjustLevelUpDown.Value = (Decimal)this.AdjustLevel.Value;
        }

        private void AdjustLevelUpDown_ValueChanged(object sender, EventArgs e)
        {
            this.AdjustLevel.Value = (int)this.AdjustLevelUpDown.Value;
            this.UpdateCanvas();
        }

        private void AdjustLevel2_Scroll(object sender, EventArgs e)
        {
            this.AdjustLevelUpDown2.Value = (Decimal)this.AdjustLevel2.Value;
        }

        private void AdjustLevelUpDown2_ValueChanged(object sender, EventArgs e)
        {
            this.AdjustLevel2.Value = (int)this.AdjustLevelUpDown2.Value;
            this.UpdateCanvas();
        }

        private unsafe void TwoParamDialog_FormClosed(object sender, FormClosedEventArgs e)
        {
            if (this.DialogResult != DialogResult.OK)
            {
                CopyMemory((void*)this.canvas.ImageData, (void*)this.Clone, this.canvas.Image.Height * this.Stride);
                this.canvas.Refresh();
            }
            if ((IntPtr)this.Clone == IntPtr.Zero)
                return;
            Marshal.FreeHGlobal((IntPtr)(void*)this.Clone);
        }

        private void ChkPreview_CheckedChanged(object sender, EventArgs e)
        {
            this.UpdateCanvas();
        }

        private unsafe void btnCancel_Click(object sender, EventArgs e)
        {
            CopyMemory((void*)this.canvas.ImageData, (void*)this.Clone, this.canvas.Image.Height * this.Stride);
            this.canvas.Refresh();
        }

        private void TwoParamDialog_Load(object sender, EventArgs e)
        {
            switch (filterType)
            {
                case FilterType.AUTO_WB:
                    this.Locked = true;
                    this.lblAdjust.Text = "Cut Limit:";
                    this.AdjustLevel.Minimum = 1;
                    this.AdjustLevel.Maximum = 100;
                    this.AdjustLevel.Value = 16;
                    this.AdjustLevelUpDown.Minimum = 1;
                    this.AdjustLevelUpDown.Maximum = 100;
                    this.AdjustLevelUpDown.Value = 16;
                    this.lblAdjust2.Text = "Contrast:";
                    this.AdjustLevel2.Minimum = 0;
                    this.AdjustLevel2.Maximum = 100;
                    this.AdjustLevel2.Value = 90;
                    this.AdjustLevelUpDown2.Minimum = 0;
                    this.AdjustLevelUpDown2.Maximum = 100;
                    this.AdjustLevelUpDown2.Value = 90;
                    this.Locked = false;
                    break;
                case FilterType.BRIGHTNESS_CONTRAST:
                    this.Locked = true;
                    this.lblAdjust.Text = "Brightness:";
                    this.AdjustLevel.Minimum = -100;
                    this.AdjustLevel.Maximum = 100;
                    this.AdjustLevel.Value = 0;
                    this.AdjustLevelUpDown.Minimum = -100;
                    this.AdjustLevelUpDown.Maximum = 100;
                    this.AdjustLevelUpDown.Value = 0;
                    this.lblAdjust2.Text = "Contrast:";
                    this.AdjustLevel2.Minimum = 0;
                    this.AdjustLevel2.Maximum = 400;
                    this.AdjustLevel2.Value = 100;
                    this.AdjustLevelUpDown2.Minimum = 0;
                    this.AdjustLevelUpDown2.Maximum = 400;
                    this.AdjustLevelUpDown2.Value = 100;
                    this.Locked = false;
                    break;
                case FilterType.WHITE_BALANCE:
                    this.Locked = true;
                    this.lblAdjust.Text = "Temperature:";
                    this.AdjustLevel.Minimum = 4000;
                    this.AdjustLevel.Maximum = 7500;
                    this.AdjustLevel.Value = 5000;
                    this.AdjustLevelUpDown.Minimum = 4000;
                    this.AdjustLevelUpDown.Maximum = 7500;
                    this.AdjustLevelUpDown.Value = 5000;
                    this.lblAdjust2.Text = "Tint:";
                    this.AdjustLevel2.Minimum = -200;
                    this.AdjustLevel2.Maximum = 200;
                    this.AdjustLevel2.Value = 0;
                    this.AdjustLevelUpDown2.Minimum = -200;
                    this.AdjustLevelUpDown2.Maximum = 200;
                    this.AdjustLevelUpDown2.Value = 0;
                    this.Locked = false;
                    break;
                case FilterType.BILATERAL:
                    this.Locked = true;
                    this.lblAdjust.Text = "Radius:";
                    this.AdjustLevel.Minimum = 0;
                    this.AdjustLevel.Maximum = 100;
                    this.AdjustLevel.Value = 3;
                    this.AdjustLevelUpDown.Minimum = 0;
                    this.AdjustLevelUpDown.Maximum = 100;
                    this.AdjustLevelUpDown.Value = 3;
                    this.lblAdjust2.Text = "Tolerance:";
                    this.AdjustLevel2.Minimum = 0;
                    this.AdjustLevel2.Maximum = 100;
                    this.AdjustLevel2.Value = 15;
                    this.AdjustLevelUpDown2.Minimum = 0;
                    this.AdjustLevelUpDown2.Maximum = 100;
                    this.AdjustLevelUpDown2.Value = 15;
                    this.Locked = false;
                    break;
                case FilterType.UNSHARP_MASK:
                    this.Locked = true;
                    this.lblAdjust.Text = "Radius:";
                    this.AdjustLevel.Minimum = 1;
                    this.AdjustLevel.Maximum = 100;
                    this.AdjustLevel.Value = 2;
                    this.AdjustLevelUpDown.Minimum = 1;
                    this.AdjustLevelUpDown.Maximum = 100;
                    this.AdjustLevelUpDown.Value = 2;
                    this.lblAdjust2.Text = "Strength:";
                    this.AdjustLevel2.Minimum = 0;
                    this.AdjustLevel2.Maximum = 500;
                    this.AdjustLevel2.Value = 100;
                    this.AdjustLevelUpDown2.Minimum = 0;
                    this.AdjustLevelUpDown2.Maximum = 500;
                    this.AdjustLevelUpDown2.Value = 100;
                    this.Locked = false;
                    break;
                case FilterType.SHADOW_HIGHLIGHT:
                    this.Locked = true;
                    this.lblAdjust.Text = "Shadow:";
                    this.AdjustLevel.Minimum = -100;
                    this.AdjustLevel.Maximum = 100;
                    this.AdjustLevel.Value = 0;
                    this.AdjustLevelUpDown.Minimum = -100;
                    this.AdjustLevelUpDown.Maximum = 100;
                    this.AdjustLevelUpDown.Value = 0;
                    this.lblAdjust2.Text = "Highlight:";
                    this.AdjustLevel2.Minimum = -100;
                    this.AdjustLevel2.Maximum = 100;
                    this.AdjustLevel2.Value = 0;
                    this.AdjustLevelUpDown2.Minimum = -100;
                    this.AdjustLevelUpDown2.Maximum = 100;
                    this.AdjustLevelUpDown2.Value = 0;
                    this.Locked = false;
                    break;
                case FilterType.MOTION_BLUR:
                    this.Locked = true;
                    this.lblAdjust.Text = "Distance:";
                    this.AdjustLevel.Minimum = 1;
                    this.AdjustLevel.Maximum = 100;
                    this.AdjustLevel.Value = 5;
                    this.AdjustLevelUpDown.Minimum = 1;
                    this.AdjustLevelUpDown.Maximum = 100;
                    this.AdjustLevelUpDown.Value = 5;
                    this.lblAdjust2.Text = "Angle:";
                    this.AdjustLevel2.Minimum = -180;
                    this.AdjustLevel2.Maximum = 180;
                    this.AdjustLevel2.Value = 30;
                    this.AdjustLevelUpDown2.Minimum = -180;
                    this.AdjustLevelUpDown2.Maximum = 180;
                    this.AdjustLevelUpDown2.Value = 30;
                    this.Locked = false;
                    break;
                default:
                    throw new NotImplementedException();
            }

            this.UpdateCanvas();
        }
    }
}
