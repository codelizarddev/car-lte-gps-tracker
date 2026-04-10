/**
 * modem.h — SIM7600 AT command driver interface
 *
 * Handles UART communication with the SIM7600 modem:
 *   - Initialization and power-on sequence
 *   - LTE network registration and data connection (PPP/PDP context)
 *   - GNSS enable/disable via AT commands
 *   - Signal quality query
 *
 * UART configuration for LILYGO T-SIM7600:
 *   TX: GPIO27, RX: GPIO26 (check board variant)
 *   Baud: 115200
 */

#pragma once

#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>

// UART port used for SIM7600 communication
#define MODEM_UART_NUM      UART_NUM_1
#define MODEM_UART_TX_PIN   27
#define MODEM_UART_RX_PIN   26
#define MODEM_UART_BAUD     115200

// Modem power pin (PWRKEY)
#define MODEM_PWRKEY_PIN    4
#define MODEM_POWER_PIN     25  // Board-level power enable

// AT command timeouts (ms)
#define AT_TIMEOUT_SHORT    2000
#define AT_TIMEOUT_MEDIUM   10000
#define AT_TIMEOUT_LONG     60000

typedef struct {
    int   rssi;         // Received signal strength (-dBm), 0 if unknown
    int   ber;          // Bit error rate
    char  operator[32]; // Network operator name
    bool  registered;   // Network registration status
} modem_status_t;

/**
 * Initialize UART and power on the SIM7600 modem.
 * Waits for modem to respond to AT commands.
 * @return ESP_OK on success
 */
esp_err_t modem_init(void);

/**
 * Connect to LTE data network using configured APN.
 * Activates PDP context and verifies IP assignment.
 * @return ESP_OK on success
 */
esp_err_t modem_connect_lte(void);

/**
 * Disconnect from LTE network and deactivate PDP context.
 */
void modem_disconnect(void);

/**
 * Enable GNSS receiver on the SIM7600.
 * @return ESP_OK on success
 */
esp_err_t modem_gnss_start(void);

/**
 * Disable GNSS receiver.
 */
void modem_gnss_stop(void);

/**
 * Query signal quality and network status.
 * @param out Pointer to modem_status_t to populate
 * @return ESP_OK on success
 */
esp_err_t modem_get_status(modem_status_t *out);

/**
 * Send a raw AT command and wait for a response containing expected_response.
 * @param cmd         AT command string (e.g. "AT+CSQ")
 * @param expected    Expected response substring (e.g. "+CSQ")
 * @param resp_buf    Buffer for full response (may be NULL)
 * @param resp_len    Size of resp_buf
 * @param timeout_ms  Timeout in milliseconds
 * @return ESP_OK if expected response found, ESP_ERR_TIMEOUT otherwise
 */
esp_err_t modem_at_cmd(const char *cmd, const char *expected,
                        char *resp_buf, size_t resp_len,
                        uint32_t timeout_ms);
