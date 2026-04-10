/**
 * mqtt_client_wrapper.h — MQTT client interface
 *
 * Connects to Mosquitto broker over TLS (port 8883).
 * Credentials and server address are loaded from credentials.h (not committed to git).
 *
 * Topic structure:
 *   car/xsara/location         — GPS position JSON (published)
 *   car/xsara/state            — System state JSON (published)
 *   car/xsara/power            — Battery/power JSON (published)
 *   car/xsara/cmd/relay        — Relay command ON/OFF (subscribed)
 *   car/xsara/cmd/relay/status — Relay state confirmation (published)
 */

#pragma once

#include "esp_err.h"
#include "gnss.h"
#include "power_monitor.h"

// MQTT Topics
#define TOPIC_LOCATION      "car/xsara/location"
#define TOPIC_STATE         "car/xsara/state"
#define TOPIC_POWER         "car/xsara/power"
#define TOPIC_CMD_RELAY     "car/xsara/cmd/relay"
#define TOPIC_RELAY_STATUS  "car/xsara/cmd/relay/status"

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
