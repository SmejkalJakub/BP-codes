using GalaSoft.MvvmLight.Messaging;
using HomeControler.Objects;
using HomeControler.Others;
using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using uPLibrary.Networking.M2Mqtt;
using uPLibrary.Networking.M2Mqtt.Messages;

namespace HomeControler.Views
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {

        public MainWindow()
        {
            InitializeComponent();
        }

        private void AddLabel(object sender, RoutedEventArgs e)
        {
            Messenger.Default.Send(sender, "addLabel");
        }
        private void AddSwitch(object sender, RoutedEventArgs e)
        {
            Messenger.Default.Send(sender, "addSwitch");
        }

        private void AddCamera(object sender, RoutedEventArgs e)
        {
            Messenger.Default.Send(sender, "addCamera");
        }
    }
}
