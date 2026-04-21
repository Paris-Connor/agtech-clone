# Wiring Reference

## D1 Mini Pin Map

```
          ┌──────────┐
          │ USB PORT │
     RST -│          │- TX  (GPIO1)
      A0 -│          │- RX  (GPIO3)
 D0  GP16-│          │- D1  (GPIO5)  SCL
 D5  GP14-│  ESP     │- D2  (GPIO4)  SDA
 D6  GP12-│  8266    │- D3  (GPIO0)  FLASH
 D7  GP13-│          │- D4  (GPIO2)  LED_BUILTIN
 D8  GP15-│          │- GND
     3V3 -│          │- 5V
          └──────────┘
```

## Current Pin Assignments

| Pin | GPIO | Device | Notes |
|-----|------|--------|-------|
| D1 | 5 | GY-30 SCL | I2C clock |
| D2 | 4 | GY-30 SDA | I2C data |
| D5 | 14 | DHT11 DATA | Temp/humidity sensor |
| D6 | 12 | RGB LED Red | 220 ohm resistor |
| D7 | 13 | RGB LED Green | 220 ohm resistor |
| D8 | 15 | RGB LED Blue | 220 ohm resistor, pull-down at boot |
| A0 | ADC | Soil Moisture | Analog 0-1V (D1 Mini scales to 3.3V) |

## Wiring Diagram

### DHT11 (Temperature & Humidity)

```
DHT11 Module (3-pin)       D1 Mini
┌─────────────────┐
│  S    VCC   GND │
└──┼─────┼─────┼──┘
   │     │     │
   │     │     └──── GND (- rail)
   │     └────────── 3V3 (+ rail)
   └──────────────── D5 (GPIO14)
```

### GY-30 / BH1750 (Light Intensity)

```
GY-30                      D1 Mini
┌─────────────────────┐
│ VCC GND SCL SDA ADDR│
└──┼───┼───┼───┼───┼──┘
   │   │   │   │   │
   │   │   │   │   └── (not connected)
   │   │   │   └────── D2 (GPIO4)
   │   │   └────────── D1 (GPIO5)
   │   └────────────── GND (- rail)
   └────────────────── 3V3 (+ rail)
```

Note: ADDR pin left unconnected = I2C address 0x23 (default).

### Soil Moisture Sensor

```
Soil Probe                 D1 Mini
┌─────────────┐
│ VCC GND SIG │
└──┼───┼───┼──┘
   │   │   │
   │   │   └──────── A0
   │   └──────────── GND (- rail)
   └──────────────── 3V3 (+ rail)
```

### RGB LED (Common Cathode)

```
RGB LED                    D1 Mini
                           (via 220 ohm resistors)

Red leg   ──[220Ω]──────── D6 (GPIO12)
GND (long leg) ──────────── GND (- rail)
Green leg ──[220Ω]──────── D7 (GPIO13)
Blue leg  ──[220Ω]──────── D8 (GPIO15)
```

## Breadboard Power Rail

```
D1 Mini 3V3 ────── + rail (red line)
D1 Mini GND ────── - rail (blue line)

All sensor VCC pins → + rail
All sensor GND pins → - rail
```

## Pins to Avoid

- **D3 (GPIO0)** - FLASH button, pulled HIGH at boot
- **D4 (GPIO2)** - Must be HIGH at boot, connected to built-in LED
- **D0 (GPIO16)** - No PWM/I2C support, used for deep sleep wake
- **D1 (GPIO5)** - Has built-in pull-up (I2C), do not use for LEDs
