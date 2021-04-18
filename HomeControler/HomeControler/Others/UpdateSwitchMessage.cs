using HomeControler.Objects;

namespace HomeControler.Others
{
    class UpdateSwitchMessage
    {
        public SubscribedSwitch subscribedSwitch { get; set; }
        public string value { get; set; }
    }
}
