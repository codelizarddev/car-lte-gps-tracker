/**
 * modem.c — SIM7600 AT command driver implementation
 */

#include "modem.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdio.h>

// credentials.h provides CONFIG_SIM_APN, CONFIG_SIM_USER, CONFIG_SIM_PASS
#include "credentials.h"

static const char *TAG = "MODEM";

#define UART_BUF_SIZE   2048
#define AT_EOL          "\r\n"

// ─── Internal helpers ────────────────────────────────────────────────────────

static void modem_uart_flush(void)
{
    uart_flush_input(MODEM_UART_NUM);
}

static void modem_power_on(void)
{
    // Board power enable
    gpio_set_level(MODEM_POWER_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(100));

    // PWRKEY pulse (SIM7600 requires >500ms low pulse to power on)
    gpio_set_level(MODEM_PWRKEY_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(500));
    gpio_set_level(MODEM_PWRKEY_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(2000));  // Wait for modem to boot
}

// ─── Public API ──────────────────────────────────────────────────────────────

esp_err_t modem_at_cmd(const char *cmd, const char *expected,
                        char *resp_buf, size_t resp_len,
                        uint32_t timeout_ms)
{
    modem_uart_flush();

    // Send command with CR+LF
    char full_cmd[256];
    snprintf(full_cmd, sizeof(full_cmd), "%s\r\n", cmd);
    uart_write_bytes(MODEM_UART_NUM, full_cmd, strlen(full_cmd));

    // Collect response until expected substring found or timeout
    char buf[UART_BUF_SIZE] = {0};
    int  total = 0;
    uint32_t elapsed = 0;
    const uint32_t tick = 50;

    while (elapsed < timeout_ms) {
        int len = uart_read_bytes(MODEM_UART_NUM,
                                  (uint8_t *)(buf + total),
                                  sizeof(buf) - total - 1,
                                  pdMS_TO_TICKS(tick));
        if (len > 0) {
            total += len;
            buf[total] = '\0';

            if (strstr(buf, expected) != NULL) {
                if (resp_buf && resp_len > 0) {
                    strncpy(resp_buf, buf, resp_len - 1);
                    resp_buf[resp_len - 1] = '\0';
                }
                ESP_LOGD(TAG, "CMD: %s  RESP: %s", cmd, buf);
                return ESP_OK;
            }

            // Hard error from modem
            if (strstr(buf, "ERROR") != NULL) {
                ESP_LOGW(TAG, "AT ERROR — cmd: %s  resp: %s", cmd, buf);
                return ESP_FAIL;
            }
        }
        elapsed += tick;
    }

    ESP_LOGW(TAG, "TIMEOUT — cmd: %s", cmd);
    return ESP_ERR_TIMEOUT;
}

esp_err_t modem_init(void)
{
    // Configure GPIO pins
    gpio_config_t io = {
        .pin_bit_mask = (1ULL << MODEM_PWRKEY_PIN) | (1ULL << MODEM_POWER_PIN),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&io);

    modem_power_on();

    // Configure UART
    uart_config_t uart_cfg = {
        .baud_rate  = MODEM_UART_BAUD,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
    };
    ESP_ERROR_CHECK(uart_param_config(MODEM_UART_NUM, &uart_cfg));
    ESP_ERROR_CHECK(uart_set_pin(MODEM_UART_NUM,
                                 MODEM_UART_TX_PIN, MODEM_UART_RX_PIN,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(MODEM_UART_NUM,
                                        UART_BUF_SIZE, UART_BUF_SIZE,
                                        0, NULL, 0));

    // Wait for modem ready, retry up to 10 times
    ESP_LOGI(TAG, "Waiting for modem...");
    for (int i = 0; i < 10; i++) {
        if (modem_at_cmd("AT", "OK", NULL, 0, AT_TIMEOUT_SHORT) == ESP_OK) {
            ESP_LOGI(TAG, "Modem responded to AT");
            break;
        }
        if (i == 9) {
            ESP_LOGE(TAG, "Modem not responding after 10 attempts");
            return ESP_ERR_TIMEOUT;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // Basic setup
    modem_at_cmd("ATE0",      "OK", NULL, 0, AT_TIMEOUT_SHORT); // Echo off
    modem_at_cmd("AT+CMEE=2", "OK", NULL, 0, AT_TIMEOUT_SHORT); // Verbose errors
    modem_at_cmd("AT+CREG=0", "OK", NULL, 0, AT_TIMEOUT_SHORT); // Disable unsolicited reg

    ESP_LOGI(TAG, "Modem initialized");
    return ESP_OK;
}

esp_err_t modem_connect_lte(void)
{
    char resp[256];

    // Wait for network registration (up to 60s)
    ESP_LOGI(TAG, "Waiting for network registration...");
    esp_err_t ret = ESP_ERR_TIMEOUT;
    for (int i = 0; i < 30; i++) {
        if (modem_at_cmd("AT+CREG?", "+CREG: 0,1", resp, sizeof(resp),
                          AT_TIMEOUT_MEDIUM) == ESP_OK ||
            modem_at_cmd("AT+CREG?", "+CREG: 0,5", resp, sizeof(resp),
                          AT_TIMEOUT_MEDIUM) == ESP_OK) {
            ret = ESP_OK;
            ESP_LOGI(TAG, "Network registered");
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Network registration failed");
        return ret;
    }

    // Set APN and activate PDP context
    char cmd[128];
    snprintf(cmd, sizeof(cmd),
             "AT+CGDCONT=1,\"IP\",\"%s\"", CONFIG_SIM_APN);
    modem_at_cmd(cmd, "OK", NULL, 0, AT_TIMEOUT_SHORT);

    // Activate context
    if (modem_at_cmd("AT+CGACT=1,1", "OK", NULL, 0, AT_TIMEOUT_LONG) != ESP_OK) {
        ESP_LOGE(TAG, "PDP context activation failed");
        return ESP_FAIL;
    }

    // Verify IP assignment
    if (modem_at_cmd("AT+CGPADDR=1", "+CGPADDR", resp, sizeof(resp),
                      AT_TIMEOUT_SHORT) != ESP_OK) {
        ESP_LOGE(TAG, "No IP address assigned");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "LTE connected: %s", resp);
    return ESP_OK;
}

void modem_disconnect(void)
{
    modem_at_cmd("AT+CGACT=0,1", "OK", NULL, 0, AT_TIMEOUT_MEDIUM);
    ESP_LOGI(TAG, "LTE disconnected");
}

esp_err_t modem_gnss_start(void)
{
    // Enable GNSS power
    esp_err_t ret = modem_at_cmd("AT+CGPS=1,1", "OK", NULL, 0, AT_TIMEOUT_MEDIUM);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start GNSS");
        return ret;
    }
    ESP_LOGI(TAG, "GNSS started");
    return ESP_OK;
}

void modem_gnss_stop(void)
{
    modem_at_cmd("AT+CGPS=0", "OK", NULL, 0, AT_TIMEOUT_MEDIUM);
}

esp_err_t modem_get_status(modem_status_t *out)
{
    if (!out) return ESP_ERR_INVALID_ARG;

    char resp[128];
    memset(out, 0, sizeof(*out));

    // Signal quality
    if (modem_at_cmd("AT+CSQ", "+CSQ:", resp, sizeof(resp),
                      AT_TIMEOUT_SHORT) == ESP_OK) {
        int rssi, ber;
        if (sscanf(strstr(resp, "+CSQ:"), "+CSQ: %d,%d", &rssi, &ber) == 2) {
            // Convert raw RSSI: dBm = -113 + 2*rssi (for values 0-31)
            out->rssi = (rssi == 99) ? 0 : (-113 + 2 * rssi);
            out->ber  = ber;
        }
    }

    // Registration status
    if (modem_at_cmd("AT+CREG?", "+CREG:", resp, sizeof(resp),
                      AT_TIMEOUT_SHORT) == ESP_OK) {
        int mode, stat;
        if (sscanf(strstr(resp, "+CREG:"), "+CREG: %d,%d", &mode, &stat) == 2) {
            out->registered = (stat == 1 || stat == 5);
        }
    }

    // Operator name
    if (modem_at_cmd("AT+COPS?", "+COPS:", resp, sizeof(resp),
                      AT_TIMEOUT_SHORT) == ESP_OK) {
        char *p = strstr(resp, "\"");
        if (p) {
            p++;
            char *end = strchr(p, '"');
            if (end) {
                size_t len = end - p;
                if (len >= sizeof(out->operator)) len = sizeof(out->operator) - 1;
                strncpy(out->operator, p, len);
                out->operator[len] = '\0';
            }
        }
    }

    return ESP_OK;
}
