/*
Zařízení zajišťující měření klimatických podmínek v místnosti
Další funkcí je detekce pohybu pomocí PIR čidla
Všechna data jsou odesílána pomocí rádia

Autor: Jakub Smejkal
Datum: 2.4.2021
*/

#include <application.h>

#define TEMPERATURE_TAG_PUB_NO_CHANGE_INTEVAL (15 * MINUTE)
#define VALUE_CHANGE_PUB_TRESHOLD 0.2f

#define NO_MOVEMENT_DETECTED_INTERVAL (MINUTE)

#define BATTERY_UPDATE_INTERVAL (60 * MINUTE)

#define CO2_PUB_NO_CHANGE_INTERVAL (15 * MINUTE)
#define CO2_CHANGE_PUB_TRESHOLD 50.0f

#define HUMIDITY_TAG_PUB_NO_CHANGE_INTEVAL (15 * MINUTE)
#define HUMIDITY_VALUE_CHANGE_PUB_TRESHOLD 1.0f

twr_module_pir_t pirModule;

twr_scheduler_task_id_t noMovementTaskId;

twr_tag_temperature_t temperatureTag;
float temperature;
twr_tick_t nextTemperaturePublish;

float co2Concentration;
twr_tick_t nextCO2Publish;

twr_tag_humidity_t humidityTag;
float humidity;
twr_tick_t nextHumidityPublish;

bool movement = false;

void no_movement_detected();

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
Event handler for the PIR module

If the motion is detected and it was not detected before it will be sended over MQTT.

Not every movement is send to save battery life
*/
void pir_event_handler(twr_module_pir_t *self, twr_module_pir_event_t event, void *event_param)
{
    if(event == TWR_MODULE_PIR_EVENT_MOTION)
    {
        twr_log_debug("MOVEMENT");
        twr_scheduler_unregister(noMovementTaskId);
        noMovementTaskId = twr_scheduler_register(no_movement_detected, NULL, twr_scheduler_get_spin_tick() + NO_MOVEMENT_DETECTED_INTERVAL);

        if(!movement)
        {
            movement = true;
            twr_radio_pub_bool("movement", &movement);
        }
    }
}

/*
Event handler for the CO2 module

It will detect the update and send the value over MQTT in case there is a big change or the selected time
elapsed from the last message
*/
void co2_event_handler(twr_module_co2_event_t event, void *event_param)
{
    if(event == TWR_MODULE_CO2_EVENT_UPDATE)
    {
        float ppm;
        twr_module_co2_get_concentration_ppm(&ppm);

        if ((fabs(ppm - co2Concentration) >= CO2_CHANGE_PUB_TRESHOLD) || (nextCO2Publish < twr_scheduler_get_spin_tick()))
        {
            twr_radio_pub_co2(&ppm);
            co2Concentration = ppm;
            nextCO2Publish = twr_scheduler_get_spin_tick() + CO2_PUB_NO_CHANGE_INTERVAL;
        }
    }
}

/*
Event handler for the Humidity sensor

It will detect the update and send the value over MQTT in case there is a big change or the selected time
elapsed from the last message
*/
void humidity_tag_event_handler(twr_tag_humidity_t *self, twr_tag_humidity_event_t event, void *event_param)
{
    twr_log_debug("PRED MERENIM");
    if(event == TWR_TAG_HUMIDITY_EVENT_UPDATE)
    {
        twr_log_debug("MERIM");
        float percentage;
        twr_tag_humidity_get_humidity_percentage(self, &percentage);

        if ((fabs(percentage - humidity) >= HUMIDITY_VALUE_CHANGE_PUB_TRESHOLD) || (nextHumidityPublish < twr_scheduler_get_spin_tick()))
        {
            twr_radio_pub_humidity(TWR_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_DEFAULT, &percentage);
            humidity = percentage;
            nextHumidityPublish = twr_scheduler_get_spin_tick() + HUMIDITY_TAG_PUB_NO_CHANGE_INTEVAL;
        }
    }
}

/*
Event handler for the Temperature sensor

It will detect the update and send the value over MQTT in case there is a big change or the selected time
elapsed from the last message
*/
void temperature_tag_event_handler(twr_tag_temperature_t *self, twr_tag_temperature_event_t event, void *event_param)
{
    if (event == TWR_TAG_TEMPERATURE_EVENT_UPDATE)
    {
        float celsius;
        twr_tag_temperature_get_temperature_celsius(self, &celsius);

        // Read temperature
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
When no movement is detected, it will be sended over the MQTT
*/
void no_movement_detected()
{
    movement = false;
    twr_radio_pub_bool("movement", &movement);
}

/*
Init function that runs once at the beginning of the program
*/
void application_init(void)
{
    twr_log_init(TWR_LOG_LEVEL_DUMP, TWR_LOG_TIMESTAMP_REL);

    // Temperature sensor inicialization
    twr_tag_temperature_init(&temperatureTag, TWR_I2C_I2C0, TWR_TAG_TEMPERATURE_I2C_ADDRESS_DEFAULT);
    twr_tag_temperature_set_event_handler(&temperatureTag, temperature_tag_event_handler, NULL);
    twr_tag_temperature_set_update_interval(&temperatureTag, 5000);

    // Humidity sensor inicialization
    twr_tag_humidity_init(&humidityTag, TWR_TAG_HUMIDITY_REVISION_R2, TWR_I2C_I2C0, TWR_TAG_HUMIDITY_I2C_ADDRESS_DEFAULT);
    twr_tag_humidity_set_event_handler(&humidityTag, humidity_tag_event_handler, NULL);
    twr_tag_humidity_set_update_interval(&humidityTag, 5000);

    // CO2 module inicialization
    twr_module_co2_init();
    twr_module_co2_set_event_handler(co2_event_handler, NULL);
    twr_module_co2_set_update_interval(5000);

    // PIR module inicialization
    twr_module_pir_init(&pirModule);
    twr_module_pir_set_sensitivity(&pirModule, TWR_MODULE_PIR_SENSITIVITY_MEDIUM);
    twr_module_pir_set_event_handler(&pirModule, pir_event_handler, NULL);

    // Batery module inicialization
    twr_module_battery_init();
    twr_module_battery_set_event_handler(battery_event_handler, NULL);
    twr_module_battery_set_update_interval(BATTERY_UPDATE_INTERVAL);

    // Register task for when no movements occurres
    noMovementTaskId = twr_scheduler_register(no_movement_detected, NULL, TWR_TICK_INFINITY);

    // Radio init and pairing
    twr_radio_init(TWR_RADIO_MODE_NODE_SLEEPING);
    twr_radio_pairing_request("motion-detector-with-co2", VERSION);
}
