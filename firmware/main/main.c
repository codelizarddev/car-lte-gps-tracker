/**
 * Car LTE GPS Tracker — Main Entry Point
 *
 * Platform: ESP32-WROVER (LILYGO T-SIM7600)
 * Framework: ESP-IDF
 *
 * https://github.com/molnard/car-lte-gps-tracker
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "modem.h"
#include "gnss.h"
#include "mqtt_client_wrapper.h"
#include "relay.h"
#include "power_monitor.h"

static const char *TAG = "MAIN";

// Task handles
static TaskHandle_t gnss_task_handle     = NULL;
static TaskHandle_t state_task_handle    = NULL;
static TaskHandle_t power_task_handle    = NULL;

/**
 * GNSS polling task — reads location and publishes every GNSS_PUBLISH_INTERVAL_S
 */
static void gnss_task(void *pvParameters)
{
    gnss_data_t location;

    while (1) {
        if (gnss_get_location(&location) == ESP_OK) {
            mqtt_publish_location(&location);
        }
        vTaskDelay(pdMS_TO_TICKS(CONFIG_GNSS_PUBLISH_INTERVAL_S * 1000));
    }
}

/**
 * Power monitor task — reads battery voltage and publishes every POWER_PUBLISH_INTERVAL_S
 */
static void power_task(void *pvParameters)
{
    power_data_t pwr;

    while (1) {
        power_monitor_read(&pwr);
        mqtt_publish_power(&pwr);
        vTaskDelay(pdMS_TO_TICKS(CONFIG_POWER_PUBLISH_INTERVAL_S * 1000));
    }
}

/**
 * State heartbeat task — republishes online state so monitoring can detect stale devices
 */
static void state_task(void *pvParameters)
{
    while (1) {
        mqtt_publish_state("online");
        vTaskDelay(pdMS_TO_TICKS(CONFIG_STATE_PUBLISH_INTERVAL_S * 1000));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Car LTE GPS Tracker starting...");

    // Initialize NVS (non-volatile storage) for credentials and state
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize relay — CLOSED by default (fail-safe)
    relay_init();
    relay_set(RELAY_CLOSED);
    ESP_LOGI(TAG, "Relay initialized: CLOSED (fuel pump enabled)");

    // Initialize ADC for battery voltage monitoring
    power_monitor_init();

    // Initialize SIM7600 modem (AT commands over UART)
    ESP_LOGI(TAG, "Initializing modem...");
    ESP_ERROR_CHECK(modem_init());
    ESP_ERROR_CHECK(modem_connect_lte());

    // Initialize GNSS
    ESP_LOGI(TAG, "Initializing GNSS...");
    ESP_ERROR_CHECK(gnss_init());

    // Initialize MQTT client and connect to broker
    ESP_LOGI(TAG, "Connecting to MQTT broker...");
    ESP_ERROR_CHECK(mqtt_client_init());
    ESP_ERROR_CHECK(mqtt_client_connect());

    // Subscribe to relay command topic
    mqtt_subscribe_relay_cmd();

    // Publish initial state
    mqtt_publish_state("online");

    // Start background tasks
    xTaskCreate(gnss_task,  "gnss_task",  4096, NULL, 5, &gnss_task_handle);
    xTaskCreate(state_task, "state_task", 2048, NULL, 2, &state_task_handle);
    xTaskCreate(power_task, "power_task", 2048, NULL, 3, &power_task_handle);

    ESP_LOGI(TAG, "System ready.");
}
