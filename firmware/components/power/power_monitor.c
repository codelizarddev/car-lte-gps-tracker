/**
 * power_monitor.c — Battery voltage monitoring via ESP32 ADC
 *
 * LILYGO T-SIM7600 battery voltage is available on GPIO35.
 * The board has an onboard voltage divider; actual ratio may vary
 * depending on board revision. Check board schematic.
 *
 * Default divider: R1=100kΩ, R2=220kΩ → ratio = 220/320 = 0.6875
 */

#include "power_monitor.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "POWER";

// ADC handle
static adc_oneshot_unit_handle_t adc_handle = NULL;
static adc_cali_handle_t         adc_cali   = NULL;
static bool                      cali_ok    = false;

// GPIO35 = ADC1 channel 7
#define BATTERY_ADC_CHANNEL  ADC_CHANNEL_7
#define BATTERY_ADC_UNIT     ADC_UNIT_1
#define SAMPLE_COUNT         8   // Average N readings to reduce noise

// ─── Internal ─────────────────────────────────────────────────────────────────

/**
 * Convert raw ADC voltage (at the divider midpoint) to battery voltage.
 * Accounts for the voltage divider ratio.
 *
 * @param adc_mv  Measured voltage at ADC pin in mV
 * @return        Battery voltage in mV
 */
static uint32_t adc_mv_to_battery_mv(uint32_t adc_mv)
{
    // Vbat = Vadc / (R2 / (R1 + R2))
    // With R1=100k, R2=220k: Vbat = Vadc / 0.6875 = Vadc * 320 / 220
    return (uint32_t)((float)adc_mv / VDIV_RATIO);
}

/**
 * Convert battery voltage (mV) to approximate percentage.
 * Uses a simple linear map between EMPTY and FULL thresholds.
 *
 * @param mv  Battery voltage in mV
 * @return    Percentage 0–100
 */
static uint8_t battery_mv_to_pct(uint32_t mv)
{
    if (mv >= BATTERY_FULL_MV)  return 100;
    if (mv <= BATTERY_EMPTY_MV) return 0;

    uint32_t range = BATTERY_FULL_MV - BATTERY_EMPTY_MV;
    uint32_t above = mv - BATTERY_EMPTY_MV;
    return (uint8_t)((above * 100U) / range);
}

// ─── Public API ───────────────────────────────────────────────────────────────

esp_err_t power_monitor_init(void)
{
    // Init ADC unit
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = BATTERY_ADC_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &adc_handle));

    // Configure channel
    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten    = ADC_ATTEN_DB_11,    // Full-scale ~3.3V
        .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle,
                                               BATTERY_ADC_CHANNEL,
                                               &chan_cfg));

    // Try to enable calibration (curve fitting or line fitting)
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_curve_fitting_config_t cali_cfg = {
        .unit_id  = BATTERY_ADC_UNIT,
        .chan     = BATTERY_ADC_CHANNEL,
        .atten   = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_12,
    };
    cali_ok = (adc_cali_create_scheme_curve_fitting(&cali_cfg, &adc_cali) == ESP_OK);
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    adc_cali_line_fitting_config_t cali_cfg = {
        .unit_id  = BATTERY_ADC_UNIT,
        .atten   = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_12,
    };
    cali_ok = (adc_cali_create_scheme_line_fitting(&cali_cfg, &adc_cali) == ESP_OK);
#endif

    ESP_LOGI(TAG, "ADC initialized, calibration: %s", cali_ok ? "yes" : "no");
    return ESP_OK;
}

void power_monitor_read(power_data_t *out)
{
    if (!out) return;
    memset(out, 0, sizeof(*out));

    // Average N samples
    int32_t raw_sum = 0;
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        int raw = 0;
        adc_oneshot_read(adc_handle, BATTERY_ADC_CHANNEL, &raw);
        raw_sum += raw;
    }
    int raw_avg = raw_sum / SAMPLE_COUNT;

    // Convert to mV at ADC pin
    uint32_t adc_mv = 0;
    if (cali_ok) {
        int mv = 0;
        adc_cali_raw_to_voltage(adc_cali, raw_avg, &mv);
        adc_mv = (uint32_t)mv;
    } else {
        // Fallback: linear approximation (ADC_VREF / ADC_MAX_RAW)
        adc_mv = (uint32_t)((float)raw_avg * ADC_VREF_MV / ADC_MAX_RAW);
    }

    // Scale back through voltage divider
    out->battery_mv  = adc_mv_to_battery_mv(adc_mv);
    out->battery_pct = battery_mv_to_pct(out->battery_mv);

    // Power source: if battery is near full and voltage is stable, assume car
    // A more reliable method would be to detect the 5V input separately.
    // For now, use a simple heuristic: >4.1V = likely on charger (car power)
    out->source = (out->battery_mv > 4100) ? POWER_SOURCE_CAR : POWER_SOURCE_BATTERY;

    ESP_LOGD(TAG, "Battery: %lu mV (%u%%) source=%s",
             out->battery_mv, out->battery_pct,
             (out->source == POWER_SOURCE_CAR) ? "car" : "battery");
}
