# AgTech - Smart Plant Monitoring System

Real-time environmental monitoring for plants using a Wemos D1 Mini (ESP8266) with multiple sensors, RGB status LED, and a live web dashboard.

**Live Dashboard:** [paris-connor.github.io/AgTech](https://paris-connor.github.io/AgTech/)

## Features

- **4 sensors** — temperature, humidity, light intensity, soil moisture
- **RGB LED status indicator** — green (ok), yellow (warning), red (danger)
- **Live web dashboard** served from the D1 Mini with real-time charts
- **Cloud dashboard** via ThingSpeak — view from anywhere
- **GitHub Pages dashboard** with demo mode, local connect, and cloud modes
- **Auto-detection** — sensors that aren't connected are gracefully skipped
- **Configurable thresholds** for all sensors

## Hardware

| Component | Pin | Details |
|-----------|-----|---------|
| DHT11 (temp/humidity) | D5 (GPIO14) | 3.3V, data pin |
| GY-30 / BH1750 (light) | D1 (SCL), D2 (SDA) | I2C, 3.3V |
| Soil Moisture Sensor | A0 | Analog, 3.3V |
| RGB LED - Red | D6 (GPIO12) | 220 ohm resistor |
| RGB LED - Green | D7 (GPIO13) | 220 ohm resistor |
| RGB LED - Blue | D8 (GPIO15) | 220 ohm resistor |
| RGB LED - GND | GND | Common cathode (long leg) |

## Thresholds

| Sensor | OK (Green) | Warning (Yellow) | Danger (Red) |
|--------|-----------|-----------------|-------------|
| Temperature | 10-30 C | 5-10 / 30-35 C | <5 / >35 C |
| Humidity | 30-70% | 20-30 / 70-80% | <20 / >80% |
| Light | 500-50k lux | <500 / >50k | <200 / >80k |
| Soil Moisture | 25-80% | <25 / >80% | <15 / >90% |

## Quick Start

```bash
# Install tools
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=./bin sh
export PATH="./bin:$PATH"

# Install board + libraries
arduino-cli config init
arduino-cli config add board_manager.additional_urls https://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli core update-index && arduino-cli core install esp8266:esp8266
arduino-cli lib install "DHT sensor library" "Adafruit Unified Sensor" "BH1750"

# Configure
cp dashboard/config.example.h dashboard/config.h
# Edit config.h with WiFi + ThingSpeak credentials

# Flash
arduino-cli compile --fqbn esp8266:esp8266:d1_mini dashboard
arduino-cli upload --fqbn esp8266:esp8266:d1_mini --port /dev/cu.usbserial-XXXXXXXX dashboard
```

## Project Structure

```
AgTech/
├── dashboard/            # Main firmware (all sensors + web UI + ThingSpeak)
│   ├── dashboard.ino
│   ├── config.h          # WiFi + API keys (git-ignored)
│   └── config.example.h
├── index.html            # GitHub Pages dashboard (demo/local/cloud modes)
├── led_blink/            # LED blink test
├── dht11_test/           # DHT11 serial test
├── rgb_test/             # RGB LED color test
├── sensor_scan/          # Auto-detect connected sensors
├── i2c_scan/             # I2C bus scanner
├── docs/
│   ├── SETUP.md          # Full setup guide
│   ├── WIRING.md         # Pin diagrams
│   └── API.md            # JSON API + ThingSpeak reference
└── README.md
```

## Dashboard Modes

The GitHub Pages dashboard supports three data sources:

| Mode | How | When |
|------|-----|------|
| **Demo** | Simulated data | Default, always works |
| **Local** | Enter D1 Mini IP | Same WiFi network |
| **Cloud** | Enter ThingSpeak Channel ID + Read Key | Anywhere with internet |

## Apple Silicon Note

```bash
softwareupdate --install-rosetta --agree-to-license
```
