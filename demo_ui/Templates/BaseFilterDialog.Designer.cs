namespace OcularDemo
{
    partial class BaseFilterDialog
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
            udAdjustLevel2 = new NumericUpDown();
            tbAdjustLevel2 = new TrackBar();
            lblAdjust2 = new Label();
            udAdjustLevel = new NumericUpDown();
            tbAdjustLevel = new TrackBar();
            lblAdjust = new Label();
            panelBottom.SuspendLayout();
            panelOptions.SuspendLayout();
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
            panelBottom.Location = new Point(0, 169);
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
            // udAdjustLevel2
            // 
            udAdjustLevel2.Location = new Point(324, 81);
            udAdjustLevel2.Minimum = new decimal(new int[] { 100, 0, 0, int.MinValue });
            udAdjustLevel2.Name = "udAdjustLevel2";
            udAdjustLevel2.Size = new Size(49, 23);
            udAdjustLevel2.TabIndex = 11;
            // 
            // tbAdjustLevel2
            // 
            tbAdjustLevel2.AutoSize = false;
            tbAdjustLevel2.Location = new Point(14, 110);
            tbAdjustLevel2.Maximum = 100;
            tbAdjustLevel2.Minimum = -100;
            tbAdjustLevel2.Name = "tbAdjustLevel2";
            tbAdjustLevel2.Size = new Size(359, 24);
            tbAdjustLevel2.TabIndex = 10;
            tbAdjustLevel2.TickStyle = TickStyle.None;
            // 
            // lblAdjust2
            // 
            lblAdjust2.AutoSize = true;
            lblAdjust2.Location = new Point(14, 83);
            lblAdjust2.Name = "lblAdjust2";
            lblAdjust2.Size = new Size(54, 15);
            lblAdjust2.TabIndex = 9;
            lblAdjust2.Text = "Amount:";
            // 
            // udAdjustLevel
            // 
            udAdjustLevel.Location = new Point(324, 16);
            udAdjustLevel.Minimum = new decimal(new int[] { 100, 0, 0, int.MinValue });
            udAdjustLevel.Name = "udAdjustLevel";
            udAdjustLevel.Size = new Size(49, 23);
            udAdjustLevel.TabIndex = 8;
            // 
            // tbAdjustLevel
            // 
            tbAdjustLevel.AutoSize = false;
            tbAdjustLevel.Location = new Point(14, 45);
            tbAdjustLevel.Maximum = 100;
            tbAdjustLevel.Minimum = -100;
            tbAdjustLevel.Name = "tbAdjustLevel";
            tbAdjustLevel.Size = new Size(359, 24);
            tbAdjustLevel.TabIndex = 7;
            tbAdjustLevel.TickStyle = TickStyle.None;
            // 
            // lblAdjust
            // 
            lblAdjust.AutoSize = true;
            lblAdjust.Location = new Point(14, 18);
            lblAdjust.Name = "lblAdjust";
            lblAdjust.Size = new Size(54, 15);
            lblAdjust.TabIndex = 6;
            lblAdjust.Text = "Amount:";
            // 
            // BaseFilterDialog
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(523, 226);
            Controls.Add(panelBottom);
            Controls.Add(panelOptions);
            FormBorderStyle = FormBorderStyle.FixedDialog;
            MaximizeBox = false;
            MinimizeBox = false;
            Name = "BaseFilterDialog";
            Opacity = 0.9D;
            ShowIcon = false;
            ShowInTaskbar = false;
            StartPosition = FormStartPosition.CenterParent;
            FormClosed += BaseFilterDialog_FormClosed;
            Load += BaseFilterDialog_Load;
            panelBottom.ResumeLayout(false);
            panelBottom.PerformLayout();
            panelOptions.ResumeLayout(false);
            panelOptions.PerformLayout();
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
    }
}