using System.Diagnostics;
using System.Runtime.InteropServices;

namespace OcularDemo
{
    public partial class ThreeParamDialog : Form
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

        public unsafe ThreeParamDialog(Canvas Parent, ToolStripStatusLabel Label, string Caption, FilterType type)
        {
            InitializeComponent();
            this.canvas = Parent;
            this.Text = Caption;
            this.filterType = type;
            this.TimeUse = Label;
            this.Stride = this.WIDTHBYTES(this.canvas.Image.Width * this.canvas.Channel);
            this.Clone = (byte*)(void*)Marshal.AllocHGlobal(this.canvas.Image.Height * this.Stride);
            ThreeParamDialog.CopyMemory((void*)this.Clone, (void*)Parent.ImageData, this.canvas.Image.Height * this.Stride);
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
                    case FilterType.SHARPEN:
                        OcularInterop.ocularSharpenExFilter(this.Clone, this.canvas.ImageData, this.canvas.Image.Width, this.canvas.Image.Height, this.canvas.Stride, (float)this.AdjustLevel.Value, (int)this.AdjustLevel2.Value * 0.01f, this.AdjustLevel3.Value);
                        break;
                    case FilterType.RGB:
                        OcularInterop.ocularRGBFilter(this.Clone, this.canvas.ImageData, this.canvas.Image.Width, this.canvas.Image.Height, this.canvas.Stride, (float)this.AdjustLevel.Value * 0.01f, (float)this.AdjustLevel2.Value * 0.01f, (float)this.AdjustLevel3.Value * 0.01f);
                        break;
                    default:
                        throw new NotImplementedException();
                }

                stopwatch.Stop();
                this.TimeUse.Text = "Processing time: " + stopwatch.ElapsedMilliseconds.ToString() + "ms";
            }
            else
            {
                ThreeParamDialog.CopyMemory((void*)this.canvas.ImageData, (void*)this.Clone, this.canvas.Image.Height * this.Stride);
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

        private void AdjustLevel3_Scroll(object sender, EventArgs e)
        {
            this.AdjustLevelUpDown3.Value = (Decimal)this.AdjustLevel3.Value;
        }

        private void AdjustLevelUpDown3_ValueChanged(object sender, EventArgs e)
        {
            this.AdjustLevel3.Value = (int)this.AdjustLevelUpDown3.Value;
            this.UpdateCanvas();

        }

        private unsafe void ThreeParamDialog_FormClosed(object sender, FormClosedEventArgs e)
        {
            if (this.DialogResult != DialogResult.OK)
            {
                ThreeParamDialog.CopyMemory((void*)this.canvas.ImageData, (void*)this.Clone, this.canvas.Image.Height * this.Stride);
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
            ThreeParamDialog.CopyMemory((void*)this.canvas.ImageData, (void*)this.Clone, this.canvas.Image.Height * this.Stride);
            this.canvas.Refresh();
        }

        private void ThreeParamDialog_Load(object sender, EventArgs e)
        {
            switch (filterType)
            {
                case FilterType.SHARPEN:
                    this.Locked = true;
                    this.lblAdjust.Text = "Radius:";
                    this.AdjustLevel.Minimum = 0;
                    this.AdjustLevel.Maximum = 100;
                    this.AdjustLevel.Value = 5;
                    this.AdjustLevelUpDown.Minimum = 0;
                    this.AdjustLevelUpDown.Maximum = 100;
                    this.AdjustLevelUpDown.Value = 4;
                    this.lblAdjust2.Text = "Sharpness:";
                    this.AdjustLevel2.Minimum = 50;
                    this.AdjustLevel2.Maximum = 500;
                    this.AdjustLevel2.Value = 140;
                    this.AdjustLevelUpDown2.Minimum = 50;
                    this.AdjustLevelUpDown2.Maximum = 500;
                    this.AdjustLevelUpDown2.Value = 140;
                    this.lblAdjust3.Text = "Intensity:";
                    this.AdjustLevel3.Minimum = 0;
                    this.AdjustLevel3.Maximum = 100;
                    this.AdjustLevel3.Value = 20;
                    this.AdjustLevelUpDown3.Minimum = 0;
                    this.AdjustLevelUpDown3.Maximum = 100;
                    this.AdjustLevelUpDown3.Value = 20;
                    this.Locked = false;
                    break;
                case FilterType.RGB:
                    this.Locked = true;
                    this.lblAdjust.Text = "Red (%):";
                    this.AdjustLevel.Minimum = 0;
                    this.AdjustLevel.Maximum = 100;
                    this.AdjustLevel.Value = 100;
                    this.AdjustLevelUpDown.Minimum = 0;
                    this.AdjustLevelUpDown.Maximum = 100;
                    this.AdjustLevelUpDown.Value = 100;
                    this.lblAdjust2.Text = "Green (%):";
                    this.AdjustLevel2.Minimum = 0;
                    this.AdjustLevel2.Maximum = 100;
                    this.AdjustLevel2.Value = 100;
                    this.AdjustLevelUpDown2.Minimum = 0;
                    this.AdjustLevelUpDown2.Maximum = 100;
                    this.AdjustLevelUpDown2.Value = 0;
                    this.lblAdjust3.Text = "Blue (%):";
                    this.AdjustLevel3.Minimum = 0;
                    this.AdjustLevel3.Maximum = 100;
                    this.AdjustLevel3.Value = 100;
                    this.AdjustLevelUpDown3.Minimum = 0;
                    this.AdjustLevelUpDown3.Maximum = 100;
                    this.AdjustLevelUpDown3.Value = 100;
                    this.Locked = false;
                    break;
                default:
                    throw new NotImplementedException();
            }

            this.UpdateCanvas();
        }
    }
}
