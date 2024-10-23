namespace OcularDemo
{
    partial class SkinToneDialog
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
            panelBottom = new Panel();
            btnReset = new Button();
            chkPreview = new CheckBox();
            btnCancel = new Button();
            btnOK = new Button();
            panelOptions = new Panel();
            udSatShift = new NumericUpDown();
            tbSatShift = new TrackBar();
            label1 = new Label();
            rbOrange = new RadioButton();
            rbGreen = new RadioButton();
            lblUpperColor = new Label();
            udHueThreshold = new NumericUpDown();
            tbHueThreshold = new TrackBar();
            lblAdjust3 = new Label();
            udHue = new NumericUpDown();
            tbHue = new TrackBar();
            lblAdjust2 = new Label();
            udAmount = new NumericUpDown();
            tbAmount = new TrackBar();
            lblAdjust = new Label();
            panelBottom.SuspendLayout();
            panelOptions.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)udSatShift).BeginInit();
            ((System.ComponentModel.ISupportInitialize)tbSatShift).BeginInit();
            ((System.ComponentModel.ISupportInitialize)udHueThreshold).BeginInit();
            ((System.ComponentModel.ISupportInitialize)tbHueThreshold).BeginInit();
            ((System.ComponentModel.ISupportInitialize)udHue).BeginInit();
            ((System.ComponentModel.ISupportInitialize)tbHue).BeginInit();
            ((System.ComponentModel.ISupportInitialize)udAmount).BeginInit();
            ((System.ComponentModel.ISupportInitialize)tbAmount).BeginInit();
            SuspendLayout();
            // 
            // panelBottom
            // 
            panelBottom.BackColor = SystemColors.ControlLight;
            panelBottom.Controls.Add(btnReset);
            panelBottom.Controls.Add(chkPreview);
            panelBottom.Controls.Add(btnCancel);
            panelBottom.Controls.Add(btnOK);
            panelBottom.Dock = DockStyle.Bottom;
            panelBottom.Location = new Point(0, 342);
            panelBottom.Name = "panelBottom";
            panelBottom.Size = new Size(635, 57);
            panelBottom.TabIndex = 7;
            // 
            // btnReset
            // 
            btnReset.Image = Properties.Resources.icons8_reset_20;
            btnReset.Location = new Point(12, 12);
            btnReset.Name = "btnReset";
            btnReset.Size = new Size(36, 33);
            btnReset.TabIndex = 8;
            btnReset.UseVisualStyleBackColor = true;
            btnReset.Click += btnReset_Click;
            // 
            // chkPreview
            // 
            chkPreview.AutoSize = true;
            chkPreview.Checked = true;
            chkPreview.CheckState = CheckState.Checked;
            chkPreview.Location = new Point(76, 20);
            chkPreview.Name = "chkPreview";
            chkPreview.Size = new Size(67, 19);
            chkPreview.TabIndex = 7;
            chkPreview.Text = "Preview";
            chkPreview.UseVisualStyleBackColor = true;
            chkPreview.CheckedChanged += chkPreview_CheckedChanged;
            // 
            // btnCancel
            // 
            btnCancel.DialogResult = DialogResult.Cancel;
            btnCancel.Location = new Point(523, 12);
            btnCancel.Name = "btnCancel";
            btnCancel.Size = new Size(100, 33);
            btnCancel.TabIndex = 6;
            btnCancel.Text = "Cancel";
            btnCancel.UseVisualStyleBackColor = true;
            btnCancel.Click += btnCancel_Click;
            // 
            // btnOK
            // 
            btnOK.DialogResult = DialogResult.OK;
            btnOK.Location = new Point(417, 12);
            btnOK.Name = "btnOK";
            btnOK.Size = new Size(100, 33);
            btnOK.TabIndex = 5;
            btnOK.Text = "OK";
            btnOK.UseVisualStyleBackColor = true;
            // 
            // panelOptions
            // 
            panelOptions.BackColor = Color.Transparent;
            panelOptions.Controls.Add(udSatShift);
            panelOptions.Controls.Add(tbSatShift);
            panelOptions.Controls.Add(label1);
            panelOptions.Controls.Add(rbOrange);
            panelOptions.Controls.Add(rbGreen);
            panelOptions.Controls.Add(lblUpperColor);
            panelOptions.Controls.Add(udHueThreshold);
            panelOptions.Controls.Add(tbHueThreshold);
            panelOptions.Controls.Add(lblAdjust3);
            panelOptions.Controls.Add(udHue);
            panelOptions.Controls.Add(tbHue);
            panelOptions.Controls.Add(lblAdjust2);
            panelOptions.Controls.Add(udAmount);
            panelOptions.Controls.Add(tbAmount);
            panelOptions.Controls.Add(lblAdjust);
            panelOptions.Location = new Point(12, 12);
            panelOptions.Name = "panelOptions";
            panelOptions.Size = new Size(392, 314);
            panelOptions.TabIndex = 9;
            // 
            // udSatShift
            // 
            udSatShift.Location = new Point(325, 213);
            udSatShift.Name = "udSatShift";
            udSatShift.Size = new Size(49, 23);
            udSatShift.TabIndex = 24;
            udSatShift.Value = new decimal(new int[] { 40, 0, 0, 0 });
            udSatShift.ValueChanged += udSatShift_ValueChanged;
            // 
            // tbSatShift
            // 
            tbSatShift.AutoSize = false;
            tbSatShift.Location = new Point(21, 242);
            tbSatShift.Maximum = 100;
            tbSatShift.Name = "tbSatShift";
            tbSatShift.Size = new Size(359, 24);
            tbSatShift.TabIndex = 23;
            tbSatShift.Tag = "2";
            tbSatShift.TickStyle = TickStyle.None;
            tbSatShift.Value = 40;
            tbSatShift.Scroll += tbSatShift_Scroll;
            // 
            // label1
            // 
            label1.AutoSize = true;
            label1.Location = new Point(21, 215);
            label1.Name = "label1";
            label1.Size = new Size(91, 15);
            label1.TabIndex = 22;
            label1.Text = "Saturation Shift:";
            // 
            // rbOrange
            // 
            rbOrange.AutoSize = true;
            rbOrange.Location = new Point(191, 279);
            rbOrange.Name = "rbOrange";
            rbOrange.Size = new Size(92, 19);
            rbOrange.TabIndex = 21;
            rbOrange.Text = "Pink/Orange";
            rbOrange.UseVisualStyleBackColor = true;
            rbOrange.CheckedChanged += rbOrange_CheckedChanged;
            // 
            // rbGreen
            // 
            rbGreen.AutoSize = true;
            rbGreen.Checked = true;
            rbGreen.Location = new Point(101, 279);
            rbGreen.Name = "rbGreen";
            rbGreen.Size = new Size(84, 19);
            rbGreen.TabIndex = 20;
            rbGreen.TabStop = true;
            rbGreen.Text = "Pink/Green";
            rbGreen.UseVisualStyleBackColor = true;
            rbGreen.CheckedChanged += rbGreen_CheckedChanged;
            // 
            // lblUpperColor
            // 
            lblUpperColor.AutoSize = true;
            lblUpperColor.Location = new Point(21, 280);
            lblUpperColor.Name = "lblUpperColor";
            lblUpperColor.Size = new Size(74, 15);
            lblUpperColor.TabIndex = 19;
            lblUpperColor.Text = "Upper Color:";
            // 
            // udHueThreshold
            // 
            udHueThreshold.Location = new Point(325, 149);
            udHueThreshold.Name = "udHueThreshold";
            udHueThreshold.Size = new Size(49, 23);
            udHueThreshold.TabIndex = 18;
            udHueThreshold.Value = new decimal(new int[] { 40, 0, 0, 0 });
            udHueThreshold.ValueChanged += udHueThreshold_ValueChanged;
            // 
            // tbHueThreshold
            // 
            tbHueThreshold.AutoSize = false;
            tbHueThreshold.Location = new Point(19, 178);
            tbHueThreshold.Maximum = 100;
            tbHueThreshold.Name = "tbHueThreshold";
            tbHueThreshold.Size = new Size(359, 24);
            tbHueThreshold.TabIndex = 17;
            tbHueThreshold.Tag = "2";
            tbHueThreshold.TickStyle = TickStyle.None;
            tbHueThreshold.Value = 40;
            tbHueThreshold.Scroll += tbHueThreshold_Scroll;
            // 
            // lblAdjust3
            // 
            lblAdjust3.AutoSize = true;
            lblAdjust3.Location = new Point(19, 151);
            lblAdjust3.Name = "lblAdjust3";
            lblAdjust3.Size = new Size(87, 15);
            lblAdjust3.TabIndex = 16;
            lblAdjust3.Text = "Hue Threshold:";
            // 
            // udHue
            // 
            udHue.Location = new Point(325, 86);
            udHue.Name = "udHue";
            udHue.Size = new Size(49, 23);
            udHue.TabIndex = 15;
            udHue.Value = new decimal(new int[] { 5, 0, 0, 0 });
            udHue.ValueChanged += udHue_ValueChanged;
            // 
            // tbHue
            // 
            tbHue.AutoSize = false;
            tbHue.Location = new Point(19, 115);
            tbHue.Maximum = 100;
            tbHue.Name = "tbHue";
            tbHue.Size = new Size(359, 24);
            tbHue.TabIndex = 14;
            tbHue.Tag = "1";
            tbHue.TickStyle = TickStyle.None;
            tbHue.Value = 5;
            tbHue.Scroll += tbHue_Scroll;
            // 
            // lblAdjust2
            // 
            lblAdjust2.AutoSize = true;
            lblAdjust2.Location = new Point(19, 88);
            lblAdjust2.Name = "lblAdjust2";
            lblAdjust2.Size = new Size(32, 15);
            lblAdjust2.TabIndex = 13;
            lblAdjust2.Text = "Hue:";
            // 
            // udAmount
            // 
            udAmount.Location = new Point(325, 21);
            udAmount.Maximum = new decimal(new int[] { 30, 0, 0, 0 });
            udAmount.Minimum = new decimal(new int[] { 30, 0, 0, int.MinValue });
            udAmount.Name = "udAmount";
            udAmount.Size = new Size(49, 23);
            udAmount.TabIndex = 12;
            udAmount.ValueChanged += udAmount_ValueChanged;
            // 
            // tbAmount
            // 
            tbAmount.AutoSize = false;
            tbAmount.Location = new Point(19, 50);
            tbAmount.Maximum = 30;
            tbAmount.Minimum = -30;
            tbAmount.Name = "tbAmount";
            tbAmount.Size = new Size(359, 24);
            tbAmount.TabIndex = 11;
            tbAmount.Tag = "0";
            tbAmount.TickStyle = TickStyle.None;
            tbAmount.Scroll += tbAmount_Scroll;
            // 
            // lblAdjust
            // 
            lblAdjust.AutoSize = true;
            lblAdjust.Location = new Point(21, 23);
            lblAdjust.Name = "lblAdjust";
            lblAdjust.Size = new Size(54, 15);
            lblAdjust.TabIndex = 10;
            lblAdjust.Text = "Amount:";
            // 
            // SkinToneDialog
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(635, 399);
            Controls.Add(panelBottom);
            Controls.Add(panelOptions);
            FormBorderStyle = FormBorderStyle.FixedDialog;
            MaximizeBox = false;
            MinimizeBox = false;
            Name = "SkinToneDialog";
            Opacity = 0.9D;
            ShowInTaskbar = false;
            StartPosition = FormStartPosition.CenterParent;
            Text = "Skin Tone";
            FormClosed += SkinToneDialog_FormClosed;
            Load += SkinToneDialog_Load;
            panelBottom.ResumeLayout(false);
            panelBottom.PerformLayout();
            panelOptions.ResumeLayout(false);
            panelOptions.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)udSatShift).EndInit();
            ((System.ComponentModel.ISupportInitialize)tbSatShift).EndInit();
            ((System.ComponentModel.ISupportInitialize)udHueThreshold).EndInit();
            ((System.ComponentModel.ISupportInitialize)tbHueThreshold).EndInit();
            ((System.ComponentModel.ISupportInitialize)udHue).EndInit();
            ((System.ComponentModel.ISupportInitialize)tbHue).EndInit();
            ((System.ComponentModel.ISupportInitialize)udAmount).EndInit();
            ((System.ComponentModel.ISupportInitialize)tbAmount).EndInit();
            ResumeLayout(false);
        }

        #endregion

        private GroupBox groupbox;
        private CheckBox chkPreview;
        private Button btnCancel;
        private Button btnOK;
        private Button btnReset;
        private Panel panelBottom;
        private Panel panelOptions;
        private NumericUpDown udHueThreshold;
        private TrackBar tbHueThreshold;
        private Label lblAdjust3;
        private NumericUpDown udHue;
        private TrackBar tbHue;
        private Label lblAdjust2;
        private NumericUpDown udAmount;
        private TrackBar tbAmount;
        private Label lblAdjust;
        private RadioButton rbOrange;
        private RadioButton rbGreen;
        private Label lblUpperColor;
        private TrackBar tbSatShift;
        private Label label1;
        private NumericUpDown udSatShift;
    }
}