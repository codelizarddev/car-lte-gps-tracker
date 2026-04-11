#pragma once

#include <stddef.h>

#include "gnss_pure.h"
#include "power_types.h"

int mqtt_build_location_json(const gnss_data_t *loc, char *buf, size_t len);
int mqtt_build_power_json(const power_data_t *pwr, char *buf, size_t len);
int mqtt_build_state_json(const char *state, char *buf, size_t len);
