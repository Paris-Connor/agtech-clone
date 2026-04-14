// Sensor Scanner for Wemos D1 Mini (ESP8266)
// Scans I2C bus, analog pin, and digital pins to detect connected sensors

#include <Wire.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(1000);

  Serial.println("\n========================================");
  Serial.println("  D1 Mini (ESP8266) - Sensor Scanner");
  Serial.println("========================================\n");

  // --- I2C Scan ---
  scanI2C();

  // --- Analog Pin Scan ---
  scanAnalog();

  // --- Digital Pin Scan ---
  scanDigital();

  // --- OneWire Scan (DS18B20 etc) on common pins ---
  scanOneWire();

  Serial.println("\n========================================");
  Serial.println("  Scan Complete");
  Serial.println("========================================");
}

void scanI2C() {
  Serial.println("--- I2C Bus Scan (SDA=D2/GPIO4, SCL=D1/GPIO5) ---");
  Wire.begin();
  int found = 0;

  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    byte error = Wire.endTransmission();

    if (error == 0) {
      found++;
      Serial.print("  Found device at 0x");
      if (addr < 16) Serial.print("0");
      Serial.print(addr, HEX);
      Serial.print("  -> ");

      // Identify known devices
      switch (addr) {
        case 0x20: case 0x21: case 0x22: case 0x23:
        case 0x24: case 0x25: case 0x26: case 0x27:
          Serial.println("PCF8574 I/O Expander (or LCD backpack)"); break;
        case 0x38: Serial.println("AHT10/AHT20 Temp+Humidity"); break;
        case 0x39: Serial.println("TSL2561 Light Sensor / AHT10"); break;
        case 0x3C: case 0x3D:
          Serial.println("SSD1306 OLED Display"); break;
        case 0x40: Serial.println("INA219 Current Sensor / HDC1080 / SHT30"); break;
        case 0x44: case 0x45:
          Serial.println("SHT30/SHT31 Temp+Humidity"); break;
        case 0x48: Serial.println("ADS1115 ADC / TMP102 Temp / PCF8591"); break;
        case 0x49: case 0x4A: case 0x4B:
          Serial.println("ADS1115 ADC / TMP102 Temp"); break;
        case 0x50: case 0x51: case 0x52: case 0x53:
          Serial.println("AT24C32 EEPROM (often on RTC module)"); break;
        case 0x57: Serial.println("AT24C32 EEPROM / MAX30102 Pulse Oximeter"); break;
        case 0x5A: Serial.println("MLX90614 IR Temp / CCS811 Air Quality"); break;
        case 0x5B: Serial.println("CCS811 Air Quality"); break;
        case 0x60: Serial.println("MPL3115A2 Barometer / SI1145 UV"); break;
        case 0x68: Serial.println("DS3231 RTC / MPU6050 IMU / DS1307 RTC"); break;
        case 0x69: Serial.println("MPU6050 IMU (alt addr)"); break;
        case 0x76: Serial.println("BME280/BMP280 Temp+Pressure+Humidity"); break;
        case 0x77: Serial.println("BME280/BMP280 (alt) / BMP180"); break;
        case 0x29: Serial.println("TSL2591 Light / VL53L0X Distance"); break;
        case 0x1E: Serial.println("HMC5883L Magnetometer"); break;
        case 0x1D: Serial.println("ADXL345 Accelerometer"); break;
        // 0x53 also used by ADXL345 Accelerometer (alt addr)
        default: Serial.println("Unknown device"); break;
      }
    }
  }

  if (found == 0) {
    Serial.println("  No I2C devices found.");
  } else {
    Serial.print("  Total: ");
    Serial.print(found);
    Serial.println(" device(s)");
  }
  Serial.println();
}

void scanAnalog() {
  Serial.println("--- Analog Pin (A0) ---");
  Serial.println("  Reading pin 5x and averaging...");
  Serial.println();

  for (int pin = A0; pin <= A0; pin++) {
    long sum = 0;
    int minVal = 1023, maxVal = 0;

    for (int i = 0; i < 5; i++) {
      int val = analogRead(pin);
      sum += val;
      if (val < minVal) minVal = val;
      if (val > maxVal) maxVal = val;
      delay(10);
    }

    int avg = sum / 5;
    float voltage = avg * (5.0 / 1023.0);
    int pinNum = pin - A0;

    // Only report pins that show a signal (not floating noise)
    // Floating pins typically read ~300-700 with high variance
    int range = maxVal - minVal;
    bool stable = (range < 20);
    bool hasSignal = stable && (avg < 200 || avg > 800);
    bool midStable = stable && (avg >= 200 && avg <= 800);

    if (hasSignal || midStable) {
      Serial.print("  A");
      Serial.print(pinNum);
      Serial.print(": avg=");
      Serial.print(avg);
      Serial.print(" (");
      Serial.print(voltage, 2);
      Serial.print("V) range=");
      Serial.print(range);

      if (avg < 10) Serial.print("  <- LOW / GND");
      else if (avg > 1013) Serial.print("  <- HIGH / VCC");
      else if (avg > 200 && avg < 800 && stable) Serial.print("  <- SENSOR? (stable mid-range)");

      Serial.println();
    }
  }

  Serial.println("  (Floating/noise pins hidden)");
  Serial.println();
}

void scanDigital() {
  // D1 Mini GPIOs: D0=16, D1=5, D2=4, D3=0, D4=2, D5=14, D6=12, D7=13, D8=15
  Serial.println("--- Digital Pins (D0-D8) ---");
  Serial.println("  Checking with INPUT_PULLUP...");
  Serial.println();

  int gpios[] = {16, 5, 4, 0, 2, 14, 12, 13, 15};
  const char* names[] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8"};
  int activePins = 0;
  for (int i = 0; i < 9; i++) {
    int pin = gpios[i];
    if (pin == 4 || pin == 5) continue;  // I2C (D1, D2)

    pinMode(pin, INPUT_PULLUP);
    delay(5);
    int pullupVal = digitalRead(pin);

    pinMode(pin, INPUT);
    delay(5);
    int floatVal = digitalRead(pin);

    // If pullup reads LOW, something is pulling the pin down (sensor/device connected)
    if (pullupVal == LOW) {
      activePins++;
      Serial.print("  ");
      Serial.print(names[i]);
      Serial.print(" (GPIO");
      Serial.print(pin);
      Serial.println("): ACTIVE LOW (device pulling pin down)");
    }
  }

  if (activePins == 0) {
    Serial.println("  No active digital pins detected.");
  }
  Serial.println();
}

void scanOneWire() {
  Serial.println("--- OneWire Probe (D3-D8) ---");
  // Simple presence pulse check for OneWire devices (DS18B20, etc.)
  int found = 0;

  int owPins[] = {0, 2, 14, 12, 13, 15};  // D3-D8
  const char* owNames[] = {"D3", "D4", "D5", "D6", "D7", "D8"};
  for (int i = 0; i < 6; i++) {
    int pin = owPins[i];
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    delayMicroseconds(480);
    pinMode(pin, INPUT);
    delayMicroseconds(70);

    if (digitalRead(pin) == LOW) {
      found++;
      Serial.print("  ");
      Serial.print(owNames[i]);
      Serial.print(" (GPIO");
      Serial.print(pin);
      Serial.println("): OneWire device detected (DS18B20 / DHT?)");
    }
    delayMicroseconds(410);
  }

  if (found == 0) {
    Serial.println("  No OneWire devices found on D3-D8.");
  }
  Serial.println();
}

void loop() {
  // Run once. Press reset to scan again.
}
