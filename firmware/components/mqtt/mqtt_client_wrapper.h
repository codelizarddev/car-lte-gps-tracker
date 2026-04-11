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

// MQTT Topic prefix — set CONFIG_VEHICLE_ID in credentials.h (e.g. "my-car", "truck-1")
// Resulting topics: car/<vehicle_id>/location, car/<vehicle_id>/cmd/relay, etc.
#define _TOPIC(suffix)      "car/" CONFIG_VEHICLE_ID "/" suffix

#define TOPIC_LOCATION      _TOPIC("location")
#define TOPIC_STATE         _TOPIC("state")
#define TOPIC_POWER         _TOPIC("power")
#define TOPIC_CMD_RELAY     _TOPIC("cmd/relay")
#define TOPIC_RELAY_STATUS  _TOPIC("cmd/relay/status")

/**
 * Initialize MQTT client (load config, create client handle).
 */
esp_err_t mqtt_client_init(void);

/**
 * Connect to MQTT broker (blocking until connected or timeout).
 */
esp_err_t mqtt_client_connect(void);

/**
 * Subscribe to relay command topic.
 */
void mqtt_subscribe_relay_cmd(void);

/**
 * Publish GPS location data.
 */
void mqtt_publish_location(const gnss_data_t *loc);

/**
 * Publish system state string (e.g. "online", "restarting").
 */
void mqtt_publish_state(const char *state);

/**
 * Publish power/battery data.
 */
void mqtt_publish_power(const power_data_t *pwr);

/**
 * Publish current relay state (confirmation after command).
 */
void mqtt_publish_relay_status(const char *status);
