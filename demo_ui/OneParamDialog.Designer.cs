namespace OcularDemo
{
    partial class OneParamDialog
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
            AdjustLevelUpDown = new NumericUpDown();
            AdjustLevel = new TrackBar();
            lblAdjust = new Label();
            btnOK = new Button();
            btnCancel = new Button();
            ChkPreview = new CheckBox();
            groupbox.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)AdjustLevelUpDown).BeginInit();
            ((System.ComponentModel.ISupportInitialize)AdjustLevel).BeginInit();
            SuspendLayout();
            // 
            // groupbox
            // 
            groupbox.Controls.Add(AdjustLevelUpDown);
            groupbox.Controls.Add(AdjustLevel);
            groupbox.Controls.Add(lblAdjust);
            groupbox.Location = new Point(12, 12);
            groupbox.Name = "groupbox";
            groupbox.Size = new Size(379, 100);
            groupbox.TabIndex = 0;
            groupbox.TabStop = false;
            groupbox.Text = "Options";
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
            ChkPreview.Location = new Point(411, 124);
            ChkPreview.Name = "ChkPreview";
            ChkPreview.Size = new Size(67, 19);
            ChkPreview.TabIndex = 3;
            ChkPreview.Text = "Preview";
            ChkPreview.UseVisualStyleBackColor = true;
            ChkPreview.CheckedChanged += ChkPreview_CheckedChanged;
            // 
            // OneParamDialog
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(523, 154);
            Controls.Add(ChkPreview);
            Controls.Add(btnCancel);
            Controls.Add(btnOK);
            Controls.Add(groupbox);
            FormBorderStyle = FormBorderStyle.FixedDialog;
            MaximizeBox = false;
            MinimizeBox = false;
            Name = "OneParamDialog";
            Opacity = 0.9D;
            ShowIcon = false;
            ShowInTaskbar = false;
            StartPosition = FormStartPosition.CenterParent;
            FormClosed += OneParamDialog_FormClosed;
            Load += OneParamDialog_Load;
            groupbox.ResumeLayout(false);
            groupbox.PerformLayout();
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
    }
}