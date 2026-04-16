# AgTech - Smart Plant Monitoring System

Environmental monitoring system using a Wemos D1 Mini (ESP8266) with DHT11 sensor, RGB status LED, and a live web dashboard.

## Features

- **Live web dashboard** with temperature and humidity graphs
- **RGB LED status indicator** — green (ok), yellow (warning), red (danger)
- **Configurable thresholds** for temperature and humidity
- **Event log** with color-coded alerts
- **Ready for expansion** — soil moisture probe, light sensor (GY-30)

## Hardware

| Component | Pin | Details |
|-----------|-----|---------|
| DHT11 (temp/humidity) | D5 (GPIO14) | Data pin, VCC to 3.3V |
| RGB LED - Red | D6 (GPIO12) | 220 ohm resistor |
| RGB LED - Green | D7 (GPIO13) | 220 ohm resistor |
| RGB LED - Blue | D1 (GPIO5) | 220 ohm resistor |
| RGB LED - GND | GND | Common cathode (long leg) |

## Thresholds

| Sensor | OK (Green) | Warning (Yellow) | Danger (Red) |
|--------|-----------|-----------------|-------------|
| Temperature | 10-30 C | 5-10 / 30-35 C | <5 / >35 C |
| Humidity | 30-70% | 20-30 / 70-80% | <20 / >80% |

## Quick Start

```bash
# Install arduino-cli
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=./bin sh
export PATH="./bin:$PATH"

# Install ESP8266 board + libraries
arduino-cli config init
arduino-cli config add board_manager.additional_urls https://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli core update-index
arduino-cli core install esp8266:esp8266
arduino-cli lib install "DHT sensor library" "Adafruit Unified Sensor"

# Configure WiFi
cp dashboard/config.example.h dashboard/config.h
# Edit config.h with your WiFi SSID and password

# Compile and upload
arduino-cli compile --fqbn esp8266:esp8266:d1_mini dashboard
arduino-cli upload --fqbn esp8266:esp8266:d1_mini --port /dev/cu.usbserial-XXXXXXXX dashboard
```

Open the IP shown in serial output (115200 baud) in your browser.

## Project Structure

```
AgTech/
├── dashboard/          # Main plant monitor (DHT11 + RGB LED + web UI)
│   ├── dashboard.ino
│   ├── config.h        # WiFi credentials (git-ignored)
│   └── config.example.h
├── led_blink/          # LED blink test
├── dht11_test/         # DHT11 serial test
├── rgb_test/           # RGB LED color test
├── sensor_scan/        # Auto-detect connected sensors
├── docs/
│   ├── SETUP.md
│   ├── WIRING.md
│   └── API.md
└── README.md
```

## Next Steps

- [ ] Add soil moisture sensor (analog, A0)
- [ ] Add GY-30 light intensity sensor (I2C, D1/D2)
- [ ] Data logging to SD card or cloud
- [ ] Connect to real plant for field testing

## Apple Silicon Note

```bash
softwareupdate --install-rosetta --agree-to-license
```
