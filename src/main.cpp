#include <Arduino.h>

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("ESP32 Booted.");
}

void loop() {
  Serial.println("Hello, from ESP32!");
  delay(1000);
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}