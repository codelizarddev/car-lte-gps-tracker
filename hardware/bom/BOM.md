# Bill of Materials

Complete component list for the Car LTE GPS Tracker.

## Main Board

| Component | Part | Quantity | Notes | Approx. Cost (EUR) |
|---|---|---|---|---|
| Main board | LILYGO T-SIM7600E-H R2 (16MB) | 1 | ESP32-WROVER + SIM7600E + GNSS + 18650 slot. **"E" = European LTE bands (B1/B3/B7/B8/B20). "R2" = newer revision, 150Mbps DL. Choose 16MB Flash variant.** | €35–43 |
| Battery | 18650 Li-ion, 2000–3500 mAh | 1 | e.g. Samsung 25R, LG HG2 | €5–12 |
| SIM card | IoT/M2M data SIM | 1 | e.g. One.hu smartwatch plan | ~€4/month |

## Power Supply

| Component | Part | Quantity | Notes | Approx. Cost (EUR) |
|---|---|---|---|---|
| DC-DC converter | LM2596 module (≥3A) | 1 | Set output to 5V | €1.00–2.00 |
| TVS diode | SMBJ58A | 1 | 12V line transient protection | €0.25–0.50 |
| Fuse | Blade fuse 2A + holder | 1 | Inline, on 12V input | €0.50–1.00 |
| Capacitor | 1000 µF 16V electrolytic, low ESR | 1 | Output buffer | €0.25–0.50 |
| Capacitor | 100 nF ceramic | 2 | Decoupling | €0.10 |

## Relay Circuit

| Component | Part | Quantity | Notes | Approx. Cost (EUR) |
|---|---|---|---|---|
| Relay | 12V automotive SPDT relay, 30A contacts | 1 | e.g. Bosch 332019150 | €1.50–3.00 |
| Relay socket | Matching 5-pin socket | 1 | Optional but recommended | €0.50–1.00 |
| Transistor | 2N2222 or BC337 | 1 | NPN, relay coil driver | €0.10 |
| Resistor | 1 kΩ, 1/4W | 1 | Base resistor | €0.05 |
| Diode | 1N4007 | 1 | Flyback / freewheeling diode | €0.05 |

## Battery Voltage Monitor

| Component | Part | Quantity | Notes | Approx. Cost (EUR) |
|---|---|---|---|---|
| Resistor | 100 kΩ, 1/4W | 1 | Voltage divider upper leg | €0.05 |
| Resistor | 220 kΩ, 1/4W | 1 | Voltage divider lower leg | €0.05 |

> **Voltage divider calculation:**
> Battery max = 4.2V, ESP32 ADC max = 3.3V
> Ratio: R2/(R1+R2) = 3.3/4.2 ≈ 0.786
> Using 100kΩ (R1) + 220kΩ (R2): ratio = 220/320 = 0.6875 → 4.2 × 0.6875 = 2.89V ✓

## Wiring & Connectors

| Component | Part | Quantity | Notes | Approx. Cost (EUR) |
|---|---|---|---|---|
| Antenna | LTE external antenna, SMA | 1 | Adhesive mount for interior | €2.50–5.00 |
| Antenna | GNSS external antenna, SMA | 1 | Place near windshield | €2.50–5.00 |
| Wire | 0.5mm² automotive wire, red/black | ~2m | 12V power lines | €0.75 |
| Wire | 0.25mm² automotive wire | ~1m | Signal lines | €0.50 |
| Connectors | JST or Dupont 2.54mm | assorted | For signal connections | €0.75 |
| Enclosure | ABS project box, ~100×60×30mm | 1 | Weatherproof if possible | €1.50–3.00 |

## Total Estimated Cost

| Category | Cost (EUR) |
|---|---|
| Main board + battery | €40–55 |
| Power supply components | €2.00–4.00 |
| Relay circuit | €2.00–4.00 |
| Antennas + wiring | €7–14 |
| **Total** | **~€51–77** |

---

## Where to Buy

### Main board
**LILYGO T-SIM7600E-H R2 — AliExpress (LILYGO Official Store)**
👉 https://www.aliexpress.com/item/1005001705250713.html

Select options: **SIM7600E-H** + **16MB Flash** → ~€35–43, free shipping, 2–4 weeks.

> ⚠️ Double-check the variant selector on the listing — make sure it says **SIM7600E-H** (not G, NA, or SA) and **16MB**.

### Other components
- **Electronic components** (resistors, caps, diode, transistor): [hestore.hu](https://hestore.hu), [tme.eu](https://tme.eu)
- **18650 battery**: local vape/battery shop, or AliExpress
- **Relay + socket**: automotive parts store, or [hestore.hu](https://hestore.hu)
- **Antennas**: AliExpress (search "SMA LTE antenna adhesive" and "SMA GPS antenna active")
