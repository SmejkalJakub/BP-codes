/*
Zařízení pro detekci zatopení

Autor: Jakub Smejkal
Datum: 2.4.2021
*/
#include <application.h>

#define FLOOD_DETECTOR_NO_CHANGE_INTEVAL (15 * MINUTE)
#define FLOOD_DETECTOR_UPDATE_NORMAL_INTERVAL  (5 * 1000)

#define TEMPERATURE_TAG_PUB_NO_CHANGE_INTEVAL (15 * MINUTE)
#define VALUE_CHANGE_PUB_TRESHOLD 0.2f

#define BATTERY_UPDATE_INTERVAL (60 * MINUTE)

twr_tmp112_t tmp112;

twr_flood_detector_t flood_detector;

bool alarmState;
twr_tick_t nextFloodPublish;

float temperature;
twr_tick_t nextTemperaturePublish;

/*
Event handler for the Battery module

This function will just send the battery percentage and charge over MQTT so it can be monitored
*/
void battery_event_handler(twr_module_battery_event_t event, void *event_param)
{
    float voltage;
    int percentage;

    if (twr_module_battery_get_voltage(&voltage))
    {
        twr_radio_pub_battery(&voltage);
    }

    if (twr_module_battery_get_charge_level(&percentage))
    {
        twr_radio_pub_int("voltage/percentage", &percentage);
    }
}

/*
Event handler for the Flood sensor

This funciton will check alarm state and send the change over MQTT
*/
void flood_detector_event_handler(twr_flood_detector_t *self, twr_flood_detector_event_t event, void *event_param)
{
    bool is_alarm;

    if (event == TWR_FLOOD_DETECTOR_EVENT_UPDATE)
    {
       is_alarm = twr_flood_detector_is_alarm(self);

       if ((is_alarm != alarmState) || (nextFloodPublish < twr_scheduler_get_spin_tick()))
       {
           twr_radio_pub_bool("flood-detector/a/alarm", &is_alarm);

           alarmState = is_alarm;
           nextFloodPublish = twr_scheduler_get_spin_tick() + FLOOD_DETECTOR_NO_CHANGE_INTEVAL;
       }
    }
}

/*
Event handler for the Temperature sensor

It will detect the update and send the value over MQTT in case there is a big change or the selected time
elapsed from the last message
*/
void tmp112_event_handler(twr_tmp112_t *self, twr_tmp112_event_t event, void *event_param)
{
    if (event == TWR_TMP112_EVENT_UPDATE)
    {
        float celsius;
        twr_tmp112_get_temperature_celsius(self, &celsius);

        if ((fabs(celsius - temperature) >= VALUE_CHANGE_PUB_TRESHOLD) || (nextTemperaturePublish < twr_scheduler_get_spin_tick()))
        {
            twr_log_debug("APP: temperature: %.2f °C", celsius);
            twr_radio_pub_temperature(TWR_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_DEFAULT, &celsius);
            temperature = celsius;
            nextTemperaturePublish = twr_scheduler_get_spin_tick() + TEMPERATURE_TAG_PUB_NO_CHANGE_INTEVAL;
        }
    }
}

/*
Init function that runs once at the beginning of the program
*/
void application_init(void)
{
    twr_log_init(TWR_LOG_LEVEL_DUMP, TWR_LOG_TIMESTAMP_ABS);

    // Temperature sensor inicialization
    twr_tmp112_init(&tmp112, TWR_I2C_I2C0, 0x49);
    twr_tmp112_set_event_handler(&tmp112, tmp112_event_handler, NULL);
    twr_tmp112_set_update_interval(&tmp112, 5000);

    twr_flood_detector_init(&flood_detector, TWR_FLOOD_DETECTOR_TYPE_LD_81_SENSOR_MODULE_CHANNEL_A);
    twr_flood_detector_set_event_handler(&flood_detector, flood_detector_event_handler, NULL);
    twr_flood_detector_set_update_interval(&flood_detector, 5000);

    // Batery module inicialization
    twr_module_battery_init();
    twr_module_battery_set_event_handler(battery_event_handler, NULL);
    twr_module_battery_set_update_interval(BATTERY_UPDATE_INTERVAL);

    // Radio init and pairing
    twr_radio_init(TWR_RADIO_MODE_NODE_SLEEPING);
    twr_radio_pairing_request("flood-detector", VERSION);
}
