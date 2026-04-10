/**
 * test_relay.c — Unit tests for relay state logic (host-compatible)
 *
 * Tests the relay state machine logic in isolation,
 * without actual GPIO hardware.
 *
 * Build & run:
 *   gcc -o test_relay test_relay.c
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

// ── Relay types (from relay.h) ────────────────────────────────────────────────
typedef enum {
    RELAY_CLOSED = 0,
    RELAY_OPEN   = 1,
} relay_state_t;

// ── Minimal relay state machine without GPIO ──────────────────────────────────
static relay_state_t _state = RELAY_CLOSED;
static int           _gpio_level = 0;
static int           _set_count  = 0;

static void relay_init_mock(void)
{
    _state      = RELAY_CLOSED;
    _gpio_level = 0;
    _set_count  = 0;
}

static void relay_set_mock(relay_state_t s)
{
    _state      = s;
    _gpio_level = (s == RELAY_OPEN) ? 1 : 0;
    _set_count++;
}

static relay_state_t relay_get_mock(void) { return _state; }

// ── Test framework ────────────────────────────────────────────────────────────
static int tests_run = 0, tests_passed = 0, tests_failed = 0;
#define RUN(fn) do { tests_run++; fn(); } while(0)

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        printf("  FAIL %s:%d\n", __FILE__, __LINE__); \
        tests_failed++; return; \
    } \
} while(0)

#define ASSERT_TRUE(c) ASSERT_EQ(!!(c), 1)
#define PASS() do { tests_passed++; printf("  PASS %s\n", __func__); } while(0)

// ── Tests ─────────────────────────────────────────────────────────────────────

static void test_init_state_is_closed(void)
{
    relay_init_mock();
    ASSERT_EQ(relay_get_mock(), RELAY_CLOSED);
    ASSERT_EQ(_gpio_level, 0);
    PASS();
}

static void test_set_open_raises_gpio(void)
{
    relay_init_mock();
    relay_set_mock(RELAY_OPEN);
    ASSERT_EQ(relay_get_mock(), RELAY_OPEN);
    ASSERT_EQ(_gpio_level, 1);
    PASS();
}

static void test_set_closed_lowers_gpio(void)
{
    relay_init_mock();
    relay_set_mock(RELAY_OPEN);
    relay_set_mock(RELAY_CLOSED);
    ASSERT_EQ(relay_get_mock(), RELAY_CLOSED);
    ASSERT_EQ(_gpio_level, 0);
    PASS();
}

static void test_double_open_idempotent(void)
{
    relay_init_mock();
    relay_set_mock(RELAY_OPEN);
    relay_set_mock(RELAY_OPEN);
    ASSERT_EQ(relay_get_mock(), RELAY_OPEN);
    ASSERT_EQ(_gpio_level, 1);
    PASS();
}

static void test_double_closed_idempotent(void)
{
    relay_init_mock();
    relay_set_mock(RELAY_CLOSED);
    ASSERT_EQ(relay_get_mock(), RELAY_CLOSED);
    ASSERT_EQ(_gpio_level, 0);
    PASS();
}

static void test_toggle_sequence(void)
{
    relay_init_mock();

    relay_set_mock(RELAY_OPEN);
    ASSERT_EQ(relay_get_mock(), RELAY_OPEN);

    relay_set_mock(RELAY_CLOSED);
    ASSERT_EQ(relay_get_mock(), RELAY_CLOSED);

    relay_set_mock(RELAY_OPEN);
    ASSERT_EQ(relay_get_mock(), RELAY_OPEN);

    relay_set_mock(RELAY_CLOSED);
    ASSERT_EQ(relay_get_mock(), RELAY_CLOSED);

    PASS();
}

static void test_mqtt_cmd_off_maps_to_open(void)
{
    // Simulate MQTT command handling logic:
    // "OFF" → relay OPEN (fuel pump disabled)
    relay_init_mock();

    const char *cmd = "OFF";
    if (strcmp(cmd, "OFF") == 0) relay_set_mock(RELAY_OPEN);
    if (strcmp(cmd, "ON")  == 0) relay_set_mock(RELAY_CLOSED);

    ASSERT_EQ(relay_get_mock(), RELAY_OPEN);
    ASSERT_EQ(_gpio_level, 1);
    PASS();
}

static void test_mqtt_cmd_on_maps_to_closed(void)
{
    relay_init_mock();

    const char *cmd = "ON";
    if (strcmp(cmd, "OFF") == 0) relay_set_mock(RELAY_OPEN);
    if (strcmp(cmd, "ON")  == 0) relay_set_mock(RELAY_CLOSED);

    ASSERT_EQ(relay_get_mock(), RELAY_CLOSED);
    ASSERT_EQ(_gpio_level, 0);
    PASS();
}

static void test_unknown_cmd_no_change(void)
{
    relay_init_mock();
    relay_set_mock(RELAY_CLOSED);
    int before_count = _set_count;

    const char *cmd = "INVALID";
    if (strcmp(cmd, "OFF") == 0) relay_set_mock(RELAY_OPEN);
    if (strcmp(cmd, "ON")  == 0) relay_set_mock(RELAY_CLOSED);

    ASSERT_EQ(_set_count, before_count);  // relay_set not called
    ASSERT_EQ(relay_get_mock(), RELAY_CLOSED);
    PASS();
}

static void test_reset_restores_closed(void)
{
    relay_init_mock();
    relay_set_mock(RELAY_OPEN);
    // Simulate reset / power loss behavior
    relay_init_mock();
    ASSERT_EQ(relay_get_mock(), RELAY_CLOSED);
    PASS();
}

// ── Main ──────────────────────────────────────────────────────────────────────

int main(void)
{
    printf("\n=== Relay Logic Tests ===\n\n");
    RUN(test_init_state_is_closed);
    RUN(test_set_open_raises_gpio);
    RUN(test_set_closed_lowers_gpio);
    RUN(test_double_open_idempotent);
    RUN(test_double_closed_idempotent);
    RUN(test_toggle_sequence);
    RUN(test_mqtt_cmd_off_maps_to_open);
    RUN(test_mqtt_cmd_on_maps_to_closed);
    RUN(test_unknown_cmd_no_change);
    RUN(test_reset_restores_closed);

    printf("\n=== Results: %d/%d passed", tests_passed, tests_run);
    if (tests_failed > 0) printf(", %d FAILED", tests_failed);
    printf(" ===\n\n");

    return tests_failed > 0 ? 1 : 0;
}
