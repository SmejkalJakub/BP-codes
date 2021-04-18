using GalaSoft.MvvmLight;
using GalaSoft.MvvmLight.Command;
using System;
using System.Collections.ObjectModel;
using System.Windows.Input;

namespace HomeControler.ViewModels
{

    public class DatabaseData
    {
        int messageId;
        string topic;
        string message_value;
        string room;
        DateTime message_recieved;

        public int MessageId
        {
            get { return messageId; }
            set { messageId = value; }
        }
        public string Topic
        {
            get { return topic; }
            set { topic = value; }
        }
        public string Value
        {
            get { return message_value; }
            set { message_value = value; }
        }
        public string Room
        {
            get { return room; }
            set { room = value; }
        }
        public DateTime Message_recieved
        {
            get { return message_recieved; }
            set { message_recieved = value; }
        }
    }
    public class DatabaseDataViewModel : ViewModelBase
    {
        private ObservableCollection<string> _filters;

        public ICommand LoadDataCommand => new RelayCommand(LoadData);

        private ObservableCollection<DatabaseData> _databaseData;
        public ObservableCollection<DatabaseData> DatabaseDataCollection
        {
            get 
            { 
                return _databaseData; 
            }
            set 
            { 
                _databaseData = value; 
            }
        }
        public ObservableCollection<string> Filters
        {
            get 
            { 
                return _filters; 
            }
            set 
            { 
                _filters = value; 
            }
        }
        private string _selectedFilter;

        public string SelectedFilter
        {
            get 
            { 
                return _selectedFilter; 
            }
            set 
            { 
                _selectedFilter = value; 
            }
        }


        private string _filterBox;
        public string FilterBox
        {
            get 
            { 
                return _filterBox; 
            }
            set 
            { 
                _filterBox = value; 
            }
        }

       
        public DatabaseDataViewModel()
        {
            DatabaseDataCollection = new ObservableCollection<DatabaseData>();
            Filters = new ObservableCollection<string>()
            {
                "All",
                "Topic",
                "Value",
                "Room",
                "message_recieved"
            };
            SelectedFilter = Filters[0];
        }

        private void LoadData()
        {
            DatabaseDataCollection.Clear();

            DatabaseDataCollection = App.HomeControllerModelProperty.GetDatabaseData(SelectedFilter, FilterBox);
            RaisePropertyChanged("DatabaseDataCollection");
        }
    }
}
