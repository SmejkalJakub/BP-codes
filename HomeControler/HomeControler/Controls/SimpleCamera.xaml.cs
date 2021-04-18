using GalaSoft.MvvmLight.Messaging;
using System.Windows.Controls;
using System.Windows.Input;

namespace HomeControler.Controls
{
    /// <summary>
    /// Interaction logic for SimpleCamera.xaml
    /// </summary>
    public partial class SimpleCamera : UserControl
    {
        public SimpleCamera()
        {
            InitializeComponent();
        }

        private void camera_MouseEnter(object sender, MouseEventArgs e)
        {
            Image img = sender as Image;

            Messenger.Default.Send(img.Name, "cameraMouseOver");
        }

        private void camera_MouseLeave(object sender, MouseEventArgs e)
        {
            Image img = sender as Image;

            Messenger.Default.Send(img.Name, "cameraMouseOverLeave");
        }
    }
}
