#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include "Encoder.h"
#include "App.h"
#include <WiFi.h>
#include "Wifi_manager.h"
#include "AircraftData.h"

Encoder encoder;
App app;
Wifi_manager wifi;
AircraftDataFetcher aircraftFetcher;

#define TFT_CS 5
#define TFT_DC 27
#define TFT_RST 33

Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_RST);

unsigned long lastAircraftFetch = 0;
const unsigned long kAircraftFetchIntervalMs = 10000;

void setup()
{
  Serial.begin(115200);
  Serial.println("==============================");
  Serial.println("Air Traffic Display Boot");
  Serial.println("==============================");
  Serial.println();
  Serial.println("Serial initialized");
  Serial.println();
  Serial.println("Boot complete");
  delay(5000);

  Serial.println("[BOOT] Initializing Display...");
  SPI.begin(18, -1, 23, 5);
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(0x0000);
  tft.setTextColor(0xFFFF);
  tft.setTextSize(2);
  tft.setCursor(20, 90);
  tft.print("Booting...");
  Serial.println("[BOOT] Display OK");

  Serial.println("[BOOT] Initializing Encoder...");
  encoder.begin();
  Serial.println("[BOOT] Encoder OK");

  Serial.println("[BOOT] Initializing WiFi...");
  if (!wifi.connectWifi())
  {
    wifi.startSetupPortal();
  }
  Serial.println("[BOOT] WiFi OK");

  Serial.println("[BOOT] Entering Main Menu");
  app.begin();

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Connected to WiFi.");
    Serial.println("Starting Radar...");
    aircraftFetcher.setLocation(51.5072f, -0.1276f, 100);

    delay(2000);
    aircraftFetcher.fetchAndPrintAircrafts();
  }
}

void loop()
{
  encoder.update();
  app.update();

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
