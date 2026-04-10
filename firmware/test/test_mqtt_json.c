/**
 * test_mqtt_json.c — Unit tests for MQTT JSON payload generation (host-compatible)
 *
 * Tests that JSON payloads are well-formed and contain correct values.
 *
 * Build & run:
 *   gcc -o test_mqtt_json test_mqtt_json.c -lm
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

// ── Minimal type definitions ──────────────────────────────────────────────────

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

typedef enum { POWER_SOURCE_CAR = 0, POWER_SOURCE_BATTERY = 1 } power_source_t;
typedef struct {
    uint32_t       battery_mv;
    uint8_t        battery_pct;
    power_source_t source;
} power_data_t;

// ── Pure JSON builders (mirrors mqtt_client_wrapper.c logic) ──────────────────

static int build_location_json(const gnss_data_t *loc, char *buf, size_t len)
{
    return snprintf(buf, len,
        "{\"lat\":%.6f,\"lon\":%.6f,\"alt\":%.1f,"
        "\"speed\":%.1f,\"course\":%.1f,"
        "\"satellites\":%u,\"hdop\":%.1f,"
        "\"date\":\"%s\",\"time\":\"%s\"}",
        loc->latitude, loc->longitude, loc->altitude,
        loc->speed, loc->course,
        loc->satellites, loc->hdop,
        loc->date, loc->utc_time);
}

static int build_power_json(const power_data_t *pwr, char *buf, size_t len)
{
    return snprintf(buf, len,
        "{\"source\":\"%s\",\"battery_mv\":%lu,\"battery_pct\":%u}",
        (pwr->source == POWER_SOURCE_CAR) ? "car" : "battery",
        (unsigned long)pwr->battery_mv,
        pwr->battery_pct);
}

static int build_state_json(const char *state, char *buf, size_t len)
{
    return snprintf(buf, len, "{\"status\":\"%s\"}", state);
}

// ── Test framework ────────────────────────────────────────────────────────────
static int tests_run = 0, tests_passed = 0, tests_failed = 0;
#define RUN(fn) do { tests_run++; fn(); } while(0)

#define ASSERT_HAS(buf, substr) do { \
    if (!strstr((buf), (substr))) { \
        printf("  FAIL %s:%d  expected \"%s\" in: %s\n", __FILE__, __LINE__, (substr), (buf)); \
        tests_failed++; return; \
    } \
} while(0)

#define ASSERT_TRUE(c) do { \
    if (!(c)) { printf("  FAIL %s:%d  %s\n", __FILE__, __LINE__, #c); tests_failed++; return; } \
} while(0)

#define PASS() do { tests_passed++; printf("  PASS %s\n", __func__); } while(0)

// ── Location JSON tests ───────────────────────────────────────────────────────

static void test_location_json_has_required_fields(void)
{
    gnss_data_t loc = {
        .latitude   = 47.499144,
        .longitude  = 19.040840,
        .altitude   = 150.7f,
        .speed      = 0.3f,
        .course     = 270.5f,
        .satellites = 8,
        .hdop       = 1.2f,
        .valid      = true,
    };
    strcpy(loc.date,     "100426");
    strcpy(loc.utc_time, "120000.0");

    char buf[256];
    build_location_json(&loc, buf, sizeof(buf));

    ASSERT_HAS(buf, "\"lat\":");
    ASSERT_HAS(buf, "\"lon\":");
    ASSERT_HAS(buf, "\"alt\":");
    ASSERT_HAS(buf, "\"speed\":");
    ASSERT_HAS(buf, "\"course\":");
    ASSERT_HAS(buf, "\"satellites\":");
    ASSERT_HAS(buf, "\"hdop\":");
    ASSERT_HAS(buf, "\"date\":");
    ASSERT_HAS(buf, "\"time\":");
    PASS();
}

static void test_location_json_correct_values(void)
{
    gnss_data_t loc = {
        .latitude  = 47.499144,
        .longitude = 19.040840,
        .altitude  = 150.7f,
        .speed     = 0.3f,
        .course    = 270.5f,
        .satellites= 8,
        .hdop      = 1.2f,
        .valid     = true,
    };
    strcpy(loc.date,     "100426");
    strcpy(loc.utc_time, "120000.0");

    char buf[256];
    build_location_json(&loc, buf, sizeof(buf));

    ASSERT_HAS(buf, "47.499144");
    ASSERT_HAS(buf, "19.040840");
    ASSERT_HAS(buf, "150.7");
    ASSERT_HAS(buf, "\"100426\"");
    ASSERT_HAS(buf, "\"120000.0\"");
    PASS();
}

static void test_location_json_starts_and_ends_with_braces(void)
{
    gnss_data_t loc = {0};
    strcpy(loc.date, "000000");
    strcpy(loc.utc_time, "000000.0");
    char buf[256];
    build_location_json(&loc, buf, sizeof(buf));
    ASSERT_TRUE(buf[0] == '{');
    ASSERT_TRUE(buf[strlen(buf) - 1] == '}');
    PASS();
}

// ── Power JSON tests ──────────────────────────────────────────────────────────

static void test_power_json_car_source(void)
{
    power_data_t pwr = { .battery_mv = 4150, .battery_pct = 90, .source = POWER_SOURCE_CAR };
    char buf[128];
    build_power_json(&pwr, buf, sizeof(buf));
    ASSERT_HAS(buf, "\"source\":\"car\"");
    ASSERT_HAS(buf, "\"battery_mv\":4150");
    ASSERT_HAS(buf, "\"battery_pct\":90");
    PASS();
}

static void test_power_json_battery_source(void)
{
    power_data_t pwr = { .battery_mv = 3700, .battery_pct = 55, .source = POWER_SOURCE_BATTERY };
    char buf[128];
    build_power_json(&pwr, buf, sizeof(buf));
    ASSERT_HAS(buf, "\"source\":\"battery\"");
    ASSERT_HAS(buf, "\"battery_mv\":3700");
    PASS();
}

static void test_power_json_zero_battery(void)
{
    power_data_t pwr = { .battery_mv = 3000, .battery_pct = 0, .source = POWER_SOURCE_BATTERY };
    char buf[128];
    build_power_json(&pwr, buf, sizeof(buf));
    ASSERT_HAS(buf, "\"battery_pct\":0");
    PASS();
}

// ── State JSON tests ──────────────────────────────────────────────────────────

static void test_state_json_online(void)
{
    char buf[64];
    build_state_json("online", buf, sizeof(buf));
    ASSERT_HAS(buf, "\"status\":\"online\"");
    PASS();
}

static void test_state_json_offline(void)
{
    char buf[64];
    build_state_json("offline", buf, sizeof(buf));
    ASSERT_HAS(buf, "\"status\":\"offline\"");
    PASS();
}

// ── Main ──────────────────────────────────────────────────────────────────────

int main(void)
{
    printf("\n=== MQTT JSON Payload Tests ===\n\n");

    printf("-- Location JSON --\n");
    RUN(test_location_json_has_required_fields);
    RUN(test_location_json_correct_values);
    RUN(test_location_json_starts_and_ends_with_braces);

    printf("\n-- Power JSON --\n");
    RUN(test_power_json_car_source);
    RUN(test_power_json_battery_source);
    RUN(test_power_json_zero_battery);

    printf("\n-- State JSON --\n");
    RUN(test_state_json_online);
    RUN(test_state_json_offline);

    printf("\n=== Results: %d/%d passed", tests_passed, tests_run);
    if (tests_failed > 0) printf(", %d FAILED", tests_failed);
    printf(" ===\n\n");

    return tests_failed > 0 ? 1 : 0;
}
