namespace OcularDemo
{
    partial class HueDialog
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
            huePicker = new HuePicker();
            groupBox = new GroupBox();
            HueUpDown = new NumericUpDown();
            lblHue = new Label();
            btnCancel = new Button();
            btnOK = new Button();
            ChkPreview = new CheckBox();
            groupBox.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)HueUpDown).BeginInit();
            SuspendLayout();
            // 
            // huePicker
            // 
            huePicker.Location = new Point(15, 57);
            huePicker.Name = "huePicker";
            huePicker.Size = new Size(152, 149);
            huePicker.TabIndex = 0;
            huePicker.ValuesChanged += huePicker_ValuesChanged;
            // 
            // groupBox
            // 
            groupBox.Controls.Add(HueUpDown);
            groupBox.Controls.Add(lblHue);
            groupBox.Controls.Add(huePicker);
            groupBox.Location = new Point(12, 12);
            groupBox.Name = "groupBox";
            groupBox.Size = new Size(181, 218);
            groupBox.TabIndex = 1;
            groupBox.TabStop = false;
            groupBox.Text = "Options";
            // 
            // HueUpDown
            // 
            HueUpDown.Location = new Point(65, 28);
            HueUpDown.Maximum = new decimal(new int[] { 359, 0, 0, 0 });
            HueUpDown.Name = "HueUpDown";
            HueUpDown.Size = new Size(80, 23);
            HueUpDown.TabIndex = 2;
            HueUpDown.ValueChanged += HueUpDown_ValueChanged;
            // 
            // lblHue
            // 
            lblHue.AutoSize = true;
            lblHue.Location = new Point(15, 28);
            lblHue.Name = "lblHue";
            lblHue.Size = new Size(32, 15);
            lblHue.TabIndex = 1;
            lblHue.Text = "Hue:";
            // 
            // btnCancel
            // 
            btnCancel.DialogResult = DialogResult.Cancel;
            btnCancel.Location = new Point(209, 70);
            btnCancel.Name = "btnCancel";
            btnCancel.Size = new Size(100, 33);
            btnCancel.TabIndex = 7;
            btnCancel.Text = "Cancel";
            btnCancel.UseVisualStyleBackColor = true;
            btnCancel.Click += btnCancel_Click;
            // 
            // btnOK
            // 
            btnOK.DialogResult = DialogResult.OK;
            btnOK.Location = new Point(209, 22);
            btnOK.Name = "btnOK";
            btnOK.Size = new Size(100, 33);
            btnOK.TabIndex = 6;
            btnOK.Text = "OK";
            btnOK.UseVisualStyleBackColor = true;
            // 
            // ChkPreview
            // 
            ChkPreview.AutoSize = true;
            ChkPreview.Checked = true;
            ChkPreview.CheckState = CheckState.Checked;
            ChkPreview.Location = new Point(209, 128);
            ChkPreview.Name = "ChkPreview";
            ChkPreview.Size = new Size(67, 19);
            ChkPreview.TabIndex = 8;
            ChkPreview.Text = "Preview";
            ChkPreview.UseVisualStyleBackColor = true;
            ChkPreview.CheckedChanged += ChkPreview_CheckedChanged;
            // 
            // HueDialog
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(320, 239);
            Controls.Add(ChkPreview);
            Controls.Add(btnCancel);
            Controls.Add(btnOK);
            Controls.Add(groupBox);
            FormBorderStyle = FormBorderStyle.FixedDialog;
            MaximizeBox = false;
            MinimizeBox = false;
            Name = "HueDialog";
            ShowIcon = false;
            ShowInTaskbar = false;
            StartPosition = FormStartPosition.CenterParent;
            Text = "Modify Hue";
            FormClosed += HueDialog_FormClosed;
            groupBox.ResumeLayout(false);
            groupBox.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)HueUpDown).EndInit();
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private HuePicker huePicker;
        private GroupBox groupBox;
        private NumericUpDown HueUpDown;
        private Label lblHue;
        private Button btnCancel;
        private Button btnOK;
        private CheckBox ChkPreview;
    }
}