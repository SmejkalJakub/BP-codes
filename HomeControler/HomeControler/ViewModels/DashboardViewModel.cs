using GalaSoft.MvvmLight;
using GalaSoft.MvvmLight.Command;
using GalaSoft.MvvmLight.Messaging;
using HomeControler.Objects;
using HomeControler.Others;
using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Windows.Input;
using System.Windows.Media.Imaging;
using System.Linq;
using System.Windows;

namespace HomeControler.ViewModels
{
    public class DashboardViewModel : ViewModelBase
    {
        private Thickness _ImagePosition;
        public Thickness ImagePosition
        {
            get
            {
                return _ImagePosition;
            }
            set
            {
                _ImagePosition = value;
                RaisePropertyChanged();
            }
        }

        private Visibility _TextBoxVisibility;
        public Visibility TextBoxVisibility
        {
            get
            {
                return _TextBoxVisibility;
            }
            set
            {
                _TextBoxVisibility = value;
                RaisePropertyChanged("TextBoxVisibility");
            }
        }

        private Thickness _TextBoxPosition;
        public Thickness TextBoxPosition
        {
            get
            {
                return _TextBoxPosition;
            }
            set
            {
                _TextBoxPosition = value;
                RaisePropertyChanged();
            }
        }

        private Dictionary<int, string> _ButtonNames;
        public Dictionary<int, string> ButtonNames
        {
            get
            {
                return _ButtonNames;
            }
            set
            {
                _ButtonNames = value;
                RaisePropertyChanged();
            }
        }

        private Dictionary<int, Thickness> _ButtonPositions;
        public Dictionary<int, Thickness> ButtonPositions
        {
            get
            {
                return _ButtonPositions;
            }
            set
            {
                _ButtonPositions = value;
                RaisePropertyChanged();
            }
        }


        private Dictionary<Guid, string> _Labels;

        public Dictionary<Guid, string> Labels
        {
            get 
            { 
                return _Labels; 
            }
            set 
            {
                _Labels = value;
                RaisePropertyChanged();
            }
        }

        private Dictionary<Guid, BitmapImage> _Switches;

        public Dictionary<Guid, BitmapImage> Switches
        {
            get
            {
                return _Switches;
            }
            set
            {
                _Switches = value;
                RaisePropertyChanged();
            }
        }

        private string _BackgroundSource;

        public string BackgroundSource
        {
            get
            {
                return _BackgroundSource;
            }
            set
            {
                _BackgroundSource = value;
                RaisePropertyChanged();
            }
        }

        private BitmapImage _OnIcon;

        public BitmapImage OnIcon
        {
            get
            {
                return _OnIcon;
            }
            set
            {
                _OnIcon = value;
                RaisePropertyChanged();
            }
        }

        private BitmapImage _OffIcon;

        public BitmapImage OffIcon
        {
            get
            {
                return _OffIcon;
            }
            set
            {
                _OffIcon = value;
                RaisePropertyChanged();
            }
        }

        private BitmapImage _CameraLastImage;

        public BitmapImage CameraLastImage
        {
            get
            {
                return _CameraLastImage;
            }
            set
            {
                _CameraLastImage = value;
                RaisePropertyChanged();
            }
        }

        private int lastButtonIndex = 0;

        private SubscribedCamera currentCamera;

        public ICommand SelectOnIconCommand => new RelayCommand(selectOnIcon);
        public ICommand SelectOffIconCommand => new RelayCommand(selectOffIcon);
        public ICommand SwitchClickedCommand => new RelayCommand<string>(switchClicked);
        public ICommand ChangeLayoutCommand => new RelayCommand<string>(changeLayout);
        public ICommand ChangeLayoutImageButton => new RelayCommand(changeLayoutImage);
        public ICommand AddNewLayoutCommand => new RelayCommand(addNewLayout);
        public ICommand ShowToRenameCommand => new RelayCommand<string>(showToRename);



        private ObservableCollection<DatabaseData> databaseData;

        public DashboardViewModel()
        {
            Messenger.Default.Register<UpdateLabelMessage>(this, "ChangeLabelValue", ChangeLabelValue);
            Messenger.Default.Register<UpdateSwitchMessage>(this, "ChangeSwitchValue", ChangeSwithcValue);
            Messenger.Default.Register<SubscribedCamera>(this, "updateCameraImage", ChangeCameraImage);

            Messenger.Default.Register<SubscribedSwitch>(this, "switchInfo", SetIconValues);

            Messenger.Default.Register<SubscribedSwitch>(this, "updateIcons", UpdateSwitchIcons);

            Messenger.Default.Register<SubscribedCamera>(this, "cameraMouseOver", CameraMouseOver);
            Messenger.Default.Register<string>(this, "cameraMouseOverLeave", CameraMouseOverLeave);

            Messenger.Default.Register<ReloadFromDatabaseMessage>(this, "getLastDatabaseValue", LoadLastDatabaseData);
            Messenger.Default.Register<string>(this, "loadLayout", loadLayout);


            databaseData = App.HomeControllerModelProperty.GetDatabaseData("All", "");

            Labels = new Dictionary<Guid, string>();
            Switches = new Dictionary<Guid, BitmapImage>();

            Messenger.Default.Register<string>(this, "backgroundSelect", setBackground);

            if (Settings.Default.backgroundImage != "")
            {
                BackgroundSource = Settings.Default.backgroundImage;
            }



            _ButtonNames = new Dictionary<int, string>();
            _ButtonNames[0] = "Default";

            _ButtonPositions = new Dictionary<int, Thickness>();
            _ButtonPositions[0] = new Thickness(10, 10, 0, 0);

            _TextBoxVisibility = Visibility.Hidden;
            _TextBoxPosition = new Thickness(80, 40, 0, 0);


        }

        void LoadLastDatabaseData(ReloadFromDatabaseMessage elements)
        {
            if(databaseData != null)
            {
                foreach(SubscribedLabel subscribedLabel in elements.subscribedLabels)
                {
                    DatabaseData databaseEntry = databaseData.Where(x => x.Topic == subscribedLabel.Topic).OrderByDescending(d => d.Message_recieved).FirstOrDefault();
                    if(databaseEntry != null)
                    {
                        ChangeLabelValue(new UpdateLabelMessage { subscribedLabel = subscribedLabel, value = databaseEntry.Value });
                    }
                }
            }
        }

        private void showToRename(string indexString)
        {
            int index = Convert.ToInt32(indexString);
            _TextBoxVisibility = Visibility.Hidden;
            _TextBoxPosition = new Thickness(ButtonPositions[index].Left, ButtonPositions[index].Top + 30, ButtonPositions[index].Right, ButtonPositions[index].Bottom);
        }

        private void loadLayout(string name)
        {
            lastButtonIndex++;
            _ButtonNames[lastButtonIndex] = lastButtonIndex.ToString();
            _ButtonPositions[lastButtonIndex] = new Thickness(_ButtonPositions[lastButtonIndex - 1].Left + 85, _ButtonPositions[lastButtonIndex - 1].Top, _ButtonPositions[lastButtonIndex - 1].Right, _ButtonPositions[lastButtonIndex - 1].Bottom);
            Messenger.Default.Send(lastButtonIndex, "addNewLayoutButton");
        }

        private void addNewLayout()
        {
            lastButtonIndex++;
            _ButtonNames[lastButtonIndex] = lastButtonIndex.ToString();
            _ButtonPositions[lastButtonIndex] = new Thickness(_ButtonPositions[lastButtonIndex - 1].Left + 85, _ButtonPositions[lastButtonIndex - 1].Top, _ButtonPositions[lastButtonIndex - 1].Right, _ButtonPositions[lastButtonIndex - 1].Bottom);
            Messenger.Default.Send(lastButtonIndex, "addNewLayoutButton");

        }

        void CameraMouseOver(SubscribedCamera cameraObject)
        {
            if(cameraObject.lastImage != null)
            {
                currentCamera = cameraObject;

                ImagePosition = new Thickness(cameraObject.Camera.Margin.Left + 300, cameraObject.Camera.Margin.Top + 300, cameraObject.Camera.Margin.Right, cameraObject.Camera.Margin.Bottom);

                CameraLastImage = cameraObject.lastImage;
            }
        }

        void changeLayout(string layoutName)
        {
            Messenger.Default.Send(layoutName, "reloadGrid");
        }

        void changeLayoutImage()
        {
            OpenFileDialog op = new OpenFileDialog();
            op.Title = "Select a picture";
            op.Filter = "All supported graphics|*.jpg;*.jpeg;*.png|" +
              "JPEG (*.jpg;*.jpeg)|*.jpg;*.jpeg|" +
              "Portable Network Graphic (*.png)|*.png";
            if (op.ShowDialog() == true)
            {
                Messenger.Default.Send(op.FileName, "backgroundSelect");
            }
        }

        void CameraMouseOverLeave(string sender)
        {
            currentCamera = null;
            CameraLastImage = null;
        }

        private void switchClicked(string name)
        {

            Messenger.Default.Send(name, "sendSwitchState");

        }

        private void setBackground(string imageUri)
        {
            BackgroundSource = imageUri;
            Settings.Default.backgroundImage = imageUri;
            Settings.Default.Save();
        }

        private void ChangeLabelValue(UpdateLabelMessage message)
        {
            Labels[message.subscribedLabel.Id] = message.subscribedLabel.Prefix + message.value + message.subscribedLabel.Postfix;
            RaisePropertyChanged("Labels");
        }

        private void ChangeSwithcValue(UpdateSwitchMessage message)
        {
            Switches[message.subscribedSwitch.Id] = new BitmapImage(new Uri(message.value));
            RaisePropertyChanged("Switches");
        }

        void ChangeCameraImage(SubscribedCamera cameraMessage)
        {
            if(currentCamera == cameraMessage)
            {
                CameraLastImage = currentCamera.lastImage;
            }
        }

        private void selectOnIcon()
        {
            OpenFileDialog op = new OpenFileDialog();
            op.Title = "Select a picture";
            op.Filter = "All supported graphics|*.jpg;*.jpeg;*.png|" +
              "JPEG (*.jpg;*.jpeg)|*.jpg;*.jpeg|" +
              "Portable Network Graphic (*.png)|*.png";
            if (op.ShowDialog() == true)
            {
                BitmapImage image = new BitmapImage(new Uri(op.FileName));
                OnIcon = image;
            }
        }

        private void selectOffIcon()
        {
            OpenFileDialog op = new OpenFileDialog();
            op.Title = "Select a picture";
            op.Filter = "All supported graphics|*.jpg;*.jpeg;*.png|" +
              "JPEG (*.jpg;*.jpeg)|*.jpg;*.jpeg|" +
              "Portable Network Graphic (*.png)|*.png";
            if (op.ShowDialog() == true)
            {
                BitmapImage image = new BitmapImage(new Uri(op.FileName));
                OffIcon = image;
            }
        }

        private void SetIconValues(SubscribedSwitch subSwitch)
        {
            if(subSwitch.OnIconUri != null && subSwitch.OffIconUri != null)
            {
                OnIcon = new BitmapImage(new Uri(subSwitch.OnIconUri));
                OffIcon = new BitmapImage(new Uri(subSwitch.OffIconUri));
            }
        }

        private void UpdateSwitchIcons(SubscribedSwitch subSwtich)
        {
            if(OnIcon != null && OffIcon != null)
            {
                subSwtich.OnIconUri = OnIcon.UriSource.AbsolutePath;
                subSwtich.OffIconUri = OffIcon.UriSource.AbsolutePath;

                Messenger.Default.Send(subSwtich.UniqueName, "getSwitchAndChangeValue");
            }
        }
    }
}
