/*
Hlavičkový soubor generovaný šablonou
Autor: HARDWARIO s.r.o.
*/

#ifndef _APPLICATION_H
#define _APPLICATION_H

#ifndef VERSION
#define VERSION "vdev"
#endif

#include <twr.h>
#include <bcl.h>

#define MINUTE 60 * 1000

void qrcode_project(char *project_name);
void twr_change_qr_value(uint64_t *id, const char *topic, void *value, void *param);
void lcd_page_with_qr_code();

typedef enum
{
    PASSWD = 0,
    SSID = 1,
    ENCRYPTION = 2
}twr_wifi_command_t;

typedef struct
{
    float value;
    uint8_t number;
    bc_tick_t next_pub;
} temperature_params;


#endif // _APPLICATION_H
