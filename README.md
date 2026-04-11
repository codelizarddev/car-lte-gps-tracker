# Car LTE GPS Tracker with Remote Relay Control

[![CI](https://github.com/CodeLizardDev/car-lte-gps-tracker/actions/workflows/ci.yml/badge.svg?branch=master)](https://github.com/CodeLizardDev/car-lte-gps-tracker/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-blue.svg)](https://www.espressif.com/)
[![Framework: ESP-IDF](https://img.shields.io/badge/Framework-ESP--IDF-red.svg)](https://docs.espressif.com/projects/esp-idf/)

A DIY embedded system built into a car that provides **LTE-based GPS tracking** and **remote relay control** via MQTT, integrated with **Home Assistant**.

Built on the [LILYGO T-SIM7600](https://www.lilygo.cc/products/t-sim7600) development board.

> **Status:** 🚧 Work in progress — hardware design & firmware development phase

---

## Features

- 📍 **GNSS tracking** (GPS/GLONASS) with position, speed, heading, and satellite data
- 📡 **LTE connectivity** via SIM7600 modem — works anywhere with mobile coverage
- 🔌 **Remote relay control** — interrupts fuel pump relay control circuit on command
- 🏠 **Home Assistant integration** via MQTT (Mosquitto broker)
- 🔋 **Backup battery** — 18650 Li-ion, automatic power-path failover
- 🔒 **Fail-safe design** — relay defaults to CLOSED (does not block vehicle operation)
- 🔋 **Battery voltage monitoring** — published via MQTT

---

## Hardware

### Main Board

**LILYGO T-SIM7600** (ESP32-WROVER + SIM7600 LTE modem)

| Feature | Details |
|---|---|
| MCU | ESP32-WROVER |
| Modem | SIM7600 (LTE Cat-4) |
| GNSS | GPS + GLONASS |
| Battery | 18650 Li-ion slot with onboard charger |
| Power path | Automatic switchover between 12V input and battery |

### Bill of Materials

See [`hardware/bom/BOM.md`](hardware/bom/BOM.md) for the full component list.

### Wiring

See [`docs/wiring/wiring-guide.md`](docs/wiring/wiring-guide.md) for detailed wiring instructions.

---

## System Architecture

```
[Car 12V] → [DC-DC 5V] → [LILYGO T-SIM7600]
                               │
                    ┌──────────┼──────────┐
                    │          │          │
                 [SIM7600]  [ESP32]    [GNSS]
                 [LTE/MQTT]    │
                               │
                    ┌──────────┤
                    │          │
              [GPIO relay]  [ADC battery]
                    │
              [Transistor]
                    │
               [12V Relay]
                    │
         [Fuel pump relay coil]
```

---

## MQTT Topics

| Topic | Direction | Payload | Description |
|---|---|---|---|
| `car/<vehicle-id>/location` | Publish | JSON | GPS position and telemetry |
| `car/<vehicle-id>/state` | Publish | JSON | System status heartbeat |
| `car/<vehicle-id>/power` | Publish | JSON | Battery voltage, percent, power source |
| `car/<vehicle-id>/cmd/relay` | Subscribe | `ON` / `OFF` | Relay control command |
| `car/<vehicle-id>/cmd/relay/status` | Publish | `ON` / `OFF` | Relay state confirmation |

### Location payload example

```json
{
  "lat": 47.4979,
  "lon": 19.0402,
  "alt": 108.4,
  "speed": 0.0,
  "course": 0.0,
  "satellites": 0,
  "hdop": 0.0,
  "date": "100426",
  "time": "120000.0"
}
```

### Power payload example

```json
{
  "source": "car",
  "battery_mv": 3987,
  "battery_pct": 85
}
```

---

## Relay Logic

The relay operates on the **fuel pump relay coil control line** (low-current, ~200mA), not the high-current pump supply line.

| Command | Relay | Circuit | Engine |
|---|---|---|---|
| `ON` (default) | Energized | CLOSED | Normal operation |
| `OFF` | De-energized | OPEN | Fuel pump disabled |

> ⚠️ **Warning:** Issuing `OFF` while driving will stall the engine. This system has no speed check or safety interlock by design. Use responsibly.

---

## Power Supply

```
Car 12V (9–16V)
    │
  [TVS diode SMBJ58A]
    │
  [Fuse 2A]
    │
  [LM2596 DC-DC → 5V]
    │
  [1000µF low ESR + 100nF ceramic]
    │
  [LILYGO T-SIM7600 5V input]
    │
  [Onboard charger → 18650]
       ↕ (automatic power-path)
  [18650 backup battery]
```

---

## Firmware

Built with **ESP-IDF**. See [`firmware/`](firmware/) for source code.

### Module structure

```
firmware/
├── main/
│   └── main.c              # Entry point, task scheduler
└── components/
    ├── modem/              # SIM7600 AT command driver
    ├── gnss/               # NMEA parser, location data
    ├── mqtt/               # MQTT client, topic handlers
    ├── relay/              # Relay GPIO control
    └── power/              # ADC battery monitor, power source detection
```

### Build

```bash
git clone https://github.com/CodeLizardDev/car-lte-gps-tracker
cd car-lte-gps-tracker/firmware
idf.py set-target esp32
cp config/credentials.example.h config/credentials.h
# Edit credentials.h with your MQTT server, SIM APN, etc.
idf.py build
idf.py flash monitor
```

---

## Home Assistant Integration

See [`homeassistant/`](homeassistant/) for ready-to-use configuration.

Includes:
- Device tracker entity
- Relay switch entity
- Battery sensor
- Lovelace card example

---

## Setup Guide

1. [Hardware assembly & BOM](hardware/bom/BOM.md)
2. [Wiring into the vehicle](docs/wiring/wiring-guide.md)
3. [MQTT broker setup (HiveMQ Cloud, free)](docs/setup/mosquitto-tls-setup.md)
4. [Firmware configuration and flashing](docs/setup/firmware-setup.md)
5. [Home Assistant configuration](docs/setup/homeassistant-setup.md)
6. [Build, QEMU emulation, and testing](docs/setup/build-and-qemu.md)

## Development

### Running unit tests (no hardware needed)

```bash
cd firmware/test/
make
# 42 tests across 4 suites — all should pass
```

Windows (WinLibs via `winget`):

```powershell
winget install --id BrechtSanders.WinLibs.POSIX.UCRT --exact
cd firmware\test
mingw32-make
```

### AT command simulator

Test the firmware connect flow without a real modem:

```bash
python3 tools/sim7600_simulator.py --interactive
# or: python3 tools/sim7600_simulator.py  (creates a virtual serial port)
```

Simulator supports: `--no-fix`, `--no-lte`, `--gps-lat`, `--gps-lon`.

---

## Target Vehicle

Developed on a 12V passenger vehicle platform during prototyping, but the design is intended to remain generic and applicable to most 12V vehicles.

---

## License

MIT License — see [LICENSE](LICENSE) for details.

---

## Contributing

Issues and pull requests are welcome. See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.
