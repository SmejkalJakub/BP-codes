/*
Device with a thermostat function and a possibility to show the QR code to connect to WIFI.

This firmware is a collection of more projects.

https://github.com/hardwario/twr-radio-lcd-thermostat
https://github.com/hardwario/bcf-radio-qr-wifi-terminal

Author: Jakub Smejkal
        HARDWARIO s.r.o.
Date:   2.4.2021
*/

#include <application.h>
#include <qrcodegen.h>

#define BATTERY_UPDATE_INTERVAL (60 * MINUTE)

#define TEMPERATURE_TAG_PUB_NO_CHANGE_INTEVAL (10 * MINUTE)
#define VALUE_CHANGE_PUB_TRESHOLD 0.2f

#define EEPROM_SET_TEMPERATURE_ADDRESS 0
#define BLACK_COLOR 1

#define SET_TEMPERATURE_PUB_INTERVAL 15 * MINUTE
#define SET_TEMPERATURE_ADD_ON_CLICK 0.5f

// GFX instance
twr_gfx_t *gfx;

float displayed_temperature = NAN;

temperature_params temperature_param = { .next_pub = 0, .value = NAN };
temperature_params thermostat_set_point;

// QR code data values
char qr_code[150];

// Custom MQTT topic for getting the QR code data
static const twr_radio_sub_t subs[] = {
    {"qr/-/chng/code", TWR_RADIO_SUB_PT_STRING, twr_change_qr_value, (void *) PASSWD}
};

uint32_t display_page_index = 0;
twr_tmp112_t temp;

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
Event handler for the Temperature sensor

It will detect the update and send the value over MQTT in case there is a big change or the selected time
elapsed from the last message
*/
void tmp112_event_handler(twr_tmp112_t *self, twr_tmp112_event_t event, void *event_param)
{
    if (event == TWR_TMP112_EVENT_UPDATE)
    {
        float temperature = 0.0;

        if (twr_tmp112_get_temperature_celsius(self, &temperature))
        {
            if ((fabsf(temperature - temperature_param.value) >= VALUE_CHANGE_PUB_TRESHOLD) || (temperature_param.next_pub < twr_scheduler_get_spin_tick()))
            {
                twr_radio_pub_temperature(TWR_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_ALTERNATE, &temperature);
                temperature_param.value = temperature;
                temperature_param.next_pub = twr_scheduler_get_spin_tick() + TEMPERATURE_TAG_PUB_NO_CHANGE_INTEVAL;
            }
        }
        else
        {
            temperature_param.value = NAN;
        }

        if (thermostat_set_point.next_pub < twr_scheduler_get_spin_tick())
        {
            thermostat_set_point.next_pub = twr_scheduler_get_spin_tick() + SET_TEMPERATURE_PUB_INTERVAL;

            twr_radio_pub_temperature(TWR_RADIO_PUB_CHANNEL_SET_POINT, &thermostat_set_point.value);
        }

        if ((fabsf(temperature_param.value - displayed_temperature) >= 0.1) || isnan(displayed_temperature))
        {
            twr_scheduler_plan_now(0);
        }
    }
}

/*
This function will be called when the message on a custom topic is received and it will change the QR code value
*/
void twr_change_qr_value(uint64_t *id, const char *topic, void *value, void *param)
{
    strncpy(qr_code, value, sizeof(qr_code));
    char ch = ';';
    strncat(qr_code, &ch, 1);

    twr_eeprom_write(0, qr_code, sizeof(qr_code));
}

static void print_qr(const uint8_t qrcode[])
{
    twr_gfx_clear(gfx);

    twr_gfx_set_font(gfx, &twr_font_ubuntu_13);
    twr_gfx_draw_string(gfx, 2, 0, "Scan and connect", true);

    uint32_t offset_x = 8;
    uint32_t offset_y = 32;
    uint32_t box_size = 3;
	int size = qrcodegen_getSize(qrcode);
	int border = 2;
	for (int y = -border; y < size + border; y++) {
		for (int x = -border; x < size + border; x++) {
            uint32_t x1 = offset_x + x * box_size;
            uint32_t y1 = offset_y + y * box_size;
            uint32_t x2 = x1 + box_size;
            uint32_t y2 = y1 + box_size;

            twr_gfx_draw_fill_rectangle(gfx, x1, y1, x2, y2, qrcodegen_getModule(qrcode, x, y));
		}
	}
    twr_gfx_update(gfx);
}

/*
Event handler for the LCD module

This function will taky care of all the LCD operations. Mostly it handles the button presses.

On left button click it will decrease the set point temperature by static value
On left button click it will increase the set point temperature by static value

On right button hold it will change the page on display

*/
void lcd_event_handler(twr_module_lcd_event_t event, void *event_param)
{
    if(event == TWR_MODULE_LCD_EVENT_RIGHT_HOLD)
    {
        if(display_page_index == 1)
        {
            display_page_index--;
        }
        else if(display_page_index == 0)
        {
            display_page_index++;
        }
        else
        {
            display_page_index = 0;
        }
        twr_scheduler_plan_now(0);

    }
    else if(event == TWR_MODULE_LCD_EVENT_LEFT_CLICK)
    {
        if(display_page_index == 0)
        {
            thermostat_set_point.value -= SET_TEMPERATURE_ADD_ON_CLICK;

            thermostat_set_point.next_pub = twr_scheduler_get_spin_tick() + SET_TEMPERATURE_PUB_INTERVAL;
            twr_radio_pub_temperature(TWR_RADIO_PUB_CHANNEL_SET_POINT, &thermostat_set_point.value);

            uint32_t neg_set_temperature;
            float *set_temperature = (float *) &neg_set_temperature;

            *set_temperature = thermostat_set_point.value;

            neg_set_temperature = ~neg_set_temperature;

            twr_eeprom_write(EEPROM_SET_TEMPERATURE_ADDRESS, &thermostat_set_point.value, sizeof(thermostat_set_point.value));
            twr_eeprom_write(EEPROM_SET_TEMPERATURE_ADDRESS + sizeof(thermostat_set_point.value), &neg_set_temperature, sizeof(neg_set_temperature));

            twr_scheduler_plan_now(0);
            twr_radio_pub_push_button(0);
        }
    }
    else if(event == TWR_MODULE_LCD_EVENT_RIGHT_CLICK)
    {
        if(display_page_index == 0)
        {
            thermostat_set_point.value += SET_TEMPERATURE_ADD_ON_CLICK;

            thermostat_set_point.next_pub = bc_scheduler_get_spin_tick() + SET_TEMPERATURE_PUB_INTERVAL;
            twr_radio_pub_temperature(TWR_RADIO_PUB_CHANNEL_SET_POINT, &thermostat_set_point.value);

            uint32_t neg_set_temperature;
            float *set_temperature = (float *) &neg_set_temperature;

            *set_temperature = thermostat_set_point.value;

            neg_set_temperature = ~neg_set_temperature;

            twr_eeprom_write(EEPROM_SET_TEMPERATURE_ADDRESS, &thermostat_set_point.value, sizeof(thermostat_set_point.value));
            twr_eeprom_write(EEPROM_SET_TEMPERATURE_ADDRESS + sizeof(thermostat_set_point.value), &neg_set_temperature, sizeof(neg_set_temperature));

            twr_scheduler_plan_now(0);
            twr_radio_pub_push_button(0);
        }
    }
}

/*
This function will call the qr code encoder and then print the QR code to the display
*/
void qrcode_handler(char *text)
{
    twr_system_pll_enable();

	// Make and print the QR Code symbol
	uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
	uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
	bool ok = qrcodegen_encodeText(text, tempBuffer, qrcode, qrcodegen_Ecc_LOW,	qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);

	if (ok)
    {
		print_qr(qrcode);
    }

    twr_system_pll_disable();
}

/*
Init function that runs once at the beginning of the program
*/
void application_init(void)
{
    twr_log_init(TWR_LOG_LEVEL_DUMP, TWR_LOG_TIMESTAMP_ABS);
    twr_log_debug("INIT");

    twr_radio_init(TWR_RADIO_MODE_NODE_SLEEPING);

    twr_radio_set_subs((twr_radio_sub_t *) subs, sizeof(subs)/sizeof(twr_radio_sub_t));
    twr_radio_set_rx_timeout_for_sleeping_node(250);

    uint32_t neg_set_temperature;
    float *set_temperature = (float *) &neg_set_temperature;

    twr_eeprom_read(EEPROM_SET_TEMPERATURE_ADDRESS, &thermostat_set_point.value, sizeof(thermostat_set_point.value));
    twr_eeprom_read(EEPROM_SET_TEMPERATURE_ADDRESS + sizeof(thermostat_set_point.value), &thermostat_set_point, sizeof(neg_set_temperature));

    neg_set_temperature = ~neg_set_temperature;

    if (thermostat_set_point.value != *set_temperature)
    {
        thermostat_set_point.value = 21.0f;
    }

    twr_module_lcd_init();
    gfx = twr_module_lcd_get_gfx();
    twr_module_lcd_set_event_handler(lcd_event_handler, NULL);
    twr_module_lcd_set_button_hold_time(500);

    twr_tmp112_init(&temp, TWR_I2C_I2C0, TWR_TAG_TEMPERATURE_I2C_ADDRESS_ALTERNATE);
    twr_tmp112_set_event_handler(&temp, tmp112_event_handler, NULL);
    twr_tmp112_set_update_interval(&temp, 10 * 1000);

    twr_eeprom_read(0, qr_code, sizeof(qr_code));

    if(strstr(qr_code, "WIFI:S:") == NULL)
    {
        strncpy(qr_code, "WIFI:S:test;T:test;P:test;;", sizeof(qr_code));
    }

    twr_module_battery_init();
    twr_module_battery_set_event_handler(battery_event_handler, NULL);
    twr_module_battery_set_update_interval(BATTERY_UPDATE_INTERVAL);

    twr_radio_pairing_request("thermostat-with-qr-terminal", VERSION);
}

/*
This function will run in a loop for the whole duration of the app.

It will take care of redrawing the data on the LCD display and changing the pages
*/
void application_task(void)
{
    if(display_page_index == 0)
    {
        static char str_temperature[10];

        if (!twr_module_lcd_is_ready())
        {
            return;
        }

        twr_system_pll_enable();

        twr_module_lcd_clear();

        twr_gfx_clear(gfx);

        twr_module_lcd_set_font(&twr_font_ubuntu_33);
        snprintf(str_temperature, sizeof(str_temperature), "%.1f   ", temperature_param.value);
        int x = twr_module_lcd_draw_string(20, 20, str_temperature, BLACK_COLOR);
        displayed_temperature = temperature_param.value;

        twr_module_lcd_set_font(&twr_font_ubuntu_24);
        twr_module_lcd_draw_string(x - 20, 25, "\xb0" "C   ", BLACK_COLOR);

        twr_module_lcd_set_font(&twr_font_ubuntu_15);
        twr_module_lcd_draw_string(10, 80, "Set temperature", BLACK_COLOR);

        snprintf(str_temperature, sizeof(str_temperature), "%.1f \xb0" "C", thermostat_set_point.value);
        twr_module_lcd_draw_string(40, 100, str_temperature, BLACK_COLOR);

        twr_module_lcd_update();

        twr_system_pll_disable();
    }
    if(display_page_index == 1)
    {
        qrcode_handler(qr_code);
    }
}
