/**
 * test_gnss.c — Unit tests for GNSS parser (host-compatible, no ESP-IDF needed)
 *
 * Tests:
 *   - gnss_nmea_to_decimal: coordinate format conversion
 *   - gnss_parse_cgpsinfo:  full AT response parser
 *
 * Build & run:
 *   gcc -o test_gnss test_gnss.c ../components/gnss/gnss_pure.c \
 *       -lm -I../components/gnss
 */

#include <stdio.h>
#include <math.h>
#include <string.h>

#include "../components/gnss/gnss_pure.h"

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
    double result = gnss_nmea_to_decimal(4729.948650);
    ASSERT_NEAR(result, 47.499144, 0.000001);
    PASS();
}

TEST(test_nmea_to_decimal_east)
{
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
    double result = gnss_nmea_to_decimal(30.0);
    ASSERT_NEAR(result, 0.5, 0.000001);
    PASS();
}

TEST(test_nmea_to_decimal_full_degrees)
{
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
    const char *resp =
        "\r\n+CGPSINFO: 3436.123456,S,05812.654321,W,100426,150000.0,10.0,0.0,0.0\r\n\r\nOK\r\n";

    gnss_data_t loc;
    esp_err_t ret = gnss_parse_cgpsinfo(resp, &loc);

    ASSERT_EQ_INT(ret, ESP_OK);
    ASSERT_TRUE(loc.valid);
    ASSERT_TRUE(loc.latitude  < 0.0);
    ASSERT_TRUE(loc.longitude < 0.0);
    PASS();
}

TEST(test_parse_no_fix_empty)
{
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

    ASSERT_TRUE(strcmp(loc.date, "100426") == 0);
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
