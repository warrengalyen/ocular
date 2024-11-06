namespace OcularDemo
{
    partial class RadialBlurDialog
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
            udAdjustLevel3 = new NumericUpDown();
            tbAdjustLevel3 = new TrackBar();
            udAdjustLevel2 = new NumericUpDown();
            tbAdjustLevel2 = new TrackBar();
            lblAdjust2 = new Label();
            udAdjustLevel = new NumericUpDown();
            tbAdjustLevel = new TrackBar();
            lblAdjust = new Label();
            panelBottom.SuspendLayout();
            panelOptions.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)udAdjustLevel3).BeginInit();
            ((System.ComponentModel.ISupportInitialize)tbAdjustLevel3).BeginInit();
            ((System.ComponentModel.ISupportInitialize)udAdjustLevel2).BeginInit();
            ((System.ComponentModel.ISupportInitialize)tbAdjustLevel2).BeginInit();
            ((System.ComponentModel.ISupportInitialize)udAdjustLevel).BeginInit();
            ((System.ComponentModel.ISupportInitialize)tbAdjustLevel).BeginInit();
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
            panelBottom.Location = new Point(0, 185);
            panelBottom.Name = "panelBottom";
            panelBottom.Size = new Size(523, 57);
            panelBottom.TabIndex = 8;
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
            btnCancel.Location = new Point(411, 12);
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
            btnOK.Location = new Point(305, 12);
            btnOK.Name = "btnOK";
            btnOK.Size = new Size(100, 33);
            btnOK.TabIndex = 5;
            btnOK.Text = "OK";
            btnOK.UseVisualStyleBackColor = true;
            // 
            // panelOptions
            // 
            panelOptions.Controls.Add(udAdjustLevel3);
            panelOptions.Controls.Add(tbAdjustLevel3);
            panelOptions.Controls.Add(udAdjustLevel2);
            panelOptions.Controls.Add(tbAdjustLevel2);
            panelOptions.Controls.Add(lblAdjust2);
            panelOptions.Controls.Add(udAdjustLevel);
            panelOptions.Controls.Add(tbAdjustLevel);
            panelOptions.Controls.Add(lblAdjust);
            panelOptions.Location = new Point(12, 11);
            panelOptions.Name = "panelOptions";
            panelOptions.Size = new Size(386, 173);
            panelOptions.TabIndex = 9;
            // 
            // udAdjustLevel3
            // 
            udAdjustLevel3.DecimalPlaces = 2;
            udAdjustLevel3.Location = new Point(324, 135);
            udAdjustLevel3.Maximum = new decimal(new int[] { 2, 0, 0, 0 });
            udAdjustLevel3.Minimum = new decimal(new int[] { 2, 0, 0, int.MinValue });
            udAdjustLevel3.Name = "udAdjustLevel3";
            udAdjustLevel3.Size = new Size(49, 23);
            udAdjustLevel3.TabIndex = 13;
            udAdjustLevel3.ValueChanged += udAdjustLevel3_ValueChanged;
            // 
            // tbAdjustLevel3
            // 
            tbAdjustLevel3.AutoSize = false;
            tbAdjustLevel3.Location = new Point(151, 137);
            tbAdjustLevel3.Maximum = 200;
            tbAdjustLevel3.Minimum = -200;
            tbAdjustLevel3.Name = "tbAdjustLevel3";
            tbAdjustLevel3.Size = new Size(167, 24);
            tbAdjustLevel3.TabIndex = 12;
            tbAdjustLevel3.TickStyle = TickStyle.None;
            tbAdjustLevel3.Scroll += tbAdjustLevel3_Scroll;
            // 
            // udAdjustLevel2
            // 
            udAdjustLevel2.DecimalPlaces = 2;
            udAdjustLevel2.Location = new Point(324, 97);
            udAdjustLevel2.Maximum = new decimal(new int[] { 2, 0, 0, 0 });
            udAdjustLevel2.Minimum = new decimal(new int[] { 20, 0, 0, -2147418112 });
            udAdjustLevel2.Name = "udAdjustLevel2";
            udAdjustLevel2.Size = new Size(49, 23);
            udAdjustLevel2.TabIndex = 11;
            udAdjustLevel2.ValueChanged += udAdjustLevel2_ValueChanged;
            // 
            // tbAdjustLevel2
            // 
            tbAdjustLevel2.AutoSize = false;
            tbAdjustLevel2.Location = new Point(151, 99);
            tbAdjustLevel2.Maximum = 200;
            tbAdjustLevel2.Minimum = -200;
            tbAdjustLevel2.Name = "tbAdjustLevel2";
            tbAdjustLevel2.Size = new Size(167, 24);
            tbAdjustLevel2.TabIndex = 10;
            tbAdjustLevel2.TickStyle = TickStyle.None;
            tbAdjustLevel2.Scroll += tbAdjustLevel2_Scroll;
            // 
            // lblAdjust2
            // 
            lblAdjust2.AutoSize = true;
            lblAdjust2.Location = new Point(14, 83);
            lblAdjust2.Name = "lblAdjust2";
            lblAdjust2.Size = new Size(45, 15);
            lblAdjust2.TabIndex = 9;
            lblAdjust2.Text = "Center:";
            // 
            // udAdjustLevel
            // 
            udAdjustLevel.Location = new Point(324, 16);
            udAdjustLevel.Name = "udAdjustLevel";
            udAdjustLevel.Size = new Size(49, 23);
            udAdjustLevel.TabIndex = 8;
            udAdjustLevel.Value = new decimal(new int[] { 20, 0, 0, 0 });
            udAdjustLevel.ValueChanged += udAdjustLevel_ValueChanged;
            // 
            // tbAdjustLevel
            // 
            tbAdjustLevel.AutoSize = false;
            tbAdjustLevel.Location = new Point(14, 45);
            tbAdjustLevel.Maximum = 100;
            tbAdjustLevel.Name = "tbAdjustLevel";
            tbAdjustLevel.Size = new Size(359, 24);
            tbAdjustLevel.TabIndex = 7;
            tbAdjustLevel.TickStyle = TickStyle.None;
            tbAdjustLevel.Value = 20;
            tbAdjustLevel.Scroll += tbAdjustLevel_Scroll;
            // 
            // lblAdjust
            // 
            lblAdjust.AutoSize = true;
            lblAdjust.Location = new Point(14, 18);
            lblAdjust.Name = "lblAdjust";
            lblAdjust.Size = new Size(55, 15);
            lblAdjust.TabIndex = 6;
            lblAdjust.Text = "Strength:";
            // 
            // RadialBlurDialog
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(523, 242);
            Controls.Add(panelBottom);
            Controls.Add(panelOptions);
            FormBorderStyle = FormBorderStyle.FixedDialog;
            MaximizeBox = false;
            MinimizeBox = false;
            Name = "RadialBlurDialog";
            Opacity = 0.9D;
            ShowIcon = false;
            ShowInTaskbar = false;
            StartPosition = FormStartPosition.CenterParent;
            Text = "Radial Blur";
            FormClosed += RadialBlurDialog_FormClosed;
            Load += RadialBlurDialog_Load;
            panelBottom.ResumeLayout(false);
            panelBottom.PerformLayout();
            panelOptions.ResumeLayout(false);
            panelOptions.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)udAdjustLevel3).EndInit();
            ((System.ComponentModel.ISupportInitialize)tbAdjustLevel3).EndInit();
            ((System.ComponentModel.ISupportInitialize)udAdjustLevel2).EndInit();
            ((System.ComponentModel.ISupportInitialize)tbAdjustLevel2).EndInit();
            ((System.ComponentModel.ISupportInitialize)udAdjustLevel).EndInit();
            ((System.ComponentModel.ISupportInitialize)tbAdjustLevel).EndInit();
            ResumeLayout(false);
        }

        #endregion
        private Panel panelBottom;
        private Button btnReset;
        private CheckBox chkPreview;
        private Button btnCancel;
        private Button btnOK;
        private Panel panelOptions;
        private NumericUpDown udAdjustLevel2;
        private TrackBar tbAdjustLevel2;
        private Label lblAdjust2;
        private NumericUpDown udAdjustLevel;
        private TrackBar tbAdjustLevel;
        private Label lblAdjust;
        private NumericUpDown udAdjustLevel3;
        private TrackBar tbAdjustLevel3;
    }
}