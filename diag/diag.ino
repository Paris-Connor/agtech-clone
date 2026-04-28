// LED color comparison test
#define RED_PIN 12
#define GREEN_PIN 13
#define BLUE_PIN 15

void setup() {
  Serial.begin(115200);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
}

void loop() {
  Serial.println("GREEN (OK)");
  analogWrite(RED_PIN, 0); analogWrite(GREEN_PIN, 1023); analogWrite(BLUE_PIN, 0);
  delay(3000);

  Serial.println("ORANGE (WARNING)");
  analogWrite(RED_PIN, 1023); analogWrite(GREEN_PIN, 50); analogWrite(BLUE_PIN, 0);
  delay(3000);

  Serial.println("RED (DANGER)");
  analogWrite(RED_PIN, 1023); analogWrite(GREEN_PIN, 0); analogWrite(BLUE_PIN, 0);
  delay(3000);
}
