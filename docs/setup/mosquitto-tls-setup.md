# MQTT Broker Setup

This project uses a **free cloud MQTT broker (HiveMQ Cloud)** so no VPS, no port forwarding,
and no home router changes are needed. The ESP32 (LTE) and Home Assistant both connect
to the same cloud broker over TLS.

```
[ESP32 / LTE]  ──────►  [HiveMQ Cloud]  ◄──────  [Home Assistant]
                          (free, TLS)
```

---

## Step 1: Create a HiveMQ Cloud account

1. Go to [console.hivemq.cloud](https://console.hivemq.cloud)
2. Sign up for a free account
3. Create a new **Serverless cluster** (Free tier)
4. You will receive a hostname like: `abc123.s1.eu.hivemq.cloud`

Free tier limits (more than enough for this project):
- 100 simultaneous connections
- TLS included (port 8883)
- No credit card required

---

## Step 2: Create MQTT credentials

In the HiveMQ Cloud dashboard:

1. Go to **Access Management → Credentials**
2. Click **Add new credentials**
3. Username: `tracker`
4. Password: choose a strong password
5. Save

---

## Step 3: Get the CA certificate

HiveMQ Cloud uses a publicly trusted certificate (Let's Encrypt or similar).
You do **not** need to download a custom CA cert — you can use the system root CAs
that are bundled with ESP-IDF.

In `credentials.h`, set:
```c
#define CONFIG_MQTT_CA_CERT  NULL   // Use built-in root CAs
```

Or for extra security, download the ISRG Root X1 cert from
[letsencrypt.org/certificates](https://letsencrypt.org/certificates/) and embed it.

---

## Step 4: Configure firmware

Copy `firmware/config/credentials.example.h` to `firmware/config/credentials.h` and fill in:

```c
#define CONFIG_SIM_APN      "internet"       // One.hu

#define CONFIG_MQTT_HOST    "abc123.s1.eu.hivemq.cloud"   // your cluster hostname
#define CONFIG_MQTT_PORT    8883
#define CONFIG_MQTT_USER    "tracker"
#define CONFIG_MQTT_PASS    "your-strong-password"
#define CONFIG_MQTT_CA_CERT  NULL            // Use system root CAs
```

---

## Step 5: Configure Home Assistant

In Home Assistant, go to **Settings → Devices & Services → MQTT** and add:

| Field | Value |
|---|---|
| Broker | `abc123.s1.eu.hivemq.cloud` |
| Port | `8883` |
| Username | `tracker` |
| Password | your password |
| TLS | ✅ Enabled |
| Certificate validation | ✅ Enabled (default) |

Or via `configuration.yaml`:

```yaml
mqtt:
  broker: abc123.s1.eu.hivemq.cloud
  port: 8883
  username: tracker
  password: your-strong-password
  tls: true
  certificate: auto
```

---

## Step 6: Test the connection

Install [MQTT Explorer](https://mqtt-explorer.com/) on your PC and connect
to the HiveMQ cluster with the same credentials. You should see topics
appear under `car/my-car/` once the tracker is running.

```
Host:     abc123.s1.eu.hivemq.cloud
Port:     8883
TLS:      ✅
Username: tracker
Password: ••••••••
```

---

## Topic overview

| Topic | Direction | Description |
|---|---|---|
| `car/my-car/location` | Tracker → HA | GPS position JSON |
| `car/my-car/state` | Tracker → HA | Online heartbeat status |
| `car/my-car/power` | Tracker → HA | Battery voltage & source |
| `car/my-car/cmd/relay` | HA → Tracker | `ON` or `OFF` command |
| `car/my-car/cmd/relay/status` | Tracker → HA | Relay state confirmation |
