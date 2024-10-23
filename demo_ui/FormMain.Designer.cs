using System.ComponentModel;

namespace OcularDemo
{
    partial class FormMain
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
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
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            components = new Container();
            ComponentResourceManager resources = new ComponentResourceManager(typeof(FormMain));
            Container = new ToolStripContainer();
            StatusBar = new StatusStrip();
            pathToolStripStatusLabel = new ToolStripStatusLabel();
            ZoomFactor = new ToolStripDropDownButton();
            zoomContextMenuStrip = new ContextMenuStrip(components);
            MenuZoom12 = new ToolStripMenuItem();
            MenuZoom25 = new ToolStripMenuItem();
            MenuZoom50 = new ToolStripMenuItem();
            MenuZoom100 = new ToolStripMenuItem();
            MenuZoom200 = new ToolStripMenuItem();
            MenuZoom300 = new ToolStripMenuItem();
            MenuZoom400 = new ToolStripMenuItem();
            MenuZoom500 = new ToolStripMenuItem();
            MenuZoom1000 = new ToolStripMenuItem();
            ChannelStatus = new ToolStripDropDownButton();
            ChannelMenu = new ContextMenuStrip(components);
            mouseToolStripStatusLabel = new ToolStripStatusLabel();
            sizeToolStripStatusLabel = new ToolStripStatusLabel();
            TimeUse = new ToolStripStatusLabel();
            FilePathStatus = new ToolStripStatusLabel();
            toolStripStatusLabel1 = new ToolStripStatusLabel();
            MainCanvas = new Canvas();
            ScrollH = new HScrollBar();
            ScrollV = new VScrollBar();
            MainMenu = new MenuStrip();
            fileToolStripMenuItem = new ToolStripMenuItem();
            MenuLoad = new ToolStripMenuItem();
            MenuSave = new ToolStripMenuItem();
            MenuExit = new ToolStripMenuItem();
            MenuEdit = new ToolStripMenuItem();
            MenuResample = new ToolStripMenuItem();
            MenuAdjust = new ToolStripMenuItem();
            MenuAutoLevel = new ToolStripMenuItem();
            MenuAutoWB = new ToolStripMenuItem();
            MenuAdjustBrightnessAndContrast = new ToolStripMenuItem();
            MenuExposure = new ToolStripMenuItem();
            gammaCorrectionToolStripMenuItem = new ToolStripMenuItem();
            MenuLut2D = new ToolStripMenuItem();
            MenuInvert = new ToolStripMenuItem();
            MenuHaze = new ToolStripMenuItem();
            MenuHue = new ToolStripMenuItem();
            MenuRGB = new ToolStripMenuItem();
            MenuSaturation = new ToolStripMenuItem();
            MenuVibrance = new ToolStripMenuItem();
            MenuThreshold = new ToolStripMenuItem();
            MenuWhiteBalance = new ToolStripMenuItem();
            MenuShadowAndHighlight = new ToolStripMenuItem();
            MenuDesaturate = new ToolStripMenuItem();
            MenuLevels = new ToolStripMenuItem();
            MenuSkinTone = new ToolStripMenuItem();
            MenuBlur = new ToolStripMenuItem();
            MenuBoxBlur = new ToolStripMenuItem();
            MenuGaussianBlur = new ToolStripMenuItem();
            MenuMotionBlur = new ToolStripMenuItem();
            MenuEnhance = new ToolStripMenuItem();
            denoiseToolStripMenuItem = new ToolStripMenuItem();
            MenuBilateral = new ToolStripMenuItem();
            MenuSharpen = new ToolStripMenuItem();
            MenuBasicSharpen = new ToolStripMenuItem();
            MenuUnsharpMask = new ToolStripMenuItem();
            MenuEffects = new ToolStripMenuItem();
            photoToolStripMenuItem = new ToolStripMenuItem();
            MenuSepia = new ToolStripMenuItem();
            MenuGrayMode = new ToolStripMenuItem();
            MenuBGR = new ToolStripMenuItem();
            MenuBGRA = new ToolStripMenuItem();
            openFileDialog1 = new OpenFileDialog();
            MenuRadialBlur = new ToolStripMenuItem();
            Container.BottomToolStripPanel.SuspendLayout();
            Container.ContentPanel.SuspendLayout();
            Container.SuspendLayout();
            StatusBar.SuspendLayout();
            zoomContextMenuStrip.SuspendLayout();
            MainMenu.SuspendLayout();
            SuspendLayout();
            // 
            // Container
            // 
            // 
            // Container.BottomToolStripPanel
            // 
            Container.BottomToolStripPanel.Controls.Add(StatusBar);
            // 
            // Container.ContentPanel
            // 
            Container.ContentPanel.BorderStyle = BorderStyle.FixedSingle;
            Container.ContentPanel.Controls.Add(MainCanvas);
            Container.ContentPanel.Controls.Add(ScrollH);
            Container.ContentPanel.Controls.Add(ScrollV);
            Container.ContentPanel.Size = new Size(1180, 752);
            Container.ContentPanel.MouseWheel += Canvas_MouseWheel;
            Container.Dock = DockStyle.Fill;
            Container.Location = new Point(0, 24);
            Container.Margin = new Padding(4);
            Container.Name = "Container";
            Container.Size = new Size(1180, 799);
            Container.TabIndex = 1;
            // 
            // StatusBar
            // 
            StatusBar.Dock = DockStyle.None;
            StatusBar.ImageScalingSize = new Size(20, 20);
            StatusBar.Items.AddRange(new ToolStripItem[] { pathToolStripStatusLabel, ZoomFactor, ChannelStatus, mouseToolStripStatusLabel, sizeToolStripStatusLabel, TimeUse, FilePathStatus, toolStripStatusLabel1 });
            StatusBar.LayoutStyle = ToolStripLayoutStyle.HorizontalStackWithOverflow;
            StatusBar.Location = new Point(0, 0);
            StatusBar.Name = "StatusBar";
            StatusBar.Size = new Size(1180, 22);
            StatusBar.TabIndex = 3;
            StatusBar.Text = "Status Bar";
            // 
            // pathToolStripStatusLabel
            // 
            pathToolStripStatusLabel.Name = "pathToolStripStatusLabel";
            pathToolStripStatusLabel.Size = new Size(0, 17);
            // 
            // ZoomFactor
            // 
            ZoomFactor.Alignment = ToolStripItemAlignment.Right;
            ZoomFactor.DisplayStyle = ToolStripItemDisplayStyle.Text;
            ZoomFactor.DropDown = zoomContextMenuStrip;
            ZoomFactor.ImageAlign = ContentAlignment.MiddleRight;
            ZoomFactor.ImageTransparentColor = Color.Magenta;
            ZoomFactor.Name = "ZoomFactor";
            ZoomFactor.RightToLeftAutoMirrorImage = true;
            ZoomFactor.Size = new Size(48, 20);
            ZoomFactor.Text = "100%";
            // 
            // zoomContextMenuStrip
            // 
            zoomContextMenuStrip.ImageScalingSize = new Size(20, 20);
            zoomContextMenuStrip.Items.AddRange(new ToolStripItem[] { MenuZoom12, MenuZoom25, MenuZoom50, MenuZoom100, MenuZoom200, MenuZoom300, MenuZoom400, MenuZoom500, MenuZoom1000 });
            zoomContextMenuStrip.Name = "contextMenuStrip_Zoom";
            zoomContextMenuStrip.OwnerItem = ZoomFactor;
            zoomContextMenuStrip.Size = new Size(109, 202);
            zoomContextMenuStrip.Text = "Zoom";
            // 
            // MenuZoom12
            // 
            MenuZoom12.Name = "MenuZoom12";
            MenuZoom12.Size = new Size(108, 22);
            MenuZoom12.Text = "12.5%";
            MenuZoom12.Click += MenuZoom12_Click;
            // 
            // MenuZoom25
            // 
            MenuZoom25.Name = "MenuZoom25";
            MenuZoom25.Size = new Size(108, 22);
            MenuZoom25.Text = "25%";
            MenuZoom25.Click += MenuZoom25_Click;
            // 
            // MenuZoom50
            // 
            MenuZoom50.Name = "MenuZoom50";
            MenuZoom50.Size = new Size(108, 22);
            MenuZoom50.Text = "50%";
            MenuZoom50.Click += MenuZoom50_Click;
            // 
            // MenuZoom100
            // 
            MenuZoom100.Checked = true;
            MenuZoom100.CheckState = CheckState.Checked;
            MenuZoom100.Name = "MenuZoom100";
            MenuZoom100.Size = new Size(108, 22);
            MenuZoom100.Text = "100%";
            MenuZoom100.Click += MenuZoom100_Click;
            // 
            // MenuZoom200
            // 
            MenuZoom200.Name = "MenuZoom200";
            MenuZoom200.Size = new Size(108, 22);
            MenuZoom200.Text = "200%";
            MenuZoom200.Click += MenuZoom200_Click;
            // 
            // MenuZoom300
            // 
            MenuZoom300.Name = "MenuZoom300";
            MenuZoom300.Size = new Size(108, 22);
            MenuZoom300.Text = "300%";
            MenuZoom300.Click += MenuZoom300_Click;
            // 
            // MenuZoom400
            // 
            MenuZoom400.Name = "MenuZoom400";
            MenuZoom400.Size = new Size(108, 22);
            MenuZoom400.Text = "400%";
            MenuZoom400.Click += MenuZoom400_Click;
            // 
            // MenuZoom500
            // 
            MenuZoom500.Name = "MenuZoom500";
            MenuZoom500.Size = new Size(108, 22);
            MenuZoom500.Text = "500%";
            MenuZoom500.Click += MenuZoom500_Click;
            // 
            // MenuZoom1000
            // 
            MenuZoom1000.Name = "MenuZoom1000";
            MenuZoom1000.Size = new Size(108, 22);
            MenuZoom1000.Text = "1000%";
            MenuZoom1000.Click += MenuZoom1000_Click;
            // 
            // ChannelStatus
            // 
            ChannelStatus.Alignment = ToolStripItemAlignment.Right;
            ChannelStatus.DisplayStyle = ToolStripItemDisplayStyle.Text;
            ChannelStatus.DropDown = ChannelMenu;
            ChannelStatus.ImageTransparentColor = Color.Magenta;
            ChannelStatus.Name = "ChannelStatus";
            ChannelStatus.RightToLeftAutoMirrorImage = true;
            ChannelStatus.Size = new Size(109, 20);
            ChannelStatus.Text = "RGB - 3 channels";
            // 
            // ChannelMenu
            // 
            ChannelMenu.Name = "ChannelMenu";
            ChannelMenu.OwnerItem = ChannelStatus;
            ChannelMenu.Size = new Size(61, 4);
            // 
            // mouseToolStripStatusLabel
            // 
            mouseToolStripStatusLabel.Alignment = ToolStripItemAlignment.Right;
            mouseToolStripStatusLabel.AutoSize = false;
            mouseToolStripStatusLabel.Name = "mouseToolStripStatusLabel";
            mouseToolStripStatusLabel.Size = new Size(300, 17);
            // 
            // sizeToolStripStatusLabel
            // 
            sizeToolStripStatusLabel.Alignment = ToolStripItemAlignment.Right;
            sizeToolStripStatusLabel.AutoSize = false;
            sizeToolStripStatusLabel.Name = "sizeToolStripStatusLabel";
            sizeToolStripStatusLabel.Size = new Size(150, 17);
            // 
            // TimeUse
            // 
            TimeUse.Alignment = ToolStripItemAlignment.Right;
            TimeUse.AutoSize = false;
            TimeUse.BorderSides = ToolStripStatusLabelBorderSides.Left;
            TimeUse.BorderStyle = Border3DStyle.RaisedOuter;
            TimeUse.Name = "TimeUse";
            TimeUse.Size = new Size(500, 17);
            // 
            // FilePathStatus
            // 
            FilePathStatus.Name = "FilePathStatus";
            FilePathStatus.Size = new Size(0, 17);
            // 
            // toolStripStatusLabel1
            // 
            toolStripStatusLabel1.Name = "toolStripStatusLabel1";
            toolStripStatusLabel1.Size = new Size(83, 17);
            toolStripStatusLabel1.Text = "ChannelStatus";
            // 
            // MainCanvas
            // 
            MainCanvas.BackColor = SystemColors.ButtonHighlight;
            MainCanvas.BackgroundImage = (Image)resources.GetObject("MainCanvas.BackgroundImage");
            MainCanvas.Channel = 3;
            MainCanvas.Image = null;
            MainCanvas.ImageFile = "";
            MainCanvas.Location = new Point(286, 164);
            MainCanvas.Margin = new Padding(4, 4, 4, 4);
            MainCanvas.Name = "MainCanvas";
            MainCanvas.Size = new Size(521, 379);
            MainCanvas.TabIndex = 2;
            MainCanvas.Zoom = 1D;
            MainCanvas.MouseMove += MainCanvas_MouseMove;
            MainCanvas.Resize += MainCanvas_Resize;
            // 
            // ScrollH
            // 
            ScrollH.Dock = DockStyle.Bottom;
            ScrollH.Location = new Point(0, 734);
            ScrollH.Name = "ScrollH";
            ScrollH.Size = new Size(1162, 16);
            ScrollH.TabIndex = 1;
            ScrollH.Visible = false;
            ScrollH.Scroll += ScrollH_Scroll;
            // 
            // ScrollV
            // 
            ScrollV.Dock = DockStyle.Right;
            ScrollV.Location = new Point(1162, 0);
            ScrollV.Name = "ScrollV";
            ScrollV.Size = new Size(16, 750);
            ScrollV.TabIndex = 0;
            ScrollV.Visible = false;
            ScrollV.Scroll += ScrollV_Scroll;
            // 
            // MainMenu
            // 
            MainMenu.AccessibleRole = AccessibleRole.MenuBar;
            MainMenu.Items.AddRange(new ToolStripItem[] { fileToolStripMenuItem, MenuEdit, MenuAdjust, MenuBlur, MenuEnhance, MenuEffects });
            MainMenu.Location = new Point(0, 0);
            MainMenu.Name = "MainMenu";
            MainMenu.Padding = new Padding(7, 2, 0, 2);
            MainMenu.Size = new Size(1180, 24);
            MainMenu.TabIndex = 0;
            MainMenu.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            fileToolStripMenuItem.DropDownItems.AddRange(new ToolStripItem[] { MenuLoad, MenuSave, MenuExit });
            fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            fileToolStripMenuItem.Size = new Size(37, 20);
            fileToolStripMenuItem.Text = "File";
            // 
            // MenuLoad
            // 
            MenuLoad.Name = "MenuLoad";
            MenuLoad.Size = new Size(103, 22);
            MenuLoad.Text = "Load ";
            MenuLoad.Click += MenuLoad_Click;
            // 
            // MenuSave
            // 
            MenuSave.Name = "MenuSave";
            MenuSave.Size = new Size(103, 22);
            MenuSave.Text = "Save";
            // 
            // MenuExit
            // 
            MenuExit.Name = "MenuExit";
            MenuExit.Size = new Size(103, 22);
            MenuExit.Text = "Exit";
            MenuExit.Click += MenuExit_Click;
            // 
            // MenuEdit
            // 
            MenuEdit.DropDownItems.AddRange(new ToolStripItem[] { MenuResample });
            MenuEdit.Name = "MenuEdit";
            MenuEdit.Size = new Size(39, 20);
            MenuEdit.Text = "Edit";
            // 
            // MenuResample
            // 
            MenuResample.Name = "MenuResample";
            MenuResample.Size = new Size(125, 22);
            MenuResample.Text = "Resample";
            MenuResample.Click += MenuResample_Click;
            // 
            // MenuAdjust
            // 
            MenuAdjust.DropDownItems.AddRange(new ToolStripItem[] { MenuAutoLevel, MenuAutoWB, MenuAdjustBrightnessAndContrast, MenuExposure, gammaCorrectionToolStripMenuItem, MenuLut2D, MenuInvert, MenuHaze, MenuHue, MenuRGB, MenuSaturation, MenuVibrance, MenuThreshold, MenuWhiteBalance, MenuShadowAndHighlight, MenuDesaturate, MenuLevels, MenuSkinTone });
            MenuAdjust.Name = "MenuAdjust";
            MenuAdjust.Size = new Size(53, 20);
            MenuAdjust.Text = "Adjust";
            // 
            // MenuAutoLevel
            // 
            MenuAutoLevel.Name = "MenuAutoLevel";
            MenuAutoLevel.Size = new Size(200, 22);
            MenuAutoLevel.Text = "Auto Level";
            MenuAutoLevel.Click += MenuAutoLevel_Click;
            // 
            // MenuAutoWB
            // 
            MenuAutoWB.Name = "MenuAutoWB";
            MenuAutoWB.Size = new Size(200, 22);
            MenuAutoWB.Text = "Auto White Balance";
            MenuAutoWB.Click += MenuAutoWB_Click;
            // 
            // MenuAdjustBrightnessAndContrast
            // 
            MenuAdjustBrightnessAndContrast.Name = "MenuAdjustBrightnessAndContrast";
            MenuAdjustBrightnessAndContrast.Size = new Size(200, 22);
            MenuAdjustBrightnessAndContrast.Text = "Brightness and Contrast";
            MenuAdjustBrightnessAndContrast.Click += MenuAdjustBrightnessAndContrast_Click;
            // 
            // MenuExposure
            // 
            MenuExposure.Name = "MenuExposure";
            MenuExposure.Size = new Size(200, 22);
            MenuExposure.Text = "Exposure";
            MenuExposure.Click += MenuExposure_Click;
            // 
            // gammaCorrectionToolStripMenuItem
            // 
            gammaCorrectionToolStripMenuItem.Name = "gammaCorrectionToolStripMenuItem";
            gammaCorrectionToolStripMenuItem.Size = new Size(200, 22);
            gammaCorrectionToolStripMenuItem.Text = "Gamma Correction";
            gammaCorrectionToolStripMenuItem.Click += MenuGammaCorrection_Click;
            // 
            // MenuLut2D
            // 
            MenuLut2D.Name = "MenuLut2D";
            MenuLut2D.Size = new Size(200, 22);
            MenuLut2D.Text = "Color Mapping";
            MenuLut2D.Click += MenuLut2D_Click;
            // 
            // MenuInvert
            // 
            MenuInvert.Name = "MenuInvert";
            MenuInvert.Size = new Size(200, 22);
            MenuInvert.Text = "Invert";
            MenuInvert.Click += MenuInvert_Click;
            // 
            // MenuHaze
            // 
            MenuHaze.Name = "MenuHaze";
            MenuHaze.Size = new Size(200, 22);
            MenuHaze.Text = "Haze (Add/Remove)";
            // 
            // MenuHue
            // 
            MenuHue.Name = "MenuHue";
            MenuHue.Size = new Size(200, 22);
            MenuHue.Text = "Hue";
            MenuHue.Click += MenuHue_Click;
            // 
            // MenuRGB
            // 
            MenuRGB.Name = "MenuRGB";
            MenuRGB.Size = new Size(200, 22);
            MenuRGB.Text = "Red/Green/Blue";
            MenuRGB.Click += MenuRGB_Click;
            // 
            // MenuSaturation
            // 
            MenuSaturation.Name = "MenuSaturation";
            MenuSaturation.Size = new Size(200, 22);
            MenuSaturation.Text = "Saturation";
            MenuSaturation.Click += MenuSaturation_Click;
            // 
            // MenuVibrance
            // 
            MenuVibrance.Name = "MenuVibrance";
            MenuVibrance.Size = new Size(200, 22);
            MenuVibrance.Text = "Vibrance";
            MenuVibrance.Click += MenuVibrance_Click;
            // 
            // MenuThreshold
            // 
            MenuThreshold.Name = "MenuThreshold";
            MenuThreshold.Size = new Size(200, 22);
            MenuThreshold.Text = "Threshold";
            MenuThreshold.Click += MenuThreshold_Click;
            // 
            // MenuWhiteBalance
            // 
            MenuWhiteBalance.Name = "MenuWhiteBalance";
            MenuWhiteBalance.Size = new Size(200, 22);
            MenuWhiteBalance.Text = "White Balance";
            MenuWhiteBalance.Click += MenuWhiteBalance_Click;
            // 
            // MenuShadowAndHighlight
            // 
            MenuShadowAndHighlight.Name = "MenuShadowAndHighlight";
            MenuShadowAndHighlight.Size = new Size(200, 22);
            MenuShadowAndHighlight.Text = "Highlight/Shadow";
            MenuShadowAndHighlight.Click += MenuShadowAndHighlight_Click;
            // 
            // MenuDesaturate
            // 
            MenuDesaturate.Name = "MenuDesaturate";
            MenuDesaturate.Size = new Size(200, 22);
            MenuDesaturate.Text = "Desaturate";
            MenuDesaturate.Click += MenuDesaturate_Click;
            // 
            // MenuLevels
            // 
            MenuLevels.Name = "MenuLevels";
            MenuLevels.Size = new Size(200, 22);
            MenuLevels.Text = "Levels";
            // 
            // MenuSkinTone
            // 
            MenuSkinTone.Name = "MenuSkinTone";
            MenuSkinTone.Size = new Size(200, 22);
            MenuSkinTone.Text = "Skin Tone";
            MenuSkinTone.Click += MenuSkinTone_Click;
            // 
            // MenuBlur
            // 
            MenuBlur.DropDownItems.AddRange(new ToolStripItem[] { MenuBoxBlur, MenuGaussianBlur, MenuMotionBlur, MenuRadialBlur });
            MenuBlur.Name = "MenuBlur";
            MenuBlur.Size = new Size(40, 20);
            MenuBlur.Text = "Blur";
            // 
            // MenuBoxBlur
            // 
            MenuBoxBlur.Name = "MenuBoxBlur";
            MenuBoxBlur.Size = new Size(180, 22);
            MenuBoxBlur.Text = "Box Blur";
            MenuBoxBlur.Click += MenuBoxBlur_Click;
            // 
            // MenuGaussianBlur
            // 
            MenuGaussianBlur.Name = "MenuGaussianBlur";
            MenuGaussianBlur.Size = new Size(180, 22);
            MenuGaussianBlur.Text = "Gaussian Blur";
            MenuGaussianBlur.Click += MenuGaussianBlur_Click;
            // 
            // MenuMotionBlur
            // 
            MenuMotionBlur.Name = "MenuMotionBlur";
            MenuMotionBlur.Size = new Size(180, 22);
            MenuMotionBlur.Text = "Motion Blur";
            MenuMotionBlur.Click += MenuMotionBlur_Click;
            // 
            // MenuEnhance
            // 
            MenuEnhance.DropDownItems.AddRange(new ToolStripItem[] { denoiseToolStripMenuItem, MenuSharpen });
            MenuEnhance.Name = "MenuEnhance";
            MenuEnhance.Size = new Size(64, 20);
            MenuEnhance.Text = "Enhance";
            // 
            // denoiseToolStripMenuItem
            // 
            denoiseToolStripMenuItem.DropDownItems.AddRange(new ToolStripItem[] { MenuBilateral });
            denoiseToolStripMenuItem.Name = "denoiseToolStripMenuItem";
            denoiseToolStripMenuItem.Size = new Size(180, 22);
            denoiseToolStripMenuItem.Text = "Denoise";
            // 
            // MenuBilateral
            // 
            MenuBilateral.Name = "MenuBilateral";
            MenuBilateral.Size = new Size(116, 22);
            MenuBilateral.Text = "Bilateral";
            MenuBilateral.Click += MenuBilateral_Click;
            // 
            // MenuSharpen
            // 
            MenuSharpen.DropDownItems.AddRange(new ToolStripItem[] { MenuBasicSharpen, MenuUnsharpMask });
            MenuSharpen.Name = "MenuSharpen";
            MenuSharpen.Size = new Size(180, 22);
            MenuSharpen.Text = "Sharpen";
            // 
            // MenuBasicSharpen
            // 
            MenuBasicSharpen.Name = "MenuBasicSharpen";
            MenuBasicSharpen.Size = new Size(149, 22);
            MenuBasicSharpen.Text = "Sharpen";
            MenuBasicSharpen.Click += MenuBasicSharpen_Click;
            // 
            // MenuUnsharpMask
            // 
            MenuUnsharpMask.Name = "MenuUnsharpMask";
            MenuUnsharpMask.Size = new Size(149, 22);
            MenuUnsharpMask.Text = "Unsharp Mask";
            MenuUnsharpMask.Click += MenuUnsharpMask_Click;
            // 
            // MenuEffects
            // 
            MenuEffects.DropDownItems.AddRange(new ToolStripItem[] { photoToolStripMenuItem });
            MenuEffects.Name = "MenuEffects";
            MenuEffects.Size = new Size(54, 20);
            MenuEffects.Text = "Effects";
            // 
            // photoToolStripMenuItem
            // 
            photoToolStripMenuItem.DropDownItems.AddRange(new ToolStripItem[] { MenuSepia });
            photoToolStripMenuItem.Name = "photoToolStripMenuItem";
            photoToolStripMenuItem.Size = new Size(106, 22);
            photoToolStripMenuItem.Text = "Photo";
            // 
            // MenuSepia
            // 
            MenuSepia.Name = "MenuSepia";
            MenuSepia.Size = new Size(141, 22);
            MenuSepia.Text = "Sepia Toning";
            MenuSepia.Click += MenuSepia_Click;
            // 
            // MenuGrayMode
            // 
            MenuGrayMode.Name = "MenuGrayMode";
            MenuGrayMode.Size = new Size(153, 22);
            MenuGrayMode.Text = "GrayMode";
            MenuGrayMode.Click += MenuGrayMode_Click;
            // 
            // MenuBGR
            // 
            MenuBGR.Name = "MenuBGR";
            MenuBGR.Size = new Size(153, 22);
            MenuBGR.Text = "BGR - 3 channel";
            MenuBGR.Click += MenuBGR_Click;
            // 
            // MenuBGRA
            // 
            MenuBGRA.Name = "MenuBGRA";
            MenuBGRA.Size = new Size(153, 22);
            MenuBGRA.Text = "BGRA - 4 channel";
            MenuBGRA.Click += MenuBGRA_Click;
            // 
            // openFileDialog1
            // 
            openFileDialog1.FileName = "openFileDialog1";
            // 
            // MenuRadialBlur
            // 
            MenuRadialBlur.Name = "MenuRadialBlur";
            MenuRadialBlur.Size = new Size(180, 22);
            MenuRadialBlur.Text = "Radial Blur";
            MenuRadialBlur.Click += MenuRadialBlur_Click;
            // 
            // FormMain
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(1180, 823);
            Controls.Add(Container);
            Controls.Add(MainMenu);
            MainMenuStrip = MainMenu;
            Margin = new Padding(4);
            Name = "FormMain";
            StartPosition = FormStartPosition.CenterScreen;
            Text = "Ocular Image Processing Library Demo";
            Load += FormMain_Load;
            Container.BottomToolStripPanel.ResumeLayout(false);
            Container.BottomToolStripPanel.PerformLayout();
            Container.ContentPanel.ResumeLayout(false);
            Container.ResumeLayout(false);
            Container.PerformLayout();
            StatusBar.ResumeLayout(false);
            StatusBar.PerformLayout();
            zoomContextMenuStrip.ResumeLayout(false);
            MainMenu.ResumeLayout(false);
            MainMenu.PerformLayout();
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private ToolStripContainer Container;
        private HScrollBar ScrollH;
        private VScrollBar ScrollV;
        protected internal Canvas MainCanvas;
        private MenuStrip MainMenu;
        private StatusStrip StatusBar;
        private ToolStripStatusLabel pathToolStripStatusLabel;
        private ToolStripStatusLabel mouseToolStripStatusLabel;
        private ToolStripStatusLabel sizeToolStripStatusLabel;
        private ToolStripStatusLabel TimeUse;
        private ToolStripStatusLabel FilePathStatus;
        private ContextMenuStrip zoomContextMenuStrip;
        private ToolStripMenuItem MenuZoom12;
        private ToolStripMenuItem MenuZoom400;
        private ToolStripMenuItem MenuZoom500;
        private ToolStripDropDownButton ZoomFactor;
        private ToolStripMenuItem MenuZoom25;
        private ToolStripMenuItem MenuZoom50;
        private ToolStripMenuItem MenuZoom100;
        private ToolStripMenuItem MenuZoom200;
        private ToolStripMenuItem MenuZoom300;
        private ToolStripMenuItem MenuZoom1000;
        private ContextMenuStrip ChannelMenu;
        private ToolStripMenuItem MenuGrayMode;
        private ToolStripMenuItem MenuBGR;
        private ToolStripMenuItem MenuBGRA;
        private ToolStripDropDownButton ChannelStatus;
        private ToolStripMenuItem fileToolStripMenuItem;
        private ToolStripMenuItem MenuLoad;
        private ToolStripMenuItem MenuSave;
        private ToolStripMenuItem MenuExit;
        private OpenFileDialog openFileDialog1;
        private ToolStripStatusLabel toolStripStatusLabel1;
        private ToolStripMenuItem MenuAdjust;
        private ToolStripMenuItem MenuExposure;
        private ToolStripMenuItem MenuVibrance;
        private ToolStripMenuItem MenuAdjustBrightnessAndContrast;
        private ToolStripMenuItem MenuWhiteBalance;
        private ToolStripMenuItem MenuThreshold;
        private ToolStripMenuItem MenuBlur;
        private ToolStripMenuItem MenuGaussianBlur;
        private ToolStripMenuItem MenuBoxBlur;
        private ToolStripMenuItem MenuEnhance;
        private ToolStripMenuItem denoiseToolStripMenuItem;
        private ToolStripMenuItem MenuBilateral;
        private ToolStripMenuItem MenuEdit;
        private ToolStripMenuItem MenuResample;
        private ToolStripMenuItem MenuSharpen;
        private ToolStripMenuItem MenuBasicSharpen;
        private ToolStripMenuItem MenuUnsharpMask;
        private ToolStripMenuItem MenuRGB;
        private ToolStripMenuItem MenuEffects;
        private ToolStripMenuItem photoToolStripMenuItem;
        private ToolStripMenuItem MenuSepia;
        private ToolStripMenuItem MenuSaturation;
        private ToolStripMenuItem MenuHue;
        private ToolStripMenuItem MenuHaze;
        private ToolStripMenuItem MenuShadowAndHighlight;
        private ToolStripMenuItem MenuLut2D;
        private ToolStripMenuItem MenuInvert;
        private ToolStripMenuItem MenuDesaturate;
        private ToolStripMenuItem MenuAutoLevel;
        private ToolStripMenuItem MenuAutoWB;
        private ToolStripMenuItem gammaCorrectionToolStripMenuItem;
        private ToolStripMenuItem MenuLevels;
        private ToolStripMenuItem MenuMotionBlur;
        private ToolStripMenuItem MenuSkinTone;
        private ToolStripMenuItem MenuRadialBlur;
    }
}
