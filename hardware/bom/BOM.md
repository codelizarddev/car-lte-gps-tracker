# Bill of Materials

Complete component list for the Car LTE GPS Tracker.

## Main Board

| Component | Part | Quantity | Notes | Approx. Cost (HUF) |
|---|---|---|---|---|
| Main board | LILYGO T-SIM7600 | 1 | ESP32-WROVER + SIM7600 + GNSS + 18650 slot | 20,000–30,000 |
| Battery | 18650 Li-ion, 2000–3500 mAh | 1 | e.g. Samsung 25R, LG HG2 | 2,000–5,000 |
| SIM card | IoT/M2M data SIM | 1 | e.g. One.hu smartwatch plan | ~1,500/month |

## Power Supply

| Component | Part | Quantity | Notes | Approx. Cost (HUF) |
|---|---|---|---|---|
| DC-DC converter | LM2596 module (≥3A) | 1 | Set output to 5V | 500–1,000 |
| TVS diode | SMBJ58A | 1 | 12V line transient protection | 100–200 |
| Fuse | Blade fuse 2A + holder | 1 | Inline, on 12V input | 200–500 |
| Capacitor | 1000 µF 16V electrolytic, low ESR | 1 | Output buffer | 100–200 |
| Capacitor | 100 nF ceramic | 2 | Decoupling | 50 |

## Relay Circuit

| Component | Part | Quantity | Notes | Approx. Cost (HUF) |
|---|---|---|---|---|
| Relay | 12V automotive SPDT relay, 30A contacts | 1 | e.g. Bosch 332019150 | 500–1,000 |
| Relay socket | Matching 5-pin socket | 1 | Optional but recommended | 200–500 |
| Transistor | 2N2222 or BC337 | 1 | NPN, relay coil driver | 50 |
| Resistor | 1 kΩ, 1/4W | 1 | Base resistor | 20 |
| Diode | 1N4007 | 1 | Flyback / freewheeling diode | 30 |

## Battery Voltage Monitor

| Component | Part | Quantity | Notes | Approx. Cost (HUF) |
|---|---|---|---|---|
| Resistor | 100 kΩ, 1/4W | 1 | Voltage divider upper leg | 20 |
| Resistor | 220 kΩ, 1/4W | 1 | Voltage divider lower leg | 20 |

> **Voltage divider calculation:**
> Battery max = 4.2V, ESP32 ADC max = 3.3V
> Ratio: R2/(R1+R2) = 3.3/4.2 ≈ 0.786
> Using 100kΩ (R1) + 220kΩ (R2): ratio = 220/320 = 0.6875 → 4.2 × 0.6875 = 2.89V ✓

## Wiring & Connectors

| Component | Part | Quantity | Notes | Approx. Cost (HUF) |
|---|---|---|---|---|
| Antenna | LTE external antenna, SMA | 1 | Adhesive mount for interior | 1,000–2,000 |
| Antenna | GNSS external antenna, SMA | 1 | Place near windshield | 1,000–2,000 |
| Wire | 0.5mm² automotive wire, red/black | ~2m | 12V power lines | 300 |
| Wire | 0.25mm² automotive wire | ~1m | Signal lines | 200 |
| Connectors | JST or Dupont 2.54mm | assorted | For signal connections | 300 |
| Enclosure | ABS project box, ~100×60×30mm | 1 | Weatherproof if possible | 500–1,000 |

## Total Estimated Cost

| Category | Cost (HUF) |
|---|---|
| Main board + battery | 22,000–35,000 |
| Power supply components | 1,000–2,000 |
| Relay circuit | 800–1,600 |
| Antennas + wiring | 3,000–5,000 |
| **Total** | **~27,000–44,000** |

---

## Where to Buy (Hungary)

- **LILYGO T-SIM7600**: AliExpress, Aliexpedite, or Chinese electronics shops
- **Electronic components**: [hestore.hu](https://hestore.hu), [tme.eu](https://tme.eu), [farnell.com](https://farnell.com)
- **18650 battery**: hazai vape/akkumulátor bolt, or AliExpress
- **Relay**: automotive parts store (autósbolt), or hestore.hu
