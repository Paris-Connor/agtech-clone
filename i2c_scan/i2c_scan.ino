// Quick I2C scanner for D1 Mini
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin(4, 5);  // SDA=D2(GPIO4), SCL=D1(GPIO5)
  delay(1000);
  Serial.println("\nI2C Scanner");
  Serial.println("Scanning...");

  int found = 0;
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    byte error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("  Device found at 0x");
      if (addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
      found++;
    }
  }

  if (found == 0) Serial.println("  No I2C devices found!");
  else { Serial.print("  Found "); Serial.print(found); Serial.println(" device(s)"); }
  Serial.println("Done.");
}

void loop() {}
