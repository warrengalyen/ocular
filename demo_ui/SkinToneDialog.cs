using System.Diagnostics;
using System.Runtime.InteropServices;

namespace OcularDemo
{
    public partial class SkinToneDialog : Form
    {
        private Canvas canvas;
        private unsafe byte* Clone = (byte*)null;
        private ToolStripStatusLabel TimeUse;
        private int Stride;
        private bool Locked = false;

        [DllImport("Kernel32.dll", EntryPoint = "RtlMoveMemory", SetLastError = true)]
        public static extern unsafe void CopyMemory(void* Dest, void* Src, int Length);

        private int WIDTHBYTES(int bytes) => (bytes * 8 + 31) / 32 * 4;

        public unsafe SkinToneDialog(Canvas Parent, ToolStripStatusLabel Label)
        {
            InitializeComponent();
            this.canvas = Parent;
            this.TimeUse = Label;
            this.Stride = this.WIDTHBYTES(this.canvas.Image.Width * this.canvas.Channel);
            this.Clone = (byte*)(void*)Marshal.AllocHGlobal(this.canvas.Image.Height * this.Stride);
            CopyMemory((void*)this.Clone, (void*)Parent.ImageData, this.canvas.Image.Height * this.Stride);
        }

        private unsafe void UpdateCanvas()
        {
            if (this.Locked)
                return;
            if (this.chkPreview.Checked)
            {
                Stopwatch stopwatch = new Stopwatch();
                stopwatch.Start();

                int upperSkinToneColor = (rbGreen.Checked ? 0 : 1);
                OcularInterop.ocularSkinToneFilter(this.Clone, this.canvas.ImageData, this.canvas.Image.Width, this.canvas.Image.Height, 3, this.canvas.Stride,
                    (float)this.tbAmount.Value * 0.01f, (float)this.tbHue.Value * 0.01f, (float)this.tbHueThreshold.Value, (float)this.tbSatShift.Value * 0.01f, upperSkinToneColor);

                stopwatch.Stop();
                this.TimeUse.Text = "Processing time: " + stopwatch.ElapsedMilliseconds.ToString() + "ms";
            }
            else
            {
                CopyMemory((void*)this.canvas.ImageData, (void*)this.Clone, this.canvas.Image.Height * this.Stride);
            }
            this.canvas.Refresh();
        }

        private void SkinToneDialog_Load(object sender, EventArgs e)
        {
            this.UpdateCanvas();
        }

        private unsafe void SkinToneDialog_FormClosed(object sender, FormClosedEventArgs e)
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

        private void chkPreview_CheckedChanged(object sender, EventArgs e)
        {
            this.UpdateCanvas();
        }

        private unsafe void btnCancel_Click(object sender, EventArgs e)
        {
            CopyMemory((void*)this.canvas.ImageData, (void*)this.Clone, this.canvas.Image.Height * this.Stride);
            this.canvas.Refresh();
        }

        private void btnReset_Click(object sender, EventArgs e)
        {
            tbAmount.Value = 0;
            tbHue.Value = 5;
            tbHueThreshold.Value = 40;
            tbSatShift.Value = 25;
            rbGreen.Checked = true;
        }

        private void tbAmount_Scroll(object sender, EventArgs e)
        {
            this.udAmount.Value = tbAmount.Value;
        }

        private void udAmount_ValueChanged(object sender, EventArgs e)
        {
            this.tbAmount.Value = (int)this.udAmount.Value;
            this.UpdateCanvas();
        }

        private void tbHue_Scroll(object sender, EventArgs e)
        {
            this.udHue.Value = tbHue.Value;
        }

        private void udHue_ValueChanged(object sender, EventArgs e)
        {
            this.tbHue.Value = (int)this.udHue.Value;
            this.UpdateCanvas();
        }

        private void tbHueThreshold_Scroll(object sender, EventArgs e)
        {
            this.udHueThreshold.Value = this.tbHueThreshold.Value;
        }

        private void udHueThreshold_ValueChanged(object sender, EventArgs e)
        {
            this.tbHueThreshold.Value = (int)this.udHueThreshold.Value;
            this.UpdateCanvas();
        }

        private void tbSatShift_Scroll(object sender, EventArgs e)
        {
            this.udSatShift.Value = this.tbSatShift.Value;
        }

        private void udSatShift_ValueChanged(object sender, EventArgs e)
        {
            this.tbSatShift.Value = (int)this.udSatShift.Value;
            this.UpdateCanvas();
        }

        private void rbGreen_CheckedChanged(object sender, EventArgs e)
        {
            if (this.rbGreen.Checked) { this.UpdateCanvas(); }
        }

        private void rbOrange_CheckedChanged(object sender, EventArgs e)
        {
            if (!this.rbOrange.Checked) { this.UpdateCanvas(); }
        }
    }
}
