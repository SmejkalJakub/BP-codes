/*
Device for monitoring air quality with a possible integration of programable LED strip

Author: Jakub Smejkal
Date: 2.4.2021
*/

#include <application.h>

#define TEMPERATURE_TAG_PUB_NO_CHANGE_INTEVAL (5 * MINUTE)
#define TEMPERATURE_VALUE_CHANGE_PUB_TRESHOLD 0.2f

#define BAROMETER_TAG_PUB_NO_CHANGE_INTEVAL (5 * MINUTE)
#define PRESSURE_VALUE_CHANGE_PUB_TRESHOLD 10.0f

#define HUMIDITY_TAG_PUB_NO_CHANGE_INTEVAL (5 * MINUTE)
#define HUMIDITY_VALUE_CHANGE_PUB_TRESHOLD 1.0f

#define VOC_TAG_PUB_NO_CHANGE_INTEVAL (5 * MINUTE)
#define VOC_VALUE_CHANGE_PUB_TRESHOLD 5.0f

#define LED_STRIP_COUNT 144
#define LED_STRIP_TYPE 4

#define LED_BRIGHTNESS_INCREMENT 20

twr_tmp112_t tmp112;
float temperature;
twr_tick_t nextTemperaturePublish;

twr_tag_barometer_t barometerTag;
float pressure;
twr_tick_t nextPressurePublish;

twr_tag_humidity_t humidityTag;
float humidity;
twr_tick_t nextHumidityPublish;

twr_tag_voc_lp_t vocLpTag;
uint16_t vocLp;
twr_tick_t nextVOCPublish;

static uint32_t _twr_module_power_led_strip_dma_buffer[LED_STRIP_COUNT * LED_STRIP_TYPE * 2];
const twr_led_strip_buffer_t led_strip_buffer =
{
    .type = LED_STRIP_TYPE,
    .count = LED_STRIP_COUNT,
    .buffer = _twr_module_power_led_strip_dma_buffer
};

/*
LED strip structure that holds all the data about the strip
*/
static struct
{
    enum
    {
        LED_STRIP_SHOW_COLOR = 0,
        LED_STRIP_SHOW_EFFECT = 2,
    } show;
    twr_led_strip_t self;
    uint8_t ledStripBrightness;
    uint32_t color;
    uint16_t currentState;
    struct
    {
        uint8_t data[TWR_RADIO_NODE_MAX_COMPOUND_BUFFER_SIZE];
        int length;
    } compound;
    twr_scheduler_task_id_t update_task_id;

} led_strip = { .show = LED_STRIP_SHOW_COLOR, .color = 0xff000000, .ledStripBrightness = 125, .currentState = 0};

twr_scheduler_task_id_t update_led_task_id;

bool twrLEDStripOn = true;

void led_strip_change_state();

/*
Radio callback to when the relay on the power module should be set to some value
*/
void twr_radio_node_on_state_set(uint64_t *id, uint8_t state_id, bool *state)
{
    if (state_id == TWR_RADIO_NODE_STATE_POWER_MODULE_RELAY)
    {
        twr_module_power_relay_set_state(*state);

        twr_radio_pub_state(TWR_RADIO_PUB_STATE_POWER_MODULE_RELAY, state);
    }
}

/*
Radio callback to when the LED strip effect should be changed
*/
void twr_radio_node_on_led_strip_effect_set(uint64_t *id, twr_radio_node_led_strip_effect_t type, uint16_t wait, uint32_t *color)
{
    led_strip.currentState = type + 1;
    led_strip.show = LED_STRIP_SHOW_EFFECT;
    twrLEDStripOn = true;

    switch (type) {
        case TWR_RADIO_NODE_LED_STRIP_EFFECT_TEST:
        {
            led_strip.currentState = 0;
            led_strip.show = LED_STRIP_SHOW_COLOR;
            twr_led_strip_effect_test(&led_strip.self);
            break;
        }
        case TWR_RADIO_NODE_LED_STRIP_EFFECT_RAINBOW:
        {
            twr_led_strip_effect_rainbow(&led_strip.self, wait);
            break;
        }
        case TWR_RADIO_NODE_LED_STRIP_EFFECT_RAINBOW_CYCLE:
        {
            twr_led_strip_effect_rainbow_cycle(&led_strip.self, wait);
            break;
        }
        case TWR_RADIO_NODE_LED_STRIP_EFFECT_THEATER_CHASE_RAINBOW:
        {
            twr_led_strip_effect_theater_chase_rainbow(&led_strip.self, wait);
            break;
        }
        case TWR_RADIO_NODE_LED_STRIP_EFFECT_COLOR_WIPE:
        {
            twr_led_strip_effect_color_wipe(&led_strip.self, *color, wait);
            led_strip.show = LED_STRIP_SHOW_COLOR;
            led_strip.color = *color;

            break;
        }
        case TWR_RADIO_NODE_LED_STRIP_EFFECT_THEATER_CHASE:
        {
            twr_led_strip_effect_theater_chase(&led_strip.self, *color, wait);
            led_strip.color = *color;

            break;
        }
        case TWR_RADIO_NODE_LED_STRIP_EFFECT_STROBOSCOPE:
        {
            twr_led_strip_effect_stroboscope(&led_strip.self, *color, wait);
            led_strip.color = *color;

            break;
        }
        case TWR_RADIO_NODE_LED_STRIP_EFFECT_ICICLE:
        {
            twr_led_strip_effect_icicle(&led_strip.self, *color, wait);
            led_strip.color = *color;

            break;
        }
        case TWR_RADIO_NODE_LED_STRIP_EFFECT_PULSE_COLOR:
        {
            twr_led_strip_effect_pulse_color(&led_strip.self, *color, wait);
            led_strip.color = *color;

            break;
        }
        default:
            return;
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

        // Read temperature
        if ((fabs(celsius - temperature) >= TEMPERATURE_VALUE_CHANGE_PUB_TRESHOLD) || (nextTemperaturePublish < twr_scheduler_get_spin_tick()))
        {
            twr_radio_pub_temperature(TWR_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_DEFAULT, &celsius);
            temperature = celsius;
            nextTemperaturePublish = twr_scheduler_get_spin_tick() + TEMPERATURE_TAG_PUB_NO_CHANGE_INTEVAL;
        }
    }
}

/*
Event handler for the Barometer tag

It will detect the update and send the value over MQTT in case there is a big change or the selected time
elapsed from the last message
*/
void barometer_tag_event_handler(twr_tag_barometer_t *self, twr_tag_barometer_event_t event, void *event_param)
{
    if(event == TWR_TAG_BAROMETER_EVENT_UPDATE)
    {
        float pascals;
        twr_tag_barometer_get_pressure_pascal(self, &pascals);

        if ((fabs(pascals - pressure) >= PRESSURE_VALUE_CHANGE_PUB_TRESHOLD) || (nextPressurePublish < twr_scheduler_get_spin_tick()))
        {
            twr_radio_pub_barometer(TWR_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_DEFAULT, &pascals, NULL);
            pressure = pascals;
            nextPressurePublish = twr_scheduler_get_spin_tick() + BAROMETER_TAG_PUB_NO_CHANGE_INTEVAL;
        }
    }
}

/*
Event handler for the Humidity tag

It will detect the update and send the value over MQTT in case there is a big change or the selected time
elapsed from the last message
*/
void humidity_tag_event_handler(twr_tag_humidity_t *self, twr_tag_humidity_event_t event, void *event_param)
{
    if(event == TWR_TAG_HUMIDITY_EVENT_UPDATE)
    {
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
Event handler for the VOC tag

It will detect the update and send the value over MQTT in case there is a big change or the selected time
elapsed from the last message
*/
void voc_lp_event_handler(twr_tag_voc_lp_t *self, twr_tag_voc_lp_event_t event, void *event_param)
{
    if(event == TWR_TAG_VOC_LP_EVENT_UPDATE)
    {
        uint16_t vocLpPpb;
        twr_tag_voc_lp_get_tvoc_ppb(self, &vocLpPpb);

        if ((fabs(vocLpPpb - vocLp) >= VOC_VALUE_CHANGE_PUB_TRESHOLD) || (nextVOCPublish < twr_scheduler_get_spin_tick()))
        {
            int sendedValue = vocLpPpb;
            twr_radio_pub_int("voc-lp-sensor/0:0/tvoc", &sendedValue);
            vocLp = vocLpPpb;
            nextVOCPublish = twr_scheduler_get_spin_tick() + VOC_TAG_PUB_NO_CHANGE_INTEVAL;
        }
    }
}

/*
Radio callback when the brightness of LED strip should be changed
*/
void twr_radio_node_on_led_strip_brightness_set(uint64_t *id, uint8_t *brightness)
{
    twr_led_strip_set_brightness(&led_strip.self, *brightness);
    twr_scheduler_plan_now(update_led_task_id);
}

/*
Radio callback to when the color of LED strip should be changed
*/
void twr_radio_node_on_led_strip_color_set(uint64_t *id, uint32_t *color)
{
    twrLEDStripOn = true;

    twr_led_strip_effect_stop(&led_strip.self);

    led_strip.color = *color;

    led_strip.show = LED_STRIP_SHOW_COLOR;

    led_strip.currentState = 0;

    led_strip_change_state();

    twr_scheduler_plan_now(update_led_task_id);
}

/*
This function will scroll all the pre programed effects on the LED strip so you can choose what suits you
*/
void led_strip_change_state()
{
    led_strip.show = LED_STRIP_SHOW_EFFECT;
    twr_led_strip_effect_stop(&led_strip.self);
    switch (led_strip.currentState) {
        case 0:
        {
            led_strip.show = LED_STRIP_SHOW_COLOR;

            twr_led_strip_fill(&led_strip.self, led_strip.color);
            break;
        }
        case 1:
        {
            twr_led_strip_effect_rainbow(&led_strip.self, 50);
            break;
        }
        case 2:
        {
            twr_led_strip_effect_rainbow_cycle(&led_strip.self, 50);
            break;
        }
        case 3:
        {
            twr_led_strip_effect_theater_chase_rainbow(&led_strip.self, 50);
            break;
        }
        case 4:
        {
            twr_led_strip_effect_color_wipe(&led_strip.self, led_strip.color, 50);
            break;
        }
        case 5:
        {
            twr_led_strip_effect_theater_chase(&led_strip.self, led_strip.color, 50);
            break;
        }
        case 6:
        {
            twr_led_strip_effect_stroboscope(&led_strip.self, led_strip.color, 50);
            break;
        }
        case 7:
        {
            twr_led_strip_effect_icicle(&led_strip.self, led_strip.color, 50);
            break;
        }
        case 8:
        {
            twr_led_strip_effect_pulse_color(&led_strip.self, led_strip.color, 50);
            break;
        }
        default:
            return;
    }
}

/*
Event handler for the Encoder module

The encoder module will take care about the brightness, changing effect and color of the LED strip

On the encoder rotation to the right the brightness will increase
On the encoder rotation to the left the brightness will decrease

On the encoder knob hold it will turn the LED strip on/off
On the encoder knob click it will change the effect so you can scroll through all the pre programed effects
*/
void encoder_event_handler(twr_module_encoder_event_t event, void *event_param)
{
    if(event == TWR_MODULE_ENCODER_EVENT_CLICK)
    {
        led_strip.currentState++;
        if(led_strip.currentState > 8)
        {
            led_strip.currentState = 0;
        }
        led_strip_change_state();
    }
    if(event == TWR_MODULE_ENCODER_EVENT_HOLD)
    {
        twr_led_strip_effect_stop(&led_strip.self);
        if(twrLEDStripOn)
        {
            twr_led_strip_fill(&led_strip.self, 0);
            twrLEDStripOn = false;
        }
        else
        {
            led_strip_change_state();
            twrLEDStripOn = true;
        }
    }

    else if(event == TWR_MODULE_ENCODER_EVENT_ROTATION)
    {
        if(twrLEDStripOn)
        {
            if(twr_module_encoder_get_increment() == 1)
            {
                if((led_strip.ledStripBrightness + LED_BRIGHTNESS_INCREMENT) < 255)
                {
                    led_strip.ledStripBrightness += LED_BRIGHTNESS_INCREMENT;
                }
                else
                {
                    led_strip.ledStripBrightness = 255;
                }
            }
            if(twr_module_encoder_get_increment() == -1)
            {
                if((led_strip.ledStripBrightness - LED_BRIGHTNESS_INCREMENT) > 0)
                {
                    led_strip.ledStripBrightness -= LED_BRIGHTNESS_INCREMENT;
                }
                else
                {
                    led_strip.ledStripBrightness = 0;
                }
            }
            twr_scheduler_plan_now(update_led_task_id);
            twr_led_strip_set_brightness(&led_strip.self, led_strip.ledStripBrightness);
        }
    }
}

/*
LED strip update task that will write the values to the LED strip periodically
*/
void led_strip_update_task(void *param)
{
    if (!twr_led_strip_is_ready(&led_strip.self))
    {
        twr_scheduler_plan_current_now();

        return;
    }

    if(twrLEDStripOn && led_strip.show == LED_STRIP_SHOW_COLOR)
    {
        twr_led_strip_fill(&led_strip.self, led_strip.color);
    }
    else if(!twrLEDStripOn)
    {
        twr_led_strip_fill(&led_strip.self, 0);
    }


    twr_led_strip_write(&led_strip.self);
    twr_scheduler_plan_current_relative(250);
}

/*
Init function that runs once at the beginning of the program
*/
void application_init(void)
{
    twr_log_init(TWR_LOG_LEVEL_DUMP, TWR_LOG_TIMESTAMP_ABS);

    twr_tmp112_init(&tmp112, TWR_I2C_I2C0, 0x49);
    twr_tmp112_set_event_handler(&tmp112, tmp112_event_handler, NULL);
    twr_tmp112_set_update_interval(&tmp112, 5000);

    twr_tag_barometer_init(&barometerTag, TWR_I2C_I2C0);
    twr_tag_barometer_set_event_handler(&barometerTag, barometer_tag_event_handler, NULL);
    twr_tag_barometer_set_update_interval(&barometerTag, 5000);

    twr_tag_humidity_init(&humidityTag, TWR_TAG_HUMIDITY_REVISION_R3, TWR_I2C_I2C0, TWR_TAG_HUMIDITY_I2C_ADDRESS_DEFAULT);
    twr_tag_humidity_set_event_handler(&humidityTag, humidity_tag_event_handler, NULL);
    twr_tag_humidity_set_update_interval(&humidityTag, 5000);

    twr_tag_voc_lp_init(&vocLpTag, TWR_I2C_I2C0);
    twr_tag_voc_lp_set_event_handler(&vocLpTag, voc_lp_event_handler, NULL);
    twr_tag_voc_lp_set_update_interval(&vocLpTag, 5000);

    twr_module_encoder_init();
    twr_module_encoder_set_event_handler(encoder_event_handler, NULL);

    twr_module_power_init();

    update_led_task_id = twr_scheduler_register(led_strip_update_task, NULL, 0);

    twr_led_strip_init(&led_strip.self, twr_module_power_get_led_strip_driver(), &led_strip_buffer);

    twr_radio_init(TWR_RADIO_MODE_NODE_LISTENING);
    twr_radio_pairing_request("climate-with-led-encoder", VERSION);

    twr_led_strip_set_brightness(&led_strip.self, led_strip.ledStripBrightness);
    led_strip_change_state();
}
