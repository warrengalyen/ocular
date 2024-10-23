using System.ComponentModel;

namespace OcularDemo
{
    public partial class HuePicker : UserControl
    {
        private HuePickerType type = HuePickerType.Value;

        private Pen blackPen;
        private Brush blackBrush;
        private Pen whitePen;
        private Brush whiteBrush;

        private System.Drawing.Point ptCenter = new System.Drawing.Point(0, 0);
        private System.Drawing.Point ptMin = new System.Drawing.Point(0, 0);
        private System.Drawing.Point ptMax = new System.Drawing.Point(0, 0);
        private int trackMode = 0;

        private int min = 0;
        private int max = 359;

        public enum HuePickerType
        {
            Value,
            Range
        }

        public event EventHandler ValuesChanged;

        [DefaultValue(0)]
        public int Value
        {
            get { return min; }
            set
            {
                if (type == HuePickerType.Value)
                {
                    min = Math.Max(0, Math.Min(359, value));
                    Invalidate();
                }
            }
        }

        [DefaultValue(0)]
        public int Min
        {
            get { return min; }
            set
            {
                if (type == HuePickerType.Range)
                {
                    min = Math.Max(0, Math.Min(359, value));
                    Invalidate();
                }
            }
        }

        [DefaultValue(359)]
        public int Max
        {
            get { return max; }
            set
            {
                if (type == HuePickerType.Range)
                {
                    max = Math.Max(0, Math.Min(359, value));
                    Invalidate();
                }
            }
        }

        [DefaultValue(HuePickerType.Value)]
        public HuePickerType Type
        {
            get { return type; }
            set
            {
                type = value;
                Invalidate();
            }
        }

        public HuePicker()
        {
            InitializeComponent();

            SetStyle(ControlStyles.AllPaintingInWmPaint | ControlStyles.ResizeRedraw |
                ControlStyles.DoubleBuffer | ControlStyles.UserPaint, true);

            blackPen = new Pen(Color.Black, 1);
            blackBrush = new SolidBrush(Color.Black);
            whitePen = new Pen(Color.White, 1);
            whiteBrush = new SolidBrush(Color.White);
        }

        protected override void OnPaint(PaintEventArgs pe)
        {
            Graphics g = pe.Graphics;
            Rectangle rc = this.ClientRectangle;
            Rectangle rcPie;
            Brush brush;
            RGB rgb = new RGB();
            HSL hsl = new HSL();

            // get pie rectangle
            rcPie = new Rectangle(4, 4, Math.Min(rc.Right, rc.Bottom) - 8, Math.Min(rc.Right, rc.Bottom) - 8);

            // init HSL value
            hsl.Luminance = 0.5f;
            hsl.Saturation = 1.0f;

            if (type == HuePickerType.Value)
            {
                // draw HSL pie
                for (int i = 0; i < 360; i++)
                {
                    hsl.Hue = i;
                    // convert from HSL to RGB
                    HSL.ToRGB(hsl, rgb);
                    // create brush
                    brush = new SolidBrush(rgb.Color);
                    // draw one hue value
                    g.FillPie(brush, rcPie, -i, -1);

                    brush.Dispose();
                }
            }
            else
            {
                // draw HSL pie
                for (int i = 0; i < 360; i++)
                {
                    if (
                        ((min < max) && (i >= min) && (i <= max)) ||
                        ((min > max) && ((i >= min) || (i <= max))))
                    {
                        hsl.Hue = i;
                        // convert from HSL to RGB
                        HSL.ToRGB(hsl, rgb);
                        // create brush
                        brush = new SolidBrush(rgb.Color);
                    }
                    else
                    {
                        brush = new SolidBrush(Color.FromArgb(128, 128, 128));
                    }

                    // draw one hue value
                    g.FillPie(brush, rcPie, -i, -1);

                    brush.Dispose();
                }
            }

            double halfWidth = (double)rcPie.Width / 2;
            double angleRad = -min * Math.PI / 180;
            double angleCos = Math.Cos(angleRad);
            double angleSin = Math.Sin(angleRad);

            double x = halfWidth * angleCos;
            double y = halfWidth * angleSin;

            ptCenter.X = rcPie.Left + (int)(halfWidth);
            ptCenter.Y = rcPie.Top + (int)(halfWidth);
            ptMin.X = rcPie.Left + (int)(halfWidth + x);
            ptMin.Y = rcPie.Top + (int)(halfWidth + y);

            // draw MIN pointer
            g.FillEllipse(blackBrush,
                rcPie.Left + (int)(halfWidth + x) - 4,
                rcPie.Top + (int)(halfWidth + y) - 4,
                8, 8);
            g.DrawLine(blackPen, ptCenter, ptMin);

            // check picker type
            if (type == HuePickerType.Range)
            {
                angleRad = -max * Math.PI / 180;
                angleCos = Math.Cos(angleRad);
                angleSin = Math.Sin(angleRad);

                x = halfWidth * angleCos;
                y = halfWidth * angleSin;

                ptMax.X = rcPie.Left + (int)(halfWidth + x);
                ptMax.Y = rcPie.Top + (int)(halfWidth + y);

                // draw MAX pointer
                g.FillEllipse(whiteBrush,
                    rcPie.Left + (int)(halfWidth + x) - 4,
                    rcPie.Top + (int)(halfWidth + y) - 4,
                    8, 8);
                g.DrawLine(whitePen, ptCenter, ptMax);
            }

            base.OnPaint(pe);
        }

        private void HuePicker_MouseDown(object sender, MouseEventArgs e)
        {
            // check coordinates of MIN pointer
            if ((e.X >= ptMin.X - 4) && (e.Y >= ptMin.Y - 4) &&
                 (e.X < ptMin.X + 4) && (e.Y < ptMin.Y + 4))
            {
                trackMode = 1;
            }
            if (type == HuePickerType.Range)
            {
                // check coordinates of MAX pointer
                if ((e.X >= ptMax.X - 4) && (e.Y >= ptMax.Y - 4) &&
                     (e.X < ptMax.X + 4) && (e.Y < ptMax.Y + 4))
                {
                    trackMode = 2;
                }
            }

            if (trackMode != 0)
                this.Capture = true;
        }

        private void HuePicker_MouseUp(object sender, MouseEventArgs e)
        {
            if (trackMode != 0)
            {
                // release capture
                this.Capture = false;
                trackMode = 0;

                // notify client
                if (ValuesChanged != null)
                    ValuesChanged(this, new EventArgs());
            }
        }

        private void HuePicker_MouseMove(object sender, MouseEventArgs e)
        {
            Cursor cursor = Cursors.Default;

            if (trackMode != 0)
            {
                cursor = Cursors.Hand;

                int dy = e.Y - ptCenter.Y;
                int dx = e.X - ptCenter.X;

                if (trackMode == 1)
                {
                    // MIN pointer tracking
                    min = (int)(Math.Atan2(-dy, dx) * 180 / Math.PI);
                    if (min < 0)
                    {
                        min = 360 + min;
                    }
                }
                else
                {
                    // MAX pointer tracking
                    max = (int)(Math.Atan2(-dy, dx) * 180 / Math.PI);
                    if (max < 0)
                    {
                        max = 360 + max;
                    }
                }

                // repaint control
                Invalidate();
            }
            else
            {
                // check coordinates of MIN pointer
                if ((e.X >= ptMin.X - 4) && (e.Y >= ptMin.Y - 4) &&
                     (e.X < ptMin.X + 4) && (e.Y < ptMin.Y + 4))
                {
                    cursor = Cursors.Hand;
                }
                if (type == HuePickerType.Range)
                {
                    // check coordinates of MAX pointer
                    if ((e.X >= ptMax.X - 4) && (e.Y >= ptMax.Y - 4) &&
                         (e.X < ptMax.X + 4) && (e.Y < ptMax.Y + 4))
                    {
                        cursor = Cursors.Hand;
                    }
                }

            }

            this.Cursor = cursor;
        }
    }
}