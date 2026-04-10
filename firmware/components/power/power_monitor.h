/**
 * power_monitor.h — Battery voltage and power source monitoring
 *
 * Uses ESP32 ADC to read 18650 battery voltage via a resistor divider:
 *   R1 = 100kΩ (upper), R2 = 220kΩ (lower)
 *   Battery 4.2V → ADC pin 2.89V (within 3.3V ADC range)
 *
 * ADC pin: GPIO34 (input-only pin, recommended for ADC use)
 */

#pragma once

#include "esp_err.h"
#include <stdint.h>

// Voltage divider ratio: R2 / (R1 + R2) = 220 / 320 = 0.6875
#define VDIV_RATIO      0.6875f
#define ADC_VREF_MV     3300
#define ADC_MAX_RAW     4095    // 12-bit ADC

// Battery thresholds (mV)
#define BATTERY_FULL_MV   4200
#define BATTERY_EMPTY_MV  3000

#define POWER_ADC_GPIO  34

typedef enum {
    POWER_SOURCE_CAR     = 0,
    POWER_SOURCE_BATTERY = 1,
} power_source_t;

typedef struct {
    uint32_t      battery_mv;   // Battery voltage in millivolts
    uint8_t       battery_pct;  // Battery percentage 0–100
    power_source_t source;      // Current power source
} power_data_t;

/**
 * Initialize ADC for battery voltage reading.
 */
esp_err_t power_monitor_init(void);

/**
 * Read current power state into the provided structure.
 * @param out Pointer to power_data_t to fill
 */
void power_monitor_read(power_data_t *out);
