#include <Arduino.h>
#include "Encoder.h"
#include "App.h"
#include <Wifi.h>
#include "Wifi_manager.h"

Encoder encoder;
App app;
Wifi_manager wifi;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(5000);
  encoder.begin();
  Serial.println("ESP32 Booted.");
   if (!wifi.connectWifi())
    {
        wifi.startSetupPortal(); // check step by step on how it goes through everything
    }
    Serial.println("Connected to WiFi.");
    Serial.println("Starting Radar...");
}

void loop() {
  encoder.update();
  
}

// put function definitions here:
