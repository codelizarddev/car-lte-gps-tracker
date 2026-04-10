/**
 * gnss.c — GNSS location implementation via SIM7600 AT+CGPSINFO
 */

#include "gnss.h"
#include "modem.h"
#include "esp_log.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

static const char *TAG = "GNSS";

// ─── Coordinate conversion ────────────────────────────────────────────────────

double gnss_nmea_to_decimal(double raw)
{
    // raw format: DDDMM.MMMMMM
    // degrees = floor(raw / 100)
    // minutes = raw - degrees * 100
    // decimal = degrees + minutes / 60
    int    degrees = (int)(raw / 100.0);
    double minutes = raw - (degrees * 100.0);
    return (double)degrees + (minutes / 60.0);
}

// ─── Parser ───────────────────────────────────────────────────────────────────

esp_err_t gnss_parse_cgpsinfo(const char *response, gnss_data_t *out)
{
    if (!response || !out) return ESP_ERR_INVALID_ARG;

    memset(out, 0, sizeof(*out));
    out->valid = false;

    // Find "+CGPSINFO:" prefix
    const char *p = strstr(response, "+CGPSINFO:");
    if (!p) {
        ESP_LOGD(TAG, "No +CGPSINFO in response");
        return ESP_FAIL;
    }
    p += strlen("+CGPSINFO:");

    // Skip spaces
    while (*p == ' ') p++;

    // If response is just ",,,,,,,," → no fix
    // Check first field is not empty
    if (*p == ',' || *p == '\0' || *p == '\r' || *p == '\n') {
        ESP_LOGD(TAG, "No GNSS fix yet");
        return ESP_ERR_NOT_FOUND;
    }

    // Parse: lat,N/S,lon,E/W,date,time,alt,speed,course
    double lat_raw, lon_raw, alt, speed, course;
    char   ns, ew;
    char   date[8] = {0}, utc[16] = {0};

    // Use sscanf with format matching SIM7600 output
    int parsed = sscanf(p, "%lf,%c,%lf,%c,%7[^,],%15[^,],%lf,%lf,%lf",
                        &lat_raw, &ns,
                        &lon_raw, &ew,
                        date, utc,
                        &alt, &speed, &course);

    if (parsed < 6) {
        ESP_LOGW(TAG, "Parse failed: only %d fields (raw: %s)", parsed, p);
        return ESP_FAIL;
    }

    // Convert coordinates
    out->latitude  = gnss_nmea_to_decimal(lat_raw);
    out->longitude = gnss_nmea_to_decimal(lon_raw);

    // Apply hemisphere sign
    if (ns == 'S' || ns == 's') out->latitude  = -out->latitude;
    if (ew == 'W' || ew == 'w') out->longitude = -out->longitude;

    out->altitude = (float)alt;
    out->speed    = (float)speed;
    out->course   = (float)course;
    out->valid    = true;

    strncpy(out->date,     date, sizeof(out->date) - 1);
    strncpy(out->utc_time, utc,  sizeof(out->utc_time) - 1);

    // satellites and hdop not available in CGPSINFO; set defaults
    out->satellites = 0;
    out->hdop       = 0.0f;

    ESP_LOGD(TAG, "Fix: %.6f,%.6f alt=%.1f spd=%.1f crs=%.1f",
             out->latitude, out->longitude, out->altitude, out->speed, out->course);

    return ESP_OK;
}

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
