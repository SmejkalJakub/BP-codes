using HomeControler.Controls;
using System;
using System.Windows.Media.Imaging;

namespace HomeControler.Objects
{
    [Serializable]
    public class SubscribedCamera : SubscribedDevice
    {
        public string UniqueName { get; set; }

        [NonSerialized] public BitmapImage lastImage;

        [NonSerialized] public SimpleCamera Camera;

    }
}
