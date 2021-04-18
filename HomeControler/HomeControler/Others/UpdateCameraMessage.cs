using HomeControler.Objects;
using System.Windows.Media.Imaging;

namespace HomeControler.Others
{
    class UpdateCameraMessage
    {
        public SubscribedCamera subscribedCamera { get; set; }
        public BitmapImage value { get; set; }
    }
}
