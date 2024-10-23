using System.Drawing.Imaging;
using System.Runtime.InteropServices;

namespace OcularDemo
{
    public partial class FormMain : Form
    {

        [DllImport("kernel32.dll ")]
        private static extern bool QueryPerformanceCounter(ref long lpPerformanceCount);

        [DllImport("kernel32")]
        private static extern bool QueryPerformanceFrequency(ref long PerformanceFrequency);

        public FormMain()
        {
            InitializeComponent();
        }

        private void SetChannelMenuStatus()
        {
            this.MenuGrayMode.Checked = false;
            this.MenuBGR.Checked = false;
            this.MenuBGRA.Checked = false;
            if (this.MainCanvas.Channel == 1)
            {
                this.MenuGrayMode.Checked = true;
                this.ChannelStatus.Text = "GrayMode";
            }
            else if (this.MainCanvas.Channel == 3)
            {
                this.MenuBGR.Checked = true;
                this.ChannelStatus.Text = "BGR - 3 Channel";
            }
            else
            {
                if (this.MainCanvas.Channel != 4)
                    return;
                this.MenuBGRA.Checked = true;
                this.ChannelStatus.Text = "BGR - 4 Channel";
            }
        }

        private void FormMain_Load(object sender, EventArgs e)
        {
            this.SetChannelMenuStatus();
            Application.DoEvents();
            this.MainCanvas_Resize(sender, e);
        }

        private void MenuLoad_Click(object sender, EventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.Filter = "All files(*.*) |*.*|Bitmap files (*.Bitmap)|*.Bmp|Jpeg files (*.jpg)|*.jpg|Png files (*.png)|*.png |Tiff files (*.tif)|*.tif";
            if (ofd.ShowDialog() != DialogResult.OK)
                return;
            if (!this.MainCanvas.Visible)
                this.MainCanvas.Visible = true;
            this.MainCanvas.ImageFile = ofd.FileName;
            this.FilePathStatus.Text = ofd.FileName;
            this.SetChannelMenuStatus();
            Application.DoEvents();
            this.MainCanvas_Resize(sender, e);
            float CenterX = 0.0f;
            float CenterY = 0.0f;

            //this.TimeUse.Text = "Gravity center of the image: (" + CenterX.ToString() + ", " + CenterY.ToString() + ")";
        }


        private void MainCanvas_Resize(object sender, EventArgs e)
        {
            if (!this.MainCanvas.Visible)
                return;
            int width1 = this.MainCanvas.Width;
            int height1 = this.MainCanvas.Height;
            int width2 = this.Container.ContentPanel.Width;
            int height2 = this.Container.ContentPanel.Height;
            if (width1 > width2)
            {
                this.ScrollH.Visible = true;
                this.ScrollH.Minimum = 0;
                this.ScrollH.Maximum = width1;
                this.ScrollH.Value = (this.ScrollH.Maximum + this.ScrollH.Minimum) / 2;
            }
            else
                this.ScrollH.Visible = false;
            if (height1 > height2)
            {
                this.ScrollV.Visible = true;
                this.ScrollV.Minimum = 0;
                this.ScrollV.Maximum = height1;
                this.ScrollV.Value = (this.ScrollV.Maximum + this.ScrollV.Minimum) / 2;
            }
            else
                this.ScrollV.Visible = false;
            Application.DoEvents();
            this.MainCanvas.Left = (width2 - width1) / 2;
            this.MainCanvas.Top = (height2 - height1) / 2;
            if (this.MainCanvas.Image == null)
                return;
            this.sizeToolStripStatusLabel.Text = this.MainCanvas.Image.Width.ToString() + "×" + this.MainCanvas.Image.Height.ToString();
        }

        private void Canvas_MouseWheel(object sender, MouseEventArgs e)
        {
            int num = (this.ScrollV.Maximum - this.ScrollV.Minimum) / 10;
            if (!this.ScrollV.Visible)
                return;
            if (e.Delta < 0)
            {
                if (this.ScrollV.Value + num > this.ScrollV.Maximum)
                    this.ScrollV.Value = this.ScrollV.Maximum;
                else
                    this.ScrollV.Value += num;
            }
            else if (this.ScrollV.Value - num < this.ScrollV.Minimum)
                this.ScrollV.Value = this.ScrollV.Minimum;
            else
                this.ScrollV.Value -= num;
            this.MainCanvas.Top = (this.Container.ContentPanel.Height - this.MainCanvas.Height) * this.ScrollV.Value / this.MainCanvas.Height;
        }

        private void ScrollH_Scroll(object sender, ScrollEventArgs e) => this.MainCanvas.Left = (this.Container.ContentPanel.Width - this.MainCanvas.Width) * this.ScrollH.Value / this.MainCanvas.Width;

        private void ScrollV_Scroll(object sender, ScrollEventArgs e) => this.MainCanvas.Top = (this.Container.ContentPanel.Height - this.MainCanvas.Height) * this.ScrollV.Value / this.MainCanvas.Height;

        private int GetPosInImage(int Pos) => (int)((double)Pos / this.MainCanvas.Zoom + 0.5);

        private unsafe void MainCanvas_MouseMove(object sender, MouseEventArgs e)
        {
            if (this.MainCanvas.Image == null)
                return;
            int posInImage1 = this.GetPosInImage(e.X);
            int posInImage2 = this.GetPosInImage(e.Y);
            if (posInImage1 < 0 || posInImage1 >= this.MainCanvas.Image.Width || posInImage2 < 0 || posInImage2 >= this.MainCanvas.Image.Height)
                return;
            byte* numPtr = this.MainCanvas.ImageData + posInImage2 * this.MainCanvas.Stride + posInImage1 * this.MainCanvas.Channel;
            if (this.MainCanvas.Channel == 1)
                this.mouseToolStripStatusLabel.Text = "X:" + posInImage1.ToString() + ", Y:" + posInImage2.ToString() + "   R = " + numPtr->ToString() + ", G = " + numPtr->ToString() + ", B = " + numPtr->ToString();
            else
                this.mouseToolStripStatusLabel.Text = "X:" + posInImage1.ToString() + ", Y:" + posInImage2.ToString() + "   R = " + numPtr[2].ToString() + ", G = " + numPtr[1].ToString() + ", B = " + numPtr->ToString();
        }


        private void ZoomCanvas(int iZoom)
        {
            this.MenuZoom12.Checked = false;
            this.MenuZoom25.Checked = false;
            this.MenuZoom50.Checked = false;
            this.MenuZoom100.Checked = false;
            this.MenuZoom200.Checked = false;
            this.MenuZoom300.Checked = false;
            this.MenuZoom400.Checked = false;
            this.MenuZoom500.Checked = false;
            this.MenuZoom1000.Checked = false;
            switch (iZoom)
            {
                case 12:
                    this.MenuZoom12.Checked = true;
                    this.MainCanvas.Zoom = 0.12;
                    this.ZoomFactor.Text = "12%";
                    break;
                case 25:
                    this.MenuZoom400.Checked = true;
                    this.MainCanvas.Zoom = 0.25;
                    this.ZoomFactor.Text = "25%";
                    break;
                case 50:
                    this.MenuZoom500.Checked = true;
                    this.MainCanvas.Zoom = 0.5;
                    this.ZoomFactor.Text = "50%";
                    break;
                case 100:
                    this.MenuZoom100.Checked = true;
                    this.MainCanvas.Zoom = 1.0;
                    this.ZoomFactor.Text = "100%";
                    break;
                case 200:
                    this.MenuZoom200.Checked = true;
                    this.MainCanvas.Zoom = 2.0;
                    this.ZoomFactor.Text = "200%";
                    break;
                case 300:
                    this.MenuZoom12.Checked = true;
                    this.MainCanvas.Zoom = 3.0;
                    this.ZoomFactor.Text = "300%";
                    break;
                case 400:
                    this.MenuZoom400.Checked = true;
                    this.MainCanvas.Zoom = 4.0;
                    this.ZoomFactor.Text = "400%";
                    break;
                case 500:
                    this.MenuZoom400.Checked = true;
                    this.MainCanvas.Zoom = 5.0;
                    this.ZoomFactor.Text = "500%";
                    break;
                case 1000:
                    this.MenuZoom500.Checked = true;
                    this.MainCanvas.Zoom = 10.0;
                    this.ZoomFactor.Text = "1000%";
                    break;
            }
        }

        private void MenuZoom12_Click(object sender, EventArgs e)
        {
            this.ZoomCanvas(12);
            Application.DoEvents();
            this.MainCanvas_Resize(sender, e);
        }

        private void MenuZoom25_Click(object sender, EventArgs e)
        {
            this.ZoomCanvas(25);
            Application.DoEvents();
            this.MainCanvas_Resize(sender, e);
        }

        private void MenuZoom50_Click(object sender, EventArgs e)
        {
            this.ZoomCanvas(50);
            Application.DoEvents();
            this.MainCanvas_Resize(sender, e);
        }

        private void MenuZoom100_Click(object sender, EventArgs e)
        {
            this.ZoomCanvas(100);
            Application.DoEvents();
            this.MainCanvas_Resize(sender, e);
        }

        private void MenuZoom200_Click(object sender, EventArgs e)
        {
            this.ZoomCanvas(200);
            Application.DoEvents();
            this.MainCanvas_Resize(sender, e);
        }

        private void MenuZoom300_Click(object sender, EventArgs e)
        {
            this.ZoomCanvas(300);
            Application.DoEvents();
            this.MainCanvas_Resize(sender, e);
        }

        private void MenuZoom400_Click(object sender, EventArgs e)
        {
            this.ZoomCanvas(400);
            Application.DoEvents();
            this.MainCanvas_Resize(sender, e);
        }

        private void MenuZoom500_Click(object sender, EventArgs e)
        {
            this.ZoomCanvas(500);
            Application.DoEvents();
            this.MainCanvas_Resize(sender, e);
        }

        private void MenuZoom1000_Click(object sender, EventArgs e)
        {
            this.ZoomCanvas(1000);
            Application.DoEvents();
            this.MainCanvas_Resize(sender, e);
        }

        private void MenuGrayMode_Click(object sender, EventArgs e)
        {
            this.MainCanvas.Channel = 1;
            this.SetChannelMenuStatus();
        }

        private void MenuBGR_Click(object sender, EventArgs e)
        {
            this.MainCanvas.Channel = 3;
            this.SetChannelMenuStatus();
        }

        private void MenuBGRA_Click(object sender, EventArgs e)
        {
            this.MainCanvas.Channel = 4;
            this.SetChannelMenuStatus();
        }

        private void MenuExit_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void MenuExposure_Click(object sender, EventArgs e)
        {
            if (this.MainCanvas.Image == null)
                return;

            new OneParamDialog(this.MainCanvas, this.TimeUse, "Exposure", FilterType.EXPOSURE).ShowDialog(this);
        }

        private void MenuVibrance_Click(object sender, EventArgs e)
        {
            if (this.MainCanvas.Image == null)
                return;

            new OneParamDialog(this.MainCanvas, this.TimeUse, "Vibrance", FilterType.VIBRANCE).ShowDialog(this);
        }

        private void MenuAdjustBrightnessAndContrast_Click(object sender, EventArgs e)
        {
            if (this.MainCanvas.Image == null)
                return;

            new TwoParamDialog(this.MainCanvas, this.TimeUse, "Brightness/Contrast", FilterType.BRIGHTNESS_CONTRAST).ShowDialog(this);
        }

        private void MenuWhiteBalance_Click(object sender, EventArgs e)
        {
            if (this.MainCanvas.Image == null)
                return;

            new TwoParamDialog(this.MainCanvas, this.TimeUse, "White Balance", FilterType.WHITE_BALANCE).ShowDialog(this);
        }

        private void MenuThreshold_Click(object sender, EventArgs e)
        {
            if (this.MainCanvas.Image == null)
                return;

            new OneParamDialog(this.MainCanvas, this.TimeUse, "Threshold", FilterType.THRESHOLD).ShowDialog(this);
        }

        private void MenuGaussianBlur_Click(object sender, EventArgs e)
        {
            if (this.MainCanvas.Image == null)
                return;

            new OneParamDialog(this.MainCanvas, this.TimeUse, "Gaussian Blur", FilterType.GAUSSIAN_BLUR).ShowDialog(this);
        }

        private void MenuBoxBlur_Click(object sender, EventArgs e)
        {
            if (this.MainCanvas.Image == null)
                return;

            new OneParamDialog(this.MainCanvas, this.TimeUse, "Box Blur", FilterType.BOX_BLUR).ShowDialog(this);
        }

        private void MenuBilateral_Click(object sender, EventArgs e)
        {
            if (this.MainCanvas.Image == null)
                return;

            new TwoParamDialog(this.MainCanvas, this.TimeUse, "Bilateral", FilterType.BILATERAL).ShowDialog(this);
        }

        private void MenuResample_Click(object sender, EventArgs e)
        {
            if (this.MainCanvas.Image == null)
                return;
            new ResampleDialog(this.MainCanvas, this.TimeUse, "Resize").ShowDialog(this);
        }

        private void MenuBasicSharpen_Click(object sender, EventArgs e)
        {
            if (this.MainCanvas.Image == null)
                return;

            new ThreeParamDialog(this.MainCanvas, this.TimeUse, "Sharpen", FilterType.SHARPEN).ShowDialog(this);
        }

        private void MenuUnsharpMask_Click(object sender, EventArgs e)
        {
            if (this.MainCanvas.Image == null)
                return;

            new TwoParamDialog(this.MainCanvas, this.TimeUse, "Unsharp Mask", FilterType.UNSHARP_MASK).ShowDialog(this);
        }

        private void MenuRGB_Click(object sender, EventArgs e)
        {
            if (this.MainCanvas.Image == null)
                return;

            new ThreeParamDialog(this.MainCanvas, this.TimeUse, "Red/Green/Blue", FilterType.RGB).ShowDialog(this);
        }

        private void MenuSepia_Click(object sender, EventArgs e)
        {
            if (this.MainCanvas.Image == null)
                return;

            new OneParamDialog(this.MainCanvas, this.TimeUse, "Sepia Toning", FilterType.SEPIA).ShowDialog(this);
        }

        private unsafe void MenuLut2D_Click(object sender, EventArgs e)
        {
            if (this.MainCanvas.Image == null)
                return;
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.Filter = "All files(*.*) |*.*|Bitmap files (*.Bitmap)|*.Bmp|Jpeg files (*.jpg)|*.jpg|Png files (*.png)|*.png";
            if (ofd.ShowDialog() != DialogResult.OK)
                return;
            Bitmap bitmap = (Bitmap)Image.FromFile(ofd.FileName);
            if (bitmap.Width == 512 && bitmap.Height == 512 && bitmap.PixelFormat == PixelFormat.Format24bppRgb)
            {
                BitmapData bitmapData = bitmap.LockBits(new Rectangle(0, 0, bitmap.Width, bitmap.Height), ImageLockMode.ReadWrite, bitmap.PixelFormat);
                long PerformanceFrequency = 0;
                long lpPerformanceCount1 = 0;
                long lpPerformanceCount2 = 0;
                FormMain.QueryPerformanceFrequency(ref PerformanceFrequency);
                FormMain.QueryPerformanceCounter(ref lpPerformanceCount1);
                OcularInterop.ocularLookupFilter(this.MainCanvas.ImageData, this.MainCanvas.ImageData, (byte*)(void*)bitmapData.Scan0, this.MainCanvas.Width, this.MainCanvas.Height, this.MainCanvas.Stride, 100.0f);
                FormMain.QueryPerformanceCounter(ref lpPerformanceCount2);
                this.TimeUse.Text = "Processing time: " + Math.Round((double)((lpPerformanceCount2 - lpPerformanceCount1) * 1000L) / (double)PerformanceFrequency, 2).ToString() + "ms.";
                bitmap.UnlockBits(bitmapData);
                this.MainCanvas.Refresh();
            }
            else
            {
                MessageBox.Show("The lookup image must be 512x512, 24-bit color.\nSee the Resources->lookup folder for examples.");
            }
            bitmap.Dispose();
        }

        private unsafe void MenuInvert_Click(object sender, EventArgs e)
        {
            if (this.MainCanvas.Image == null)
                return;
            OcularInterop.ocularColorInvertFilter(this.MainCanvas.ImageData, this.MainCanvas.ImageData, this.MainCanvas.Width, this.MainCanvas.Height, this.MainCanvas.Stride);
            this.MainCanvas.Refresh();
        }

        private unsafe void MenuDesaturate_Click(object sender, EventArgs e)
        {
            if (this.MainCanvas.Image == null)
                return;
            OcularInterop.ocularGrayscaleFilter(this.MainCanvas.ImageData, this.MainCanvas.ImageData, this.MainCanvas.Width, this.MainCanvas.Height, this.MainCanvas.Stride);
            this.MainCanvas.Channel = 1;
            this.ChannelStatus.Text = "Grayscale";
            this.MainCanvas.Refresh();
        }

        private void MenuAutoLevel_Click(object sender, EventArgs e)
        {
            if (this.MainCanvas.Image == null)
                return;

            new OneParamDialog(this.MainCanvas, this.TimeUse, "Auto Level", FilterType.AUTO_LEVEL).ShowDialog(this);
        }

        private void MenuAutoWB_Click(object sender, EventArgs e)
        {
            if (this.MainCanvas.Image == null)
                return;

            new TwoParamDialog(this.MainCanvas, this.TimeUse, "Auto White Balance", FilterType.AUTO_WB).ShowDialog(this);
        }

        private void MenuGammaCorrection_Click(object sender, EventArgs e)
        {
            if (this.MainCanvas.Image == null)
                return;
            if (this.MainCanvas.Channel != 3)
            {
                MessageBox.Show("Filter cannot be applied to Grayscale image, requires RGB Channels.");
            }
            new GammaDialog(this.MainCanvas, this.TimeUse).ShowDialog(this);
        }

        private void MenuSaturation_Click(object sender, EventArgs e)
        {
            if (this.MainCanvas.Image == null)
                return;
            if (this.MainCanvas.Channel != 3)
            {
                MessageBox.Show("Filter cannot be applied to Grayscale image, requires RGB Channels.");
            }
            new OneParamDialog(this.MainCanvas, this.TimeUse, "Saturation", FilterType.SATURATION).ShowDialog(this);
        }

        private void MenuShadowAndHighlight_Click(object sender, EventArgs e)
        {
            if (this.MainCanvas.Image == null)
                return;
            if (this.MainCanvas.Channel != 3)
            {
                MessageBox.Show("Filter cannot be applied to Grayscale image, requires RGB Channels.");
            }
            new TwoParamDialog(this.MainCanvas, this.TimeUse, "Highlight/Shadow", FilterType.SHADOW_HIGHLIGHT).ShowDialog(this);
        }

        private void MenuMotionBlur_Click(object sender, EventArgs e)
        {
            if (this.MainCanvas.Image == null)
                return;
            new TwoParamDialog(this.MainCanvas, this.TimeUse, "Motion Blur", FilterType.MOTION_BLUR).ShowDialog(this);
        }

        private void MenuHue_Click(object sender, EventArgs e)
        {
            if (this.MainCanvas.Image == null)
                return;

            new HueDialog(this.MainCanvas, this.TimeUse).ShowDialog(this);
        }

        private void MenuSkinTone_Click(object sender, EventArgs e)
        {
            if (this.MainCanvas.Image == null)
                return;

            new SkinToneDialog(this.MainCanvas, this.TimeUse).ShowDialog(this);
        }

        private void MenuRadialBlur_Click(object sender, EventArgs e)
        {
            if (this.MainCanvas.Image == null)
                return;

            new RadialBlurDialog(this.MainCanvas, this.TimeUse).ShowDialog(this);
        }
    }
}
