# Home Assistant Setup Guide

## Prerequisites

- Home Assistant (any recent version)
- Mosquitto MQTT broker add-on installed in HA, or HiveMQ Cloud account
- MQTT integration enabled in HA

---

## Step 1: Configure MQTT integration

In HA go to **Settings → Devices & Services → Add Integration → MQTT**.

| Field | Value |
|---|---|
| Broker | `your-cluster.hivemq.cloud` |
| Port | `8883` |
| Username | `tracker` |
| Password | your HiveMQ password |
| TLS | ✅ Enabled |

Or in `configuration.yaml`:

```yaml
mqtt:
  broker: your-cluster.hivemq.cloud
  port: 8883
  username: tracker
  password: !secret mqtt_password
  tls: true
  certificate: auto
```

Add to `secrets.yaml`:
```yaml
mqtt_password: your-strong-password
```

---

## Step 2: Add entities

Copy the contents of `homeassistant/configuration.yaml` into your HA `configuration.yaml`
and restart HA.
Replace `my-car` in the sample topics with the same `CONFIG_VEHICLE_ID` value you use in the firmware.

This creates:
- `device_tracker.car_tracker` — GPS location
- `sensor.tracker_battery_percent` — battery %
- `sensor.tracker_battery_voltage` — battery mV
- `sensor.tracker_power_source` — `car` or `battery`
- `sensor.tracker_speed` — speed km/h
- `sensor.tracker_gps_satellites` — satellite count
- `sensor.tracker_status` — `online` / `offline`
- `switch.tracker_fuel_pump` — relay control

---

## Step 3: Add automations

Copy the contents of `homeassistant/automations/automations.yaml` into your
HA `automations.yaml`, then update `YOUR_NOTIFY_SERVICE` with your notification
service name (e.g. `notify.mobile_app_your_phone`).

Included automations:

| Automation | Trigger | Action |
|---|---|---|
| Tracker Offline Alert | No heartbeat for 5 min | Push notification |
| Tracker Back Online | `status` → `online` | Push notification |
| Low Battery | Battery < 20% on battery power | Push notification |
| Critical Battery | Battery < 5% on battery power | Push notification |
| Relay State Changed | Relay toggled | Push notification + logbook |
| Car Power Restored | `battery` → `car` power source | Push notification |

---

## Step 4: Add Lovelace dashboard card

1. Open your dashboard in **Edit** mode
2. Add a new card → **Manual**
3. Paste the contents of `homeassistant/lovelace/dashboard.yaml`
4. Save

The dashboard includes:
- Live map with 2-hour GPS track
- Status glance row (speed, satellites, power source)
- Battery level + voltage
- Fuel pump relay toggle with **confirmation dialog** (prevents accidental OFF)
- 24-hour battery history graph

---

## Step 5: Verify

Once the tracker is running and connected, you should see:

1. `sensor.tracker_status` → `online`
2. `device_tracker.car_tracker` → shows location on map
3. `sensor.tracker_battery_percent` → some value

To test the relay from HA Developer Tools → Services:
```yaml
service: switch.turn_off
target:
  entity_id: switch.tracker_fuel_pump
```

The tracker will respond with a confirmation on `car/my-car/cmd/relay/status`.
