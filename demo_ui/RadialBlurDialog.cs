using System.Diagnostics;
using System.Runtime.InteropServices;

namespace OcularDemo
{
    public partial class RadialBlurDialog : Form
    {
        private Canvas canvas;
        private unsafe byte* Clone = (byte*)null;
        private ToolStripStatusLabel TimeUse;
        private int Stride;
        private bool Locked = false;

        [DllImport("Kernel32.dll", EntryPoint = "RtlMoveMemory", SetLastError = true)]
        public static extern unsafe void CopyMemory(void* Dest, void* Src, int Length);

        private int WIDTHBYTES(int bytes) => (bytes * 8 + 31) / 32 * 4;

        public unsafe RadialBlurDialog(Canvas Parent, ToolStripStatusLabel Label)
        {
            InitializeComponent();
            canvas = Parent;
            TimeUse = Label;
            Stride = WIDTHBYTES(canvas.Image.Width * canvas.Channel);
            Clone = (byte*)(void*)Marshal.AllocHGlobal(canvas.Image.Height * Stride);
            CopyMemory((void*)Clone, (void*)Parent.ImageData, canvas.Image.Height * Stride);
        }

        private void convertToCoord(decimal normX, decimal normY, out int x, out int y)
        {
            int centerX = canvas.Image.Width / 2;
            int centerY = canvas.Image.Height / 2;

            x = (int)Math.Round(normX * centerX, 0) + centerX;
            y = (int)Math.Round(normY * centerY, 0) + centerY;
        }

        private unsafe void UpdateCanvas()
        {
            if (Locked)
                return;
            if (chkPreview.Checked)
            {
                Stopwatch stopwatch = new Stopwatch();
                stopwatch.Start();

                int centerX, centerY;
                convertToCoord(udAdjustLevel2.Value, udAdjustLevel3.Value, out centerX, out centerY);
                OcularInterop.ocularRadialBlur(Clone, canvas.ImageData, canvas.Image.Width, canvas.Image.Height, canvas.Stride, centerX, centerY, tbAdjustLevel.Value);

                stopwatch.Stop();
                TimeUse.Text = "Processing time: " + stopwatch.ElapsedMilliseconds.ToString() + "ms";
            }
            else
            {
                CopyMemory((void*)canvas.ImageData, (void*)Clone, canvas.Image.Height * Stride);
            }
            canvas.Refresh();
        }

        private void tbAdjustLevel_Scroll(object sender, EventArgs e)
        {
            udAdjustLevel.Value = (Decimal)tbAdjustLevel.Value;
        }

        private void udAdjustLevel_ValueChanged(object sender, EventArgs e)
        {
            tbAdjustLevel.Value = (int)udAdjustLevel.Value;
            UpdateCanvas();
        }

        private void tbAdjustLevel2_Scroll(object sender, EventArgs e)
        {
            udAdjustLevel2.Value = (Decimal)tbAdjustLevel2.Value / 100;
        }

        private void udAdjustLevel2_ValueChanged(object sender, EventArgs e)
        {
            tbAdjustLevel2.Value = (int)udAdjustLevel2.Value * 100;
            UpdateCanvas();
        }


        private void tbAdjustLevel3_Scroll(object sender, EventArgs e)
        {
            udAdjustLevel3.Value = (Decimal)tbAdjustLevel3.Value / 100;
        }

        private void udAdjustLevel3_ValueChanged(object sender, EventArgs e)
        {
            tbAdjustLevel3.Value = (int)udAdjustLevel3.Value * 100;
            UpdateCanvas();
        }

        private unsafe void RadialBlurDialog_FormClosed(object sender, FormClosedEventArgs e)
        {
            if (DialogResult != DialogResult.OK)
            {
                CopyMemory((void*)canvas.ImageData, (void*)Clone, canvas.Image.Height * Stride);
                canvas.Refresh();
            }
            if ((IntPtr)Clone == IntPtr.Zero)
                return;
            Marshal.FreeHGlobal((IntPtr)(void*)Clone);
        }

        private void chkPreview_CheckedChanged(object sender, EventArgs e)
        {
            UpdateCanvas();
        }

        private unsafe void btnCancel_Click(object sender, EventArgs e)
        {
            CopyMemory((void*)canvas.ImageData, (void*)Clone, canvas.Image.Height * Stride);
            canvas.Refresh();
        }

        private void RadialBlurDialog_Load(object sender, EventArgs e)
        {
            UpdateCanvas();
        }

        private void btnReset_Click(object sender, EventArgs e)
        {

        }
    }
}
