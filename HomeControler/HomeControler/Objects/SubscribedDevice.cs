using System;

namespace HomeControler.Objects
{
    [Serializable]
    public class SubscribedDevice
    {
        public Guid Id { get; set; }
        public string Name { get; set; }
        public string Topic { get; set; }
    }
}
