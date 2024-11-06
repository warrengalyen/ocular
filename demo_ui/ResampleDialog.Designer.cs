namespace OcularDemo
{
    partial class ResampleDialog
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
            btnCancel = new Button();
            btnOK = new Button();
            groupbox = new GroupBox();
            CmbMethod = new ComboBox();
            lblSrcH = new Label();
            lblNew = new Label();
            lblSrcW = new Label();
            lblOriginal = new Label();
            label1 = new Label();
            DstHUpDown = new NumericUpDown();
            lblHeight = new Label();
            DstWUpDown = new NumericUpDown();
            lblWidth = new Label();
            groupbox.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)DstHUpDown).BeginInit();
            ((System.ComponentModel.ISupportInitialize)DstWUpDown).BeginInit();
            SuspendLayout();
            // 
            // btnCancel
            // 
            btnCancel.DialogResult = DialogResult.Cancel;
            btnCancel.Location = new Point(303, 70);
            btnCancel.Name = "btnCancel";
            btnCancel.Size = new Size(100, 33);
            btnCancel.TabIndex = 5;
            btnCancel.Text = "Cancel";
            btnCancel.UseVisualStyleBackColor = true;
            btnCancel.Click += btnCancel_Click;
            // 
            // btnOK
            // 
            btnOK.DialogResult = DialogResult.OK;
            btnOK.Location = new Point(303, 22);
            btnOK.Name = "btnOK";
            btnOK.Size = new Size(100, 33);
            btnOK.TabIndex = 4;
            btnOK.Text = "OK";
            btnOK.UseVisualStyleBackColor = true;
            btnOK.Click += btnOK_Click;
            // 
            // groupbox
            // 
            groupbox.Controls.Add(CmbMethod);
            groupbox.Controls.Add(lblSrcH);
            groupbox.Controls.Add(lblNew);
            groupbox.Controls.Add(lblSrcW);
            groupbox.Controls.Add(lblOriginal);
            groupbox.Controls.Add(label1);
            groupbox.Controls.Add(DstHUpDown);
            groupbox.Controls.Add(lblHeight);
            groupbox.Controls.Add(DstWUpDown);
            groupbox.Controls.Add(lblWidth);
            groupbox.Location = new Point(12, 12);
            groupbox.Name = "groupbox";
            groupbox.Size = new Size(273, 185);
            groupbox.TabIndex = 3;
            groupbox.TabStop = false;
            groupbox.Text = "Options";
            // 
            // CmbMethod
            // 
            CmbMethod.DropDownStyle = ComboBoxStyle.DropDownList;
            CmbMethod.FormattingEnabled = true;
            CmbMethod.Items.AddRange(new object[] { "Lanczos" });
            CmbMethod.Location = new Point(78, 145);
            CmbMethod.Name = "CmbMethod";
            CmbMethod.Size = new Size(121, 23);
            CmbMethod.TabIndex = 11;
            CmbMethod.SelectedIndexChanged += CmbMethod_SelectedIndexChanged;
            // 
            // lblSrcH
            // 
            lblSrcH.Location = new Point(78, 93);
            lblSrcH.Name = "lblSrcH";
            lblSrcH.Size = new Size(49, 15);
            lblSrcH.TabIndex = 10;
            lblSrcH.Text = "100";
            lblSrcH.TextAlign = ContentAlignment.MiddleCenter;
            // 
            // lblNew
            // 
            lblNew.AutoSize = true;
            lblNew.Location = new Point(206, 19);
            lblNew.Name = "lblNew";
            lblNew.Size = new Size(31, 15);
            lblNew.TabIndex = 9;
            lblNew.Text = "New";
            // 
            // lblSrcW
            // 
            lblSrcW.Location = new Point(78, 58);
            lblSrcW.Name = "lblSrcW";
            lblSrcW.Size = new Size(49, 15);
            lblSrcW.TabIndex = 8;
            lblSrcW.Text = "100";
            lblSrcW.TextAlign = ContentAlignment.MiddleCenter;
            // 
            // lblOriginal
            // 
            lblOriginal.AutoSize = true;
            lblOriginal.Location = new Point(78, 19);
            lblOriginal.Name = "lblOriginal";
            lblOriginal.Size = new Size(49, 15);
            lblOriginal.TabIndex = 7;
            lblOriginal.Text = "Original";
            // 
            // label1
            // 
            label1.AutoSize = true;
            label1.Location = new Point(10, 148);
            label1.Name = "label1";
            label1.Size = new Size(52, 15);
            label1.TabIndex = 6;
            label1.Text = "Method:";
            // 
            // DstHUpDown
            // 
            DstHUpDown.Location = new Point(206, 91);
            DstHUpDown.Minimum = new decimal(new int[] { 100, 0, 0, int.MinValue });
            DstHUpDown.Name = "DstHUpDown";
            DstHUpDown.Size = new Size(49, 23);
            DstHUpDown.TabIndex = 5;
            // 
            // lblHeight
            // 
            lblHeight.AutoSize = true;
            lblHeight.Location = new Point(10, 93);
            lblHeight.Name = "lblHeight";
            lblHeight.Size = new Size(46, 15);
            lblHeight.TabIndex = 3;
            lblHeight.Text = "Height:";
            // 
            // DstWUpDown
            // 
            DstWUpDown.Location = new Point(206, 58);
            DstWUpDown.Minimum = new decimal(new int[] { 100, 0, 0, int.MinValue });
            DstWUpDown.Name = "DstWUpDown";
            DstWUpDown.Size = new Size(49, 23);
            DstWUpDown.TabIndex = 2;
            // 
            // lblWidth
            // 
            lblWidth.AutoSize = true;
            lblWidth.Location = new Point(10, 58);
            lblWidth.Name = "lblWidth";
            lblWidth.Size = new Size(42, 15);
            lblWidth.TabIndex = 0;
            lblWidth.Text = "Width:";
            // 
            // ResampleDialog
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(412, 208);
            Controls.Add(btnCancel);
            Controls.Add(btnOK);
            Controls.Add(groupbox);
            FormBorderStyle = FormBorderStyle.FixedDialog;
            MaximizeBox = false;
            MinimizeBox = false;
            Name = "ResampleDialog";
            Opacity = 0.9D;
            ShowIcon = false;
            ShowInTaskbar = false;
            SizeGripStyle = SizeGripStyle.Hide;
            StartPosition = FormStartPosition.CenterParent;
            Text = "Resize";
            FormClosing += ResampleDialog_FormClosing;
            Load += ResampleDialog_Load;
            groupbox.ResumeLayout(false);
            groupbox.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)DstHUpDown).EndInit();
            ((System.ComponentModel.ISupportInitialize)DstWUpDown).EndInit();
            ResumeLayout(false);
        }

        #endregion

        private Button btnCancel;
        private Button btnOK;
        private GroupBox groupbox;
        private Label label1;
        private NumericUpDown AdjustLevelUpDown2;
        private TrackBar AdjustLevel2;
        private Label lblHeight;
        private NumericUpDown DstWUpDown;
        private NumericUpDown DstHUpDown;
        private TrackBar AdjustLevel;
        private Label lblWidth;
        private Label lblSrcW;
        private Label lblOriginal;
        private Label lblSrcH;
        private Label lblNew;
        private ComboBox CmbMethod;
    }
}