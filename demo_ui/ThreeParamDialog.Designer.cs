namespace OcularDemo
{
    partial class ThreeParamDialog
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
            groupbox = new GroupBox();
            AdjustLevelUpDown3 = new NumericUpDown();
            AdjustLevel3 = new TrackBar();
            lblAdjust3 = new Label();
            AdjustLevelUpDown2 = new NumericUpDown();
            AdjustLevel2 = new TrackBar();
            lblAdjust2 = new Label();
            AdjustLevelUpDown = new NumericUpDown();
            AdjustLevel = new TrackBar();
            lblAdjust = new Label();
            btnOK = new Button();
            btnCancel = new Button();
            ChkPreview = new CheckBox();
            groupbox.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)AdjustLevelUpDown3).BeginInit();
            ((System.ComponentModel.ISupportInitialize)AdjustLevel3).BeginInit();
            ((System.ComponentModel.ISupportInitialize)AdjustLevelUpDown2).BeginInit();
            ((System.ComponentModel.ISupportInitialize)AdjustLevel2).BeginInit();
            ((System.ComponentModel.ISupportInitialize)AdjustLevelUpDown).BeginInit();
            ((System.ComponentModel.ISupportInitialize)AdjustLevel).BeginInit();
            SuspendLayout();
            // 
            // groupbox
            // 
            groupbox.Controls.Add(AdjustLevelUpDown3);
            groupbox.Controls.Add(AdjustLevel3);
            groupbox.Controls.Add(lblAdjust3);
            groupbox.Controls.Add(AdjustLevelUpDown2);
            groupbox.Controls.Add(AdjustLevel2);
            groupbox.Controls.Add(lblAdjust2);
            groupbox.Controls.Add(AdjustLevelUpDown);
            groupbox.Controls.Add(AdjustLevel);
            groupbox.Controls.Add(lblAdjust);
            groupbox.Location = new Point(12, 12);
            groupbox.Name = "groupbox";
            groupbox.Size = new Size(379, 222);
            groupbox.TabIndex = 0;
            groupbox.TabStop = false;
            groupbox.Text = "Options";
            // 
            // AdjustLevelUpDown3
            // 
            AdjustLevelUpDown3.Location = new Point(316, 154);
            AdjustLevelUpDown3.Minimum = new decimal(new int[] { 100, 0, 0, int.MinValue });
            AdjustLevelUpDown3.Name = "AdjustLevelUpDown3";
            AdjustLevelUpDown3.Size = new Size(49, 23);
            AdjustLevelUpDown3.TabIndex = 8;
            AdjustLevelUpDown3.ValueChanged += AdjustLevelUpDown3_ValueChanged;
            // 
            // AdjustLevel3
            // 
            AdjustLevel3.AutoSize = false;
            AdjustLevel3.Location = new Point(10, 183);
            AdjustLevel3.Maximum = 100;
            AdjustLevel3.Minimum = -100;
            AdjustLevel3.Name = "AdjustLevel3";
            AdjustLevel3.Size = new Size(359, 24);
            AdjustLevel3.TabIndex = 7;
            AdjustLevel3.TickStyle = TickStyle.None;
            AdjustLevel3.Scroll += AdjustLevel3_Scroll;
            // 
            // lblAdjust3
            // 
            lblAdjust3.AutoSize = true;
            lblAdjust3.Location = new Point(10, 156);
            lblAdjust3.Name = "lblAdjust3";
            lblAdjust3.Size = new Size(54, 15);
            lblAdjust3.TabIndex = 6;
            lblAdjust3.Text = "Amount:";
            // 
            // AdjustLevelUpDown2
            // 
            AdjustLevelUpDown2.Location = new Point(316, 91);
            AdjustLevelUpDown2.Minimum = new decimal(new int[] { 100, 0, 0, int.MinValue });
            AdjustLevelUpDown2.Name = "AdjustLevelUpDown2";
            AdjustLevelUpDown2.Size = new Size(49, 23);
            AdjustLevelUpDown2.TabIndex = 5;
            AdjustLevelUpDown2.ValueChanged += AdjustLevelUpDown2_ValueChanged;
            // 
            // AdjustLevel2
            // 
            AdjustLevel2.AutoSize = false;
            AdjustLevel2.Location = new Point(10, 120);
            AdjustLevel2.Maximum = 100;
            AdjustLevel2.Minimum = -100;
            AdjustLevel2.Name = "AdjustLevel2";
            AdjustLevel2.Size = new Size(359, 24);
            AdjustLevel2.TabIndex = 4;
            AdjustLevel2.TickStyle = TickStyle.None;
            AdjustLevel2.Scroll += AdjustLevel2_Scroll;
            // 
            // lblAdjust2
            // 
            lblAdjust2.AutoSize = true;
            lblAdjust2.Location = new Point(10, 93);
            lblAdjust2.Name = "lblAdjust2";
            lblAdjust2.Size = new Size(54, 15);
            lblAdjust2.TabIndex = 3;
            lblAdjust2.Text = "Amount:";
            // 
            // AdjustLevelUpDown
            // 
            AdjustLevelUpDown.Location = new Point(316, 26);
            AdjustLevelUpDown.Minimum = new decimal(new int[] { 100, 0, 0, int.MinValue });
            AdjustLevelUpDown.Name = "AdjustLevelUpDown";
            AdjustLevelUpDown.Size = new Size(49, 23);
            AdjustLevelUpDown.TabIndex = 2;
            AdjustLevelUpDown.ValueChanged += AdjustLevelUpDown_ValueChanged;
            // 
            // AdjustLevel
            // 
            AdjustLevel.AutoSize = false;
            AdjustLevel.Location = new Point(6, 55);
            AdjustLevel.Maximum = 100;
            AdjustLevel.Minimum = -100;
            AdjustLevel.Name = "AdjustLevel";
            AdjustLevel.Size = new Size(359, 24);
            AdjustLevel.TabIndex = 1;
            AdjustLevel.TickStyle = TickStyle.None;
            AdjustLevel.Scroll += AdjustLevel_Scroll;
            // 
            // lblAdjust
            // 
            lblAdjust.AutoSize = true;
            lblAdjust.Location = new Point(6, 28);
            lblAdjust.Name = "lblAdjust";
            lblAdjust.Size = new Size(54, 15);
            lblAdjust.TabIndex = 0;
            lblAdjust.Text = "Amount:";
            // 
            // btnOK
            // 
            btnOK.DialogResult = DialogResult.OK;
            btnOK.Location = new Point(411, 22);
            btnOK.Name = "btnOK";
            btnOK.Size = new Size(100, 33);
            btnOK.TabIndex = 1;
            btnOK.Text = "OK";
            btnOK.UseVisualStyleBackColor = true;
            // 
            // btnCancel
            // 
            btnCancel.DialogResult = DialogResult.Cancel;
            btnCancel.Location = new Point(411, 70);
            btnCancel.Name = "btnCancel";
            btnCancel.Size = new Size(100, 33);
            btnCancel.TabIndex = 2;
            btnCancel.Text = "Cancel";
            btnCancel.UseVisualStyleBackColor = true;
            btnCancel.Click += btnCancel_Click;
            // 
            // ChkPreview
            // 
            ChkPreview.AutoSize = true;
            ChkPreview.Checked = true;
            ChkPreview.CheckState = CheckState.Checked;
            ChkPreview.Location = new Point(411, 121);
            ChkPreview.Name = "ChkPreview";
            ChkPreview.Size = new Size(67, 19);
            ChkPreview.TabIndex = 3;
            ChkPreview.Text = "Preview";
            ChkPreview.UseVisualStyleBackColor = true;
            ChkPreview.CheckedChanged += ChkPreview_CheckedChanged;
            // 
            // ThreeParamDialog
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(523, 245);
            Controls.Add(ChkPreview);
            Controls.Add(btnCancel);
            Controls.Add(btnOK);
            Controls.Add(groupbox);
            FormBorderStyle = FormBorderStyle.FixedDialog;
            MaximizeBox = false;
            MinimizeBox = false;
            Name = "ThreeParamDialog";
            Opacity = 0.9D;
            ShowIcon = false;
            ShowInTaskbar = false;
            StartPosition = FormStartPosition.CenterParent;
            FormClosed += ThreeParamDialog_FormClosed;
            Load += ThreeParamDialog_Load;
            groupbox.ResumeLayout(false);
            groupbox.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)AdjustLevelUpDown3).EndInit();
            ((System.ComponentModel.ISupportInitialize)AdjustLevel3).EndInit();
            ((System.ComponentModel.ISupportInitialize)AdjustLevelUpDown2).EndInit();
            ((System.ComponentModel.ISupportInitialize)AdjustLevel2).EndInit();
            ((System.ComponentModel.ISupportInitialize)AdjustLevelUpDown).EndInit();
            ((System.ComponentModel.ISupportInitialize)AdjustLevel).EndInit();
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private GroupBox groupbox;
        private Label lblAdjust;
        private Button btnOK;
        private Button btnCancel;
        private TrackBar AdjustLevel;
        private NumericUpDown AdjustLevelUpDown;
        private CheckBox ChkPreview;
        private NumericUpDown AdjustLevelUpDown2;
        private TrackBar AdjustLevel2;
        private Label lblAdjust2;
        private NumericUpDown AdjustLevelUpDown3;
        private TrackBar AdjustLevel3;
        private Label lblAdjust3;
    }
}