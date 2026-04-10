# Firmware Setup Guide

## Prerequisites

- ESP-IDF v5.2+ ([install guide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/))
- Python 3.8+
- LILYGO T-SIM7600 board
- USB cable (for flashing)

---

## Step 1: Clone the repository

```bash
git clone https://github.com/CodeLizardDev/car-lte-gps-tracker.git
cd car-lte-gps-tracker
```

---

## Step 2: Set up ESP-IDF environment

```bash
# Source the ESP-IDF environment (run this in every new terminal session)
source ~/esp/esp-idf/export.sh
```

---

## Step 3: Configure credentials

```bash
cd firmware/
cp config/credentials.example.h config/credentials.h
```

Edit `config/credentials.h`:

```c
#define CONFIG_VEHICLE_ID   "xsara"          // Your vehicle ID (used in MQTT topics)
#define CONFIG_SIM_APN      "internet"       // One.hu APN
#define CONFIG_MQTT_HOST    "abc.hivemq.cloud"
#define CONFIG_MQTT_PORT    8883
#define CONFIG_MQTT_USER    "tracker"
#define CONFIG_MQTT_PASS    "your-password"
#define CONFIG_MQTT_CA_CERT  NULL            // Use system root CAs (works with HiveMQ)
#define CONFIG_GNSS_PUBLISH_INTERVAL_S   10
#define CONFIG_POWER_PUBLISH_INTERVAL_S  30
```

> ⚠️ `credentials.h` is in `.gitignore` and will never be committed to git.

---

## Step 4: Build

```bash
idf.py set-target esp32
idf.py build
```

---

## Step 5: Flash

Connect the LILYGO T-SIM7600 via USB.

```bash
# Linux
idf.py -p /dev/ttyUSB0 flash

# macOS
idf.py -p /dev/cu.usbserial-* flash

# Windows
idf.py -p COM3 flash
```

---

## Step 6: Monitor serial output

```bash
idf.py -p /dev/ttyUSB0 monitor
# Press Ctrl+] to exit
```

Expected boot log:
```
I (xxx) MAIN: Car LTE GPS Tracker starting...
I (xxx) RELAY: Relay GPIO initialized (CLOSED)
I (xxx) POWER: ADC initialized, calibration: yes
I (xxx) MODEM: Waiting for modem...
I (xxx) MODEM: Modem responded to AT
I (xxx) MODEM: Modem initialized
I (xxx) MODEM: Network registered
I (xxx) MODEM: LTE connected: +CGPADDR: 1,10.x.x.x
I (xxx) GNSS: GNSS started
I (xxx) MQTT: Connected to broker
I (xxx) MQTT: Subscribed to car/xsara/cmd/relay
I (xxx) MAIN: System ready.
```

---

## Testing without hardware: AT Simulator

Use the included simulator to test the firmware connect flow without a real modem:

```bash
# Terminal 1 — start simulator
python3 tools/sim7600_simulator.py
# → Prints: Virtual port: /dev/pts/3

# Terminal 2 — flash firmware pointing to virtual port
idf.py -p /dev/pts/3 monitor   # or update MODEM_UART port in modem.h
```

Simulator options:
```
--no-fix          Simulate no GNSS fix
--no-lte          Simulate no network registration
--gps-lat 48.2    Custom GPS latitude
--gps-lon 16.3    Custom GPS longitude
--interactive     Manual AT command mode (stdin/stdout)
```

---

## Running unit tests (no hardware needed)

```bash
cd firmware/test/
make
```

All 42 tests should pass in under 1 second.
