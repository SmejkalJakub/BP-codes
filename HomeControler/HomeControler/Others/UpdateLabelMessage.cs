using HomeControler.Objects;

namespace HomeControler.Others
{
    public class UpdateLabelMessage
    {
        public SubscribedLabel subscribedLabel { get; set; }
        public string value { get; set; }
    }
}
