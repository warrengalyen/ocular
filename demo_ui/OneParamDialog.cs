using System.Diagnostics;
using System.Runtime.InteropServices;

namespace OcularDemo
{
    public partial class OneParamDialog : Form
    {
        private Canvas canvas;
        private unsafe byte* Clone = (byte*)null;
        private ToolStripStatusLabel TimeUse;
        private int Stride;
        private bool Locked;
        private FilterType filterType;

        [DllImport("Kernel32.dll", EntryPoint = "RtlMoveMemory", SetLastError = true)]
        public static extern unsafe void CopyMemory(void* Dest, void* Src, int Length);

        private int WIDTHBYTES(int bytes) => (bytes * 8 + 31) / 32 * 4;

        public unsafe OneParamDialog(Canvas Parent, ToolStripStatusLabel Label, string Caption, FilterType type)
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
            if (this.Locked)
                return;
            if (this.ChkPreview.Checked)
            {
                Stopwatch stopwatch = new Stopwatch();
                stopwatch.Start();

                switch (filterType)
                {
                    case FilterType.AUTO_LEVEL:
                        // Normally you would not require a parameter for a auto color adjustment filter. The cut limit would be made adjusted as a global value like Photoshop/PaintShop Pro does with it's clipping values.
                        OcularInterop.ocularAutoLevel(this.Clone, this.canvas.ImageData, this.canvas.Image.Width, this.canvas.Image.Height, this.canvas.Stride, (float)this.AdjustLevel.Value * 0.0001f);
                        break;
                    case FilterType.EXPOSURE:
                        OcularInterop.ocularExposureFilter(this.Clone, this.canvas.ImageData, this.canvas.Image.Width, this.canvas.Image.Height, this.canvas.Stride, (float)this.AdjustLevel.Value * 0.01f);
                        break;
                    case FilterType.VIBRANCE:
                        OcularInterop.ocularVibranceFilter(this.Clone, this.canvas.ImageData, this.canvas.Image.Width, this.canvas.Image.Height, this.canvas.Stride, (float)this.AdjustLevel.Value * 0.01f);
                        break;
                    case FilterType.THRESHOLD:
                        OcularInterop.ocularLuminanceThresholdFilter(this.Clone, this.canvas.ImageData, this.canvas.Image.Width, this.canvas.Image.Height, this.canvas.Stride, (byte)this.AdjustLevel.Value);
                        break;
                    case FilterType.GAUSSIAN_BLUR:
                        OcularInterop.ocularGaussianBlurFilter(this.Clone, this.canvas.ImageData, this.canvas.Image.Width, this.canvas.Image.Height, this.canvas.Stride, (float)this.AdjustLevel.Value);
                        break;
                    case FilterType.BOX_BLUR:
                        OcularInterop.ocularBoxBlurFilter(this.Clone, this.canvas.ImageData, this.canvas.Image.Width, this.canvas.Image.Height, this.canvas.Stride, this.AdjustLevel.Value);
                        break;
                    case FilterType.SEPIA:
                        OcularInterop.ocularSepiaFilter(this.Clone, this.canvas.ImageData, this.canvas.Image.Width, this.canvas.Image.Height, this.canvas.Stride, this.AdjustLevel.Value);
                        break;
                    case FilterType.SATURATION:
                        OcularInterop.ocularSaturationFilter(this.Clone, this.canvas.ImageData, this.canvas.Image.Width, this.canvas.Image.Height, this.canvas.Stride, (float)this.AdjustLevel.Value * 0.01f);
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

        private unsafe void OneParamDialog_FormClosed(object sender, FormClosedEventArgs e)
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

        private void OneParamDialog_Load(object sender, EventArgs e)
        {
            switch (filterType)
            {
                case FilterType.AUTO_LEVEL:
                    this.Locked = true;
                    this.lblAdjust.Text = "Clip Limit:";
                    this.AdjustLevel.Minimum = 1;
                    this.AdjustLevel.Maximum = 100;
                    this.AdjustLevel.Value = 16;
                    this.AdjustLevelUpDown.Minimum = 1;
                    this.AdjustLevelUpDown.Maximum = 100;
                    this.AdjustLevelUpDown.Value = 16;
                    this.Locked = false;
                    break;
                case FilterType.EXPOSURE:
                    this.Locked = true;
                    this.lblAdjust.Text = "Amount:";
                    this.AdjustLevel.Minimum = -100;
                    this.AdjustLevel.Maximum = 100;
                    this.AdjustLevel.Value = 0;
                    this.AdjustLevelUpDown.Minimum = -100;
                    this.AdjustLevelUpDown.Maximum = 100;
                    this.AdjustLevelUpDown.Value = 0;
                    this.Locked = false;
                    break;
                case FilterType.VIBRANCE:
                    this.Locked = true;
                    this.lblAdjust.Text = "Strength:";
                    this.AdjustLevel.Minimum = -100;
                    this.AdjustLevel.Maximum = 100;
                    this.AdjustLevel.Value = 0;
                    this.AdjustLevelUpDown.Minimum = -100;
                    this.AdjustLevelUpDown.Maximum = 100;
                    this.AdjustLevelUpDown.Value = 0;
                    this.Locked = false;
                    break;
                case FilterType.THRESHOLD:
                    this.Locked = true;
                    this.lblAdjust.Text = "Threshold:";
                    this.AdjustLevel.Minimum = 1;
                    this.AdjustLevel.Maximum = 255;
                    this.AdjustLevel.Value = 128;
                    this.AdjustLevelUpDown.Minimum = 1;
                    this.AdjustLevelUpDown.Maximum = 255;
                    this.AdjustLevelUpDown.Value = 128;
                    this.Locked = false;
                    break;
                case FilterType.GAUSSIAN_BLUR:
                    this.Locked = true;
                    this.lblAdjust.Text = "Radius:";
                    this.AdjustLevel.Minimum = 0;
                    this.AdjustLevel.Maximum = 100;
                    this.AdjustLevel.Value = 5;
                    this.AdjustLevelUpDown.Minimum = 0;
                    this.AdjustLevelUpDown.Maximum = 100;
                    this.AdjustLevelUpDown.Value = 5;
                    this.Locked = false;
                    break;
                case FilterType.BOX_BLUR:
                    this.Locked = true;
                    this.lblAdjust.Text = "Radius:";
                    this.AdjustLevel.Minimum = 1;
                    this.AdjustLevel.Maximum = 100;
                    this.AdjustLevel.Value = 2;
                    this.AdjustLevelUpDown.Minimum = 1;
                    this.AdjustLevelUpDown.Maximum = 100;
                    this.AdjustLevelUpDown.Value = 2;
                    this.Locked = false;
                    break;
                case FilterType.SEPIA:
                    this.Locked = true;
                    this.lblAdjust.Text = "Amount to age:";
                    this.AdjustLevel.Minimum = 0;
                    this.AdjustLevel.Maximum = 100;
                    this.AdjustLevel.Value = 60;
                    this.AdjustLevelUpDown.Minimum = 0;
                    this.AdjustLevelUpDown.Maximum = 100;
                    this.AdjustLevelUpDown.Value = 60;
                    this.Locked = false;
                    break;
                case FilterType.SATURATION:
                    this.Locked = true;
                    this.lblAdjust.Text = "Amount:";
                    this.AdjustLevel.Minimum = -100;
                    this.AdjustLevel.Maximum = 100;
                    this.AdjustLevel.Value = 0;
                    this.AdjustLevelUpDown.Minimum = -100;
                    this.AdjustLevelUpDown.Maximum = 100;
                    this.AdjustLevelUpDown.Value = 0;
                    this.Locked = false;
                    break;
                default:
                    throw new NotImplementedException();
            }

            this.UpdateCanvas();
        }
    }
}
