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

#include "gnss_pure.h"

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
