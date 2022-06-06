#ifndef LOG_H
#define LOG_H

#include "nordic_common.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_backend_usb.h"

#include "nrf_pwr_mgmt.h"

void logs_init(void);

void idle_state_handle(void);

#endif