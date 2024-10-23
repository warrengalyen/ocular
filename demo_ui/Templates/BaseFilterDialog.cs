using System.Diagnostics;
using System.Runtime.InteropServices;

namespace OcularDemo
{
    public partial class BaseFilterDialog : Form
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

        public unsafe BaseFilterDialog(Canvas Parent, ToolStripStatusLabel Label, string Caption, FilterType type)
        {
            InitializeComponent();
            canvas = Parent;
            Text = Caption;
            filterType = type;
            TimeUse = Label;
            Stride = WIDTHBYTES(canvas.Image.Width * canvas.Channel);
            Clone = (byte*)(void*)Marshal.AllocHGlobal(canvas.Image.Height * Stride);
            CopyMemory((void*)Clone, (void*)Parent.ImageData, canvas.Image.Height * Stride);
        }

        private unsafe void UpdateCanvas()
        {
            if (Locked)
                return;
            if (chkPreview.Checked)
            {
                Stopwatch stopwatch = new Stopwatch();
                stopwatch.Start();



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
            udAdjustLevel2.Value = (Decimal)tbAdjustLevel2.Value;
        }

        private void udAdjustLevel2_ValueChanged(object sender, EventArgs e)
        {
            tbAdjustLevel2.Value = (int)udAdjustLevel2.Value;
            UpdateCanvas();

        }

        private unsafe void BaseFilterDialog_FormClosed(object sender, FormClosedEventArgs e)
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

        private void BaseFilterDialog_Load(object sender, EventArgs e)
        {
            UpdateCanvas();
        }

        private void btnReset_Click(object sender, EventArgs e)
        {
            tbAdjustLevel.Value = 0;
            tbAdjustLevel2.Value = 0;
        }
    }
}
