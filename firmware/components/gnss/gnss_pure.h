#pragma once

#include <stdbool.h>
#include <stdint.h>

#if defined(__has_include)
#if __has_include("esp_err.h")
#include "esp_err.h"
#else
typedef int esp_err_t;
#ifndef ESP_OK
#define ESP_OK 0
#endif
#ifndef ESP_FAIL
#define ESP_FAIL -1
#endif
#ifndef ESP_ERR_INVALID_ARG
#define ESP_ERR_INVALID_ARG -2
#endif
#ifndef ESP_ERR_NOT_FOUND
#define ESP_ERR_NOT_FOUND -3
#endif
#endif
#else
#include "esp_err.h"
#endif

typedef struct {
    double  latitude;       // Decimal degrees, positive = North
    double  longitude;      // Decimal degrees, positive = East
    float   altitude;       // Meters above sea level
    float   speed;          // Speed over ground, km/h
    float   course;         // Course over ground, degrees (0-360)
    uint8_t satellites;     // Number of satellites in use
    float   hdop;           // Horizontal dilution of precision
    bool    valid;          // Fix is valid
    char    utc_time[16];   // UTC time string "HHMMSS.S"
    char    date[8];        // Date string "DDMMYY"
} gnss_data_t;

esp_err_t gnss_parse_cgpsinfo(const char *response, gnss_data_t *out);
double gnss_nmea_to_decimal(double raw);
