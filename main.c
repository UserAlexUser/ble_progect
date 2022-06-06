#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "log.h"
#include "pwm.h"
#include "ble_service.h"

int main(void)
{
    // Initialize.
    logs_init();
    pwm_rgb_init();
    start_ble();

    while (true)
    {
        idle_state_handle();
    }
}