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

## Current Wiring: DHT11

```
DHT11 Module (3-pin)       D1 Mini
┌─────────────────┐
│  S    VCC   GND │
│  │     │     │  │
└──┼─────┼─────┼──┘
   │     │     │
   │     │     └──────── GND
   │     └────────────── 3V3
   └──────────────────── D5 (GPIO14)
```

### Pin Choice Rationale

| Pin | GPIO | Used For | Notes |
|-----|------|----------|-------|
| D4 | GPIO2 | Built-in LED | Avoid - used by LED, also boot mode pin |
| **D5** | **GPIO14** | **DHT11 DATA** | **Clean GPIO, no boot conflicts** |
| D1 | GPIO5 | Reserved | I2C SCL (future use) |
| D2 | GPIO4 | Reserved | I2C SDA (future use) |

### Pins to Avoid

- **D3 (GPIO0)** - FLASH button, pulled HIGH at boot. Can cause boot failure.
- **D4 (GPIO2)** - Must be HIGH at boot. Connected to built-in LED.
- **D8 (GPIO15)** - Must be LOW at boot. Has external pull-down.
- **D0 (GPIO16)** - Used for deep sleep wake. No PWM/I2C support.

## Raw 4-Pin DHT11

If using the bare sensor (4 pins) instead of a module:

```
DHT11 (4-pin)              D1 Mini
┌────────────────┐
│ 1   2   3   4 │
│VCC DATA  NC GND│
└─┬───┬───────┬──┘
  │   │       │
  │   ├─[10k]-┤
  │   │       │
  │   │       └──── GND
  │   └──────────── D5 (GPIO14)
  └──────────────── 3V3
```

Pin 3 is not connected. The 10k ohm resistor is a pull-up between DATA and VCC.
