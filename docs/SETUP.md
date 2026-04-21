# Setup Guide

## Hardware Required

- Wemos D1 Mini (ESP8266)
- DHT11 temperature/humidity sensor (3-pin module)
- GY-30 / BH1750 light intensity sensor (I2C)
- Capacitive soil moisture sensor (analog)
- RGB LED (common cathode) + 3x 220 ohm resistors
- Breadboard + jumper wires
- Micro-USB cable

## Software Installation

### 1. Install Rosetta 2 (Apple Silicon Macs only)

```bash
softwareupdate --install-rosetta --agree-to-license
```

### 2. Install arduino-cli

```bash
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=./bin sh
export PATH="./bin:$PATH"
```

### 3. Install ESP8266 board support

```bash
arduino-cli config init
arduino-cli config add board_manager.additional_urls \
  https://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli core update-index
arduino-cli core install esp8266:esp8266
```

### 4. Install libraries

```bash
arduino-cli lib install "DHT sensor library" "Adafruit Unified Sensor" "BH1750"
```

### 5. Configure WiFi and ThingSpeak

```bash
cp dashboard/config.example.h dashboard/config.h
```

Edit `config.h` with your WiFi credentials and ThingSpeak Write API key.

### 6. Compile and upload

```bash
arduino-cli compile --fqbn esp8266:esp8266:d1_mini dashboard
arduino-cli upload --fqbn esp8266:esp8266:d1_mini --port /dev/cu.usbserial-XXXXXXXX dashboard
```

### 7. Find the dashboard

Open serial monitor at 115200 baud to see the IP address, then open it in a browser.

## ThingSpeak Setup

1. Create a free account at https://thingspeak.mathworks.com
2. Create a new channel with 4 fields: Temperature, Humidity, Light, Soil Moisture
3. Copy the **Write API Key** into `dashboard/config.h`
4. Copy the **Channel ID** and **Read API Key** for the GitHub Pages dashboard
5. Open https://paris-connor.github.io/AgTech/ and enter the Channel ID + Read API Key

## Soil Moisture Calibration

The soil sensor outputs an analog voltage that maps to moisture percentage. Default calibration:

- `SOIL_DRY = 1023` (raw value in air = 0% moisture)
- `SOIL_WET = 300` (raw value in water = 100% moisture)

To calibrate for your soil:
1. Read the raw A0 value in dry air (check serial output)
2. Read the raw A0 value submerged in water
3. Update `SOIL_DRY` and `SOIL_WET` in `dashboard.ino`

## Troubleshooting

| Problem | Solution |
|---------|----------|
| `bad CPU type in executable` | Install Rosetta 2 |
| Board not detected | Try a different USB cable |
| `Failed to read from DHT11` | Check wiring: DATA to D5, VCC to 3.3V, GND |
| WiFi won't connect | Verify SSID/password. D1 only supports 2.4GHz |
| GY-30 not found | Check solder joints, verify SDA=D2 SCL=D1 |
| Soil reads N/A | Sensor not connected or A0 reads 1023 (open circuit) |
| ThingSpeak fails | Check API key, ensure WiFi connected, 15s minimum interval |
| Serial shows garbage | Ensure baud rate is 115200 |
| Blue LED always on | Don't use D1 (GPIO5) for LED - has built-in pull-up |
