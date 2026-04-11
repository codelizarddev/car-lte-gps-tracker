/**
 * gnss.c — GNSS location implementation via SIM7600 AT+CGPSINFO
 */

#include "gnss.h"
#include "modem.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "GNSS";

// ─── Public API ───────────────────────────────────────────────────────────────

esp_err_t gnss_init(void)
{
    esp_err_t ret = modem_gnss_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start GNSS on modem");
        return ret;
    }
    ESP_LOGI(TAG, "GNSS initialized");
    return ESP_OK;
}

esp_err_t gnss_get_location(gnss_data_t *out)
{
    if (!out) return ESP_ERR_INVALID_ARG;

    char resp[256] = {0};
    esp_err_t ret = modem_at_cmd("AT+CGPSINFO", "+CGPSINFO:",
                                  resp, sizeof(resp),
                                  AT_TIMEOUT_MEDIUM);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "AT+CGPSINFO failed");
        return ret;
    }

    return gnss_parse_cgpsinfo(resp, out);
}
