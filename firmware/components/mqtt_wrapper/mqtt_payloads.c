#include "mqtt_payloads.h"

#include <stdio.h>

int mqtt_build_location_json(const gnss_data_t *loc, char *buf, size_t len)
{
    if (!loc || !buf || len == 0) return -1;

    return snprintf(buf, len,
                    "{\"lat\":%.6f,\"lon\":%.6f,\"alt\":%.1f,"
                    "\"speed\":%.1f,\"course\":%.1f,"
                    "\"satellites\":%u,\"hdop\":%.1f,"
                    "\"date\":\"%s\",\"time\":\"%s\"}",
                    loc->latitude, loc->longitude, loc->altitude,
                    loc->speed, loc->course,
                    loc->satellites, loc->hdop,
                    loc->date, loc->utc_time);
}

int mqtt_build_power_json(const power_data_t *pwr, char *buf, size_t len)
{
    if (!pwr || !buf || len == 0) return -1;

    return snprintf(buf, len,
                    "{\"source\":\"%s\",\"battery_mv\":%lu,\"battery_pct\":%u}",
                    (pwr->source == POWER_SOURCE_CAR) ? "car" : "battery",
                    (unsigned long)pwr->battery_mv,
                    pwr->battery_pct);
}

int mqtt_build_state_json(const char *state, char *buf, size_t len)
{
    if (!state || !buf || len == 0) return -1;

    return snprintf(buf, len, "{\"status\":\"%s\"}", state);
}
