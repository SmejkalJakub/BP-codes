/*
* Jednoduchá třída pro zasílání zpráv zkrzte Messenger díky které je možno komunikovat mezi ViewModely
* Autor: Jakub Smejkal, xsmejk28
*/

namespace HomeControler.Others
{
    public class SimpleMessage
    {
        public SimpleMessage()
        {
        }

        /// <summary>
        /// Enum type který rozlišuje druhy zpráv
        /// </summary>
        public enum MessageType
        {
            SettingsUpdated,
            ConnectedToMQTT,
            MQTTTimeout
        }

        public MessageType Type { get; set; }

        public string Message { get; set; }
    }
}
