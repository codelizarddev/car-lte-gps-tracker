#include "gnss_pure.h"

#include <stdio.h>
#include <string.h>

double gnss_nmea_to_decimal(double raw)
{
    int    degrees = (int)(raw / 100.0);
    double minutes = raw - (degrees * 100.0);
    return (double)degrees + (minutes / 60.0);
}

esp_err_t gnss_parse_cgpsinfo(const char *response, gnss_data_t *out)
{
    if (!response || !out) return ESP_ERR_INVALID_ARG;

    memset(out, 0, sizeof(*out));
    out->valid = false;

    const char *p = strstr(response, "+CGPSINFO:");
    if (!p) return ESP_FAIL;
    p += strlen("+CGPSINFO:");

    while (*p == ' ') p++;

    if (*p == ',' || *p == '\0' || *p == '\r' || *p == '\n') {
        return ESP_ERR_NOT_FOUND;
    }

    double lat_raw, lon_raw, alt = 0.0, speed = 0.0, course = 0.0;
    char   ns, ew;
    char   date[8] = {0}, utc[16] = {0};

    int parsed = sscanf(p, "%lf,%c,%lf,%c,%7[^,],%15[^,],%lf,%lf,%lf",
                        &lat_raw, &ns,
                        &lon_raw, &ew,
                        date, utc,
                        &alt, &speed, &course);

    if (parsed < 6) return ESP_FAIL;

    out->latitude  = gnss_nmea_to_decimal(lat_raw);
    out->longitude = gnss_nmea_to_decimal(lon_raw);

    if (ns == 'S' || ns == 's') out->latitude  = -out->latitude;
    if (ew == 'W' || ew == 'w') out->longitude = -out->longitude;

    out->altitude = (float)alt;
    out->speed    = (float)speed;
    out->course   = (float)course;
    out->valid    = true;

    strncpy(out->date, date, sizeof(out->date) - 1);
    strncpy(out->utc_time, utc, sizeof(out->utc_time) - 1);

    out->satellites = 0;
    out->hdop       = 0.0f;

    return ESP_OK;
}
