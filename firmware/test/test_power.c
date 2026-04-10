/**
 * test_power.c — Unit tests for power monitor calculations (host-compatible)
 *
 * Tests the pure math functions: voltage divider back-calculation,
 * battery percentage estimation, and power source heuristic.
 *
 * Build & run:
 *   gcc -o test_power test_power.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// ── Constants from power_monitor.h ───────────────────────────────────────────
#define VDIV_RATIO       0.6875f
#define ADC_VREF_MV      3300
#define ADC_MAX_RAW      4095
#define BATTERY_FULL_MV  4200
#define BATTERY_EMPTY_MV 3000

typedef enum { POWER_SOURCE_CAR = 0, POWER_SOURCE_BATTERY = 1 } power_source_t;
typedef struct {
    uint32_t       battery_mv;
    uint8_t        battery_pct;
    power_source_t source;
} power_data_t;

// ── Pure functions under test ─────────────────────────────────────────────────

static uint32_t adc_mv_to_battery_mv(uint32_t adc_mv)
{
    return (uint32_t)((float)adc_mv / VDIV_RATIO);
}

static uint8_t battery_mv_to_pct(uint32_t mv)
{
    if (mv >= BATTERY_FULL_MV)  return 100;
    if (mv <= BATTERY_EMPTY_MV) return 0;
    uint32_t range = BATTERY_FULL_MV - BATTERY_EMPTY_MV;
    uint32_t above = mv - BATTERY_EMPTY_MV;
    return (uint8_t)((above * 100U) / range);
}

static power_source_t detect_source(uint32_t battery_mv)
{
    return (battery_mv > 4100) ? POWER_SOURCE_CAR : POWER_SOURCE_BATTERY;
}

// ── Test framework ────────────────────────────────────────────────────────────
static int tests_run = 0, tests_passed = 0, tests_failed = 0;
#define RUN(fn)  do { tests_run++; fn(); } while(0)

#define ASSERT_EQ_UINT(a, b) do { \
    if ((a) != (b)) { \
        printf("  FAIL %s:%d  expected %u got %u\n", __FILE__, __LINE__, (unsigned)(b), (unsigned)(a)); \
        tests_failed++; return; \
    } \
} while(0)

#define ASSERT_NEAR_F(a, b, eps) do { \
    if (fabs((double)(a)-(double)(b)) > (eps)) { \
        printf("  FAIL %s:%d  expected ~%.1f got %.1f\n", __FILE__, __LINE__, (double)(b), (double)(a)); \
        tests_failed++; return; \
    } \
} while(0)

#define ASSERT_TRUE(cond) do { \
    if (!(cond)) { \
        printf("  FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); \
        tests_failed++; return; \
    } \
} while(0)

#define PASS() do { tests_passed++; printf("  PASS %s\n", __func__); } while(0)

// ── Voltage divider tests ─────────────────────────────────────────────────────

static void test_vdiv_full_battery(void)
{
    // Full battery 4200mV, divider → 4200 * 0.6875 = 2887mV at ADC
    uint32_t adc_mv  = (uint32_t)(4200.0f * VDIV_RATIO);  // 2887
    uint32_t bat_mv  = adc_mv_to_battery_mv(adc_mv);
    // Should recover ~4200 (within 5mV rounding)
    ASSERT_TRUE(bat_mv >= 4195 && bat_mv <= 4205);
    PASS();
}

static void test_vdiv_empty_battery(void)
{
    uint32_t adc_mv = (uint32_t)(3000.0f * VDIV_RATIO);   // 2062
    uint32_t bat_mv = adc_mv_to_battery_mv(adc_mv);
    ASSERT_TRUE(bat_mv >= 2995 && bat_mv <= 3005);
    PASS();
}

static void test_vdiv_nominal(void)
{
    // 3700mV nominal → ADC = 3700 * 0.6875 = 2543mV
    uint32_t adc_mv = (uint32_t)(3700.0f * VDIV_RATIO);
    uint32_t bat_mv = adc_mv_to_battery_mv(adc_mv);
    ASSERT_TRUE(bat_mv >= 3695 && bat_mv <= 3705);
    PASS();
}

// ── Percentage tests ──────────────────────────────────────────────────────────

static void test_pct_full(void)
{
    ASSERT_EQ_UINT(battery_mv_to_pct(4200), 100);
    PASS();
}

static void test_pct_overfull_clamps(void)
{
    ASSERT_EQ_UINT(battery_mv_to_pct(4300), 100);
    PASS();
}

static void test_pct_empty(void)
{
    ASSERT_EQ_UINT(battery_mv_to_pct(3000), 0);
    PASS();
}

static void test_pct_underempty_clamps(void)
{
    ASSERT_EQ_UINT(battery_mv_to_pct(2800), 0);
    PASS();
}

static void test_pct_midpoint(void)
{
    // Midpoint: (4200 + 3000) / 2 = 3600 mV → 50%
    uint8_t pct = battery_mv_to_pct(3600);
    ASSERT_TRUE(pct >= 48 && pct <= 52);
    PASS();
}

static void test_pct_three_quarter(void)
{
    // 75%: 3000 + 0.75 * 1200 = 3900 mV
    uint8_t pct = battery_mv_to_pct(3900);
    ASSERT_TRUE(pct >= 73 && pct <= 77);
    PASS();
}

static void test_pct_monotone(void)
{
    // Percentage must be strictly non-decreasing with voltage
    uint8_t prev = 0;
    for (uint32_t mv = 3000; mv <= 4200; mv += 50) {
        uint8_t p = battery_mv_to_pct(mv);
        ASSERT_TRUE(p >= prev);
        prev = p;
    }
    PASS();
}

// ── Power source detection tests ──────────────────────────────────────────────

static void test_source_car_when_charging(void)
{
    ASSERT_TRUE(detect_source(4150) == POWER_SOURCE_CAR);
    ASSERT_TRUE(detect_source(4200) == POWER_SOURCE_CAR);
    PASS();
}

static void test_source_battery_when_discharging(void)
{
    ASSERT_TRUE(detect_source(4100) == POWER_SOURCE_BATTERY);
    ASSERT_TRUE(detect_source(3700) == POWER_SOURCE_BATTERY);
    ASSERT_TRUE(detect_source(3000) == POWER_SOURCE_BATTERY);
    PASS();
}

// ── Main ──────────────────────────────────────────────────────────────────────

int main(void)
{
    printf("\n=== Power Monitor Tests ===\n\n");

    printf("-- Voltage divider back-calculation --\n");
    RUN(test_vdiv_full_battery);
    RUN(test_vdiv_empty_battery);
    RUN(test_vdiv_nominal);

    printf("\n-- Battery percentage --\n");
    RUN(test_pct_full);
    RUN(test_pct_overfull_clamps);
    RUN(test_pct_empty);
    RUN(test_pct_underempty_clamps);
    RUN(test_pct_midpoint);
    RUN(test_pct_three_quarter);
    RUN(test_pct_monotone);

    printf("\n-- Power source detection --\n");
    RUN(test_source_car_when_charging);
    RUN(test_source_battery_when_discharging);

    printf("\n=== Results: %d/%d passed", tests_passed, tests_run);
    if (tests_failed > 0) printf(", %d FAILED", tests_failed);
    printf(" ===\n\n");

    return tests_failed > 0 ? 1 : 0;
}
