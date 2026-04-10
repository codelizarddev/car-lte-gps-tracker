/**
 * relay.c — Relay control implementation
 */

#include "relay.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "RELAY";
static relay_state_t current_state = RELAY_CLOSED;

void relay_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << RELAY_GPIO_PIN),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,  // Default LOW = relay closed
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level(RELAY_GPIO_PIN, 0);  // Ensure closed on boot
    current_state = RELAY_CLOSED;
    ESP_LOGI(TAG, "Relay GPIO initialized (CLOSED)");
}

void relay_set(relay_state_t state)
{
    gpio_set_level(RELAY_GPIO_PIN, (state == RELAY_OPEN) ? 1 : 0);
    current_state = state;
    ESP_LOGI(TAG, "Relay set to: %s", (state == RELAY_OPEN) ? "OPEN (pump disabled)" : "CLOSED (pump enabled)");
}

relay_state_t relay_get(void)
{
    return current_state;
}
