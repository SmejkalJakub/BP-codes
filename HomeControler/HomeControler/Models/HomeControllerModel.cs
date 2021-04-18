using HomeControler.ViewModels;
using MySql.Data.MySqlClient;
using System;
using System.Collections.ObjectModel;
using System.Data;

namespace HomeControler.Models
{
    public class HomeControllerModel
    {
        //MySqlConnection conn = new MySqlConnection(ConfigurationManager.ConnectionStrings["ConnectionString"].ConnectionString);

        public ObservableCollection<DatabaseData> GetDatabaseData(string selectedFilterType, string filter)
        {
            MySqlConnection conn = new MySqlConnection("Server=" + Settings.Default.databaseAddress + ";userid=" + Settings.Default.databaseUserName + ";password=" + Settings.Default.databaseTablePasswd + ";Database=" + Settings.Default.databaseName);
            ObservableCollection<DatabaseData> localData = new ObservableCollection<DatabaseData>();

            try
            {
                conn.Open();

                MySqlCommand cmd;
                if (selectedFilterType == "All")
                {
                    cmd = new MySqlCommand("SELECT messageId, topic, value, room, message_recieved FROM " + Settings.Default.databaseTableName + " WHERE topic LIKE '%" + filter + "%' OR value LIKE '%" + filter + "%' OR room LIKE '%" + filter + "%' OR message_recieved LIKE '%" + filter + "%'", conn);

                }
                else
                {
                    cmd = new MySqlCommand("SELECT messageId, topic, value, room, message_recieved FROM " + Settings.Default.databaseTableName + " WHERE " + selectedFilterType.ToLower() + " LIKE '%" + filter + "%'", conn);
                }
                MySqlDataAdapter adp = new MySqlDataAdapter(cmd);
                DataTable dt = new DataTable();
                adp.Fill(dt);

                for (int i = 0; i < dt.Rows.Count; i++)
                {

                    localData.Add(new DatabaseData
                    {
                        MessageId = Convert.ToInt32(dt.Rows[i][0]),
                        Topic = dt.Rows[i][1].ToString(),
                        Value = dt.Rows[i][2].ToString(),
                        Room = dt.Rows[i][3].ToString(),
                        Message_recieved = Convert.ToDateTime(dt.Rows[i][4])
                    });
                }

            }
            catch (MySqlException ex)
            {
                return null;
            }
            finally
            {
                conn.Close();
            }

            return localData;
        }
    }
}
