using HomeControler.Objects;
using System.Collections.Generic;

namespace HomeControler.Others
{
    public class ReloadFromDatabaseMessage
    {
        public List<SubscribedLabel> subscribedLabels { get; set; }
        public List<SubscribedSwitch> subscribedSwitches { get; set; }
        public List <SubscribedCamera> subscribedCameras { get; set; }
    }
}
