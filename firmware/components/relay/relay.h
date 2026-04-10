/**
 * relay.h — Relay control interface
 *
 * The relay operates on the fuel pump relay coil control line.
 * Default state: CLOSED (fuel pump enabled, fail-safe).
 *
 * Relay logic:
 *   RELAY_CLOSED → GPIO LOW  → transistor OFF → relay coil de-energized → NC contact closed → pump works
 *   RELAY_OPEN   → GPIO HIGH → transistor ON  → relay coil energized   → NC contact open   → pump disabled
 */

#pragma once

#include "esp_err.h"

// GPIO pin connected to relay transistor base resistor
#define RELAY_GPIO_PIN  26

typedef enum {
    RELAY_CLOSED = 0,  // Default — fuel pump enabled
    RELAY_OPEN   = 1,  // Fuel pump disabled
} relay_state_t;

/**
 * Initialize relay GPIO. Sets output LOW (relay closed) immediately.
 */
void relay_init(void);

/**
 * Set relay state.
 * @param state RELAY_CLOSED or RELAY_OPEN
 */
void relay_set(relay_state_t state);

/**
 * Get current relay state.
 * @return Current relay_state_t
 */
relay_state_t relay_get(void);
