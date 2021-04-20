/*
Zařízení, které detekuje stisk tlačítka

Autor: Jakub Smejkal
Datum: 2.4.2021
*/

#include <application.h>

twr_button_t button;

/*
Event handler for the Button

In this function it is possible to make desision based on the button state
*/
void button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    if(event == TWR_BUTTON_EVENT_PRESS)
    {
        twr_log_debug("stisknuto");
    }
}

/*
Init function that runs once at the beginning of the program
*/
void application_init(void)
{
    twr_log_init(TWR_LOG_LEVEL_DUMP, TWR_LOG_TIMESTAMP_ABS);

    // Sensor module inicialization
    twr_module_sensor_init();

    // Button inicialization on GPIO PIN 4 with Pull up resistor
    twr_button_init(&button, TWR_GPIO_P4, TWR_GPIO_PULL_UP, 1);
    twr_button_set_event_handler(&button, button_event_handler, NULL);

    // Radio init and pairing
    twr_radio_init(TWR_RADIO_MODE_NODE_SLEEPING);
    twr_radio_pairing_request("custom-button", VERSION);
}
