#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "log.h"
#include "pwm.h"
#include "ble_service.h"
#include "cli.h"

int main(void)
{
    // Initialize.
    logs_init();
    pwm_rgb_init();
    start_ble();
    init_usb_cli();

    while (true)
    {   
        while (app_usbd_event_queue_process())
        {
            /* Nothing to do */
        }

        idle_state_handle();
    }
}