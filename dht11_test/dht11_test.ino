// DHT11 Temperature & Humidity Test for Wemos D1 Mini
// Data pin: D5 (GPIO14)

#include <DHT.h>

#define DHTPIN 14      // D5 = GPIO14
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  Serial.println("\nDHT11 Sensor Test");
  Serial.println("─────────────────");
  dht.begin();
}

void loop() {
  float humidity = dht.readHumidity();
  float tempC = dht.readTemperature();
  float tempF = dht.readTemperature(true);

  if (isnan(humidity) || isnan(tempC)) {
    Serial.println("Error: Failed to read from DHT11 sensor!");
    delay(2000);
    return;
  }

  Serial.print("Temp: ");
  Serial.print(tempC, 1);
  Serial.print("°C  (");
  Serial.print(tempF, 1);
  Serial.print("°F)  |  Humidity: ");
  Serial.print(humidity, 1);
  Serial.println("%");

  delay(2000);  // DHT11 needs ~2s between reads
}
