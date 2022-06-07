#ifndef CLI_H
#define CLI_H

#include "app_usbd.h"
#include "app_usbd_serial_num.h"
#include "app_usbd_cdc_acm.h"

void init_usb_cli(void);

typedef void(*cmd_handler_t)(void);

typedef struct
{
    const char* cmd_name;
    cmd_handler_t cmd_handler;
} cli_cmd_t;

#endif