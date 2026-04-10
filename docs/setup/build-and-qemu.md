# Build and QEMU Emulation Guide

## Prerequisites

Install ESP-IDF v5.x following the [official guide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/).

Quick install (Linux/macOS):
```bash
mkdir -p ~/esp && cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf && git checkout v5.2
./install.sh esp32
source ./export.sh
```

---

## Step 1: Configure credentials

```bash
cd firmware/
cp config/credentials.example.h config/credentials.h
# Edit credentials.h with your HiveMQ host, MQTT credentials, APN, and vehicle ID
```

---

## Step 2: Build firmware

```bash
cd firmware/
idf.py set-target esp32
idf.py build
```

Expected output: `Project build complete. To flash, run this command: ...`

---

## Step 3: Flash to device

Connect the LILYGO T-SIM7600 via USB, then:

```bash
idf.py -p /dev/ttyUSB0 flash monitor
# On Windows: idf.py -p COM3 flash monitor
```

Press `Ctrl+]` to exit the monitor.

---

## Step 4: QEMU emulation (logic test without hardware)

> ⚠️ QEMU can run the firmware logic, but hardware peripherals (UART/modem, ADC, GPIO relay)
> will not function. Use QEMU to verify boot sequence, NVS, and task startup.

### Install espressif QEMU

```bash
# macOS (via brew)
brew install espressif/tap/qemu-esp32

# Linux — download pre-built binary
pip install esptool
wget https://github.com/espressif/qemu/releases/latest/download/qemu-esp32-linux-amd64.tar.bz2
tar -xjf qemu-esp32-linux-amd64.tar.bz2
export PATH=$PATH:$(pwd)/qemu-esp32/bin
```

### Run in QEMU

```bash
cd firmware/

# Build with QEMU target
idf.py set-target esp32
idf.py build

# Run in QEMU (IDF v5.x built-in support)
idf.py qemu monitor
```

Expected boot output:
```
I (xxx) MAIN: Car LTE GPS Tracker starting...
I (xxx) RELAY: Relay GPIO initialized (CLOSED)
I (xxx) POWER: ADC initialized, calibration: no    ← normal in QEMU
I (xxx) MODEM: Waiting for modem...
W (xxx) MODEM: TIMEOUT — cmd: AT                    ← expected, no real modem
```

The timeout on the modem AT command is expected in QEMU — it confirms the UART
and task logic work correctly, and that the fail-safe relay state (CLOSED) is set
before any modem operation.

---

## Step 5: Run host unit tests

No ESP-IDF needed. Requires only `gcc` and `make`.

```bash
cd firmware/test/
make
```

Expected output:
```
=== Results: 12/12 passed ===   (gnss)
=== Results: 12/12 passed ===   (power)
=== Results: 10/10 passed ===   (relay)
=== Results:  8/ 8 passed ===   (mqtt_json)
Test suites: 4 total, 4 passed, 0 failed
```
