/*
Device for monitoring the environmental values in indoor or outdoor

Author: Jakub Smejkal
Date: 2.4.2021
*/

#include <application.h>

#define BATTERY_UPDATE_INTERVAL (60 * MINUTE)
#define CLIMATE_PUB_NO_CHANGE_INTEVAL (15 * MINUTE)
#define VALUE_TEMPERATURE_CHANGE_PUB_TRESHOLD 0.2f
#define VALUE_HUMIDITY_CHANGE_PUB_TRESHOLD 1.0f
#define VALUE_LUX_CHANGE_PUB_TRESHOLD 25.0f
#define BAROMETER_TAG_PUB_NO_CHANGE_INTEVAL (5 * MINUTE)
#define PRESSURE_VALUE_CHANGE_PUB_TRESHOLD 10.0f

float temperature;
twr_tick_t nextTemperaturePublish;

float humidity;
twr_tick_t nextHumidityPublish;

float luxValue;
twr_tick_t nextLuxPublish;

float pressure;
twr_tick_t nextPressurePublish;

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
This function measures the data from the sensors on climate module and send them over MQTT

This function will be called every 5000ms.
Data will be sent over MQTT if some significant change occurres or after fixed time to save battery life
*/
void climate_module_event_handler(twr_module_climate_event_t event, void *event_param)
{
    if (event == TWR_MODULE_CLIMATE_EVENT_UPDATE_THERMOMETER)
    {
        float celsius;
        twr_module_climate_get_temperature_celsius(&celsius);

        if ((fabs(celsius - temperature) >= VALUE_TEMPERATURE_CHANGE_PUB_TRESHOLD) || (nextTemperaturePublish < twr_scheduler_get_spin_tick()))
        {
            twr_radio_pub_temperature(TWR_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_DEFAULT, &celsius);
            temperature = celsius;
            nextTemperaturePublish = twr_scheduler_get_spin_tick() + CLIMATE_PUB_NO_CHANGE_INTEVAL;
        }
    }
    else if (event == TWR_MODULE_CLIMATE_EVENT_UPDATE_HYGROMETER)
    {
        float percentage;
        twr_module_climate_get_humidity_percentage(&percentage);

        if ((fabs(percentage - humidity) >= VALUE_HUMIDITY_CHANGE_PUB_TRESHOLD) || (nextHumidityPublish < twr_scheduler_get_spin_tick()))
        {
            twr_radio_pub_humidity(TWR_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_DEFAULT, &percentage);
            humidity = percentage;
            nextHumidityPublish = twr_scheduler_get_spin_tick() + CLIMATE_PUB_NO_CHANGE_INTEVAL;
        }
    }
    else if (event == TWR_MODULE_CLIMATE_EVENT_UPDATE_LUX_METER)
    {
        float lux;
        twr_module_climate_get_illuminance_lux(&lux);

        if ((fabs(lux - luxValue) >= VALUE_LUX_CHANGE_PUB_TRESHOLD) || (nextLuxPublish < twr_scheduler_get_spin_tick()))
        {
            twr_radio_pub_luminosity(TWR_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_DEFAULT, &lux);
            luxValue = lux;
            nextLuxPublish = twr_scheduler_get_spin_tick() + CLIMATE_PUB_NO_CHANGE_INTEVAL;
        }
    }
    else if (event == TWR_MODULE_CLIMATE_EVENT_UPDATE_BAROMETER)
    {
        float pascals;
        twr_module_climate_get_pressure_pascal(&pascals);

        if ((fabs(pascals - pressure) >= PRESSURE_VALUE_CHANGE_PUB_TRESHOLD) || (nextPressurePublish < twr_scheduler_get_spin_tick()))
        {
            twr_radio_pub_barometer(TWR_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_DEFAULT, &pascals, NULL);
            pressure = pascals;
            nextPressurePublish = twr_scheduler_get_spin_tick() + CLIMATE_PUB_NO_CHANGE_INTEVAL;
        }
    }
}

/*
Init function that runs once at the beginning of the program
*/
void application_init(void)
{
    twr_log_init(TWR_LOG_LEVEL_DUMP, TWR_LOG_TIMESTAMP_ABS);

    // Climate module inicialization
    twr_module_climate_init();
    twr_module_climate_set_event_handler(climate_module_event_handler, NULL);
    twr_module_climate_set_update_interval_thermometer(5000);
    twr_module_climate_set_update_interval_hygrometer(5000);
    twr_module_climate_set_update_interval_lux_meter(5000);
    twr_module_climate_set_update_interval_barometer(5000);
    twr_module_climate_measure_all_sensors();

    // Batery module inicialization
    twr_module_battery_init();
    twr_module_battery_set_event_handler(battery_event_handler, NULL);
    twr_module_battery_set_update_interval(BATTERY_UPDATE_INTERVAL);

    // Radio init and pairing
    twr_radio_init(TWR_RADIO_MODE_NODE_SLEEPING);
    twr_radio_pairing_request("climate-monitor", VERSION);
}
