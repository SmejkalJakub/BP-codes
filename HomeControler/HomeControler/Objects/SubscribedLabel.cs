using HomeControler.Controls;
using System;

namespace HomeControler.Objects
{
    [Serializable]
    public class SubscribedLabel : SubscribedDevice
    {
        [NonSerialized] public SimpleLabel Label;
        public string UniqueName { get; set; }
        public string Prefix { get; set; }
        public string Postfix { get; set; }

    }
}
