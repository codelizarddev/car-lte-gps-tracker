/**
 * mqtt_client_wrapper.h — MQTT client interface
 *
 * Connects to Mosquitto broker over TLS (port 8883).
 * Credentials and server address are loaded from credentials.h (not committed to git).
 *
 * Topic structure:
 *   car/my-car/location         — GPS position JSON (published)
 *   car/my-car/state            — System state JSON (published)
 *   car/my-car/power            — Battery/power JSON (published)
 *   car/my-car/cmd/relay        — Relay command ON/OFF (subscribed)
 *   car/my-car/cmd/relay/status — Relay state confirmation (published)
 */

#pragma once

#include "esp_err.h"
#include "gnss.h"
#include "power_monitor.h"

#define _TOPIC(suffix)      "car/" CONFIG_VEHICLE_ID "/" suffix

#define TOPIC_LOCATION      _TOPIC("location")
#define TOPIC_STATE         _TOPIC("state")
#define TOPIC_POWER         _TOPIC("power")
#define TOPIC_CMD_RELAY     _TOPIC("cmd/relay")
#define TOPIC_RELAY_STATUS  _TOPIC("cmd/relay/status")

esp_err_t mqtt_client_init(void);
esp_err_t mqtt_client_connect(void);
void mqtt_subscribe_relay_cmd(void);
void mqtt_publish_location(const gnss_data_t *loc);
void mqtt_publish_state(const char *state);
void mqtt_publish_power(const power_data_t *pwr);
void mqtt_publish_relay_status(const char *status);
