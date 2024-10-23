using OcularDemo.Controls;

namespace OcularDemo
{
    partial class TestForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(TestForm));
            colorSlider1 = new ColorSlider.ColorSlider();
            curveControl = new CurveControlRgb();
            labelCoordinates = new Label();
            SuspendLayout();
            // 
            // colorSlider1
            // 
            colorSlider1.BackColor = Color.Transparent;
            colorSlider1.BarInnerColor = Color.Silver;
            colorSlider1.BarPenColorBottom = Color.FromArgb(87, 94, 110);
            colorSlider1.BarPenColorTop = Color.FromArgb(55, 60, 74);
            colorSlider1.BarWidth = 4;
            colorSlider1.BorderRoundRectSize = new Size(8, 8);
            colorSlider1.DrawSemitransparentThumb = false;
            colorSlider1.ElapsedInnerColor = Color.Silver;
            colorSlider1.ElapsedPenColorBottom = Color.FromArgb(87, 94, 110);
            colorSlider1.ElapsedPenColorTop = Color.FromArgb(55, 60, 74);
            colorSlider1.Font = new Font("Microsoft Sans Serif", 6F, FontStyle.Regular, GraphicsUnit.Point);
            colorSlider1.ForeColor = Color.White;
            colorSlider1.LargeChange = new decimal(new int[] { 5, 0, 0, 0 });
            colorSlider1.Location = new Point(362, 272);
            colorSlider1.Maximum = new decimal(new int[] { 100, 0, 0, 0 });
            colorSlider1.Minimum = new decimal(new int[] { 0, 0, 0, 0 });
            colorSlider1.Name = "colorSlider1";
            colorSlider1.ScaleDivisions = new decimal(new int[] { 10, 0, 0, 0 });
            colorSlider1.ScaleSubDivisions = new decimal(new int[] { 5, 0, 0, 0 });
            colorSlider1.ShowDivisionsText = false;
            colorSlider1.ShowSmallScale = true;
            colorSlider1.Size = new Size(426, 66);
            colorSlider1.SmallChange = new decimal(new int[] { 1, 0, 0, 0 });
            colorSlider1.TabIndex = 0;
            colorSlider1.Text = "colorSlider1";
            colorSlider1.ThumbInnerColor = Color.DarkGray;
            colorSlider1.ThumbPenColor = Color.DarkGray;
            colorSlider1.ThumbRoundRectSize = new Size(4, 4);
            colorSlider1.ThumbSize = new Size(8, 20);
            colorSlider1.TickAdd = 0F;
            colorSlider1.TickColor = Color.Gray;
            colorSlider1.TickDivide = 0F;
            colorSlider1.Value = new decimal(new int[] { 30, 0, 0, 0 });
            // 
            // curveControl
            // 
            curveControl.BorderStyle = BorderStyle.FixedSingle;

            curveControl.Location = new Point(54, 87);
            curveControl.Name = "curveControl";
            curveControl.Size = new Size(253, 251);
            curveControl.TabIndex = 1;
            curveControl.TabStop = false;
            // 
            // labelCoordinates
            // 
            labelCoordinates.AutoSize = true;
            labelCoordinates.Location = new Point(269, 60);
            labelCoordinates.Name = "labelCoordinates";
            labelCoordinates.Size = new Size(38, 15);
            labelCoordinates.TabIndex = 2;
            labelCoordinates.Text = "label1";
            labelCoordinates.TextAlign = ContentAlignment.MiddleRight;
            // 
            // TestForm
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(800, 450);
            Controls.Add(labelCoordinates);
            Controls.Add(curveControl);
            Controls.Add(colorSlider1);
            Name = "TestForm";
            Text = "Form1";
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private ColorSlider.ColorSlider colorSlider1;
        private CurveControlRgb curveControl;
        private Label labelCoordinates;
    }
}