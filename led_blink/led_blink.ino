// LED Blink Test for Wemos D1 / D1 Mini (ESP8266)
// Built-in LED is on GPIO2 (D4) - active LOW

#define LED_PIN LED_BUILTIN  // GPIO2 on D1 Mini

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("LED Blink Test Started!");
}

void loop() {
  digitalWrite(LED_PIN, LOW);   // LED ON (active low)
  Serial.println("LED ON");
  delay(500);

  digitalWrite(LED_PIN, HIGH);  // LED OFF
  Serial.println("LED OFF");
  delay(500);
}
