#pragma once

#include <stdint.h>

typedef enum {
    POWER_SOURCE_CAR     = 0,
    POWER_SOURCE_BATTERY = 1,
} power_source_t;

typedef struct {
    uint32_t       battery_mv;   // Battery voltage in millivolts
    uint8_t        battery_pct;  // Battery percentage 0-100
    power_source_t source;       // Current power source
} power_data_t;
