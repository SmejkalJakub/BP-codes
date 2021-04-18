using GalaSoft.MvvmLight;
using GalaSoft.MvvmLight.Messaging;
using HomeControler.Controls;

namespace HomeControler.ViewModels
{
    public class NodeViewModel : ViewModelBase
    {
        

        public NodeViewModel()
        {
            Messenger.Default.Register<SimpleLabel>(this, "labelSettings", showLabelData);
        }

        private void showLabelData(SimpleLabel labelData)
        {

        }
    }

}
