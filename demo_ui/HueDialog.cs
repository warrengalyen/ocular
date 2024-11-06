using System.Diagnostics;
using System.Runtime.InteropServices;

namespace OcularDemo
{
    public partial class HueDialog : Form
    {
        private Canvas canvas;
        private unsafe byte* Clone = (byte*)null;
        private ToolStripStatusLabel TimeUse;
        private int Stride;

        public unsafe HueDialog(Canvas Parent, ToolStripStatusLabel Label)
        {
            InitializeComponent();
            canvas = Parent;
            TimeUse = Label;
            Stride = Utility.WIDTHBYTES(canvas.Image.Width * canvas.Channel);
            Clone = (byte*)(void*)Marshal.AllocHGlobal(canvas.Image.Height * Stride);
            OcularInterop.CopyMemory((void*)Clone, (void*)Parent.ImageData, canvas.Image.Height * Stride);
        }

        private unsafe void UpdateCanvas()
        {
            if (ChkPreview.Checked)
            {
                Stopwatch stopwatch = new Stopwatch();
                stopwatch.Start();
                OcularInterop.ocularHueFilter(Clone, canvas.ImageData, canvas.Image.Width, canvas.Image.Height, canvas.Stride, (float)HueUpDown.Value);
                stopwatch.Stop();
            }
        }

        private void HueUpDown_ValueChanged(object sender, EventArgs e)
        {
            huePicker.Value = Convert.ToInt32(HueUpDown.Value);
            UpdateCanvas();
        }

        private void huePicker_ValuesChanged(object sender, EventArgs e)
        {
            HueUpDown.Value = huePicker.Value;
        }

        private unsafe void btnCancel_Click(object sender, EventArgs e)
        {
            OcularInterop.CopyMemory((void*)canvas.ImageData, (void*)Clone, canvas.Image.Height * Stride);
            canvas.Refresh();
        }

        private void ChkPreview_CheckedChanged(object sender, EventArgs e)
        {
            UpdateCanvas();
        }

        private unsafe void HueDialog_FormClosed(object sender, FormClosedEventArgs e)
        {
            if (DialogResult != DialogResult.OK)
            {
                OcularInterop.CopyMemory((void*)canvas.ImageData, (void*)Clone, canvas.Image.Height * Stride);
                canvas.Refresh();
            }
            if ((IntPtr)Clone == IntPtr.Zero)
                return;
            Marshal.FreeHGlobal((IntPtr)(void*)Clone);
        }
    }
}
