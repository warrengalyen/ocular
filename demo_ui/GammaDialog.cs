using System.Diagnostics;
using System.Drawing.Drawing2D;
using System.Runtime.InteropServices;

namespace OcularDemo
{
    public partial class GammaDialog : Form
    {
        private Canvas canvas;
        private unsafe byte* Clone = (byte*)null;
        private ToolStripStatusLabel TimeUse;
        private int Stride;
        private bool Locked;

        [DllImport("Kernel32.dll", EntryPoint = "RtlMoveMemory", SetLastError = true)]
        public static extern unsafe void CopyMemory(void* Dest, void* Src, int Length);

        private int WIDTHBYTES(int bytes) => (bytes * 8 + 31) / 32 * 4;

        public unsafe GammaDialog(Canvas Parent, ToolStripStatusLabel Label)
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

                float[] gammaValues = new float[3];
                gammaValues[0] = (float)this.udGammaRed.Value;
                gammaValues[1] = (float)this.udGammaGreen.Value;
                gammaValues[2] = (float)this.udGammaBlue.Value;
                OcularInterop.ocularGammaFilter(this.Clone, this.canvas.ImageData, this.canvas.Image.Width, this.canvas.Image.Height, this.canvas.Stride, gammaValues);

                stopwatch.Stop();
                this.TimeUse.Text = "Processing time: " + stopwatch.ElapsedMilliseconds.ToString() + "ms";
            }
            else
            {
                CopyMemory((void*)this.canvas.ImageData, (void*)this.Clone, this.canvas.Image.Height * this.Stride);
            }
            this.canvas.Refresh();
        }

        private void tbGamma_Scroll(object sender, EventArgs e)
        {
            this.Locked = true;
            if (chkSync.Checked)
            {
                switch ((sender as TrackBar).Tag)
                {
                    case "0":
                        tbGammaGreen.Value = tbGammaRed.Value;
                        tbGammaBlue.Value = tbGammaRed.Value;
                        break;
                    case "1":
                        tbGammaRed.Value = tbGammaGreen.Value;
                        tbGammaBlue.Value = tbGammaGreen.Value;
                        break;
                    case "2":
                        tbGammaRed.Value = tbGammaBlue.Value;
                        tbGammaGreen.Value = tbGammaBlue.Value;
                        break;
                    default:
                        break;
                }

                this.udGammaRed.Value = (Decimal)(System.Math.Clamp(this.tbGammaRed.Value * 0.01f, 0.0, 3.0));
                this.udGammaGreen.Value = (Decimal)(System.Math.Clamp(this.tbGammaGreen.Value * 0.01f, 0.0, 3.0));
                this.udGammaBlue.Value = (Decimal)(System.Math.Clamp(this.tbGammaBlue.Value * 0.01f, 0.0, 3.0));
                picChart.Refresh();
            }
            else
            {
                this.udGammaRed.Value = (Decimal)(System.Math.Clamp(this.tbGammaRed.Value * 0.01f, 0.0, 3.0));
                this.udGammaGreen.Value = (Decimal)(System.Math.Clamp(this.tbGammaGreen.Value * 0.01f, 0.0, 3.0));
                this.udGammaBlue.Value = (Decimal)(System.Math.Clamp(this.tbGammaBlue.Value * 0.01f, 0.0, 3.0));
                picChart.Refresh();
            }
            this.Locked = false;
            UpdateCanvas();
        }

        private void udGamma_ValueChanged(object sender, EventArgs e)
        {
            //this.tbGammaRed.Value = (int)(System.Math.Clamp(this.udGammaRed.Value * 100, 0, 300));
            UpdateCanvas();
        }

        private void DrawGammaCurve(Graphics g, Color color, float gamma)
        {
            Pen pen = new Pen(color, 1.6f);
            PointF[] points = new PointF[picChart.Width];

            for (int x = 0; x < picChart.Width; x++)
            {
                float normalizedValue = x / (float)picChart.Width;
                float gammaCorrectedValue = (float)Math.Pow(normalizedValue, gamma);
                int y = (int)(gammaCorrectedValue * picChart.Height);
                points[x] = new PointF(x, picChart.Height - y);
            }

            g.SmoothingMode = SmoothingMode.HighQuality;
            g.PixelOffsetMode = PixelOffsetMode.Half;
            g.DrawLines(pen, points);
        }

        private void GammaDialog_Load(object sender, EventArgs e)
        {
            this.UpdateCanvas();
            picChart.Refresh();
        }

        private unsafe void GammaDialog_FormClosed(object sender, FormClosedEventArgs e)
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
            tbGammaRed.Value = 100;
            tbGammaGreen.Value = 100;
            tbGammaBlue.Value = 100;
        }

        private void chkSync_CheckedChanged(object sender, EventArgs e)
        {
            if (chkSync.Checked)
            {
                float newGamma = ((float)udGammaRed.Value + (float)udGammaGreen.Value + (float)(udGammaBlue.Value) / 3);

                this.Locked = true;
                tbGammaRed.Value = (int)(System.Math.Clamp(newGamma * 100, 0, 300));
                tbGammaGreen.Value = (int)(System.Math.Clamp(newGamma * 100, 0, 300));
                tbGammaBlue.Value = (int)(System.Math.Clamp(newGamma * 100, 0, 300));
                this.Locked = false;
            }

            UpdateCanvas();
        }

        private void picChart_Paint(object sender, PaintEventArgs e)
        {
            Graphics g = e.Graphics;

            if (chkSync.Checked)
            {
                // Draw gray reference line
                Pen grayPen = new Pen(Color.Gray, 1);
                g.SmoothingMode = SmoothingMode.None;
                g.DrawLine(grayPen, 0, picChart.Height - 2, picChart.Width - 2, 0);

                // If all channels are synced, their curves will overlap so don't waste
                // time drawing each channel, only draw blue.
                DrawGammaCurve(g, Color.Blue, (float)udGammaBlue.Value);
            }
            else
            {
                // Draw gray reference line
                Pen grayPen = new Pen(Color.Gray, 1);
                g.SmoothingMode = SmoothingMode.None;
                g.DrawLine(grayPen, 0, picChart.Height - 2, picChart.Width - 2, 0);

                DrawGammaCurve(g, Color.Red, (float)udGammaRed.Value);
                DrawGammaCurve(g, Color.Green, (float)udGammaGreen.Value);
                DrawGammaCurve(g, Color.Blue, (float)udGammaBlue.Value);
            }
        }
    }
}
