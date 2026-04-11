/**
 * mqtt_client_wrapper.c — MQTT client over TLS (HiveMQ Cloud)
 *
 * Uses esp-mqtt component from ESP-IDF.
 * Connects to broker with TLS + username/password.
 * On receiving a relay command, calls relay_set() directly.
 */

#include "mqtt_client_wrapper.h"
#include "mqtt_payloads.h"
#include "relay.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "credentials.h"

static const char *TAG = "MQTT";

static esp_mqtt_client_handle_t client      = NULL;
static EventGroupHandle_t       mqtt_events = NULL;

#define MQTT_CONNECTED_BIT  BIT0
#define MQTT_CONNECT_TIMEOUT_MS  30000

static bool topic_matches(const char *topic, int topic_len, const char *expected)
{
    size_t expected_len = strlen(expected);
    return topic_len == (int)expected_len &&
           strncmp(topic, expected, expected_len) == 0;
}

static bool payload_equals_trimmed(const char *payload, int len, const char *expected)
{
    size_t expected_len = strlen(expected);

    while (len > 0 && (*payload == ' ' || *payload == '\r' || *payload == '\n' || *payload == '\t')) {
        payload++;
        len--;
    }

    while (len > 0) {
        char c = payload[len - 1];
        if (c != ' ' && c != '\r' && c != '\n' && c != '\t') break;
        len--;
    }

    return len == (int)expected_len && strncmp(payload, expected, expected_len) == 0;
}

// ─── Event handler ────────────────────────────────────────────────────────────

static void on_relay_command(const char *payload, int len)
{
    ESP_LOGI(TAG, "Relay command received (%d bytes)", len);

    if (payload_equals_trimmed(payload, len, "OFF")) {
        relay_set(RELAY_OPEN);
        mqtt_publish_relay_status("OFF");
    } else if (payload_equals_trimmed(payload, len, "ON")) {
        relay_set(RELAY_CLOSED);
        mqtt_publish_relay_status("ON");
    } else {
        ESP_LOGW(TAG, "Unknown relay command payload");
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                                int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "Connected to broker");
        xEventGroupSetBits(mqtt_events, MQTT_CONNECTED_BIT);
        // Re-subscribe after reconnect
        esp_mqtt_client_subscribe(client, TOPIC_CMD_RELAY, 1);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "Disconnected from broker — will retry");
        xEventGroupClearBits(mqtt_events, MQTT_CONNECTED_BIT);
        break;

    case MQTT_EVENT_DATA:
        if (event->topic_len > 0 && topic_matches(event->topic, event->topic_len, TOPIC_CMD_RELAY)) {
            on_relay_command(event->data, event->data_len);
        }
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "MQTT error: type=%d",
                 event->error_handle->error_type);
        break;

    default:
        break;
    }
}

// ─── Public API ───────────────────────────────────────────────────────────────

esp_err_t mqtt_client_init(void)
{
    mqtt_events = xEventGroupCreate();
    if (!mqtt_events) return ESP_ERR_NO_MEM;

    esp_mqtt_client_config_t cfg = {
        .broker = {
            .address = {
                .hostname  = CONFIG_MQTT_HOST,
                .port      = CONFIG_MQTT_PORT,
                .transport = MQTT_TRANSPORT_OVER_SSL,
            },
            .verification = {
                .certificate = CONFIG_MQTT_CA_CERT,
            },
        },
        .credentials = {
            .username = CONFIG_MQTT_USER,
            .authentication = {
                .password = CONFIG_MQTT_PASS,
            },
        },
        .session = {
            .last_will = {
                .topic   = TOPIC_STATE,
                .msg     = "{\"status\":\"offline\"}",
                .qos     = 1,
                .retain  = 1,
            },
            .keepalive = 60,
        },
        .network = {
            .reconnect_timeout_ms = 5000,
        },
    };

    client = esp_mqtt_client_init(&cfg);
    if (!client) {
        ESP_LOGE(TAG, "Failed to create MQTT client");
        return ESP_FAIL;
    }

    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID,
                                   mqtt_event_handler, NULL);
    return ESP_OK;
}

esp_err_t mqtt_client_connect(void)
{
    esp_err_t ret = esp_mqtt_client_start(client);
    if (ret != ESP_OK) return ret;

    // Wait for connection
    EventBits_t bits = xEventGroupWaitBits(mqtt_events, MQTT_CONNECTED_BIT,
                                            pdFALSE, pdTRUE,
                                            pdMS_TO_TICKS(MQTT_CONNECT_TIMEOUT_MS));
    if (!(bits & MQTT_CONNECTED_BIT)) {
        ESP_LOGE(TAG, "MQTT connect timeout");
        return ESP_ERR_TIMEOUT;
    }
    return ESP_OK;
}

void mqtt_subscribe_relay_cmd(void)
{
    esp_mqtt_client_subscribe(client, TOPIC_CMD_RELAY, 1);
    ESP_LOGI(TAG, "Subscribed to %s", TOPIC_CMD_RELAY);
}

void mqtt_publish_location(const gnss_data_t *loc)
{
    if (!loc || !loc->valid) return;

    char payload[256];
    mqtt_build_location_json(loc, payload, sizeof(payload));

    esp_mqtt_client_publish(client, TOPIC_LOCATION, payload, 0, 0, 0);
}

void mqtt_publish_state(const char *state)
{
    char payload[64];
    mqtt_build_state_json(state, payload, sizeof(payload));
    esp_mqtt_client_publish(client, TOPIC_STATE, payload, 0, 1, 1);
}

void mqtt_publish_power(const power_data_t *pwr)
{
    if (!pwr) return;

    char payload[128];
    mqtt_build_power_json(pwr, payload, sizeof(payload));

    esp_mqtt_client_publish(client, TOPIC_POWER, payload, 0, 0, 0);
}

void mqtt_publish_relay_status(const char *status)
{
    esp_mqtt_client_publish(client, TOPIC_RELAY_STATUS, status, 0, 1, 1);
}
