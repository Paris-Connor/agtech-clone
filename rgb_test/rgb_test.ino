// RGB LED Test for Wemos D1 Mini
// Red=D6 (GPIO12), Green=D7 (GPIO13), Blue=D1 (GPIO5)
// Common Cathode - HIGH = ON

#define RED_PIN   12  // D6
#define GREEN_PIN 13  // D7
#define BLUE_PIN   5  // D1

void setColor(int r, int g, int b) {
  // Common anode: invert PWM (1023 = off, 0 = full on)
  analogWrite(RED_PIN, 1023 - r);
  analogWrite(GREEN_PIN, 1023 - g);
  analogWrite(BLUE_PIN, 1023 - b);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nRGB LED Test");

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  setColor(0, 0, 0);
}

void loop() {
  Serial.println("RED");
  setColor(1023, 0, 0);
  delay(1000);

  Serial.println("GREEN");
  setColor(0, 1023, 0);
  delay(1000);

  Serial.println("BLUE");
  setColor(0, 0, 1023);
  delay(1000);

  Serial.println("YELLOW");
  setColor(1023, 1023, 0);
  delay(1000);

  Serial.println("CYAN");
  setColor(0, 1023, 1023);
  delay(1000);

  Serial.println("MAGENTA");
  setColor(1023, 0, 1023);
  delay(1000);

  Serial.println("WHITE");
  setColor(1023, 1023, 1023);
  delay(1000);

  // Smooth rainbow fade
  Serial.println("RAINBOW FADE...");
  for (int i = 0; i < 360; i += 2) {
    float h = i / 360.0;
    float r, g, b;
    int sector = (int)(h * 6);
    float f = h * 6 - sector;
    switch (sector % 6) {
      case 0: r = 1; g = f;     b = 0;     break;
      case 1: r = 1-f; g = 1;   b = 0;     break;
      case 2: r = 0; g = 1;     b = f;     break;
      case 3: r = 0; g = 1-f;   b = 1;     break;
      case 4: r = f; g = 0;     b = 1;     break;
      case 5: r = 1; g = 0;     b = 1-f;   break;
    }
    setColor((int)(r*1023), (int)(g*1023), (int)(b*1023));
    delay(20);
  }
}
