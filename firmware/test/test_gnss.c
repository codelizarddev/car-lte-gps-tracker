/**
 * test_gnss.c — Unit tests for GNSS parser (host-compatible, no ESP-IDF needed)
 *
 * Tests:
 *   - gnss_nmea_to_decimal: coordinate format conversion
 *   - gnss_parse_cgpsinfo:  full AT response parser
 *
 * Build & run:
 *   gcc -o test_gnss test_gnss.c test_runner.c ../components/gnss/gnss_pure.c \
 *       -lm -I../components/gnss -I../components/modem -I.
 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

// ── Minimal stubs so gnss logic compiles without ESP-IDF ──────────────────────
typedef int esp_err_t;
#ifndef ESP_OK
#define ESP_OK           0
#endif
#ifndef ESP_FAIL
#define ESP_FAIL        -1
#endif
#ifndef ESP_ERR_INVALID_ARG
#define ESP_ERR_INVALID_ARG  -2
#endif
#ifndef ESP_ERR_NOT_FOUND
#define ESP_ERR_NOT_FOUND    -3
#endif
#define ESP_LOGD(tag, fmt, ...)  /* no-op */
#define ESP_LOGW(tag, fmt, ...)  /* no-op */
#define ESP_LOGI(tag, fmt, ...)  /* no-op */
#define ESP_LOGE(tag, fmt, ...)  /* no-op */

// Minimal gnss_data_t (duplicated here to avoid ESP-IDF include chain)
typedef struct {
    double  latitude;
    double  longitude;
    float   altitude;
    float   speed;
    float   course;
    uint8_t satellites;
    float   hdop;
    bool    valid;
    char    utc_time[16];
    char    date[8];
} gnss_data_t;

// ── Include pure-logic functions from gnss source ─────────────────────────────
// We compile the pure functions by providing a "gnss_pure.c" that contains
// only gnss_nmea_to_decimal() and gnss_parse_cgpsinfo().
// Alternatively, we re-implement them here for test isolation.

double gnss_nmea_to_decimal(double raw)
{
    int    degrees = (int)(raw / 100.0);
    double minutes = raw - (degrees * 100.0);
    return (double)degrees + (minutes / 60.0);
}

esp_err_t gnss_parse_cgpsinfo(const char *response, gnss_data_t *out);

// ── Simple test framework ─────────────────────────────────────────────────────
static int tests_run    = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name)  static void name(void)
#define RUN(name)   do { tests_run++; name(); } while(0)

#define ASSERT_EQ_INT(a, b) do { \
    if ((a) != (b)) { \
        printf("  FAIL %s:%d  expected %d got %d\n", __FILE__, __LINE__, (int)(b), (int)(a)); \
        tests_failed++; return; \
    } \
} while(0)

#define ASSERT_NEAR(a, b, eps) do { \
    if (fabs((a)-(b)) > (eps)) { \
        printf("  FAIL %s:%d  expected ~%.6f got %.6f (diff %.6f)\n", \
               __FILE__, __LINE__, (double)(b), (double)(a), fabs((a)-(b))); \
        tests_failed++; return; \
    } \
} while(0)

#define ASSERT_TRUE(cond) do { \
    if (!(cond)) { \
        printf("  FAIL %s:%d  condition false: %s\n", __FILE__, __LINE__, #cond); \
        tests_failed++; return; \
    } \
} while(0)

#define ASSERT_FALSE(cond) ASSERT_TRUE(!(cond))
#define PASS() do { tests_passed++; printf("  PASS %s\n", __func__); } while(0)

// ── Tests: coordinate conversion ─────────────────────────────────────────────

TEST(test_nmea_to_decimal_north)
{
    // Budapest latitude: 4729.948650 → 47 + 29.948650/60 = 47.499144...
    double result = gnss_nmea_to_decimal(4729.948650);
    ASSERT_NEAR(result, 47.499144, 0.000001);
    PASS();
}

TEST(test_nmea_to_decimal_east)
{
    // Budapest longitude: 01902.450420 → 19 + 02.450420/60 = 19.040840...
    double result = gnss_nmea_to_decimal(1902.450420);
    ASSERT_NEAR(result, 19.040840, 0.000001);
    PASS();
}

TEST(test_nmea_to_decimal_zero)
{
    double result = gnss_nmea_to_decimal(0.0);
    ASSERT_NEAR(result, 0.0, 0.000001);
    PASS();
}

TEST(test_nmea_to_decimal_equator)
{
    // 0030.000000 = 0 degrees, 30 minutes = 0.5 decimal
    double result = gnss_nmea_to_decimal(30.0);
    ASSERT_NEAR(result, 0.5, 0.000001);
    PASS();
}

TEST(test_nmea_to_decimal_full_degrees)
{
    // 9000.000000 = exactly 90 degrees
    double result = gnss_nmea_to_decimal(9000.0);
    ASSERT_NEAR(result, 90.0, 0.000001);
    PASS();
}

// ── Tests: AT+CGPSINFO parser ─────────────────────────────────────────────────

TEST(test_parse_valid_fix_budapest)
{
    const char *resp =
        "\r\n+CGPSINFO: 4729.948650,N,01902.450420,E,100426,120000.0,150.7,0.3,270.5\r\n\r\nOK\r\n";

    gnss_data_t loc;
    esp_err_t ret = gnss_parse_cgpsinfo(resp, &loc);

    ASSERT_EQ_INT(ret, ESP_OK);
    ASSERT_TRUE(loc.valid);
    ASSERT_NEAR(loc.latitude,  47.499144, 0.000010);
    ASSERT_NEAR(loc.longitude, 19.040840, 0.000010);
    ASSERT_NEAR(loc.altitude,  150.7,     0.1);
    ASSERT_NEAR(loc.speed,     0.3,       0.01);
    ASSERT_NEAR(loc.course,    270.5,     0.1);
    PASS();
}

TEST(test_parse_south_west_coordinates)
{
    // Southern hemisphere, western hemisphere (e.g. Argentina)
    const char *resp =
        "\r\n+CGPSINFO: 3436.123456,S,05812.654321,W,100426,150000.0,10.0,0.0,0.0\r\n\r\nOK\r\n";

    gnss_data_t loc;
    esp_err_t ret = gnss_parse_cgpsinfo(resp, &loc);

    ASSERT_EQ_INT(ret, ESP_OK);
    ASSERT_TRUE(loc.valid);
    ASSERT_TRUE(loc.latitude  < 0.0);   // South → negative
    ASSERT_TRUE(loc.longitude < 0.0);   // West → negative
    PASS();
}

TEST(test_parse_no_fix_empty)
{
    // SIM7600 returns commas only when no fix
    const char *resp = "\r\n+CGPSINFO: ,,,,,,,,\r\n\r\nOK\r\n";

    gnss_data_t loc;
    esp_err_t ret = gnss_parse_cgpsinfo(resp, &loc);

    ASSERT_EQ_INT(ret, ESP_ERR_NOT_FOUND);
    ASSERT_FALSE(loc.valid);
    PASS();
}

TEST(test_parse_no_cgpsinfo_prefix)
{
    const char *resp = "\r\nERROR\r\n";
    gnss_data_t loc;
    esp_err_t ret = gnss_parse_cgpsinfo(resp, &loc);
    ASSERT_EQ_INT(ret, ESP_FAIL);
    ASSERT_FALSE(loc.valid);
    PASS();
}

TEST(test_parse_null_response)
{
    gnss_data_t loc;
    esp_err_t ret = gnss_parse_cgpsinfo(NULL, &loc);
    ASSERT_EQ_INT(ret, ESP_ERR_INVALID_ARG);
    PASS();
}

TEST(test_parse_null_output)
{
    esp_err_t ret = gnss_parse_cgpsinfo("+CGPSINFO: ...", NULL);
    ASSERT_EQ_INT(ret, ESP_ERR_INVALID_ARG);
    PASS();
}

TEST(test_parse_date_time_fields)
{
    const char *resp =
        "\r\n+CGPSINFO: 4729.948650,N,01902.450420,E,100426,133742.5,155.0,1.2,90.0\r\n\r\nOK\r\n";

    gnss_data_t loc;
    gnss_parse_cgpsinfo(resp, &loc);

    ASSERT_TRUE(strcmp(loc.date,     "100426")   == 0);
    ASSERT_TRUE(strcmp(loc.utc_time, "133742.5") == 0);
    PASS();
}

// ── Main ──────────────────────────────────────────────────────────────────────

int main(void)
{
    printf("\n=== GNSS Parser Tests ===\n\n");

    printf("-- Coordinate conversion --\n");
    RUN(test_nmea_to_decimal_north);
    RUN(test_nmea_to_decimal_east);
    RUN(test_nmea_to_decimal_zero);
    RUN(test_nmea_to_decimal_equator);
    RUN(test_nmea_to_decimal_full_degrees);

    printf("\n-- AT+CGPSINFO parser --\n");
    RUN(test_parse_valid_fix_budapest);
    RUN(test_parse_south_west_coordinates);
    RUN(test_parse_no_fix_empty);
    RUN(test_parse_no_cgpsinfo_prefix);
    RUN(test_parse_null_response);
    RUN(test_parse_null_output);
    RUN(test_parse_date_time_fields);

    printf("\n=== Results: %d/%d passed", tests_passed, tests_run);
    if (tests_failed > 0)
        printf(", %d FAILED", tests_failed);
    printf(" ===\n\n");

    return tests_failed > 0 ? 1 : 0;
}

// ── gnss_parse_cgpsinfo implementation (duplicated for host build) ─────────────
#include <stdlib.h>

esp_err_t gnss_parse_cgpsinfo(const char *response, gnss_data_t *out)
{
    if (!response || !out) return ESP_ERR_INVALID_ARG;

    memset(out, 0, sizeof(*out));
    out->valid = false;

    const char *p = strstr(response, "+CGPSINFO:");
    if (!p) return ESP_FAIL;
    p += strlen("+CGPSINFO:");

    while (*p == ' ') p++;

    if (*p == ',' || *p == '\0' || *p == '\r' || *p == '\n')
        return ESP_ERR_NOT_FOUND;

    double lat_raw, lon_raw, alt, speed, course;
    char   ns, ew;
    char   date[8] = {0}, utc[16] = {0};

    int parsed = sscanf(p, "%lf,%c,%lf,%c,%7[^,],%15[^,],%lf,%lf,%lf",
                        &lat_raw, &ns, &lon_raw, &ew,
                        date, utc, &alt, &speed, &course);

    if (parsed < 6) return ESP_FAIL;

    out->latitude  = gnss_nmea_to_decimal(lat_raw);
    out->longitude = gnss_nmea_to_decimal(lon_raw);
    if (ns == 'S' || ns == 's') out->latitude  = -out->latitude;
    if (ew == 'W' || ew == 'w') out->longitude = -out->longitude;
    out->altitude  = (float)alt;
    out->speed     = (float)speed;
    out->course    = (float)course;
    out->valid     = true;
    strncpy(out->date,     date, sizeof(out->date) - 1);
    strncpy(out->utc_time, utc,  sizeof(out->utc_time) - 1);

    return ESP_OK;
}
