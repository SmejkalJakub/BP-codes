using GalaSoft.MvvmLight;
using GalaSoft.MvvmLight.Command;
using GalaSoft.MvvmLight.Messaging;
using HomeControler.Controls;
using HomeControler.Models;
using HomeControler.Objects;
using HomeControler.Others;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Net;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Input;
using System.Windows.Media.Imaging;
using uPLibrary.Networking.M2Mqtt;
using uPLibrary.Networking.M2Mqtt.Messages;

namespace HomeControler.ViewModels
{
    public class MainViewModel : ViewModelBase
    {
        private readonly ViewModelLocator _locator = new ViewModelLocator();
        public ICommand SettingsCommand => new RelayCommand(SettingsView);
        public ICommand DashboardCommand => new RelayCommand(Dashboard);
        public ICommand DatabaseCommand => new RelayCommand(Database);
        public ICommand DatabaseChartCommand => new RelayCommand(DatabaseChart);

        MqttClient client;

        List<string> subscribedStrings = new List<string>();

        List<SubscribedLabel> subscribedLabels = new List<SubscribedLabel>();
        List<SubscribedSwitch> subscribedSwitches = new List<SubscribedSwitch>();
        List<SubscribedCamera> subscribedCameras = new List<SubscribedCamera>();

        private ViewModelBase _currentViewModel;
        public ViewModelBase CurrentViewModel
        {
            get
            {
                return _currentViewModel;
            }
            set
            {
                if (_currentViewModel == value)
                    return;
                _currentViewModel = value;
                RaisePropertyChanged();
            }
        }


        public MainViewModel()
        {
            Messenger.Default.Register<SettingsModel>(this, ConnectToMQTT);
            Messenger.Default.Register<object>(this, "updateObject", CreateOrUpdateSubscribedDevice);
            Messenger.Default.Register<SubscribedLabel>(this, "labelAdd", AddLabel);
            Messenger.Default.Register<SubscribedSwitch>(this, "switchAdd", AddSwitch);
            Messenger.Default.Register<SubscribedCamera>(this, "cameraAdd", AddCamera);

            Messenger.Default.Register<object>(this, "showConfig", ShowConfig);

            Messenger.Default.Register<string>(this, "saveObjects", saveObjects);

            Messenger.Default.Register<Tuple<SimpleLabel, string>>(this, "loadLabel", LoadLabel);
            Messenger.Default.Register<Tuple<SimpleSwitch, string>>(this, "loadSwitch", LoadSwitch);
            Messenger.Default.Register<Tuple<SimpleCamera, string>>(this, "loadCamera", LoadCamera);

            Messenger.Default.Register<string>(this, "getSwitchAndChangeValue", ChangeIconSwitch);

            Messenger.Default.Register<string>(this, "sendSwitchState", SendSwitchState);

            Messenger.Default.Register<string>(this, "subscribeToTopics", SubscribeToTheStrings);

            Messenger.Default.Register<string>(this, "cameraMouseOver", CameraMouseOver);

            Messenger.Default.Register<string>(this, "getSubscribedDevices", ReloadDevicesData);

            CurrentViewModel = _locator.Settings;
            CurrentViewModel = _locator.Dashboard;

            if(Settings.Default.mqttBroker != "")
            {
                Messenger.Default.Send(Settings.Default.mqttBroker, "settingsBroker");
                ConnectToMQTT(Settings.Default.mqttBroker);
            }

            if (File.Exists(@"labels.bin") && File.Exists(@"switches.bin") && File.Exists(@"cameras.bin"))
            {
                loadObjects();
            }
        }

        void CameraMouseOver(string name)
        {
            SubscribedCamera foundedCamera = subscribedCameras.Find(x => x.UniqueName == (name.Remove(name.Length - 6)));

            Messenger.Default.Send(foundedCamera, "cameraMouseOver");
        }

        private void SendSwitchState(string name)
        {
            SubscribedSwitch subscribedSwitch = subscribedSwitches.Find(x => x.UniqueName == name);

            try
            {
                client.Publish(subscribedSwitch.Topic, Encoding.UTF8.GetBytes((!subscribedSwitch.State).ToString()), MqttMsgBase.QOS_LEVEL_EXACTLY_ONCE, false);
            }
            catch
            {
                Debug.WriteLine("Error in publishing");
            }
        }

        private void ChangeIconSwitch(string name)
        {
            SubscribedSwitch subscribedSwitch = subscribedSwitches.Find(x => x.UniqueName == name);

            if (subscribedSwitch.State)
            {
                Messenger.Default.Send(new UpdateSwitchMessage { subscribedSwitch = subscribedSwitch, value = subscribedSwitch.OnIconUri }, "ChangeSwitchValue");
            }
            else
            {
                Messenger.Default.Send(new UpdateSwitchMessage { subscribedSwitch = subscribedSwitch, value = subscribedSwitch.OffIconUri }, "ChangeSwitchValue");
            }
        }

        private void LoadSwitch(Tuple<SimpleSwitch, string> switchData)
        {
            SimpleSwitch subSwitch = switchData.Item1;
            string uniqueName = switchData.Item2;

            SubscribedSwitch subscribedSwitch = subscribedSwitches.Find(x => x.UniqueName == uniqueName);

            subscribedSwitch.SimpleSwitch = subSwitch;

            Messenger.Default.Send(subscribedSwitch, "addBindingSwitch");

            Grid grid = subscribedSwitch.SimpleSwitch.Content as Grid;
            CheckBox box = grid.Children[0] as CheckBox;
            Image icon = grid.Children[1] as Image;


            if(subscribedSwitch.Clickable)
            {
                Binding commandBinding = new Binding("SwitchClickedCommand");
                BindingOperations.SetBinding(icon.InputBindings[0], InputBinding.CommandProperty, commandBinding);
            }

            icon.InputBindings[0].CommandParameter = subscribedSwitch.UniqueName;

            Messenger.Default.Send(uniqueName, "getSwitchAndChangeValue");

            Binding myBinding = new Binding("Switches[" + subscribedSwitch.Id + "]");

            BindingOperations.SetBinding(icon, Image.SourceProperty, myBinding);

            if (subscribedSwitch.State)
            {
                Messenger.Default.Send(new UpdateSwitchMessage { subscribedSwitch = subscribedSwitch, value = subscribedSwitch.OnIconUri }, "ChangeSwitchValue");
            }
            else
            {
                Messenger.Default.Send(new UpdateSwitchMessage { subscribedSwitch = subscribedSwitch, value = subscribedSwitch.OffIconUri }, "ChangeSwitchValue");
            }

        }

        private void LoadLabel(Tuple<SimpleLabel, string> labelData)
        {
            SimpleLabel label = labelData.Item1;
            string uniqueName = labelData.Item2;

            SubscribedLabel subscribedLabel = subscribedLabels.Find(x => x.UniqueName == uniqueName);

            subscribedLabel.Label = label;

            Binding myBinding = new Binding("Labels[" + subscribedLabel.Id + "]");
            subscribedLabel.Label.Label.SetBinding(ContentControl.ContentProperty, myBinding);
            Messenger.Default.Send(new UpdateLabelMessage { subscribedLabel = subscribedLabel, value = "null" }, "ChangeLabelValue");
        }

        private void LoadCamera(Tuple<SimpleCamera, string> cameraData)
        {
            SimpleCamera camera = cameraData.Item1;
            string uniqueName = cameraData.Item2;

            SubscribedCamera subscribedCamera = subscribedCameras.Find(x => x.UniqueName == uniqueName);

            subscribedCamera.Camera = camera;

            Grid grid = subscribedCamera.Camera.Content as Grid;
            Image cameraImage = grid.Children[0] as Image;

            cameraImage.Name = subscribedCamera.UniqueName + "_image";
        }


        void SubscribeToTheStrings(string value)
        {
            foreach(SubscribedLabel label in subscribedLabels)
            {
                if(label.Topic != "")
                {
                    subscribedStrings.Add(label.Topic);
                }
            }

            foreach (SubscribedSwitch subSwitch in subscribedSwitches)
            {
                if (subSwitch.Topic != "")
                {
                    subscribedStrings.Add(subSwitch.Topic);

                }
            }
            foreach (SubscribedCamera subCamera in subscribedCameras)
            {
                if (subCamera.Topic != "")
                {
                    subscribedStrings.Add(subCamera.Topic);

                }
            }
            if (subscribedStrings.Count != 0 && client.IsConnected)
            {
                byte[] qosArray = new byte[subscribedStrings.Count];
                Array.Fill(qosArray, MqttMsgBase.QOS_LEVEL_EXACTLY_ONCE);

                client.Subscribe(subscribedStrings.ToArray(), qosArray);
            }

        }

        private void ShowConfig(object sender)
        {
            switch (sender.GetType().Name)
            {
                case "SimpleLabel":
                    Messenger.Default.Send(subscribedLabels.Find(x => x.Label == (SimpleLabel)sender), "topic");
                    break;
                case "SimpleSwitch":
                    Messenger.Default.Send(subscribedSwitches.Find(x => x.SimpleSwitch == (SimpleSwitch)sender), "topic");
                    break;
                case "SimpleCamera":
                    Messenger.Default.Send(subscribedCameras.Find(x => x.Camera == (SimpleCamera)sender), "topic");
                    break;
                default:
                    break;
            }
        }

        private void SettingsView()
        {
            CurrentViewModel = _locator.Settings;
        }

        private void Dashboard()
        {
            CurrentViewModel = _locator.Dashboard;
        }

        private void Database()
        {
            CurrentViewModel = _locator.DatabaseData;
        }

        private void DatabaseChart()
        {
            CurrentViewModel = _locator.DatabaseChartData;
        }


        private void ConnectToMQTT(SettingsModel settings)
        {
            Debug.WriteLine("Connecting " + settings.BrokerIpAddress);
            if(client == null)
            {
                client = new MqttClient(IPAddress.Parse(settings.BrokerIpAddress));
            }
            else
            {
                if(client.IsConnected)
                {
                    client.Disconnect();
                }
                client = new MqttClient(IPAddress.Parse(settings.BrokerIpAddress));
            }
            connectToServer();
        }

        private void ConnectToMQTT(string mqttBroker)
        {
            Debug.WriteLine("Connecting " + mqttBroker);
            if (client == null)
            {
                client = new MqttClient(IPAddress.Parse(mqttBroker));
            }
            else
            {
                if (client.IsConnected)
                {
                    client.Disconnect();
                }
                client = new MqttClient(IPAddress.Parse(mqttBroker));
            }
            connectToServer();

        }

        void connectToServer()
        {
            try
            {
                // register to message received 
                client.MqttMsgPublishReceived += Client_MqttMsgPublishReceived;

                string clientId = Guid.NewGuid().ToString();
                client.Connect(clientId);

                // subscribe to the topic "/home/temperature" with QoS 2 
                if (subscribedStrings.Count != 0)
                {
                    client.Subscribe(subscribedStrings.ToArray(), new byte[] { MqttMsgBase.QOS_LEVEL_EXACTLY_ONCE });
                }
            }
            catch
            {
                Debug.WriteLine("Server not responding");
                Messenger.Default.Send(new SimpleMessage { Type = SimpleMessage.MessageType.MQTTTimeout });
                return;
            }
            Messenger.Default.Send(new SimpleMessage { Type = SimpleMessage.MessageType.ConnectedToMQTT });
        }

        private void saveObjects(string value)
        {
            string serializationFile = @"labels.bin";

            using (Stream stream = File.Open(serializationFile, FileMode.Create))
            {
                var bformatter = new System.Runtime.Serialization.Formatters.Binary.BinaryFormatter();

                bformatter.Serialize(stream, subscribedLabels);
            }

            serializationFile = @"switches.bin";

            using (Stream stream = File.Open(serializationFile, FileMode.Create))
            {
                var bformatter = new System.Runtime.Serialization.Formatters.Binary.BinaryFormatter();

                bformatter.Serialize(stream, subscribedSwitches);
            }

            serializationFile = @"cameras.bin";

            using (Stream stream = File.Open(serializationFile, FileMode.Create))
            {
                var bformatter = new System.Runtime.Serialization.Formatters.Binary.BinaryFormatter();

                bformatter.Serialize(stream, subscribedCameras);
            }
        }

        private void loadObjects()
        {
            string serializationFile = @"labels.bin";

            //deserialize
            using (Stream stream = File.Open(serializationFile, FileMode.Open))
            {
                var bformatter = new System.Runtime.Serialization.Formatters.Binary.BinaryFormatter();

                subscribedLabels.Clear();
                subscribedLabels = (List<SubscribedLabel>)bformatter.Deserialize(stream);
            }

            serializationFile = @"switches.bin";

            //deserialize
            using (Stream stream = File.Open(serializationFile, FileMode.Open))
            {
                var bformatter = new System.Runtime.Serialization.Formatters.Binary.BinaryFormatter();

                subscribedSwitches.Clear();
                subscribedSwitches = (List<SubscribedSwitch>)bformatter.Deserialize(stream);
            }

            serializationFile = @"cameras.bin";

            //deserialize
            using (Stream stream = File.Open(serializationFile, FileMode.Open))
            {
                var bformatter = new System.Runtime.Serialization.Formatters.Binary.BinaryFormatter();

                subscribedCameras.Clear();
                subscribedCameras = (List<SubscribedCamera>)bformatter.Deserialize(stream);
            }
        }

        private void ReloadDevicesData(string sender)
        {
            ReloadFromDatabaseMessage message = new ReloadFromDatabaseMessage
            {
                subscribedCameras = subscribedCameras,
                subscribedLabels = subscribedLabels,
                subscribedSwitches = subscribedSwitches
            };

            Messenger.Default.Send(message, "getLastDatabaseValue");
        }

        void CreateOrUpdateSubscribedDevice(object updatedObject)
        {
            string topicString = "";
            Debug.WriteLine(updatedObject.GetType().Name);
            switch (updatedObject.GetType().Name)
            {   
                case "SubscribedLabel":
                    SubscribedLabel updatedLabel = updatedObject as SubscribedLabel;

                    SubscribedLabel foundedLabel = subscribedLabels.Find(x => x.Label == updatedLabel.Label);
                    topicString = updatedLabel.Topic;

                    break;
                case "SubscribedSwitch":
                    SubscribedSwitch updatedSwitch = updatedObject as SubscribedSwitch;

                    SubscribedSwitch foundedSwitch = subscribedSwitches.Find(x => x.SimpleSwitch == updatedSwitch.SimpleSwitch);
                    topicString = foundedSwitch.Topic;

                    if(!foundedSwitch.Clickable)
                    {
                        SimpleSwitch simpleSwitch = foundedSwitch.SimpleSwitch;
                        Grid grid = simpleSwitch.Content as Grid;

                        Image icon = grid.Children[1] as Image;

                        icon.InputBindings[0].Command = null;
                    }
                    else
                    {
                        SimpleSwitch simpleSwitch = foundedSwitch.SimpleSwitch;

                        Grid grid = simpleSwitch.Content as Grid;

                        Image icon = grid.Children[1] as Image;

                        Binding commandBinding = new Binding("SwitchClickedCommand");
                        BindingOperations.SetBinding(icon.InputBindings[0], InputBinding.CommandProperty, commandBinding);
                    }

                    break;
                case "SubscribedCamera":
                    SubscribedCamera updatedCamera = updatedObject as SubscribedCamera;

                    SubscribedCamera foundedCamera = subscribedCameras.Find(x => x.Camera == updatedCamera.Camera);
                    topicString = foundedCamera.Topic; 
                    break;
                default:
                    break;
            }

            if(topicString != "" && !subscribedStrings.Contains(topicString))
            {

                subscribedStrings.Add(topicString);
            }

            if (subscribedStrings.Count != 0)
            {
                byte[] qosArray = new byte[subscribedStrings.Count];
                Array.Fill(qosArray, MqttMsgBase.QOS_LEVEL_EXACTLY_ONCE);

                client.Subscribe(subscribedStrings.ToArray(), qosArray);
            }
        }

        private void AddLabel(SubscribedLabel label)
        {
            subscribedLabels.Add(label);
        }

        private void AddSwitch(SubscribedSwitch subSwitch)
        {
            subscribedSwitches.Add(subSwitch);
        }

        private void AddCamera(SubscribedCamera subCamera)
        {
            subscribedCameras.Add(subCamera);
        }


        void ChangeImage(byte[] img, SubscribedCamera camera)
        {
            var image = new BitmapImage();
            using (var mem = new MemoryStream(img))
            {
                mem.Position = 0;
                image.BeginInit();
                image.CreateOptions = BitmapCreateOptions.PreservePixelFormat;
                image.CacheOption = BitmapCacheOption.OnLoad;
                image.UriSource = null;
                image.StreamSource = mem;
                image.EndInit();
            }

            camera.lastImage = image;
        }

        void Client_MqttMsgPublishReceived(object sender, MqttMsgPublishEventArgs e)
        {
            string ReceivedMessage = Encoding.UTF8.GetString(e.Message);

            SubscribedLabel foundedLabel = subscribedLabels.Find(x => x.Topic == e.Topic);
            SubscribedSwitch foundedSwitch = subscribedSwitches.Find(x => x.Topic == e.Topic);
            SubscribedCamera foundedCamera = subscribedCameras.Find(x => x.Topic == e.Topic);

            if (foundedLabel != null)
            {
                Messenger.Default.Send(new UpdateLabelMessage { subscribedLabel = foundedLabel, value = ReceivedMessage }, "ChangeLabelValue");
            }
            else if (foundedSwitch != null)
            {
                foundedSwitch.State = Convert.ToBoolean(ReceivedMessage);
                Application.Current.Dispatcher.Invoke(new Action(() => { ChangeIconSwitch(foundedSwitch.UniqueName); }));
            }
            else if(foundedCamera != null)
            {
                byte[] imageData = e.Message;

                Application.Current.Dispatcher.Invoke(new Action(() => { ChangeImage(imageData, foundedCamera); }));

                Messenger.Default.Send(foundedCamera, "updateCameraImage");
            }
        }
    }
}
