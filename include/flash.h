#ifndef MEMORY_MODULE_H
#define MEMORY_MODULE_H

#include <stdint.h>
#include "nrf_fstorage.h"
#include "pwm.h"

#define FIRST_PAGE_FLASH    0xDE000U
#define END_PAGE_FLASH      0xDEFFFU
#define SIZE_STEP_FLASH     0x4U
#define COUNT_PAGES_FLASH   2

void init_flash(rgb_t *color);

bool read_flash(rgb_t* out_data);

void write_to_flash(const rgb_t* in_data);

#endif