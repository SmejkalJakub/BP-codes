
namespace HomeControler.ViewModels
{
    public class ViewModelLocator
    {
        private static MainViewModel _main;
        private static DashboardViewModel _dashboard;
        private static SettingsViewModel _settings;
        private static DatabaseDataViewModel _databaseData;
        private static DatabaseDataChartViewModel _databaseChartData;


        public ViewModelLocator()
        {


        }

        public MainViewModel Main
        {
            get
            {
                if (_main == null)
                {
                    _main = new MainViewModel();
                }
                return _main;
            }
        }
        public DashboardViewModel Dashboard
        {
            get
            {
                if (_dashboard == null)
                {
                    _dashboard = new DashboardViewModel();
                }
                return _dashboard;
            }
        }
        public SettingsViewModel Settings
        {
            get
            {
                if (_settings == null)
                {
                    _settings = new SettingsViewModel();
                }
                return _settings;
            }
        }
        public DatabaseDataViewModel DatabaseData
        {
            get
            {
                if (_databaseData == null)
                {
                    _databaseData = new DatabaseDataViewModel();
                }
                return _databaseData;
            }
        }
        public DatabaseDataChartViewModel DatabaseChartData
        {
            get
            {
                if (_databaseChartData == null)
                {
                    _databaseChartData = new DatabaseDataChartViewModel();
                }
                return _databaseChartData;
            }
        }
    }
}
