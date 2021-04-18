using System.Windows.Controls;
using System.Windows.Controls.Primitives;

namespace HomeControler.Others
{
    public class MoveThumb : Thumb
    {
        public MoveThumb()
        {
            DragDelta += new DragDeltaEventHandler(this.MoveThumb_DragDelta);
        }

        private void MoveThumb_DragDelta(object sender, DragDeltaEventArgs e)
        {
            Control item = this.DataContext as Control;

            if (item != null)
            {
                double left = item.Margin.Left;
                double top = item.Margin.Top;
                item.Margin = new System.Windows.Thickness(left + e.HorizontalChange, top + e.VerticalChange, item.Margin.Right, item.Margin.Bottom);
            }
        }
    }
}
