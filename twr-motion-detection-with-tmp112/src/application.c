/*
Zařízení měřící teplotu a detekující pohyb
Údaje o teplotě a pohybu jsou odesílány rádiem

Autor: Jakub Smejkal
Datum: 2.4.2021
*/

#include <application.h>

#define TEMPERATURE_TAG_PUB_NO_CHANGE_INTEVAL (15 * MINUTE)
#define VALUE_CHANGE_PUB_TRESHOLD 0.2f

#define NO_MOVEMENT_DETECTED_INTERVAL (MINUTE)

#define BATTERY_UPDATE_INTERVAL (60 * MINUTE)

twr_tmp112_t tmp112;

twr_module_pir_t pirModule;

twr_scheduler_task_id_t noMovementTaskId;

float temperature;
twr_tick_t nextTemperaturePublish;

bool movement = false;

void no_movement_detected();

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

void tmp112_event_handler(twr_tmp112_t *self, twr_tmp112_event_t event, void *event_param)
{
    if (event == TWR_TMP112_EVENT_UPDATE)
    {
        float celsius;
        twr_tmp112_get_temperature_celsius(self, &celsius);

        if ((fabs(celsius - temperature) >= VALUE_CHANGE_PUB_TRESHOLD) || (nextTemperaturePublish < twr_scheduler_get_spin_tick()))
        {
            twr_log_debug("APP: temperature: %.2f °C", celsius);
            twr_radio_pub_temperature(TWR_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_ALTERNATE, &celsius);
            temperature = celsius;
            nextTemperaturePublish = twr_scheduler_get_spin_tick() + TEMPERATURE_TAG_PUB_NO_CHANGE_INTEVAL;
        }
    }
}

void no_movement_detected()
{
    movement = false;
    twr_radio_pub_bool("movement", &movement);
}

void application_init(void)
{
    twr_log_init(TWR_LOG_LEVEL_DUMP, TWR_LOG_TIMESTAMP_REL);

    twr_tmp112_init(&tmp112, TWR_I2C_I2C0, 0x49);
    twr_tmp112_set_event_handler(&tmp112, tmp112_event_handler, NULL);
    twr_tmp112_set_update_interval(&tmp112, 5000);

    twr_module_pir_init(&pirModule);
    twr_module_pir_set_sensitivity(&pirModule, TWR_MODULE_PIR_SENSITIVITY_MEDIUM);
    twr_module_pir_set_event_handler(&pirModule, pir_event_handler, NULL);

    twr_module_battery_init();
    twr_module_battery_set_event_handler(battery_event_handler, NULL);
    twr_module_battery_set_update_interval(BATTERY_UPDATE_INTERVAL);

    noMovementTaskId = twr_scheduler_register(no_movement_detected, NULL, TWR_TICK_INFINITY);

    twr_radio_init(TWR_RADIO_MODE_NODE_SLEEPING);
    twr_radio_pairing_request("motion-detector-with-tmp112", VERSION);
}
