namespace OcularDemo.Controls
{
    public sealed class CurveControlRgb : CurveControl
    {
        public CurveControlRgb()
            : base(3, 256)
        {
            mask = new bool[3] { true, true, true };
            visualColors = new Color[] {
                                               Color.Red,
                                               Color.Green,
                                               Color.Blue
                                           };
            channelNames = new string[]{
                "Red", "Green", "Blue"
            };
            ResetControlPoints();
        }

        public override ColorTransferMode ColorTransferMode
        {
            get
            {
                return ColorTransferMode.Rgb;
            }
        }
    }
}
