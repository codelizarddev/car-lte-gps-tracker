# Wiring Guide

Complete wiring instructions for installing the Car LTE GPS Tracker in a vehicle.

> Tested on Citroën Xsara Picasso (2003). Concepts apply to most 12V vehicles.

---

## 1. Power Supply Wiring

### 12V Input

Connect to a **permanent +12V** source (not ignition-switched), so the tracker works even when the car is off.

```
Car battery / fuse box permanent +12V
        │
      [2A fuse]
        │
      [TVS SMBJ58A] ─── GND
        │
      [LM2596 IN+]
        │
      [LM2596 OUT+] → LILYGO T-SIM7600 5V input
        │
      [1000µF cap] ─── GND
      [100nF cap]  ─── GND
```

**GND**: Connect to chassis ground or directly to battery negative.

### Voltage setting

Adjust the LM2596 trimmer potentiometer until output reads exactly **5.0V** before connecting to the board.

---

## 2. Relay Circuit

### Schematic

```
ESP32 GPIO (e.g. GPIO26)
        │
      [1kΩ resistor]
        │
      [2N2222 Base (B)]
        │
   ┌────┤ Collector (C) ─── Relay coil (-) ─── GND
   │    │                         │
   │  Emitter (E) ─── GND       [1N4007 flyback diode]
   │                              │
   │                   Relay coil (+) ─── 12V
   │
   └─── [1N4007 cathode → 12V, anode → relay coil (+)]
```

### Relay connections

| Relay pin | Connection |
|---|---|
| 85 (coil –) | Transistor collector |
| 86 (coil +) | +12V |
| 30 (common) | From ECU / fuel pump relay trigger signal |
| 87 (NO) | Not used |
| 87a (NC) | To fuel pump relay coil / ECU input |

> **NC (Normally Closed)** is used. When the relay is not energized (default/power-off), the circuit is closed = fuel pump works normally.

---

## 3. Fuel Pump Relay Tap

### Principle

Do **not** interrupt the high-current fuel pump power supply line. Instead, interrupt the **control coil signal** of the vehicle's fuel pump relay.

```
ECU fuel pump trigger signal
        │
      [Our relay COM (pin 30)]
        │
      [Our relay NC (pin 87a)]
        │
      Vehicle fuel pump relay coil (pin 85 or 86)
```

### Finding the correct wire (Citroën Xsara Picasso 2003)

1. Locate the engine fuse/relay box in the engine bay
2. Find the fuel pump relay (consult the owner's manual or fuse box lid diagram)
3. Identify the coil control wires (typically low-current, 12V trigger from ECU)
4. **Do not cut the high-current supply wire** (usually 30A+)
5. Cut the ECU control wire and insert our relay in series

> ⚠️ Ask a mechanic to identify the exact wire if unsure. Using the wrong wire can damage the ECU.

---

## 4. Battery Voltage Monitor

```
18650 battery positive terminal
        │
      [100kΩ R1]
        │─────────── ESP32 ADC pin (e.g. GPIO34)
      [220kΩ R2]
        │
       GND
```

The voltage divider scales 4.2V → 2.89V, safely within the ESP32 ADC range (3.3V max).

---

## 5. Antenna Placement

### LTE Antenna
- Place inside the passenger compartment
- Avoid metal surfaces (use adhesive mount on plastic trim or dashboard)
- Minimum 20cm from other electronics

### GNSS Antenna
- Place on the dashboard, near the windshield
- Needs clear sky view — avoid placing under metallic tinting
- Use adhesive mount or suction cup

---

## 6. Enclosure & Mounting

- Mount the board in an ABS enclosure
- Avoid locations near the engine (heat, vibration)
- Good locations: under dashboard, behind glove box, in trunk near C-pillar
- Secure with double-sided tape or cable ties
- Ensure cables are not pinched by moving parts

---

## 7. Wiring Checklist

- [ ] 12V input fused (2A)
- [ ] TVS diode installed on 12V line
- [ ] LM2596 output verified at 5.0V before connecting board
- [ ] GND connected to chassis or battery negative
- [ ] Relay flyback diode installed
- [ ] Relay coil connected to correct vehicle wire
- [ ] GNSS antenna with clear sky view
- [ ] LTE antenna away from metal surfaces
- [ ] All connections crimped or soldered (no loose wires)
- [ ] Enclosure secured and cables strain-relieved
