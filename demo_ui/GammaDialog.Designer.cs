namespace OcularDemo
{
    partial class GammaDialog
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
            picChart = new PictureBox();
            panelOptions = new Panel();
            chkSync = new CheckBox();
            udGammaBlue = new NumericUpDown();
            tbGammaBlue = new TrackBar();
            lblAdjust3 = new Label();
            udGammaGreen = new NumericUpDown();
            tbGammaGreen = new TrackBar();
            lblAdjust2 = new Label();
            udGammaRed = new NumericUpDown();
            tbGammaRed = new TrackBar();
            lblAdjust = new Label();
            panelBottom.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)picChart).BeginInit();
            panelOptions.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)udGammaBlue).BeginInit();
            ((System.ComponentModel.ISupportInitialize)tbGammaBlue).BeginInit();
            ((System.ComponentModel.ISupportInitialize)udGammaGreen).BeginInit();
            ((System.ComponentModel.ISupportInitialize)tbGammaGreen).BeginInit();
            ((System.ComponentModel.ISupportInitialize)udGammaRed).BeginInit();
            ((System.ComponentModel.ISupportInitialize)tbGammaRed).BeginInit();
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
            panelBottom.Location = new Point(0, 260);
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
            // picChart
            // 
            picChart.BackColor = Color.Black;
            picChart.BorderStyle = BorderStyle.FixedSingle;
            picChart.Location = new Point(423, 12);
            picChart.Name = "picChart";
            picChart.Size = new Size(200, 200);
            picChart.TabIndex = 8;
            picChart.TabStop = false;
            picChart.Paint += picChart_Paint;
            // 
            // panelOptions
            // 
            panelOptions.BackColor = Color.Transparent;
            panelOptions.Controls.Add(chkSync);
            panelOptions.Controls.Add(udGammaBlue);
            panelOptions.Controls.Add(tbGammaBlue);
            panelOptions.Controls.Add(lblAdjust3);
            panelOptions.Controls.Add(udGammaGreen);
            panelOptions.Controls.Add(tbGammaGreen);
            panelOptions.Controls.Add(lblAdjust2);
            panelOptions.Controls.Add(udGammaRed);
            panelOptions.Controls.Add(tbGammaRed);
            panelOptions.Controls.Add(lblAdjust);
            panelOptions.Location = new Point(12, 12);
            panelOptions.Name = "panelOptions";
            panelOptions.Size = new Size(392, 257);
            panelOptions.TabIndex = 9;
            // 
            // chkSync
            // 
            chkSync.AutoSize = true;
            chkSync.Checked = true;
            chkSync.CheckState = CheckState.Checked;
            chkSync.Location = new Point(34, 217);
            chkSync.Name = "chkSync";
            chkSync.Size = new Size(86, 19);
            chkSync.TabIndex = 19;
            chkSync.Text = "Sync colors";
            chkSync.UseVisualStyleBackColor = true;
            chkSync.CheckedChanged += chkSync_CheckedChanged;
            // 
            // udGammaBlue
            // 
            udGammaBlue.DecimalPlaces = 2;
            udGammaBlue.Location = new Point(325, 149);
            udGammaBlue.Maximum = new decimal(new int[] { 30, 0, 0, 65536 });
            udGammaBlue.Minimum = new decimal(new int[] { 1, 0, 0, 131072 });
            udGammaBlue.Name = "udGammaBlue";
            udGammaBlue.Size = new Size(49, 23);
            udGammaBlue.TabIndex = 18;
            udGammaBlue.Value = new decimal(new int[] { 10, 0, 0, 65536 });
            // 
            // tbGammaBlue
            // 
            tbGammaBlue.AutoSize = false;
            tbGammaBlue.Location = new Point(19, 178);
            tbGammaBlue.Maximum = 300;
            tbGammaBlue.Minimum = 10;
            tbGammaBlue.Name = "tbGammaBlue";
            tbGammaBlue.Size = new Size(359, 24);
            tbGammaBlue.TabIndex = 17;
            tbGammaBlue.Tag = "2";
            tbGammaBlue.TickStyle = TickStyle.None;
            tbGammaBlue.Value = 100;
            tbGammaBlue.Scroll += tbGamma_Scroll;
            // 
            // lblAdjust3
            // 
            lblAdjust3.AutoSize = true;
            lblAdjust3.Location = new Point(19, 151);
            lblAdjust3.Name = "lblAdjust3";
            lblAdjust3.Size = new Size(33, 15);
            lblAdjust3.TabIndex = 16;
            lblAdjust3.Text = "Blue:";
            // 
            // udGammaGreen
            // 
            udGammaGreen.DecimalPlaces = 2;
            udGammaGreen.Location = new Point(325, 86);
            udGammaGreen.Maximum = new decimal(new int[] { 30, 0, 0, 65536 });
            udGammaGreen.Minimum = new decimal(new int[] { 1, 0, 0, 131072 });
            udGammaGreen.Name = "udGammaGreen";
            udGammaGreen.Size = new Size(49, 23);
            udGammaGreen.TabIndex = 15;
            udGammaGreen.Value = new decimal(new int[] { 10, 0, 0, 65536 });
            // 
            // tbGammaGreen
            // 
            tbGammaGreen.AutoSize = false;
            tbGammaGreen.Location = new Point(19, 115);
            tbGammaGreen.Maximum = 300;
            tbGammaGreen.Minimum = 10;
            tbGammaGreen.Name = "tbGammaGreen";
            tbGammaGreen.Size = new Size(359, 24);
            tbGammaGreen.TabIndex = 14;
            tbGammaGreen.Tag = "1";
            tbGammaGreen.TickStyle = TickStyle.None;
            tbGammaGreen.Value = 100;
            tbGammaGreen.Scroll += tbGamma_Scroll;
            // 
            // lblAdjust2
            // 
            lblAdjust2.AutoSize = true;
            lblAdjust2.Location = new Point(19, 88);
            lblAdjust2.Name = "lblAdjust2";
            lblAdjust2.Size = new Size(41, 15);
            lblAdjust2.TabIndex = 13;
            lblAdjust2.Text = "Green:";
            // 
            // udGammaRed
            // 
            udGammaRed.DecimalPlaces = 2;
            udGammaRed.Location = new Point(325, 21);
            udGammaRed.Maximum = new decimal(new int[] { 30, 0, 0, 65536 });
            udGammaRed.Minimum = new decimal(new int[] { 1, 0, 0, 131072 });
            udGammaRed.Name = "udGammaRed";
            udGammaRed.Size = new Size(49, 23);
            udGammaRed.TabIndex = 12;
            udGammaRed.Value = new decimal(new int[] { 10, 0, 0, 65536 });
            // 
            // tbGammaRed
            // 
            tbGammaRed.AutoSize = false;
            tbGammaRed.Location = new Point(19, 50);
            tbGammaRed.Maximum = 300;
            tbGammaRed.Minimum = 10;
            tbGammaRed.Name = "tbGammaRed";
            tbGammaRed.Size = new Size(359, 24);
            tbGammaRed.TabIndex = 11;
            tbGammaRed.Tag = "0";
            tbGammaRed.TickStyle = TickStyle.None;
            tbGammaRed.Value = 100;
            tbGammaRed.Scroll += tbGamma_Scroll;
            // 
            // lblAdjust
            // 
            lblAdjust.AutoSize = true;
            lblAdjust.Location = new Point(15, 23);
            lblAdjust.Name = "lblAdjust";
            lblAdjust.Size = new Size(30, 15);
            lblAdjust.TabIndex = 10;
            lblAdjust.Text = "Red:";
            // 
            // GammaDialog
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(635, 317);
            Controls.Add(picChart);
            Controls.Add(panelBottom);
            Controls.Add(panelOptions);
            FormBorderStyle = FormBorderStyle.FixedDialog;
            MaximizeBox = false;
            MinimizeBox = false;
            Name = "GammaDialog";
            Opacity = 0.9D;
            ShowInTaskbar = false;
            StartPosition = FormStartPosition.CenterParent;
            Text = "Gamma Correction";
            FormClosed += GammaDialog_FormClosed;
            Load += GammaDialog_Load;
            panelBottom.ResumeLayout(false);
            panelBottom.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)picChart).EndInit();
            panelOptions.ResumeLayout(false);
            panelOptions.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)udGammaBlue).EndInit();
            ((System.ComponentModel.ISupportInitialize)tbGammaBlue).EndInit();
            ((System.ComponentModel.ISupportInitialize)udGammaGreen).EndInit();
            ((System.ComponentModel.ISupportInitialize)tbGammaGreen).EndInit();
            ((System.ComponentModel.ISupportInitialize)udGammaRed).EndInit();
            ((System.ComponentModel.ISupportInitialize)tbGammaRed).EndInit();
            ResumeLayout(false);
        }

        #endregion

        private GroupBox groupbox;
        private CheckBox chkPreview;
        private Button btnCancel;
        private Button btnOK;
        private Button btnReset;
        private PictureBox picChart;
        private Panel panelBottom;
        private Panel panelOptions;
        private CheckBox chkSync;
        private NumericUpDown udGammaBlue;
        private TrackBar tbGammaBlue;
        private Label lblAdjust3;
        private NumericUpDown udGammaGreen;
        private TrackBar tbGammaGreen;
        private Label lblAdjust2;
        private NumericUpDown udGammaRed;
        private TrackBar tbGammaRed;
        private Label lblAdjust;
    }
}