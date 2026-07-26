#include <Arduino.h>
#include "Encoder.h"
#include "App.h"
#include <WiFi.h>
#include "Wifi_manager.h"
#include "AircraftData.h"

Encoder encoder;
App app;
Wifi_manager wifi;
AircraftDataFetcher aircraftFetcher;

unsigned long lastAircraftFetch = 0;
const unsigned long kAircraftFetchIntervalMs = 10000;

void setup() {
  Serial.begin(115200);
  delay(5000);
  encoder.begin();
  Serial.println("ESP32 Booted.");

  if (!wifi.connectWifi())
  {
    wifi.startSetupPortal();
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Connected to WiFi.");
    Serial.println("Starting Radar...");
    aircraftFetcher.setLocation(51.5072f, -0.1276f, 100);
    
    // Give the connection a moment to stabilize before first fetch
    delay(2000);
    aircraftFetcher.fetchAndPrintAircrafts();
  }
}

void loop() {
  encoder.update();

  unsigned long now = millis();
  if (now - lastAircraftFetch >= kAircraftFetchIntervalMs)
  {
    lastAircraftFetch = now;
    if (WiFi.status() == WL_CONNECTED)
    {
      aircraftFetcher.fetchAndPrintAircrafts();
    }
  }
}
