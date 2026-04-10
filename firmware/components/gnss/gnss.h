/**
 * gnss.h — GNSS location data interface
 *
 * Retrieves GPS/GLONASS position data from the SIM7600 via AT+CGPSINFO.
 *
 * Response format:
 *   +CGPSINFO: <lat>,<N/S>,<lon>,<E/W>,<date>,<UTC time>,<alt>,<speed>,<course>
 *   Example:
 *   +CGPSINFO: 4729.948650,N,01902.450420,E,100426,120000.0,150.7,0.3,270.5
 *
 * Coordinates are in DDDMM.MMMMMM format, converted to decimal degrees.
 */

#pragma once

#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    double  latitude;       // Decimal degrees, positive = North
    double  longitude;      // Decimal degrees, positive = East
    float   altitude;       // Meters above sea level
    float   speed;          // Speed over ground, km/h
    float   course;         // Course over ground, degrees (0–360)
    uint8_t satellites;     // Number of satellites in use
    float   hdop;           // Horizontal dilution of precision
    bool    valid;          // Fix is valid
    char    utc_time[16];   // UTC time string "HHMMSS.S"
    char    date[8];        // Date string "DDMMYY"
} gnss_data_t;

/**
 * Initialize GNSS subsystem (starts modem GNSS via AT+CGPS).
 * @return ESP_OK on success
 */
esp_err_t gnss_init(void);

/**
 * Poll current GNSS position from modem via AT+CGPSINFO.
 * @param out  Pointer to gnss_data_t to fill
 * @return ESP_OK if valid fix obtained, ESP_ERR_NOT_FOUND if no fix yet
 */
esp_err_t gnss_get_location(gnss_data_t *out);

/**
 * Parse a raw AT+CGPSINFO response string into a gnss_data_t.
 * Exposed for unit testing.
 *
 * @param response  Raw modem response string
 * @param out       Output struct
 * @return ESP_OK on successful parse, ESP_FAIL if format invalid or no fix
 */
esp_err_t gnss_parse_cgpsinfo(const char *response, gnss_data_t *out);

/**
 * Convert DDDMM.MMMMMM coordinate to decimal degrees.
 * Exposed for unit testing.
 *
 * @param raw   Value in DDDMM.MMMMMM format
 * @return Decimal degrees
 */
double gnss_nmea_to_decimal(double raw);
