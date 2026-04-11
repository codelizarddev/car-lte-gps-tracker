/**
 * credentials.example.h
 *
 * Copy this file to credentials.h and fill in your values.
 * credentials.h is listed in .gitignore and will never be committed.
 *
 * cp config/credentials.example.h config/credentials.h
 */

#pragma once

// ─── Vehicle ID (used as MQTT topic prefix: car/<id>/...) ───────────────────
// Use a short, URL-safe identifier for your vehicle (no spaces)
#define CONFIG_VEHICLE_ID   "my-car"

// ─── SIM / LTE ──────────────────────────────────────────────────────────────
#define CONFIG_SIM_APN      "internet"      // One.hu: "internet"
#define CONFIG_SIM_USER     ""              // Usually empty for data SIMs
#define CONFIG_SIM_PASS     ""              // Usually empty for data SIMs

// ─── MQTT Broker (HiveMQ Cloud Free) ────────────────────────────────────────
// Sign up at https://console.hivemq.cloud — free tier, no credit card needed
// Your cluster hostname looks like: abc123.s1.eu.hivemq.cloud
#define CONFIG_MQTT_HOST    "abc123.s1.eu.hivemq.cloud"
#define CONFIG_MQTT_PORT    8883            // TLS port
#define CONFIG_MQTT_USER    "tracker"
#define CONFIG_MQTT_PASS    "your-strong-password-here"

// TLS: HiveMQ uses a publicly trusted cert — set to NULL to use ESP-IDF system roots
// For extra security, embed the ISRG Root X1 PEM from letsencrypt.org
#define CONFIG_MQTT_CA_CERT  NULL

// ─── Timing ─────────────────────────────────────────────────────────────────
#define CONFIG_GNSS_PUBLISH_INTERVAL_S   10   // Publish GPS every N seconds
#define CONFIG_STATE_PUBLISH_INTERVAL_S  60   // Publish online heartbeat every N seconds
#define CONFIG_POWER_PUBLISH_INTERVAL_S  30   // Publish battery every N seconds
