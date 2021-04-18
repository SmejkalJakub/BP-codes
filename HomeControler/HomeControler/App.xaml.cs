using HomeControler.Models;
using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;

namespace HomeControler
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        private static HomeControllerModel _homeControllerModel = new HomeControllerModel();
        public static HomeControllerModel HomeControllerModelProperty
        {
            get
            {
                return _homeControllerModel;
            }
        }

        private static SettingsModel _settingsModel = new SettingsModel();
        public static SettingsModel SettingsModelProperty
        {
            get
            {
                return _settingsModel;
            }
            set
            {
                _settingsModel = value;
            }
        }
    }
}
