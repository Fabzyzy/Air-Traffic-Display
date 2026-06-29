#include <Arduino.h>
#include "Encoder.h"
#include "App.h"

Encoder encoder;
App app;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  encoder.begin();
  Serial.println("ESP32 Booted.");
}

void loop() {
  encoder.update();
}

// put function definitions here:
