# Setup Guide

## Prerequisites

- macOS (tested on Apple Silicon with Rosetta 2)
- USB cable (micro-USB for D1 Mini)
- Wemos D1 Mini board
- DHT11 sensor (3-pin module or 4-pin raw sensor)

## Step-by-Step Installation

### 1. Install Rosetta 2 (Apple Silicon only)

```bash
softwareupdate --install-rosetta --agree-to-license
```

The ESP8266 compiler toolchain is x86-only and requires Rosetta.

### 2. Install arduino-cli

```bash
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=./bin sh
export PATH="./bin:$PATH"
```

### 3. Configure board manager

```bash
arduino-cli config init
arduino-cli config add board_manager.additional_urls \
  https://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli core update-index
arduino-cli core install esp8266:esp8266
```

### 4. Install required libraries

```bash
arduino-cli lib install "DHT sensor library" "Adafruit Unified Sensor"
```

### 5. Verify board connection

Plug in the D1 Mini via USB, then:

```bash
arduino-cli board list
```

You should see a serial port like `/dev/cu.usbserial-XXXXXXXX`.

## Sketch Progression

The project contains three sketches, each building on the last:

### led_blink (Step 1)

Basic connectivity test. Blinks the built-in LED (GPIO2) at 500ms intervals. Upload this first to verify the board and USB connection work.

```bash
arduino-cli compile --fqbn esp8266:esp8266:d1_mini led_blink
arduino-cli upload --fqbn esp8266:esp8266:d1_mini --port /dev/cu.usbserial-XXXXXXXX led_blink
```

### dht11_test (Step 2)

Sensor validation. Reads DHT11 and prints temperature (C/F) and humidity to serial at 115200 baud. Confirms the sensor wiring is correct.

```bash
arduino-cli compile --fqbn esp8266:esp8266:d1_mini dht11_test
arduino-cli upload --fqbn esp8266:esp8266:d1_mini --port /dev/cu.usbserial-XXXXXXXX dht11_test
```

Monitor output:

```bash
arduino-cli monitor --port /dev/cu.usbserial-XXXXXXXX --config baudrate=115200
```

### dashboard (Step 3)

Full web dashboard with WiFi. Configure WiFi first:

```bash
cp dashboard/config.example.h dashboard/config.h
# Edit config.h with your WiFi credentials
```

```bash
arduino-cli compile --fqbn esp8266:esp8266:d1_mini dashboard
arduino-cli upload --fqbn esp8266:esp8266:d1_mini --port /dev/cu.usbserial-XXXXXXXX dashboard
```

After upload, check serial output for the assigned IP address, then open it in a browser.

## Troubleshooting

| Problem | Solution |
|---------|----------|
| `bad CPU type in executable` | Install Rosetta 2 (see step 1) |
| Board not detected | Try a different USB cable (some are charge-only) |
| `Failed to read from DHT11` | Check wiring: DATA to D5, VCC to 3.3V, GND to GND |
| WiFi won't connect | Verify SSID/password in `config.h`. D1 only supports 2.4GHz |
| Serial shows garbage | Ensure baud rate is set to 115200 |
